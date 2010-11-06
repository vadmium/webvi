/*
 * webvideo.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <getopt.h>
#include <time.h>
#include <vdr/plugin.h>
#include <vdr/tools.h>
#include <vdr/videodir.h>
#include <vdr/i18n.h>
#include <vdr/skins.h>
#include <libwebvi.h>
#include "menu.h"
#include "history.h"
#include "download.h"
#include "request.h"
#include "mimetypes.h"
#include "config.h"
#include "player.h"
#include "common.h"
#include "timer.h"

const char *VERSION               = "0.3.2";
static const char *DESCRIPTION    = trNOOP("Download video files from the web");
static const char *MAINMENUENTRY  = "Webvideo";
cMimeTypes *MimeTypes             = NULL;

class cPluginWebvideo : public cPlugin {
private:
  // Add any member variables or functions you may need here.
  cHistory history;
  cProgressVector summaries;
  cString templatedir;
  cString destdir;
  cString conffile;
  cString postprocesscmd;

  static int nextMenuID;

  void UpdateOSDFromHistory(const char *statusmsg=NULL);
  void UpdateStatusMenu(bool force=false);
  bool StartStreaming(const cString &streamurl);
  void ExecuteTimers(void);
  void HandleFinishedRequests(void);
  cString CreateWvtRef(const char *url);

public:
  cPluginWebvideo(void);
  virtual ~cPluginWebvideo();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return tr(DESCRIPTION); }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual void MainThreadHook(void);
  virtual cString Active(void);
  virtual const char *MainMenuEntry(void) { return MAINMENUENTRY; }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  };

int cPluginWebvideo::nextMenuID = 1;

cPluginWebvideo::cPluginWebvideo(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
}

cPluginWebvideo::~cPluginWebvideo()
{
  // Clean up after yourself!
  webvi_cleanup(0);
}

const char *cPluginWebvideo::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return "  -d DIR,   --downloaddir=DIR  Save downloaded files to DIR\n" \
         "  -t DIR,   --templatedir=DIR  Read video site templates from DIR\n" \
         "  -c FILE,  --conf=FILE        Load settings from FILE\n" \
         "  -p CMD,   --postprocess=CMD  Execute CMD after downloading\n";
}

bool cPluginWebvideo::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  static struct option long_options[] = {
    { "downloaddir", required_argument, NULL, 'd' },
    { "templatedir", required_argument, NULL, 't' },
    { "conf",        required_argument, NULL, 'c' },
    { "postprocess", required_argument, NULL, 'p' },
    { NULL }
  };

  int c;
  while ((c = getopt_long(argc, argv, "d:t:c:", long_options, NULL)) != -1) {
    switch (c) {
    case 'd':
      destdir = cString(optarg);
      break;
    case 't':
      templatedir = cString(optarg);
      break;
    case 'c':
      conffile = cString(optarg);
      break;
    case 'p':
      postprocesscmd = cString(optarg);
      break;
    default:
      return false;
    }
  }
  return true;
}

bool cPluginWebvideo::Initialize(void)
{
  // Initialize any background activities the plugin shall perform.

  // Test that run-time and compile-time libxml versions are compatible
  LIBXML_TEST_VERSION;

  // default values if not given on the command line
  if ((const char *)destdir == NULL)
    webvideoConfig->SetDownloadPath(cString(VideoDirectory));
  if ((const char *)conffile == NULL)
    conffile = AddDirectory(ConfigDirectory(Name()), "webvi.plugin.conf");

  webvideoConfig->ReadConfigFile(conffile);

  if ((const char *)destdir)
    webvideoConfig->SetDownloadPath(destdir);
  if ((const char *)templatedir)
    webvideoConfig->SetTemplatePath(templatedir);
  if ((const char *)postprocesscmd)
    webvideoConfig->SetPostProcessCmd(postprocesscmd);

  cString mymimetypes = AddDirectory(ConfigDirectory(Name()), "mime.types");
  const char *mimefiles [] = {"/etc/mime.types", (const char *)mymimetypes, NULL};
  MimeTypes = new cMimeTypes(mimefiles);

  if (webvi_global_init() != 0) {
    error("Failed to initialize libwebvi");
    return false;
  }

  cWebviTimerManager::Instance().Load(ConfigDirectory(Name()));

  cWebviThread::Instance().SetTemplatePath(webvideoConfig->GetTemplatePath());

  return true;
}

bool cPluginWebvideo::Start(void)
{
  // Start any background activities the plugin shall perform.
  cWebviThread::Instance().Start();

  return true;
}

void cPluginWebvideo::Stop(void)
{
  // Stop any background activities the plugin shall perform.
  cWebviThread::Instance().Stop();
  delete MimeTypes;

  cWebviTimerManager::Instance().Save(ConfigDirectory(Name()), Version());

  xmlCleanupParser();
}

void cPluginWebvideo::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.

  cWebviTimerManager::Instance().Save(ConfigDirectory(Name()), Version());
}

void cPluginWebvideo::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
  ExecuteTimers();

  HandleFinishedRequests();
}

void cPluginWebvideo::ExecuteTimers(void)
{
  static int counter = 0;

  // don't do this too often
  if (counter++ > 1800) {
    cWebviTimerManager::Instance().Update();
    counter = 0;
  }
}

void cPluginWebvideo::HandleFinishedRequests(void)
{
  bool forceStatusUpdate = false;
  cMenuRequest *req;
  cFileDownloadRequest *dlreq;
  cString streamurl;
  cWebviTimer *timer;
  cString timermsg;

  while ((req = cWebviThread::Instance().GetFinishedRequest())) {
    int cid = -1;
    int code = req->GetStatusCode();
    if (history.Current()) {
      cid = history.Current()->GetID();
    }

    debug("Finished request: %d (current: %d), type = %d, status = %d", 
          req->GetID(), cid, req->GetType(), code);

    if (req->Success()) {
      switch (req->GetType()) {
      case REQT_MENU:
	// Only change the menu if the request was launched from the
	// current menu.
	if (req->GetID() == cid) {
	  if (cid == 0) {
	    // Special case: replace the placeholder menu
	    history.Clear();
	  }

	  if (history.Current())
	    history.Current()->RememberSelected(menuPointers.navigationMenu->Current());
          history.TruncateAndAdd(new cHistoryObject(req->GetResponse(),
                                                    req->GetReference(),
                                                    nextMenuID++));
          UpdateOSDFromHistory();
        }
	break;

      case REQT_STREAM:
        streamurl = req->GetResponse();
        if (streamurl[0] == '\0')
          Skins.Message(mtError, tr("Streaming failed: no URL"));
        else if (!StartStreaming(streamurl))
          Skins.Message(mtError, tr("Failed to launch media player"));
	break;

      case REQT_FILE:
        dlreq = dynamic_cast<cFileDownloadRequest *>(req);

        if (dlreq) {
          for (int i=0; i<summaries.Size(); i++) {
            if (summaries[i]->GetRequest() == dlreq) {
              delete summaries[i];
              summaries.Remove(i);
              break;
            }
          }
        }

        timermsg = cString("");
        if (req->GetTimer()) {
          req->GetTimer()->RequestFinished(req->GetReference(), NULL);

          timermsg = cString::sprintf(" (%s)", tr("timer"));
        }

        Skins.Message(mtInfo, cString::sprintf(tr("One download completed, %d remains%s"),
                              cWebviThread::Instance().GetUnfinishedCount(),
                              (const char *)timermsg));
	forceStatusUpdate = true;
	break;

      case REQT_TIMER:
        timer = req->GetTimer();
        if (timer)
          timer->DownloadStreams(req->GetResponse(), summaries);
        break;

      default:
	break;
      }
    } else { // failed request
      if (req->GetType() == REQT_TIMER) {
        warning("timer request failed (%d: %s)",
                code, (const char*)req->GetStatusPharse());

        timer = req->GetTimer();
        if (timer)
          timer->CheckFailed(req->GetStatusPharse());
      } else {
        warning("request failed (%d: %s)",
		code, (const char*)req->GetStatusPharse());

        if (code == -2 || code == 402)
          Skins.Message(mtError, tr("Download aborted"));
        else
          Skins.Message(mtError, cString::sprintf(tr("Download failed (error = %d)"), code));

        dlreq = dynamic_cast<cFileDownloadRequest *>(req);
        if (dlreq) {
          for (int i=0; i<summaries.Size(); i++) {
            if (summaries[i]->GetRequest() == dlreq) {
              summaries[i]->AssociateWith(NULL);
              break;
            }
          }
        }

        if (req->GetTimer())
          req->GetTimer()->RequestFinished(req->GetReference(),
                 (const char*)req->GetStatusPharse());

        forceStatusUpdate = true;
      }
    }

    delete req;
  }

  UpdateStatusMenu(forceStatusUpdate);
}

cString cPluginWebvideo::Active(void)
{
  // Return a message string if shutdown should be postponed
  int c = cWebviThread::Instance().GetUnfinishedCount();
  if (c > 0)
    return cString::sprintf(tr("%d downloads not finished"), c);
  else
    return NULL;
}

cOsdObject *cPluginWebvideo::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.
  const char *mainMenuReference = "wvt:///?srcurl=mainmenu";
  const char *placeholderMenu = "<wvmenu><title>Webvideo</title></wvmenu>";
  const char *statusmsg = NULL;
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 100*1000*1000; // 100 ms

  menuPointers.navigationMenu = new cNavigationMenu(&history, summaries);

  cHistoryObject *hist = history.Home();
  if (!hist) {
    cWebviThread::Instance().AddRequest(new cMenuRequest(0, mainMenuReference));
    cHistoryObject *placeholder = new cHistoryObject(placeholderMenu, mainMenuReference, 0);
    history.TruncateAndAdd(placeholder);

    // The main menu response should come right away. Try to update
    // the menu here without having to wait for the next
    // MainThreadHook call by VDR main loop.
    for (int i=0; i<4; i++) {
      nanosleep(&ts, NULL);
      HandleFinishedRequests();
      if (history.Current() != placeholder) {
        return menuPointers.navigationMenu;
      }
    };

    statusmsg = tr("Retrieving...");
  }

  UpdateOSDFromHistory(statusmsg);
  return menuPointers.navigationMenu;
}

cMenuSetupPage *cPluginWebvideo::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return NULL;
}

bool cPluginWebvideo::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return false;
}

bool cPluginWebvideo::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}

const char **cPluginWebvideo::SVDRPHelpPages(void)
{
  static const char *HelpPages[] = {
    "PLAY <file>\n"
    "    Stream a media file.",
    "DWLD <file>\n"
    "    Download a media file.",
    NULL
    };
  return HelpPages;
}

cString cPluginWebvideo::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  if(strcasecmp(Command, "PLAY") == 0 || strcasecmp(Command, "DWLD") == 0) {
    if(*Option) {
      debug("SVDRP(%s, %s)", Command, Option);
      cString twvtref = CreateWvtRef(Option);
      if (twvtref != "") {
        cMenuRequest *req;
        if (strcasecmp(Command, "PLAY") == 0)
          req = new cStreamUrlRequest(0, twvtref);
        else
          req = new cFileDownloadRequest(0, twvtref, summaries.NewDownload());
        cWebviThread::Instance().AddRequest(req);
        ReplyCode = 250; // Ok
        return cString("Downloading video file");
      } else {
        ReplyCode = 550; // Requested action not taken
        return cString("Unable to parse URL");
      }
    } else {
      ReplyCode = 550; // Requested action not taken
      return cString("File name missing");
    }
  }

  return NULL;
}

cString cPluginWebvideo::CreateWvtRef(const char *url) {
  cString domain = parseDomain(url);
  if (domain == "")
    return "";
  
  char *encoded = URLencode(url);
  cString res = cString::sprintf("wvt:///%s/videopage.xsl?srcurl=%s",
                                 (const char *)domain, encoded);
  free(encoded);
  return res;
}

void cPluginWebvideo::UpdateOSDFromHistory(const char *statusmsg) {
  if (menuPointers.navigationMenu) {
    cHistoryObject *hist = history.Current();
    menuPointers.navigationMenu->Populate(hist, statusmsg);
    menuPointers.navigationMenu->Display();
  } else {
    debug("OSD is not ours.");
  }
}

void cPluginWebvideo::UpdateStatusMenu(bool force) {
  if (menuPointers.statusScreen && 
      (force || menuPointers.statusScreen->NeedsUpdate())) {
    menuPointers.statusScreen->Update();
  }
}

bool cPluginWebvideo::StartStreaming(const cString &streamurl) {
  cMediaPlayer *players[2];

  if (webvideoConfig->GetPreferXineliboutput()) {
    players[0] = new cXineliboutputPlayer();
    players[1] = new cMPlayerPlayer();
  } else {
    players[0] = new cMPlayerPlayer();
    players[1] = new cXineliboutputPlayer();
  }

  bool ret = false;
  for (int i=0; i<2; i++) {
    if (players[i]->Launch(streamurl)) {
      ret = true;
      break;
    }
  }

  for (int i=0; i<2 ; i++) {
    delete players[i];
  }

  return ret;
}

VDRPLUGINCREATOR(cPluginWebvideo); // Don't touch this!

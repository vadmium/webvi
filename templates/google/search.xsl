<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<wvmenu>
  <title>Google video search</title>

  <textfield name="q">
    <label>Search terms</label>
  </textfield>

  <itemlist name="so">
    <label>Sort by</label>
    <item value="0">Relevance</item>
    <item value="3">Rating</item>
    <item value="4">Popularity</item>
    <item value="1">Date</item>
  </itemlist>

  <itemlist name="dur">
    <label>Duration</label>
    <item value="">All durations</item>
    <item value="1">Short (&lt; 4 min)</item>
    <item value="2">Medium (4-20 min)</item>
    <item value="3">Long (&gt; 20 min)</item>
  </itemlist>

  <button>
    <label>Search</label>
    <submission>wvt:///google/searchresults.xsl?srcurl=<xsl:value-of select="str:encode-uri('http://video.google.com/videosearch?q={q}&amp;so={so}&amp;dur={dur}', true())"/></submission>
  </button>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

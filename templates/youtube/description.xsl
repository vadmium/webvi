<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  xmlns:atom="http://www.w3.org/2005/Atom"
  xmlns:media="http://search.yahoo.com/mrss/"
  xmlns:gd="http://schemas.google.com/g/2005"
  xmlns:yt="http://gdata.youtube.com/schemas/2007"
  exclude-result-prefixes="atom str media gd yt">

<!-- Convert $seconds to hours:min:sec format -->
<xsl:template name="pretty-print-seconds">
  <xsl:param name="seconds"/>

  <xsl:variable name="sec" select="$seconds mod 60"/>
  <xsl:variable name="min" select="floor($seconds div 60) mod 60"/>
  <xsl:variable name="hour" select="floor($seconds div 3600)"/>

  <xsl:value-of select="concat($hour, ':', format-number($min, '00'), ':', format-number($sec, '00'))"/>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="/atom:entry/atom:title"/></title>
  <textarea>
    <label><xsl:value-of select="/atom:entry/media:group/media:description"/></label>
  </textarea>

  <textarea>
    <label>Duration: <xsl:call-template name="pretty-print-seconds">
        <xsl:with-param name="seconds">
          <xsl:value-of select="/atom:entry/media:group/yt:duration/@seconds"/>
        </xsl:with-param>
      </xsl:call-template>
    </label>
  </textarea>

  <textarea>
    <label>Views: <xsl:value-of select="/atom:entry/yt:statistics/@viewCount"/></label>
  </textarea>

  <textarea>
    <label>Rating: <xsl:value-of select="/atom:entry/gd:rating/@average"/></label>
  </textarea>

  <textarea>
    <label>Published: <xsl:value-of select="str:replace(str:replace(/atom:entry/atom:published, '.000', ' '), 'T', ' ')"/></label>
  </textarea>

  <xsl:if test="/atom:entry/atom:link[@rel='http://gdata.youtube.com/schemas/2007#video.responses']">
    <link>
      <label>Video responses</label>
      <ref>wvt:///youtube/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(/atom:entry/atom:link[@rel='http://gdata.youtube.com/schemas/2007#video.responses']/@href, true())"/></ref>
    </link>
  </xsl:if>

  <xsl:if test="/atom:entry/atom:link[@rel='http://gdata.youtube.com/schemas/2007#video.related']">
    <link>
      <label>Related videos</label>
      <ref>wvt:///youtube/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(/atom:entry/atom:link[@rel='http://gdata.youtube.com/schemas/2007#video.related']/@href, true())"/></ref>
    </link>
  </xsl:if>

  <link>
    <label>Download this video</label>
    <stream>wvt:///youtube/video.xsl?srcurl=http://www.youtube.com/watch?v=<xsl:value-of select="/atom:entry/media:group/yt:videoid"/></stream>
  </link>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

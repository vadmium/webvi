<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template name="mediaurl">
<mediaurl>
  <title><xsl:value-of select="concat(/Playerdata/Behavior/Program/@program_name, ' ', /Playerdata/Behavior/Program/@episode_name)"/></title>

  <xsl:choose>
    <xsl:when test="starts-with(/Playerdata/Clip/SourceFile, 'rtmp://') or 
                    starts-with(/Playerdata/Clip/SourceFile, 'rtmpe://') or
                    starts-with(/Playerdata/Clip/SourceFile, 'rtmpt://') or
                    starts-with(/Playerdata/Clip/SourceFile, 'rtmpte://') or
                    starts-with(/Playerdata/Clip/SourceFile, 'rtmps://')">
      <url priority="50">wvt:///bin/ruutu-dl?contenttype=video/x-flv&amp;arg=<xsl:value-of select="str:encode-uri(/Playerdata/Clip/SourceFile, true())"/>&amp;arg=http://www.ruutu.fi/video</url>
    </xsl:when>
    <xsl:otherwise>
      <url priority="50"><xsl:value-of select="/Playerdata/Clip/SourceFile"/></url>
    </xsl:otherwise>
  </xsl:choose>
</mediaurl>
</xsl:template>

<xsl:template match="/">
  <xsl:call-template name="mediaurl"/>
</xsl:template>

</xsl:stylesheet>

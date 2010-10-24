<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="docurl"/>

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
  <title><xsl:value-of select="/Playerdata/Behavior/Program/@program_name"/></title>

  <xsl:if test="/Playerdata/Behavior/Program/@description">
    <textarea>
      <label><xsl:value-of select="/Playerdata/Behavior/Program/@description"/></label>
    </textarea>
  </xsl:if>

  <textarea>
    <label><xsl:value-of select="/Playerdata/Behavior/Program/@episode_name"/></label>
  </textarea>

  <textarea>
    <label>Kesto: <xsl:call-template name="pretty-print-seconds">
        <xsl:with-param name="seconds">
          <xsl:value-of select="/Playerdata/Behavior/Program/@episode_duration"/>
        </xsl:with-param>
      </xsl:call-template>
    </label>
  </textarea>

  <link>
    <label>Lataa</label>
    <stream>wvt:///www.ruutu.fi/video.xsl?srcurl=<xsl:value-of select="str:encode-uri($docurl, true())"/></stream>
  </link>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

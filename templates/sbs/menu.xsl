<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="links"/>

<xsl:template match="/">
<wvmenu>
  <xsl:choose>
  <xsl:when test="$links">
    <xsl:for-each select="str:split($links)">
    <link>
      <label>
        <xsl:value-of select="str:decode-uri(str:split(., '=')[1])"/>
      </label>
      <ref>
        <xsl:value-of select="str:decode-uri(str:split(., '=')[2])"/>
      </ref>
    </link>
    </xsl:for-each>
  </xsl:when>
  <xsl:otherwise>
    <xsl:apply-templates/>
  </xsl:otherwise>
  </xsl:choose>
</wvmenu>
</xsl:template>

<xsl:template match="/settings//setting[@name='menuURL']">
<link>
  <label>Menu</label>
  <ref>wvt:///sbs/menu.xsl?srcurl=http://player.sbs.com.au<xsl:value-of select="@value"/></ref>
</link>
</xsl:template>

<xsl:template match="/menu/title">
<title><xsl:value-of select="."/></title>
</xsl:template>

<xsl:template match="/menu/menu">
<link>
  <label><xsl:value-of select="title"/></label>
  <ref>
    <xsl:text>wvt:///sbs/menu.xsl?param=links,</xsl:text>
    <xsl:apply-templates select="menu"/>
  </ref>
</link>
</xsl:template>

<xsl:template match="/menu/menu//menu">
  <xsl:variable name="label" select="str:encode-uri(title, true())"/>
  <xsl:variable name="ref" select="str:encode-uri(concat(
    'wvt:///sbs/menu.xsl?srcurl=http://player.sbs.com.au',
    playlist/@xmlSrc), true())"/>
  
  <xsl:value-of select="str:encode-uri(concat($label, '=', $ref), true())"/>
  <xsl:if test="position() != last()">
    <xsl:value-of select="str:encode-uri(' ', true())"/>
  </xsl:if>
</xsl:template>

<xsl:template match="/playlist/video">
<link>
  <label><xsl:value-of select="title"/></label>
  <ref>wvt:///sbs/menu.xsl?srcurl=http://player.sbs.com.au<xsl:value-of select="@src"/></ref>
</link>
</xsl:template>

<xsl:template match="/smil/body/switch/video">
<link>
  <label><xsl:value-of select="@system-bitrate div 1000"/> kbit/s</label>
  <stream>wvt:///sbs/stream.xsl?param=base,<xsl:value-of select="str:encode-uri(//meta/@base, true())"/>&amp;param=src,<xsl:value-of select="str:encode-uri(@src, true())"/></stream>
</link>
</xsl:template>

</xsl:stylesheet>

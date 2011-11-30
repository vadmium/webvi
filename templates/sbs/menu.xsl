<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template match="/">
<wvmenu>
  <xsl:apply-templates/>
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

<xsl:template match="/menu//menu[not(playlist)]/title">
<textarea>
  <label><xsl:value-of select="."/></label>
</textarea>
</xsl:template>

<xsl:template match="/menu//playlist">
<link>
  <label><xsl:value-of select="../title"/></label>
  <ref>wvt:///sbs/menu.xsl?srcurl=http://player.sbs.com.au<xsl:value-of select="@xmlSrc"/></ref>
</link>
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

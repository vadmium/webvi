<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  xmlns:exsl="http://exslt.org/common"
  exclude-result-prefixes="str exsl">

<xsl:import href="../wvmenu-ref.xsl"/>

<xsl:template match="/">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="/settings//setting[@name='menuURL']">
  <xsl:apply-templates
    select="document(concat('http://player.sbs.com.au', @value))"/>
</xsl:template>

<xsl:template match="/menu | /playlist | /smil">
<wvmenu>
  <xsl:apply-templates/>
</wvmenu>
</xsl:template>

<xsl:template match="/menu/title">
<title><xsl:value-of select="."/></title>
</xsl:template>

<xsl:template match="/menu//menu">
<link>
  <label><xsl:value-of select="title"/></label>
  
  <xsl:if test="not(playlist)">
    <xsl:variable name="wvmenu"><wvmenu>
      <xsl:apply-templates select="menu"/>
    </wvmenu></xsl:variable>
  
  <ref>
    <xsl:apply-templates mode="wvmenu-ref" select="exsl:node-set($wvmenu)"/>
  </ref>
  </xsl:if>
  
  <xsl:for-each select="playlist">
  <ref>
    <xsl:value-of select="concat(
      'wvt:///sbs/menu.xsl?srcurl=http://player.sbs.com.au', @xmlSrc)"/>
  </ref>
  </xsl:for-each>
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

<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<!-- Helper function for extracting progId from href -->
<xsl:template name="extract_pid">
  <xsl:param name="href"/>

  <xsl:choose>
    <xsl:when test="contains(substring-after($href, 'progId='), '&amp;')">
      <xsl:value-of select="substring-before(substring-after($href, 'progId='), '&amp;')"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="substring-after($href, 'progId=')"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- Create video link -->
<xsl:template name="video_link">
  <xsl:param name="title"/>
  <xsl:param name="href"/>
  <xsl:variable name="pid">
    <xsl:call-template name="extract_pid"> 
      <xsl:with-param name="href" select="$href"/> 
    </xsl:call-template> 
  </xsl:variable>

  <link>
    <label><xsl:value-of select="$title"/></label>
    <ref>wvt:///katsomo.fi/description.xsl?srcurl=<xsl:value-of select="str:encode-uri($href, true())"/></ref>
    <stream>wvt:///katsomo.fi/video.xsl?param=pid,<xsl:value-of select="$pid"/>&amp;param=title,<xsl:value-of select="str:encode-uri($title, true())"/></stream>
  </link>
</xsl:template>

<xsl:template match="id('search_episodes')/div">
  <xsl:call-template name="video_link">
    <xsl:with-param name="title" select="normalize-space(h4/a)"/>
    <xsl:with-param name="href" select="h4/a/@href"/>
  </xsl:call-template>
</xsl:template>

<xsl:template match="id('search_clips')/div">
  <xsl:call-template name="video_link">
    <xsl:with-param name="title" select="normalize-space(concat(p/a[1], ' ', h4/a))"/>
    <xsl:with-param name="href" select="h4/a/@href"/>
  </xsl:call-template>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="/html/head/title"/></title>

  <!-- Program search results require javascript. This is not a -->
  <!-- problem because all programs are listed on the main page -->
  <!-- anyway. -->
  <!-- <xsl:apply-templates select="id('search_results')/div[contains(@class, 'item')]"/> -->

  <textarea>
    <label>Jaksot</label>
  </textarea>
  <xsl:apply-templates select="id('search_episodes')/div[contains(@class, 'item')]"/>

  <textarea>
    <label>Klipit</label>
  </textarea>
  <xsl:apply-templates select="id('search_clips')/div[contains(@class, 'item')]"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template name="main">
<xsl:variable name="episodetitle">
  <xsl:choose>
    <xsl:when test="contains(/html/head/meta[@name='title']/@content, ' - Katsomo')">
      <xsl:value-of select="substring-before(/html/head/meta[@name='title']/@content, ' - Katsomo')"/>
    </xsl:when>
    <xsl:when test="contains(/html/head/meta[@name='title']/@content, ' - MTV3 Katsomo')">
      <xsl:value-of select="substring-before(/html/head/meta[@name='title']/@content, ' - MTV3 Katsomo')"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="/html/head/meta[@name='title']/@content"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>

<xsl:variable name="pid" select="substring-before(substring-after(/html/body/script[contains(., 'userNav.play(')], 'userNav.play('), ',')"/>

<xsl:if test="$pid">
  <mediaurl>
    <title><xsl:value-of select="$episodetitle"/></title>
    <url><xsl:value-of select="concat('http://www.katsomo.fi/metafile.asx?p=', $pid, '&amp;bw=800')"/></url>
  </mediaurl>
</xsl:if>
</xsl:template>

<xsl:template match="/">
  <xsl:call-template name="main"/>
</xsl:template>

</xsl:stylesheet>

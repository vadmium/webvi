<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="docurl"/>
<xsl:param name="live"/>

<xsl:template match="/">
<xsl:variable name="episodetitle">
  <xsl:choose>
    <xsl:when test="contains(/html/head/meta[@name='title']/@content, ' - Katsomo')">
      <xsl:value-of select="normalize-space(substring-before(/html/head/meta[@name='title']/@content, ' - Katsomo'))"/>
    </xsl:when>
    <xsl:when test="contains(/html/head/meta[@name='title']/@content, ' - MTV3 Katsomo')">
      <xsl:value-of select="normalize-space(substring-before(/html/head/meta[@name='title']/@content, ' - MTV3 Katsomo'))"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="normalize-space(/html/head/meta[@name='title']/@content)"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>

<xsl:variable name="title">
  <xsl:choose>
    <xsl:when test="normalize-space(id('program_content')/h1) = $episodetitle">
      <xsl:value-of select="$episodetitle"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="normalize-space(concat(id('program_content')/h1, ': ', $episodetitle))"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>

<xsl:variable name="pid" select="substring-after($docurl, 'progId=')"/>

<wvmenu>
  <title><xsl:value-of select="$title"/></title>

  <textarea>
    <label><xsl:value-of select="/html/head/meta[@name='description']/@content"/></label>
  </textarea>

  <xsl:choose>
    <xsl:when test="$live">
      <textarea>
	<label><xsl:value-of select="$live"/></label>
      </textarea>
    </xsl:when>

    <xsl:when test="$pid">
      <link>
	<label>Download this video</label>
	<stream>wvt:///katsomo.fi/video.xsl?param=pid,<xsl:value-of select="$pid"/>&amp;param=title,<xsl:value-of select="str:encode-uri($title, true())"/></stream>
      </link>
    </xsl:when>
  </xsl:choose>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

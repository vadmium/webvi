<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template match="div[contains(@class, 'item')]">
  <xsl:choose>
    <xsl:when test="div[@class='row1']/span[@class='live']">
      <xsl:variable name="upcoming" select="concat(div[@class='row1']/span[@class='live'], ' ', div[@class='row1']/span[contains(@class, 'time')])"/>

      <link>
	<label><xsl:value-of select="normalize-space(concat(div[@class='row2']/a, ' ', $upcoming))"/></label>
	<ref>wvt:///katsomo.fi/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(div[@class='row2']/a/@href, true())"/>&amp;param=live,<xsl:value-of select="str:encode-uri($upcoming, true())"/></ref>
      </link>
    </xsl:when>
    <xsl:otherwise>
      <link>
	<label><xsl:value-of select="div[@class='row2']/a"/></label>
	<ref>wvt:///katsomo.fi/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(div[@class='row2']/a/@href, true())"/></ref>
	<stream>wvt:///katsomo.fi/video.xsl?param=pid,<xsl:value-of select="substring-after(div[@class='row2']/a/@href, 'progId=')"/>&amp;param=title,<xsl:value-of select="normalize-space(str:encode-uri(concat(id('program_content')/h1, ' ', div[@class='row2']/a), true()))"/></stream>
      </link>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="/html/head/meta[@name='title']/@content"/></title>

  <xsl:apply-templates select="//div[@class='content episode']/div[contains(@class, 'item') and not(contains(@class, 'not-free'))]"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

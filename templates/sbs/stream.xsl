<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="base"/>
<xsl:param name="src"/>

<xsl:template match="/">
<mediaurl>
  <title><xsl:value-of select="str:tokenize($src, '/')[last()]"/></title>
  <url>
    <xsl:text>wvt:///bin/rtmpdump?arg=--rtmp&amp;arg=</xsl:text>
    <xsl:value-of select="str:encode-uri($base, true())"/>
    <xsl:text>&amp;arg=--playpath&amp;arg=</xsl:text>
    <xsl:value-of select="str:encode-uri($src, true())"/>
    <xsl:text>&amp;arg=--quiet&amp;arg=--</xsl:text>
  </url>
</mediaurl>
</xsl:template>

</xsl:stylesheet>

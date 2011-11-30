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
  <url>wvt:///bin/rtmpdump?arg=--rtmp&amp;arg=<xsl:value-of select="str:encode-uri($base, true())"/>&amp;arg=--playpath&amp;arg=<xsl:value-of select="str:encode-uri($src, true())"/>&amp;arg=--quiet</url>
</mediaurl>
</xsl:template>

</xsl:stylesheet>

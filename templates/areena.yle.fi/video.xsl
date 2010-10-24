<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:param name="title"/>
<xsl:param name="docurl"/>

<xsl:template name="mediaurl">
  <xsl:param name="streamtitle"/>

  <mediaurl>
    <title>
      <xsl:value-of select="$streamtitle"/>
    </title>
    <url>wvt:///bin/yle-dl?contenttype=video/x-flv&amp;arg=<xsl:value-of select="str:encode-uri($docurl, true())"/></url>
  </mediaurl>
</xsl:template>

<xsl:template match="/">
  <xsl:call-template name="mediaurl">
    <xsl:with-param name="streamtitle" select="$title"/>
  </xsl:call-template>
</xsl:template>

</xsl:stylesheet>

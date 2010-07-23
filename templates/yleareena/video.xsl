<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:param name="title"/>
<xsl:param name="docurl"/>

<xsl:template match="/">
  <mediaurl>
    <title><xsl:value-of select="$title"/></title>
    <url>wvt:///bin/yle-dl?contenttype=video/x-flv&amp;arg=<xsl:value-of select="str:encode-uri($docurl, true())"/></url>
</mediaurl>

</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="title"/>
<xsl:param name="desc"/>
<xsl:param name="pubdate"/>
<xsl:param name="pid"/>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="$title"/></title>

  <textarea>
    <label><xsl:value-of select="$desc"/></label>
  </textarea>

  <textarea>
    <label><xsl:value-of select="$pubdate"/></label>
  </textarea>

  <link>
    <label>Lataa</label>
    <stream>wvt:///www.sub.fi/video.xsl?param=pid,<xsl:value-of select="$pid"/>&amp;param=title,<xsl:value-of select="str:encode-uri($title, true())"/></stream>
  </link>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

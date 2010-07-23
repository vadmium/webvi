<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="tr">
  <link>
    <label><xsl:value-of select="td[1]/a"/></label>
    <ref>wvt:///yleareena/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(concat(td[1]/a/@href, '/feed/rss'), true())"/>&amp;param=title,<xsl:value-of select="str:encode-uri(td[1]/a, true())"/></ref>
  </link>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title>Ohjelmat A-Ö</title>

  <xsl:apply-templates select="id('programlist-ao')/table/tbody/tr[td]"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

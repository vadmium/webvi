<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template match="/">
<wvmenu>
  <title>MTV3 Katsomo</title>

  <link>
    <label>Haku</label>
    <ref>wvt:///katsomo/search.xsl</ref>
  </link>

  <xsl:for-each select="id('mainMenu')/li[a/@href != '/']"> 
    <link>
      <label><xsl:value-of select="a"/></label>
      <ref>wvt:///katsomo/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(a/@href, true())"/></ref>
    </link>
  </xsl:for-each>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template match="id('all_programs_tab1')//a">
  <link>
    <label><xsl:value-of select="normalize-space(string(.))"/></label>
    <ref>wvt:///katsomo.fi/program.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></ref>
  </link>
</xsl:template>


<xsl:template match="/">
<wvmenu>
  <title>MTV3 Katsomo</title>

  <link>
    <label>Haku</label>
    <ref>wvt:///katsomo.fi/search.xsl</ref>
  </link>

  <xsl:apply-templates select="id('all_programs_tab1')//a"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template match="/">
<wvmenu>
  <title>Subin netti-TV</title>

  <xsl:for-each select="//div[@class='netissakaikki']/ul/li/a"> 
    <link>
      <label><xsl:value-of select="."/></label>
      <ref>wvt:///www.sub.fi/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></ref>
    </link>
  </xsl:for-each>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

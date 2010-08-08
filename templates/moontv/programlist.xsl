<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<wvmenu>
  <title>MoonTV ohjelmat</title>

  <xsl:for-each select="//ul[@id='ohjelmat-list']/li/h5/a">
    <link>
      <label><xsl:value-of select="."/></label>
      <ref>wvt:///moontv/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(concat('http://moontv.fi',@href), true())"/></ref>
    </link>
  </xsl:for-each>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

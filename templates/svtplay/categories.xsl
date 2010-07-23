<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<wvmenu>
  <title>SVT Play</title>

  <xsl:for-each select="//div[@id='categorylist']//ul/li//a">
    <link>
      <label><xsl:value-of select="span[@class='category-header']"/></label>
      <ref>wvt:///svtplay/navigation.xsl?srcurl=<xsl:value-of select="@href"/></ref>
    </link>
  </xsl:for-each>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

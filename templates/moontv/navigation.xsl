<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="//div[@id='content']/h2"/></title>
  <xsl:for-each select="//div[@class='entry']">
    <link>
      <label><xsl:value-of select="h3"/></label>
      <ref>wvt:///moontv/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(h3/a/@href, true())"/></ref>
      <stream>wvt:///moontv/video.xsl?srcurl=<xsl:value-of select="str:encode-uri(h3/a/@href, true())"/></stream>
    </link>
  </xsl:for-each>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

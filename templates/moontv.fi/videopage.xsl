<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
  <mediaurl>
    <!-- Jakson/ohjelman nimi -->
    <title><xsl:value-of select="normalize-space(concat(substring-before(//div[@id='single-entry']/h3, '&#8211;'), ' - ' , substring-after(//div[@id='single-entry']/h3, '&#8211;')))"/></title>
    <url priority="50"><xsl:value-of select='substring-before(substring-after(//div[@id="single-entry"]/script, "file&apos;,&apos;"), "&apos;")'/></url>
  </mediaurl>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="normalize-space(concat(substring-before(//div[@id='single-entry']/h3, '&#8211;'), ' - ' , substring-after(//div[@id='single-entry']/h3, '&#8211;')))"/></title>
  <textarea>
    <label><xsl:value-of select="//div[@id='single-entry']/p"/></label>
  </textarea>
  <textarea>
    <!-- Julkaistu -->
    <label>Julkaistu: <xsl:value-of select="//div[@id='single-entry']/div[@class='entry-footer']/p"/></label>
  </textarea>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

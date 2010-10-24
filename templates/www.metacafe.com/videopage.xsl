<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
  <mediaurl>
    <title><xsl:value-of select="normalize-space(id('ItemTitle'))"/></title>
    <url><xsl:value-of select="str:decode-uri(substring-before(substring-after(//param[@name='flashvars']/@value, 'mediaURL='), '&amp;'))"/></url>
  </mediaurl>
</xsl:template>

</xsl:stylesheet>

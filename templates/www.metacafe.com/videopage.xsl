<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
  <xsl:variable name="mediadata">
    <xsl:value-of select="str:decode-uri(substring-before(substring-after(//param[@name='flashvars']/@value, 'mediaData='), '&amp;'))"/>
  </xsl:variable>

  <mediaurl>
    <title><xsl:value-of select="normalize-space(id('ItemTitle'))"/></title>
    <url><xsl:value-of select="concat(str:replace(substring-before(substring-after($mediadata, 'mediaURL&quot;:&quot;'), '&quot;'), '\/', '/'), '?__gda__=', substring-before(substring-after($mediadata, 'key&quot;:&quot;'), '&quot;'))"/></url>
  </mediaurl>
</xsl:template>

</xsl:stylesheet>

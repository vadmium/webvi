<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<mediaurl>
  <title>
    <xsl:choose>
      <xsl:when test="normalize-space(//h1/a/img/@alt)">
        <xsl:value-of select="concat(normalize-space(//h1/a/img/@alt), ' ', //div[@class='info']//h2)"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="concat(normalize-space(//h1/a), ' ', //div[@class='info']//h2)"/>
      </xsl:otherwise>
    </xsl:choose>
  </title>

  <url priority="50"><xsl:value-of select="substring-before(substring-after((//object/param[@name='flashvars'])[1]/@value, 'pathflv='), '&amp;')"/></url>
  <url priority="40"><xsl:value-of select="//a[@class='external-player']/@href"/></url>
</mediaurl>
</xsl:template>

</xsl:stylesheet>

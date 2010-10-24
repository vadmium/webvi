<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="div[@class='info']/ul">
  <textarea>
    <label>
      <xsl:value-of select="normalize-space(li[@class='title']/div)"/>
    </label>
  </textarea>
  <textarea>
    <label>
      <xsl:value-of select="normalize-space(li[@class='episode']/div)"/>
    </label>
  </textarea>
  <textarea>
    <label>
      <xsl:value-of select="concat(normalize-space(li[1]/span[2]), ' ', normalize-space(li/span[2]/following-sibling::text()))"/>
    </label>
   </textarea>
</xsl:template>

<xsl:template match="/">
<wvmenu>
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

  <xsl:apply-templates select="//div[@class='info']/ul"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

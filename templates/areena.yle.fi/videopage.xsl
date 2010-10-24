<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:import href="video.xsl"/>

<xsl:param name="docurl"/>

<xsl:template match="/">
  <xsl:call-template name="mediaurl">
    <xsl:with-param name="streamtitle" select="normalize-space(concat(//div[@class='basic']/h1, ' - ', //li[contains(text(), 'Julkaistu:')]/span))"/>
  </xsl:call-template>
</xsl:template>

</xsl:stylesheet>

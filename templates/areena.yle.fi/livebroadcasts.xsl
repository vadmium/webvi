<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="text()"/>

<!-- Käynnissä olevat lähetykset -->
<xsl:template match="div[@class='ongoing']//div[@class='showlistitem-description']">
  <link>
    <label><xsl:value-of select="a"/></label>
    <stream>wvt:///areena.yle.fi/livestream.xsl?param=stream,<xsl:value-of select='substring-before(substring-after(a/@onclick, "stream&apos;, &apos;"), "&apos;")'/></stream>
  </link>
</xsl:template>

<!-- "Aina suorana" -->
<xsl:template match="div[contains(@class, 'live-container')]">
  <link>
    <label><xsl:value-of select="h2/span/a"/></label>
    <stream>wvt:///areena.yle.fi/livestream.xsl?param=stream,<xsl:value-of select='substring-before(substring-after(h2/span/a/@onclick, "stream&apos;, &apos;"), "&apos;")'/></stream>
  </link>
</xsl:template>

<!-- Tulevat lähetykset -->
<xsl:template match="div[@class='upcoming']/div/div[@class='showlistitem-description']">
  <textarea>
    <label><xsl:value-of select="h3"/>, <xsl:value-of select="ul/li[1]"/></label>
  </textarea>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title>Suorat lähetykset</title>

  <xsl:apply-templates select="id('liveshows')/div[@class='ongoing']"/>

  <xsl:apply-templates select="id('liveshows')/div/div[contains(@class, 'live-container')]"/>

  <textarea>
    <label>Tulossa seuraavaksi:</label>
  </textarea>
  <xsl:apply-templates select="id('liveshows')/div[@class='upcoming']"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

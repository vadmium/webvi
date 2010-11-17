<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="docurl"/>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="normalize-space(//h1[@class='cliptitle'])"/></title>
  <textarea>
    <label><xsl:value-of select="normalize-space(id('relatedinfo')//div[@class='relatedinfo-text description'])"/></label>
  </textarea>
  <textarea>
    <!-- Kesto -->
    <label><xsl:value-of select="id('relatedinfo')/div/div/div[@class='relatedinfo-text meta']/ul/li[contains(., 'Kesto')]"/></label>
  </textarea>
  <textarea>
    <!-- Julkaistu -->
    <label><xsl:value-of select="id('relatedinfo-more')/div/div[1]/ul/li[contains(., 'Julkaistu')]"/></label>
  </textarea>
  <textarea>
    <!-- Kieli -->
    <label><xsl:value-of select="id('relatedinfo-more')/div/div[2]/ul[1]/li[1]"/></label>
  </textarea>
  <textarea>
    <!-- Kanava -->
    <label><xsl:value-of select="id('relatedinfo')//div[@class='relatedinfo-text meta']/ul/li[1]"/></label>
  </textarea>
  <link>
    <label>Download this video</label>
    <stream>wvt:///areena.yle.fi/videopage.xsl?srcurl=<xsl:value-of select="$docurl"/></stream>
  </link>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

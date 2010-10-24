<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:param name="docurl"/>

<xsl:template match="/rss/channel/item">
  <link>
    <label><xsl:value-of select="concat(category, ': ',title)"/></label>
    <ref>wvt:///moontv.fi/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(link, true())"/></ref>
    <!-- MoonTV rss-linkit -->
    <stream>wvt:///moontv.fi/videopage.xsl?srcurl=<xsl:value-of select="link"/></stream>
  </link>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <!-- Videolinkit -->
  <xsl:if test="/rss">
    <title><xsl:value-of select="/rss/channel/title"/></title>
    <xsl:apply-templates select="/rss/channel/item"/>
  </xsl:if>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

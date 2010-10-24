<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:param name="searchtoken"/>

<xsl:template match="/">
<wvmenu>
  <title>Search results</title>

  <xsl:for-each select="//div[@class='title']/a">
    <link>
      <label><xsl:value-of select="."/></label>
      <stream>wvt:///vimeo.com/video.xsl?srcurl=http://www.vimeo.com/moogaloop/load/clip:<xsl:value-of select="str:split(@href, '/')[last()]"/></stream>
      <ref>wvt:///vimeo.com/description.xsl?srcurl=http://vimeo.com/api/v2/video/<xsl:value-of select="str:split(@href, '/')[last()]"/>.xml</ref>
    </link>
  </xsl:for-each>

  <xsl:for-each select="//div[@class='pagination']/ul/li[@class='arrow']/a">
    <link>
      <xsl:if test="img/@alt = 'previous'">
        <label>Previous</label>
      </xsl:if>
      <xsl:if test="img/@alt = 'next'">
        <label>Next</label>
      </xsl:if>
      <ref>wvt:///vimeo.com/searchresults.xsl?srcurl=<xsl:value-of select="./@href"/>&amp;HTTP-header=cookie,xsrft%3D<xsl:value-of select="$searchtoken"/>;searchtoken%3D<xsl:value-of select="$searchtoken"/>&amp;param=searchtoken,<xsl:value-of select="$searchtoken"/></ref>
    </link>
  </xsl:for-each>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

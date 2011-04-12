<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="searchtoken"/>

<xsl:template match="/">
<wvmenu>
  <title>Search results</title>

  <xsl:for-each select="//div[@class='title']/a">
    <link>
      <xsl:variable name="clipid">
	<xsl:choose>
	  <xsl:when test="contains(@href, '?')">
	    <xsl:value-of select="str:split(substring-before(@href, '?'), '/')[last()]"/>
	  </xsl:when>
	    
	  <xsl:otherwise>
	    <xsl:value-of select="str:split(@href, '/')[last()]"/>
	  </xsl:otherwise>
	</xsl:choose>
      </xsl:variable>

      <label><xsl:value-of select="."/></label>
      <stream>wvt:///vimeo.com/video.xsl?srcurl=http://www.vimeo.com/moogaloop/load/clip:<xsl:value-of select="$clipid"/>&amp;HTTP-header=user-agent,Wget/1.2%20%28linux-gnu%29</stream>
      <ref>wvt:///vimeo.com/description.xsl?srcurl=http://vimeo.com/api/v2/video/<xsl:value-of select="$clipid"/>.xml</ref>
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
      <ref>wvt:///vimeo.com/searchresults.xsl?srcurl=<xsl:value-of select="translate(./@href, ' ', '+')"/>&amp;HTTP-header=cookie,uid%3D0;searchtoken%3D<xsl:value-of select="$searchtoken"/>&amp;param=searchtoken,<xsl:value-of select="$searchtoken"/></ref>
    </link>
  </xsl:for-each>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

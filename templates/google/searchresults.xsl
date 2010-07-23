<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<wvmenu>
  <title>Search results</title>

  <xsl:choose>
    <xsl:when test="not(//div[@class='rl-item'])">
      <textarea>
        <label>
          <xsl:text>Your search did not return any results.</xsl:text>
        </label>
      </textarea>
    </xsl:when>

    <xsl:otherwise>
      <xsl:for-each select="//div[@class='rl-item']">
        <xsl:choose>
          <xsl:when test="starts-with(div/@srcurl, 'http://www.youtube.com/')">
	    <link>
	      <label><xsl:value-of select="normalize-space(div/div/div[@class='rl-title']/a)" /></label>
	      <stream>wvt:///youtube/video.xsl?srcurl=<xsl:value-of select="str:encode-uri(div/@srcurl, true())"/></stream>
	      <ref>wvt:///youtube/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(concat('http://gdata.youtube.com/feeds/api/videos/', substring-after(div/@srcurl, 'v='), '?v=2'), true())"/></ref>
	    </link>
          </xsl:when>

          <xsl:when test="starts-with(div/@srcurl, 'http://video.google.com/')">
	    <link>
	      <label><xsl:value-of select="normalize-space(div/div/div[@class='rl-title']/a)"/></label>
	      <stream>wvt:///google/video.xsl?srcurl=<xsl:value-of select="str:encode-uri(div/@srcurl, true())"/></stream>
	      <ref>wvt:///google/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(div/@srcurl, true())"/></ref>
	    </link>
          </xsl:when>

          <xsl:when test="starts-with(div/@srcurl, 'http://www.metacafe.com/')">
	    <link>
	      <label><xsl:value-of select="normalize-space(div/div/div[@class='rl-title']/a)"/></label>
	      <stream>wvt:///metacafe/video.xsl?srcurl=<xsl:value-of select="str:encode-uri(div/@srcurl)"/></stream>
	      <ref>wvt:///metacafe/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(div/@srcurl)"/></ref>
	    </link>
          </xsl:when>

          <xsl:when test="starts-with(div/@srcurl, 'http://vimeo.com/')">
	    <link>
	      <label><xsl:value-of select="normalize-space(div/div/div[@class='rl-title']/a)"/></label>
	      <stream>wvt:///vimeo/video.xsl?srcurl=http://www.vimeo.com/moogaloop/load/clip:<xsl:value-of select="substring-after(div/@srcurl, 'http://vimeo.com/')"/></stream>
	      <ref>wvt:///vimeo/description.xsl?srcurl=http://vimeo.com/api/v2/video/<xsl:value-of select="substring-after(div/@srcurl, 'http://vimeo.com/')"/>.xml</ref>
	    </link>
          </xsl:when>

          <xsl:when test="starts-with(div/@srcurl, 'http://svtplay.se/')">
	    <link>
	      <label><xsl:value-of select="normalize-space(div/div/div[@class='rl-title']/a)"/></label>
              <stream>wvt:///svtplay/video.xsl?srcurl=<xsl:value-of select="str:encode-uri(div/@srcurl, true())"/></stream>
              <ref>wvt:///svtplay/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(div/@srcurl, true())"/></ref>
	    </link>
          </xsl:when>

        </xsl:choose>
      </xsl:for-each>

      <xsl:if test="//td[@class='prev']/a">
        <link>
          <label>Previous</label>
          <ref>wvt:///google/searchresults.xsl?srcurl=<xsl:value-of select="str:encode-uri(//td[@class='prev']/a/@href, true())"/></ref>
        </link>
      </xsl:if>

      <xsl:if test="//td[@class='next']/a">
        <link>
          <label>Next</label>
          <ref>wvt:///google/searchresults.xsl?srcurl=<xsl:value-of select="str:encode-uri(//td[@class='next']/a/@href, true())"/></ref>
        </link>
      </xsl:if>
    </xsl:otherwise>
  </xsl:choose>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

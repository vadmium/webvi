<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<wvmenu>
  <title>Search results</title>

  <xsl:choose>
    <xsl:when test="not(//li[contains(@class, 'videobox')]//td/h3/a)">
      <textarea>
        <label>
          <xsl:text>Your search did not return any results.</xsl:text>
        </label>
      </textarea>
    </xsl:when>

    <xsl:otherwise>
      <xsl:for-each select="//li[contains(@class, 'videobox')]//td/h3/a">
        <xsl:choose>
          <xsl:when test="starts-with(@href, 'http://www.youtube.com/')">
	    <link>
	      <label><xsl:value-of select="normalize-space(.)" /></label>
	      <stream>wvt:///www.youtube.com/videopage.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></stream>
	      <ref>wvt:///www.youtube.com/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(concat('http://gdata.youtube.com/feeds/api/videos/', substring-after(@href, 'v='), '?v=2'), true())"/></ref>
	    </link>
          </xsl:when>

          <xsl:when test="starts-with(@href, 'http://video.google.com/')">
	    <link>
	      <label><xsl:value-of select="normalize-space(.)"/></label>
	      <stream>wvt:///video.google.com/videopage.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></stream>
	      <ref>wvt:///video.google.com/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></ref>
	    </link>
          </xsl:when>

          <xsl:when test="starts-with(@href, 'http://www.metacafe.com/')">
	    <link>
	      <label><xsl:value-of select="normalize-space(.)"/></label>
	      <stream>wvt:///www.metacafe.com/videopage.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></stream>
	      <ref>wvt:///www.metacafe.com/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></ref>
	    </link>
          </xsl:when>

          <xsl:when test="starts-with(@href, 'http://vimeo.com/')">
	    <link>
	      <label><xsl:value-of select="normalize-space(.)"/></label>
	      <stream>wvt:///www.vimeo.com/video.xsl?srcurl=http://www.vimeo.com/moogaloop/load/clip:<xsl:value-of select="substring-after(@href, 'http://vimeo.com/')"/></stream>
	      <ref>wvt:///www.vimeo.com/description.xsl?srcurl=http://vimeo.com/api/v2/video/<xsl:value-of select="substring-after(@href, 'http://vimeo.com/')"/>.xml</ref>
	    </link>
          </xsl:when>

          <xsl:when test="starts-with(@href, 'http://svtplay.se/')">
	    <link>
	      <label><xsl:value-of select="normalize-space(.)"/></label>
              <stream>wvt:///svtplay.se/videopage.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></stream>
              <ref>wvt:///svtplay.se/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></ref>
	    </link>
          </xsl:when>

        </xsl:choose>
      </xsl:for-each>

      <xsl:for-each select="id('nav')//td[@class='b']/a">
        <link>
          <label><xsl:value-of select="span[2]/text()"/></label>
          <ref>wvt:///video.google.com/searchresults.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></ref>
        </link>
      </xsl:for-each>
    </xsl:otherwise>
  </xsl:choose>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

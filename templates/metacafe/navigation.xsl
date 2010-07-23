<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="item">
  <link>
    <label><xsl:value-of select="title" /></label>
    <xsl:choose>
      <xsl:when test="starts-with(id, 'yt-')">
        <stream>wvt:///youtube/video.xsl?srcurl=http%3A//www.youtube.com/watch%3Fv=<xsl:value-of select="substring(id, 4)"/></stream>
      </xsl:when>
      <xsl:otherwise>
        <stream>wvt:///metacafe/video.xsl?srcurl=<xsl:value-of select="link"/></stream>
      </xsl:otherwise>
    </xsl:choose>

    <ref>wvt:///metacafe/description.xsl?srcurl=/api/item/<xsl:value-of select="id"/></ref>
  </link>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="/rss/channel/title"/></title>

  <xsl:apply-templates select="/rss/channel/item"/>

  <xsl:if test="count(/rss/channel/item) = 0">
    <textarea>
      <label>No matching results.</label>
    </textarea>
  </xsl:if>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

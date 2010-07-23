<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:strip-space elements="div" />

<!-- old variables (before appr. April 2010) -->
<xsl:variable name="t1" select="substring-before(substring-after(html/head/script[contains(., 'swfArgs') or contains(., 'SWF_ARGS')], '&quot;t&quot;: &quot;'), '&quot;')"/>
<xsl:variable name="video_id1" select="substring-before(substring-after(html/head/script[contains(., 'swfArgs') or contains(., 'SWF_ARGS')], '&quot;video_id&quot;: &quot;'), '&quot;')"/>

<!-- new variables -->
<xsl:variable name="t2" select="substring-before(substring-after(//script[contains(., 'swfHTML')], '&amp;t='), '&amp;')"/>
<xsl:variable name="video_id2" select="substring-before(substring-after(//script[contains(., 'swfHTML')], '&amp;video_id='), '&amp;')"/>

<xsl:variable name="t">
  <xsl:choose>
    <xsl:when test="$t1"><xsl:value-of select="$t1"/></xsl:when>
    <xsl:otherwise><xsl:value-of select="$t2"/></xsl:otherwise>
  </xsl:choose>
</xsl:variable>
<xsl:variable name="video_id">
  <xsl:choose>
    <xsl:when test="$video_id1"><xsl:value-of select="$video_id1"/></xsl:when>
    <xsl:otherwise><xsl:value-of select="$video_id2"/></xsl:otherwise>
  </xsl:choose>
</xsl:variable>

<xsl:template match="/">
<mediaurl>
  <title>
    <xsl:choose>
      <xsl:when test="/html/head/meta[@name='title']/@content">
	<xsl:value-of select="/html/head/meta[@name='title']/@content"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="//div[@id='watch-vid-title']//h1"/>
      </xsl:otherwise>
    </xsl:choose>
  </title>

  <url priority="70">http://www.youtube.com/get_video?video_id=<xsl:value-of select="$video_id"/>&amp;t=<xsl:value-of select="$t"/>&amp;fmt=22</url>
  <url priority="60">http://www.youtube.com/get_video?video_id=<xsl:value-of select="$video_id"/>&amp;t=<xsl:value-of select="$t"/>&amp;fmt=18</url>
  <url priority="50">http://www.youtube.com/get_video?video_id=<xsl:value-of select="$video_id"/>&amp;t=<xsl:value-of select="$t"/></url>
</mediaurl>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="title">katsomo_video</xsl:param>
<xsl:param name="pid"/>

<xsl:template name="mediaurl">
<xsl:param name="title"/>
<xsl:param name="pid"/>
<mediaurl>
  <title><xsl:value-of select="$title"/></title>
  <url>
    <xsl:if test="$pid">
      <xsl:value-of select="concat('http://www.katsomo.fi/metafile.asx?p=', $pid, '&amp;bw=800')"/>
    </xsl:if>
  </url>
</mediaurl>
</xsl:template>

<xsl:template match="/">
  <xsl:call-template name="mediaurl">
    <xsl:with-param name="title" select="$title"/>
    <xsl:with-param name="pid" select="$pid"/>
  </xsl:call-template>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:import href="../vimeo.com/video.xsl"/>

<xsl:param name="docurl"/>

<xsl:template match="/">
  <xsl:variable name="videoid">
    <xsl:value-of select="substring-after($docurl, 'http://www.vimeo.com/')"/>
  </xsl:variable>

  <xsl:if test="$videoid">
    <xsl:apply-templates select="document(concat('http://www.vimeo.com/moogaloop/load/clip:', $videoid))" mode="included"/>
  </xsl:if>
</xsl:template>

</xsl:stylesheet>

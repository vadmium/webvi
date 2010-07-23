<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="video">
  <link>
    <label><xsl:value-of select="title"/></label>
    <stream>wvt:///vimeo/video.xsl?srcurl=http://www.vimeo.com/moogaloop/load/clip:<xsl:value-of select="id"/></stream>
    <ref>wvt:///vimeo/description.xsl?srcurl=http://vimeo.com/api/v2/video/<xsl:value-of select="id"/>.xml</ref>
  </link>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title>Vimeo videos</title>

  <xsl:apply-templates select="/videos/video"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

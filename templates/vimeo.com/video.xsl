<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template name="mediaurl">
<mediaurl>
  <title><xsl:value-of select="/xml/video/caption"/></title>

  <url priority="50">http://www.vimeo.com/moogaloop/play/clip:<xsl:value-of select="/xml/video/nodeId"/>/<xsl:value-of select="/xml/request_signature"/>/<xsl:value-of select="/xml/request_signature_expires"/>/?q=sd</url>
</mediaurl>
</xsl:template>

<xsl:template match="/">
  <xsl:call-template name="mediaurl"/>
</xsl:template>

<xsl:template match="/" mode="included">
  <xsl:call-template name="mediaurl"/>
</xsl:template>

</xsl:stylesheet>

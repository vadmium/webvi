<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:import href="video.xsl"/>

<xsl:param name="docurl"/>

<xsl:template match="/">
  <xsl:call-template name="mediaurl">
    <xsl:with-param name="title" select="/html/head/title"/>
    <xsl:with-param name="pid" select="substring-after($docurl, '?')"/>
  </xsl:call-template>
</xsl:template>

</xsl:stylesheet>

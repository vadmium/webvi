<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:import href="video.xsl"/>

<xsl:template match="/">
  <xsl:variable name="videoinfo">
    <xsl:value-of select="substring-before(substring-after(//script[contains(., 'flashvars=\&quot;')], 'flashvars=\&quot;'), '\&quot;')"/>
  </xsl:variable>

  <xsl:call-template name="mediaurl">
    <xsl:with-param name="videoinfo" select="$videoinfo"/>
    <xsl:with-param name="title" select="/html/head/meta/@content"/>
  </xsl:call-template>
</xsl:template>

</xsl:stylesheet>

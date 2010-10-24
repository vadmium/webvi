<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:import href="video.xsl"/>

<xsl:param name="docurl"/>

<xsl:template match="/" mode="video_config">
  <xsl:call-template name="mediaurl"/>
</xsl:template>

<xsl:template match="/">
  <xsl:param name="mediatype" select="substring-before(substring-after($docurl, 'vt='), '&amp;vid=')"/>
  <xsl:param name="videoid" select="substring-after($docurl, '&amp;vid=')"/>
  <xsl:variable name="videourl">http://www.nelonen.fi/utils/video_config/?q=<xsl:value-of select="$mediatype"/>/<xsl:value-of select="$videoid"/>&amp;site=www.ruutu.fi&amp;ageCheckURL=http://sso.nelonenmedia.fi/ajax/check_age/&amp;current_page=http://www.ruutu.fi/video</xsl:variable>
  <xsl:apply-templates select="document($videourl)" mode="video_config"/>
</xsl:template>

</xsl:stylesheet>

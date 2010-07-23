<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:media="http://search.yahoo.com/mrss/">

<!-- Convert $seconds to hours:min:sec format -->
<xsl:template name="pretty-print-seconds">
  <xsl:param name="seconds"/>

  <xsl:variable name="sec" select="$seconds mod 60"/>
  <xsl:variable name="min" select="floor($seconds div 60) mod 60"/>
  <xsl:variable name="hour" select="floor($seconds div 3600)"/>

  <xsl:value-of select="concat($hour, ':', format-number($min, '00'), ':', format-number($sec, '00'))"/>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="/rss/channel/item/title"/></title>

  <textarea>
    <label><xsl:value-of select="/rss/channel/item/media:description"/></label>
  </textarea>

  <textarea>
    <label>Duration: <xsl:call-template name="pretty-print-seconds">
        <xsl:with-param name="seconds">
          <xsl:value-of select="/rss/channel/item/media:content/@duration"/>
        </xsl:with-param>
      </xsl:call-template>
    </label>
  </textarea>

  <textarea>
    <label>Rating: <xsl:value-of select="/rss/channel/item/rank"/></label>
  </textarea>

  <textarea>
    <label>published: <xsl:value-of select="/rss/channel/item/pubDate"/></label>
  </textarea>


</wvmenu>
</xsl:template>

</xsl:stylesheet>

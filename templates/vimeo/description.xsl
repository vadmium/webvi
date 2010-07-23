<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

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
  <title><xsl:value-of select="/videos/video/title"/></title>
  <textarea>
    <label><xsl:value-of select="/videos/video/description"/></label>
  </textarea>

  <textarea>
    <label>Duration: <xsl:call-template name="pretty-print-seconds">
        <xsl:with-param name="seconds">
          <xsl:value-of select="/videos/video/duration"/>
        </xsl:with-param>
      </xsl:call-template>
    </label>
  </textarea>

  <textarea>
    <label>Views: <xsl:value-of select="/videos/video/stats_number_of_plays"/></label>
  </textarea>

  <textarea>
    <label>Likes: <xsl:value-of select="/videos/video/stats_number_of_likes"/></label>
  </textarea>

  <textarea>
    <label>Published: <xsl:value-of select="/videos/video/upload_date"/></label>
  </textarea>

  <link>
    <label>More videos by <xsl:value-of select="/videos/video/user_name"/></label>
    <ref>wvt:///vimeo/navigation.xsl?srcurl=http://vimeo.com/api/v2/<xsl:value-of select="str:split(/videos/video/user_url, '/')[last()]"/>/videos.xml</ref>
  </link>

  <link>
    <label>Download this video</label>
    <stream>wvt:///vimeo/video.xsl?srcurl=http://www.vimeo.com/moogaloop/load/clip:<xsl:value-of select="/videos/video/id"/></stream>
  </link>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:atom="http://www.w3.org/2005/Atom"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  xmlns:media="http://search.yahoo.com/mrss/"
  xmlns:yt="http://gdata.youtube.com/schemas/2007"
  exclude-result-prefixes="atom str media yt">

<xsl:template match="atom:entry">
  <link>
    <label><xsl:value-of select="atom:title"/></label>
    <stream>wvt:///www.youtube.com/videopage.xsl?srcurl=http://www.youtube.com/watch?v=<xsl:value-of select="media:group/yt:videoid"/></stream>
    <ref>wvt:///www.youtube.com/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(atom:link[@rel='self']/@href, true())"/></ref>
  </link>
</xsl:template>

<xsl:template match="atom:link">
  <xsl:if test="@rel = 'previous'">
    <link>
      <label>Previous</label>
      <ref>wvt:///www.youtube.com/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></ref>
    </link>
  </xsl:if>

  <xsl:if test="@rel = 'next'">
    <link>
      <label>Next</label>
      <ref>wvt:///www.youtube.com/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></ref>a
    </link>
  </xsl:if>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="/atom:feed/atom:title"/></title>

  <xsl:if test="/atom:feed/atom:link[@rel='http://schemas.google.com/g/2006#spellcorrection']">
    <link>
      <label>Did you mean <xsl:value-of select="/atom:feed/atom:link[@rel='http://schemas.google.com/g/2006#spellcorrection']/@title"/>?</label>
      <ref>wvt:///www.youtube.com/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(/atom:feed/atom:link[@rel='http://schemas.google.com/g/2006#spellcorrection']/@href, true())"/></ref>
    </link>
  </xsl:if>


  <!-- Video links -->
  <xsl:apply-templates select="/atom:feed/atom:entry"/>

  <xsl:if test="count(/atom:feed/atom:entry) = 0">
    <textarea>
      <label>No match</label>
    </textarea>
  </xsl:if>

  <!-- Next and prev links -->
  <xsl:apply-templates select="/atom:feed/atom:link[@rel='previous']|/atom:feed/atom:link[@rel='next']"/>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

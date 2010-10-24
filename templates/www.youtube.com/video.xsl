<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  xmlns:map="mapping"
  exclude-result-prefixes="str map">

<!-- Mapping from Youtube fmt parameter (key) to webvideo priority
     score (value) -->
<map:associativearray>
  <map:item key="17" value="40"/>
  <map:item key="5" value="50"/>
  <map:item key="34" value="52"/>
  <map:item key="35" value="54"/>
  <map:item key="18" value="60"/>
  <map:item key="43" value="65"/>
  <map:item key="22" value="70"/>
  <map:item key="45" value="75"/>
  <map:item key="37" value="80"/>
</map:associativearray>

<xsl:template name="fmturl">
  <xsl:variable name="fmt">
    <xsl:value-of select="str:split(., '|')[1]"/>
  </xsl:variable>
  <xsl:variable name="url">
    <xsl:value-of select="str:split(., '|')[2]"/>
  </xsl:variable>

  <xsl:if test="$url">
    <url>
      <xsl:attribute name="priority">
	<xsl:choose>
	  <xsl:when test="document('')/*/map:associativearray/map:item[@key=$fmt]">
	    <xsl:value-of select="document('')/*/map:associativearray/map:item[@key=$fmt]/@value"/>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:value-of select="50"/>
	  </xsl:otherwise>
	</xsl:choose>
      </xsl:attribute>
      <xsl:value-of select="$url"/>
    </url>
  </xsl:if>
</xsl:template>

<xsl:template name="mediaurl">
<xsl:param name="videoinfo"/>
<xsl:param name="title"/>

<mediaurl>
  <title>
    <xsl:variable name="titleparam">
      <xsl:choose>
	<xsl:when test="$title">
	  <xsl:value-of select="$title"/>
	</xsl:when>
      	<xsl:when test="contains(substring-after($videoinfo, '&amp;title='), '&amp;')">
      	  <xsl:value-of select="substring-before(substring-after($videoinfo, '&amp;title='), '&amp;')"/>
      	</xsl:when>
      	<xsl:otherwise>
      	  <xsl:value-of select="substring-after($videoinfo, '&amp;title=')"/>
      	</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    
    <xsl:value-of select="str:decode-uri(str:replace($titleparam, '+', ' '))"/>
  </title>

  <xsl:for-each select="str:split(str:decode-uri(substring-before(substring-after($videoinfo, '&amp;fmt_url_map='), '&amp;')), ',')">
    <xsl:call-template name="fmturl"/>
  </xsl:for-each>

</mediaurl>
</xsl:template>

<xsl:template match="/">
  <xsl:call-template name="mediaurl">
    <xsl:with-param name="videoinfo" select="."/>
  </xsl:call-template>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="wvmenu"/>

<xsl:template match="/">
<wvmenu>
  <xsl:for-each select="str:split($wvmenu)">
    <xsl:variable name="parts" select="str:split(., '=')"/>
    <xsl:variable name="element" select="$parts[1]"/>
    <xsl:choose>
      <xsl:when test="$element = 'title' ">
      <title><xsl:value-of select="str:decode-uri($parts[2])"/></title>
      </xsl:when>
      
      <xsl:when test="$element = 'link' ">
      <link>
        <label><xsl:value-of select="str:decode-uri($parts[2])"/></label>
        <ref><xsl:value-of select="str:decode-uri($parts[3])"/></ref>
      </link>
      </xsl:when>
    </xsl:choose>
  </xsl:for-each>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

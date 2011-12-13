<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template mode="wvmenu-ref" match="/wvmenu">
  <xsl:text>wvt:///wvmenu.xsl?param=wvmenu,</xsl:text>
  <xsl:for-each select="*">
    <xsl:apply-templates mode="wvmenu-ref" select="."/>
    <xsl:if test="position() != last()">
      <xsl:value-of select="str:encode-uri(' ', true())"/>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

<xsl:template mode="wvmenu-ref" match="title">
  <xsl:value-of select="str:encode-uri(concat(
    'title=', str:encode-uri(., true())
  ), true())"/>
</xsl:template>

<xsl:template mode="wvmenu-ref" match="link">
  <xsl:value-of select="str:encode-uri(concat(
    'link=', str:encode-uri(label, true()), '=', str:encode-uri(ref, true())
  ), true())"/>
</xsl:template>

</xsl:stylesheet>

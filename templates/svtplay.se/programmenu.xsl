<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="text()" />

<xsl:template match="div[@id='pb']">
  <xsl:apply-templates/>
</xsl:template>

<!-- Broadcasts -->
<xsl:template match="div[@class='content']//ul/li/a">
  <link>
    <label><xsl:value-of select="normalize-space(span)"/></label>
    <ref>wvt:///svtplay.se/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></ref>
    <stream>wvt:///svtplay.se/videopage.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></stream>
  </link>
</xsl:template>

<!-- next/prev links -->
<xsl:template match="div[@class='footer']/div[@class='pagination']/ul[@class='pagination program']/li">
    <xsl:if test="@class='prev '">
      <link>
        <label><xsl:value-of select="a/img/@alt"/></label>
        <ref>wvt:///svtplay.se/programmenu.xsl?srcurl=<xsl:value-of select="str:encode-uri(a/@href, true())"/></ref>
      </link>
    </xsl:if>

    <xsl:if test="@class='next '">
      <link>
        <label><xsl:value-of select="a/img/@alt"/></label>
        <ref>wvt:///svtplay.se/programmenu.xsl?srcurl=<xsl:value-of select="str:encode-uri(a/@href, true())"/></ref>
      </link>
    </xsl:if>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title>
    <xsl:choose>
      <xsl:when test="normalize-space(//h1/a)">
        <xsl:value-of select="normalize-space(//h1/a)"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="normalize-space(//h1/a/img/@alt)"/>
      </xsl:otherwise>
    </xsl:choose>
  </title>

  <xsl:apply-templates select="//div[@id='sb']"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

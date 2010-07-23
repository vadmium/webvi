<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="text()" />

<xsl:template match="div[@id='pb']">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="div[@id='sb']">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="div[@id='se']">
  <xsl:apply-templates/>
</xsl:template>

<!-- Programs -->
<xsl:template match="div[@class='content']//ul/li/a[1]">
  <link>
    <label><xsl:value-of select="normalize-space(span)"/></label>
    <ref>wvt:///svtplay/programmenu.xsl?srcurl=<xsl:value-of select="str:encode-uri(@href, true())"/></ref>
  </link>
</xsl:template>

<!-- next/prev links -->
<xsl:template match="div[@class='footer']/div[@class='pagination']/ul[@class='pagination program']/li">
    <xsl:if test="@class='prev '">
      <link>
        <label><xsl:value-of select="a/img/@alt"/></label>
        <ref>wvt:///svtplay/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(a/@href, true())"/></ref>
      </link>
    </xsl:if>

    <xsl:if test="@class='next '">
      <link>
        <label><xsl:value-of select="a/img/@alt"/></label>
        <ref>wvt:///svtplay/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(a/@href, true())"/></ref>
      </link>
    </xsl:if>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title>
    <xsl:choose>
      <xsl:when test="normalize-space(//h1)">
        <xsl:value-of select="normalize-space(//h1)"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="normalize-space(//h1/a/img/@alt)"/>
      </xsl:otherwise>
    </xsl:choose>
  </title>

  <!-- In most categories the content is in pb and se nodes, except
       for Nyheter and Sport, where the content is in sb and se nodes.
       On the other hand, we can't match sb unconditionally because in
       Öppet arkiv sb contains klips instead of programs! -->
  <xsl:choose>
    <xsl:when test="//div[@id='pb']">
      <xsl:apply-templates select="//div[@id='pb']|//div[@id='se']"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select="//div[@id='sb']|//div[@id='se']"/>
    </xsl:otherwise>
  </xsl:choose>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

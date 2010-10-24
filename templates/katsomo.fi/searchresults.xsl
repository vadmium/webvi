<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template match="a">
  <xsl:variable name="progId" select="substring-after(@href, 'progId=')"/>
  <xsl:variable name="title" select="normalize-space(.)"/>

  <link>
    <label><xsl:value-of select="$title"/></label>
    <stream>wvt:///katsomo.fi/video.xsl?srcurl=<xsl:value-of select="str:encode-uri(concat('http://katsomo.fi/showContent.do?progId=', $progId, '&amp;adData=%7B%22ad%22%3A%20%7B%7D%7D&amp;ajax=true&amp;serial=1'), true())"/>&amp;param=title,<xsl:value-of select="str:encode-uri($title, true())"/>&amp;HTTP-header=cookie,webtv.bandwidth%3D1000%3BautoFullScreen%3Dfalse%3Bwebtv.playerPlatform%3D0</stream>
  </link>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title>Hakutulokset: <xsl:value-of select="id('searchResults')/div/div[@class='description']/span"/></title>

  <xsl:if test="not(id('resultList')/div[@class='item'])">
    <textarea>
      <label><xsl:value-of select="normalize-space(id('siteMapList')/p)"/></label>
    </textarea>
  </xsl:if>

  <xsl:apply-templates select="id('resultList')/div[@class='item']/h6/a[not(@class='programType')]"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

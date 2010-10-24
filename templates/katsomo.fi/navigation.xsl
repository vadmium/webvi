<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="docurl"/>

<xsl:template match="ol[@class='categoryList']/li">
  <link>
    <label><xsl:value-of select="normalize-space(a)"/></label>
    <ref>wvt:///katsomo.fi/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri(a/@href, true())"/></ref>
  </link>
</xsl:template>

<xsl:template match="ol[@class='programList']/li">
  <xsl:variable name="progId" select="substring-after(a/@href, 'progId=')"/>
  <xsl:variable name="treeId" select="substring-after($docurl, 'treeId=')"/>
  <xsl:variable name="title" select="normalize-space(a[string(.)])"/>

  <link>
    <label><xsl:value-of select="$title"/></label>
    <stream>wvt:///katsomo.fi/video.xsl?srcurl=<xsl:value-of select="str:encode-uri(concat('http://katsomo.fi/showContent.do?treeId=', $treeId, '&amp;progId=', $progId, '&amp;adData=%7B%22ad%22%3A%20%7B%7D%7D&amp;ajax=true&amp;serial=1'), true())"/>&amp;param=title,<xsl:value-of select="str:encode-uri($title, true())"/>&amp;HTTP-header=cookie,webtv.bandwidth%3D1000%3BautoFullScreen%3Dfalse%3Bwebtv.playerPlatform%3D0</stream>
  </link>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="/html/head/meta[@name='title']/@content"/></title>

  <xsl:if test="//ol[@class='categoryList']/li and //ol[@class='programList']/li">
    <textarea>
      <label>Ohjelmat</label>
    </textarea>
  </xsl:if>
  <xsl:apply-templates select="//ol[@class='categoryList']/li"/>

  <xsl:if test="//ol[@class='categoryList']/li and //ol[@class='programList']/li">
    <textarea>
      <label>Jaksot</label>
    </textarea>
  </xsl:if>
  <xsl:apply-templates select="//ol[@class='programList']/li"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

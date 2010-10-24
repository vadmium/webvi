<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="docurl"/>
<xsl:variable name="programname" select="id('page')/div[@class='ohjelma_yla ohjelmanavi']/h1"/>

<xsl:template match="li">
  <xsl:variable name="progId" select="substring-after(div[@class='outerwrap']//a/@href, '?')"/>
  <xsl:variable name="title" select="concat($programname, ' - ', normalize-space(.//h5))"/>

  <xsl:if test="$progId">
    <link>
      <label><xsl:value-of select="normalize-space(.//h5)"/></label>
      <stream>wvt:///www.sub.fi/video.xsl?srcurl=<xsl:value-of select="str:encode-uri($docurl, true())"/>&amp;param=pid,<xsl:value-of select="$progId"/>&amp;param=title,<xsl:value-of select="str:encode-uri($title, true())"/></stream>
      <ref>wvt:///www.sub.fi/description.xsl?param=title,<xsl:value-of select="str:encode-uri($title, true())"/>&amp;param=desc,<xsl:value-of select="str:encode-uri(.//span[@class='verho_content']/div, true())"/>&amp;param=pubdate,<xsl:value-of select="str:encode-uri(p[@class='julkaistu'], true())"/>&amp;param=pid,<xsl:value-of select="$progId"/></ref>
    </link>
  </xsl:if>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="$programname"/></title>

  <xsl:choose>
    <xsl:when test="id('uusimmat')/li">
      <xsl:apply-templates select="id('uusimmat')/li"/>
    </xsl:when>
    <xsl:otherwise>
      <textarea>
	<label>Ei jaksoja</label>
      </textarea>
    </xsl:otherwise>
  </xsl:choose>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

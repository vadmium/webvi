<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:str="http://exslt.org/strings">

<xsl:template match="item">
  <link>
    <label><xsl:value-of select="title" /> (<xsl:value-of select="videos"/> videos, avg. rank: <xsl:value-of select="avg_rank"/>)</label>
    <ref>wvt:///www.metacafe.com/navigation.xsl?srcurl=/api/users/<xsl:value-of select="str:encode-uri(translate(title, ' ', '+'), true())"/>/channel?time=all_time</ref>
  </link>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="/rss/channel/title"/></title>

  <xsl:apply-templates select="/rss/channel/item"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

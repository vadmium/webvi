<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<mediaurl>
  <title><xsl:value-of select="/html/head/title" /></title>
  <xsl:for-each select="/html/body/script">
    <xsl:variable name="videourl" select="str:decode-uri(substring-before(substring-after(., 'videoUrl\x3d'), '\x26'))"/>
    <xsl:if test="$videourl">
      <url><xsl:value-of select="$videourl"/></url>
    </xsl:if>
  </xsl:for-each>
</mediaurl>
</xsl:template>

</xsl:stylesheet>

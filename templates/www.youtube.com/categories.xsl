<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  xmlns:app="http://www.w3.org/2007/app"
  xmlns:atom="http://www.w3.org/2005/Atom"
  xmlns:yt="http://gdata.youtube.com/schemas/2007"
  exclude-result-prefixes="str app atom yt">

<xsl:template match="/">
<wvmenu>
  <title>Youtube</title>

  <link>
    <label>Search</label>
    <ref>wvt:///www.youtube.com/search.xsl</ref>
  </link>

  <xsl:for-each select="/app:categories/atom:category[yt:browsable]">
    <link>
      <label><xsl:value-of select="@label"/></label>
      <ref>wvt:///www.youtube.com/navigation.xsl?srcurl=http://gdata.youtube.com/feeds/api/standardfeeds/most_popular_<xsl:value-of select="str:encode-uri(@term, true())"/>%3Fmax-results%3D20%26v%3D2</ref>
    </link>
  </xsl:for-each>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

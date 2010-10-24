<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<wvmenu>
  <title>YLE Areena</title>

  <link>
    <label>Haku</label>
    <ref>wvt:///areena.yle.fi/search.xsl?srcurl=http://areena.yle.fi/haku</ref>
  </link>

  <link>
    <label>Suorat lähetykset</label>
    <ref>wvt:///areena.yle.fi/livebroadcasts.xsl?srcurl=http://areena.yle.fi/live</ref>
  </link>

  <link>
    <label>Kaikki ohjelmat</label>
    <ref>wvt:///areena.yle.fi/programlist.xsl?srcurl=http://areena.yle.fi/ohjelmat</ref>
  </link>

  <xsl:for-each select="//div[h4='Sisältö aihealueittain']/ul/li/a">
    <link>
      <label><xsl:value-of select="."/></label>
      <ref><xsl:value-of select="concat('wvt:///areena.yle.fi/navigation.xsl?srcurl=', str:encode-uri(concat(./@href, '/feed/rss'), true()))"/></ref>
    </link>
  </xsl:for-each>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

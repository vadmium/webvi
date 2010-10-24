<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<wvmenu>
  <title>Katsomo</title>

  <link>
    <label>MoonTV ohjelmat</label>
    <ref>wvt:///moontv.fi/programlist.xsl?srcurl=http://moontv.fi/ohjelmat</ref>
  </link>
  <link>
    <label>MoonTV uusimmat videot</label>
    <ref>wvt:///moontv.fi/rss.xsl?srcurl=http://feeds.feedburner.com/Moontv?format=rss</ref>
  </link>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

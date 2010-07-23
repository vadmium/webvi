<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<mediaurl>
  <title><xsl:value-of select="concat(id('ruutuVideoInfo')/p[@class='name'], ' ', id('ruutuVideoInfo')/p[@class='timeStamp'])"/></title>

  <url priority="50">wvt:///bin/ruutu-dl?contenttype=video/x-flv&amp;arg=<xsl:value-of select='substring-before(substring-after(//script[contains(., "vplayer1")], "providerURL&apos;, &apos;"), "&apos;")'/>&amp;arg=<xsl:value-of select="str:encode-uri($docurl, true())"/></url>
</mediaurl>
</xsl:template>

</xsl:stylesheet>

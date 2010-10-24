<?xml version="1.0" encoding="UTf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template match="/">
<wvmenu>
  <title>Haku</title>

  <textfield name="query">
    <label>Hakusana</label>
  </textfield>

  <button>
    <label>Hae</label>
    <submission>wvt:///katsomo.fi/searchresults.xsl?srcurl=<xsl:value-of select="str:encode-uri('http://katsomo.fi/search.do?keywords={query}&amp;treeId=9992', true())"/></submission>
  </button>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

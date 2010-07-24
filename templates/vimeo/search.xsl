<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<wvmenu>
  <title>Vimeo Search</title>

  <textfield name="keywords">
    <label>Search terms</label>
  </textfield>

  <itemlist name="orderby">
    <label>Show me</label>
    <item value="">most relevant</item>
    <item value="/sort:newest">newest</item>
    <item value="/sort:plays">most played</item>
    <item value="/sort:likes">most liked</item>
  </itemlist>

  <button>
    <label>Search</label>
    <submission>wvt:///vimeo/searchresults.xsl?srcurl=<xsl:value-of select="concat(str:encode-uri('http://vimeo.com/videos/search:', true()), '{keywords}/', substring(id('xsrft')/@value, 0, 9), '{orderby}')"/>&amp;HTTP-header=cookie,xsrft%3D<xsl:value-of select="substring(id('xsrft')/@value, 0, 9)"/>;searchtoken%3D<xsl:value-of select="substring(id('xsrft')/@value, 0, 9)"/>&amp;param=searchtoken,<xsl:value-of select="substring(id('xsrft')/@value, 0, 9)"/></submission>
  </button>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

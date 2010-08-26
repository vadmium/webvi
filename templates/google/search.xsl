<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="/">
<wvmenu>
  <title>Google video search</title>

  <textfield name="q">
    <label>Search terms</label>
  </textfield>

  <itemlist name="so">
    <label>Sort by</label>
    <item value="">Relevance</item>
    <item value=",sbd:1">Date</item>
  </itemlist>

  <itemlist name="dur">
    <label>Duration</label>
    <item value="">All durations</item>
    <item value=",dur:s">Short (&lt; 4 min)</item>
    <item value=",dur:m">Medium (4-20 min)</item>
    <item value=",dur:l">Long (&gt; 20 min)</item>
  </itemlist>

  <button>
    <label>Search</label>
    <submission>wvt:///google/searchresults.xsl?srcurl=<xsl:value-of select="str:encode-uri('http://www.google.com/search?q={q}&amp;tbs=vid:1{dur}{so}', true())"/>&amp;HTTP-header=User-Agent,Mozilla/5.0</submission>
  </button>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

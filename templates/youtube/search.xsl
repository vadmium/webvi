<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template match="/">
<wvmenu>
  <title>Youtube Search</title>

  <textfield name="q">
    <label>Search terms</label>
  </textfield>

  <itemlist name="orderby">
    <label>Sort by</label>
    <item value="relevance">Relevance</item>
    <item value="published">Date Added</item>
    <item value="viewCount">View Count</item>
    <item value="rating">Rating</item>
  </itemlist>

  <itemlist name="time">
    <label>Uploaded</label>
    <item value="all_time">Anytime</item>
    <item value="today">Today</item>
    <item value="this_week">This week</item>
    <item value="this_month">This month</item>
  </itemlist>

  <button>
    <label>Search</label>
    <submission>wvt:///youtube/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri('http://gdata.youtube.com/feeds/api/videos?q={q}&amp;orderby={orderby}&amp;time={time}&amp;max-results=20&amp;safeSearch=none&amp;format=5&amp;v=2', true())"/></submission>
  </button>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<wvmenu>
  <title>Metacafe Search</title>

  <textfield name="vq">
    <label>Keywords</label>
  </textfield>

  <itemlist name="orderby">
    <label>Sort by</label>
    <item value="updated">Most recent</item>
    <item value="viewCount">View Count</item>
    <item value="discussed">Most discussed</item>
  </itemlist>

  <itemlist name="time">
    <label>Published</label>
    <item value="all_time">Anytime</item>
    <item value="today">During last 24 hours</item>
    <item value="this_week">This week</item>
    <item value="this_month">This month</item>
  </itemlist>

  <button>
    <label>Search</label>
    <submission>wvt:///www.metacafe.com/navigation.xsl?srcurl=http%3A//www.metacafe.com/api/videos%3Fvq=%7Bvq%7D%26orderby=%7Borderby%7D%26time=%7Btime%7D</submission>
  </button>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

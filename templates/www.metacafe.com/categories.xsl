<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<wvmenu>
  <title>Metacafe</title>

  <link>
    <label>Search</label>
    <ref>wvt:///www.metacafe.com/search.xsl</ref>
  </link>

  <link>
    <label>Most viewed channels</label>
    <ref>wvt:///www.metacafe.com/channellist.xsl?srcurl=/api/channels/</ref>
  </link>

  <xsl:for-each select="id('LeftCol')/ul/li/a">
    <!-- '18+ Only' is empty unless family filter is off. Ignore the
         category until I find a way to turn off the filter. -->
    <xsl:if test="@title != '18+ Only'">
      <link>
	<label><xsl:value-of select="@title"/></label>
	<ref>wvt:///www.metacafe.com/navigation.xsl?srcurl=/api/videos/-/<xsl:value-of select="substring-after(@href, '/videos/')"/></ref>
      </link>
    </xsl:if>
  </xsl:for-each>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

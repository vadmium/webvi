<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="stream"></xsl:param>

<xsl:template match="/">
  <mediaurl>
    <title>livestream-<xsl:value-of select="$stream"/></title>
    <xsl:choose>
      <xsl:when test="$stream">
        <url>wvt:///bin/yle-dl?contenttype=video/x-flv&amp;arg=http%3A//areena.yle.fi/player/index.php%3Fstream%3D<xsl:value-of select="$stream"/>%26language%3Dfi</url>
      </xsl:when>
      <xsl:otherwise>
        <url/>
      </xsl:otherwise>
    </xsl:choose>
</mediaurl>

</xsl:template>

</xsl:stylesheet>

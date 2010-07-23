<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:output method="xml" version="1.0" encoding="UTF-8" />

<xsl:template match="dict">
  <xsl:if test="is_video=1">
    <link>
      <label><xsl:value-of select="name"/></label>
      <ref>wvt:///ruutufi/program.xsl?srcurl=http://www.ruutu.fi/ajax/media_get_nettitv_video/all/video_episode/<xsl:value-of select="str:encode-uri(str:encode-uri(url_encode_name, true()), true())"/>/latestdesc/0/25/true/__&amp;postprocess=json2xml</ref>
      <!-- Yes, ruutu.fi really expects url_encode_name to be double-url-encoded! -->
    </link>
  </xsl:if>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title>Kaikki sarjat</title>

  <xsl:apply-templates select="/jsondocument/list/li/dict"/>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

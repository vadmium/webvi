<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<wvmenu>
  <title>Ruutu.fi</title>

<!--
  <link>
    <label>Haku</label>
    <ref>wvt:///ruutufi/search.xsl</ref>
  </link>
-->

  <link>
    <label>Listaa sarjat</label>
    <ref>wvt:///ruutufi/series.xsl?srcurl=http://www.ruutu.fi/ajax/media_get_netti_tv_series_list/all/false&amp;postprocess=json2xml</ref>
  </link>

  <link>
    <label>Uusimmat</label>
    <ref>wvt:///ruutufi/program.xsl?srcurl=http://www.ruutu.fi/ajax/media_get_nettitv_media/all/video_episode/__/latestdesc/0/25/true/__&amp;postprocess=json2xml</ref>
  </link>

  <link>
    <label>Katsotuimmat</label>
    <ref>wvt:///ruutufi/program.xsl?srcurl=http://www.ruutu.fi/ajax/media_get_nettitv_media/all/video_episode/__/most_watched/0/25/true/__&amp;postprocess=json2xml</ref>
  </link>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

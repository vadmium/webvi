<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<wvmenu>
  <title>Vimeo</title>

  <link>
    <label>Search</label>
    <ref>wvt:///vimeo.com/search.xsl?srcurl=http://vimeo.com/</ref>
  </link>

  <link>
    <label>Channels</label>
    <ref>wvt:///vimeo.com/channels.xsl?srcurl=http://vimeo.com/channels/all</ref>
  </link>

  <link>
    <label>Groups</label>
    <ref>wvt:///vimeo.com/groups.xsl?srcurl=http://vimeo.com/groups/all</ref>
  </link>

</wvmenu>
</xsl:template>

</xsl:stylesheet>

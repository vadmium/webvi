<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<wvmenu>
  <title><xsl:value-of select="//title"/></title>

  <textarea>
    <label><xsl:value-of select="//span[@id='long-desc']"/></label>
  </textarea>
  <textarea>
    <label>Duration: <xsl:value-of select="//span[@id='video-duration']"/></label>
  </textarea>
  <textarea>
    <label>Date: <xsl:value-of select="//span[@id='video-date']"/></label>
  </textarea>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

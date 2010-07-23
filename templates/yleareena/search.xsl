<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:template match="fieldset">
  <xsl:if test="select">
    <itemlist>
      <xsl:attribute name="name"><xsl:value-of select="select/@name"/></xsl:attribute>
      <label><xsl:value-of select="label"/></label>
      <xsl:for-each select="select/option|select/optgroup/option">
        <item>
          <xsl:attribute name="value"><xsl:value-of select="@value"/></xsl:attribute>
          <xsl:value-of select="."/>
        </item>
      </xsl:for-each>
    </itemlist>
  </xsl:if>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <title>Hae Areenasta</title>

  <textfield name="keyword">
    <label>Hakusana</label>
  </textfield>

  <xsl:apply-templates select="id('widesearch')/form/fieldset[not(contains(@class, 'search-keyword'))]"/>

  <itemlist name="naytetaan_ulkomailla">
    <label>Vain Suomen ulkopuolella katsottavat</label>
    <item value="kaikki">Kaikki</item>
    <item value="kylla">Kyllä</item>
  </itemlist>

  <button>
    <label>Hae</label>
    <submission>wvt:///yleareena/navigation.xsl?srcurl=http%3A//areena.yle.fi/haku/{category}/uusimmat/hakusana/{keyword}/kanava/{channel}/media/{mediatype}/julkaistu/{date}/kieli/{language}/naytetaan_ulkomailla/{naytetaan_ulkomailla}/feed/rss</submission>
  </button>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="docurl"/>

<xsl:template match="dict">
  <xsl:param name="mediatype" select="video"/>

  <xsl:variable name="videoid">
    <xsl:choose>
      <xsl:when test="video_id_to_use">
	<xsl:value-of select="video_id_to_use"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="substring-after(nodeurl, 'vid=')"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <link>
    <label>
      <xsl:choose>
	<xsl:when test="program_episode_name">
	  <xsl:value-of select="concat(program_episode_name, ' ', video_datetime_to_use)"/>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:value-of select="title"/>
	</xsl:otherwise>
      </xsl:choose>
    </label>
    
    <xsl:variable name="videourl">http://www.nelonen.fi/utils/video_config/%3Fq%3D<xsl:value-of select="$mediatype"/>/<xsl:value-of select="$videoid"/>%26site%3Dwww.ruutu.fi%26ageCheckURL%3Dhttp://sso.nelonenmedia.fi/ajax/check_age/%26current_page%3Dhttp://www.ruutu.fi/video</xsl:variable>

    <ref>wvt:///www.ruutu.fi/description.xsl?srcurl=<xsl:value-of select="$videourl"/></ref>
    <stream>wvt:///www.ruutu.fi/video.xsl?srcurl=<xsl:value-of select="$videourl"/></stream>
  </link>
</xsl:template>

<xsl:template match="/">
<wvmenu>
  <xsl:variable name="start">
    <xsl:value-of select="number(str:tokenize($docurl, '/')[9])"/>
  </xsl:variable>

  <!-- title -->
  <title>
    <xsl:choose>
      <xsl:when test="/jsondocument/dict/video_episode/list/li[1]/dict/series_name">
	<xsl:value-of select="/jsondocument/dict/video_episode/list/li[1]/dict/series_name"/>
      </xsl:when>
      <xsl:when test="/jsondocument/dict/video/list/li[1]/dict/clip_series_name">
	<xsl:value-of select="/jsondocument/dict/video/list/li[1]/dict/clip_series_name"/>
      </xsl:when>
      <xsl:otherwise>Ruutu.fi</xsl:otherwise>
    </xsl:choose>
  </title>

  <!-- Video links -->
  <xsl:if test="not(/jsondocument/dict/video | /jsondocument/dict/video_episode)">
    <textarea>
      <label>Ei jaksoja</label>
    </textarea>
  </xsl:if>

  <xsl:apply-templates select="/jsondocument/dict/video_episode/list/li/dict">
    <xsl:with-param name="mediatype">video_episode</xsl:with-param>
  </xsl:apply-templates>
  <xsl:apply-templates select="/jsondocument/dict/video/list/li/dict">
    <xsl:with-param name="mediatype">video</xsl:with-param>
  </xsl:apply-templates>

  <xsl:if test="contains($docurl, '/video_episode/') and ($start = 0)">
    <link>
      <label>Klipit</label>
      <ref>wvt:///www.ruutu.fi/program.xsl?srcurl=<xsl:value-of select="str:replace($docurl, '/video_episode/', '/video/')"/>&amp;postprocess=json2xml</ref>
    </link>
  </xsl:if>

  <!-- prev/next links -->
  <xsl:variable name="total">
    <xsl:value-of select="number(/jsondocument/dict/total_count)"/>
  </xsl:variable>

  <xsl:variable name="urlend">
    <xsl:text>/</xsl:text><xsl:value-of select="str:tokenize($docurl, '/')[10]"/><xsl:text>/</xsl:text><xsl:value-of select="str:tokenize($docurl, '/')[11]"/><xsl:text>/</xsl:text><xsl:value-of select="str:tokenize($docurl, '/')[12]"/>
  </xsl:variable>

  <xsl:variable name="prevstart">
    <xsl:choose>
      <xsl:when test="$start >= 25">
	<xsl:value-of select="string($start - 25)"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:if test="$start > 0">
    <link>
      <label>Edellinen</label>
      <ref>wvt:///www.ruutu.fi/program.xsl?srcurl=<xsl:value-of select="str:encode-uri(str:replace($docurl, concat(string($start), $urlend), concat($prevstart, $urlend)), true())"/>&amp;postprocess=json2xml</ref>
    </link>
  </xsl:if>

  <xsl:if test="$start + 25 &lt; $total">
    <link>
      <label>Seuraava</label>
      <ref>wvt:///www.ruutu.fi/program.xsl?srcurl=<xsl:value-of select="str:encode-uri(str:replace($docurl, concat(string($start), $urlend), concat(string($start+25), $urlend)), true())"/>&amp;postprocess=json2xml</ref>
    </link>
  </xsl:if>
  
</wvmenu>
</xsl:template>

</xsl:stylesheet>

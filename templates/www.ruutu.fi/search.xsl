<?xml version="1.0" encoding="UTf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:template match="/">
<wvmenu>
  <title>Haku</title>

  <textfield name="query">
    <label>Hakusana</label>
  </textfield>

  <button>
    <label>Hae</label>
    <submission>wvt:///www.ruutu.fi/program.xsl?srcurl=<xsl:value-of select="str:encode-uri('http://www.ruutu.fi/search/search_new.php?params=%7B%22search%22%3A%22{query}%22%2C%22groups%22%3A%7B%22video%22%3A%7B%22types%22%3A%5B%22video_clip%22%5D%7D%2C%22video_episode%22%3A%7B%22types%22%3A%5B%22video_episode%22%5D%7D%2C%22audio%22%3A%7B%22types%22%3A%5B%22audio%22%5D%7D%7D%7D', true())"/>&amp;postprocess=json2xml</submission>
  </button>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

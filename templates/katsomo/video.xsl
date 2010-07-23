<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  exclude-result-prefixes="str">

<xsl:param name="title">katsomovideo</xsl:param>

<xsl:template match="/">
<mediaurl>
  <title><xsl:value-of select="$title"/></title>

  <url><xsl:value-of select='substring-before(substring-after(//script, "metaUrl&apos;: &apos;"), "&apos;")'/></url>
</mediaurl>
</xsl:template>

</xsl:stylesheet>

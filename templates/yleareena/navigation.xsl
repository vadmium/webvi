<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings">

<xsl:param name="docurl"/>
<xsl:param name="title" select="/rss/channel/title"/>

<xsl:template name="prevnextlinks">
  <!-- Add previous and next links for a navigation page.

       Extract the current page number from the URL (the number after
       /sivu/) and adds links to previous and following pages. If the
       page number is missing, it is assumed to be 1.

       BUG: if the last page has 20 links, an extra "next" link is
       generated
    -->

  <xsl:variable name="page" select="number(substring-before(substring-after($docurl, '/sivu/'), '/'))"/>

  <xsl:choose>
    <xsl:when test="$page &gt; 1">

      <xsl:variable name="urlprefix" select="substring-before($docurl, '/sivu/')"/>
      <xsl:variable name="urlpostfix" select="substring-after(substring-after($docurl, '/sivu/'), '/')"/>

      <xsl:variable name="prevurl" select="concat($urlprefix, '/sivu/', $page - 1, '/', $urlpostfix)"/>
      <xsl:variable name="nexturl" select="concat($urlprefix, '/sivu/', $page + 1, '/', $urlpostfix)"/>

      <link>
        <label>Edellinen</label>
        <ref>wvt:///yleareena/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri($prevurl, true())"/></ref>
      </link>      

      <xsl:if test="count(/rss/channel/item) &gt;= 20">
	<link>
          <label>Seuraava</label>
          <ref>wvt:///yleareena/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri($nexturl, true())"/></ref>
	</link>
      </xsl:if>
    </xsl:when>

    <xsl:otherwise>

      <xsl:if test="count(/rss/channel/item) &gt;= 20">
	<xsl:variable name="nexturl">
	  <xsl:choose>
	    <xsl:when test="contains($docurl, '/sivu/')">
	      <xsl:value-of select="concat(substring-before($docurl, '/sivu/'), '/sivu/2/', substring-after(substring-after($docurl, '/sivu/'), '/'))"/>
	    </xsl:when>

	    <xsl:when test="contains($docurl, '/feed/rss')">
	      <xsl:value-of select="str:replace($docurl, '/feed/rss', '/sivu/2/feed/rss')"/>
	    </xsl:when>

	    <xsl:otherwise>
	      <xsl:value-of select="concat($docurl, '/sivu/2')"/>
	    </xsl:otherwise>
	  </xsl:choose>
	</xsl:variable>

	<link>
          <label>Seuraava</label>
          <ref>wvt:///yleareena/navigation.xsl?srcurl=<xsl:value-of select="str:encode-uri($nexturl, true())"/></ref>
	</link>
      </xsl:if>

    </xsl:otherwise>


  </xsl:choose>
</xsl:template>


<xsl:template match="/rss/channel/item">
  <link>
    <label><xsl:value-of select="title"/></label>
    <ref>wvt:///yleareena/description.xsl?srcurl=<xsl:value-of select="str:encode-uri(link, true())"/></ref>
    <stream>wvt:///yleareena/video.xsl?srcurl=<xsl:value-of select="str:encode-uri(link, true())"/>&amp;param=title,<xsl:value-of select="str:encode-uri(concat(title, '-', str:split(pubDate, ' ')[2], '-', str:split(pubDate, ' ')[3], '-', str:split(pubDate, ' ')[4]), true())"/></stream>
  </link>
</xsl:template>


<xsl:template match="/">
<wvmenu>
  <xsl:choose>

    <!-- Regular video links -->
    <xsl:when test="/rss">
      <title><xsl:value-of select="$title"/></title>

      <xsl:apply-templates select="/rss/channel/item"/>

      <xsl:call-template name="prevnextlinks"/>
    </xsl:when>

    <!-- No search results -->
    <xsl:otherwise>
      <title>Hae Areenasta: Ei osumia</title>

      <textarea>
        <xsl:choose>
        <xsl:when test="//h4">
          <label><xsl:value-of select="//h4"/></label>
        </xsl:when>
        <xsl:otherwise>
          <label>Ei osumia</label>
        </xsl:otherwise>
        </xsl:choose>
      </textarea>
    </xsl:otherwise>

  </xsl:choose>
</wvmenu>
</xsl:template>

</xsl:stylesheet>

<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output
    method="xml"
    version="version"
    omit-xml-declaration="yes"
    standalone="no"
    cdata-section-elements="namelist"
    indent="yes"/>
  

  <xsl:template match="/">

    <article>
      <id_article><xsl:value-of select="$article_id"/></id_article>
      <id_rubrique><xsl:value-of select="$rubrique_id"/></id_rubrique>
      <date><xsl:value-of select="$date"/></date>
      <statut>publie</statut>
      <id_secteur><xsl:value-of select="$section_id"/></id_secteur>
      <maj>20041105212810</maj>
      <export>oui</export>
      <date_redac><xsl:value-of select="$date"/></date_redac>
      <visites>0</visites>
      <referers>0</referers>
      <popularite>0</popularite>
      <accepter_forum>non</accepter_forum>
      <auteur_modif>1</auteur_modif>
      <date_modif><xsl:value-of select="$date"/></date_modif>
      <lang><xsl:value-of select="$language"/></lang>
      <langue_choisie>oui</langue_choisie>
      <id_trad><xsl:value-of select="$traduction_id"/></id_trad>
      <nom_site>__REMOVEME__</nom_site>
      <url_site>__REMOVEME__</url_site>
      <extra>__REMOVEME__</extra>
      <idx>oui</idx>
      <id_version>0</id_version>
      <lien_auteur>1</lien_auteur>
      <ps>__REMOVEME__</ps>

      <xsl:for-each select="GComprisBoards/GCompris/Board[starts-with(@section,$section) and (substring-after(substring(@section,string-length($section)),'/')='')]">

        <titre>
          <xsl:variable name="tmptext" select="title[@xml:lang=$language]"/>
          <xsl:if test="not($tmptext)">
            <xsl:value-of select="title"/>
          </xsl:if>
          <xsl:value-of select="$tmptext"/>
        </titre>

      </xsl:for-each>

      <texte>

        <HTML>
          <DIV class="conteneur">
            
            <xsl:for-each select="GComprisBoards/GCompris/Board[starts-with(@section,$section) and (substring-after(substring(@section,string-length($section)),'/')='.')]">
              <!-- order the result by difficulty -->
              <xsl:sort select="@difficulty"
                data-type="number"
                order="ascending"/>

              <P class="flottante">
                <A>
                  <xsl:attribute name="href" align="center">
                    <xsl:value-of select="concat($language,':',@name,'.xml')"/>
                  </xsl:attribute>
                  <IMG border="0" align="top" alt="">
                    <xsl:attribute name="src">
                      <xsl:value-of select="concat('screenshots/',@name)"/>_small.jpg
                    </xsl:attribute>
                    <xsl:attribute name="title">
                      <xsl:variable name="tmptext" select="title[@xml:lang=$language]"/>
                      <xsl:if test="not($tmptext)">
                        <xsl:value-of select="title"/>
                      </xsl:if>
                      <xsl:value-of select="$tmptext"/>
                    </xsl:attribute>
                  </IMG>
                </A>
              </P>
            </xsl:for-each>
            <DIV class="spacer">__REMOVEME__</DIV>
          </DIV>
        </HTML>
      </texte>
    </article>
  </xsl:template>

</xsl:stylesheet>


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
    <xsl:for-each select="GCompris/Board">
      <xsl:if test="@type != 'menu'">
        <xsl:if test="@difficulty != 0">
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
            <nom_site></nom_site>
            <url_site></url_site>
            <extra></extra>
            <idx>oui</idx>
            <id_version>0</id_version>
            <lien_auteur>1</lien_auteur>
            <ps>none</ps>

            <surtitre>
              <xsl:value-of select="@author"/>
              (<xsl:value-of select="@name"/>.xml)
            </surtitre>

            <titre>
              <xsl:variable name="tmptext" select="title[@xml:lang=$language]"/>
              <xsl:if test="not($tmptext)">
                <xsl:value-of select="title"/>
              </xsl:if>
              <xsl:value-of select="$tmptext"/>
            </titre>

            <descriptif>
              <xsl:variable name="tmptext" select="description[@xml:lang=$language]"/>
              <xsl:if test="not($tmptext)">
                <xsl:value-of select="description"/>
              </xsl:if>
              <xsl:value-of select="$tmptext"/>
            </descriptif>

            <soustitre>
              <xsl:variable name="tmptext" select="prerequisite[@xml:lang=$language]"/>
              <xsl:if test="not($tmptext)">
                <xsl:value-of select="prerequisite"/>
              </xsl:if>
              <xsl:value-of select="$tmptext"/>
            </soustitre>

            <chapo>
              <xsl:variable name="tmptext" select="goal[@xml:lang=$language]"/>
              <xsl:if test="not($tmptext)">
                <xsl:value-of select="goal"/>
              </xsl:if>
              <xsl:value-of select="$tmptext"/>
            </chapo>

            <texte>
              <HTML>
                <IMG border="0" alt="" align="top">
                  <xsl:attribute name="src">
                    <xsl:value-of select="concat('screenshots/',@name)"/>.jpg</xsl:attribute>
                  </IMG>

                  <xsl:if test="(@type != 'chess') and (starts-with(@type,'python:')=false)">
                    <IMG border="0" alt="" src="boardicons/win.png">
                    </IMG>
                  </xsl:if>
                  <xsl:if test="@difficulty">
                    <IMG border="0" alt="" align="top">
                      <xsl:attribute name="src">
                        <xsl:value-of select="concat('boardicons/difficulty_star',@difficulty,'.png')"/>
                      </xsl:attribute>
                    </IMG>
                  </xsl:if>

                  <IMG border="0" alt="" align="top">
                    <xsl:attribute name="src"><xsl:value-of select="@icon"/></xsl:attribute>
                  </IMG>
                </HTML>
                
                <xsl:variable name="tmptext5" select="manual[@xml:lang=$language]"/>
                <xsl:if test="not($tmptext5)">
                  <xsl:value-of select="manual"/>
                </xsl:if>
                <xsl:value-of select="$tmptext5"/>

              </texte>
            </article>
          </xsl:if>
        </xsl:if>
      </xsl:for-each>
    </xsl:template>

  </xsl:stylesheet>

#!/usr/bin/python
# -*- coding: utf-8 -*-

from xml.sax import parse
from xml.sax.handler import ContentHandler
import sys
import re

class WikiHandler(ContentHandler):

    def __init__ (self, searchWords, locale):

        self.searchWords= searchWords;
        self.locale = locale

        self.isPageElement = False

        self.isTitleElement = False
        self.titleContent = u""

        self.isTextElement = False
        self.textContent = u""

        self.lilevel = []

    def startElement(self, name, attrs):

        if name == 'page':
            self.isPageElement = True
        elif name == 'title':
            self.isTitleElement = True
            self.titleContent = ""
        elif name == 'text':
            self.isTextElement = True
            self.textContent = ""

        self.wordTypes = {
            "{{-nom-.*}}": "noun",
            "{{-nom-pr.*}}": "proper noun",
            "{{-verb.*}}": "proper noun",
            "{-pronom-.*}": "pronoun",
            "{-verb-.*}}": "verb",
            "{-adj-.*}}": "adjective",
            "{-adv-.*}}": "adverb",
            "{-art-.*}}": "article",
            "{-conj-.*}}": "conjunction",
            "{-prèp-.*}}": "preposition",
            "{-post-.*}}": "postposition"
            }

        self.wordSubTypes = {
            "{{1ergroupe}}": "1er groupe",
            "{{2egroupe}}": "2eme groupe",
            "{{3egroupe}}": "3eme groupe",
            }

        # Some content in wiktionary is obvioulsy not appropriate
        # for children. This contains a list of regexp.
        self.filterContent = [ r"de sade",
                               r"pénis",
                               r"homosexuel",
                               r"vagin"]


    def endElement(self, name):

        if name == 'page':
            self.isPageElement= False
            if self.titleContent in self.searchWords:
                print "<hr></hr><h1>" + self.titleContent + "</h1>"
                self.parseText()

            self.titleContent = ""
            self.textContent = ""

        elif name == 'title':
            self.isTitleElement= False
        elif name == 'text':
            self.isTextElement = False

    def characters (self, ch):
        if self.isTitleElement:
            self.titleContent += ch
        elif self.isTextElement:
            self.textContent += ch


    # Manages bullets and numbered lists
    # mediawiki specification:
    # Bulleted: *
    # Numbered: #
    # Indent with no marking: :
    # Definition list: ;
    # Notes:
    # These may be combined at the start of the line to create
    # nested lists, e.g. *** to give a bulleted list three levels
    # deep, or **# to have a numbered list within two-levels of
    # bulleted list nesting.
    #
    # html specification:
    # Bulleted: <ul> [<li> </li>]+ </ul>
    # Numbered: <ol> [<li> </li>]+ </ol>
    # Notes:
    # These may be nested.
    #
    # We keep the level of indentation to close in the stack:
    # self.lilevel
    #
    def indents2xml(self, text):
        result = re.search(r"^[ ]*[*#:;]+[ ]*", text)
        if not result:
            close = ""
            while self.lilevel:
                close += self.lilevel.pop()
            text = close + text
            return text

        indent = result.group(0).rstrip()
        text = text[result.end():]

        result = ""
        # Close indents if needed
        while len(self.lilevel) > len(indent):
            result += self.lilevel.pop()

        # Open new indents
        # Remove the current level from it
        indent = indent[len(self.lilevel):]
        for char in list(indent):
            if char in "*:;":
                result += "<ul>"
                self.lilevel.append("</ul>")
            elif char == "#":
                result += "<ol>"
                self.lilevel.append("</ol>")

        return result + "<li>" + text + "</li>"

    def quote2xml(self, quote, openXml, closeXml, text):
        index = 0
        while index >= 0:
            index = text.find(quote)
            if index >= 0:
                text = text.replace(quote, openXml, 1)
            else:
                return text

            index = text.find(quote)
            if index >= 0:
                text = text.replace(quote, closeXml, 1)
            else:
                # Malformed statement, should fix wiktionary
                text += closeXml
        return text

    # Replace standard Wiki tags to XML
    def wiki2xml(self, text):

        text = re.sub(r"{{[-\)\(]}}", "", text)
        text = re.sub(r"\[\[\w+:\w+\]\]", "", text)
        text = re.sub(r"{{\(\|(.*)}}", r"\1", text)
        if text == "":
            return ""

        text = self.indents2xml(text)
        text = re.sub(r"{{par ext[^}]+}}", r"(Par extension)", text)
        text = re.sub(r"{{litt[^}]+}}", r"(Littéraire)", text)
        text = re.sub(r"{{figuré[^}]+}}", r"(Figuré)", text)
        text = re.sub(r"{{dés[^}]+}}", r"(Désuet)", text)
        text = re.sub(r"{{vx[^}]+}}", r"(Vieilli)", text)
        text = re.sub(r"{{w\|([^}]+)}}", r"<i>\1</i>", text)
        text = re.sub(r"{{source\|([^}]+)}}", r"- (\1)", text)

        # Remove all unrecognized wiki tags
        text = re.sub(r"{{[^}]+}}", "", text)

        # bold
        text = self.quote2xml("'''", "<b>", "</b>", text)
        # italic
        text = self.quote2xml("''", "<i>", "</i>", text)

        # Get rid of hyperlinks
        while text.find("[[") != -1:
            start = text.find("[[")
            stop = text.find("]]")
            pipe = text.find("|", start, stop)
            if pipe == -1:
                text = text.replace("[[", "", 1)
                text = text.replace("]]", "", 1)
            else:
                text = text[:start] + text[pipe+1:]
                text = text.replace("]]", "", 1)

        return text

    # Wikipedia text content is interpreted and transformed in XML
    def parseText(self):
        inDefinition = False
        inAnagram = False
        inSynonym = False
        inAntonym = False
        inPron = False
        wordType = ""
        wordSubType = ""

        # Append and end of text marker, it makes my life easier
        self.textContent += "\n{{-EndOfTest-}}"

        # Remove html comment (multilines)
        self.textContent = re.sub(r"<!--[^>]*-->", "",
                                  self.textContent, re.M)

        for l in self.textContent.splitlines():

            for filter in self.filterContent:
                if re.search(filter, l, re.I):
                    return

            # Are we still in the correct language section
            # We assume the correct language is ahead
            lang = re.match(r"== {{=([a-z]+)=}} ==", l)
            if lang and lang.group(1) != None and lang.group(1) != self.locale:
                return

            for wt in self.wordTypes.keys():
                if re.search(wt, l):
                    wordType = self.wordTypes[wt]

            for wt in self.wordSubTypes.keys():
                if re.search(wt, l):
                    wordSubType = self.wordSubTypes[wt]

            if inDefinition:
                if not re.search(r"{{-.*-.*}}", l):
                    print self.wiki2xml(l)
                else:
                    inDefinition = False
                    print self.wiki2xml("</definition>")

            if inAnagram:
                if not re.search(r"{{-.*-.*}}", l):
                    print self.wiki2xml(l)
                else:
                    inAnagram = False
                    print self.wiki2xml("</anagram>")

            if inSynonym:
                if not re.search(r"{{-.*-.*}}", l):
                    print self.wiki2xml(l)
                else:
                    inSynonym = False
                    print self.wiki2xml("</synonym>")

            if inAntonym:
                if not re.search(r"{{-.*-.*}}", l):
                    print self.wiki2xml(l)
                else:
                    inAntonym = False
                    print self.wiki2xml("</antonym>")

            if inPron:
                if l != "" and l.find(".ogg") != -1:
                    # Search the .ogg file
                    file = l.split("=")
                    if len(file) >= 2:
                        file = file[1].replace("}}", "")
                        print "<a href=http://fr.wiktionary.org/wiki/Fichier:" \
                            + file + ">" + file  + "</a>"
                else:
                    inPron = False
                    print self.wiki2xml("</prononciation>")

            if l.startswith("'''" + self.titleContent + "'''"):
                inDefinition = True
                print "<h2>Definition " + self.titleContent + "</h2>"
                print("<definition name='" + self.titleContent + "'" +
                      " type='" + wordType + "'" +
                      " subtype='" + wordSubType + "'" + ">")
                print("<h3>" + wordType + " " + wordSubType + "</h3>")
                wordType = ""
                wordSubType = ""
            elif l == "{{-anagr-}}":
                inAnagram = True
                print "<h2>Anagram</h2>"
                print("<anagram>")
            elif l == "{{-syn-}}":
                inSynonym= True
                print "<h2>Synonym</h2>"
                print("<synonym>")
            elif l == "{{-ant-}}":
                inAntonym = True
                print "<h2>Antonym</h2>"
                print("<antonym>")
            elif l == "{{-pron-}}":
                inPron = True
                print "<h2>Prononciation</h2>"
                print("<prononciation>")



def printHtmlHeader():
    print '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">'
    print '<html xmlns="http://www.w3.org/1999/xhtml" lang="fr" dir="ltr">'
    print '<head>'
    print '<title>accueil - Wiktionnaire</title>'
    print '<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />'

def printHtmlFooter():
    print '</head>'
    print '</html>'

def usage():
    print "wiki2xml.py <wiki file> <word list file>"

# Set UTF-8 stdout in case of the user piping our output
reload(sys)
sys.setdefaultencoding('utf-8')

if len(sys.argv) != 3:
    usage()
    sys.exit()

wikiFile = sys.argv[1]
wordsFile = sys.argv[2]

# Import the list of words
f = open(wordsFile, "r")
words = []
words = [w.rstrip() for w in f.readlines()]
f.close()

printHtmlHeader()
parse(wikiFile, WikiHandler(words, 'fr'))
printHtmlFooter()


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
        result = re.search("[*#:;]+[ ]*", text)
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

    # Replace standard Wiki tags to XML
    def wiki2xml(self, text):

        text = re.sub(r"{{[-\)]}}", "", text)
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

        # Remove all recognized wiki tags
        text = re.sub(r"{{[^}]+}}", "", text)

        # italic
        while text.find("'''") != -1:
            text = text.replace("'''", "<b>", 1)
            text = text.replace("'''", "</b>", 1)

        # bold
        while text.find("''") != -1:
            text = text.replace("''", "<i>", 1)
            text = text.replace("''", "</i>", 1)

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

        for l in self.textContent.splitlines():

            # Are we still in the correct language section
            # We assume the correct language is ahead
            lang = re.match(r"== {{=([a-z]+)=}} ==", l)
            if lang and lang.group(1) != None and lang.group(1) != self.locale:
                return

            if inDefinition:
                if l != "":
                    print self.wiki2xml(l)
                else:
                    inDefinition = False
                    print self.wiki2xml("</definition>")

            if inAnagram:
                if l != "":
                    print self.wiki2xml(l)
                else:
                    inAnagram = False
                    print self.wiki2xml("</anagram>")

            if inSynonym:
                if l != "":
                    print self.wiki2xml(l)
                else:
                    inSynonym = False
                    print self.wiki2xml("</synonym>")

            if inAntonym:
                if l != "":
                    print self.wiki2xml(l)
                else:
                    inAntonym = False
                    print self.wiki2xml("</antonym>")

            if inPron:
                if l != "" and l.find(".ogg") != -1:
                    # Search the .ogg file
                    file = l.split("=")
                    if len(file) >= 2:
                        file = file[1]
                        print "voices/fr/words/" + file.replace("}}", "")
                else:
                    inPron = False
                    print self.wiki2xml("</prononciation>")

            if l.startswith("'''" + self.titleContent + "'''"):
                inDefinition = True
                print("<definition name='" + self.titleContent + "'>")
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


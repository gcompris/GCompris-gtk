#!/usr/bin/python
# -*- coding: utf-8 -*-

from xml.sax import parse
from xml.sax.handler import ContentHandler
import sys
import re

import wiktio

class WikiHandler(ContentHandler):

    def __init__ (self, searchWords, locale, _wiktio):

        self.searchWords= searchWords;
        self.locale = locale
        self.wiktio = _wiktio

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
                word = self.parseText()
                if word:
                    self.wiktio.addWord(word)

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
    def indents2xml(self, text, asText):
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

        if asText:
            return text
        else:
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
    def wiki2xml(self, text, asText):

        text = re.sub(r"{{[-\)\(]}}", "", text)
        text = re.sub(r"\[\[\w+:\w+\]\]", "", text)
        text = re.sub(r"{{\(\|(.*)}}", r"\1", text)
        if text == "":
            return ""

        text = self.indents2xml(text, asText)
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
        inWord = None
        inDefinition = None
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
                    return inWord

            # Categories
            # print "1>" + l
            # if re.search(r"^\[\[Cat\Sgorie:", l, re.U):
            #     text = re.sub(r"\[\[Cat\Sgorie:(\S\s)+\]\]", r"\1", l)
            #     print "ICI" + text
            #     print "<category>" + text + "</category>"
            #     continue

            # Are we still in the correct language section
            # We assume the correct language is ahead
            lang = re.match(r"== {{=([a-z]+)=}} ==", l)
            if lang and lang.group(1) != None and lang.group(1) != self.locale:
                return inWord

            for wt in self.wordTypes.keys():
                if re.search(wt, l):
                    wordType = self.wordTypes[wt]

            for wt in self.wordSubTypes.keys():
                if re.search(wt, l):
                    wordSubType = self.wordSubTypes[wt]

            if inDefinition:
                if not re.search(r"{{-.*-.*}}", l):
                    inDefinition.addText(self.wiki2xml(l, False))
                else:
                    inWord.addDefinition(inDefinition)
                    inDefinition = None

            if inAnagram:
                if not re.search(r"{{-.*-.*}}", l) and len(l) > 0:
                    inWord.addAnagram(self.wiki2xml(l, True))
                else:
                    inAnagram = False

            if inSynonym:
                if not re.search(r"{{-.*-.*}}", l):
                    inWord.addSynonym(self.wiki2xml(l, True))
                else:
                    inSynonym = False

            if inAntonym:
                if not re.search(r"{{-.*-.*}}", l):
                    inWord.addAntonym(self.wiki2xml(l, True))
                else:
                    inAntonym = False

            if inPron:
                if not re.search(r"{{-.*-.*}}", l):
                    if l.find(".ogg") != -1:
                        # Search the .ogg file
                        file = l.split("=")
                        if len(file) >= 2:
                            file = file[1].replace("}}", "")
                            inWord.addPronociation(file)
                else:
                    inPron = False

            if l.startswith("'''" + self.titleContent + "'''"):
                inWord = wiktio.Word(self.titleContent)
                inDefinition = wiktio.Definition()
                inDefinition.setType(wordType)
                inDefinition.setSubType(wordSubType)
                wordType = ""
                wordSubType = ""
            elif l == "{{-anagr-}}":
                inAnagram = True
            elif l == "{{-syn-}}":
                inSynonym= True
            elif l == "{{-ant-}}":
                inAntonym = True
            elif l == "{{-pron-}}":
                inPron = True

        return inWord

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

_wiktio = wiktio.Wiktio()

parse(wikiFile, WikiHandler(words, 'fr', _wiktio))


_wiktio.dump2html()

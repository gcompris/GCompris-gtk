#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2010 Bruno Coudoin
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, see <http://www.gnu.org/licenses/>.
#

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

        self.genders = {
            "{{m}}": u"masculin",
            "{{f}}": u"féminin",
            "{{mf}}": u"masculin et féminin"
            }

        self.wordTypes = {
            u"{{-nom-.*}}": u"nom",
            u"{{-nom-pr.*}}": u"nom propre",
            u"{{-verb.*}}": u"verbe",
            u"{-pronom-.*}": u"pronom",
            u"{-adj-.*}}": u"adjectif",
            u"{-adv-.*}}": u"adverbe",
            u"{-art-.*}}": u"article",
            u"{-conj-.*}}": u"conjunction",
            u"{-prèp-.*}}": u"préposition",
            u"{-post-.*}}": u"postposition"
            }

        self.wordSubTypes = {
            "{{1ergroupe}}": "1er groupe",
            "{{2egroupe}}": "2eme groupe",
            "{{3egroupe}}": "3eme groupe",
            }

        # Some content in wiktionary is obvioulsy not appropriate
        # for children. This contains a list of regexp.
        self.filterContent = [ ur"de sade",
                               ur"pénis",
                               ur"homosexuel",
                               ur"vagin"]

        self.filterDefinitionType = [ ur"{{vulg[^}]*}}",
                                      ur"{{injur[^}]*}}",
                                      ur"{{dés[^}]*}}",
                                      ur"{{vx[^}]*}}",
                                      ur"{{métonymie[^}]*}}",
                                      ur"{{familier[^}]*}}",
                                      ur"coït",
                                      ur"argot"]

    def endElement(self, name):

        if name == 'page':
            self.isPageElement= False
            if self.titleContent in self.searchWords:
                word = self.parseText()
                if word and word.name:
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
            if not asText:
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

    # Replaces '''xx''' and ''xx'' from the given text
    # with openXml xx closeXml
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
        text = re.sub(r"{{\(\|(.*)}}", r"", text)
        if text == "":
            return self.indents2xml(text, asText)

        text = self.indents2xml(text, asText)
        text = re.sub(ur"{{par ext[^}]*}}", ur"(Par extension)", text)
        text = re.sub(ur"{{litt[^}]*}}", ur"(Littéraire)", text)
        text = re.sub(ur"{{figuré[^}]*}}", ur"(Figuré)", text)
        text = re.sub(ur"{{w\|([^}]+)}}", ur"<i>\1</i>", text)
        text = re.sub(ur"{{source\|([^}]+)}}", ur"- (\1)", text)

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
        inWord = wiktio.Word()
        definition = None

        (DEFINITION,
         ANAGRAM,
         SYNONYM,
         ANTONYM,
         HYPERONYM,
         HYPONYM,
         PRON,
         SKIP) = range(0, 8)

        state = SKIP

        wordType = ""
        wordSubType = ""
        filterIndent = ""
        gender = ""

        # Append an end of text marker, it makes my life easier
        self.textContent += "\n{{-EndOfTest-}}"

        # Remove html comment (multilines)
        self.textContent = re.sub(r"<!--[^>]*-->", "",
                                  self.textContent, re.M)

        definition = wiktio.Definition()
        concat = ""
        for l in self.textContent.splitlines():
            l = concat + l
            next = False

            if re.search(r"<[^>]+$", l):
                # Wiki uses a trick to format text area by ending in uncomplete
                # html tags. In this case, we concat this line with the next one
                # before processing it
                concat = l
                continue

            # Determine the section of the document we are in
            if l.startswith("'''" + self.titleContent + "'''"):
                inWord.setName(self.titleContent)
                # Get rid of the word, we don't want it in the definition
                l = re.sub(r"'''.*'''(.*)", r"\1", l)
                state = DEFINITION
            elif l == "{{-anagr-}}":
                state = ANAGRAM
            elif l == "{{-syn-}}":
                state = SYNONYM
            elif l == "{{-ant-}}":
                state = ANTONYM
            elif l == "{{-hyper-}}":
                state = HYPERONYM
            elif l == "{{-hypo-}}":
                state = HYPONYM
            elif l == "{{-pron-}}":
                state = PRON
            elif re.search(r"{{-.*-.*}}", l):
                if definition.text != "":
                    # Force a <ul> close if needed
                    definition.addText(self.wiki2xml("", False))
                    inWord.addDefinition(definition)
                    # Next definition
                    definition = wiktio.Definition()
                state = SKIP

            # Are we still in the correct language section
            # We assume the correct language is ahead
            lang = re.match(r"== {{=([a-z]+)=}} ==", l)
            if lang and lang.group(1) != None and lang.group(1) != self.locale:
                return inWord

            # Image
            if definition and re.match(ur"\[\[Image:", l):
                text = re.sub(ur"\[\[Image:([^|}\]]+).*", r"\1", l)
                definition.addImage(text)
                continue

            for wt in self.wordTypes.keys():
                if re.search(wt, l):
                    wordType = self.wordTypes[wt]
                    definition.setType(wordType)

            for wt in self.genders.keys():
                if re.search(wt, l):
                    gender = self.genders[wt]
                    definition.setGender(gender)

            for wt in self.wordSubTypes.keys():
                if re.search(wt, l):
                    wordSubType = self.wordSubTypes[wt]
                    definition.setSubType(wordSubType)

            if state == SKIP:
                continue

            for filter in self.filterContent:
                if re.search(filter, l, re.I):
                    if definition:
                        definition.filtered = True

            if filterIndent != "":
                # We are filtering, check this line is
                # at a lower indentation level
                result = re.search(r"^[ ]*[*#:;]+[ ]*", l)
                if result:
                    if len(result.group(0).rstrip()) > len(filterIndent):
                        next = True
                    else:
                        filterIndent = ""
                else:
                    filterIndent = ""


            for filter in self.filterDefinitionType:
                if re.search(filter, l, re.I):
                    result = re.search(r"^[ ]*[*#:;]+[ ]*", l)
                    if result:
                        # Keep the indent level for which we filter
                        filterIndent = result.group(0).rstrip()
                    next = True

            if next:
                continue

            # Categories
            if definition and re.match(ur"\[\[Catégorie:", l):
                text = re.sub(ur"\[\[Catégorie:([^|}\]]+).*", r"\1", l)
                definition.addCategory(text)
                continue

            if state == DEFINITION:
                definition.addText(self.wiki2xml(l, False))
            elif state == ANAGRAM:
                if len(l) > 0:
                    definition.addAnagram(self.wiki2xml(l, True))
            elif state == SYNONYM:
                definition.addSynonym(self.wiki2xml(l, True))
            elif state == ANTONYM:
                definition.addAntonym(self.wiki2xml(l, True))
            elif state == HYPERONYM:
                definition.addHyperonym(self.wiki2xml(l, True))
            elif state == HYPONYM:
                definition.addHyponym(self.wiki2xml(l, True))
            elif state == PRON:
                file = re.subn(r".*audio=([^|}]+).*", r"\1", l)
                if file[1] == 1:
                    definition.addPrononciation(file[0])

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

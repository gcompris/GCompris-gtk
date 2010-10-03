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

#
# Implementation of the wiktionary model
# used

import os.path

class Definition:

    def __init__ (self):
        self.text = ""
        self.type = ""
        self.subType = ""
        self.filtered = False
        self.gender = ""
        self.synonym = []
        self.antonym = []
        self.anagram = []
        self.hyperonym = []
        self.hyponym = []
        self.prononciation = []
        self.category = []
        self.image = []

    def addText(self, text):
        self.text += text

    def setType(self, type):
        self.type = type

    def setSubType(self, subType):
        self.subType = subType

    def setGender(self, gender):
        self.gender = gender

    def add(self, atype, text):
        if len(text) == 0:
            return

        if atype == Wiktio.ANAGRAM:
            self.anagram.append(text)
        elif atype == Wiktio.SYNONYM:
            self.synonym.append(text)
        elif atype == Wiktio.ANTONYM:
            self.antonym.append(text)
        elif atype == Wiktio.HYPERONYM:
            self.hyperonym.append(text)
        elif atype == Wiktio.HYPONYM:
            self.hyponym.append(text)
        elif atype == Wiktio.PRON:
            self.prononciation.append(text)
        elif atype == Wiktio.IMAGE:
            self.image.append(text)
        elif atype == Wiktio.CATEGORY:
            self.category.append(text)
        else:
            print "!!ERROR!!: Type not supported"

    def dump2htmlImage(self, f):
        if self.image:
            prefix = "http://fr.wiktionary.org/wiki/Fichier:"
            for img in self.image:
                f.write ( "<a href='" + prefix + img + "'>" + \
                    img + '</a><br/>' )

    def dump2htmlPrononciation(self, f, title, liste):
        prefix = "http://commons.wikimedia.org/wiki/File:"
        if len(liste):
            f.write ( "<h2>" + title + "</h2>" )
            f.write ( "<ul>" )
            for s in liste:
                f.write ( "<li><a href='" + prefix + s + "'>" \
                    + s + "</a></li>" )
            f.write ( "</ul>" )

    def dump2htmlItem(self, f, title, liste):

        if len(liste):
            f.write ( "<h2>" + title + "</h2>" )
            for s in liste:
                if s.find(":") >= 0:
                    f.write ( "<br/>" + s )
                else:
                    f.write ( s + ", " )

    def dump2html(self, f):
        if self.filtered or self.text == "":
            return
        f.write ( "<h3>" + self.type + \
            " " + self.subType + \
            " " + self.gender + "</h3>" )
        self.dump2htmlImage(f)
        f.write ( self.text )

        self.dump2htmlItem(f, "Synonymes", self.synonym)
        self.dump2htmlItem(f, "Antonymes", self.antonym)
        self.dump2htmlItem(f, "Anagrammes", self.anagram)
        self.dump2htmlItem(f, "Hyperonymes", self.hyperonym)
        self.dump2htmlItem(f, "Hyponymes", self.hyponym)
        self.dump2htmlPrononciation(f, "Prononciation", self.prononciation)
        self.dump2htmlItem(f, u"Catégories", self.category)

class Word:

    def __init__ (self, name = None):
        self.name = name
        self.definition = []

    def setName(self, name):
        self.name = name

    def addDefinition(self, definition):
        self.definition.append(definition)

    def dump2html(self, f):
        f.write ( "<hr/>" )
        f.write ( "<h1>" + self.name + "</h1>" )
        if not self.definition:
            f.write ( "<h2>ERROR: NO DEFINITION</h2>" )
            return
        for d in self.definition:
            d.dump2html(f)


class Wiktio:

    (DEFINITION,
     ANAGRAM,
     SYNONYM,
     ANTONYM,
     HYPERONYM,
     HYPONYM,
     PRON,
     IMAGE,
     CATEGORY,
     SKIP) = range(0, 10)

    def __init__ (self):
        self.words = []

    def addWord(self, word):
        self.words.append(word)

    def getWords(self):
        return self.words

    def sort(self):
        self.words.sort(key=lambda word: word.name.lower())

    def dumpHtmlHeader(self, f):
        f.write ( """
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="fr" dir="ltr">
<head>
<title>Mini - Wiktionnaire</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
""")

    def dumpHtmlFooter(self, f):
        f.write ("""
</head>
""")

    # Creates a big HTML file, useful to debug
    def dump2html(self, file):
        with open(file, 'w') as f:
            self.dumpHtmlHeader(f)
            self.sort()
            for w in self.words:
                w.dump2html(f)
            self.dumpHtmlFooter(f)

    # Creates a static HTML site in the given directory
    def dump2htmlSite(self, baseDir):
        if not os.path.isdir(baseDir):
            print "ERROR: Directory '" + baseDir + "' does not exists."
            return

        letter = "/"
        self.sort()
        with open(baseDir + '/index.html', 'w') as f_index:
            self.dumpHtmlHeader(f_index)
            for w in self.words:
                if letter[0] != w.name[0].upper():
                    letter = w.name[0].upper()
                    f_index.write ( "<hr/><h1>" + letter[0] + "</h1>" )
                f_index.write ( "<a href='" + w.name + ".html'>" + w.name + "</a> " )
                with open(baseDir + '/' + w.name + '.html', 'w') as f:
                    self.dumpHtmlHeader(f)
                    w.dump2html(f)
                    self.dumpHtmlFooter(f)

            self.dumpHtmlFooter(f_index)


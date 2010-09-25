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

    def addSynonym(self, synonym):
        if len(synonym):
            self.synonym.append(synonym)

    def addAntonym(self, antonym):
        if len(antonym):
            self.antonym.append(antonym)

    def addAnagram(self, anagram):
        if len(anagram):
            self.anagram.append(anagram)

    def addHyperonym(self, hyperonym):
        if len(hyperonym):
            self.hyperonym.append(hyperonym)

    def addHyponym(self, hyponym):
        if len(hyponym):
            self.hyponym.append(hyponym)

    def addPrononciation(self, prononciation):
        if len(prononciation):
            self.prononciation.append(prononciation)

    def addCategory(self, category):
        if len(category):
            self.category.append(category)

    def addImage(self, image):
        if len(image):
            self.image.append(image)

    def dump2htmlImage(self):
        if self.image:
            prefix = "http://fr.wiktionary.org/wiki/Fichier:"
            for img in self.image:
                print "<a href='" + prefix + img + "'>" + \
                    img + '</a><br/>'

    def dump2htmlPrononciation(self, title, liste):
        prefix = "http://commons.wikimedia.org/wiki/File:"
        if len(liste):
            print "<h2>" + title + "</h2>"
            print "<ul>"
            for s in liste:
                print "<li><a href=" + prefix + s + ">" \
                    + s + "</a></li>"
            print "</ul>"

    def dump2htmlItem(self, title, liste):

        if len(liste):
            print "<h2>" + title + "</h2>"
            for s in liste:
                if s.find(":") >= 0:
                    print "<br/>" + s
                else:
                    print s + ", "

    def dump2html(self):
        if self.filtered:
            return
        print "<h3>" + self.type + \
            " " + self.subType + \
            " " + self.gender + "</h3>"
        self.dump2htmlImage()
        print self.text

        self.dump2htmlItem("Synonymes", self.synonym)
        self.dump2htmlItem("Antonymes", self.antonym)
        self.dump2htmlItem("Anagrammes", self.anagram)
        self.dump2htmlItem("Hyperonymes", self.hyperonym)
        self.dump2htmlItem("Hyponymes", self.hyponym)
        self.dump2htmlPrononciation("Prononciation", self.prononciation)
        self.dump2htmlItem(u"Catégories", self.category)

class Word:

    def __init__ (self, name = None):
        self.name = name
        self.definition = []

    def setName(self, name):
        self.name = name

    def addDefinition(self, definition):
        self.definition.append(definition)

    def dump2html(self):
        print "<hr></hr>"
        print "<h1>" + self.name + "</h1>"
        for d in self.definition:
            d.dump2html()


class Wiktio:

    def __init__ (self):
        self.words = []

    def addWord(self, word):
        self.words.append(word)

    def getWords(self):
        return self.words

    def sort(self):
        self.words.sort(key=lambda word: word.name.lower())

    def dumpHtmlHeader(self):
        print """
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="fr" dir="ltr">
<head>
<title>Mini - Wiktionnaire</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
"""
    def dumpHtmlFooter(self):
        print """
</head>
"""
    def dump2html(self):
        self.dumpHtmlHeader()
        self.sort()
        for w in self.words:
            w.dump2html()
        self.dumpHtmlFooter()

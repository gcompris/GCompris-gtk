# -*- coding: utf-8 -*-

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

    def addText(self, text):
        self.text += text

    def setType(self, type):
        self.type = type

    def setSubType(self, subType):
        self.subType = subType

    def setGender(self, gender):
        self.gender = gender

    def dump2html(self):
        if self.filtered:
            return
        print "<h3>" + self.type + \
            " " + self.subType + \
            " " + self.gender + "</h3>"
        print self.text

class Word:

    def __init__ (self, name = None):
        self.name = name
        self.definition = []
        self.synonym = []
        self.antonym = []
        self.anagram = []
        self.prononciation = []
        self.category = []

    def setName(self, name):
        self.name = name

    def addDefinition(self, definition):
        self.definition.append(definition)

    def addSynonym(self, synonym):
        if len(synonym):
            self.synonym.append(synonym)

    def addAntonym(self, antonym):
        if len(antonym):
            self.antonym.append(antonym)

    def addAnagram(self, anagram):
        if len(anagram):
            self.anagram.append(anagram)

    def addPrononciation(self, prononciation):
        if len(prononciation):
            self.prononciation.append(prononciation)

    def addCategory(self, category):
        if len(category):
            self.category.append(category)

    def dump2htmlItem(self, title, liste):
        if len(liste):
            print "<h2>" + title + "</h2>"
            for s in liste:
                if s.find(":") >= 0:
                    print "<br></br>" + s
                else:
                    print s

    def dump2htmlPrononciation(self, title, liste):
        prefix = "http://commons.wikimedia.org/wiki/File:"
        if len(liste):
            print "<h2>" + title + "</h2>"
            print "<ul>"
            for s in liste:
                print "<li><a href=" + prefix + s + ">" \
                    + s + "</a></li>"
            print "</ul>"

    def dump2html(self):
        print "<hr></hr>"
        print "<h1>" + self.name + "</h1>"
        for d in self.definition:
            d.dump2html()

        self.dump2htmlItem("Synonym", self.synonym)
        self.dump2htmlItem("Antonym", self.antonym)
        self.dump2htmlItem("Anagram", self.anagram)
        self.dump2htmlPrononciation("Prononciation", self.prononciation)
        self.dump2htmlItem("Category", self.category)

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

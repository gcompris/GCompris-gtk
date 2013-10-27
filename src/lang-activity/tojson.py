#!/usr/bin/python
#  gcompris - tojson.py
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
# lang activity xml data file to json converter.
#
# To pipe its output you must first do:
# export PYTHONIOENCODING=utf-8

import xml.dom.minidom
import codecs
import os.path
import sys
import gettext

def isNode(e, name):
    return e.nodeType == e.ELEMENT_NODE and e.localName == name


class Triplet:
    def __init__(self, elem, translations, locale):
        self.description = None
        self.descriptionTranslated = None
        self.image = None
        self.voice = None
        self.type = None
        self.valid = True
        self.parse(elem, translations, locale)

    def parse(self, elem, translations, locale):
        for e in elem.childNodes:
            if isNode(e, "image"):
                if e.firstChild:
                    self.image = e.firstChild.nodeValue
                else:
                    self.valid = False
            elif isNode(e, "description"):
                self.description = e.firstChild.nodeValue if e.firstChild else None
            elif isNode(e, "voice"):
                if e.firstChild:
                    self.voice = e.firstChild.nodeValue.replace("$LOCALE", locale)
                else:
                    self.valid = False
            elif isNode(e, "type"):
                self.type = e.firstChild.nodeValue if e.firstChild else None

        if self.valid:
            if not os.path.exists("resources/" + self.image):
                self.valid = False
            elif not os.path.exists("../../boards/" + self.voice):
                self.valid = False

        # Get the translation
        if self.valid:
            try:
                # The file name
                fileName = self.voice.rsplit("/", 1)[1]
                self.descriptionTranslated = translations[fileName]
            except:
                self.descriptionTranslated = \
                    _("Missing translation for '{word}'".format(word = self.description))


    def isValid(self):
        return self.valid

    def dump(self):
        return '            { "description": "'+ self.descriptionTranslated + '", ' \
            + '"image": "' + str(self.image) + '", "voice": "' + str(self.voice) + '"}'

class Lesson:

    def __init__(self, lesson, locale):
        self.name = None
        self.description = None
        self.triplets = []
        if lesson:
            self.name = lesson.name
            self.description = lesson.description

    def getTriplets(self):
        return self.triplets

    def dump(self):
        result = '       { "name": "'+ _( self.name ) + '",\n'
        result += '          "triplets": [\n'
        tripletResults = []
        for triplet in self.triplets:
            tripletResults.append( triplet.dump() )
        result += ",\n".join(tripletResults) + "\n"
        result += '          ]\n'
        result += '        }'
        return result

class LessonCreator:

    MAX_TRIPLETS = 12

    def __init__(self, elem, translations, locale):
        self.lessons = []
        self.parse(elem, translations, locale)

    def parse(self, elem, translations, locale):
        lesson = Lesson(None, locale)
        for e in elem.childNodes:
            if isNode(e, "name"):
                lesson.name = e.firstChild.nodeValue
            elif isNode(e, "description"):
                lesson.description = e.firstChild.nodeValue if e.firstChild else None
            elif isNode(e, "Triplet"):
                triplet = Triplet(e, translations, locale)
                if triplet.isValid():
                    if len(lesson.triplets) < self.MAX_TRIPLETS:
                        lesson.triplets.append( triplet )
                    else:
                        self.lessons.append(lesson)
                        lesson = Lesson(lesson, locale)


        if len(lesson.triplets) and len(lesson.triplets) < self.MAX_TRIPLETS:
            # There is no enough triplet for this level, add the first
            # of the first lesson
            if len(self.lessons):
                for triplet in self.lessons[0].getTriplets()[0:self.MAX_TRIPLETS-len(lesson.triplets)]:
                    lesson.triplets.append(triplet)
            self.lessons.append(lesson)


    def getLessons(self):
        return self.lessons


class Chapter:
    def __init__(self, elem, translations, locale):
        self.name = None
        self.description = None
        self.lessons = []
        self.parse(elem, translations, locale)

    def parse(self, elem, translations, locale):
        for e in elem.childNodes:
            if isNode(e, "name"):
                self.name = e.firstChild.nodeValue if e.firstChild else None
            elif isNode(e, "description"):
                self.description = e.firstChild.nodeValue if e.firstChild else None
            elif isNode(e, "Lesson"):
                lessonCreator = LessonCreator(e, translations, locale)
                for lesson in lessonCreator.getLessons():
                    self.lessons.append( lesson )

    def getLessons(self):
        return self.lessons

    def dump(self):
        result = '    { "name": "'+ _( self.name ) + '",\n'
        result += '      "lessons": [\n'
        results = []
        for lesson in self.lessons:
            results.append(lesson.dump())
        result += ",\n".join(results) + "\n"
        result += '    ]\n'
        result += '    }'
        return result

class Chapters:
    def __init__(self, doc, translations, locale):
        self.chapters = {}
        self.parse(doc, translations, locale)

    def parse(self, doc, translations, locale):
        for elem in doc:
            chapter = Chapter(elem, translations, locale)
            self.chapters[chapter.name] = chapter

    def getChapters(self):
        return self.chapters

    def dump(self):
        result = '  "chapters": [\n'
        results = []
        for k, chapter in self.chapters.iteritems():
            results.append(chapter.dump())
        result += ",\n".join(results) + "\n"
        result += "  ]\n"
        return result

class LangLib:
    def __init__(self, fileName, locale):
        self.translationsFile = "../../boards/voices/" + locale + "/words/content.txt"
        self.translations = {}

        if self.translationsFile:
            for line in codecs.open(self.translationsFile, 'r', 'utf-8'):
                lineSplitted = line.rstrip('\r\n').split(":")
                if line[0] == '#' or len(lineSplitted) < 2: continue
                self.translations[lineSplitted[0]] = unicode(lineSplitted[1])

        doc = xml.dom.minidom.parse(fileName)
        self.chapters = Chapters( doc.getElementsByTagName('Chapter'), self.translations, locale )

    def dump(self):
        result = "{"
        result += I18N(locale).dump()
        result += Locales(locale).dump()
        result += self.chapters.dump()
        result += "}"
        return result

    def getChapters(self):
        return self.chapters

    def getLesson(self, chapterName, lessonIndex):
        try:
            return self.chapters.getChapters()[chapterName].getLessons()[lessonIndex]
        except:
            return None

class Locales:
    '''This adds the list of locales to the web interface'''
    def __init__(self, locale):
        self.locale = locale

    def dump(self):
        result = ' "locales": [\n'
        result += '    {"name": "' + _("English (United States)") + '", "code": "en" },\n'
        result += '    {"name": "' + _("Spanish") + '", "code": "es" },\n'
        result += '    {"name": "' + _("French") + '", "code": "fr" },\n'
        result += '    {"name": "' + _("Scottish Gaelic") + '", "code": "gd" }\n'
        result += '  ],\n'
        return result

class I18N:
    '''This adds some more translated strings'''
    def __init__(self, locale):
        self.locale = locale

    def dump(self):
        result = ' "strings": {\n'
        result += '    "help": "' \
            + _("Review a set of words. Each word is shown with a voice a text and an image. When done you are suggested an exercice in which, given the voice, you must find the right word. In the configuration you can select the language you want to learn.") \
            + '"\n'
        result += '  },\n'
        return result

try:
    filename = sys.argv[1]
except:
    print "Missing filename (resource/lang/words.xml)"
    sys.exit(1)

try:
    locale = sys.argv[2]
except:
    print "Missing locale"
    sys.exit(1)


# Load the proper locale catalog
try:
    t = gettext.translation('gcompris', '/usr/local/share/locale', languages=[locale])
    t.install()
except:
    if locale == 'en':
        t = gettext.NullTranslations()
    else:
         print "ERROR, could not load translation"
         sys.exit(1)

_ = t.ugettext

l = LangLib(filename, locale)
sys.stdout.write(l.dump().encode('utf-8'))


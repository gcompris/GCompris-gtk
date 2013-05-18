#  gcompris - langLib.py
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
# lang activity.

from gcompris import gcompris_gettext as _
import gcompris.utils
import xml.dom.minidom

def isNode(e, name):
    return e.nodeType == e.ELEMENT_NODE and e.localName == name


class Triplet:
    def __init__(self, elem, translations):
        self.description = None
        self.descriptionTranslated = None
        self.image = None
        self.voice = None
        self.type = None
        self.valid = True
        self.parse(elem, translations)

    def parse(self, elem, translations):
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
                    self.voice = e.firstChild.nodeValue
                else:
                    self.valid = False
            elif isNode(e, "type"):
                self.type = e.firstChild.nodeValue if e.firstChild else None

        if self.valid:
            if not gcompris.utils.find_file_absolute(self.image):
                self.valid = False
            elif not gcompris.utils.find_file_absolute(self.voice):
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
        print "    Triplet "+ self.description + " / " \
            + str(self.image) + " / " + str(self.voice) \
            + " / " + str(self.type)

class Lesson:

    def __init__(self, lesson):
        self.name = None
        self.description = None
        self.triplets = []
        if lesson:
            self.name = lesson.name
            self.description = lesson.description

    def getTriplets(self):
        return self.triplets

    def dump(self):
        print "  Lesson "+ self.name + " / " + str(self.description)
        for triplet in self.triplets:
            triplet.dump()


class LessonCreator:

    MAX_TRIPLETS = 12

    def __init__(self, elem, translations):
        self.lessons = []
        self.parse(elem, translations)

    def parse(self, elem, translations):
        lesson = Lesson(None)
        for e in elem.childNodes:
            if isNode(e, "name"):
                lesson.name = e.firstChild.nodeValue
            elif isNode(e, "description"):
                lesson.description = e.firstChild.nodeValue if e.firstChild else None
            elif isNode(e, "Triplet"):
                triplet = Triplet(e, translations)
                if triplet.isValid():
                    if len(lesson.triplets) < self.MAX_TRIPLETS:
                        lesson.triplets.append( triplet )
                    else:
                        self.lessons.append(lesson)
                        lesson = Lesson(lesson)

                        
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
    def __init__(self, elem, translations):
        self.name = None
        self.description = None
        self.lessons = []
        self.parse(elem, translations)

    def parse(self, elem, translations):
        for e in elem.childNodes:
            if isNode(e, "name"):
                self.name = e.firstChild.nodeValue if e.firstChild else None
            elif isNode(e, "description"):
                self.description = e.firstChild.nodeValue if e.firstChild else None
            elif isNode(e, "Lesson"):
                lessonCreator = LessonCreator(e, translations)
                for lesson in lessonCreator.getLessons():
                    self.lessons.append( lesson )

    def getLessons(self):
        return self.lessons

    def dump(self):
        print "Chapter "+ self.name + " / " + str(self.description)
        for lesson in self.lessons:
            lesson.dump()

class Chapters:
    def __init__(self, doc, translations):
        self.chapters = {}
        self.parse(doc, translations)

    def parse(self, doc, translations):
        for elem in doc:
            chapter = Chapter(elem, translations)
            self.chapters[chapter.name] = chapter

    def getChapters(self):
        return self.chapters

    def dump(self):
        print "Dump"
        for k, chapter in self.chapters.iteritems():
            chapter.dump()


class LangLib:
    def __init__(self, fileName):
        self.translationsFile = gcompris.utils.find_file_absolute("voices/$LOCALE/words/content.txt")
        self.translations = {}

        if self.translationsFile:
            for line in open(self.translationsFile, 'r'):
                lineSplitted = line.rstrip('\r\n').split(":")
                if line[0] == '#' or len(lineSplitted) < 2: continue
                self.translations[lineSplitted[0]] = lineSplitted[1]

        doc = xml.dom.minidom.parse(fileName)
        self.chapters = Chapters( doc.getElementsByTagName('Chapter'), self.translations )

    def dump(self):
        self.chapters.dump()

    def getChapters(self):
        return self.chapters

    def getLesson(self, chapterName, lessonIndex):
        try:
            return self.chapters.getChapters()[chapterName].getLessons()[lessonIndex]
        except:
            return None

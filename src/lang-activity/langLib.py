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

import gcompris.utils
import xml.dom.minidom

def isNode(e, name):
    return e.nodeType == e.ELEMENT_NODE and e.localName == name


class Triplet:
    def __init__(self, elem):
        self.description = None
        self.image = None
        self.voice = None
        self.type = None
        self.valid = True
        self.parse(elem)

    def parse(self, elem):
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

    def isValid(self):
        return self.valid

    def dump(self):
        print "    Triplet "+ self.description + " / " \
            + str(self.image) + " / " + str(self.voice) \
            + " / " + str(self.type)

class Lesson:
    def __init__(self, elem):
        self.name = None
        self.description = None
        self.triplets = []
        self.parse(elem)

    def parse(self, elem):
        for e in elem.childNodes:
            if isNode(e, "name"):
                self.name = e.firstChild.nodeValue
            elif isNode(e, "description"):
                self.description = e.firstChild.nodeValue if e.firstChild else None
            elif isNode(e, "Triplet"):
                triplet = Triplet(e)
                if triplet.isValid():
                    self.triplets.append( triplet )

    def getTriplets(self):
        return self.triplets

    def dump(self):
        print "  Lesson "+ self.name + " / " + str(self.description)
        for triplet in self.triplets:
            triplet.dump()


class Chapter:
    def __init__(self, elem):
        self.name = None
        self.description = None
        self.lessons = []
        self.parse(elem)

    def parse(self, elem):
        for e in elem.childNodes:
            if isNode(e, "name"):
                self.name = e.firstChild.nodeValue if e.firstChild else None
            elif isNode(e, "description"):
                self.description = e.firstChild.nodeValue if e.firstChild else None
            elif isNode(e, "Lesson"):
                self.lessons.append( Lesson(e) )

    def getLessons(self):
        return self.lessons

    def dump(self):
        print "Chapter "+ self.name + " / " + str(self.description)
        for lesson in self.lessons:
            lesson.dump()

class Chapters:
    def __init__(self, doc):
        self.chapters = {}
        self.parse(doc)

    def parse(self, doc):
        for elem in doc:
            chapter = Chapter(elem)
            self.chapters[chapter.name] = chapter

    def getChapters(self):
        return self.chapters

    def dump(self):
        print "Dump"
        for chapter in self.chapters:
            chapter.dump()


class LangLib:
    def __init__(self, fileName):
        doc = xml.dom.minidom.parse(fileName)
        self.chapters = Chapters( doc.getElementsByTagName('Chapter') )

    def dump(self):
        self.chapters.dump()

    def getChapters(self):
        return self.chapters

    def getLesson(self, chapterName, lessonIndex):
        return self.chapters.getChapters()[chapterName].getLessons()[lessonIndex]

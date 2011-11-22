#  gcompris - findit.py
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
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.sound
import goocanvas
import pango
import random

from gcompris import gcompris_gettext as _
from langLib import *

class Findit:
    """An exercice that given a lesson asks the children to find"""
    """the good anwer from a source text, a target text or a voice"""

    def __init__(self, lang, parentitem, lesson):
        self.lang = lang
        self.lesson = lesson
        self.triplets = list(lesson.getTriplets())
        random.shuffle(self.triplets)
        self.rootitem = goocanvas.Group( parent = parentitem )
        self.gameroot = None
        self.currentIndex = 0
        self.tripletToFind = None

    def start(self):

        if self.currentIndex >= len(self.triplets):
            self.stop()
            self.lang.next_level()
            return

        self.gameroot = goocanvas.Group( parent = self.rootitem )

        self.tripletToFind = self.triplets[self.currentIndex]
        self.lang.playVoice(self.tripletToFind)
        self.currentIndex += 1
        # Display the triplet to find
        goocanvas.Text(
            parent = self.gameroot,
            x = gcompris.BOARD_WIDTH / 2,
            y = gcompris.BOARD_HEIGHT / 4,
            fill_color = "black",
            font = gcompris.skin.get_font("gcompris/subtitle"),
            text = self.tripletToFind.description,
            anchor = gtk.ANCHOR_CENTER,
            alignment = pango.ALIGN_CENTER,
            width = 300
            )

        # Propose some triplet including the good one
        triplets2 = list(self.lesson.getTriplets())
        triplets2.remove(self.tripletToFind)
        random.shuffle(triplets2)
        numberOfItem = 3
        y = gcompris.BOARD_HEIGHT / 2
        triplets2 = triplets2[:numberOfItem-1]
        triplets2.insert(random.randint(0, numberOfItem-1),
                         self.tripletToFind)
        for i in range(0, numberOfItem):
            item = goocanvas.Text(
                parent = self.gameroot,
                x = gcompris.BOARD_WIDTH / 2,
                y = y,
                fill_color = "black",
                font = gcompris.skin.get_font("gcompris/subtitle"),
                text = triplets2[i].description,
                anchor = gtk.ANCHOR_CENTER,
                alignment = pango.ALIGN_CENTER,
                width = 300
                )
            item.connect("button_press_event", self.ok, triplets2[i])
            y += 30


    def stop(self):
        self.rootitem.remove()

    def ok(self, event, target, item, triplet):
        if self.tripletToFind == triplet:
            self.gameroot.remove()
            self.start()
        else:
            self.triplets.append(self.tripletToFind)


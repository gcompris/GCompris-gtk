#  gcompris - langFindit.py
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
# lang activity exercice.
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

class SpotTarget:
  """Display a triplet"""

  def __init__(self, parentitem, x, y, triplet, callback, mode):
      rootitem = goocanvas.Group( parent = parentitem )
      self.width = 380
      self.height = 100
      # The background
      fill_color_off = 0x33333366L
      fill_color_on  = 0x99999966L
      itembg = \
          goocanvas.Rect( parent = rootitem,
                          x = x,
                          y = y,
                          width = self.width,
                          height = self.height,
                          radius_x = 5,
                          radius_y = 5,
                          stroke_color_rgba = 0x666666FFL,
                          fill_color_rgba = fill_color_off,
                          line_width = 2.0 )
      itembg.connect("button_press_event", callback, triplet)
      itembg.connect("enter_notify_event",
                   (lambda s,e,t,i: i.set_properties(fill_color_rgba = fill_color_on)),
                   itembg )
      itembg.connect("leave_notify_event",
                   (lambda s,e,t,i: i.set_properties(fill_color_rgba = fill_color_off)),
                   itembg )

      # The text description
      textx = 120
      if mode & Findit.WITH_TEXT:
        item = goocanvas.Text(
          parent = rootitem,
          x = x + textx,
          y = y + 10,
          fill_color = "black",
          font = gcompris.skin.get_font("gcompris/subtitle"),
          text = triplet.descriptionTranslated,
          anchor = gtk.ANCHOR_NW,
          alignment = pango.ALIGN_LEFT,
          width = self.width - textx - 10
          )
        item.connect("button_press_event", callback, triplet)
        item.connect("enter_notify_event",
                     (lambda s,e,t,i: i.set_properties(fill_color_rgba = fill_color_on)),
                     itembg )
        item.connect("leave_notify_event",
                     (lambda s,e,t,i: i.set_properties(fill_color_rgba = fill_color_off)),
                     itembg )

      # The image
      if triplet.image and (mode & Findit.WITH_IMAGE):
          pixbuf = gcompris.utils.load_pixmap(gcompris.DATA_DIR + "/" +
                                              triplet.image)
          item = goocanvas.Image( parent = rootitem,
                                  pixbuf = pixbuf,
                                  x = x  + 5,
                                  y = y  + 5,
                                  width = 110,
                                  height = 90
                                  )
          item.connect("button_press_event", callback, triplet)
          item.connect("enter_notify_event",
                       (lambda s,e,t,i: i.set_properties(fill_color_rgba = fill_color_on)),
                       itembg )
          item.connect("leave_notify_event",
                       (lambda s,e,t,i: i.set_properties(fill_color_rgba = fill_color_off)),
                       itembg )


class Findit:
    """An exercice that given a lesson asks the children to find"""
    """the good anwer from a source text, a target text or a voice"""

    WITH_QUESTION = 0x4
    WITH_TEXT = 0x2
    WITH_IMAGE = 0x1

    def __init__(self, lang, parentitem, lesson, mode):
        self.lang = lang
        self.lesson = lesson
        self.mode = mode
        self.triplets = list(lesson.getTriplets())
        random.shuffle(self.triplets)
        self.rootitem = goocanvas.Group( parent = parentitem )
        self.gameroot = None
        self.currentIndex = 0
        self.tripletToFind = None

    def start(self):

        if self.currentIndex >= len(self.triplets):
            self.stop()
            self.lang.win()
            return

        self.gameroot = goocanvas.Group( parent = self.rootitem )

        self.tripletToFind = self.triplets[self.currentIndex]
        self.lang.playVoice(self.tripletToFind)
        self.currentIndex += 1
        # Display the triplet to find
        if self.mode & Findit.WITH_QUESTION:
          goocanvas.Text(
            parent = self.gameroot,
            x = gcompris.BOARD_WIDTH / 2,
            y = 100,
            fill_color = "black",
            font = gcompris.skin.get_font("gcompris/subtitle"),
            text = self.tripletToFind.descriptionTranslated,
            anchor = gtk.ANCHOR_CENTER,
            alignment = pango.ALIGN_CENTER,
            width = 300
            )

        # Propose some triplet including the good one
        triplets2 = list(self.lesson.getTriplets())
        triplets2.remove(self.tripletToFind)
        random.shuffle(triplets2)
        numberOfItem = 4
        y_start = 200
        y = y_start
        x = 10
        triplets2 = triplets2[:numberOfItem-1]
        triplets2.insert(random.randint(0, numberOfItem-1),
                         self.tripletToFind)
        for i in range(0, numberOfItem):
            spot = SpotTarget(self.gameroot, x, y,
                              triplets2[i], self.ok,
                              self.mode)
            y += spot.height + 20
            if (i+1) % 2 == 0:
                y = y_start
                x = gcompris.BOARD_WIDTH / 2 + 10

    def stop(self):
        self.rootitem.remove()

    def ok(self, event, target, item, triplet):
        if self.tripletToFind == triplet:
            self.gameroot.remove()
            self.start()
        else:
            self.triplets.append(self.tripletToFind)
            self.lang.loose()

    def repeat(self):
      self.lang.playVoice(self.tripletToFind)
  
    def key_press(self, keyval, commit_str, preedit_str):
      pass

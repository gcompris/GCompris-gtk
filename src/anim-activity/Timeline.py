#  gcompris - anim : timeline management
#
# Copyright (C) 2008 Bruno Coudoin
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

import gobject
import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.sound
import gtk
import gtk.gdk

class Timeline:

    def __init__(self, rootitem, drawing_area):
        self.rootitem = goocanvas.Group(parent = rootitem)
        self.drawing_area = drawing_area
        self.running = False

        self.previous_selection = None
        self.selected = None

    def hide(self):
        self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE

    def show(self):
        self.rootitem.props.visibility = goocanvas.ITEM_VISIBLE

    def getTime(self):
        if self.selected:
            self.selected.get_data("time")

    # Display the timeline selector
    def draw(self):

        self.selected_color = 0x3030AAFFL
        self.default_color = 0xFFFFFFFFL

        # Timeline coord
        x1 = self.drawing_area[0]
        x2 = self.drawing_area[2]
        y  = self.drawing_area[3] + 5
        w = 20
        h = 30


        i = x1
        t = 0
        while i + w < x2:

            item = goocanvas.Rect(
                parent = self.rootitem,
                x = i,
                y = y,
                width = w,
                height = h,
                fill_color_rgba = self.default_color,
                stroke_color_rgba = 0x101080FFL,
                line_width=1.0)
            item.set_data("time", i)

            if not self.selected:
                self.selected = item

            item.connect("button_press_event",
                         self.timeline_item_event,
                         t)

            i += w + 1
            t += 1

    #
    def timeline_item_event(self, item, target, event, time):

        if self.previous_selection:
            self.previous_selection.set_properties(fill_color_rgba = self.default_color)

        item.set_properties(fill_color_rgba = self.selected_color)
        self.previous_selection = item
        self.selected = item


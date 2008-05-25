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

    def __init__(self, anim):
        self.anim = anim
        self.rootitem = goocanvas.Group(parent = anim.rootitem)
        self.drawing_area = anim.drawing_area
        self.running = False

        self.selected = None
        self.timelinelist = []
        self.current_time = 0

    def hide(self):
        self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE

    def show(self):
        self.rootitem.props.visibility = goocanvas.ITEM_VISIBLE

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
            item.set_data("time", t)

            if not self.selected:
                self.selected = item

            self.timelinelist.append(item)
            item.connect("button_press_event",
                         self.timeline_item_event)

            i += w + 1
            t += 1


        # Select the first item in the timeline
        self.current_time = 0
        self.select_it(self.selected)

    # Return the current selected time
    def get_time(self):
        return self.current_time

    def set_time(self, time):
        self.select_it(item, self.timelinelist[time])

    def next(self):
        self.current_time += 1
        if self.current_time >= len(self.timelinelist):
            self.current_time = 0
        self.select_it(self.timelinelist[self.current_time])


    def previous(self):
        self.current_time -= 1
        if self.current_time < 0:
            self.current_time = len(self.timelinelist) - 1
        self.select_it(self.timelinelist[self.current_time])


    def select_it(self, item):
        # Disable previous selection
        if self.selected:
            self.selected.set_properties(fill_color_rgba = self.default_color)

        item.set_properties(fill_color_rgba = self.selected_color)
        self.selected = item
        self.current_time = item.get_data("time")

        # Let anim knows there is a new time set
        self.anim.refresh(self.get_time())

    #
    def timeline_item_event(self, item, target, event):

        self.select_it(item)



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

# This is the timeline of the animation activity.

class Timeline:

    def __init__(self, anim):
        self.anim = anim
        self.rootitem = goocanvas.Group(parent = anim.rootitem)
        self.drawing_area = anim.drawing_area
        self.running = False

        self.zoom = 1
        self.selected = None
        self.timelinelist = []
        self.current_time = 0
        self.lastmark = -1

    def hide(self):
        self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE

    def show(self):
        self.rootitem.props.visibility = goocanvas.ITEM_VISIBLE

    # Display the timeline selector
    def draw(self):

        self.selected_color = 0x3030AAFFL
        self.default_color = 0xFFFFFFFFL
        self.default_stroke = 0x101080FFL
        self.marked_stroke = 0xC01010FFL

        # Timeline coord
        x1 = self.drawing_area[0]
        x2 = self.drawing_area[2]
        y  = self.drawing_area[3] + 5

        # Our timeline repesentation respects the drawing area ratio
        # If we could display a thumbnail of the current frame in each
        # time zone, and we could scroll the time zone.
        #self.zoom = (( self.drawing_area[2] - self.drawing_area[0] ) /
        #             ( self.drawing_area[3] - self.drawing_area[1] ))
        # w = h * self.zoom
        w = 14
        h = 27


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
                stroke_color_rgba = self.default_stroke,
                line_width=1.0,
                radius_x=3.0,
                radius_y=3.0)
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
        self.lastmark_it(self.timelinelist[t-1])

    # Return the current selected time
    def get_time(self):
        return self.current_time

    def set_time(self, time):
        self.select_it(self.timelinelist[time])

    def next(self):
        self.current_time += 1
        if self.current_time >= min(len(self.timelinelist),
                                    self.lastmark + 1):
            self.current_time = 0
        self.select_it(self.timelinelist[self.current_time])


    def previous(self):
        self.current_time -= 1
        if self.current_time < 0:
            self.current_time = min(len(self.timelinelist) - 1,
                                    self.lastmark)

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

    def lastmark_it(self, item):
        # Unmark previous mark
        if self.lastmark >= 0:
            marked_item = self.timelinelist[self.lastmark]
            marked_item.set_properties(stroke_color_rgba = self.default_stroke)

        item.set_properties(stroke_color_rgba = self.marked_stroke)
        self.lastmark = item.get_data("time")

    def get_lastmark(self):
        return self.lastmark

    def set_lastmark(self, lastmark):
        self.lastmark_it(self.timelinelist[lastmark])

    #
    def timeline_item_event(self, item, target, event):

        if event.button == 1:
            self.select_it(item)
        else:
            self.lastmark_it(item)


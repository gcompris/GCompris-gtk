#  gcompris - anim : color management
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

# This is the color selector of the animation activity.

class Color:

    # We keep the current color here
    stroke = 0x101010FFL
    fill   = 0xAA55CCFFL

    def __init__(self, rootitem, drawing_area):
        self.rootitem = goocanvas.Group(parent = rootitem)
        self.root_coloritem = goocanvas.Group(parent = rootitem)
        self.drawing_area = drawing_area
        self.running = False
        # Part of UI: colors buttons
        # COLOR SELECTION
        # RGBA unsigned long. A is always FF.
        # keep in mind if you change that to change the svg export: it does not pass A.
        self.colors = [ 0x000000FFL, 0x202020FFL, 0x404040FFL, 0x505050FFL,
                        0x815a38FFL, 0xb57c51FFL, 0xe5a370FFL, 0xfcc69cFFL,
                        0xb20c0cFFL, 0xea2c2cFFL, 0xf26363FFL, 0xf7a3a3FFL,
                        0xff6600FFL, 0xff8a3dFFL, 0xfcaf7bFFL, 0xf4c8abFFL,
                        0x9b8904FFL, 0xd3bc10FFL, 0xf4dd2cFFL, 0xfcee85FFL,
                        0x255b0cFFL, 0x38930eFFL, 0x56d11dFFL, 0x8fe268FFL,
                        0x142f9bFFL, 0x2d52e5FFL, 0x667eddFFL, 0xa6b4eaFFL,
                        0x328989FFL, 0x37b2b2FFL, 0x3ae0e0FFL, 0x96e0e0FFL,
                        0x831891FFL, 0xc741d8FFL, 0xde81eaFFL, 0xeecdf2FFL,
                        0x666666FFL, 0x838384FFL, 0xc4c4c4FFL, 0xffffffFFL
                        ]
        self.previous_color = 0L

    # Display the color selector
    def draw(self):

        # Color selector buttons coord
        x = 60
        y = 420
        w = 25
        h = 20

        self.fillcolor_tool = goocanvas.Rect(
            parent = self.rootitem,
            x = x + w/2,
            y = y + h/2,
            width = w,
            height = h,
            fill_color_rgba = self.fill,
            stroke_color_rgba = 0xFFFFFFFFL,
            line_width=1.0,
            )

        self.strokecolor_tool = goocanvas.Rect(
            parent = self.rootitem,
            x = x,
            y = y,
            width = w,
            height = h,
            fill_color_rgba = self.stroke,
            stroke_color_rgba = 0xFFFFFFFFL,
            line_width=1.0,
            )

        self.fillcolor_tool.connect("button_press_event",
                                    self.displaycolor_item_event,
                                    True)
        self.strokecolor_tool.connect("button_press_event",
                                      self.displaycolor_item_event,
                                      False)

        pixmap = gcompris.utils.load_pixmap("anim/color-selector.png")

        x = (self.drawing_area[2] - self.drawing_area[0]
             - pixmap.get_width())/2 + self.drawing_area[0]

        color_pixmap_height = pixmap.get_height()

        y = gcompris.BOARD_HEIGHT - color_pixmap_height - 30

        c = 0

        self.root_coloritem.props.visibility = goocanvas.ITEM_INVISIBLE


        goocanvas.Image(
            parent = self.root_coloritem,
            pixbuf = pixmap,
            x=x,
            y=y,
            )

        for i in range(0,10):
            x1=x+26+i*56

            for j in range(0,4):
                c = i*4 +j
                item = \
                    goocanvas.Rect(
                        parent = self.root_coloritem,
                        x = x1 + 26*(j%2),
                        y = y + 8 + (color_pixmap_height/2 -6)*(j/2),
                        width = 24,
                        height = color_pixmap_height/2 - 8,
                        fill_color_rgba = self.colors[c],
                        stroke_color_rgba = 0x07A3E0FFL
                        )

                item.connect("button_press_event",
                             self.color_item_event, self.colors[c])

    # Fill Color event: display the color selector
    def displaycolor_item_event(self, item, target, event, fill):
        if self.running:
            return

        if self.previous_color:
            self.root_coloritem.props.visibility = goocanvas.ITEM_INVISIBLE
            self.root_coloritem.lower(None)
            if fill:
                self.fill = self.previous_color
            else:
                self.stroke = self.previous_color
            self.previous_color = 0L

        else:

            self.root_coloritem.props.visibility = goocanvas.ITEM_VISIBLE
            self.root_coloritem.raise_(None)

            if fill:
                self.previous_color = self.fill
                self.fill = 0L
            else:
                self.previous_color = self.stroke
                self.stroke = 0L

    # Color event
    def color_item_event(self, item, target, event, color):
        if self.running:
            return

        if event.type == gtk.gdk.BUTTON_PRESS:
            gcompris.sound.play_ogg("sounds/drip.wav")
            self.previous_color = 0L

            if self.stroke == 0L:
                self.stroke = color
                self.strokecolor_tool.set_properties(fill_color_rgba = color)
            else:
                self.fill = color
                self.fillcolor_tool.set_properties(fill_color_rgba = color)

            self.root_coloritem.props.visibility = goocanvas.ITEM_INVISIBLE
            self.root_coloritem.lower(None)


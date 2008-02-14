#  gcompris - module.py
#
# Copyright (C) 2005 Bruno Coudoin and Yves Combe
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# This is the base Class for all Administration modules.

import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
from gcompris import gcompris_gettext as _

class Module:
    """GCompris Administrative Module"""

    def __init__(self, canvas, module_name, module_label):
        self.module_name = module_name
        self.module_label = module_label
        self.canvas = canvas

        self.selector_color = 0xC5D2DDFFL
        self.outline_color  = 0XD8B09AFFL
        self.module_panel_ofset = 3

    # Return the position it must have in the administration menu
    # The smaller number is the highest.
    # By default, return 999 to let the module be low
    def position(self):
        return 999

    def init(self, index, select_area, callback):
        height = 80
        gap = 35
        x   = select_area[0] + (select_area[2] - select_area[0]) / 2
        y1  = select_area[1] + height * index + 2
        y2  = select_area[1] + height * (index + 1) + 1

        # Create our rootitem. We put each canvas item in it so at the end we
        # only have to kill it. The canvas deletes all the items it contains automaticaly.

        self.root_select_item = goocanvas.Group(
            parent = self.canvas,
            )

        self.select_item = goocanvas.Rect(
            parent = self.root_select_item,
            x = select_area[0]+2,
            y = y1,
            width = select_area[2]-2 - select_area[0]+2,
            height = y2 - y1,
            fill_color="white",
            stroke_color="white",
            line_width = 1.0
            )
        self.select_item.connect("button_press_event", callback, self)

        y1 += 30
        item = goocanvas.Image(
            parent = self.root_select_item,
            pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("config_" +
                                                                            self.module_name +
                                                                            ".png")),
            x = x,
            y = y1,
            )
        item.connect("button_press_event", callback, self)

        y1 += gap

        item = goocanvas.Text(
            parent = self.root_select_item,
            text=_(self.module_label),
            font=gcompris.skin.get_font("gcompris/tiny"),
            x = x,
            y = y1,
            fill_color="black"
            )
        item.connect("button_press_event", callback, self)


    def get_module_name(self):
        return self.module_name

    def start(self):
        self.select_item.set_properties(
            fill_color_rgba=self.selector_color,
            stroke_color_rgba=self.outline_color
            )

    def stop(self):
        self.select_item.set_properties(
            fill_color="white",
            stroke_color="white"
            )


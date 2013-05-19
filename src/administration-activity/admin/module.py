#  gcompris - module.py
#
# Copyright (C) 2005, 2008 Bruno Coudoin and Yves Combe
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

# This is the base Class for all Administration modules.

import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
import gettext
from gcompris import gcompris_gettext as _
from gettext import dgettext as D_

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
        height = 90
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
            width = select_area[2] - select_area[0] - 4,
            height = y2 - y1,
            fill_color="white",
            stroke_color="white",
            line_width = 1.0
            )
        self.select_item.connect("button_press_event", callback, self)

        y1 += 5

        itemimg = goocanvas.Svg(
            parent = self.root_select_item,
            svg_handle = gcompris.utils.load_svg("administration/config_" +
                                                self.module_name +
                                                 ".svgz")
            )

        bounds = itemimg.get_bounds()
        centered_x = x - (bounds.x2 - bounds.x1)/2
        itemimg.translate(centered_x, y1);
        itemimg.connect("button_press_event", callback, self)
        gcompris.utils.item_focus_init(itemimg, None)

        y1 += bounds.y2 - bounds.y1 + 3

        item = goocanvas.Text(
            parent = self.root_select_item,
            text=D_(gcompris.GETTEXT_ADMIN,self.module_label),
            font = gcompris.skin.get_font("gcompris/board/minuscule"),
            x = centered_x,
            y = y1,
            fill_color="black"
            )
        bounds = item.get_bounds()
        centered_x = x - (bounds.x2 - bounds.x1)/2
        item.props.x = centered_x
        item.connect("button_press_event", callback, self)
        gcompris.utils.item_focus_init(item, itemimg)

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


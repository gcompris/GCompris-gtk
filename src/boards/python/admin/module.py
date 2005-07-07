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

import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
from gettext import gettext as _

class Module:
    """GCompris Administrative Module"""

    def __init__(self, canvas, module_name, module_label):
        self.module_name = module_name
        self.module_label = module_label
        self.canvas = canvas

        self.selector_color = 0xC5D2DDFFL
        self.outline_color  = 0XD8B09AFFL


    # Return the position it must have in the administration menu
    # The smaller number is the highest.
    # By default, return 999 to let the module be low
    def position(self):
        return 999
    
    def init(self, index, select_area, callback):
        print("Gcompris_administration init panel.")

        height = 80
        gap = 35
        x   = select_area[0] + (select_area[2] - select_area[0]) / 2
        y1  = select_area[1] + height * index + 2
        y2  = select_area[1] + height * (index + 1) + 1

        # Create our rootitem. We put each canvas item in it so at the end we
        # only have to kill it. The canvas deletes all the items it contains automaticaly.

        self.root_select_item = self.canvas.add(
            gnome.canvas.CanvasGroup,
            x=0.0,
            y=0.0
            )

        self.select_item = self.root_select_item.add(
            gnome.canvas.CanvasRect,
            x1=select_area[0]+2,
            y1=y1,
            x2=select_area[2]-2,
            y2=y2,
            fill_color="white",
            outline_color="white",
            width_units=1.0
            )
        self.select_item.connect("event", callback, self)

        y1 += 30
        print "config_" + self.module_name + ".png"
        print gcompris.skin.image_to_skin("config_" +
                                                                            self.module_name +
                                                                            ".png")
        item = self.root_select_item.add(
            gnome.canvas.CanvasPixbuf,
            pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("config_" +
                                                                            self.module_name +
                                                                            ".png")),
            x = x,
            y = y1,
            anchor=gtk.ANCHOR_CENTER,
            )
        item.connect("event", callback, self)

        y1 += gap
    
        item = self.root_select_item.add (
            gnome.canvas.CanvasText,
            text=_(self.module_label),
            font=gcompris.skin.get_font("gcompris/content"),
            x = x,
            y = y1,
            fill_color="black"
            )
        item.connect("event", callback, self)


    def get_module_name(self):
        return self.module_name
    
    def start(self):
        self.select_item.set(
            fill_color_rgba=self.selector_color,
            outline_color_rgba=self.outline_color
            )

    def stop(self):
        self.select_item.set(
            fill_color="white",
            outline_color="white"
            )


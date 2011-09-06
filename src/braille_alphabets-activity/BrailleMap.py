#  gcompris - BrailleMap.py
#
# Copyright (C) 2011 Srishti Sethi
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
# This class display a braille char to a given location
# The char may be static of dynamic. It maintains the value
# of the dots and the char it represents in real time.
#
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas
import pango
from BrailleChar import *
from gcompris import gcompris_gettext as _

SYMBOL = ['+', '-', '*', '/', '#']
CELL_WIDTH = 30

COLOR_ON = 0xFF0000FFL
COLOR_OFF = 0X00000000L
CIRCLE_FILL = "white"
CIRCLE_STROKE = "black"

class BrailleMap:
  """Braille Map"""
  def __init__(self, rootitem, move_back_callback):

      rootitem = goocanvas.Group(parent = rootitem)
      item = \
          goocanvas.Rect(parent = rootitem,
                         x = 30,
                         y = 20,
                         width = 740,
                         height = 460,
                         stroke_color_rgba = 0x222222CCL,
                         fill_color_rgba = 0x999999EEL,
                         radius_x = 5.0,
                         radius_y = 5.0,
                         line_width = 2.0)
      item.connect("button_press_event", move_back_callback)

      # Place alphabets & numbers in array format
      for index, letter in enumerate(string.ascii_uppercase[:10]):
          BrailleChar(rootitem, index*(CELL_WIDTH+40)+60,
                              40, 38, letter ,COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                              CIRCLE_STROKE,True ,False ,True , None)
      for index, letter in enumerate(string.ascii_uppercase[10:20]):
          BrailleChar(rootitem, index*(CELL_WIDTH+40)+60,
                              130, 38, letter ,COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                              CIRCLE_STROKE,True ,False ,True , None)
      # Put the W at the end to align the braille codes properly
      for index, letter in enumerate(['U', 'V', 'X', 'Y', 'Z']):
          BrailleChar(rootitem, index*(CELL_WIDTH+40)+60,
                              220, 38, letter ,COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                              CIRCLE_STROKE,True ,False ,True , None)
      BrailleChar(rootitem, (index + 2) *(CELL_WIDTH+40)+60,
                  220, 38, 'W' ,COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                  CIRCLE_STROKE,True ,False ,True , None)

      # The number line (Keep it aligned with the ASCII in the Braille sense)
      for index in range(0, 9):
          BrailleChar(rootitem, index *(CELL_WIDTH + 40)+60,
                      310, 38, index + 1,COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                      CIRCLE_STROKE, True ,False ,True , None)
      BrailleChar(rootitem, 9 *(CELL_WIDTH + 40)+60,
                  310, 38, 0 ,COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                  CIRCLE_STROKE, True ,False ,True , None)

      # The math operators +-*/
      for index, value in enumerate( SYMBOL ):
        BrailleChar(rootitem,index * (CELL_WIDTH + 40) + 60,
                    400 , 38, SYMBOL[index], COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                    CIRCLE_STROKE,True ,False ,True , None)

      # Move back item
      item = goocanvas.Image(parent = rootitem,
                             pixbuf = gcompris.utils.load_pixmap("braille_alphabets/back.svg"),
                             x = 430,
                             y = 400,
                             tooltip = _("Back to the activity")
                             )
      gcompris.utils.item_focus_init(item, None)
      item.connect("button_press_event", move_back_callback)

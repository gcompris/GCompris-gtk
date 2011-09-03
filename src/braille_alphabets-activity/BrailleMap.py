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

SYMBOL = ['+', '_', '*', '/', '#']
CELL_WIDTH = 30

class BrailleMap:
  """Braille Map"""
  def __init__(self, rootitem, color_on, color_off, circle_fill, circle_stroke):

      # Place alphabets & numbers in array format
      for index, letter in enumerate(string.ascii_uppercase[:10]):
          BrailleChar(rootitem, index*(CELL_WIDTH+40)+60,
                              40, 38, letter ,color_on, color_off, circle_fill,
                              circle_stroke,True ,False ,True , None)
      for index, letter in enumerate(string.ascii_uppercase[10:20]):
          BrailleChar(rootitem, index*(CELL_WIDTH+40)+60,
                              130, 38, letter ,color_on, color_off, circle_fill,
                              circle_stroke,True ,False ,True , None)
      for index, letter in enumerate(string.ascii_uppercase[20:26]):
          BrailleChar(rootitem, index*(CELL_WIDTH+40)+60,
                              220, 38, letter ,color_on, color_off, circle_fill,
                              circle_stroke,True ,False ,True , None)

      # The number line (Keep it aligned with the ASCII in the Braille sense)
      for index in range(0, 9):
          BrailleChar(rootitem, index *(CELL_WIDTH + 40)+60,
                      310, 38, index + 1,color_on, color_off, circle_fill,
                      circle_stroke, True ,False ,True , None)
      BrailleChar(rootitem, 9 *(CELL_WIDTH + 40)+60,
                  310, 38, 0 ,color_on, color_off, circle_fill,
                  circle_stroke, True ,False ,True , None)

      # The math operators +-*/
      for index, value in enumerate( SYMBOL ):
        BrailleChar(rootitem,index * (CELL_WIDTH + 40) + 60,
                    400 , 38,SYMBOL[index], color_on, color_off, circle_fill,
                    circle_stroke,True ,False ,True , None)


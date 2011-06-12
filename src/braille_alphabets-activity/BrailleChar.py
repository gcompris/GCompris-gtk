#  gcompris - BrailleChar.py
#
# Copyright (C) 2011 xxxx
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
import gcompris.sound
import string
from gcompris import gcompris_gettext as _

BRAILLE_LETTERS = {
    "A": [1], "B": [1, 2], "C": [1, 4], "D": [1, 4, 5], "E": [1, 5],
    "F": [1, 2, 4], "G": [1, 2, 4, 5], "H": [1, 2, 5], "I": [2, 4],
    "J": [2, 4, 5], "K": [1, 3], "L": [1, 2, 3], "M": [1, 3, 4],
    "N": [1, 3, 4, 5], "O": [1, 3, 5], "P": [1, 2, 3, 4], "Q": [1, 2, 3, 4, 5],
    "R": [1, 2, 3, 5], "S": [2, 3, 4], "T": [2, 3, 4, 5], "U": [1, 3, 6],
    "V": [1, 2, 3, 6], "W": [2, 4, 5, 6], "X": [1, 3, 4, 6], "Y": [1, 3, 4, 5, 6],
    "Z": [1, 3, 5, 6], 1: [2],2 :[2,3], 3 : [2,5], 4: [2,5,6],5 : [2,6],
    6 : [2,3,5],7 : [2,3,5,6],8 : [2,3,6], 9 : [3,5],0 :[3,5,6]
}

class BrailleChar:
  """Braille Char"""
  def __init__(self, rootitem,
               x, y, width, letter,DOT_ON,
               DOT_OFF,fill,stroke,display_letter, clickable,
               rectangle,callback):

    self.letter = letter
    self.callback = callback
    self.display_letter = display_letter
    self.clickable = clickable
    self.DOT_ON = DOT_ON
    self.DOT_OFF = DOT_OFF
    self.fill = fill
    self.stroke = stroke
    self.rectangle = rectangle

    height = width * 1.33
    cell_radius = (width / 7.5)
    self.rootitem = goocanvas.Group(parent=rootitem)
    if(letter == '' or rectangle == False):
        """no rect"""
    else :
        self.item = goocanvas.Rect(parent=self.rootitem,
                          x=x,
                          y=y,
                          width=width,
                          height=height,
                          stroke_color="blue",
                          fill_color="#DfDfDf",
                          line_width=2.0)


    self.text = goocanvas.Text(parent=self.rootitem,
                               x=x + (width / 2.0),
                               y=y + height + 15,
                               text=str(letter),
                               fill_color="blue",
                               alignment=pango.ALIGN_CENTER,
                               anchor = gtk.ANCHOR_CENTER,
                               font = 'Sans BOLD')
    if not display_letter:
        self.text.props.visibility = goocanvas.ITEM_INVISIBLE

    dot = 1
    self.dot_items = []
    for u in range(2):
        for v in range(3):
            cell = goocanvas.Ellipse(parent=self.rootitem,
                                     center_x=x + width / 3.0 * ( u + 1 ),
                                     center_y=y + height / 4.0 * ( v + 1 ),
                                     radius_x=cell_radius,
                                     radius_y=cell_radius,
                                     stroke_color=self.stroke,
                                     fill_color=self.fill,
                                     line_width=width/25)
            # To fill the circles in lower board with red color
            if (clickable == True):
                cell.connect("button_press_event", self.dot_event)
                gcompris.utils.item_focus_init(cell, None)

            if isinstance(letter,int):
                fillings = BRAILLE_LETTERS.get(letter)
            else :
                fillings = BRAILLE_LETTERS.get(letter.upper())

            if fillings == None:
                """only braille cell"""
            elif dot in fillings:
                cell.set_property("fill_color_rgba", self.DOT_ON)
            else :
                cell.set_property("fill_color_rgba", self.DOT_OFF)

            self.dot_items.append(cell)
            dot += 1

  def get_letter(self):
      """Return the letter represented by this braille item"""
      return self.letter

  def calculate_char(self):
      """Calculate the represented char"""
      cells = []

      # Create the dot list
      for l in range(6):
          if(self.dot_items[l].get_property("fill_color_rgba") == self.DOT_ON):
              cells.append(l+1)

      self.letter = ''
      for k,v in BRAILLE_LETTERS.items():
          if v == cells:
              self.letter = k

      if isinstance(self.letter,int):
          self.text.set_property("text",self.letter)
      else :
          self.text.set_property("text", str.upper(self.letter))

      if self.callback:
          self.callback(self.letter)


  def dot_event(self, event, target, item):
      """A dot has been clicked, change its state and calculate our new letter value"""
      if target.get_property("fill_color_rgba") == self.DOT_ON:
          target.set_property("fill_color_rgba", self.DOT_OFF)
      else:
          target.set_property("fill_color_rgba", self.DOT_ON)
      self.calculate_char()
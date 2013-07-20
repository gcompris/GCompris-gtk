#  gcompris - BrailleChar.py
#
# Copyright (C) 2011 Bruno Coudoin and Srishti Sethi
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
import string

# For ASCII each letter, this represent the active dots in Braille.
BRAILLE_LETTERS = {
    "A": [1], "B": [1, 2], "C": [1, 4], "D": [1, 4, 5], "E": [1, 5],
    "F": [1, 2, 4], "G": [1, 2, 4, 5], "H": [1, 2, 5], "I": [2, 4],
    "J": [2, 4, 5], "K": [1, 3], "L": [1, 2, 3], "M": [1, 3, 4],
    "N": [1, 3, 4, 5], "O": [1, 3, 5], "P": [1, 2, 3, 4], "Q": [1, 2, 3, 4, 5],
    "R": [1, 2, 3, 5], "S": [2, 3, 4], "T": [2, 3, 4, 5], "U": [1, 3, 6],
    "V": [1, 2, 3, 6], "W": [2, 4, 5, 6], "X": [1, 3, 4, 6], "Y": [1, 3, 4, 5, 6],
    "Z": [1, 3, 5, 6], "+" : [3,4,6], "-": [3,6], "*" : [1,6], "/" : [3,4],
    "#" : [3,4,5,6],1: [1],2 :[1, 2], 3 : [1, 4], 4: [1, 4, 5], 5 : [1, 5],
    6 : [1, 2, 4], 7 : [1, 2, 4, 5], 8 : [1, 2, 5], 9 : [2, 4], 0 :[3, 5, 6]
}

class BrailleChar:
  """Braille Char"""
  def __init__(self, rootitem,
               x, y, width, letter,
               dot_on, dot_off,
               fill_color, stroke_color,
               display_letter, clickable,
               rectangle, callback, braille_letter = "alphabet"):

    self.letter = letter
    self.callback = callback
    self.display_letter = display_letter
    self.clickable = clickable
    self.dot_on = dot_on
    self.dot_off = dot_off
    self.fill_color = fill_color
    self.stroke_color = stroke_color
    self.rectangle = rectangle
    self.braille_letter = braille_letter

    height = width * 1.33
    self.rootitem = goocanvas.Group(parent = rootitem)
    if(rectangle):
        self.item = goocanvas.Rect(parent = self.rootitem,
                                   x = x,
                                   y = y,
                                   width = width,
                                   height = height,
                                   stroke_color = self.stroke_color,
                                   fill_color = self.fill_color,
                                   line_width=2.0)


    self.text = goocanvas.Text(parent = self.rootitem,
                               x = x + (width / 2.0),
                               y = y + height + 15,
                               text = str(letter),
                               fill_color = self.stroke_color,
                               alignment = pango.ALIGN_CENTER,
                               anchor = gtk.ANCHOR_CENTER,
                               font = gcompris.skin.get_font("gcompris/board/medium bold"))
    if not display_letter:
        self.text.props.visibility = goocanvas.ITEM_INVISIBLE

    dot = 1
    self.dot_items = []
    self.cell_array = []
    cell_radius = (width / 6.3)
    for u in range(2):
        for v in range(3):
            xx = x + width / 3.0 * ( u + 1 ) - width / 20
            yy = y + height / 4.0 * ( v + 1 ) - height / 20
            if u:
              xx = x + width / 3.0 * ( u + 1 ) + width / 20
            if v == 1:
              yy = y + height / 4.0 * ( v + 1 )
            if v == 2:
              yy = y + height / 4.0 * ( v + 1 ) + height / 20


            self.cell = \
                goocanvas.Ellipse(parent = self.rootitem,
                                  center_x = xx,
                                  center_y = yy,
                                  radius_x = cell_radius,
                                  radius_y = cell_radius,
                                  stroke_color = self.stroke_color,
                                  fill_color = self.fill_color,
                                  line_width = width/25)
            self.cell_array.append(self.cell)

            # To fill the circles in lower board with red color
            if (clickable == True):
                self.cell.connect("button_press_event", self.dot_event)
                gcompris.utils.item_focus_init(self.cell, None)

            if isinstance(letter,int):
                fillings = BRAILLE_LETTERS.get(letter)
            else :
                fillings = BRAILLE_LETTERS.get(letter.upper())

            if fillings == None:
                """only braille self.cell"""
            elif dot in fillings:
                self.cell.set_property("fill_color_rgba", self.dot_on)
            else :
                self.cell.set_property("fill_color_rgba", self.dot_off)

            self.dot_items.append(self.cell)
            dot += 1

  def get_letter(self):
      """Return the letter represented by this braille item"""
      return self.letter

  def calculate_char(self):
      """Calculate the represented char"""
      cells = []

      # Create the dot list
      for l in range(6):
          if(self.dot_items[l].get_property("fill_color_rgba") == self.dot_on):
              cells.append(l+1)

      self.letter = ''
      for k,v in BRAILLE_LETTERS.items():
          if (self.braille_letter == "alphabet") and (v == cells):
              if isinstance(k, basestring):
                  self.letter = k
                  self.text.set_property("text", str.upper(self.letter))
          if (self.braille_letter == "number") and (v == cells):
              if isinstance(k, int):
                  self.letter = k
                  self.text.set_property("text",self.letter)

      if self.callback:
          self.callback(self.letter)


  def dot_event(self, event, target, item):
      """A dot has been clicked, change its state and calculate our new letter value"""
      if target.get_property("fill_color_rgba") == self.dot_on:
          target.set_property("fill_color_rgba", self.dot_off)
      else:
          target.set_property("fill_color_rgba", self.dot_on)
      self.calculate_char()

  #### This function has been added specially for BrailleLotto Activity
  #### to fix the focus issue on ticket numbers
  def ticket_focus(self, rectangle, number_cross_function, counter):
      for index in range(6):
          gcompris.utils.item_focus_init(self.cell_array[index], rectangle)
          self.cell_array[index].connect("button_press_event", number_cross_function, counter)

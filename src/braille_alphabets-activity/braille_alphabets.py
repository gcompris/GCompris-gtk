#  gcompris - braille_alphabets.py
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
# braille_alphabets activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import goocanvas
import pango
import gcompris.sound
import string
import random
from gcompris import gcompris_gettext as _
from BrailleChar import *
from BrailleMap import *

CELL_WIDTH = 30
COLOR_ON = 0xFF0000FFL
COLOR_OFF = 0X00000000L
CIRCLE_FILL = "white"
CIRCLE_STROKE = "black"


class Gcompris_braille_alphabets:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    #defining the number of levels in activity
    self.counter = 0
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level = 1
    self.gcomprisBoard.maxlevel = 5
    self.gcomprisBoard.sublevel = 1
    self.gcomprisBoard.number_of_sublevel=1

    #Boolean variable decaration
    self.mapActive = False

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_set_level(self.gcomprisBoard)

    pixmap = gcompris.utils.load_svg("braille_alphabets/target.svg")
    gcompris.bar_set_repeat_icon(pixmap)
    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT_ICON)
    gcompris.bar_location(20, -1, 0.6)
    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.

    self.rootitem = goocanvas.Group(parent=
                                   self.gcomprisBoard.canvas.get_root_item())
    self.board_upper(self.gcomprisBoard.level)

    # The root item for the help
    self.map_rootitem = \
        goocanvas.Group( parent = self.gcomprisBoard.canvas.get_root_item() )
    BrailleMap(self.map_rootitem, self.move_back)
    self.map_rootitem.props.visibility = goocanvas.ITEM_INVISIBLE


  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()
    self.map_rootitem.remove()

  def ok(self):
    print("learnbraille ok.")

  def repeat(self):
      if(self.mapActive):
          self.map_rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
          self.mapActive = False
      else :
          self.map_rootitem.props.visibility = goocanvas.ITEM_VISIBLE
          self.mapActive = True

  def move_back(self,event,target,item):
    self.map_rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.mapActive = False


  def config(self):
    pass

  def key_press(self, keyval, commit_str, preedit_str):
    pass

  def pause(self,pause):
    if(pause == 0):
        self.counter +=1
        if (self.counter == self.sublevel):
            self.increment_level()
        self.end()
        self.start()

  def set_level(self,level):
    gcompris.sound.play_ogg("sounds/receive.wav")
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()

  def increment_level(self):
    self.counter = 0
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
        self.gcomprisBoard.sublevel=1
        self.gcomprisBoard.level += 1
        if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
            self.gcomprisBoard.level = 1


  def board_upper(self, level):
    if(level == 1):
        gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "braille_alphabets/braille_tux.svgz")
        goocanvas.Text(parent=self.rootitem,
                                 x = 385,
                                 y = 100,
                                 fill_color = "black",
                                 font = gcompris.skin.get_font("gcompris/title"),
                                 anchor = gtk.ANCHOR_CENTER,
                                 text = _("Braille : Unlocking the Code"))
        # Braille Intro
        text = _("The Braille system is a method that is used by blind people to read and write.")
        # Braille Description
        text += "\n" + \
            _("Each Braille character, or cell, is made up of six dot positions, arranged in "
              "a rectangle containing two columns of three dots each. As seen on the left, each dot "
              "is referenced by a number from 1 to 6.")
        goocanvas.Text(parent=self.rootitem,
                                 x = 490,
                                 y = 280,
                                 fill_color = "black",
                                 font = gcompris.skin.get_font("gcompris/subtitle"),
                                 width = 395,
                                 anchor = gtk.ANCHOR_CENTER,
                                 text = text)


        # TUX svghandle
        svghandle = gcompris.utils.load_svg("braille_alphabets/braille_tux.svgz")
        self.tuxitem = goocanvas.Svg(
                                     parent = self.rootitem,
                                     svg_handle = svghandle,
                                     svg_id = "#TUX-5",
                                     tooltip = _("I am braille TUX")
                                     )
        self.tuxitem.connect("button_press_event", self.next_level)
        gcompris.utils.item_focus_init(self.tuxitem, None)

        goocanvas.Text(parent = self.rootitem,
                        x = 445,
                        y = 475,
                        fill_color ="black",
                        font = "Sans 10",
                        anchor= gtk.ANCHOR_CENTER,
                        width = 355,
                        text = _("When you are ready, click on "
                        "me and try reproducing Braille characters."))
    elif(level == 2):
        chars = ['A','B','C','D','E','F','G','H','I','J']
        self.sublevel = len(chars)
        if self.counter == 0:
          self.chars_shuffled = list(chars)
          random.shuffle( self.chars_shuffled )
        self.board_tile( chars )
        self.random_letter = self.chars_shuffled[self.counter]
        self.braille_cell(level)

    elif(level == 3) :
        chars = ['K','L','M','N','O','P','Q','R','S','T']
        self.sublevel = len(chars)
        if self.counter == 0:
          self.chars_shuffled = list(chars)
          random.shuffle(self.chars_shuffled)
        self.board_tile( chars )
        self.random_letter = self.chars_shuffled[self.counter]
        self.braille_cell(level)

    elif(level == 4):
        chars = ['U','V','X','Y','Z','W']
        self.sublevel = len(chars)
        if self.counter == 0:
          self.chars_shuffled = list(chars)
          random.shuffle(self.chars_shuffled)
        self.board_tile( chars )
        self.random_letter = self.chars_shuffled[self.counter]
        self.braille_cell(level)

    elif(level == 5):
        chars = [0,1,2,3,4,5,6,7,8,9]
        self.sublevel = len(chars)
        if self.counter == 0:
          self.chars_shuffled = list(chars)
          random.shuffle(self.chars_shuffled)
        self.board_number()
        self.random_letter = self.chars_shuffled[self.counter]
        self.braille_letter = "number"
        self.braille_cell(level)


  def next_level(self,event,target,item):
      self.increment_level()
      self.end()
      self.start()

  def board_tile(self, chars):
      for i, letter in enumerate( chars ):
          tile = BrailleChar(self.rootitem, i*(CELL_WIDTH+40)+60,
                              60, 50, letter ,COLOR_ON ,COLOR_OFF ,CIRCLE_FILL,
                              CIRCLE_STROKE, True ,False ,True, None)

  def board_number(self):
      for letter in range(0, 10):
          tile = BrailleChar(self.rootitem, letter *(CELL_WIDTH+40)+60,
                             60, 50, letter ,COLOR_ON ,COLOR_OFF ,CIRCLE_FILL,
                             CIRCLE_STROKE, True,False ,True, None)

  def display_letter(self,letter):
      goocanvas.Text(parent=self.rootitem,
                                 x=690,
                                 y=330,
                                 fill_color="black",
                                 font="Sans 78",
                                 anchor=gtk.ANCHOR_CENTER,
                                 text=str(letter))

  def braille_cell(self, level):
      # Translators : Do not translate the token {letter}
      message = _("Click on the dots in braille cell area to produce the "
                  "letter {letter}.").format(letter = self.random_letter)

      self.letter = "alphabet"
      if (level == 3):
        message += "\n" + _("Look at the char map and observe the first and second"
                            " line how they are similar.")
      elif (level == 4):
        message += "\n" + _("Take care, the 'W' letter was added afterwards.")
      elif (level == 5):
        message += "\n" + _("This is easy, numbers are the same as letters from A to J.")
        self.letter = "number"

      gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "braille_alphabets/mosaic.svgz")

      goocanvas.Text(parent = self.rootitem,
                     x = 400,
                     y = 200,
                     text = message,
                     fill_color = "black",
                     font = 'SANS 14',
                     width = 780,
                     anchor = gtk.ANCHOR_CENTER,
                     alignment = pango.ALIGN_CENTER)

      goocanvas.Text(parent=self.rootitem,
                      x=160.0, y=250.0,
                     text=_("Braille Cell"),
                     fill_color="black",
                     font='Sans BOLD')

      BrailleChar(self.rootitem, 150, 270, 120, '',
                  COLOR_ON ,COLOR_OFF, CIRCLE_FILL, CIRCLE_STROKE,
                  False, True, False, callback = self.letter_change,
                  braille_letter = self.letter)

      for i in range(2):
          for j in range(3):
                  goocanvas.Text(parent = self.rootitem,
                                 text = ( str(j + 1 + i * 3) ),
                                 font = 'Sans 20',
                                 fill_color = "black",
                                 x = i * 120 + 140,
                                 y = j * 45 + 290)

      # OK Button
      ok = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#OK",
                         tooltip = _("Click to confirm your selection of dots")
                         )
      ok.translate( -165,-155)

      ok.connect("button_press_event", self.ok_event)
      gcompris.utils.item_focus_init(ok, None)

  def ok_event(self,item,target,event):
      if(self.random_letter == self.correct_letter):
          self.display_letter(self.correct_letter)
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.SMILEY)
      else :
          gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.SMILEY)


  def letter_change(self,letter):
      self.correct_letter = letter

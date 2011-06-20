#  gcompris - rhymes_riddles.py
#
# Copyright (C) 2003, 2008 Bruno Coudoin
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
# rhymes_riddles activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import goocanvas
import pango
import random
import string

from BrailleChar import *
from gcompris import gcompris_gettext as _

#Constants Declaration
COLOR_ON = 0XCC3399L
COLOR_OFF = 0X00000000L
CIRCLE_FILL = "#CC3399"
CELL_WIDTH = 30

RHYMES_RIDDLES_LIST_LEVEL_ONE = [
      ["What goes up white and comes down \n" "yellow and white?","egg"],
      ["What rises and waves all day?","flag"],
      ["What has two wheels and speeds up \n " "hills for a hike?","bike"],
      ["What hides in shoes and crosses \n" "the street?","feet"],
      ["What falls from the sky without \n" "hurting your brain?","rain"]
      ]

RHYMES_RIDDLES_LIST_LEVEL_TWO = [
  ["This is a word which rhymes with cat, It \n" "grows on your head because it's a :","hat"],
  ["I'm useful for journey when you are going\n"" far, I need lots of petrol because I'm a :","car"],
  ["This is a word which rhymes with up.\n""You can drink out of me because I'm a :","cup"],
  ["This is a word which rhymes with bake,\n""I'm nice to eat because I'm a : ","cake"],
  ["This is a word which rhymes with spoon,\n""I shine at night because I'm a :","moon"]
  ]

RHYMES_RIDDLES_LIST_LEVEL_THREE =[
  ["A never ending circle,a brightly shiny \n""thing,It's on my fourth finger because its a :","ring"],
  ["The more I dry,The wetter I get","towel"],
  ["What has roots as nobody sees, Is taller than \n""trees,up it goes and yet never grows","mountain"],
  ["You hear my sound you feel me when I \n""move,But see me when you never will","wind"],
  ["You'll find us near ponds or sitting on logs, we\n"" jump and we cloak because we are : ","frogs"]
  ]

RHYMES_RIDDLES_LIST_LEVEL_FOUR = [
  ["What do you get when you put a car and a\n"" pet together?","carpet"],
  ["What can speak in every language , but \n""never went to school ?","echo"],
  ["What's expensive and floats through the room?","perfume"],
  ["What got loose and spoiled the race","shoelace"],
  ["Some fly, some sting, some hide in rugs","bugs"]
  ]


random.shuffle(RHYMES_RIDDLES_LIST_LEVEL_ONE)
random.shuffle(RHYMES_RIDDLES_LIST_LEVEL_TWO)
random.shuffle(RHYMES_RIDDLES_LIST_LEVEL_THREE)
random.shuffle(RHYMES_RIDDLES_LIST_LEVEL_FOUR)

class Gcompris_rhymes_riddles:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    self.gcomprisBoard.maxlevel = 4

    self.counter = 0
    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(300,-1,0.6)
    gcompris.bar_set_level(self.gcomprisBoard)

    # Set a background image
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "rhymes_riddles/riddle.jpg")
    gcompris.set_cursor(gcompris.CURSOR_SELECT)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    self.display_game(self.gcomprisBoard.level)


  def display_game(self,level):
      if(level == 1):
          RHYMES_LIST_NAME = RHYMES_RIDDLES_LIST_LEVEL_ONE
          self.rhymes_rhymes(RHYMES_LIST_NAME)

      elif(level == 2):
          RHYMES_LIST_NAME = RHYMES_RIDDLES_LIST_LEVEL_TWO
          self.rhymes_rhymes(RHYMES_LIST_NAME)

      elif(level == 3):
          RHYMES_LIST_NAME = RHYMES_RIDDLES_LIST_LEVEL_THREE
          self.rhymes_rhymes(RHYMES_LIST_NAME)

      elif(level == 4):
          RHYMES_LIST_NAME = RHYMES_RIDDLES_LIST_LEVEL_FOUR
          self.rhymes_rhymes(RHYMES_LIST_NAME)

  def rhymes_rhymes(self, list_name):
      goocanvas.Text(parent = self.rootitem,
                   x=290.0,
                   y=270.0,
                   text=list_name[self.counter][0],
                   fill_color="black",
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = 'SANS 17'
                   )
      str = list_name[self.counter][1]
      for index in range(str.__len__()):
          BrailleChar(self.rootitem,index*(CELL_WIDTH+20)+100,350,40,str[index],
                      COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                      CIRCLE_FILL, False, False, True,None)
      # the answer area
      self.entry = gtk.Entry()
      self.entry.connect("activate", self.enter_callback,self.entry,list_name)

      goocanvas.Widget(
                       parent = self.rootitem,
                       widget=self.entry,
                       x = 100,
                       y = 300,
                       width = 170,
                       height= 300,
                       anchor=gtk.ANCHOR_NW)

      self.entry.show()
      self.entry.set_text(_("Type your answer here"))


  def enter_callback(self,widget, entry,list_name):
      if(self.entry.get_text() == list_name[self.counter][1]):
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.FLOWER)
      else:
          self.gamewon = 0
          gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.FLOWER)

  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("rhymes_riddles ok.")


  def repeat(self):
    print("rhymes_riddles repeat.")


  def config(self):
    print("rhymes_riddles config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = '%c' % utf8char

  def pause(self, pause):
      self.board_paused = pause
      if(pause == 0) and (self.gamewon == 1):
          self.gamewon = 0
          self.counter +=1
          if(self.counter == 5):
              self.increment_level()
          self.end()
          self.start()

  def set_level(self, level):
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
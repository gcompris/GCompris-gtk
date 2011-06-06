#  gcompris - braille_alphabets.py
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

cell_width = 40
braille_desc = {'intro' : "A system of writing for the blinds that \n"
                "uses characters made of raised dots. \n\n"
                "The braille Cell is composed of 6 dot \n"
                "cells organized in form of two vertical\n"
                "columns with 3 dots {1,2,3} side\n"
                "by side on left and 3 dots side by\n"
                "on right {4,5,6}"}
class Gcompris_braille_alphabets:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    #defining the number of levels in activity
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level = 1
    self.gcomprisBoard.maxlevel=6
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    # Set the buttons we want in the bar
    gcompris.bar_set_level(self.gcomprisBoard)
    gcompris.bar_set(gcompris.BAR_LEVEL)
    pixmap = gcompris.utils.load_svg("target.svg")
    gcompris.bar_set_repeat_icon(pixmap)
    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT_ICON)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.

    self.rootitem = goocanvas.Group(parent=
                                   self.gcomprisBoard.canvas.get_root_item())
    self.board_upper(self.gcomprisBoard.level)

  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()

  def ok(self):
    print("learnbraille ok.")

  def repeat(self):
    self.end()
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
    self.rootitem = goocanvas.Group(parent=
                                   self.gcomprisBoard.canvas.get_root_item())
    #Place alphabets & numbers in array format
    for i, letter in enumerate(string.ascii_uppercase[:10]):
        tile = BrailleChar(self.rootitem, i*(cell_width+30)+60,
                              60, 50, letter ,True ,False,None)
    for i, letter in enumerate(string.ascii_uppercase[10:20]):
        tile = BrailleChar(self.rootitem, i*(cell_width+30)+60,
                              150, 50, letter ,True ,False,None)
    for i, letter in enumerate(string.ascii_uppercase[20:25]):
        tile = BrailleChar(self.rootitem, i*(cell_width+30)+60,
                              250, 50, letter ,True ,False,None)
    for letter in range(0,10):
          tile = BrailleChar(self.rootitem,letter *(cell_width+30)+60,
                             350, 50, letter ,True,False ,None)
    #Move back item
    self.backitem = goocanvas.Image(parent = self.rootitem,
                    pixbuf = gcompris.utils.load_pixmap("back.png"),
                    x = 600,
                    y = 450,
                    tooltip = "Move Back"
                    )
    self.backitem.connect("button_press_event", self.move_back)
    gcompris.utils.item_focus_init(self.backitem, None)

  def move_back(self,event,target,item):
      self.end()
      self.start()

  def config(self):
    print("learnbraille config.")

  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char
    print("Gcompris_learnbraille key press keyval=%i %s" % (keyval, strn))

  def pause(self,pause):
    if(pause == 0):
        self.increment_level()
        self.end()
        self.start()
        return

  def set_level(self,level):
    gcompris.sound.play_ogg("sounds/receive.wav")
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()

  def increment_level(self):
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
        self.gcomprisBoard.sublevel=1
        self.gcomprisBoard.level += 1
        if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
            self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

  def board_upper(self,level):
    if(level == 1):
        gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "braille_tux.svgz")
        goocanvas.Text(parent=self.rootitem,
                                 x=390,
                                 y=100,
                                 fill_color="dark blue",
                                 font="Sans 15",
                                 anchor=gtk.ANCHOR_CENTER,
                                 text="Braille : Unlocking the Code")
        #Braille Description
        goocanvas.Text(parent=self.rootitem,
                                 x=520,
                                 y=260,
                                 fill_color="dark blue",
                                 font="Sans 15",
                                 anchor=gtk.ANCHOR_CENTER,
                                 text=braille_desc.get('intro'))

        #TUX svghandle
        svghandle = gcompris.utils.load_svg("braille_tux.svgz")
        self.tuxitem = goocanvas.Svg(
                                     parent = self.rootitem,
                                     svg_handle = svghandle,
                                     svg_id = "#TUX-5",
                                     tooltip = "I am braille TUX"
                                     )
        self.tuxitem.connect("button_press_event", self.next_level)
        gcompris.utils.item_focus_init(self.tuxitem, None)

        goocanvas.Text(parent = self.rootitem,
                        x = 410,
                        y= 430,
                        fill_color ="black",
                        font = "Sans 10",
                        anchor= gtk.ANCHOR_CENTER,
                        text = "Finished reading braille ! Now click \n"
                        "me and try reproducing braille characters")
    elif(level ==2):
        self.random_letter = string.uppercase[random.randint(0,6)]
        range_lower= 0
        range_upper= 7
        self.braille_cell()
        self.board_tile(range_lower,range_upper)
    elif(level == 3) :
        range_lower= 7
        range_upper= 14
        self.random_letter = string.uppercase[random.randint(7,13)]
        self.braille_cell()
        self.board_tile(range_lower,range_upper)
    elif(level == 4):
        range_lower= 14
        range_upper= 21
        self.random_letter = string.uppercase[random.randint(14,20)]
        self.braille_cell()
        self.board_tile(range_lower,range_upper)
    elif(level == 5):
        range_lower= 21
        range_upper= 26
        self.random_letter = string.uppercase[random.randint(21,25)]
        self.braille_cell()
        self.board_tile(range_lower,range_upper)
    elif(level == 6):
        range_lower= 0
        range_upper= 10
        self.random_letter = random.randint(0,8)
        self.braille_cell()
        self.board_number(range_lower,range_upper)

  def next_level(self,event,target,item):
      self.pause(0)

  def board_tile(self,range_x,range_y):
      for i, letter in enumerate(string.ascii_uppercase[range_x:range_y]):
          tile = BrailleChar(self.rootitem, i*(cell_width+60)+60,
                              80, 50, letter ,True ,False ,None)
  def board_number(self,num_1,num_2):
      for letter in range(num_1,num_2):
          tile = BrailleChar(self.rootitem,letter *(cell_width+30)+60,
                             80, 50, letter ,True,False ,None)

  def display_letter(self,letter):
      goocanvas.Text(parent=self.rootitem,
                                 x=600,
                                 y=350,
                                 fill_color="blue",
                                 font="Sans 78",
                                 anchor=gtk.ANCHOR_CENTER,
                                 text=str(letter))

  def braille_cell(self):
      gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "mosaic.svgz")
      goocanvas.Text(parent = self.rootitem,
                     x = 100,
                     y = 200,
                     text="Click on the dots in braille cell area to produce letter"
                      + ' '+str(self.random_letter),
                     fill_color="blue",
                     font='SANS 15')

      goocanvas.Text(parent=self.rootitem, x=800.0, y=260.0, text="Alphabet",
                   width=500, height=500, fill_color="blue",
                   anchor=gtk.ANCHOR_CENTER, font='Sans BOLD')

      goocanvas.Text(parent=self.rootitem,
                      x=160.0, y=250.0,
                     text=_("Braille Cell"),
                     fill_color="blue",
                     font='Sans BOLD')
      BrailleChar(self.rootitem, 150, 270, 120, '',False,True,callback = self.letter_change)
      for i in range(2):
          for j in range(3):
                  goocanvas.Text(parent=self.rootitem,
                                 text=(str(j + 1 + i * 3)),
                                 font='Sans 20',
                                 fill_color="blue",
                                 x=i * 120 + 140,
                                 y=j * 45 + 290)

  def letter_change(self, letter):
      if(self.random_letter == letter):
          self.display_letter(letter)
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.SMILEY)
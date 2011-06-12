#  gcompris - dicey_dicey.py
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
# dicey_dicey activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.score
import goocanvas
import pango
import random
import gcompris.bonus
from BrailleChar import *
from gcompris import gcompris_gettext as _


on = 0XFFFFFFFFL
off = 0X00000000L
circle_stroke = "black"
circle_fill = "#660066"
symbol_operators = {1 : "plus.svg",2 : "minus.svg",
                    3 : "multiply.svg" ,4 : "divide.svg"}

sign = {1 : "+" , 2 : "_" , 3 : "x" , 4 : "/"}
class Gcompris_dicey_dicey:
  """Empty gcompris python class"""

  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=3
    self.gcomprisBoard.maxlevel = 2


    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(-1,-1,0.8)
    # Set a background image
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    #set background
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "dice_area.svg")

    gcompris.score.start(gcompris.score.STYLE_NOTE, 570, 490,
                         self.gcomprisBoard.number_of_sublevel)
    gcompris.bar_set_level(self.gcomprisBoard)
    gcompris.score.set(self.gcomprisBoard.sublevel)

    #Display title of activity
    goocanvas.Text(parent = self.rootitem,
                   x=400.0,
                   y=100.0,
                   text="Dicey - Dicey",
                   fill_color="black",
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = 'SANS 20'
                   )
    goocanvas.Text(parent = self.rootitem,
                   x=280.0,
                   y=200.0,
                   text="___",
                   anchor = gtk.ANCHOR_CENTER,
                   font='SANS 25')
    goocanvas.Text(parent = self.rootitem,
                   x=520.0,y=200.0,
                   text="=",
                   anchor = gtk.ANCHOR_CENTER,
                   font='SANS 25')
    goocanvas.Text(parent = self.rootitem,
                   x=650.0,y=300.0,
                   text="___",
                   anchor = gtk.ANCHOR_CENTER,
                   font='SANS 25')
    goocanvas.Text(parent = self.rootitem,
                   x=350.0,y=352.0,
                   text="Choose an operator to begin",
                   anchor = gtk.ANCHOR_CENTER,
                   font='SANS 15')
    self.display_function(self.gcomprisBoard.level)

  def display_function(self,level):
      if(level == 1):
          """Taking two random numbers to be displayed on the dice"""
          i =  random.randint(3,4)
          j =  random.randint(1,2)
          for l in range(3):
              item = goocanvas.Image(parent = self.rootitem,
                                 pixbuf = gcompris.utils.load_pixmap("diceyo.svg"),
                                 x = 80 *(3*l+1),
                                 y = 120,
                                 )
              BrailleChar(self.rootitem,120,135,70, i ,on ,off ,circle_fill,
                              circle_stroke,False,False ,False,None)
              BrailleChar(self.rootitem,360,135,70,j,on,off,circle_fill,
                              circle_stroke,False, False, False, None)
              braille_cell = BrailleChar(self.rootitem,600,135,70,
                                 '',on,off,circle_fill,circle_stroke,
                                 False,True,False,callback = self.letter_change)
              #Display operator images
              for operator_number in range(4):
                  self.I = goocanvas.Image(parent = self.rootitem,
                         pixbuf = gcompris.utils.load_pixmap
                         (symbol_operators.get(operator_number+1)),
                         x= 85 * (2 *operator_number + 1),
                         y=380
                         )

                  gcompris.utils.item_focus_init(self.I,None)
                  self.I.connect("button_press_event",self.calculate,operator_number,i,j)

              goocanvas.Text(parent = self.rootitem,
                     x=140.0,y=300.0,
                     text="%i" % i,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 20')
              goocanvas.Text(parent = self.rootitem,
                     x=400.0,y=300.0,
                     text="%i" % j,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 20')
      elif(level == 2):
          print "hi"


  def calculate(self,item,target,event,symbol,num_1,num_2):
      self.end()
      self.start()
      self.T = goocanvas.Text(parent = self.rootitem,
                     x=280.0,
                     y=180.0,
                     text=sign.get(symbol+1),
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')

      if(symbol == 0 ):
          self.result =  num_1 + num_2
          print self.result
      elif(symbol == 1):
          self.result =  num_1 - num_2
          print self.result
      elif(symbol == 2):
          self.result =  num_1 * num_2
          print self.result
      elif(symbol == 3):
          self.result =  num_1 / num_2
          print self.result

  def end(self):

    print "dicey_dicey end"
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("dicey_dicey ok.")


  def repeat(self):
    print("dicey_dicey repeat.")


  def config(self):
    print("dicey_dicey config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_dicey_dicey key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    if(pause == 0):
        self.end()
        self.start()
        self.increment_level()


  def set_level(self, level):
    print("dicey_dicey set level. %i" % level)
    gcompris.sound.play_ogg("sounds/receive.wav")
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()

  def increment_level(self):
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.gcomprisBoard.sublevel += 1
    print "sublevel  %i" % self.gcomprisBoard.sublevel

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
        self.gcomprisBoard.sublevel=1
        self.gcomprisBoard.level += 1
        if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
            self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

  def letter_change(self, letter):
      if (self.result == letter):
          goocanvas.Text(parent = self.rootitem,
                     x=650.0,
                     y=290.0,
                     text=self.result,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.FLOWER)
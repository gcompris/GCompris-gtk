#  gcompris - dicey_dicey.py
#
# Copyright (C) 2003, 2008 Bruno Coudoin | Srishti Sethi
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
from BrailleMap import *
from gcompris import gcompris_gettext as _

#Constants Declaration
COLOR_ON = 0XFFFFFFFFL
COLOR_OFF = 0X00000000L
PLUS_COLOR_ON = 0XFF0000FFL
MINUS_COLOR_ON = 0XFF00FFL
MULT_COLOR_ON = 0X00FFFFL
DIV_COLOR_ON = 0x3D59ABL

CIRCLE_STROKE = "black"
CIRCLE_FILL = "#660066"
SYMBOL_OFF = "#DfDfDf"
CELL_WIDTH = 30
SYMBOL = ['+','_','*','/']
COLOR_LIST = ["red","green","blue","dark green"]

#Declaring list of all questions
#used in different levels
QUESTIONS= [
    [# level 1
     [3,"-",2],[1,"+",1],[2,"-",2]
     ],
    [# level 2
     [2,"+",1],[3,"+",2],[2,"+",2]
     ],
    [#level 3
    [4 ,"+" ,4],[4, "+", 5],[9 ,"-", 3],[9 , "-" ,2]
     ],
    [ #level 4
     [0,"*",1],[1,"+",1],[3,"-",3],[6,"/",3]
    ],
    [# level 5
     [3 , "+" , 2],[5, "-", 2],[4, "*", 1],[8,"/",2]
     ],
     [# level 6
      [4, "+", 5],[9, "-", 2],[3, "*", 2],[8, "/", 1]
      ]
 ]
class Gcompris_dicey_dicey:
  """Empty gcompris python class"""

  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    self.gcomprisBoard.maxlevel = 6

    self.counter = 0

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

    for index in range(6):
        random.shuffle(QUESTIONS[index])
    random.shuffle(SYMBOL)


  def start(self):

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(300,-1,0.8)
    # Set a background image
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    #Boolean variable declaration
    self.mapActive = False

    #REPEAT ICON
    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT_ICON)
    gcompris.bar_location(300,-1,0.6)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())
    gcompris.bar_set_level(self.gcomprisBoard)


    #Display title of activity
    goocanvas.Text(parent = self.rootitem,
                   x=400.0,
                   y=100.0,
                   text=_("Dicey - Dicey"),
                   fill_color="black",
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = 'SANS 20'
                   )
    self.display_function(self.gcomprisBoard.level)

  def display_function(self,level):
      if(level == 1 or level == 2 or level == 3):
          self.dicey_dicey()
          self.calculate(level)
          gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "dicey_dicey/dice.svg")
          ok = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#OK",
                         tooltip = _("Click to confirm your selection of dots")
                         )
          ok.translate(40,-30)
          ok.connect("button_press_event", self.ok_event,level)
          gcompris.utils.item_focus_init(ok, None)
      elif(level == 4 or level == 5 or level == 6):
          self.dicey_dicey()
          self.dicey_dicey1()
          self.calculate(level)


  def calculate(self,level):
      i = QUESTIONS[level - 1][self.counter][0]
      j= QUESTIONS[level - 1][self.counter][2]
      self.random_sign = QUESTIONS[level - 1][self.counter][1]

      #Mathematical calculations
      if(self.random_sign == '+'):
          self.result = i + j
      elif(self.random_sign == '-'):
          self.result = i - j
      elif(self.random_sign == '*'):
          self.result = i * j
      elif(self.random_sign == '/'):
          self.result = i / j


      BrailleChar(self.rootitem, 120, 160, 70 , i, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_STROKE, False, False ,False, None)
      BrailleChar(self.rootitem, 360, 160, 70, j, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_STROKE, False, False, False, None)
      goocanvas.Text(parent = self.rootitem, x=140.0 ,y=330.0,
                     text="%i" % i, anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
      goocanvas.Text(parent = self.rootitem,
                     x=400,y=330.0,
                     text="%i" % j,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
      if(level == 4 or level == 5 or level == 6):
          BrailleChar(self.rootitem, 600, 160, 70, self.result,
                  COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                  CIRCLE_STROKE, False, False, False, None)
          goocanvas.Text(parent = self.rootitem,
                     x=280.0,
                     y=230.0,
                     text='?',
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 30')
          goocanvas.Text(parent = self.rootitem, x=640.0,y=330.0,
                     text="%i" % self.result,anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')

      elif(level == 1 or level == 2 or level == 3):
          goocanvas.Text(parent = self.rootitem,
                     x=280.0,
                     y=230.0,
                     text=self.random_sign,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 30')
          BrailleChar(self.rootitem,600,160,70,'' ,
                      COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                      CIRCLE_STROKE, False, True, False,
                      callback = self.letter_change, braille_letter = "number")


  def dicey_dicey(self):
      #Display dices
      for index in range(3):
          item = goocanvas.Image(parent = self.rootitem,
                                 pixbuf = gcompris.utils.load_pixmap("dicey_dicey/diceyo.svg"),
                                 x = 80 *(3*index+1),
                                 y = 150,
                                 )
      goocanvas.Text(parent = self.rootitem,
                   x=520.0,y=230.0,
                   text="=",
                   anchor = gtk.ANCHOR_CENTER,
                   font='SANS 30')
      goocanvas.Text(parent = self.rootitem,
                   x=655.0,y=323.0,
                   text="___",
                   anchor = gtk.ANCHOR_CENTER,
                       font='SANS 30')


  def dicey_dicey1(self):
      gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "dicey_dicey/dice_area.svg")
      goocanvas.Text(parent = self.rootitem,
                     x=300.0,
                     y=358.0,
                     text = _("Choose an operator to indicate the question mark"),
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 12')

      #Display operator images
      self.cell = []
      for index in range(4):
          Item = goocanvas.Image(parent = self.rootitem,
                         pixbuf = gcompris.utils.load_pixmap("dicey_dicey/cube.svg"),
                         x=48 * (2*index + 5),
                         y=380
                         )
          self.cell.append(Item)

          gcompris.utils.item_focus_init(Item,None)
          if(SYMBOL[index] == '+'):
              COLOR_LIST = PLUS_COLOR_ON
              self.braille_math_symbol(index,COLOR_LIST)
          elif(SYMBOL[index] == '_'):
              COLOR_LIST = MINUS_COLOR_ON
              self.braille_math_symbol(index,COLOR_LIST)
          elif(SYMBOL[index] == '*'):
              COLOR_LIST = MULT_COLOR_ON
              self.braille_math_symbol(index,COLOR_LIST)
          elif(SYMBOL[index] == '/'):
              COLOR_LIST = DIV_COLOR_ON
              self.braille_math_symbol(index,COLOR_LIST)
      for index in range(4):
          self.cell[index].connect("button_press_event",self.symbol_identify,index)

  def braille_math_symbol(self, index, COLOR_LIST):
      BrailleChar(self.rootitem,(index*98)+255,375,50,
              SYMBOL[index],COLOR_LIST,COLOR_OFF,SYMBOL_OFF,CIRCLE_FILL,
              False,False,False,None)

  def symbol_identify(self, event, target, item, index):
      if((SYMBOL[index] == '+' and self.random_sign == '+') or
         (SYMBOL[index] == '_' and self.random_sign == '-') or
          (SYMBOL[index] == '*' and self.random_sign == '*')
        or (SYMBOL[index] == '/' and self.random_sign == '/')):
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.FLOWER)
      else :
          self.gamewon = 0
          gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.FLOWER)

  def ok_event(self, item, target, event, level):
        if(self.result == self.correct_letter):
              goocanvas.Text(parent = self.rootitem,
                     x=650.0,
                     y=330.0,
                     text=self.result,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
              self.gamewon = 1
              gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.FLOWER)
        else :
              self.gamewon = 0
              gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.FLOWER)

  def letter_change(self, letter):
      self.correct_letter = letter


  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("dicey_dicey ok.")


  def repeat(self):
      if(self.mapActive):
          self.end()
          self.start()
          self.mapActive = False
      else :
          self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
          self.rootitem = goocanvas.Group(parent=
                                   self.gcomprisBoard.canvas.get_root_item())
          gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
          map_obj = BrailleMap(self.rootitem, COLOR_ON, COLOR_OFF, CIRCLE_FILL, CIRCLE_STROKE)
          self.mapActive = True

  def config(self):
    print("dicey_dicey config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

  def pause(self, pause):
      self.board_paused = pause
      if(pause == 0) and (self.gamewon == 1):
          self.gamewon = 0
          self.counter +=1
          if ((self.gcomprisBoard.level == 1 or self.gcomprisBoard.level == 2 or
               self.gcomprisBoard.level == 3) and self.counter == 3):
              self.increment_level()
          elif(self.counter == 4):
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

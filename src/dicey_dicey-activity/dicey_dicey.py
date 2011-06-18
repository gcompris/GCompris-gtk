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
PLUS_LIST_LEVEL_ONE_A = [0,1,2]
PLUS_LIST_LEVEL_ONE_B = [0,1,2]
MINUS_LIST_LEVEL_ONE_A = [2,2,2]
MINUS_LIST_LEVEL_ONE_B = [0,1,2]
PLUS_LIST_LEVEL_TWO_A = [3,4,5]
PLUS_LIST_LEVEL_TWO_B = [2,3,4]
MINUS_LIST_LEVEL_TWO_A = [5,5,5]
MINUS_LIST_LEVEL_TWO_B = [3,4,5]
PLUS_LIST_LEVEL_THREE_A = [6,7,8,9]
PLUS_LIST_LEVEL_THREE_B = [6,7,8,9]
MINUS_LIST_LEVEL_THREE_A = [9,9,9]
MINUS_LIST_LEVEL_THREE_B = [6,7,8,9]
MULT_LIST_LEVEL_FOUR_A= [0,1,2]
MULT_LIST_LEVEL_FOUR_B = [0,1,2]
DIV_LIST_LEVEL_FOUR_A = [0,1,2]
DIV_LIST_LEVEL_FOUR_B = [1,2,3]
MULT_LIST_LEVEL_FIVE_A = [3,4,3,4]
MULT_LIST_LEVEL_FIVE_B = [1,2,1,2]
DIV_LIST_LEVEL_FIVE_A = [2,4,6]
DIV_LIST_LEVEL_FIVE_B = [2,2,2]
MULT_LIST_LEVEL_SIX_A = [6,7,8,9]
MULT_LIST_LEVEL_SIX_B = [6,7,8,9]
DIV_LIST_LEVEL_SIX_A = [3,6,9]
DIV_LIST_LEVEL_SIX_B = [3,3,3]


#Shuffling all the lists
random.shuffle(PLUS_LIST_LEVEL_ONE_A)
random.shuffle(PLUS_LIST_LEVEL_ONE_B)
random.shuffle(MINUS_LIST_LEVEL_ONE_B)
random.shuffle(PLUS_LIST_LEVEL_TWO_A)
random.shuffle(PLUS_LIST_LEVEL_TWO_B)
random.shuffle(MINUS_LIST_LEVEL_TWO_A)
random.shuffle(MINUS_LIST_LEVEL_TWO_B)
random.shuffle(PLUS_LIST_LEVEL_THREE_A)
random.shuffle(PLUS_LIST_LEVEL_THREE_B)
random.shuffle(MINUS_LIST_LEVEL_THREE_A)
random.shuffle(MINUS_LIST_LEVEL_THREE_B)
random.shuffle(MULT_LIST_LEVEL_FOUR_A)
random.shuffle(MULT_LIST_LEVEL_FOUR_B)
random.shuffle(DIV_LIST_LEVEL_FOUR_A)
random.shuffle(DIV_LIST_LEVEL_FOUR_B)
random.shuffle(MULT_LIST_LEVEL_FIVE_A)
random.shuffle(MULT_LIST_LEVEL_FIVE_B)
random.shuffle(DIV_LIST_LEVEL_FIVE_A)
random.shuffle(DIV_LIST_LEVEL_FIVE_B)
random.shuffle(MULT_LIST_LEVEL_SIX_A)
random.shuffle(MULT_LIST_LEVEL_SIX_B)
random.shuffle(DIV_LIST_LEVEL_SIX_A)
random.shuffle(DIV_LIST_LEVEL_SIX_B)
random.shuffle(SYMBOL)

x1 = 100
x2 = 300
x3 = 340
x4 = 120
x5 = 360
x6 = 400

SIGN_LIST_ONE = ['+','_','+','_']
random.shuffle(SIGN_LIST_ONE)
SIGN_LIST_TWO = ['+','_','*','/']
random.shuffle(SIGN_LIST_TWO)

class Gcompris_dicey_dicey:
  """Empty gcompris python class"""

  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=2
    self.gcomprisBoard.maxlevel = 6

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

    gcompris.score.start(gcompris.score.STYLE_NOTE, 50, 470,
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
    self.display_function(self.gcomprisBoard.level)

  def display_function(self,level):
      if(level == 1):
          self.random_sign = SIGN_LIST_ONE[self.counter]
          self.dicey_dicey()
          self.calculate(level)

      if(level == 2):
          self.random_sign = SIGN_LIST_ONE[self.counter]
          self.dicey_dicey()
          self.calculate(level)

      if(level == 3):
          self.random_sign = SIGN_LIST_ONE[self.counter]
          if(self.random_sign == '+'):
              self.dicey_dicey1()
          else :
              self.dicey_dicey()
          self.calculate(level)

      if(level == 4):
          self.random_sign = SIGN_LIST_TWO[self.counter]
          self.dicey_dicey()
          self.dicey_dicey2()
          self.calculate(level)

      if(level == 5):
          self.random_sign = SIGN_LIST_TWO[self.counter]
          self.dicey_dicey()
          self.dicey_dicey2()
          self.calculate(level)

      if(level == 6):
          self.random_sign = SIGN_LIST_TWO[self.counter]
          if((self.random_sign == '+') or (self.random_sign == '*')):
              self.dicey_dicey1()
              self.dicey_dicey2()
          else:
              self.dicey_dicey()
              self.dicey_dicey2()
          self.calculate(level)

      if(level == 1 or level == 2 or level == 3):
          gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "dicey_dicey/dice.svg")
          ok = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#OK",
                         tooltip = "Click to confirm your selection of dots"
                         )
          ok.translate(40,-30)
          ok.connect("button_press_event", self.ok_event,level)
          gcompris.utils.item_focus_init(ok, None)

  def calculate(self,level):
      if(self.random_sign == '+'):
          if(level == 1 or level == 4):
              i = PLUS_LIST_LEVEL_ONE_A[self.counter]
              j = PLUS_LIST_LEVEL_ONE_B[self.counter]
          elif(level == 2 or level == 5):
              i = PLUS_LIST_LEVEL_TWO_A[self.counter]
              j = PLUS_LIST_LEVEL_TWO_B[self.counter]
          elif(level == 3 or level == 6):
              i = PLUS_LIST_LEVEL_THREE_A[self.counter]
              j = PLUS_LIST_LEVEL_THREE_B[self.counter]

          self.result = i + j
      elif(self.random_sign == '_'):
          if(level == 1 or level == 4):
              i = MINUS_LIST_LEVEL_ONE_A[self.counter]
              j = MINUS_LIST_LEVEL_ONE_B[self.counter]
          elif(level == 2 or level == 5):
              i = MINUS_LIST_LEVEL_TWO_A[self.counter]
              j = MINUS_LIST_LEVEL_TWO_B[self.counter]
          elif(level == 3 or level == 6):
              i = MINUS_LIST_LEVEL_THREE_A[self.counter]
              j = MINUS_LIST_LEVEL_THREE_B[self.counter]
          self.result = i - j
      elif(self.random_sign == '*'):
          if(level == 4):
              i = MULT_LIST_LEVEL_FOUR_A[self.counter]
              j = MULT_LIST_LEVEL_FOUR_B[self.counter]
          elif(level == 5):
              i = MULT_LIST_LEVEL_FIVE_A[self.counter]
              j = MULT_LIST_LEVEL_FIVE_B[self.counter]
          elif(level == 6):
              i = MULT_LIST_LEVEL_SIX_A[self.counter]
              j = MULT_LIST_LEVEL_SIX_B[self.counter]
          self.result = i * j
      elif(self.random_sign == '/'):
          if(level == 4):
              i = DIV_LIST_LEVEL_FOUR_A[self.counter]
              j = DIV_LIST_LEVEL_FOUR_B[self.counter]
          elif(level == 5):
              i = DIV_LIST_LEVEL_FIVE_A[self.counter]
              j = DIV_LIST_LEVEL_FIVE_B[self.counter]
          elif(level == 6):
              i = DIV_LIST_LEVEL_SIX_A[self.counter]
              j = DIV_LIST_LEVEL_SIX_B[self.counter]
          self.result = i / j

      if(level ==1 or level == 2):
          goocanvas.Text(parent = self.rootitem,
                     x=280.0,
                     y=200.0,
                     text=self.random_sign,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
          BrailleChar(self.rootitem,600,160,70,'' ,
                      COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                      CIRCLE_STROKE, False, True, False,
                      callback = self.letter_change1)
          self.braille_display(x4,x5,x6,i,j)

      if(level == 4 or level == 5):
          self.braille_display(x4,x5,x6,i,j)
          goocanvas.Text(parent = self.rootitem,
                     x=280.0,
                     y=200.0,
                     text='?',
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
          BrailleChar(self.rootitem,600, 160, 70, self.result,
                      COLOR_ON,COLOR_OFF,CIRCLE_FILL,CIRCLE_STROKE,
                      False,False,False,None)
          self.text_display()

      if(level == 3):
          if(self.random_sign == '+'):
              self.braille_display(x1, x2, x3 ,i, j)
              self.a1 = self.result / 10
              self.a2 = self.result % 10
              goocanvas.Text(parent = self.rootitem,
                     x=240.0,
                     y=200.0,
                     text=self.random_sign,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
              braille_cell1 = BrailleChar(self.rootitem,490, 160, 70,
                                 '', COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                                 CIRCLE_STROKE, False, True, False,
                                 callback = self.letter_change1)
              braille_cell2 = BrailleChar(self.rootitem,630, 160, 70,
                                 '', COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                                 CIRCLE_STROKE, False, True, False,
                                 callback = self.letter_change2)
          else :
              self.braille_display(x4, x5, x6, i, j)
              braille_cell = BrailleChar(self.rootitem,600, 160, 70,
                                 '',COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                                 CIRCLE_STROKE, False, True, False,
                                 callback = self.letter_change1)
              goocanvas.Text(parent = self.rootitem,
                     x=280.0,
                     y=200.0,
                     text=self.random_sign,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
      elif(level == 6):
          if(self.random_sign == '+' or self.random_sign == '*'):
                goocanvas.Text(parent = self.rootitem,
                     x=240.0,
                     y=200.0,
                     text='?',
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
                self.braille_display(x1, x2, x3, i, j)
                self.two_cells()
                self.text_display()

          else :
                goocanvas.Text(parent = self.rootitem,
                     x=280.0,
                     y=200.0,
                     text='?',
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
                self.braille_display(x4, x5, x6, i, j)
                self.result_display()
                self.text_display()

  def result_display(self):
      BrailleChar(self.rootitem, 600, 160, 70, self.result,
                  COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                  CIRCLE_STROKE, False, False, False, None)


  def text_display(self):
      goocanvas.Text(parent = self.rootitem, x=640.0,y=330.0,
                     text="%i" % self.result,anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
  def two_cells(self):
      self.a1 = self.result / 10
      self.a2 = self.result % 10
      braille_cell1 = BrailleChar(self.rootitem, 490, 160, 70,
                                 self.a1, COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                                 CIRCLE_STROKE, False, False, False,
                                 callback = self.letter_change1)
      braille_cell2 = BrailleChar(self.rootitem, 630, 160, 70,
                                 self.a2, COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                                 CIRCLE_STROKE, False, False, False,
                                 callback = self.letter_change2)
  def braille_display(self,x4,x5,x6,i,j):
      BrailleChar(self.rootitem, x4, 160, 70 , i, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_STROKE, False, False ,False, None)
      BrailleChar(self.rootitem, x5, 160, 70, j, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_STROKE, False, False, False, None)
      goocanvas.Text(parent = self.rootitem, x=140.0 ,y=330.0,
                     text="%i" % i, anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
      goocanvas.Text(parent = self.rootitem,
                     x=x6,y=330.0,
                     text="%i" % j,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')

  def dicey_dicey(self):
      #Display dices
      for index in range(3):
          item = goocanvas.Image(parent = self.rootitem,
                                 pixbuf = gcompris.utils.load_pixmap("dicey_dicey/diceyo.svg"),
                                 x = 80 *(3*index+1),
                                 y = 150,
                                 )

      goocanvas.Text(parent = self.rootitem,
                   x=280.0,
                   y=230.0,
                   text="___",
                   anchor = gtk.ANCHOR_CENTER,
                   font='SANS 25')
      goocanvas.Text(parent = self.rootitem,
                   x=520.0,y=230.0,
                   text="=",
                   anchor = gtk.ANCHOR_CENTER,
                   font='SANS 25')
      goocanvas.Text(parent = self.rootitem,
                   x=655.0,y=325.0,
                   text="___",
                   anchor = gtk.ANCHOR_CENTER,
                       font='SANS 25')


  def dicey_dicey1(self):
      for index in range(3):
                item1 = goocanvas.Image(parent = self.rootitem,
                                 pixbuf = gcompris.utils.load_pixmap("dicey_dicey/diceyo.svg"),
                                 x = 65 *(3*index+1),
                                 y = 150,
                                 )
      item2 = goocanvas.Image(parent = self.rootitem,
                                    pixbuf = gcompris.utils.load_pixmap("dicey_dicey/diceyo.svg"),
                                    x = 600,
                                    y = 150,
                                    )
      goocanvas.Text(parent = self.rootitem,
                   x=240.0,
                   y=230.0,
                   text="___",
                   anchor = gtk.ANCHOR_CENTER,
                   font='SANS 25')
      goocanvas.Text(parent = self.rootitem,
                   x=430.0,y=230.0,
                   text="=",
                   anchor = gtk.ANCHOR_CENTER,
                   font='SANS 25')
      goocanvas.Text(parent = self.rootitem,
                   x=628.0,y=325.0,
                   text="___",
                   anchor = gtk.ANCHOR_CENTER,
                   font='SANS 25')

  def dicey_dicey2(self):
      gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "dicey_dicey/dice_area.svg")
      goocanvas.Text(parent = self.rootitem,
                     x=300.0,
                     y=358.0,
                     text="Choose an operator to indicate the question mark",
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
         (SYMBOL[index] == '_' and self.random_sign == '_') or
          (SYMBOL[index] == '*' and self.random_sign == '*')
        or (SYMBOL[index] == '/' and self.random_sign == '/')):
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.FLOWER)
      else :
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.FLOWER)

  def ok_event(self, item, target, event, level):
      if(level == 1 or level == 2 or level == 3):
            if(level == 3 and self.random_sign == '+'):
                if((self.a1 == self.correct_letter1) and
                   (self.a2 == self.correct_letter2)):
                  goocanvas.Text(parent = self.rootitem,
                         x = 620,
                         y = 330,
                         text = str(self.correct_letter1) + ''
                          +str(self.correct_letter2),
                         anchor = gtk.ANCHOR_CENTER,
                         font = 'SANS 25'
                         )
                  #If there is a Win
                  self.gamewon = 1
                  gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.FLOWER)

                else :
                  #If there is a loose
                  self.gamewon = 1
                  gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.FLOWER)
            else :
                if(self.result == self.correct_letter1):
                    goocanvas.Text(parent = self.rootitem,
                     x=650.0,
                     y=330.0,
                     text=self.result,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
                    self.gamewon = 1
                    gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.FLOWER)
                else :
                    self.gamewon = 1
                    gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.FLOWER)

  def letter_change1(self, letter):
      self.correct_letter1 = letter

  def letter_change2(self, letter):
      self.correct_letter2 = letter

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

          #Place alphabets & numbers in array format
          for index, letter in enumerate(string.ascii_uppercase[:10]):
              tile = BrailleChar(self.rootitem, index*(CELL_WIDTH+40)+60,
                              60, 38, letter ,COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                              CIRCLE_FILL, True ,False ,True , None)
          for index, letter in enumerate(string.ascii_uppercase[10:20]):
              tile = BrailleChar(self.rootitem, index*(CELL_WIDTH+40)+60,
                              135, 38, letter ,COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                              CIRCLE_FILL, True ,False ,True, None)
          for index, letter in enumerate(string.ascii_uppercase[20:25]):
              tile = BrailleChar(self.rootitem, index*(CELL_WIDTH+40)+60,
                              210, 38, letter ,COLOR_ON ,COLOR_OFF ,CIRCLE_FILL,
                              CIRCLE_FILL, True ,False,True, None)
          BrailleChar(self.rootitem,60, 285, 38, "#",COLOR_ON,COLOR_OFF, CIRCLE_FILL,
                      CIRCLE_FILL, True, False,True, None)
          for index in range(0,10):
              tile = BrailleChar(self.rootitem,(index+1) *(CELL_WIDTH + 33)+60,
                             285, 38, index ,COLOR_ON,COLOR_OFF ,CIRCLE_FILL,
                             CIRCLE_FILL, True ,False ,True, None)
          for index in range(4):
              BrailleChar(self.rootitem,index * (CELL_WIDTH + 40) + 60,
                              360 , 38,SYMBOL[index],COLOR_ON,COLOR_OFF,CIRCLE_FILL
                              ,CIRCLE_FILL,True, False , True, None)

          #Move back item
          self.backitem = goocanvas.Image(parent = self.rootitem,
                    pixbuf = gcompris.utils.load_pixmap("dicey_dicey/back.png"),
                    x = 600,
                    y = 450,
                    tooltip = "Move Back"
                    )
          self.backitem.connect("button_press_event", self.move_back)
          gcompris.utils.item_focus_init(self.backitem, None)

          self.mapActive = True


  def move_back(self,event,target,item):
      self.end()
      self.start()

  def config(self):
    print("dicey_dicey config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_dicey_dicey key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
      self.board_paused = pause
      if(pause == 0) and (self.gamewon == 1):
          self.gamewon = 0
          self.counter +=1
          if (self.counter == 3):
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
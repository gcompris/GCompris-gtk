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
plus_on = 0XFF0000FFL
minus_on = 0XFF00FFL
mult_on = 0X00FFFFL
div_on = 0x3D59ABL

circle_stroke = "black"
circle_fill = "#660066"
symbol_off = "#DfDfDf"
cell_width = 30

symbol = ['+','_','X',"/"]
random.shuffle(symbol)
color = ["red","green","blue","dark green"]

x1 = 100
x2 = 300
x3 = 340
x4 = 120
x5 = 360
x6 = 400
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

    if(self.gcomprisBoard.level == 1 or self.gcomprisBoard.level == 2 or
        self.gcomprisBoard.level == 3):
        sign = ['+','_']
        random.shuffle(sign)
        self.random_sign = sign[self.counter]
    else :
        sign = ['+','_','X','/']
        random.shuffle(sign)
        self.random_sign = sign[self.counter]
    self.display_function(self.gcomprisBoard.level)

  def display_function(self,level):
      if(level == 1):
          """Taking two random numbers to be displayed on the dice"""
          plus_arr_a = [0,1,2]
          plus_arr_b = [0,1,2]
          minus_arr_a = [2]
          minus_arr_b = [0,1,2]
          self.dicey_dicey1()
          self.calculate(plus_arr_a,plus_arr_b,minus_arr_a,minus_arr_b,
                None,None,None,None,level)

      if(level == 2):
          plus_arr_a = [3,4,5]
          plus_arr_b = [3,4]
          minus_arr_a = [5]
          minus_arr_b = [3,4,5]
          self.dicey_dicey1()
          self.calculate(plus_arr_a,plus_arr_b,minus_arr_a,minus_arr_b,
                None,None,None,None,level)

      if(level == 3):
          plus_arr_a = [6,7,8,9]
          plus_arr_b = [6,7,8,9]
          minus_arr_a = [9]
          minus_arr_b = [6,7,8,9]
          if(self.random_sign == '+'):
              self.dicey_dicey2()
          else :
              self.dicey_dicey1()
          self.calculate(plus_arr_a,plus_arr_b,minus_arr_a,minus_arr_b,
                None,None,None,None,level)

      if(level == 4):
          plus_arr_a = [0,1,2]
          plus_arr_b = [0,1,2]
          minus_arr_a = [2]
          minus_arr_b = [0,1,2]
          mult_arr_a = [0,1,2]
          mult_arr_b = [0,1,2]
          div_arr_a = [0,1,2]
          div_arr_b = [1,2]
          self.dicey_dicey1()
          self.dicey_dicey4()
          self.calculate(plus_arr_a,plus_arr_b,minus_arr_a,minus_arr_b,
                mult_arr_a,mult_arr_b,div_arr_a,div_arr_b,level)

      if(level == 5):
          plus_arr_a = [3,4,5]
          plus_arr_b = [3,4]
          minus_arr_a = [5]
          minus_arr_b = [3,4,5]
          mult_arr_a = [3,4]
          mult_arr_b = [1,2]
          div_arr_a = [2,4,6]
          div_arr_b = [2]
          self.dicey_dicey1()
          self.dicey_dicey4()
          self.calculate(plus_arr_a,plus_arr_b,minus_arr_a,minus_arr_b,
                mult_arr_a,mult_arr_b,div_arr_a,div_arr_b,level)

      if(level == 6):
          plus_arr_a = [6,7,8,9]
          plus_arr_b = [6,7,8,9]
          minus_arr_a = [9]
          minus_arr_b = [6,7,8,9]
          mult_arr_a = [6,7,8,9]
          mult_arr_b = [6,7,8,9]
          div_arr_a = [3,6,9]
          div_arr_b = [3]
          if((self.random_sign == '+') or (self.random_sign == 'X')):
              self.dicey_dicey2()
              self.dicey_dicey4()
          else:
              self.dicey_dicey1()
              self.dicey_dicey4()

          self.calculate(plus_arr_a,plus_arr_b,minus_arr_a,minus_arr_b,
                mult_arr_a,mult_arr_b,div_arr_a,div_arr_b,level)

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

  def calculate(self,plus_arr_a,plus_arr_b,minus_arr_a,minus_arr_b,
                mult_arr_a,mult_arr_b,div_arr_a,div_arr_b,level):
      if(self.random_sign == '+'):
              i =  random.choice(plus_arr_a)
              j =  random.choice(plus_arr_b)
              self.result = i + j
      elif(self.random_sign == '_'):
              i = random.choice(minus_arr_a)
              j = random.choice(minus_arr_b)
              self.result = i - j
      elif(self.random_sign == 'X'):
              i = random.choice(mult_arr_a)
              j = random.choice(mult_arr_b)
              self.result = i * j
      elif(self.random_sign == '/'):
              i = random.choice(div_arr_a)
              j = random.choice(div_arr_b)
              self.result = i / j

      if(level ==1 or level == 2):
          goocanvas.Text(parent = self.rootitem,
                     x=280.0,
                     y=200.0,
                     text=self.random_sign,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
          BrailleChar(self.rootitem,600,160,70,
                                 '',on,off,circle_fill,circle_stroke,
                                 False,True,False,callback = self.letter_change1)
          self.braille_display(x4,x5,x6,i,j)

      if(level == 4 or level == 5):
          self.braille_display(x4,x5,x6,i,j)
          goocanvas.Text(parent = self.rootitem,
                     x=280.0,
                     y=200.0,
                     text='?',
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
          BrailleChar(self.rootitem,600,160,70,
                                 self.result,on,off,circle_fill,circle_stroke,
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
              braille_cell1 = BrailleChar(self.rootitem,490,160,70,
                                 '',on,off,circle_fill,circle_stroke,
                                 False,True,False,callback = self.letter_change1)
              braille_cell2 = BrailleChar(self.rootitem,630,160,70,
                                 '',on,off,circle_fill,circle_stroke,
                                 False,True,False,callback = self.letter_change2)
          else :
              self.braille_display(x4, x5, x6, i, j)
              braille_cell = BrailleChar(self.rootitem,600,160,70,
                                 '',on,off,circle_fill,circle_stroke,
                                 False,True,False,callback = self.letter_change1)
              goocanvas.Text(parent = self.rootitem,
                     x=280.0,
                     y=200.0,
                     text=self.random_sign,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
      elif(level == 6):
          if(self.random_sign == '+' or self.random_sign == 'X'):
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
      BrailleChar(self.rootitem,600,160,70,self.result,on,off,circle_fill,
                  circle_stroke,False,False,False,None)


  def text_display(self):
      goocanvas.Text(parent = self.rootitem, x=640.0,y=330.0,
                     text="%i" % self.result,anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
  def two_cells(self):
      self.a1 = self.result / 10
      self.a2 = self.result % 10
      braille_cell1 = BrailleChar(self.rootitem,490,160,70,
                                 self.a1,on,off,circle_fill,circle_stroke,
                                 False,False,False,callback = self.letter_change1)
      braille_cell2 = BrailleChar(self.rootitem,630,160,70,
                                 self.a2,on,off,circle_fill,circle_stroke,
                                 False,False,False,callback = self.letter_change2)
  def braille_display(self,x4,x5,x6,i,j):
      BrailleChar(self.rootitem,x4,160,70, i ,on ,off ,circle_fill,
                              circle_stroke,False,False ,False,None)
      BrailleChar(self.rootitem,x5,160,70,j,on,off,circle_fill,
                              circle_stroke,False, False, False, None)
      goocanvas.Text(parent = self.rootitem, x=140.0,y=330.0,
                     text="%i" % i,anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')
      goocanvas.Text(parent = self.rootitem,
                     x=x6,y=330.0,
                     text="%i" % j,
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 25')

  def dicey_dicey1(self):

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


  def dicey_dicey2(self):
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

  def dicey_dicey4(self):
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
          """goocanvas.Text(parent = self.rootitem,
                     x=(index * 98)+280,
                     y=455,
                     text = symbol[index],
                     fill_color = color[index],
                     anchor = gtk.ANCHOR_CENTER,
                     font='SANS 28')"""
          if(symbol[index] == '+'):
              BrailleChar(self.rootitem,(index*98)+255,375,50,
              '+',plus_on,off,symbol_off,circle_fill,
              False,False,False,None)
          elif(symbol[index] == '_'):
              BrailleChar(self.rootitem,(index*98)+255,375,50,'_',
                minus_on,off,symbol_off,circle_fill,
                False,False,False,None)
          elif(symbol[index] == 'X'):
              BrailleChar(self.rootitem,(index*98)+237,375,50,
              '.',mult_on,off,symbol_off,circle_fill,
              False,False,False,None)
              BrailleChar(self.rootitem,((index*98)+237)+34,375,50,
              'X',mult_on,off,symbol_off,circle_fill,
              False,False,False,None)
          elif(symbol[index] == '/'):
              BrailleChar(self.rootitem,(index*98)+237,375,50,
              '/',div_on,off,symbol_off,circle_fill,
              False,False,False,None)
              BrailleChar(self.rootitem,((index*98)+237)+34,375,50,
              '-',div_on,off,symbol_off,circle_fill,
              False,False,False,None)
      for index in range(4):
          self.cell[index].connect("button_press_event",self.symbol_identify,index)

  def symbol_identify(self,event,target,item,index):
      if((symbol[index] == '+' and self.random_sign == '+') or
         (symbol[index] == '_' and self.random_sign == '_') or
          (symbol[index] == 'X' and self.random_sign == 'X')
        or (symbol[index] == '/' and self.random_sign == '/')):
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.FLOWER)
      else :
          self.gamewon = 0
          gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.FLOWER)

  def ok_event(self,item,target,event,level):
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
                  self.gamewon = 0
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
                    self.gamewon = 0
                    gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.FLOWER)

  def letter_change1(self,letter):
      self.correct_letter1 = letter

  def letter_change2(self,letter):
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
          for index in range(0,10):
              tile = BrailleChar(self.rootitem,index *(cell_width+40)+60,
                             50, 38, index ,on ,off ,circle_fill,
                             circle_fill, True ,False ,True, None)
          for index, letter in enumerate(string.ascii_uppercase[:10]):
              tile = BrailleChar(self.rootitem, index*(cell_width+40)+60,
                              130, 38, letter ,on,off,circle_fill,
                              circle_fill,True ,False,True,None)
          for index, letter in enumerate(string.ascii_uppercase[10:20]):
              tile = BrailleChar(self.rootitem, index*(cell_width+40)+60,
                              210, 38, letter ,on, off, circle_fill,
                              circle_fill, True ,False,True,None)
          for index, letter in enumerate(string.ascii_uppercase[20:25]):
              tile = BrailleChar(self.rootitem, index*(cell_width+40)+60,
                              285, 38, letter ,on ,off ,circle_fill,
                              circle_fill, True ,False,True, None)
          BrailleChar(self.rootitem,60, 360 , 38,
                                 '+',on,off,circle_fill,circle_fill,
                                 True, False , True, None)
          BrailleChar(self.rootitem,130,360,38,
                                 '_',on,off,circle_fill,circle_fill,
                                 True , False , True, None)
          BrailleChar(self.rootitem,210,360,38,
                                 '.',on,off,circle_fill,circle_fill,
                                 False, False ,True,None)
          BrailleChar(self.rootitem,245,360,38,
                                 'X',on,off,circle_fill,circle_fill,
                                 False,False,True , None)
          BrailleChar(self.rootitem,330,360,38,
                                 '/',on,off,circle_fill,circle_fill,
                                 False,False,True,None)
          BrailleChar(self.rootitem,365,360,38,
                                 '-',on,off,circle_fill,circle_fill,
                                 False,False,True,None)
          goocanvas.Text(parent = self.rootitem,
                   x=245.0,
                   y=425.0,
                   text="X",
                   fill_color=circle_fill,
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = 'SANS BOLD'
                   )
          goocanvas.Text(parent = self.rootitem,
                   x=365.0,
                   y=425.0,
                   text="/",
                   fill_color=circle_fill,
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = 'SANS BOLD'
                   )

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
      if(pause == 0):
          self.gamewon = 0
          self.counter +=1
          if (self.counter == 2):
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
#  gcompris - braille_lotto.py
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
# braille_lotto activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.timer
import goocanvas
import random
import pango
from BrailleChar import *
from gcompris import gcompris_gettext as _

#CONSTANTS for generating TICKET A
TICKET_A_a = random.randint(1,25)
TICKET_A_b = random.randint(1,25)
TICKET_A_c = random.randint(26,50)
TICKET_A_d = random.randint(51,75)
TICKET_A_e = random.randint(51,75)
TICKET_A_f = random.randint(76,100)

#CONSTANTS for generating TICKET B
TICKET_B_a = random.randint(1,25)
TICKET_B_b = random.randint(1,25)
TICKET_B_c = random.randint(26,50)
TICKET_B_d = random.randint(51,75)
TICKET_B_e = random.randint(51,75)
TICKET_B_f = random.randint(76,100)

PLAYER_ONE = ['P','L','A','Y','E','R',1]
PLAYER_TWO = ['P','L','A','Y','E','R',2]

COLOR_ON = 0X00FFFFL
COLOR_OFF = 0X00000000L
CIRCLE_FILL = "light green"
CELL_WIDTH = 30

class Gcompris_braille_lotto:
  """Empty gcompris python class"""

  def __init__(self, gcomprisBoard):
    print "braille_lotto init"

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    print "braille_lotto start"

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_set(0)
    gcompris.bar_location(400, -1, 0.8)

    # Set a background image
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    #Boolean variable declaration
    self.mapActive = False
    self.board_paused = 0

    #REPEAT ICON
    gcompris.bar_set(gcompris.BAR_REPEAT_ICON)
    gcompris.bar_location(320,-1,0.8)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    self.lotto_board()

  def lotto_board(self):
    #Lotto Board
    board = goocanvas.Rect(
      parent = self.rootitem,
      x = 40,
      y = 20,
      width = 420,
      height = 270,
      stroke_color = "blue",
      fill_color = "light blue" ,
      line_width=5)

    #Horizontal polylines for lotto board
    for index in range(8):
        p_points = goocanvas.Points([(40.0, 30 * index + 50),(460.0, 30 * index + 50)])
        polyline = goocanvas.Polyline(parent = self.rootitem, points=p_points, stroke_color="blue")

    #Vertical polylines for lotto board
    for index in range(9):
        p_points = goocanvas.Points([(42 * index + 80, 20.0),(42 * index + 80, 290.0)])
        polyline = goocanvas.Polyline(parent = self.rootitem, points=p_points, stroke_color="blue")


    #Displaying ticket for player one
    ticket_one = goocanvas.Rect(
      parent = self.rootitem,
      x = 40,
      y = 310,
      width = 280,
      height = 180,
      stroke_color = "dark green",
      fill_color = "light green" ,
      line_width=10)

    self.rectangle_one = []
    self.rectangle_one_x = []
    self.rectangle_one_y = []
    for i in range(4):
        for j in range(3):
              rect = goocanvas.Rect(
                             parent = self.rootitem,
                             x = 45 + 68 * i,
                             y = 315 + 57 * j,
                             width = 65,
                             height = 54,
                             stroke_color = "dark green",
                             fill_color = "light green" ,
                             line_width=2)
              self.rectangle_one.append(rect)
              self.rectangle_one_x.append(45 + 68 * i)
              self.rectangle_one_y.append(315 + 57 * j)
    for index in range(12):
        gcompris.utils.item_focus_init(self.rectangle_one[index],None)
        self.rectangle_one[index].connect("button_press_event",self.test, index)


    #Displaying ticket for player two
    ticket_two = goocanvas.Rect(
      parent = self.rootitem,
      x = 500,
      y = 310,
      width = 280,
      height = 180,
      stroke_color = "dark green",
      fill_color = "light green" ,
      line_width=10)

    self.rectangle_two = []
    self.rectangle_two_x = []
    self.rectangle_two_y = []
    for i in range(4):
        for j in range(3):
              rect = goocanvas.Rect(
                             parent = self.rootitem,
                             x = 505 + 68 * i,
                             y = 315 + 57 * j,
                             width = 65,
                             height = 54,
                             stroke_color = "dark green",
                             fill_color = "light green" ,
                             line_width=2)
              self.rectangle_two.append(rect)
              self.rectangle_two_x.append(505 + 68 * i)
              self.rectangle_two_y.append(315 + 57 * j)
    for index in range(12):
        gcompris.utils.item_focus_init(self.rectangle_two[index],None)
        self.rectangle_two[index].connect("button_press_event",self.test)

    #Displaying player_one and player_two

    for index in range(7):
        #PLAYER 1
        goocanvas.Text(
                parent = self.rootitem,
                x=340.0,
                y=330 + 20 * index,
                text=str(PLAYER_ONE[index]),
                fill_color="black",
                anchor = gtk.ANCHOR_CENTER,
                alignment = pango.ALIGN_CENTER,
                )
        #PLAYER TWO
        goocanvas.Text(
                parent = self.rootitem,
                x=480.0,
                y=330 + 20 * index,
                text=str(PLAYER_TWO[index]),
                fill_color="black",
                anchor = gtk.ANCHOR_CENTER,
                alignment = pango.ALIGN_CENTER,
                )
    # The Lotto Room Area
    # ---------------
    self.lotto_room_sw = gtk.ScrolledWindow()
    self.lotto_room_sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
    self.lotto_room_sw.set_shadow_type(gtk.SHADOW_ETCHED_OUT)

    w = 100.0
    h = gcompris.BOARD_HEIGHT - 380.0
    y = 20.0 # The upper limit of the text boxes
    x = gcompris.BOARD_WIDTH - w - 20.0

    self.lotto_room_tb = gtk.TextBuffer()
    self.lotto_room_tv = gtk.TextView(self.lotto_room_tb)
    self.lotto_room_tv.set_editable(False)
    self.lotto_room_sw.add(self.lotto_room_tv)
    # save name and selected color in a map
    self.lotto_room_map = {}

    self.lotto_room_tb.set_text("")

    self.lotto_room_tv.set_wrap_mode(gtk.WRAP_CHAR)

    goocanvas.Widget(
      parent = self.rootitem,
      widget=self.lotto_room_sw,
      x=x,
      y=y,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_NW)
    self.lotto_room_tv.show()
    self.lotto_room_sw.show()

    # A label for the room area
    goocanvas.Text(
      parent = self.rootitem,
      text=_("Lotto Room"),
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x=x+(w/2),
      y=y+h+15,
      anchor=gtk.ANCHOR_CENTER,
      )

    # The channel area
    # ---------------
    self.channel = gtk.Entry()

    goocanvas.Widget(
      parent = self.rootitem,
      widget=self.channel,
      x=500,
      y=60,
      width=160,
      height= 10,
      anchor=gtk.ANCHOR_NW)

    self.channel.show()
    self.channel.set_text("Type your channel name in order to start playing")


    # A label for the channel area
    goocanvas.Text(
      parent = self.rootitem,
      text=_("Your Channel"),
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x=560,
      y=30,
      anchor=gtk.ANCHOR_CENTER,
      )

    #Button to display the number to be checked in the ticket
    goocanvas.Image(parent = self.rootitem,
                    pixbuf = gcompris.utils.load_pixmap("braille_lotto/button.jpg"),
                    x = 530,
                    y = 200,
                    )

    #Check number
    goocanvas.Text(
      parent = self.rootitem,
      text=_("Check Number"),
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x=630,
      y=220,
      anchor=gtk.ANCHOR_CENTER,
      )

    goocanvas.Text(
      parent = self.rootitem,
      text= "23",
      x=635,
      y=250,
      font = "SANS 20",
      anchor=gtk.ANCHOR_CENTER,
      )

    #Setting Timer
    gcompris.timer.display(560,100,gcompris.timer.CLOCK,10,self.say)
    gcompris.timer.pause(9)
    gcompris.timer.add(10)

    #Displaying Numbers in Lotto Board
    k = 0
    i = 0
    j = 10
    self.board_array = []
    self.board_array_x = []
    self.board_array_y = []
    while k < 9:
        spacing = 0
        for index in range(i,j):
            goocanvas.Text(
                    parent = self.rootitem,
                    text = (index + 1),
                    font = gcompris.skin.get_font("gcompris/board/medium"),
                    x = spacing * 43 + 55,
                    y = 30 * k + 35,
                    anchor=gtk.ANCHOR_CENTER,
                    )
            #Append number, x and y location to arrays
            self.board_array.append(index + 1)
            self.board_array_x.append(spacing * 43 + 55)
            self.board_array_y.append(30 * k + 35)
            spacing = spacing + 1
        i += 10
        j += 10
        k += 1

    #Place the checked button on number which is being called
    number = 23
    if number == 23:
        goocanvas.Image(parent = self.rootitem,
                    pixbuf = gcompris.utils.load_pixmap("braille_lotto/button_checked.png"),
                    x = self.board_array_x[number- 1] - 10 ,
                    y = self.board_array_y[number- 1]- 10,
                    )

    #Displaying the Braille Code for randomly generated numbers for TICKET_A
    #TICKET_A_a
    #TICKET_A_b
    if (TICKET_A_a < 10) :
        BrailleChar(self.rootitem, 55, 320, 35 , TICKET_A_a, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    if(TICKET_A_b < 10):
        BrailleChar(self.rootitem, 55, 432, 35 , TICKET_A_b, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    if(TICKET_A_a >= 10):
        tens_digit_a = TICKET_A_a / 10
        ones_digit_a = TICKET_A_a % 10
        BrailleChar(self.rootitem, 47, 320, 33 , tens_digit_a, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
        BrailleChar(self.rootitem, 74, 320, 33 , ones_digit_a, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    if(TICKET_A_b >= 10):
        tens_digit_b = TICKET_A_b / 10
        ones_digit_b = TICKET_A_b % 10
        BrailleChar(self.rootitem, 47, 432, 33 , tens_digit_b, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
        BrailleChar(self.rootitem, 72, 432, 33 , ones_digit_b, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    #TICKET_A_c
    tens_digit_b = TICKET_A_c / 10
    ones_digit_b = TICKET_A_c % 10
    BrailleChar(self.rootitem, 115, 377, 33 , tens_digit_b, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    BrailleChar(self.rootitem, 142, 377, 33 , ones_digit_b, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    #TICKET_A_d
    tens_digit_d = TICKET_A_d / 10
    ones_digit_d = TICKET_A_d % 10
    BrailleChar(self.rootitem, 184, 320, 33 , tens_digit_d, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    BrailleChar(self.rootitem, 211, 320, 33 , ones_digit_d, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    #TICKET_A_e
    tens_digit_e = TICKET_A_e / 10
    ones_digit_e = TICKET_A_e % 10
    BrailleChar(self.rootitem, 184, 432, 33 , tens_digit_e, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    BrailleChar(self.rootitem, 211, 432, 33 , ones_digit_e, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    #TICKET_A_f
    tens_digit_f = TICKET_A_f / 10
    ones_digit_f = TICKET_A_f % 10
    BrailleChar(self.rootitem, 252, 377, 33 , tens_digit_f, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    BrailleChar(self.rootitem, 279, 377, 33 , ones_digit_f, COLOR_ON, COLOR_OFF ,
                    CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    #Displaying the Braille Code for randomly generated numbers for TICKET_B
    #TICKET_B_a
    #TICKET_B_b
    if (TICKET_B_a < 10) :
        BrailleChar(self.rootitem, 515, 320, 35 , TICKET_B_a, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    if(TICKET_B_b < 10):
        BrailleChar(self.rootitem, 515, 432, 35 , TICKET_B_b, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    if(TICKET_B_a >= 10):
        tens_digit_a = TICKET_B_a / 10
        ones_digit_a = TICKET_B_a % 10
        BrailleChar(self.rootitem, 508, 320, 33 , tens_digit_a, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
        BrailleChar(self.rootitem, 535, 320, 33 , ones_digit_a, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    if(TICKET_B_b >= 10):
        tens_digit_b = TICKET_B_b / 10
        ones_digit_b = TICKET_B_b % 10
        BrailleChar(self.rootitem, 508, 432, 33 , tens_digit_b, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
        BrailleChar(self.rootitem, 535, 432, 33 , ones_digit_b, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    #TICKET_B_c
    tens_digit_b = TICKET_B_c / 10
    ones_digit_b = TICKET_B_c % 10
    BrailleChar(self.rootitem, 575, 377, 33 , tens_digit_b, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    BrailleChar(self.rootitem, 602, 377, 33 , ones_digit_b, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    #TICKET_B_d
    tens_digit_d = TICKET_B_d / 10
    ones_digit_d = TICKET_B_d % 10
    BrailleChar(self.rootitem, 643, 320, 33 , tens_digit_d, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    BrailleChar(self.rootitem, 670, 320, 33 , ones_digit_d, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    #TICKET_B_e
    tens_digit_e = TICKET_B_e / 10
    ones_digit_e = TICKET_B_e % 10
    BrailleChar(self.rootitem, 643, 432, 33 , tens_digit_e, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    BrailleChar(self.rootitem, 670, 432, 33 , ones_digit_e, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

    #TICKET_B_f
    tens_digit_f = TICKET_B_f / 10
    ones_digit_f = TICKET_B_f % 10
    BrailleChar(self.rootitem, 711, 377, 33 , tens_digit_f, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
    BrailleChar(self.rootitem, 738, 377, 33 , ones_digit_f, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)



  def say(self):
      print "hello"

  def test(self,item, event, target, index):
    #Cross Sign
    goocanvas.Image(parent = self.rootitem,
                    pixbuf = gcompris.utils.load_pixmap("braille_lotto/cross_button.png"),
                    x = self.rectangle_one_x[index]+8 ,
                    y = self.rectangle_one_y[index]+5,
                    )

  def end(self):
    print "braille_lotto end"
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("braille_lotto ok.")


  def repeat(self):
      if(self.mapActive):
          self.end()
          self.start()
          self.mapActive = False
          self.pause(0)
      else :
          self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
          self.rootitem = goocanvas.Group(parent=
                                   self.gcomprisBoard.canvas.get_root_item())
          gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

          #Place alphabets & numbers in array format
          for index, letter in enumerate(string.ascii_uppercase[:10]):
              tile = BrailleChar(self.rootitem, index*(CELL_WIDTH+40)+60,
                              55, 50, letter ,COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                              CIRCLE_FILL, True ,False ,True , None)
          for index, letter in enumerate(string.ascii_uppercase[10:20]):
              tile = BrailleChar(self.rootitem, index*(CELL_WIDTH+40)+60,
                              150, 50, letter ,COLOR_ON, COLOR_OFF, CIRCLE_FILL,
                              CIRCLE_FILL, True ,False ,True, None)
          for index, letter in enumerate(string.ascii_uppercase[20:25]):
              tile = BrailleChar(self.rootitem, index*(CELL_WIDTH+40)+60,
                              250, 50, letter ,COLOR_ON ,COLOR_OFF ,CIRCLE_FILL,
                              CIRCLE_FILL, True ,False,True, None)
          BrailleChar(self.rootitem,60, 350, 50, "#",COLOR_ON,COLOR_OFF, CIRCLE_FILL,
                      CIRCLE_FILL, True, False,True, None)
          for index in range(0,10):
              tile = BrailleChar(self.rootitem,(index+1) *(CELL_WIDTH + 33)+60,
                             350, 50, index ,COLOR_ON,COLOR_OFF ,CIRCLE_FILL,
                             CIRCLE_FILL, True ,False ,True, None)
          self.mapActive = True
          self.pause(1)


  def config(self):
    print("braille_lotto config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_braille_lotto key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
      self.board_paused = pause
      # There is a problem with GTK widgets, they are not covered by the help
      # We hide/show them here
      if(self.board_paused):
          self.channel.hide()
          self.lotto_room_tv.hide()
          self.lotto_room_sw.hide()
          gcompris.timer.end()
      else :
          self.channel.show()
          self.lotto_room_tv.show()
          self.lotto_room_sw.show()
          gcompris.timer.pause(9)


  def set_level(self, level):
    print("braille_lotto set level. %i" % level)
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
import gobject
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.timer
import gcompris.anim
import goocanvas
import random
import pango

from BrailleChar import *
from braille_alphabets import *
from gcompris import gcompris_gettext as _

COLOR_ON = 0X00FFFFL
COLOR_OFF = 0X00000000L
CIRCLE_FILL = "light green"
CELL_WIDTH = 30

CHECK_RANDOM = []

class Gcompris_braille_lotto:
  """Empty gcompris python class"""

  def __init__(self, gcomprisBoard):
    print "braille_lotto init"

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

    for index in range(1, 91):
        CHECK_RANDOM.append(index)

    random.shuffle(CHECK_RANDOM)

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
    self.timerAnim = 0
    self.counter = 0
    self.gamewon = 0
    self.countAnim = 200
    self.score_player_a = 0
    self.score_player_b = 0
    self.status_timer = 50


    #REPEAT ICON
    gcompris.bar_set(gcompris.BAR_REPEAT_ICON)
    gcompris.bar_location(320,-1,0.8)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.root = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    self.lotto_board()

  def lotto_board(self):
    #Lotto Board
    board = goocanvas.Rect(
      parent = self.root,
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
        polyline = goocanvas.Polyline(parent = self.root, points=p_points, stroke_color="blue")

    #Vertical polylines for lotto board
    for index in range(9):
        p_points = goocanvas.Points([(42 * index + 80, 20.0),(42 * index + 80, 290.0)])
        polyline = goocanvas.Polyline(parent = self.root, points=p_points, stroke_color="blue")

    #Display Rectangle Ticket Boxes
    self.rect = []
    self.rect_x = []
    self.rect_y = []
    self.displayTicketBox(40 , 310)
    self.displayTicketBox(500, 310)

    #Rectangle box with ticket number is made clickable
    index = 0
    even = 0
    while (index < 12):
        if(even % 2 == 0):
            gcompris.utils.item_focus_init(self.rect[even],None)
            self.rect[even].connect("button_press_event",self.cross_number, index)
        even += 2
        index += 1

    #Displaying player_one and player_two
    #PLAYER 1
    goocanvas.Text(
                parent = self.root,
                x=170.0,
                y=500.0,
                text=_("PLAYER 1"),
                fill_color="black",
                anchor = gtk.ANCHOR_CENTER,
                alignment = pango.ALIGN_CENTER,
                )
    #PLAYER TWO
    goocanvas.Text(
                parent = self.root,
                x=640.0,
                y=500.0,
                text=_("PLAYER 2"),
                fill_color="black",
                anchor = gtk.ANCHOR_CENTER,
                alignment = pango.ALIGN_CENTER,
                )

    #Button to display the number to be checked in the ticket
    goocanvas.Image(parent = self.root,
                    pixbuf = gcompris.utils.load_pixmap("braille_lotto/button.jpg"),
                    x = 530,
                    y = 35,
                    )

    #Check number
    goocanvas.Text(
      parent = self.root,
      text= _("Check Number"),
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x=630,
      y=55,
      anchor=gtk.ANCHOR_CENTER,
      )

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
                    parent = self.root,
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

    #Adding a timer
    self.displayTimer()

    #Calling the random number and checking it on lotto board
    self.number_call()

    #An array to store the ticket numbers
    self.ticket_array = []

    #Displaying the Braille Code for TICKETS A & B
    #TICKET A
    self.displayTicket(1, 25, 55, 320)
    self.displayTicket(1, 25, 55, 432)
    self.displayTicket(26, 50, 122, 377)
    self.displayTicket(51, 75, 191, 320)
    self.displayTicket(51, 75, 191, 432)
    self.displayTicket(76, 90, 259, 377)

    #TICKET B
    self.displayTicket(1, 25, 515, 320)
    self.displayTicket(1, 25, 515, 432)
    self.displayTicket(26, 50, 582, 377)
    self.displayTicket(51, 75, 650, 320)
    self.displayTicket(51, 75, 650, 432)
    self.displayTicket(76, 90, 718, 377)


  def animTimer(self):
        self.countAnim -= 1
        if self.countAnim > 0:
            self.timerAnim = gobject.timeout_add(200, self.animTimer)
        else:
            self.check_number.set_property("text","")
            self.countAnim = 200
            self.counter += 1
            self.number_call()


  def displayTimer(self):
      self.timericon = gcompris.anim.CanvasItem( gcompris.anim.Animation("braille_lotto/sablier.txt"),
            self.root)
      self.timericon.goocanvas.translate(580, 150)
      self.timerAnim = gobject.timeout_add(200, self.animTimer)
      self.timericon.goocanvas.props.visibility = goocanvas.ITEM_VISIBLE

  def number_call(self):
      if(self.counter == 90):
          gcompris.utils.dialog(_("Game Over"),None)
          self.pause(1)
      self.timerAnim = gobject.timeout_add(200, self.animTimer)
      self.check_number = goocanvas.Text(
                            parent = self.root,
                            text= CHECK_RANDOM[self.counter],
                            x=630,
                            y=100,
                            font = "SANS 20",
                            anchor=gtk.ANCHOR_CENTER,
                            )
      #Place the checked button on number which is being called
      goocanvas.Image(parent = self.root,
                    pixbuf = gcompris.utils.load_pixmap("braille_lotto/button_checked.png"),
                    x = self.board_array_x[CHECK_RANDOM[self.counter] - 1] - 10 ,
                    y = self.board_array_y[CHECK_RANDOM[self.counter] - 1]- 10,
                    )


  def displayTicketBox(self, x, y):
      goocanvas.Rect(
      parent = self.root,
      x = x + 5,
      y = y + 5,
      width = 268,
      height = 168,
      stroke_color = "dark green",
      fill_color = "light green" ,
      line_width=7)

      for i in range(4):
        for j in range(3):
              box = goocanvas.Rect(
                             parent = self.root,
                             x = x + 5 + 68 * i,
                             y = y + 5 + 57 * j,
                             width = 65,
                             height = 54,
                             stroke_color = "dark green",
                             fill_color = "light green" ,
                             line_width=2)
              self.rect.append(box)
              self.rect_x.append(x + 5 + 68 * i)
              self.rect_y.append(y + 5 + 57 * j)

  def displayTicket(self, a, b, x, y):
      ticket = random.randint(a, b)
      self.ticket_array.append(ticket)
      if (ticket < 10):
          BrailleChar(self.root, x, y, 35 , ticket, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
      else :
          tens_digit = ticket / 10
          ones_digit = ticket % 10
          BrailleChar(self.root, x - 7, y, 33 ,tens_digit, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)
          BrailleChar(self.root, x + 20, y, 33 , ones_digit, COLOR_ON, COLOR_OFF ,
                  CIRCLE_FILL, CIRCLE_FILL, False, False ,True, None)

  def cross_number(self,item, event, target, index):
    #Cross Sign
    goocanvas.Image(parent = self.root,
                    pixbuf = gcompris.utils.load_pixmap("braille_lotto/cross_button.png"),
                    x = self.rect_x[index * 2]+8 ,
                    y = self.rect_y[index * 2]+5,
                    )

    if( CHECK_RANDOM[self.counter] == self.ticket_array[index]):
        if(index in (0, 1, 2, 3, 4, 5)):
            self.score_player_a +=1
        else:
            self.score_player_b +=1

    if(self.score_player_a == 6 or self.score_player_b == 6):
        if(self.score_player_a == 6):
            text = "PLAYER 1 \n" "You WON"
            color = "blue"
        elif(self.score_player_b == 6):
            text = "PLAYER 2 \n" "You WON"
            color = "dark green"
        status = goocanvas.Text(
                    parent = self.root,
                    text= text,
                    x=410,
                    y=370,
                    font = "SANS 20",
                    fill_color = color,
                    anchor=gtk.ANCHOR_CENTER,
                   )
        self.timer_inc  = gobject.timeout_add(self.status_timer,
                                            self.timer_loop)
  def timer_loop(self):
      self.status_timer -= 1
      if(self.status_timer == 0):
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)
      self.timer_inc  = gobject.timeout_add(self.status_timer,
                                            self.timer_loop)

  def end(self):
    print "braille_lotto end"
    if self.timer_inc:
      gobject.source_remove(self.timer_inc)
    if self.timerAnim:
      gobject.source_remove(self.timerAnim)
    del self.timerAnim
    del self.timer_inc

    # Remove the root item removes all the others inside it
    self.root.remove()
    gcompris.end_board()

  def ok(self):
    print("braille_lotto ok.")

  def repeat(self):
      if(self.mapActive):
          self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
          self.root.props.visibility = goocanvas.ITEM_VISIBLE
          self.mapActive = False
          self.pause(0)
      else :
          self.root.props.visibility = goocanvas.ITEM_INVISIBLE
          self.rootitem = goocanvas.Group(parent=
                                   self.gcomprisBoard.canvas.get_root_item())
          gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
          map_obj = Gcompris_braille_alphabets(self.gcomprisBoard)
          map_obj.Braille_Map(self.rootitem, COLOR_ON, COLOR_OFF, CIRCLE_FILL, CIRCLE_STROKE)
          self.mapActive = True
          self.pause(1)

  def config(self):
    print("braille_lotto config.")

  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

  def pause(self, pause):
      self.board_paused = pause
      if(self.board_paused == 1 and (self.counter == 90 or self.gamewon == 1)):
          self.end()
          self.start()

  def set_level(self, level):
    print("braille_lotto set level. %i" % level)
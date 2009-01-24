#  gcompris - Hexagon
#
# Copyright (C) 2004, 2008  Christof Petig and Ingo Konrad
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
import goocanvas
import gcompris
import gcompris.utils
import gcompris.bonus
import gcompris.skin
import gcompris.sound
import gtk
import gtk.gdk
import random
import math

class Gcompris_hexagon:
  """The hexagon activity"""

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.rootitem = None

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = False;


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=9
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
    gcompris.bar_set_level(self.gcomprisBoard)
    self.set_level(1)
    self.game_start()


  def end(self):
    self.cleanup()

  def ok(self):
    pass

  def key_press(self, keyval, commit_str, preedit_str):
    return False

  # Called by gcompris core
  def pause(self, pause):

    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(pause == 0 and self.gamewon):
      self.gamewon = 0
      self.set_level(self.gcomprisBoard.level)
      self.game_start()

    return

  # Called by gcompris when the user click on the level icon
  def set_level(self, level):
    self.gcomprisBoard.level=level
    self.gcomprisBoard.sublevel=1
    # Set the level in the control bar
    gcompris.bar_set_level(self.gcomprisBoard);

    if(self.gcomprisBoard.level == 1):
      self.r = 30
      self.nbx = 15
      self.nby = 10
    elif(self.gcomprisBoard.level == 2):
      self.r = 28
      self.nbx = 16
      self.nby = 10
    elif(self.gcomprisBoard.level == 3):
      self.r = 27
      self.nbx = 16
      self.nby = 11
    elif(self.gcomprisBoard.level == 4):
      self.r = 25
      self.nbx = 18
      self.nby = 12
    elif(self.gcomprisBoard.level == 5):
      self.r = 23
      self.nbx = 19
      self.nby = 13
    elif(self.gcomprisBoard.level == 6):
      self.r = 22
      self.nbx = 20
      self.nby = 13
    elif(self.gcomprisBoard.level == 7):
      self.r = 21
      self.nbx = 21
      self.nby = 14
    elif(self.gcomprisBoard.level == 8):
      self.r = 19
      self.nbx = 23
      self.nby = 15
    elif(self.gcomprisBoard.level == 9):
      self.r = 17
      self.nbx = 26
      self.nby = 17

    self.game_start()


  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------

  def game_start(self):
    self.cleanup()
    self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())
    self.paint_skin()
    self.random_catx = random.randrange(self.nbx)
    self.random_caty = random.randrange(self.nby)

  def cleanup(self):
    self.gamewon       = False;
    # Remove the root item removes all the others inside it
    if self.rootitem != None:
     self.rootitem.remove()
     self.rootitem = None

  def paint_hex(self, x, y, color=0x0099FFCCL):
    ax = 30+self.sqrt3*self.r*x
    ay = 30+1.5*self.r*y

    if y&1 :
     ax+=self.sqrt3/2*self.r
    pts = []
    for i in range (len(self.cx)):
     pts.append ((ax+self.cx [i], ay+self.cy [i]))

    s = goocanvas.Polyline (parent =  self.rootitem,
                            points = goocanvas.Points(pts),
                            close_path = True,
                            stroke_color = "black",
                            fill_color_rgba = color,
                            line_width = 2.5)
    return s

  def paint_skin(self):
    self.cx = []
    self.cy = []
    self.sqrt3 = math.sqrt(3)
    for i in range (6):
     self.cx.append(int(self.r*math.cos(math.pi/6+i*math.pi/3)))
     self.cy.append(int(self.r*math.sin(math.pi/6+i*math.pi/3)))
    for x in range (self.nbx):
     for y in range (self.nby):
      s = self.paint_hex(x, y)

      s.connect ("button_press_event", self.on_click, x-int(y/2), y)

  def coloring(self,dist):
    r=b=g=0
    if dist <0.25:
     r = 0xFF
     g = long(4*0xFF*dist)
    elif dist <0.5:
     g = 0xFF
     r = 0xFF-long(4*(dist-0.25)*0xFF)
    elif dist <0.75:
     g = 0xFF-long(4*(dist-0.5)*0xFF)
     b = long(0xFF*4*(dist-0.5))
    elif dist <=1:
     b = 0xFF
     r = long(0xFF*4*(dist-0.75))
    else :
     pass
    color = r*0x1000000+g*0x10000+b*0x100+0xFF
    return color

  def on_click (self, widget, target, event=None, x=0, y=0):
    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1 :

      catdistance = self.distance_cat(x,y)
      #print self.random_catx, self.random_caty,x,y,catdistance

      if catdistance<0.1:
        gcompris.sound.play_ogg("sounds/smudge.wav")

        self.paint_cat()
        self.increment_level()
        self.gamewon = True;
        gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.GNU)
      else:
        gcompris.sound.play_ogg("sounds/bleep.wav")

        color = self.coloring (catdistance/30.0)
        widget.props.fill_color_rgba = color;


  def paint_cat(self):
    position = 19 + self.sqrt3 * self.r * self.random_catx
    if self.random_caty % 2:
     position += self.sqrt3/2 * self.r
    pixbuf2 = gcompris.utils.load_pixmap \
    ("hexagon/strawberry.svg")
    h2 = 30
    w2 = pixbuf2.get_width() * h2/pixbuf2.get_height()
    img = goocanvas.Image (parent = self.rootitem,
                           pixbuf = pixbuf2.scale_simple(w2, h2,
                                                  gtk.gdk.INTERP_BILINEAR),
                           x = position - 5,
                           y = 14+1.5*self.random_caty*self.r)

  def distance_cat (self,x,y):
    dx = self.random_catx-x-int(self.random_caty/2)
    dy = self.random_caty-y

    if dx*dy >=0:
     return abs(dx)+abs(dy)
    if dx*dy<0:
     return max(abs(dx),abs(dy))

  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return 1 if continue, 0 if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
        self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

    return 1

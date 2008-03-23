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
    gcompris.bar_set (0)
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    self.game_start()


  def end(self):
    self.cleanup()

  def ok(self):
    print("Gcompris_hexagonagon ok.")

  def key_press(self, keyval, commit_str, preedit_str):
    #print("got key %i" % keyval)
    return False

  # Called by gcompris core
  def pause(self, pause):

    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(pause == 0 and self.gamewon):
      self.finished()
      self.gamewon = 0

    return


  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------

  def game_start(self):
    self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())
    self.paint_skin()
    self.random_catx = random.randrange(21)
    self.random_caty = random.randrange(15)

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
    self.r = 20
    self.cx = []
    self.cy = []
    self.sqrt3 = math.sqrt(3)
    for i in range (6):
     self.cx.append(int(self.r*math.cos(math.pi/6+i*math.pi/3)))
     self.cy.append(int(self.r*math.sin(math.pi/6+i*math.pi/3)))
    for x in range (22):
     for y in range (16):
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

  def finished(self):
    self.cleanup()
    self.game_start()

  def on_click (self, widget, target, event=None, x=0, y=0):
    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1 :

      catdistance = self.distance_cat(x,y)
      #print self.random_catx, self.random_caty,x,y,catdistance

      if catdistance<0.1:
        gcompris.sound.play_ogg("sounds/smudge.wav")

        self.paint_cat()
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

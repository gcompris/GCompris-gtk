#  gcompris - Hexagon
# 
# Time-stamp: <2004/31/03 15:10:00 Ingo Konrad>
# 
# Copyright (C) 2004  Christof Petig and Ingo Konrad
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.bonus
import gcompris.skin
import gtk
import gtk.gdk
import random
import math

class Gcompris_hexagon:
  """The minouche activity"""
  
  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.rootitem = None
    self.timer    = 0

  def start(self): 
    gcompris.bar_set (0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    self.rootitem = self.gcomprisBoard.canvas.root().add(
     gnome.canvas.CanvasGroup,
     x=0.0,
     y=0.0
     )
    self.paint_skin()
    self.random_catx = random.randrange(21)
    self.random_caty = random.randrange(15)


  def end(self):
    self.cleanup()
        
  def ok(self):
    print("Gcompris_minouche ok.")

  def key_press(self, keyval):
    #print("got key %i" % keyval)
    return gtk.FALSE

  
  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------


  def cleanup(self):

    if(self.timer):
      gtk.timeout_remove(self.timer)

    # Remove the root item removes all the others inside it
    if self.rootitem != None: 
     self.rootitem.destroy()
     self.rootitem = None

  def paint_hex(self, x, y, color=0x0099FFCCL):
    ax = 30+self.sqrt3*self.r*x
    ay = 30+1.5*self.r*y
      
    if y&1 :
     ax+=self.sqrt3/2*self.r
    self.pts = []
    for i in range (len(self.cx)):
     self.pts.append (ax+self.cx [i])
     self.pts.append (ay+self.cy [i]) 
   
    s = self.rootitem.add(gnome.canvas.CanvasPolygon, points = self.pts, 
         fill_color_rgba = color, outline_color = "black", width_units = 
         2.5)
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
       
      s.connect ("event", self.on_click, x-int(y/2), y)
      
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
    gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
    self.timer = 0
    
  def on_click (self, widget, event=None, x=0, y=0):
    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1 :
      
     catdistance = self.distance_cat(x,y)
     #print self.random_catx, self.random_caty,x,y,catdistance
                             
     if catdistance<0.1:
      self.paint_cat()
      self.timer = gtk.timeout_add(5000, self.finished) 
     else:
       color = self.coloring (catdistance/30.0)
       widget.set(fill_color_rgba=color);
     

  def paint_cat(self):
    position =19+self.sqrt3*self.r*self.random_catx
    if self.random_caty%2:
     position +=self.sqrt3/2*self.r
    pixbuf2 = gcompris.utils.load_pixmap \
    ("gcompris/misc/strawberry.png")
    h2 = 30
    w2 = pixbuf2.get_width()*h2/pixbuf2.get_height()
    self.rootitem.add(gnome.canvas.CanvasPixbuf, 
                      pixbuf=pixbuf2.scale_simple(w2, h2,
                                                  gtk.gdk.INTERP_BILINEAR),
                      x=position,
                      y=14+1.5*self.random_caty*self.r)
    
  def distance_cat (self,x,y):
    dx = self.random_catx-x-int(self.random_caty/2)
    dy = self.random_caty-y
    
    if dx*dy >=0:
     return abs(dx)+abs(dy)
    if dx*dy<0:
     return max(abs(dx),abs(dy))

#  gcompris - minouche
# 
# Time-stamp: <2004/29/03 15:26:00 Ingo Konrad>
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
import gcompris.skin
import gcompris.bonus
import gtk
import gtk.gdk
import random
import math

class Gcompris_minouche:
  """The minouche activity"""
  
  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.anzahl = 8
    print("Gcompris_minouche __init__.")
    self.rootitem = None
    self.distancex = 40
    self.distancey = 40
    self.leftx = 170
    self.lefty = 40
    self.size = 30

  def start(self): 
    gcompris.bar_set (0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                                       "gcompris/animals/singegc.jpg")
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    self.paint_image()
    print("Gcompris_minouche start.")
    self.random_catx = random.randrange(11)
    self.random_caty = random.randrange(11)
    

  def end(self):
    self.cleanup()
    print("Gcompris_minouche end.")
        
  def ok(self):
    print("Gcompris_minouche ok.")

  def cleanup(self):
    
    # Remove the root item removes all the others inside it
    if self.rootitem != None: 
     self.rootitem.destroy()
     self.rootitem = None

  def paint_image(self):
   for x in range(11):
    for y in range(11):
     s = self.rootitem.add(
     gnome.canvas.CanvasRect,
     x1=self.leftx+self.size+self.distancex*x,
     y1=self.lefty+self.size+self.distancey*y,
     x2=self.leftx+self.distancex*x,
     y2=self.lefty+self.distancey*y,
     fill_color_rgba=0x0099FFCCL,
     width_units=0.0
      )
     
     s.connect ("event", self.on_click, x, y)

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
  
  def on_click (self, widget, event=None, x=0, y=0):
   if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1 :
   
    xpoint=x
    ypoint=y

    catdistance = math.hypot((self.random_catx-xpoint),  
                            (self.random_caty-ypoint))
                            
    color = self.coloring(catdistance/15)
    widget.set(fill_color_rgba=color);

    if catdistance<0.1:
     self.paint_cat ()
     gcompris.bonus.board_finished(gcompris.bonus.    
     FINISHED_RANDOM) 

  def paint_cat (self):
   pixbuf2 = gcompris.utils.load_pixmap \
    ("gcompris/misc/strawberry.png")
   h2 = self.size
   w2 = pixbuf2.get_width()*h2/pixbuf2.get_height()
   self.rootitem.add(gnome.canvas.CanvasPixbuf, 
     pixbuf=pixbuf2.scale_simple(w2, h2,
                         gtk.gdk.INTERP_BILINEAR),
     x=self.leftx+3+self.distancex*self.random_catx,
     y=self.lefty+self.distancey*self.random_caty)
   del pixbuf2

#  gcompris - melody
# 
# Time-stamp: <2003/12/09 20:54:45 jose>
# 
# Copyright (C) 2003 Jose Jorge
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
import gcompris.sound
import gtk
import gtk.gdk

class Gcompris_melody:
  """The melody activity"""
  

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    
    print("Gcompris_melody __init__.")
  

  def start(self):  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1 
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            "melody/background.jpg")
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    # son1
    self.son1_item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("melody/son1.png"),
      x=150.0,
      y=50.0
      )
    self.son1_item.connect("event", self.son1_item_event)
    # This item is clickeable and it must be seen
    self.son1_item.connect("event", gcompris.utils.item_event_focus)

    # son2
    self.son2_item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("melody/son2.png"),
      x=550.0,
      y=50.0
      )
    self.son2_item.connect("event", self.son2_item_event)
    # This item is clickeable and it must be seen
    self.son2_item.connect("event", gcompris.utils.item_event_focus)

    # son3
    self.son3_item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("melody/son3.png"),
      x=150.0,
      y=250.0
      )
    self.son3_item.connect("event", self.son3_item_event)
    # This item is clickeable and it must be seen
    self.son3_item.connect("event", gcompris.utils.item_event_focus)

    # son4
    self.son4_item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("melody/son4.png"),
      x=550.0,
      y=250.0
      )
    self.son4_item.connect("event", self.son4_item_event)
    # This item is clickeable and it must be seen
    self.son4_item.connect("event", gcompris.utils.item_event_focus)

    print("Gcompris_melody start.")
    
  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

    print("Gcompris_melody end.")
        

  def ok(self):
    print("Gcompris_melody ok.")
          

  def repeat(self):
    print("Gcompris_melody repeat.")
            

  def config(self):
    print("Gcompris_melody config.")
              
  def key_press(self, keyval):
    print("got key %i" % keyval)
    return


  def pause(self, pause):  
    print("Gcompris_melody pause. %i" % pause)
                  
  def set_level(self, level):  
    print("Gcompris_melody set level. %i" % level)

  def son1_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
          gcompris.sound.play_ogg("melody/son1")
    return gtk.FALSE
								  
  def son2_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
          gcompris.sound.play_ogg("melody/son2")
    return gtk.FALSE
								  
  def son3_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
          gcompris.sound.play_ogg("melody/son3")
    return gtk.FALSE
								  
  def son4_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
          gcompris.sound.play_ogg("melody/son4")
    return gtk.FALSE
								  

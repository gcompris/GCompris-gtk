#  gcompris - searace
# 
# Time-stamp: <2001/08/20 00:54:45 bruno>
# 
# Copyright (C) 2003 Bruno Coudoin
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
import gtk
import gtk.gdk
import random

class Gcompris_searace:
  """The Boat Racing activity"""
  

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    # Some constants
    border_x  = 30
    self.sea_area = (border_x , 30, gcompris.BOARD_WIDTH-border_x , 350)
    self.weather   = []
    
    print("Gcompris_searace __init__.")
  

  def start(self):  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1 
    self.gcomprisBoard.number_of_sublevel=1

    # The basic tick for object moves
    self.timerinc = 50

    # Need to manage the timers to quit properly
    self.kid_timer = 0
    self.tux_timer = 0
    
    gcompris.bar_set(0)
    #    gcompris.set_background(self.gcomprisBoard.canvas.root(),
    #                            "searace/background.png")
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.display_sea_area()
    self.display_weather()
    
    print("Gcompris_searace start.")

    
  def end(self):
    # Remove all the timer first
    if self.kid_timer :
      gtk.timeout_remove(self.kid_timer)
    if self.tux_timer :
      gtk.timeout_remove(self.tux_timer)
    
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

    print("Gcompris_searace end.")
        

  def ok(self):
    print("Gcompris_searace ok.")
          

  def repeat(self):
    print("Gcompris_searace repeat.")
            

  def config(self):
    print("Gcompris_searace config.")
              
  def key_press(self, keyval):
    print("got key %i" % keyval)
    return

  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------

  def display_sea_area(self):
    # Some constant to define the sea area
    # The sea area is defined in the global self.sea_area
    step_x    = (self.sea_area[2]-self.sea_area[0])/20
    step_y    = (self.sea_area[3]-self.sea_area[1])/10

    text_x    = self.sea_area[0] - 15
    text_y    = self.sea_area[1] - 15

    # We manage a 2 colors grid
    ci = 0
    ca = 0xAACCFFFFL
    cb = 0x1D0DFFFFL

    for y in range (self.sea_area[1], self.sea_area[3]+1, step_y):
      if(ci%2):
        color = ca
      else:
        color = cb
      ci += 1

      # Shadow for text number
      self.rootitem.add (
        gnome.canvas.CanvasText,
        text=int(ci),
        font=gcompris.skin.get_font("gcompris/content"),
        x=text_x+1,
        y=y+1,
        fill_color_rgba=0x000000FFL
        )
      # Text number
      self.rootitem.add (
        gnome.canvas.CanvasText,
        text=int(ci),
        font=gcompris.skin.get_font("gcompris/content"),
        x=text_x,
        y=y,
        fill_color_rgba=cb
        )

      self.rootitem.add(
        gnome.canvas.CanvasLine,
        points=(self.sea_area[0], y, self.sea_area[2], y),
        fill_color_rgba = color,
         width_units=1.0
        )

    ci = 0
    for x in range (self.sea_area[0], self.sea_area[2]+1, step_x):
      if(ci%2):
        color = ca
      else:
        color = cb
      ci += 1

      # Shadow for text number
      self.rootitem.add (
        gnome.canvas.CanvasText,
        text=int(ci),
        font=gcompris.skin.get_font("gcompris/content"),
        x=x+1,
        y=text_y+1,
        fill_color_rgba=0x000000FFL
        )
      # Text number
      self.rootitem.add (
        gnome.canvas.CanvasText,
        text=int(ci),
        font=gcompris.skin.get_font("gcompris/content"),
        x=x,
        y=text_y,
        fill_color_rgba=cb
        )
      self.rootitem.add(
        gnome.canvas.CanvasLine,
        points=(x, self.sea_area[1], x, self.sea_area[3]),
        fill_color_rgba = color,
         width_units=1.0
        )
  

  # Weather condition is a 2 value pair (angle wind_speed)
  # Weather is a list of the form:
  # (rectangle coordinate) (weather)
  def display_weather(self):
    # Some constant to define the sea area
    # The sea area is defined in the global self.sea_area
    slice_x = 5
    slice_y = 3
    
    step_x  = (self.sea_area[2]-self.sea_area[0])/slice_x
    step_y  = (self.sea_area[3]-self.sea_area[1])/slice_y

    stop_x  = self.sea_area[0]+step_x*slice_x
    stop_y  = self.sea_area[1]+step_y*slice_y

    for x in range (self.sea_area[0], stop_x, step_x):
      for y in range (self.sea_area[1], stop_y, step_y):
        print x, step_x, self.sea_area[2]
        angle = 0
        direction = random.randint(0,6)
        if(direction < 3):
          # There is more chance to go forward than others
          angle = random.randint(-45,45)
        elif(direction < 4):
          angle = random.randint(135,225)
        elif(direction == 5):
          angle = random.randint(80, 110)
        elif(direction == 6):
          angle = random.randint(260, 280)

        speed = random.randint(1,10)
        condition = [ (x, y, x+step_x, y+step_y), (angle, speed) ]
        self.display_condition(condition)
        self.weather.append(condition)
    
    
    return

  # Display the given weather condition
  def display_condition(self, condition):

    print condition
    # Calc the center
    cx = condition[0][0]+(condition[0][2]-condition[0][0])/2
    cy = condition[0][1]+(condition[0][3]-condition[0][1])/2
    
    pixmap = gcompris.utils.load_pixmap("images/arrow.png")
    item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=cx-pixmap.get_width()/2,
      y=cy-pixmap.get_height()/2
      )
    gcompris.utils.item_rotate_relative(item, condition[1][0]);

    # Text number
    self.rootitem.add (
      gnome.canvas.CanvasText,
      text=condition[1][1],
      font=gcompris.skin.get_font("gcompris/content"),
      x=cx+1,
      y=cy+1,
      fill_color_rgba=0x000000FFL
      )
    
    # Text number
    self.rootitem.add (
      gnome.canvas.CanvasText,
      text=condition[1][1],
      font=gcompris.skin.get_font("gcompris/content"),
      x=cx,
      y=cy,
      fill_color_rgba=0xFFFFFFFFL
      )
    
    return
  

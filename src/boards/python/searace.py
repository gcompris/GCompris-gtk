#  gcompris - searace
# 
# Time-stamp: <2001/08/20 00:54:45 bruno>
# 
# Copyright (C) 2004 Bruno Coudoin
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
import math
import time
from gettext import gettext as _

class Boat:
  """The Boat Class"""
  tb   = None
  tv   = None
  x    = 0.0
  y    = 0.0
  angle  = 0

  # To move the ship
  dx   = 0.0
  dy   = 0.0

  # The user commands parsing
  line = 0
  # The boat item
  item = []
  player = 0

  # Store a timer object
  timer  = 0
    
class Gcompris_searace:
  """The Boat Racing activity"""
  

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    # Some constants
    border_x  = 30
    self.sea_area = (border_x , 30, gcompris.BOARD_WIDTH-border_x , 350)
    self.weather   = []

    self.left_boat  = Boat()
    self.left_boat.player = 0
    
    self.right_boat = Boat()
    self.left_boat.player = 1
    
    # The boat coordinate
    self.left_boat.x = border_x
    self.left_boat.y = 150
    self.right_boat.x = self.left_boat.x
    self.right_boat.y = self.left_boat.y + 30

    # The basic tick for object moves
    self.timerinc = 50
    self.timer1 = 0
    self.timer2 = 0

    print("Gcompris_searace __init__.")
  

  def start(self):  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1 
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(gcompris.BAR_OK|gcompris.BAR_LEVEL)
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

    print("Gcompris_searace start.")

    
  def end(self):
    # Remove all the timer first
    if self.timer1 :
      gtk.timeout_remove(self.timer1)
    
    if self.timer2 :
      gtk.timeout_remove(self.timer2)
    
    if self.left_boat.timer :
      gtk.timeout_remove(self.left_boat.timer)
    
    if self.right_boat.timer :
      gtk.timeout_remove(self.right_boat.timer)
    
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

    print("Gcompris_searace end.")
        

  def ok(self):
    print("Gcompris_searace ok.")

    # This is a real go
    # We set a timer. At each tick an entry in each user box is read analysed and run
    if(not self.left_boat.timer and not self.right_boat.timer):
      self.race_one_command(self.left_boat)
      self.race_one_command(self.right_boat)
    else:
      print "Race is already running"

  def repeat(self):
    print("Gcompris_searace repeat.")
            

  def config(self):
    print("Gcompris_searace config.")
              
  def key_press(self, keyval):
    #print("got key %i" % keyval)
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

    # Display the weather now
    self.display_weather()

    # The grid is done
    # ----------------
    
    # The Programming input area LEFT
    sw = gtk.ScrolledWindow()
    sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
    sw.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
      
    w = 250.0
    h = 100.0
    self.left_boat.tb = gtk.TextBuffer()
    self.left_boat.tv = gtk.TextView(self.left_boat.tb)
    sw.add(self.left_boat.tv)
    self.left_boat.tb.set_text("turnleft 90\nforward 10")
    self.left_boat.tv.set_wrap_mode(gtk.WRAP_CHAR)
    self.rootitem.add(
      gnome.canvas.CanvasWidget,
      widget=sw,
      x=gcompris.BOARD_WIDTH/4,
      y=400,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_N,
      size_pixels=gtk.FALSE);
    self.left_boat.tv.show();
    sw.show();

    # The Programming input area RIGHT
    sw = gtk.ScrolledWindow()
    sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
    sw.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
      
    w = 250.0
    h = 100.0
    self.right_boat.tb = gtk.TextBuffer()
    self.right_boat.tv = gtk.TextView(self.right_boat.tb)
    sw.add(self.right_boat.tv)
    self.right_boat.tb.set_text("turnleft 10")
    self.right_boat.tv.set_wrap_mode(gtk.WRAP_CHAR)
    self.rootitem.add(
      gnome.canvas.CanvasWidget,
      widget=sw,
      x=(gcompris.BOARD_WIDTH/4)*3,
      y=400,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_N,
      size_pixels=gtk.FALSE);
    self.right_boat.tv.show();
    sw.show();

    # Display the player boats
    pixmap = gcompris.utils.load_pixmap("images/top_boat_red.png")
    self.left_boat.item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=self.left_boat.x,
      y=self.left_boat.y,
      anchor=gtk.ANCHOR_CENTER,
      )
      
    pixmap = gcompris.utils.load_pixmap("images/top_boat_green.png")
    self.right_boat.item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=self.right_boat.x,
      y=self.right_boat.y,
      anchor=gtk.ANCHOR_CENTER,
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

  def cmd_forward(self, boat, value):
    #    print "Player " + str(boat.player) + " cmd_forward " + str(value) + " dx=" + str(boat.dx) + " dy=" + str(boat.dy)

    value -= 1
    if value <= 0:
      # Process next command
      self.race_one_command(boat)
      return
    
    
    # Move it
    boat.x += 1
    boat.y += 0
    #    print "  x=" + str(int(boat.x)) + "  y=" + str(int(boat.y))
    boat.item.set(x = boat.x,
                  y = boat.y)
    boat.timer = gtk.timeout_add(self.timerinc, self.cmd_forward, boat, value)


  # Counter Clock wise rotation (use negative param to turn clock wise)
  def cmd_turn_left(self, boat, value):
    #    print "Player " + str(boat.player) + " turn left " + str(value)

    if value == 0:
      # Process next command
      self.race_one_command(boat)
      return

    turn = 1
    if value > 0:
      turn = -1
      
    boat.angle += turn
    value    += turn

    gcompris.utils.item_rotate_relative(boat.item, turn);
    boat.timer = gtk.timeout_add(self.timerinc, self.cmd_turn_left, boat, value)
  
  # Run the race
  def race_one_command(self, boat):

    a = boat.tb.get_iter_at_line(boat.line)
    b = boat.tb.get_iter_at_line(boat.line)
    b.forward_to_line_end()
    boat.line+=1
    
    if (boat.line > boat.tb.get_line_count()):
      print "No more commands to process for player " + str(boat.player)
      boat.line = 0
      boat.timer = 0
      return
    
    cmd   = boat.tb.get_text(a, b, gtk.FALSE)
    cmds  = cmd.split()
    if ( len(cmds) != 2):
      print "Player " + str(boat.player) + " Syntax error command=" + cmd + " At line " + str(boat.line)
      boat.line = 0
      boat.timer = 0
      return
      
    value = int(cmd.split()[1])
    if( cmd.startswith(_("forward"))):
      # Initialize the move
      boat.timer = gtk.timeout_add(self.timerinc, self.cmd_forward, boat, value)
    elif( cmd.startswith(_("turnleft"))):
      boat.timer = gtk.timeout_add(self.timerinc, self.cmd_turn_left, boat, value)
    elif( cmd.startswith(_("turnright"))):
      boat.timer = gtk.timeout_add(self.timerinc, self.cmd_turn_left, boat, value*-1)
    else:
      boat.line = 0
      boat.timer = 0
      print "Player " + str(boat.player) + " Unknown command: " + "'" + cmd + "'"
      print "Player " + str(boat.player) + "Stop processing your commands"


  # ---- TEST ONLY ----
  def test(self):
    boat = self.left_boat
    boat.x = 30
    boat.y = 30
    boat.angle = 45.0
    boat.item.set(x = boat.x,
                  y = boat.y)
    boat.dx   = math.cos(boat.angle*(math.pi/180))
    boat.dy   = math.sin(boat.angle*(math.pi/180))
    self.rootitem.add(
      gnome.canvas.CanvasLine,
      points = (boat.x , boat.y,
                boat.x + boat.dx * 100 , boat.y + boat.dy * 100),
      fill_color_rgba = 0x0033FFFFL,
      width_units = 2.0
      )

    for angle in range(1, 360, 10):
      gcompris.utils.item_rotate_relative(boat.item, 10);
      print "Angle=" + str(boat.angle) + " dx=" + str(boat.dx) + " dy=" + str(boat.dy)

      for i in range(50):
        while gtk.events_pending():
          gtk.main_iteration()
        boat.x += 1.0
        boat.y += 0.0
        print "  x=" + str(int(boat.x)) + "  y=" + str(int(boat.y))
        boat.item.set(x = boat.x,
                      y = boat.y)
        self.rootitem.add(
          gnome.canvas.CanvasLine,
          points = (30.0 , 30.0,
                    boat.x , boat.y),
          fill_color_rgba = 0x0F331FFFL,
          width_units = 4.0
          )

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

  # The Text View and Scrolled windows
  tb   = None
  tv   = None
  sw   = None
  x    = 0.0
  y    = 0.0
  angle  = 0
  arrived = False
  won     = False
  
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

  # Weather condition cache to avoid to find it each time
  condition = []

  # Display the speed here
  speeditem = []
  
class Gcompris_searace:
  """The Boat Racing activity"""
  

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    self.board_paused = False
    
    # Some constants
    self.border_x  = 30
    self.sea_area = (self.border_x , 30, gcompris.BOARD_WIDTH-self.border_x , 350)
    self.weather   = []

    self.left_boat  = Boat()
    self.left_boat.player = 0
    
    self.right_boat = Boat()
    self.left_boat.player = 1
    
    # The basic tick for object moves
    self.timerinc = 40
    self.timer_turn = 15
    self.timer1 = 0
    self.timer2 = 0

    # How to transform user visible sea size to pixels (calculated later)
    self.sea_ratio = 1

    # We display what's going on here
    self.statusitem = []
    
    #print("Gcompris_searace __init__.")
  

  def start(self):  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=4
    self.gcomprisBoard.sublevel=1 
    self.gcomprisBoard.number_of_sublevel=1

    self.board_paused = False

    pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_reload.png"))
    if(pixmap):
      gcompris.bar_set_repeat_icon(pixmap)
      gcompris.bar_set(gcompris.BAR_OK|gcompris.BAR_LEVEL|gcompris.BAR_REPEAT_ICON)
    else:
      gcompris.bar_set(gcompris.BAR_OK|gcompris.BAR_LEVEL|gcompris.BAR_REPEAT);
    
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.display_sea_area()

    self.root_weather_item = self.rootitem.add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    # Display the weather now
    self.display_weather()

    # And finaly the players boats
    self.init_boats()
    
    #print("Gcompris_searace start.")


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

    #print("Gcompris_searace end.")
        

  def pause(self, pause):
    
    self.board_paused = pause
    
    # There is a problem with GTK widgets, they are not covered by the help
    # We hide/show them here
    if(pause):
      self.left_boat.sw.hide()
      self.right_boat.sw.hide()
    else:
      self.left_boat.sw.show()
      self.right_boat.sw.show()
      self.repeat()

    return
                  
  def ok(self):
    # This is a real go
    # We set a timer. At each tick an entry in each user box is read analysed and run
    if(not self.left_boat.timer and not self.right_boat.timer):
      self.left_boat.tv.set_editable(False)
      self.right_boat.tv.set_editable(False)
      self.race_one_command(self.left_boat)
      self.race_one_command(self.right_boat)
    else:
      self.statusitem.set(text=_("Race is already running"))

  # Called by gcompris when the user click on the level icon
  def set_level(self, level):
    if(self.left_boat.timer or self.right_boat.timer):
      self.statusitem.set(text=_("Race is already running"))
    else:
      self.gcomprisBoard.level=level;
      self.gcomprisBoard.sublevel=1;
      
      # Set the level in the control bar
      gcompris.bar_set_level(self.gcomprisBoard);
      
      # Remove the root item removes all the others inside it
      self.root_weather_item.destroy()

      self.root_weather_item = self.rootitem.add(
        gnome.canvas.CanvasGroup,
        x=0.0,
        y=0.0
        )

      # Display the weather now
      self.display_weather()

      self.init_boats()
      
            
  def repeat(self):
    # Want to rerun it
    if(self.left_boat.timer or self.right_boat.timer):
      self.statusitem.set(text=_("Race is already running"))
    else:
      self.init_boats()
      
            
  def config(self):
    #print("Gcompris_searace config.")
    return
              
  def key_press(self, keyval):
    #print("got key %i" % keyval)
    return

  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------

  # Set the initial coordinates of the boats and display them
  def init_boats(self):

    self.left_boat.x      = self.border_x
    self.left_boat.y      = 150
    self.right_boat.x     = self.left_boat.x
    self.right_boat.y     = self.left_boat.y + 28
    self.left_boat.angle  = 0
    self.right_boat.angle = 0
    
    # Display the player boats
    if(self.left_boat.item):
      self.left_boat.item.destroy()
      
    pixmap = gcompris.utils.load_pixmap("images/top_boat_red.png")
    self.left_boat.item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=self.left_boat.x,
      y=self.left_boat.y,
      anchor=gtk.ANCHOR_CENTER,
      )
    self.left_boat.item.raise_to_top()
    
    if(self.right_boat.item):
      self.right_boat.item.destroy()

    pixmap = gcompris.utils.load_pixmap("images/top_boat_green.png")
    self.right_boat.item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=self.right_boat.x,
      y=self.right_boat.y,
      anchor=gtk.ANCHOR_CENTER,
      )
    self.right_boat.item.raise_to_top()
    
    # Reset command line processing as well.
    self.left_boat.line     = 0
    self.right_boat.line    = 0
    self.left_boat.arrived  = False
    self.right_boat.arrived = False
    self.left_boat.won      = False
    self.right_boat.won     = False
    self.statusitem.set(text="")

    # Let the user enter comands
    self.left_boat.tv.set_editable(True)
    self.right_boat.tv.set_editable(True)

  #----------------------------------------
  # Display the whole playing field
  # This is called once only
  def display_sea_area(self):
    # Some constant to define the sea area
    # The sea area is defined in the global self.sea_area
    step_x    = (self.sea_area[2]-self.sea_area[0])/20
    step_y    = (self.sea_area[3]-self.sea_area[1])/10
    self.sea_ratio = step_x
    
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


    # The ARRIVAL LINE
    self.rootitem.add(
      gnome.canvas.CanvasLine,
      points=(self.sea_area[2], self.sea_area[1]-5, self.sea_area[2], self.sea_area[3]+5),
      fill_color_rgba = 0xFF0000FFL,
      width_units=5.0
      )
    
    # The grid is done
    # ----------------
    
    # The Programming input area LEFT
    self.left_boat.sw = gtk.ScrolledWindow()
    self.left_boat.sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
    self.left_boat.sw.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
      
    w = 250.0
    h = 100.0
    y = 400.0 # The upper limit of the text boxes
    x_left  = gcompris.BOARD_WIDTH/4 - 30
    x_right = (gcompris.BOARD_WIDTH/4)*3 + 30

    self.left_boat.tb = gtk.TextBuffer()
    self.left_boat.tv = gtk.TextView(self.left_boat.tb)
    self.left_boat.sw.add(self.left_boat.tv)

    command_example = _("droite") + " 45\n" + _("forward") + " 5\n" + _("gauche") + " 45"
    self.left_boat.tb.set_text(command_example)
    
    self.left_boat.tv.set_wrap_mode(gtk.WRAP_CHAR)
    self.rootitem.add(
      gnome.canvas.CanvasWidget,
      widget=self.left_boat.sw,
      x=x_left,
      y=y,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_N,
      size_pixels=gtk.FALSE)
    self.left_boat.tv.show()
    self.left_boat.sw.show()

    # The Programming input area RIGHT
    self.right_boat.sw = gtk.ScrolledWindow()
    self.right_boat.sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
    self.right_boat.sw.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
      
    self.right_boat.tb = gtk.TextBuffer()
    self.right_boat.tv = gtk.TextView(self.right_boat.tb)
    self.right_boat.sw.add(self.right_boat.tv)

    command_example = _("gauche") + " 45\n" + _("forward") + " 5\n" + _("droite") + " 45"
    self.right_boat.tb.set_text(command_example)
    
    self.right_boat.tv.set_wrap_mode(gtk.WRAP_CHAR)
    self.rootitem.add(
      gnome.canvas.CanvasWidget,
      widget=self.right_boat.sw,
      x=x_right,
      y=y,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_N,
      size_pixels=gtk.FALSE)
    self.right_boat.tv.show()
    self.right_boat.sw.show()

    # Text Labels
    self.left_boat.speeditem = self.rootitem.add (
      gnome.canvas.CanvasText,
      text="",
      font=gcompris.skin.get_font("gcompris/content"),
      x=x_left,
      y=y-15,
      fill_color_rgba=0xFF0000FFL
      )

    self.right_boat.speeditem = self.rootitem.add (
      gnome.canvas.CanvasText,
      text="",
      font=gcompris.skin.get_font("gcompris/content"),
      x=x_right,
      y=y-15,
      fill_color_rgba=0X027308FFL
      )

    # The status area
    self.statusitem = self.rootitem.add (
      gnome.canvas.CanvasText,
      text="",
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH/2,
      y=y-40,
      fill_color_rgba=0XFF66FFFFL
      )

    # The decoration boats
    pixmap = gcompris.utils.load_pixmap("images/top_boat_red.png")
    item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=25,
      y=y+40,
      anchor=gtk.ANCHOR_CENTER,
      )
    gcompris.utils.item_rotate_relative(item, -90);

    pixmap = gcompris.utils.load_pixmap("images/top_boat_green.png")
    item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=gcompris.BOARD_WIDTH-25,
      y=y+40,
      anchor=gtk.ANCHOR_CENTER,
      )
    gcompris.utils.item_rotate_relative(item, -90);

    # The commands
    hl = 18
    y += 7
    text_color = 0x0000FFFFL
    self.rootitem.add (
      gnome.canvas.CanvasText,
      text=_("COMMANDS ARE"),
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH/2,
      y=y,
      fill_color_rgba=text_color
      )
    
    self.rootitem.add (
      gnome.canvas.CanvasText,
      text=_("forward"),
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH/2,
      y=y+hl,
      fill_color_rgba=text_color
      )
    
    self.rootitem.add (
      gnome.canvas.CanvasText,
      text=_("left"),
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH/2,
      y=y+hl*2,
      fill_color_rgba=text_color
      )
    
    self.rootitem.add (
      gnome.canvas.CanvasText,
      text=_("right"),
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH/2,
      y=y+hl*3,
      fill_color_rgba=text_color
      )
    

  # Weather condition is a 2 value pair (angle wind_speed)
  # Weather is a list of the form:
  # (rectangle coordinate) (weather)
  def display_weather(self):

    # Reset the weather list
    self.weather   = []

    # Some constant to define the sea area
    # The sea area is defined in the global self.sea_area
    slice_x = 5 + self.gcomprisBoard.level
    slice_y = 3 + self.gcomprisBoard.level
    
    step_x  = (self.sea_area[2]-self.sea_area[0])/slice_x
    step_y  = (self.sea_area[3]-self.sea_area[1])/slice_y

    stop_x  = self.sea_area[0]+step_x*slice_x
    stop_y  = self.sea_area[1]+step_y*slice_y

    for x in range (self.sea_area[0], stop_x, step_x):
      for y in range (self.sea_area[1], stop_y, step_y):
        #print x, step_x, self.sea_area[2]
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

    #print condition
    # Calc the center
    cx = condition[0][0]+(condition[0][2]-condition[0][0])/2
    cy = condition[0][1]+(condition[0][3]-condition[0][1])/2
    
    pixmap = gcompris.utils.load_pixmap("images/arrow.png")
    item = self.root_weather_item.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=cx,
      y=cy,
      anchor=gtk.ANCHOR_CENTER
      )
    gcompris.utils.item_rotate_relative(item, condition[1][0]);

    # Text number Shadow
    self.root_weather_item.add (
      gnome.canvas.CanvasText,
      text=condition[1][1],
      font=gcompris.skin.get_font("gcompris/content"),
      x=cx+1+pixmap.get_width()/2,
      y=cy+1+pixmap.get_height()/2,
      fill_color_rgba=0x000000FFL
      )
    
    # Text number
    self.root_weather_item.add (
      gnome.canvas.CanvasText,
      text=condition[1][1],
      font=gcompris.skin.get_font("gcompris/content"),
      x=cx+pixmap.get_width()/2,
      y=cy+pixmap.get_height()/2,
      fill_color_rgba=0xFFFFFFFFL
      )
    return

  # Given a boat item, return it's weather condition
  def get_weather_condition(self, boat):
    (x, y)= boat.item.i2w( boat.x, boat.y)

    # Look in the cache to speed the process
    if(boat.condition):
      coord = boat.condition[0]
      condition = boat.condition[1]
      if(boat.x > coord[0] and boat.x < coord[2] and boat.y > coord[1] and boat.y < coord[3]):
        return(condition)
    
    for conditions in self.weather:
      coord = conditions[0]
      condition = conditions[1]
      #print "Testing coord="+str(coord)+" Boat coord x="+str(x)+" y="+str(y)
      if(x > coord[0] and x < coord[2] and y > coord[1] and y < coord[3]):
        boat.condition = conditions
        #print "Found: x="+str(x)+" y="+str(y)+" coord="+str(coord)
        #print "  angle=" + str(int(condition[0])) + "  speed=" + str(condition[1])
        return(condition)
    # Should not happen, return a normal condition anyway
    return(0,1)

  #
  # Boat moving
  # -----------
  def cmd_forward(self, boat, value):
    #    print "Player " + str(boat.player) + " cmd_forward " + str(value) + " dx=" + str(boat.dx) + " dy=" + str(boat.dy)

    if(self.board_paused):
      boat.timer = 0
      return
    
    value -= 1
    if value <= 0:
      # Process next command
      self.race_one_command(boat)
      boat.timer = 0
      return
    
    # Move it
    boat.x += 1
    boat.y += 0

    # We need to convert the coord to the rootitem coordinate to check limits
    (x, y)= boat.item.i2w( boat.x, boat.y)
    if(y<self.sea_area[1]):
      y = self.sea_area[3]
      (boat.x, boat.y)= boat.item.w2i( x, y)
    elif(y>self.sea_area[3]):
      y = self.sea_area[1]
      (boat.x, boat.y)= boat.item.w2i( x, y)
    elif(x>self.sea_area[2]):
      boat.arrived = True

      if(not self.left_boat.won and not self.right_boat.won):
        boat.won = True
           
      if(self.left_boat.won):
        self.statusitem.set(text=_("The Red boat has won"))
      elif(self.right_boat.won):
        self.statusitem.set(text=_("The Green boat has won"))
           
      boat.timer = 0
      return

    condition = self.get_weather_condition(boat)

    boat.item.set(x = boat.x,
                  y = boat.y)

    # Calculate the timer inc depending on the wind angle + speed
    wind_angle = condition[0] - boat.angle
    if(wind_angle>360):
      wind_angle-=360
    elif(wind_angle<-360):
      wind_angle+=360

    if(abs(wind_angle)>180):
      wind_angle=180-(abs(wind_angle)-180)

    # Increase the timer depending on wind force and direction
    angle_pi = wind_angle*math.pi/180

    cx = math.cos(angle_pi)
    penalty=3
    if(cx<0):
      penalty*=2
    wind = cx*condition[1]*-1*penalty
    #print "Player " + str(boat.player) + "  wind_angle=" + str(abs(wind_angle)) + " condition=" + str(condition[1]) + " cx=" + str(cx) + "     wind=" + str(wind)
    boat.speeditem.set(text = _("Angle:") + str(condition[0]) + " " + _("Wind:") + str(int(wind)*-1))
    boat.timer = gtk.timeout_add(int(self.timerinc+wind), self.cmd_forward, boat, value)

    


  # Counter Clock wise rotation (use negative param to turn clock wise)
  def cmd_turn_left(self, boat, value):
    #    print "Player " + str(boat.player) + " turn left " + str(value)

    if(self.board_paused):
      boat.timer = 0
      return
    
    if value == 0:
      # Process next command
      self.race_one_command(boat)
      boat.timer = 0
      return

    turn = 1
    if value > 0:
      turn = -1
      
    boat.angle += turn
    if(boat.angle>360):
      boat.angle-=360
    elif(boat.angle<360):
      boat.angle+=360
      
    value    += turn

    gcompris.utils.item_rotate_relative(boat.item, turn);
    boat.timer = gtk.timeout_add(self.timer_turn, self.cmd_turn_left, boat, value)
  
  # Run the race
  def race_one_command(self, boat):

    if(self.board_paused):
      # Let the user enter commands
      boat.tv.set_editable(True)
      
      boat.line = 0
      boat.timer = 0
      return
    
    a = boat.tb.get_iter_at_line(boat.line)
    b = boat.tb.get_iter_at_line(boat.line)
    b.forward_to_line_end()
    boat.line+=1
    
    if (boat.line > boat.tb.get_line_count()):
      # No more commands to process for this player

      # Let the user enter commands
      boat.tv.set_editable(True)
      
      boat.line = 0
      boat.timer = 0
      return
    
    cmd   = boat.tb.get_text(a, b, gtk.FALSE)
    cmd   = cmd.lstrip("\n\t ")
    cmds  = cmd.split()
    # Manage default cases (no params given)
    if ( len(cmds) == 1 and cmd.startswith(_("forward")) ):
      cmd += " 1"
    elif ( len(cmds) == 1 and cmd.startswith(_("gauche")) ):
      cmd += " 45"
    elif ( len(cmds) == 1 and cmd.startswith(_("droite")) ):
      cmd += " 45"
    elif ( len(cmds) > 2):
      boat.speeditem.set(text=_("Syntax error at line") + " " + str(boat.line) + " (" + cmd + ")")

      # Let the user enter commands
      boat.tv.set_editable(True)
      
      boat.line = 0
      boat.timer = 0
      return
      
    value = int(cmd.split()[1])
    if( cmd.startswith(_("forward"))):
      # Transform the value from user visible sea size to pixels
      value *= self.sea_ratio

      # Initialize the move
      boat.timer = gtk.timeout_add(self.timerinc, self.cmd_forward, boat, value)
    elif( cmd.startswith(_("left"))):
      boat.timer = gtk.timeout_add(self.timerinc, self.cmd_turn_left, boat, value)
    elif( cmd.startswith(_("right"))):
      boat.timer = gtk.timeout_add(self.timerinc, self.cmd_turn_left, boat, value*-1)
    else:
      # Let the user enter commands
      boat.tv.set_editable(True)
      
      boat.line = 0
      boat.timer = 0
      boat.speeditem.set(text=_("Unknown command at line") + " " + str(boat.line) + "(" + cmd + ")")

  # Will return a text string: the tux move
  def tux_move(self):

    # The sea area is defined in the global self.sea_area
    step_x    = (self.sea_area[2]-self.sea_area[0])/20/2
    step_y    = (self.sea_area[3]-self.sea_area[1])/10/2
    
    #for y in range (self.sea_area[1], self.sea_area[3]+1, step_y):
    #for x in range (self.sea_area[0], self.sea_area[2]+1, step_x):
        

    return

#  gcompris - anim
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
from gettext import gettext as _
# PythonTest Board module
import gobject
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gtk
import gtk.gdk
import copy
import math
import time


#class Gcompris_anim:
class Gcompris_anim:
  """The cartoon activity"""
    
  def __init__(self, gcomprisBoard):

    self.gcomprisBoard = gcomprisBoard
    self.empty="empty"

    # These are used to let us restart only after the bonux is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;

    self.DEFAULT_ANCHOR_SIZE	= 8
    self.ANCHOR_COLOR = 0x36ede480
    # anchortype
    self.ANCHOR_NW = 1
    self.ANCHOR_N  = 2
    self.ANCHOR_NE = 3
    self.ANCHOR_E  = 4
    self.ANCHOR_W  = 5
    self.ANCHOR_SW = 6
    self.ANCHOR_S  = 7
    self.ANCHOR_SE = 8
    
    self.anchors = { 'LINE': [ self.ANCHOR_SW , self.ANCHOR_NE ],
                     'RECT': [ self.ANCHOR_N,
                               self.ANCHOR_NE,
                               self.ANCHOR_E,
                               self.ANCHOR_SE,
                               self.ANCHOR_S,
                               self.ANCHOR_SW,
                               self.ANCHOR_W,
                               self.ANCHOR_NW
                               ]
                     }
    self.anchors ['FILL_RECT'] =  self.anchors ['RECT']
    self.anchors ['CIRCLE'] =  self.anchors ['RECT']
    self.anchors ['FILL_CIRCLE'] =  self.anchors ['RECT']
    
                               
    # TOOL SELECTION
    self.tools = [
      ["RECT",           "draw/tool-rectangle.png",       "draw/tool-rectangle_on.png",          gcompris.CURSOR_RECT],
      ["FILL_RECT",      "draw/tool-filledrectangle.png", "draw/tool-filledrectangle_on.png",    gcompris.CURSOR_FILLRECT],
      ["CIRCLE",         "draw/tool-circle.png",          "draw/tool-circle_on.png",             gcompris.CURSOR_CIRCLE],
      ["FILL_CIRCLE",    "draw/tool-filledcircle.png",    "draw/tool-filledcircle_on.png",       gcompris.CURSOR_FILLCIRCLE],
      ["LINE",           "draw/tool-line.png",            "draw/tool-line_on.png",               gcompris.CURSOR_LINE],
      ["DEL",            "draw/tool-del.png",             "draw/tool-del_on.png",                gcompris.CURSOR_DEL],
      ["FILL",           "draw/tool-fill.png",            "draw/tool-fill_on.png",               gcompris.CURSOR_FILL],
      ["SELECT",         "draw/tool-select.png",          "draw/tool-select_on.png",             gcompris.CURSOR_SELECT],
#      ["RAISE",         "draw/tool-up.png",          "draw/tool-up_on.png",             gcompris.CURSOR_DEFAULT],
#      ["LOWER",         "draw/tool-down.png",          "draw/tool-down_on.png",             gcompris.CURSOR_DEFAULT],
#      ["CCW",         "draw/tool-rotation-ccw.png",          "draw/tool-rotation-ccw_on.png",             gcompris.CURSOR_DEFAULT],
#      ["CW",         "draw/tool-rotation-cw.png",          "draw/tool-rotation-cw_on.png",             gcompris.CURSOR_DEFAULT],
         
      ]
    
    self.current_tool=0

    # COLOR SELECTION
    self.colors = [   0x000000FFL, 0x202020FFL, 0x404040FFL, 0x505050FFL, 
                      0x815a38FFL, 0xb57c51FFL, 0xe5a370FFL, 0xfcc69cFFL, 
                      0xb20c0cFFL, 0xea2c2cFFL, 0xf26363FFL, 0xf7a3a3FFL, 
                      0xff6600FFL, 0xff8a3dFFL, 0xfcaf7bFFL, 0xf4c8abFFL, 
                      0x9b8904FFL, 0xd3bc10FFL, 0xf4dd2cFFL, 0xfcee85FFL, 
                      0x255b0cFFL, 0x38930eFFL, 0x56d11dFFL, 0x8fe268FFL, 
                      0x142f9bFFL, 0x2d52e5FFL, 0x667eddFFL, 0xa6b4eaFFL, 
                      0x328989FFL, 0x37b2b2FFL, 0x3ae0e0FFL, 0x96e0e0FFL, 
                      0x831891FFL, 0xc741d8FFL, 0xde81eaFFL, 0xeecdf2FFL, 
                      0x666666FFL, 0x838384FFL, 0xc4c4c4FFL, 0xffffffFFL
                      ]

    self.current_color = 0
    self.current_step = 0

    # selected object
    self.selected = None

    # Drawing area is editing image area
    # Palying area is playing map 
    self.drawing_area = [124.0, 20.0, gcompris.BOARD_WIDTH - 15, gcompris.BOARD_HEIGHT - 78]
    self.playing_area = [ 70.0, 20.0,  gcompris.BOARD_WIDTH - 69, gcompris.BOARD_HEIGHT - 78]

    # Global used for the select event
    self.in_select_ofx = -1
    self.in_select_ofy = -1

    # The error root item
    self.root_erroritem = []

  def start(self):  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=0
    self.gcomprisBoard.number_of_sublevel=0
    
    gcompris.bar_set(0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    # initialisation
    self.draw_tools()
    self.draw_animtools()
    self.draw_colors()
    self.draw_drawing_area(10)
    self.draw_playing_area()
    self.pause(0)

  def end(self):
    # stop the animation
    if self.running:
      self.running = False
      gobject.source_remove(self.timeout)
      self.run_anim()
    
    # Remove the root item removes all the others inside it
    gcompris.set_cursor(gcompris.CURSOR_DEFAULT);
    self.rootitem.destroy()
    self.root_drawingitem.destroy()
    self.root_playingitem.destroy()
        # Delete error previous mark if any
    if(self.root_erroritem):
      self.root_erroritem.destroy()

# Mandatory?
  def pause(self, pause):
    #used to stop the event reception at the end?
    self.board_paused = pause
    return
            
  def repeat(self):
    print("Gcompris_anim repeat.")
            
  def config(self):
    print("Gcompris_anim config.")
              
  def key_press(self, keyval):
    print("got key %i" % keyval)
    return


  # Display the tools
  def draw_tools(self):

    self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("draw/tool-selector.jpg"),
      x=5,
      y=20.0,
      width=30.0
      )

    x1=11.0
    x2=56.0
    y=30.0
    stepy=45

    # Display the tools
    for i in range(0,len(self.tools)):
      if(i%2):
        theX = x2
      else:
        theX = x1
        
      item = self.rootitem.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = gcompris.utils.load_pixmap(self.tools[i][1]),
        x=theX,
        y=y
        )
      item.connect("event", self.tool_item_event, i)
      if i%2:
        y += stepy

    # The last item is select, we select it by default
    self.current_tool = i
    self.old_tool_item = item
    self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[i][2]))
    gcompris.set_cursor(self.tools[i][3]);


  # Event when a tool is selected
  def tool_item_event(self, item, event, tool):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        # unselect object if necessary
        if (self.tools[tool][0] != "SELECT") and (self.selected != None):
          self.selected.item_list[1].hide()
          self.selected = None
          
        # Deactivate old button
        self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[self.current_tool][1]))

        # Activate new button                         
        self.current_tool = tool
        self.old_tool_item = item
        self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[self.current_tool][2]))
        gcompris.set_cursor(self.tools[self.current_tool][3]);

  # Display the color selector
  def draw_colors(self):

    pixmap = gcompris.utils.load_pixmap("draw/color-selector.jpg")

    x = (self.drawing_area[2] - self.drawing_area[0]
         - pixmap.get_width())/2 + self.drawing_area[0]

    color_pixmap_height = pixmap.get_height()

    y = gcompris.BOARD_HEIGHT - color_pixmap_height - 5

    c = 0

    self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=x,
      y=y,
      )

    for i in range(0,10):
      x1=x+26+i*56

      for j in range(0,4):
        c = i*4 +j
        item = self.rootitem.add(
          gnome.canvas.CanvasRect,
          x1=x1 + 26*(j%2),
          y1=y+8 + (color_pixmap_height/2 -6)*(j/2),
          x2=x1 + 24  + 26*(j%2),
          y2=y + color_pixmap_height/2  + (color_pixmap_height/2 -6)*(j/2),
          fill_color_rgba=self.colors[c],
          outline_color_rgba=0x07A3E080L
          )
        
        item.connect("event", self.color_item_event, c)
        if (c==0):
          self.current_color = c
          self.old_color_item = item
          self.old_color_item.set(width_units = 4.0,
                                  outline_color_rgba= 0x16EC3DFFL)

  # Color event
  def color_item_event(self, item, event, color):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        # Deactivate old button
        self.old_color_item.set(width_units = 0.0,
                                outline_color_rgba= 0x144B9DFFL)

        # Activate new button
        self.current_color = color
        self.old_color_item = item
        self.old_color_item.set(width_units = 4.0,
                                outline_color_rgba= 0x16EC3DFFL)


  # Display the drawing area
  def draw_drawing_area(self,step):

    x1=self.drawing_area[0]
    y1=self.drawing_area[1]
    x2=self.drawing_area[2]
    y2=self.drawing_area[3]

    item = self.rootitem.add (
      gnome.canvas.CanvasRect,
      x1=x1,
      y1=y1,
      x2=x2,
      y2=y2,
      fill_color_rgba=0xFFFFFFFFL,
      width_units=2.0,
      outline_color_rgba=0x111199FFL
      )
    item.connect("event", self.create_item_event)

    # The CanvasGroup for the edit space.
    self.root_drawingitem = self.rootitem.add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    self.draw_grid(x1,x2,y1,y2,step)

    self.flash = self.rootitem.add (
      gnome.canvas.CanvasRect,
      x1=x1,
      y1=y1,
      x2=x2,
      y2=y2,
      fill_color_rgba=0xf4dd2c10L,
      width_units=2.0,
      outline_color_rgba=0x111199FFL
      )
    self.flash.hide()

  # Display the drawing area
  def draw_playing_area(self):

    x1=self.playing_area[0]
    y1=self.playing_area[1]
    x2=self.playing_area[2]
    y2=self.playing_area[3]


    # The CanvasGroup for the playing space.
    self.root_playingitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    self.root_playingitem.hide()

    item = self.root_playingitem.add (
      gnome.canvas.CanvasRect,
      x1=x1,
      y1=y1,
      x2=x2,
      y2=y2,
      fill_color_rgba=0xFFFFFFFFL,
      width_units=2.0,
      outline_color_rgba=0x111199FFL
      )
    item.connect("event", self.playing_event)

    run = self.root_playingitem.add(
      gnome.canvas.CanvasPixbuf,
#      pixbuf = gcompris.utils.load_pixmap(self.gcomprisBoard.icon_name),
      pixbuf = gcompris.utils.load_pixmap("boardicons/anim.png"),
#      x = gcompris.BOARD_WIDTH - 60,
      x = 11,
      y = 20 ,
      width = 60,
      height = 60,
      width_set = 1,
      height_set = 1
      )
    run.connect("event", self.playing_event)

    # intervall = 1000 / anim_speed
    self.anim_speed=5

    run = self.root_playingitem.add(
      gnome.canvas.CanvasPixbuf,
#      pixbuf = gcompris.utils.load_pixmap(self.gcomprisBoard.icon_name),
      pixbuf = gcompris.utils.load_pixmap("submarine/up.png"),
#      x = gcompris.BOARD_WIDTH - 60,
      x = 10,
      y = 100 ,
      width = 20,
      height = 20,
      width_set = 1,
      height_set = 1
      )
    run.connect("event", self.speed_event,True)

    self.speed_item = self.root_playingitem.add(
      gnome.canvas.CanvasText,
      text=self.anim_speed,
      font=gcompris.skin.get_font("gcompris/content"),
      x=38,
      y=100,
      anchor=gtk.ANCHOR_CENTER,
      fill_color_rgba=0xFFFFFFFFL
      )


    run = self.root_playingitem.add(
      gnome.canvas.CanvasPixbuf,
#      pixbuf = gcompris.utils.load_pixmap(self.gcomprisBoard.icon_name),
      pixbuf = gcompris.utils.load_pixmap("submarine/down.png"),
#      x = gcompris.BOARD_WIDTH - 60,
      x = 46,
      y = 100,
      width = 20,
      height = 20,
      width_set = 1,
      height_set = 1
      )
    run.connect("event", self.speed_event,False)

    
    self.root_anim = self.root_playingitem.add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    gcompris.utils.item_absolute_move(self.root_anim,
                                      self.playing_area[0]-self.drawing_area[0],
                                      self.playing_area[1]-self.drawing_area[1]
                                      )

  def speed_event(self, item, event, up):
  
    if event.type == gtk.gdk.BUTTON_PRESS:
      if up:
        if self.anim_speed==25:
          return
        else:
          self.anim_speed=self.anim_speed+1
      else:
        if self.anim_speed==1:
          return
        else:
          self.anim_speed=self.anim_speed-1

      gobject.source_remove(self.timeout)
      self.timeout=gobject.timeout_add(1000/self.anim_speed, self.run_anim)
      self.speed_item.set(text=self.anim_speed)
  
  # Draw the grid
  #
  def draw_grid(self, x1, x2, y1, y2, step):

    self.current_step = step
    
    color = 0x1D0DFFFFL

    self.grid = self.rootitem.add (
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    self.grid.hide()
      
    for i in range(x1,x2,step):
      item = self.grid.add (
        gnome.canvas.CanvasLine,
        points=(i , y1, i , y2),
        fill_color_rgba=color,
        width_units=1.0,
        )
      # Clicking on lines let you create object
      item.connect("event", self.create_item_event)
 
    for i in range(y1,y2,step):
      item = self.grid.add (
        gnome.canvas.CanvasLine,
        points=(x1, i, x2 , i),
        fill_color_rgba=color,
        width_units=1.0,
        )
      item.connect("event", self.create_item_event)

  # Given x,y return a new x,y snapped to the grid
  def snap_to_grid(self, x, y):
    result = []
    tmp = round(((x+(self.current_step)) -
               self.drawing_area[0])/self.current_step) - 1
    result.append(float(self.drawing_area[0] + tmp*self.current_step))

    tmp = round(((y+(self.current_step)) -
               self.drawing_area[1])/self.current_step) - 1
    result.append(float(self.drawing_area[1] + tmp*self.current_step))
    return result


  # Event when a click on any item. Perform the move
  def move_item_event(self, item, event):

    if self.tools[self.current_tool][0] != "SELECT":
      return gtk.FALSE

    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        # deactivate the anchors
        if self.selected != None:
          self.selected.item_list[1].hide()
          self.selected=None
      
    #
    # MOUSE DRAG STOP
    # ---------------
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        # activate the anchors
        self.selected=item.get_property("parent")
        self.selected.item_list[1].show()
 
        
        # Reset the in_select_ofx ofset
        self.in_select_ofx = -1
        self.in_select_ofy = -1

        return gtk.TRUE
        
    if event.state & gtk.gdk.BUTTON1_MASK:
      x=event.x
      y=event.y

      #bounds = item.get_bounds()
      bounds = self.get_bounds(item)

      # Save the ofset between the mouse pointer and the upper left corner of the object
      if(self.in_select_ofx == -1):
        self.in_select_ofx = x-bounds[0]
        self.in_select_ofy = y-bounds[1]

      x -= self.in_select_ofx
      y -= self.in_select_ofy

      x,y = self.snap_to_grid(x,y)

      # Check drawing boundaries
      if(x<self.drawing_area[0]):
        x=self.drawing_area[0]
      if(x>(self.drawing_area[2]-(bounds[2]-bounds[0]))):
        x=self.drawing_area[2]-(bounds[2]-bounds[0])
        # We need to realign x cause the bounds values are not precise enough
        x,n = self.snap_to_grid(x,y)
      if(y<self.drawing_area[1]):
        y=self.drawing_area[1]
      if(y>(self.drawing_area[3]-(bounds[3]-bounds[1]))):
        y=self.drawing_area[3]-(bounds[3]-bounds[1])
        # We need to realign y cause the bounds values are not precise enough
        n,y = self.snap_to_grid(x,y)

      print "delta bounded" , x, y

      # Now perform the object move
      #gcompris.utils.item_absolute_move(item.get_property("parent"), x, y)
      self.object_move(
        item.get_property("parent"),
        x-bounds[0],
        y-bounds[1]
        )

      return gtk.TRUE
    
    return gtk.FALSE
          
  # Event when a click on an item happen on fill in type object
  def fillin_item_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "FILL":
          item.set(fill_color_rgba=self.colors[self.current_color])
          return gtk.TRUE
    return gtk.FALSE
          
  # Event when a click on an item happen on border fill type object
  def fillout_item_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "FILL":
          item.set(outline_color_rgba=self.colors[self.current_color])
          return gtk.TRUE
    return gtk.FALSE

  # Del an item and internal struct cleanup
  def del_item(self, item):
    item.get_property("parent").destroy()

  # Event when a click on an item happen
  def del_item_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "DEL":
          self.del_item(item);
          return gtk.TRUE
    return gtk.FALSE

  # Event when an event on the drawing area happen
  def create_item_event(self, item, event):

    if event.type == gtk.gdk.BUTTON_PRESS:
      
      # Delete error previous mark if any
      if(self.root_erroritem):
        self.root_erroritem.destroy()

      if event.button == 1:
        self.newitem = None
        self.newitemgroup = self.root_drawingitem.add(
          gnome.canvas.CanvasGroup,
          x=0.0,
          y=0.0
          )


        if (self.tools[self.current_tool][0] == "DEL" or
            self.tools[self.current_tool][0] == "SELECT" or
            self.tools[self.current_tool][0] == "FILL"):
          # This event is treated in del_item_event to avoid
          # operating on background item and grid
          return gtk.FALSE
        
        elif self.tools[self.current_tool][0] == "LINE":
          
          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y

# un item doit être remplacé par un group contenant:
#     - le group "poignées" avec toutes les poignées
#     - l'item lui même
#
#     ItemGroup:
#        AnchorsGroup
#           ANCHOR_SE
#           .....
#        Item

          self.newitem = self.newitemgroup.add(
            gnome.canvas.CanvasLine,
            points=(self.pos_x, self.pos_y, x, y),          
            fill_color_rgba=self.colors[self.current_color],
            width_units=8.0
            )
          self.newitem.connect("event", self.fillin_item_event)
          self.newitem.connect("event", self.move_item_event)

        elif self.tools[self.current_tool][0] == "RECT":
          
          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y
          
          self.newitem = self.newitemgroup.add(
            gnome.canvas.CanvasRect,
            x1=self.pos_x,
            y1=self.pos_y,
            x2=x,
            y2=y,
            outline_color_rgba=self.colors[self.current_color],
            width_units=4.0
            )
          self.newitem.set_data('empty',True)
          self.newitem.connect("event", self.fillout_item_event)
          self.newitem.connect("event", self.move_item_event)

        elif self.tools[self.current_tool][0] == "FILL_RECT":

          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y
          
          self.newitem = self.newitemgroup.add(
            gnome.canvas.CanvasRect,
            x1=self.pos_x,
            y1=self.pos_y,
            x2=x,
            y2=y,
            fill_color=self.colors[self.current_color],
            fill_color_rgba=self.colors[self.current_color],
            outline_color_rgba=0x000000FFL,
            width_units=1.0
            )
          self.newitem.connect("event", self.fillin_item_event)
          self.newitem.connect("event", self.move_item_event)

        elif self.tools[self.current_tool][0] == "CIRCLE":

          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y
          
          self.newitem = self.newitemgroup.add(
            gnome.canvas.CanvasEllipse,
            x1=self.pos_x,
            y1=self.pos_y,
            x2=x,
            y2=y,
            outline_color_rgba=self.colors[self.current_color],
            width_units=5.0
            )
          self.newitem.set_data('empty',True)
          self.newitem.connect("event", self.fillout_item_event)
          self.newitem.connect("event", self.move_item_event)

        elif self.tools[self.current_tool][0] == "FILL_CIRCLE":

          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y
          
          self.newitem = self.newitemgroup.add(
            gnome.canvas.CanvasEllipse,
            x1=self.pos_x,
            y1=self.pos_y,
            x2=x,
            y2=y,
            fill_color_rgba=self.colors[self.current_color],
            outline_color_rgba=0x000000FFL,
            width_units=1.0
            )
          self.newitem.connect("event", self.fillin_item_event)
          self.newitem.connect("event", self.move_item_event)
          
        if self.newitem != 0:
          self.newitem.connect("event", self.create_item_event)
          self.newitem.connect("event", self.del_item_event)

        anchorgroup=self.newitemgroup.add(
          gnome.canvas.CanvasGroup,
          x=0,
          y=0
          )
        anchorgroup.set_data('anchors',True)
        anchorgroup.hide()
        
        for anchor_type in self.anchors[self.tools[self.current_tool][0]]:
          anchor=anchorgroup.add(
            gnome.canvas.CanvasRect,
            fill_color_rgba=self.ANCHOR_COLOR,
            outline_color_rgba=0x000000FFL,
            width_pixels=1,
            )
          anchor.set_data('anchor_type',anchor_type)
          anchor.connect("event", self.resize_item_event,anchor_type)
          
      return gtk.TRUE

    #
    # MOTION EVENT
    # ------------
    if event.type == gtk.gdk.MOTION_NOTIFY:
      if event.state & gtk.gdk.BUTTON1_MASK:
        x=event.x
        y=event.y
        x,y = self.snap_to_grid(event.x,event.y)
        
        # Check drawing boundaries
        if(event.x<self.drawing_area[0]):
          x=self.drawing_area[0]
        if(event.x>self.drawing_area[2]):
          x=self.drawing_area[2]
        if(event.y<self.drawing_area[1]):
          y=self.drawing_area[1]
        if(event.y>self.drawing_area[3]):
          y=self.drawing_area[3]
          
#        if self.tools[self.current_tool][0] == "LINE":
#          self.newitem.set( points=( self.pos_x, self.pos_y, x, y) )
#        elif (self.tools[self.current_tool][0] == "RECT" or
#              self.tools[self.current_tool][0] == "FILL_RECT" or
#              self.tools[self.current_tool][0] == "CIRCLE" or
#              self.tools[self.current_tool][0] == "FILL_CIRCLE"):
#          self.newitem.set(
#            x2=x,
#            y2=y)

        if self.tools[self.current_tool][0] == "LINE":
          points= self.newitem.get_property("points")
          x1=points[0]
          y1=points[1]
        else:
          x1=self.newitem.get_property("x1")
          y1=self.newitem.get_property("y1")
        self.object_set_size_and_pos(self.newitemgroup,
                                     x1=x1,
                                     y1=y1,
                                     x2=x,
                                     y2=y
                                     )
    #
    # MOUSE DRAG STOP
    # ---------------
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        # We have to remove empty created items (the kid did not drag enough)
        if self.tools[self.current_tool][0] == "LINE":
          # need to delete empty line. self.newitem est l'objet courant
          pass
        elif (self.tools[self.current_tool][0] == "RECT" or
              self.tools[self.current_tool][0] == "FILL_RECT" or
              self.tools[self.current_tool][0] == "CIRCLE" or
              self.tools[self.current_tool][0] == "FILL_CIRCLE"):
          # Oups, empty rect
          #self.del_item(self.newitem)
          pass
        
        print self.tools[self.current_tool][0]
        print self.newitem.get_bounds()
        print self.newitemgroup.get_bounds()

        
        return gtk.TRUE
    return gtk.FALSE


  __list_properties = {'GnomeCanvasText' : [
       "anchor",
#       "attributes",
       "clip",
       "clip-height",
       "clip-width",
       "family",
       "family-set",
#       "fill-color",
       "fill-color-gdk",
       "fill-color-rgba",
       "fill-stipple",
       "font",
       "font-desc",
       "justification",
#      "markup",
       "rise",
       "rise-set",
       "scale",
       "scale-set",
       "size",
       "size-points",
       "size-set",
       "stretch",
       "stretch-set",
       "strikethrough",
       "strikethrough-set",
       "style",
       "style-set",
       "text",
#       "text-height",
#       "text-width",
       "underline",
       "underline-set",
       "variant",
       "variant-set",
       "weight",
       "weight-set",
       "x",
       "x-offset",
       "y",
       "y-offset"
       ],
       'GnomeCanvasRE':[
       "x1",
       "x2",
       "y1",
       "y2",
       "cap-style",
       "dash",
#       "fill-color",
       "fill-color-gdk",
       "fill-color-rgba",
       "fill-stipple",
       "join-style",
       "miterlimit",
#       "outline-color",
       "outline-color-gdk",
       "outline-color-rgba",
       "outline-stipple",
       "width-pixels",
#       "width-units",
       "wind"
       ],
       'GnomeCanvasLine':[
       "arrow-shape-a",
       "arrow-shape-b",
       "arrow-shape-c",
       "cap-style",
#       "fill-color",
       "fill-color-gdk",
       "fill-color-rgba",
       "fill-stipple",
       "first-arrowhead",
       "join-style",
       "last-arrowhead",
       "line-style",
       "points",
       "smooth",
       "spline-steps",
       "width-pixels",
#      "width-units"
       ]
       }

  __list_properties['GnomeCanvasRect'] = __list_properties['GnomeCanvasRE']
  __list_properties['GnomeCanvasEllipse'] = __list_properties['GnomeCanvasRE']

  def AnimShot(self, modele):
      shot = self.root_anim.add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

      # tous ls objets de premiers niveau sont des canvasGroup;
      # il contiennent soit les listes d'objets,
      # soit des associations poignées-objets
      for item in modele.item_list:
        self.clone(shot,item)
      shot.hide()

  def clone(self, parent, item):
    cloned= parent.add(
      gobject.type_from_name(gobject.type_name(item))
      )
    l=0.0,0.0,0.0,0.0,0.0,0.0,
    aff = item.i2c_affine(l)
    cloned.affine_relative(aff)
    try:
      empty=item.get_data('empty')
    except:
      empty=False

    if gobject.type_name(cloned)=="GnomeCanvasGroup":
      for rec_item in item.item_list:
        self.clone(cloned,rec_item)
      
    for param in gobject.list_properties(gobject.type_name(item)):
      try:
        if (not empty) or ('fill' not in param.name):
          cloned.set_property(param.name,item.get_property(param.name))
      except:
        pass

    try:
      anchors=item.get_data('anchors')
    except:
      pass

    if anchors:
      cloned.hide()

    return cloned

  def run_anim(self):
    self.cartoon[self.current_image].hide()
    self.current_image=(self.current_image+1)%(len(self.cartoon))
    if self.running:
      self.cartoon[self.current_image].show()
    return self.running

  def AnimRun(self):
    self.cartoon=self.root_anim.item_list
    if len(self.cartoon)==0:
      print "Mmm... Need to make shots before run anim !!"
      return
    self.rootitem.hide()
    self.root_playingitem.show()
    self.cartoon[0].show()
    self.current_image=0
    self.timeout=gobject.timeout_add(1000/self.anim_speed, self.run_anim)


  def snapshot_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      self.flash.show()
      gobject.timeout_add(100, self.run_flash)
      self.AnimShot(self.root_drawingitem)

  def run_flash(self):
    self.flash.hide()
    return False

  def playing_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if not self.running:
        self.running=True
	self.AnimRun()
      else:
        self.running=False
        self.root_playingitem.hide()
        self.rootitem.show()


  # Display the animation tools
  def draw_animtools(self):

    snapshot = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("anim/camera.png"),
#      x = gcompris.BOARD_WIDTH - 60,
      x = 11,
      y = 50 + len(self.tools)/2*45 ,
      width = 60,
      height = 60,
      width_set = 1,
      height_set = 1
      )
    snapshot.connect("event", self.snapshot_event)

    self.running = False

    run = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("boardicons/anim.png"),
#      x = gcompris.BOARD_WIDTH - 60,
      x = 11,
      y = 110 + len(self.tools)/2*45 ,
      width = 60,
      height = 60,
      width_set = 1,
      height_set = 1
      )
    run.connect("event", self.playing_event)

  def object_move(self,object,dx,dy):
    if gobject.type_name(object.item_list[0])=="GnomeCanvasLine":
      (x1,y1,x2,y2)=object.item_list[0].get_property('points')
    else:
      x1=object.item_list[0].get_property("x1")
      y1=object.item_list[0].get_property("y1")
      x2=object.item_list[0].get_property("x2")
      y2=object.item_list[0].get_property("y2")

    self.object_set_size_and_pos(object, x1+dx, y1+dy, x2+dx, y2+dy)

  def object_set_size_and_pos(self, object, x1, y1, x2, y2):
    if gobject.type_name(object.item_list[0])=="GnomeCanvasLine":
      object.item_list[0].set(
        points=(x1,y1,x2,y2)
        )
    else:
      object.item_list[0].set(
        x1=x1,
        x2=x2,
        y1=y1,
        y2=y2,
        )
      
    for anchor in object.item_list[1].item_list:
      anchor_type = anchor.get_data('anchor_type')

      if anchor_type == self.ANCHOR_N:
        anchor.set(
          x1= (x1 + x2 - self.DEFAULT_ANCHOR_SIZE)/2,
          x2= (x1 + x2 + self.DEFAULT_ANCHOR_SIZE)/2,
          y1= y2,
          y2= y2 + self.DEFAULT_ANCHOR_SIZE
          )
      elif anchor_type == self.ANCHOR_NE:
        anchor.set(
          x1= x2,
          x2= x2 + self.DEFAULT_ANCHOR_SIZE,
          y1= y2,
          y2= y2 + self.DEFAULT_ANCHOR_SIZE
          )
      elif anchor_type == self.ANCHOR_E:
        anchor.set(
          x1= x2,
          x2= x2 + self.DEFAULT_ANCHOR_SIZE,
          y1= (y1 + y2 - self.DEFAULT_ANCHOR_SIZE)/2,
          y2= (y1 + y2 + self.DEFAULT_ANCHOR_SIZE)/2
          )
      elif anchor_type == self.ANCHOR_SE:
        anchor.set(
          x1= x2,
          x2= x2 + self.DEFAULT_ANCHOR_SIZE,
          y1= y1,
          y2= y1 - self.DEFAULT_ANCHOR_SIZE
          )
      elif anchor_type == self.ANCHOR_S:
        anchor.set(
          x1= (x1 + x2 - self.DEFAULT_ANCHOR_SIZE)/2,
          x2= (x1 + x2 + self.DEFAULT_ANCHOR_SIZE)/2,
          y1= y1,
          y2= y1 - self.DEFAULT_ANCHOR_SIZE
          )
      elif anchor_type == self.ANCHOR_SW:
        anchor.set(
          x1= x1,
          x2= x1 - self.DEFAULT_ANCHOR_SIZE,
          y1= y1,
          y2= y1 - self.DEFAULT_ANCHOR_SIZE
          )
      elif anchor_type == self.ANCHOR_W:
        anchor.set(
          x1= x1,
          x2= x1 - self.DEFAULT_ANCHOR_SIZE,
          y1= (y1 + y2 - self.DEFAULT_ANCHOR_SIZE)/2,
          y2=  (y1 + y2 + self.DEFAULT_ANCHOR_SIZE)/2,
          )
      elif anchor_type == self.ANCHOR_NW:
        anchor.set(
          x1= x1,
          x2= x1 - self.DEFAULT_ANCHOR_SIZE,
          y1= y2,
          y2= y2 + self.DEFAULT_ANCHOR_SIZE
          )

      
  def resize_item_event(self, item, event, anchor_type):
    if event.state & gtk.gdk.BUTTON1_MASK:
      # warning: anchor is in a group of anchors, wich is in the object group
      parent=item.get_property("parent").get_property("parent")
      real_item=parent.item_list[0]

      x=event.x
      y=event.y

      if gobject.type_name(real_item)=="GnomeCanvasLine":
        points= real_item.get_property("points")
        x1=points[0]
        y1=points[1]
        x2=points[2]
        y2=points[3]
      else:
        x1=real_item.get_property("x1")
        y1=real_item.get_property("y1")
        x2=real_item.get_property("x2")
        y2=real_item.get_property("y2")

      if (anchor_type == self.ANCHOR_N):
        self.object_set_size_and_pos(parent,
                                     x1=x1,
                                     y1=y1,
                                     x2=x2,
                                     y2=y
                                     )
      elif (anchor_type == self.ANCHOR_NE):
        self.object_set_size_and_pos(parent,
                                     x1=x1,
                                     y1=y1,
                                     x2=x,
                                     y2=y
                                     )
      elif (anchor_type == self.ANCHOR_E):
        self.object_set_size_and_pos(parent,
                                     x1=x1,
                                     y1=y1,
                                     x2=x,
                                     y2=y2
                                     )
      elif (anchor_type == self.ANCHOR_SE):
        self.object_set_size_and_pos(parent,
                                     x1=x1,
                                     y1=y,
                                     x2=x,
                                     y2=y2
                                     )
      elif (anchor_type == self.ANCHOR_S):
        self.object_set_size_and_pos(parent,
                                     x1=x1,
                                     y1=y,
                                     x2=x2,
                                     y2=y2
                                     )
      elif (anchor_type == self.ANCHOR_SW):
        self.object_set_size_and_pos(parent,
                                     x1=x,
                                     y1=y,
                                     x2=x2,
                                     y2=y2
                                     )
      elif (anchor_type == self.ANCHOR_W):
        self.object_set_size_and_pos(parent,
                                     x1=x,
                                     y1=y1,
                                     x2=x2,
                                     y2=y2
                                     )
      elif (anchor_type == self.ANCHOR_NW):
        self.object_set_size_and_pos(parent,
                                     x1=x,
                                     y1=y1,
                                     x2=x2,
                                     y2=y
                                     )


  def get_bounds(self, item):
    if gobject.type_name(item)=="GnomeCanvasLine":
      (x1,y1,x2,y2)=item.get_property("points")
    else:
      x1=item.get_property("x1")
      y1=item.get_property("y1")
      x2=item.get_property("x2")
      y2=item.get_property("y2")

    return (min(x1,x2),min(y1,y2),max(x1,x2),max(y1,y2))
      

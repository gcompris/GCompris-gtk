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
import gtk.keysyms
import copy
import math
import time

# TO BE REMOVED
#import sys
#sys.path.append("/home/yves/GCompris")
#import animutils

#Print
import os
import tempfile

fles=None

#class Gcompris_anim:
class Gcompris_anim:
  """The cartoon activity"""
    
  def __init__(self, gcomprisBoard):

    self.gcomprisBoard = gcomprisBoard
    self.empty="empty"
    global fles
    fles=self

    # These are used to let us restart only after the bonux is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    self.MAX_TEXT_CHAR = 50
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
                               ],
                     'TEXT': [ self.ANCHOR_N ]
                     }
    self.anchors ['FILL_RECT'] =  self.anchors ['RECT']
    self.anchors ['CIRCLE'] =  self.anchors ['RECT']
    self.anchors ['FILL_CIRCLE'] =  self.anchors ['RECT']
    self.anchors ['IMAGE'] =  self.anchors ['RECT']
#    self.anchors ['TEXT'] =  self.anchors ['RECT']


    self.events = { 'LINE' : [ self.fillin_item_event,
                               self.move_item_event,
                               self.create_item_event,
                               self.del_item_event ] ,
                    'RECT' : [ self.fillout_item_event,
                               self.move_item_event,
                               self.create_item_event,
                               self.del_item_event ],
                    'TEXT' : [ self.fillin_item_event,
                               self.move_item_event,
                               self.create_item_event,
                               self.del_item_event ],
                    'IMAGE' : [ self.move_item_event,
                                self.create_item_event,
                                self.del_item_event ]
                    }
    
    self.events ['FILL_RECT']         = self.events ['LINE']
    self.events ['FILL_CIRCLE']       = self.events ['LINE']
    self.events ['CIRCLE']  = self.events ['RECT']
#    self.events ['TEXT']  = self.events ['IMAGE']
                               
    # TOOL SELECTION
    self.tools = [
      ["SAVE",           "draw/tool-save.png",            "draw/tool-save.png",                  gcompris.CURSOR_SELECT],
      ["LOAD",           "draw/tool-load.png",            "draw/tool-load.png",                  gcompris.CURSOR_SELECT],  
      ["MOVIE",          "draw/tool-movie.png",           "draw/tool-movie_on.png",              gcompris.CURSOR_SELECT],
      ["PICTURE",        "draw/tool-camera.png",          "draw/tool-camera_on.png",             gcompris.CURSOR_SELECT],  
      ["RECT",           "draw/tool-rectangle.png",       "draw/tool-rectangle_on.png",          gcompris.CURSOR_RECT],
      ["FILL_RECT",      "draw/tool-filledrectangle.png", "draw/tool-filledrectangle_on.png",    gcompris.CURSOR_FILLRECT],
      ["CIRCLE",         "draw/tool-circle.png",          "draw/tool-circle_on.png",             gcompris.CURSOR_CIRCLE],
      ["FILL_CIRCLE",    "draw/tool-filledcircle.png",    "draw/tool-filledcircle_on.png",       gcompris.CURSOR_FILLCIRCLE],
      ["LINE",           "draw/tool-line.png",            "draw/tool-line_on.png",               gcompris.CURSOR_LINE],
      ["FILL",           "draw/tool-fill.png",            "draw/tool-fill_on.png",               gcompris.CURSOR_FILL],
      ["DEL",            "draw/tool-del.png",             "draw/tool-del_on.png",                gcompris.CURSOR_DEL],
      ["SELECT",         "draw/tool-select.png",          "draw/tool-select_on.png",             gcompris.CURSOR_SELECT],
      ["RAISE",          "draw/tool-up.png",              "draw/tool-up_on.png",                 gcompris.CURSOR_DEFAULT],
      ["LOWER",          "draw/tool-down.png",            "draw/tool-down_on.png",               gcompris.CURSOR_DEFAULT],
      ["CCW",            "draw/tool-rotation-ccw.png",    "draw/tool-rotation-ccw_on.png",       gcompris.CURSOR_DEFAULT],
      ["CW",             "draw/tool-rotation-cw.png",     "draw/tool-rotation-cw_on.png",        gcompris.CURSOR_DEFAULT],
      ["FLIP",           "draw/tool-flip.png",            "draw/tool-flip_on.png",               gcompris.CURSOR_DEFAULT],
      ["TEXT",           "draw/tool-text.png",            "draw/tool-text_on.png",               gcompris.CURSOR_LINE],
      ["IMAGE",          "draw/tool-image.png",           "draw/tool-image_on.png",              gcompris.CURSOR_DEFAULT],
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
    self.playing_area = [124.0, 20.0, gcompris.BOARD_WIDTH - 15, gcompris.BOARD_HEIGHT - 78]

    # Global used for the select event
    self.in_select_ofx = -1
    self.in_select_ofy = -1

    # The error root item
    self.root_erroritem = []

    # The frame counter
    self.item_frame_counter = []
    self.current_image = 0

    # The root items
    self.root_coloritem = []
    self.root_toolitem  = []
    
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
    
    if (keyval == gtk.keysyms.F1):
      gcompris.file_selector_save( self.gcomprisBoard, "anim", svg_save)
    elif (keyval == gtk.keysyms.F2):
      gcompris.file_selector_load( self.gcomprisBoard, "anim", svg_restore)

    elif (keyval == gtk.keysyms.F3):
      self.ps_print(self.root_drawingitem)
    
    elif ((keyval == gtk.keysyms.Shift_L) or
          (keyval == gtk.keysyms.Shift_R) or
          (keyval == gtk.keysyms.Control_L) or
          (keyval == gtk.keysyms.Control_R) or
          (keyval == gtk.keysyms.Caps_Lock) or
          (keyval == gtk.keysyms.Shift_Lock) or
          (keyval == gtk.keysyms.Meta_L) or
          (keyval == gtk.keysyms.Meta_R) or
          (keyval == gtk.keysyms.Alt_L) or
          (keyval == gtk.keysyms.Alt_R) or
          (keyval == gtk.keysyms.Super_L) or
          (keyval == gtk.keysyms.Super_R) or
          (keyval == gtk.keysyms.Hyper_L) or
          (keyval == gtk.keysyms.Hyper_R) or
          (keyval == gtk.keysyms.Mode_switch) or
          (keyval == gtk.keysyms.dead_circumflex) or
          (keyval == gtk.keysyms.Num_Lock)):
      return gtk.FALSE
    if (keyval == gtk.keysyms.KP_0):
      keyval= gtk.keysyms._0
    if (keyval == gtk.keysyms.KP_1):
      keyval= gtk.keysyms._1
    if (keyval == gtk.keysyms.KP_2):
      keyval= gtk.keysyms._2
    if (keyval == gtk.keysyms.KP_2):
      keyval= gtk.keysyms._2
    if (keyval == gtk.keysyms.KP_3):
      keyval= gtk.keysyms._3
    if (keyval == gtk.keysyms.KP_4):
      keyval= gtk.keysyms._4
    if (keyval == gtk.keysyms.KP_5):
      keyval= gtk.keysyms._5
    if (keyval == gtk.keysyms.KP_6):
      keyval= gtk.keysyms._6
    if (keyval == gtk.keysyms.KP_7):
      keyval= gtk.keysyms._7
    if (keyval == gtk.keysyms.KP_8):
      keyval= gtk.keysyms._8
    if (keyval == gtk.keysyms.KP_9):
      keyval= gtk.keysyms._9

    if (self.selected == None):
      return True
    elif (gobject.type_name(self.selected.item_list[0])!="GnomeCanvasText"):
      print "Not Text object when got key !!!"
      return True

    textItem = self.selected.item_list[0]
    oldtext = textItem.get_property('text')
    print oldtext
    
    if ((keyval == gtk.keysyms.BackSpace) or
        (keyval == gtk.keysyms.Delete)):
      print "DEL", oldtext, len(oldtext)
      if (len(oldtext) != 1):
        newtext = oldtext[:-1]
      else:
        newtext = u'?'
    else:
    
      utf8char=gtk.gdk.keyval_to_unicode(keyval)
      str = u'%c' % utf8char

      print "str", str
      
      if ((oldtext[:1] == u'?') and (len(oldtext)==1)):
        oldtext = u' '
        oldtext = oldtext.strip()

      if (len(oldtext) < self.MAX_TEXT_CHAR):
        newtext = oldtext + str
      else:
        newtext = oldtext

    print "newtext", newtext
    
      
    textItem.set(text=newtext)
    self.updated_text(textItem)

    return True
  
  # Display the tools
  def draw_tools(self):

    self.root_toolitem = self.rootitem.add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.root_toolitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("draw/tool-selector.jpg"),
      x=5,
      y=5.0,
      width=100.0,
      height=458.0,
      width_set=True,
      height_set=True
      )

    x1=11.0
    x2=56.0
    y=11.0
    stepy=45

    # Display the tools
    for i in range(0,len(self.tools)):
      if(i%2):
        theX = x2
      else:
        theX = x1
        
      item = self.root_toolitem.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = gcompris.utils.load_pixmap(self.tools[i][1]),
        x=theX,
        y=y
        )
      item.connect("event", self.tool_item_event, i)
      if i%2:
        y += stepy

      if(self.tools[i][0]=="SELECT"):
        self.select_tool = item
        self.select_tool_number = i
        # Always select the SELECT item by default
        self.current_tool = i
        self.old_tool_item = item
        self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[i][2]))
        gcompris.set_cursor(self.tools[i][3]);


  # Event when a tool is selected
  # Perform instant action or swich the tool selection
  def tool_item_event(self, item, event, tool):

    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        # Some button have instant effects
        if (self.tools[tool][0] == "SAVE"):
          gcompris.file_selector_save( self.gcomprisBoard, "anim", svg_save)
          
        elif (self.tools[tool][0] == "LOAD"):
          gcompris.file_selector_load( self.gcomprisBoard, "anim", svg_restore)
          
        elif (self.tools[tool][0] == "IMAGE"):
          self.pos_x = gcompris.BOARD_WIDTH/2
          self.pos_y = gcompris.BOARD_HEIGHT/2

          gcompris.images_selector_start(self.gcomprisBoard,
                                         gcompris.DATA_DIR + "/dataset/mrpatate.xml",
                                         image_selected);
          return gtk.TRUE
          
        elif (self.tools[tool][0] == "PICTURE"):
          self.AnimShot(self.root_drawingitem)
          return gtk.TRUE
        
        elif (self.tools[tool][0] == "MOVIE"):
          if not self.running:
            self.playing_start()
          else:
            # unselect object if necessary
            if (self.selected != None):
              self.selected.item_list[1].hide()
              self.selected = None
     
              # Deactivate old button
              self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[self.current_tool][1]))
        
              # Activate new button                         
              self.current_tool = self.select_tool_number
              self.old_tool_item = self.select_tool
              self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[self.current_tool][2]))
              gcompris.set_cursor(self.tools[self.current_tool][3]);
              self.playing_stop()
              return gtk.TRUE
            
            # unselect object if necessary
        elif (self.tools[tool][0] != "SELECT") and (self.selected != None):
          self.selected.item_list[1].hide()
          self.selected = None

        #
        # Normal case, tool button switch
        # -------------------------------
        
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

    self.root_coloritem = self.rootitem.add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    
    self.root_coloritem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=x,
      y=y,
      )

    for i in range(0,10):
      x1=x+26+i*56

      for j in range(0,4):
        c = i*4 +j
        item = self.root_coloritem.add(
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
    if self.running:
      return
    
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
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("anim/camera.png"),
      x=300,
      y=200,
      )
    self.flash.hide()

  # Display the drawing area
  def draw_playing_area(self):

    x1=self.playing_area[0]
    y1=self.playing_area[1]
    x2=self.playing_area[2]
    y2=self.playing_area[3]


    # The CanvasGroup for the playing area.
    self.root_playingitem = self.rootitem.add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    self.root_playingitem.hide()

    # intervall = 1000 / anim_speed
    self.anim_speed=5

    run = self.root_playingitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("submarine/down.png"),
      x = 15,
      y = 410,
      width = 20,
      height = 20,
      width_set = 1,
      height_set = 1
      )
    run.connect("event", self.speed_event,False)

    self.speed_item = self.root_playingitem.add(
      gnome.canvas.CanvasText,
      text=self.anim_speed,
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x=52,
      y=420,
      anchor=gtk.ANCHOR_CENTER,
      )


    run = self.root_playingitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("submarine/up.png"),
      x = 70,
      y = 410,
      width = 20,
      height = 20,
      width_set = 1,
      height_set = 1
      )
    run.connect("event", self.speed_event,True)

    
    self.root_anim = self.rootitem.add(
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
    if self.tools[self.current_tool][0] == "CCW":
      if event.type == gtk.gdk.BUTTON_PRESS:
        #gcompris.utils.item_rotate_relative(item.get_property("parent"), -10);
        self.item_rotate_relative(item, -10);
        return gtk.TRUE
      else:
        return gtk.FALSE

    if self.tools[self.current_tool][0] == "CW":
      if event.type == gtk.gdk.BUTTON_PRESS:
        #gcompris.utils.item_rotate_relative(item.get_property("parent"), 10);
        self.item_rotate_relative(item, 10);
        return gtk.TRUE
      else:
        return gtk.FALSE
    if self.tools[self.current_tool][0] == "FLIP":
      if event.type == gtk.gdk.BUTTON_PRESS:
        self.item_flip(item);
        return gtk.TRUE
      else:
        return gtk.FALSE

    if self.tools[self.current_tool][0] == "RAISE":
      if event.type == gtk.gdk.BUTTON_PRESS:
        item.get_property("parent").raise_to_top()
        return gtk.TRUE
      else:
        return gtk.FALSE

    if self.tools[self.current_tool][0] == "LOWER":
      if event.type == gtk.gdk.BUTTON_PRESS:
        item.get_property("parent").lower(1)
        return gtk.TRUE
      else:
        return gtk.FALSE
    
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
      wx=event.x
      wy=event.y
      #pass in item relative coordinate
      (x, y)= item.w2i( wx, wy)

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
      #needs to be corrected with item relative coordinate
      # FIXME
#      if(x<self.drawing_area[0]):
#        x=self.drawing_area[0]
#      if(x>(self.drawing_area[2]-(bounds[2]-bounds[0]))):
#        x=self.drawing_area[2]-(bounds[2]-bounds[0])
        # We need to realign x cause the bounds values are not precise enough
#        x,n = self.snap_to_grid(x,y)
#      if(y<self.drawing_area[1]):
#        y=self.drawing_area[1]
#      if(y>(self.drawing_area[3]-(bounds[3]-bounds[1]))):
#        y=self.drawing_area[3]-(bounds[3]-bounds[1])
        # We need to realign y cause the bounds values are not precise enough
#        n,y = self.snap_to_grid(x,y)

      # Now perform the object move
      #gcompris.utils.item_absolute_move(item.get_property("parent"), x, y)
      # pass it in item coordinate:
      #(idx, idy) =  item.w2i( x-bounds[0], y-bounds[1] )
      (idx, idy) =  ( x-bounds[0], y-bounds[1] )
      self.object_move(
        item.get_property("parent"),
        idx,
        idy
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
    if(event.type == gtk.gdk.BUTTON_PRESS and self.running==True):
      self.playing_stop()
      return gtk.FALSE
    
    # Right button is a shortcup to Shot
    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 3:
      self.AnimShot(self.root_drawingitem)
      return gtk.FALSE
    
    if (not (self.tools[self.current_tool][0] == "RECT" or
             self.tools[self.current_tool][0] == "CIRCLE" or
             self.tools[self.current_tool][0] == "FILL_RECT" or
             self.tools[self.current_tool][0] == "FILL_CIRCLE" or
             self.tools[self.current_tool][0] == "IMAGE" or
             self.tools[self.current_tool][0] == "TEXT" or
             self.tools[self.current_tool][0] == "LINE")):
      return gtk.FALSE

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
        elif self.tools[self.current_tool][0] == "TEXT":

          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y

          self.newitem = self.newitemgroup.add(
            gnome.canvas.CanvasText,
            x=self.pos_x,
            y=self.pos_y,
            fill_color_rgba=self.colors[self.current_color],
            font=gcompris.FONT_BOARD_BIG_BOLD,
            text=u'?',
            anchor=gtk.ANCHOR_CENTER
            )
                  
        if self.newitem != 0:
          self.anchorize(self.newitemgroup)
        
        if self.tools[self.current_tool][0] == "TEXT":
          self.updated_text(self.newitem)
          (x1, x2, y1, y2) = self.get_bounds(self.newitem)
          self.object_set_size_and_pos(self.newitemgroup, x1, x2, y1, y2)
          self.select_item(self.newitemgroup)

      return gtk.TRUE

    #
    # MOTION EVENT
    # ------------
    if event.type == gtk.gdk.MOTION_NOTIFY:
      if ((self.tools[self.current_tool][0] == "IMAGE") or
          (self.tools[self.current_tool][0] == "TEXT")):
        return gtk.FALSE
      
      if event.state & gtk.gdk.BUTTON1_MASK:
        if (self.tools[self.current_tool][0] == "RAISE" or
            self.tools[self.current_tool][0] == "LOWER"):
          return gtk.FALSE
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
      if ((self.tools[self.current_tool][0] == "IMAGE") or
          (self.tools[self.current_tool][0] == "TEXT")):
        return gtk.FALSE
      
      if event.button == 1:
        if (self.tools[self.current_tool][0] == "RAISE" or
            self.tools[self.current_tool][0] == "LOWER"):
          return gtk.FALSE
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
        
#        print self.tools[self.current_tool][0]
#        print self.newitem.get_bounds()
#        print self.newitemgroup.get_bounds()

        return gtk.TRUE
    return gtk.FALSE

  # tous les objets de premiers niveau sont des canvasGroup;
  # il contiennent soit les listes d'objets,
  # soit des associations poignées-objets
  def AnimShot(self, modele):
    self.flash.show()
    
    shot = self.root_anim.add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    for item in modele.item_list:
      #animutils.clone_item(shot,item)
      item2 = gcompris.utils.clone_item(item, shot)
      # Set back the callback so that we will be abble to edit the image again
      # FIXME Do not work. How to do this ?
      #self.anchorize(item2)
      
    shot.hide()
      
    self.current_image+=1

    self.item_frame_counter.set(text=self.current_image + 1)
    gtk.timeout_add(500, self.run_flash)

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
      self.item_frame_counter.set(text=self.current_image + 1)
    return self.running

  def AnimRun(self):
    self.cartoon=self.root_anim.item_list
    if len(self.cartoon)==0:
      print "Mmm... Need to make shots before run anim !!"
      self.running=False
      svg_restore("gcompris.svg")
      return
    self.cartoon[0].show()
    self.current_image=0
    self.timeout=gobject.timeout_add(1000/self.anim_speed, self.run_anim)


  def snapshot_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      self.AnimShot(self.root_drawingitem)

  def run_flash(self):
    self.flash.hide()
    return False

  def playing_start(self):
    if not self.running:
      self.running=True
      self.root_coloritem.hide()
      self.root_toolitem.hide()
      self.root_drawingitem.hide()
      self.root_playingitem.show()
      self.AnimRun()
      
  def playing_stop(self):
    if self.running:
      self.running=False
      self.root_playingitem.hide()
      self.root_drawingitem.show()
      self.root_coloritem.show()
      self.root_toolitem.show()
      self.item_frame_counter.set(text=len(self.cartoon)+1)

  def playing_event(self, item, event, state):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if state:
        self.playing_start()
      else:
        self.playing_stop()

  def image_select_event(self, item, event, state):
    if self.running:
      return

    if event.type == gtk.gdk.BUTTON_PRESS:
      self.cartoon=self.root_anim.item_list
      self.root_anim.show()
      self.cartoon[self.current_image].hide()
      if state == "first":
        self.current_image = 0
      elif state == "last":
        self.current_image = len(self.cartoon)-1
      elif state == "previous":
        self.current_image=(self.current_image-1)%(len(self.cartoon))
      elif state == "next":
        self.current_image=(self.current_image+1)%(len(self.cartoon))

      print "self.current_image=" + str(self.current_image)
      self.cartoon[self.current_image].show()
      self.item_frame_counter.set(text=self.current_image + 1)

  # Display the animation tools
  def draw_animtools(self):
    x_left = 8
    y_top  = 472
    minibutton_width = 32
    minibutton_height = 20

    # Draw the background area
    self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("draw/tool-selector.jpg"),
      x=x_left-3,
      y=y_top - 6,
      width=100.0,
      height=47.0,
      width_set=True,
      height_set=True
      )

    # First
    item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("anim/minibutton.png"),
      x = x_left,
      y = y_top,
      )
    item.connect("event", self.image_select_event, "first")
    item = self.rootitem.add(
      gnome.canvas.CanvasText,
      text = "<<",
      x = x_left + 14,
      y = y_top + 7,
      )
    item.connect("event", self.image_select_event, "first")

    # Image Number
    self.item_frame_counter = self.rootitem.add(
      gnome.canvas.CanvasText,
      text = self.current_image + 1,
      x = x_left + minibutton_width + 14,
      y = y_top + 15,
      font = gcompris.skin.get_font("gcompris/board/medium"))

    # Last
    item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("anim/minibutton.png"),
      x = x_left + 2*minibutton_width,
      y = y_top,
      )
    item.connect("event", self.image_select_event, "last")
    item = self.rootitem.add(
      gnome.canvas.CanvasText,
      text = ">>",
      x = x_left + 2*minibutton_width + 14,
      y = y_top + 7,
      )
    item.connect("event", self.image_select_event, "last")

    # Next line
    y_top += minibutton_height
    
    # Previous
    item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("anim/minibutton.png"),
      x = x_left,
      y = y_top,
      )
    item.connect("event", self.image_select_event, "previous")
    item = self.rootitem.add(
      gnome.canvas.CanvasText,
      text = "<",
      x = x_left + 14,
      y = y_top + 7,
      )
    item.connect("event", self.image_select_event, "previous")

    # Next
    item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("anim/minibutton.png"),
      x = x_left + 2*minibutton_width,
      y = y_top,
      )
    item.connect("event", self.image_select_event, "next")
    item = self.rootitem.add(
      gnome.canvas.CanvasText,
      text = ">",
      x = x_left + 2*minibutton_width + 14,
      y = y_top + 7,
      )
    item.connect("event", self.image_select_event, "next")

    # Last button line
    y_top += minibutton_height

    self.running = False

  def object_move(self,object,dx,dy):
    (x1,y1,x2,y2) = self.get_bounds(object.item_list[0])

    self.object_set_size_and_pos(object, x1+dx, y1+dy, x2+dx, y2+dy)


  def object_set_size_and_pos(self, object, x1, y1, x2, y2):
    if gobject.type_name(object.item_list[0])=="GnomeCanvasLine":
      object.item_list[0].set(
        points=(x1,y1,x2,y2)
        )
    elif gobject.type_name(object.item_list[0])=="GnomeCanvasPixbuf":
      object.item_list[0].set(
        x=x1,
        y=y1,
        width=x2-x1,
        height=y2-y1
        )
    elif gobject.type_name(object.item_list[0])=="GnomeCanvasText":
      object.item_list[0].set(
        x=(x1+x2)/2,
        y=(y1+y2)/2
        )
    else:
      object.item_list[0].set(
        x1=x1,
        x2=x2,
        y1=y1,
        y2=y2
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
    if self.running:
      return

    # Right button is a shortcup to Shot
    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 3:
      self.AnimShot(self.root_drawingitem)
      return gtk.FALSE
    
    if event.state & gtk.gdk.BUTTON1_MASK:
      # warning: anchor is in a group of anchors, wich is in the object group
      parent=item.get_property("parent").get_property("parent")
      real_item=parent.item_list[0]
      if gobject.type_name(real_item)=="GnomeCanvasText":
        return

      wx=event.x
      wy=event.y
      #passing x, y to item relative coordinate
      (x,y)= item.w2i(wx,wy)

      if gobject.type_name(real_item)=="GnomeCanvasLine":
        points= real_item.get_property("points")
        x1=points[0]
        y1=points[1]
        x2=points[2]
        y2=points[3]
      elif gobject.type_name(real_item)=="GnomeCanvasPixbuf":
        x1=real_item.get_property("x")
        y1=real_item.get_property("y")
        x2=x1+real_item.get_property("width")
        y2=y1+real_item.get_property("height")
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
    elif gobject.type_name(item)=="GnomeCanvasPixbuf":
      x1=item.get_property("x")
      y1=item.get_property("y")
      x2=item.get_property("x")+item.get_property("width")
      y2=item.get_property("y")+item.get_property("height")
    elif gobject.type_name(item)=="GnomeCanvasText":
      x=item.get_property("x")
      y=item.get_property("y")
      width=item.get_property("text_width")
      height=item.get_property("text_height")
      x1=x-width/2
      y1=y-height/2
      x2=x1+width
      y2=y1+height
    else:
      x1=item.get_property("x1")
      y1=item.get_property("y1")
      x2=item.get_property("x2")
      y2=item.get_property("y2")

    return (min(x1,x2),min(y1,y2),max(x1,x2),max(y1,y2))

    
  def item_type(self, item):
    if gobject.type_name(item)=="GnomeCanvasLine":
      item_type='LINE'
    elif gobject.type_name(item)=="GnomeCanvasPixbuf":
      item_type='IMAGE'
    elif gobject.type_name(item)=="GnomeCanvasRect":
      try:
        empty = item.get_data('empty')
        if empty == None:
          empty = Fale
        else:
          empty = True
        # empty is passed from C, not python object
        # if we get it that means is True
      except:
        empty = False
        
      if empty:
        item_type='RECT'
      else:
        item_type='FILL_RECT'

    elif gobject.type_name(item)=="GnomeCanvasEllipse":
      try:
        empty = item.get_data('empty')
        if empty == None:
          empty = Fale
        else:
          empty = True
        # empty is passed from C, not python object
        # if we get it that means is True
      except:
        empty = False

      if empty:
        item_type='CIRCLE'
      else:
        item_type='FILL_CIRCLE'

    elif gobject.type_name(item)=="GnomeCanvasText":
      item_type='TEXT'


    return item_type


  def anchorize( self, group):
    # group contains normal items.

    item = group.item_list[0]

    item_type = self.item_type(item)

    for event in self.events[item_type]:
      item.connect("event", event)

    anchorgroup=group.add(
      gnome.canvas.CanvasGroup,
      x=0,
      y=0
      )
    anchorgroup.set_data('anchors',True)
    anchorgroup.hide()

    for anchor_type in self.anchors[item_type]:
      anchor=anchorgroup.add(
        gnome.canvas.CanvasRect,
        fill_color_rgba=self.ANCHOR_COLOR,
        outline_color_rgba=0x000000FFL,
        width_pixels=1,
        )
      anchor.set_data('anchor_type',anchor_type)
      anchor.connect("event", self.resize_item_event,anchor_type)


  def select_item(self, group):
    if (self.selected != None):
      self.selected.item_list[1].hide()
      self.selected = None

    # Deactivate old button
    self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[self.current_tool][1]))
        
    # Activate new button                         
    self.current_tool = self.select_tool_number
    self.old_tool_item = self.select_tool
    self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[self.current_tool][2]))
    gcompris.set_cursor(self.tools[self.current_tool][3]);

    self.selected = group
    self.selected.item_list[1].show()


  # gcompris.utils fonction seems have center problem
  def item_rotate_relative(self, item, angle):
    bounds = self.get_bounds(item)
    (cx, cy) = ( (bounds[2]+bounds[0])/2 , (bounds[3]+bounds[1])/2)
    
    t = math.radians(angle)

    # This matrix rotate around ( cx, cy )

    #     This is the résult of the product:


    #            T_{-c}             Rot (t)                 T_c

    #       1    0   cx       cos(t) -sin(t)    0        1    0  -cx
    #       0    1   cy  by   sin(t)  cos(t)    0   by   0    1  -cy
    #       0    0    1         0       0       1        0    0   1

    
    mat = ( math.cos(t),
            math.sin(t),
            -math.sin(t),
            math.cos(t),
            (1-math.cos(t))*cx + math.sin(t)*cy,
            -math.sin(t)*cx + (1 - math.cos(t))*cy)
   
    item.get_property("parent").affine_relative(mat)

  def item_flip(self, item):
    bounds = self.get_bounds(item)
    (cx, cy) = ( (bounds[2]+bounds[0])/2 , (bounds[3]+bounds[1])/2)
    
    mat = ( -1, 0, 0, 1, 2*cx, 0)
   
    item.get_property("parent").affine_relative(mat)


  def updated_text(self, item):
#    item.set(clip=1)
#    bounds = self.get_bounds(item)
#    print bounds, bounds[2]-bounds[0], bounds[3]-bounds[1]
#    item.set(clip_width=bounds[2]-bounds[0],
#             clip_height=bounds[3]-bounds[1]
#             )
    return

  def ps_print(self, group):
    file = tempfile.mkstemp('.svg','anim',text=True)
    print file
    # file will be reopen by svg_save
    os.close(file[0])
    #svg_save(file[1])
    
    #used to suppress the anchors
    SaveGroup = self.rootitem.add(
      gnome.canvas.CanvasGroup,
      x = 0,
      y = 0
      )
    gcompris.utils.clone_item(self.root_drawingitem, SaveGroup)
    print len(SaveGroup.item_list),  len(self.root_drawingitem.item_list)
    
    gcompris.utils.svg_save("anim",
                            file[1],
                            SaveGroup.item_list[0],
                            gcompris.BOARD_WIDTH,
                            gcompris.BOARD_HEIGHT,
                            0
                            )
    SaveGroup.destroy()

    # Check we find the convert programm
    resultList = os.popen('type -p convert').readlines()
    if (len(resultList)==0):
      print "Print is only possible with ImageMagick installed!!"
    else:
      resultList = os.popen('convert ' +  file[1]  + ' ' + file[1][:-3] +  'ps').readlines()

    

def svg_restore(filename):
  print "svg_restore", filename
  
  global fles

  gcompris.utils.svg_restore("anim",
                             filename,
                             fles.root_anim
                             )

  if (len(fles.root_anim.item_list) > 0):
    last_picture = fles.root_anim.item_list[-1]

    for item in last_picture.item_list:
      gcompris.utils.clone_item(item, fles.root_drawingitem)
      fles.anchorize(fles.root_drawingitem.item_list[-1])

  # unselect object if necessary
  if (fles.selected != None):
    fles.selected.item_list[1].hide()
    fles.selected = None
        
  # Deactivate old button
  fles.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(fles.tools[fles.current_tool][1]))
        
  # Activate new button                         
  fles.current_tool = fles.select_tool_number
  fles.old_tool_item = fles.select_tool
  fles.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(fles.tools[fles.current_tool][2]))
  gcompris.set_cursor(fles.tools[fles.current_tool][3]);

def svg_save(filename):
  print "svg_save", filename

  global fles
  gcompris.utils.svg_save("anim",
                          filename,
                          fles.root_anim,
                          gcompris.BOARD_WIDTH,
                          gcompris.BOARD_HEIGHT,
                          10
                          )

def image_selected(image):
  #fles is used because self is not passed through callback
  global fles
  
  pixmap = gcompris.utils.load_pixmap(image)
  #print image
  #pixmap = gcompris.utils.load_pixmap("GCompris/anchor.svg")
  
  
  fles.newitem = None
  fles.newitemgroup = fles.root_drawingitem.add(
    gnome.canvas.CanvasGroup,
    x=0.0,
    y=0.0
    )

  x= fles.pos_x
  y= fles.pos_y
  width = pixmap.get_width()
  height = pixmap.get_height()

  fles.newitem = fles.newitemgroup.add(
    gnome.canvas.CanvasPixbuf,
    pixbuf = pixmap,
    x=x,
    y=y,
    width=width,
    height=height,
    width_set = True,
    height_set = True
    )
  
  gcompris.utils.filename_pass(fles.newitem,image)

  fles.anchorize(fles.newitemgroup)
  fles.object_set_size_and_pos(fles.newitemgroup, x, y, x+width, y+height)
  fles.select_item(fles.newitemgroup)

  fles.newitem = None
  fles.newitemgroup = None

#  gcompris - anim
#
# Copyright (C) 2003, 2008 Bruno Coudoin (redraw code), 2004 Yves Combe (anim code)
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


#  Version 3 of anim
# Rewrote to support a timeline

#  Version 2 of anim
# Due to performance, the animation code as been rewriten
# For now, the animation is done keeping difference
# in parameters of graphicals object between shots?


from gcompris import gcompris_gettext as _
# PythonTest Board module
import gobject
import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.sound
import gtk
import gtk.gdk
import gtk.keysyms
import copy
import math
import time
import os
import sys
import tempfile
import cPickle as pickle
import base64

from Color import *
from Timeline import *
from AnimItem import *

fles=None

# When anim is passed the mode 'draw', animation is disabled.
#

#class Gcompris_anim:
class Gcompris_anim:
  """The cartoon activity"""

  def __init__(self, gcomprisBoard):

    self.gcomprisBoard = gcomprisBoard
    self.timeout = 0

    # There is two board in the same code
    # here the diff in parameters
    if self.gcomprisBoard.mode == 'draw':
      self.format_string = { 'gcompris' : 'GCompris draw 2 cPikle file',
                             'svg' : 'GCompris draw 2 svg file'
                             }
    else:
      self.format_string = { 'gcompris' : 'GCompris anim 2 cPikle file',
                             'svg' : 'GCompris anim 2 svg file'
                             }

    if self.gcomprisBoard.mode == 'draw':
      # DRAW
      #
      # draw is adapted to little kids : big anchors
      self.DEFAULT_ANCHOR_SIZE	= 12

      # Step used in grid is wider
      self.grid_step = 10

      # draw specific UI
      self.selector_section = "draw2"
    else:
      # Anim
      #
      # Normal anchors
      self.DEFAULT_ANCHOR_SIZE	= 8

      # Step used in grid is wider
      self.grid_step = 5

      # anim specific UI
      self.selector_section = "anim2"

    # Initialisation. Should not change in draw.
    self.running = False

    # In draw objects are created without drag&drop
    # Default size for rect, circle, line
    self.draw_defaults_size = { 'RECT' : {'width' : 60 , 'height' : 40 },
				'FILL_RECT' : {'width' : 60 , 'height' : 40 },
                   		'CIRCLE' : {'width' : 60 , 'height' : 40 },
                                'FILL_CIRCLE' : {'width' : 60 , 'height' : 40 },
                                'LINE' : {'width' : 60 , 'height' : 40 }
                              }

    # global parameter to access object structures from global fonctions
    global fles
    fles=self

    self.file_type = ".gcanim"

    self.MAX_TEXT_CHAR = 50

    # Part of UI : tools buttons
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

    # keep the tool selected
    self.current_tool=0

    # step of the grid used for positioning objects
    # TODO : add a parameters to put step=5 in draw and step=1 in anim
    self.current_step = 0

    # selected object
    self.selected = None

    # Part of UI : drawing_area is the drawing zone
    # when anim is played, it's masked and playing_area is displayed
    #
    # Drawing area is editing image area
    # Palying area is playing map
    self.drawing_area = [124.0, 20.0, gcompris.BOARD_WIDTH - 15, gcompris.BOARD_HEIGHT - 78]
    self.playing_area = [124.0, 20.0, gcompris.BOARD_WIDTH - 15, gcompris.BOARD_HEIGHT - 78]

    # Global used for the select event
    #
    # used to keep the distance between pointer and corner in moving objects
    self.in_select_ofx = -1
    self.in_select_ofy = -1

    # The frame counter
    # TODO : check if used
    self.item_frame_counter = []

    # Not used for the moment in anim2
    # TODO : fix that
    #self.current_image = 0

    # Part of UI
    # The root items
    self.root_coloritem = []
    self.root_toolitem  = []

    # Anim2 variables
    # animlist is the full list of all items.
    # each item is keeped with it's frame information
    #    - frames where it's modified
    #    - all modifications for each frame
    #
    # list of items in current frame
    self.framelist = []
    # list of items in the full animation
    self.animlist = []
    # rank of the current frame being processed
    self.current_frame = 0
    self.frames_total =  self.current_frame
    # list of z values in last shot
    self.list_z_last_shot = []
    # list of actual z values
    self.list_z_actual = []

    # used to handle draw creation of object
    self.created_object = None

  def start(self):

    self.last_commit = None

    # GCompris initialisation
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=0
    self.gcomprisBoard.number_of_sublevel=0

    gcompris.bar_set(0)
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(
      parent =  self.gcomprisBoard.canvas.get_root_item())

    # initialisation
    self.draw_tools()
    self.draw_animtools()

    self.timeline = Timeline(self.rootitem, self.drawing_area)
    self.timeline.draw()

    self.color = Color(self.rootitem, self.drawing_area)
    self.color.draw()

    self.draw_drawing_area(self.grid_step)
    self.draw_playing_area()


  def end(self):
    # stop the animation
    if self.running:
      self.playing_stop()

    if self.timeout:
      gobject.source_remove(self.timeout)
    self.timeout = 0

    # Remove the root item removes all the others inside it
    gcompris.set_cursor(gcompris.CURSOR_DEFAULT);
    self.rootitem.remove()

  def pause(self, pause):
    return

  def repeat(self):
    print("Gcompris_anim repeat.")

  def config(self):
    print("Gcompris_anim config.")

  def key_press(self, keyval, commit_str, preedit_str):
    #
    # I suppose codec is the stdin one.
    #
    codec = sys.stdin.encoding

    # keyboard shortcuts
    if (keyval == gtk.keysyms.F1):
      gcompris.file_selector_save( self.gcomprisBoard,
                                   self.selector_section, self.file_type,
                                   general_save)
    elif (keyval == gtk.keysyms.F2):
      gcompris.file_selector_load( self.gcomprisBoard,
                                   self.selector_section, self.file_type,
                                   general_restore)

    # Printing
    # Bruno we need a print button !
    #
    # was in anim1, but not print an animation is not interesting.
    elif (keyval == gtk.keysyms.F3):
      pass

    # AFAIR The keyboard part was written by bruno
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
      return False

    if ( (not self.selected) or
         (gobject.type_name(self.selected.get_child(0)) != "GooCanvasText")
         ):
      # No Text object selected
      return False

    textItem = self.selected.get_child(0)
    if (not self.last_commit):
      oldtext = textItem.get_property('text').decode('UTF-8')
    else:
      oldtext = self.last_commit

    if ((keyval == gtk.keysyms.BackSpace) or
        (keyval == gtk.keysyms.Delete)):
      if (len(oldtext) != 1):
        newtext = oldtext[:-1]
      else:
        newtext = u'?'
      self.last_commit = newtext

    elif (keyval == gtk.keysyms.Return):
      newtext = oldtext + '\n'
      self.last_commit = newtext

    else:
      if ((oldtext[:1] == u'?') and (len(oldtext)==1)):
        oldtext = u' '
        oldtext = oldtext.strip()

      if (commit_str):
        str = commit_str
        self.last_commit = oldtext + str
      if (preedit_str):
        str = '<span foreground="red">'+ preedit_str +'</span>'
        self.last_commit = oldtext

      if (len(oldtext) < self.MAX_TEXT_CHAR):
        newtext = oldtext + str
      else:
        newtext = oldtext

    textItem.set_properties(text = newtext.encode('UTF-8'))

    return True

  # Display the tools
  def draw_tools(self):

    self.root_toolitem = \
      goocanvas.Group(
        parent = self.rootitem,
      )

    goocanvas.Image(
      parent = self.root_toolitem,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/tool-selector.png")),
      x = 5,
      y = 5.0,
      width = 107.0,
      height = 517.0,
      )

    x1 = 11.0
    x2 = 56.0
    y = 11.0
    stepy = 45

    # Display the tools
    for i in range(0,len(self.tools)):

      # Exclude the anim specific buttons
      if self.gcomprisBoard.mode == 'draw':
        if self.tools[i][0]=="MOVIE" or self.tools[i][0]=="PICTURE":
          continue

      if(i%2):
        theX = x2
      else:
        theX = x1

      item = \
        goocanvas.Image(
          parent = self.root_toolitem,
        pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[i][1])),
        x=theX,
        y=y
        )
      gcompris.utils.item_focus_init(item, None)
      item.connect("button_press_event", self.tool_item_event, i)
      if i%2:
        y += stepy

      if(self.tools[i][0]=="SELECT"):
        self.select_tool = item
        self.select_tool_number = i
        # Always select the SELECT item by default
        self.current_tool = i
        self.old_tool_item = item
        self.old_tool_item.props.pixbuf = \
            gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[i][2]))
        gcompris.set_cursor(self.tools[i][3]);


  # Event when a tool is selected
  # Perform instant action or swich the tool selection
  def tool_item_event(self, item, target, event, tool):

    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/bleep.wav")
        # Some button have instant effects
        if (self.tools[tool][0] == "SAVE"):
#          self.Anim2Shot()
          gcompris.file_selector_save( self.gcomprisBoard,
                                       self.selector_section, self.file_type,
                                       general_save)
          return False

        elif (self.tools[tool][0] == "LOAD"):
          gcompris.file_selector_load( self.gcomprisBoard,
                                       self.selector_section, self.file_type,
                                       general_restore)
          return False

        elif (self.tools[tool][0] == "IMAGE"):
          self.pos_x = gcompris.BOARD_WIDTH/2
          self.pos_y = gcompris.BOARD_HEIGHT/2

          gcompris.images_selector_start(self.gcomprisBoard,
                                         "dataset",
                                         image_selected);
          return False

        elif (self.tools[tool][0] == "PICTURE"):
          self.Anim2Shot()
          return False

        elif (self.tools[tool][0] == "MOVIE"):
          if self.frames_total == 0:
            print 'Mmm... Need to make shots before run anim !!'
            return False

          if not self.running:
            # unselect object if necessary
            #self.anim_item_unselect()

            self.playing_start()
            return False

        elif (self.tools[tool][0] != "SELECT") and (self.selected):
          #self.anim_item_unselect()
          pass

        #
        # Normal case, tool button switch
        # -------------------------------

        # Deactivate old button
        self.old_tool_item.props.pixbuf = \
            gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[self.current_tool][1]))

        # Activate new button
        self.current_tool = tool
        self.old_tool_item = item
        self.old_tool_item.props.pixbuf = \
            gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[self.current_tool][2]))
        gcompris.set_cursor(self.tools[self.current_tool][3]);


  # Display the drawing area
  def draw_drawing_area(self,step):

    x1 = self.drawing_area[0]
    y1 = self.drawing_area[1]
    x2 = self.drawing_area[2]
    y2 = self.drawing_area[3]

    item = \
      goocanvas.Rect(
        parent = self.rootitem,
      x = x1,
      y = y1,
      width = x2 - x1,
      height = y2 - y1,
      fill_color_rgba=0xFFFFFFFFL,
      line_width=2.0,
      stroke_color_rgba=0x111199FFL
      )
    item.connect("button_press_event", self.item_event)
    item.connect("button_release_event", self.item_event)
    item.connect("motion_notify_event", self.item_event)

    # The CanvasGroup for the edit space.
    self.root_drawingitem = \
      goocanvas.Group(
        parent = self.rootitem,
      )
    self.draw_grid(x1,x2,y1,y2,step)

    # Create the root_anim group which contains all the drawings.
    # At root_anim root, there is a group for each drawings.
    self.root_anim = \
      goocanvas.Group(
        parent = self.rootitem,
      )

    gcompris.utils.item_absolute_move(self.root_anim,
                                      int(self.playing_area[0]-self.drawing_area[0]),
                                      int(self.playing_area[1]-self.drawing_area[1])
                                      )

    # Create a group for the first drawing

    self.flash = \
      goocanvas.Image(
        parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/camera.png")),
      x=300,
      y=200,
      )
    self.flash.props.visibility = goocanvas.ITEM_INVISIBLE


  # Display the drawing area
  def draw_playing_area(self):

    x1=self.playing_area[0]
    y1=self.playing_area[1]
    x2=self.playing_area[2]
    y2=self.playing_area[3]


    # The CanvasGroup for the playing area.
    self.root_playingitem = \
      goocanvas.Group(
        parent = self.rootitem,
      )
    self.root_playingitem.props.visibility = goocanvas.ITEM_INVISIBLE

    # intervall = 1000 / anim_speed
    self.anim_speed=5

    run = \
      goocanvas.Image(
        parent = self.root_playingitem,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/down.png")),
      x = 15,
      y = 410,
      width = 20,
      height = 20,
      )
    run.connect("button_press_event", self.speed_event,False)

    self.speed_item = \
      goocanvas.Text(
        parent = self.root_playingitem,
      text=self.anim_speed,
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x=52,
      y=420,
      anchor=gtk.ANCHOR_CENTER,
      )


    run = \
      goocanvas.Image(
        parent = self.root_playingitem,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/up.png")),
      x = 70,
      y = 410,
      width = 20,
      height = 20,
      )
    run.connect("button_press_event", self.speed_event,True)

    # And finaly a STOP icon
    run = \
      goocanvas.Image(
        parent = self.root_playingitem,
      pixbuf = gcompris.utils.load_pixmap("anim/draw.svg"),
      x = 16,
      y = 110,
      )
    run.connect("button_press_event", self.stop_event,True)


  def stop_event(self, item, target, event, up):
    if event.type == gtk.gdk.BUTTON_PRESS:
      gcompris.sound.play_ogg("sounds/bleep.wav")
      self.playing_stop()

  def playing_stop(self):
    self.running=False
    gobject.source_remove(self.timeout)
    #self.run_anim2()

  def speed_event(self, item, target, event, up):

    if event.type == gtk.gdk.BUTTON_PRESS:
      gcompris.sound.play_ogg("sounds/bleep.wav")
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
      self.timeout=gobject.timeout_add(1000/self.anim_speed, self.run_anim2)
      self.speed_item.set_properties(text=self.anim_speed)

  # Draw the grid
  #
  def draw_grid(self, x1, x2, y1, y2, step):

    self.current_step = step

    color = 0x1D0DFFFFL

    self.grid = \
      goocanvas.Group(
        parent = self.rootitem,
      )
    self.grid.props.visibility = goocanvas.ITEM_INVISIBLE

    for i in range(int(x1), int(x2), step):
      item = \
        goocanvas.Polyline(
          parent = self.grid,
        points = goocanvas.Points([(i , y1), (i , y2)]),
        fill_color_rgba=color,
        line_width=1.0,
        )
      # Clicking on lines let you create object
      item.connect("button_press_event", self.item_event)
      item.connect("button_release_event", self.item_event)
      item.connect("motion_notify_event", self.item_event)

    for i in range(int(y1), int(y2), step):
      item = \
        goocanvas.Polyline(
          parent = self.grid,
        points= goocanvas.Points([(x1, i), (x2 , i)]),
        fill_color_rgba=color,
        line_width=1.0,
        )
      item.connect("button_press_event", self.item_event)
      item.connect("button_release_event", self.item_event)
      item.connect("motion_notify_event", self.item_event)





  # Event when a click on any item. Perform the move
  def move_item_event(self, item, target, event):

    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
      gcompris.sound.play_ogg("sounds/bleep.wav")
      #self.anim_item_unselect()

    if self.tools[self.current_tool][0] == "CCW":
      if ((event.type == gtk.gdk.BUTTON_PRESS) and
          (event.button == 1) and
          (gobject.type_name(item)!="GnomeCanvasText")):
        # this one seems broken
        #gcompris.utils.item_rotate_relative(item.get_property("parent"),-10)
        self.rotate_relative(item.get_property("parent"), -10)
        return True
      else:
        return False

    if self.tools[self.current_tool][0] == "CW":
      if ((event.type == gtk.gdk.BUTTON_PRESS) and
          (event.button == 1) and
          (gobject.type_name(item)!="GnomeCanvasText")):
        self.rotate_relative(item.get_property("parent"), 10)
        return True
      else:
        return False

    if self.tools[self.current_tool][0] == "FLIP":
      if ((event.type == gtk.gdk.BUTTON_PRESS) and
          (event.button == 1) and
          (gobject.type_name(item)!="GnomeCanvasText")):
        self.item_flip(item);
        return True
      else:
        return False

    if self.tools[self.current_tool][0] == "RAISE":
      if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
        item.get_property("parent").raise_(None)
        return True
      else:
        return False

    if self.tools[self.current_tool][0] == "LOWER":
      if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
        item.get_property("parent").lower(None)
        return True
      else:
        return False

    if self.tools[self.current_tool][0] == "SELECT":
      if (event.type == gtk.gdk.BUTTON_PRESS) and (event.button == 1):
        self.in_select_ofx = event.x
        self.in_select_ofy = event.y
        return False

    #
    # MOUSE DRAG STOP
    # ---------------
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        if self.created_object:
          self.created_object = None
          return True

        # activate the anchors
        #self.anim_item_select(item.get_property("parent"))

        return True

    # The move motion
    if (self.tools[self.current_tool][0] == "SELECT"
        and event.type == gtk.gdk.MOTION_NOTIFY
        and event.state & gtk.gdk.BUTTON1_MASK):

      dx = event.x - self.in_select_ofx
      dy = event.y - self.in_select_ofy

      item.get_property("parent").translate(dx, dy)
      return True

    return False

  # Main callback on item comes here first
  # And are then dispatched to the proper functions
  def item_event(self, item, target, event):
    if(event.type == gtk.gdk.BUTTON_PRESS and self.running==True):
      self.playing_stop()
      return False

    # Right button is a shortcup to Shot
    if (self.gcomprisBoard.mode != 'draw' and
        event.type == gtk.gdk.BUTTON_PRESS and
        event.button == 3):
      self.Anim2Shot()
      return False

    if event.type == gtk.gdk.BUTTON_PRESS:

      if event.button == 1:
        gcompris.sound.play_ogg("sounds/bleep.wav")

        animItem = item.get_data("AnimItem")
        if animItem:
          if self.tools[self.current_tool][0] == "FILL":
            animItem.fill(self.color.fill,
                          self.color.stroke)
          elif self.tools[self.current_tool][0] == "FILL":
            animItem.flip()
          elif self.tools[self.current_tool][0] == "DEL":
            animItem.delete()
          elif self.tools[self.current_tool][0] == "SELECT":
            if self.selected:
              self.selected.deselect()
            animItem.select()
            self.selected = animItem
          elif self.tools[self.current_tool][0] == "RAISE":
            animItem.raise_()
          elif self.tools[self.current_tool][0] == "LOWER":
            animItem.lower()
          elif self.tools[self.current_tool][0] == "CCW":
            animItem.rotate_ccw()
          elif self.tools[self.current_tool][0] == "CW":
            animItem.rotate_cw()

        if self.tools[self.current_tool][0] == "FILL_RECT":
          self.created_object = AnimItemFillRect(self,
                                                 event.x, event.y)

    #
    # MOTION EVENT
    # ------------
    elif (event.type == gtk.gdk.MOTION_NOTIFY
        and self.created_object):
      # That's used only in item creation.
      # In draw mode, item creation does not use drag&drop
      if self.gcomprisBoard.mode == 'draw':
        return False

      if event.state & gtk.gdk.BUTTON1_MASK:
        x = event.x
        y = event.y

        self.created_object.resize_item_event(item,
                                              target,
                                              event)

    #
    # MOUSE DRAG STOP
    # ---------------
    elif (event.type == gtk.gdk.BUTTON_RELEASE
          and self.created_object):
      # That's used only in item creation.
      # In draw mode, item creation does not use drag&drop
      if self.created_object:
        self.created_object = False
        return True

    return False


  # Event when an event on the drawing area happen
  def create_item_event_OLD(self, item, target, event):
    if(event.type == gtk.gdk.BUTTON_PRESS
       and self.running==True):
      self.playing_stop()
      return False

    # Right button is a shortcup to Shot
    if (self.gcomprisBoard.mode != 'draw' and
        event.type == gtk.gdk.BUTTON_PRESS and
        event.button == 3):
      self.Anim2Shot()
      return False

    if (not (self.tools[self.current_tool][0] == "RECT" or
             self.tools[self.current_tool][0] == "CIRCLE" or
             self.tools[self.current_tool][0] == "FILL_RECT" or
             self.tools[self.current_tool][0] == "FILL_CIRCLE" or
             self.tools[self.current_tool][0] == "IMAGE" or
             self.tools[self.current_tool][0] == "TEXT" or
             self.tools[self.current_tool][0] == "LINE")):
      return False

    if event.type == gtk.gdk.BUTTON_PRESS:

      if event.button == 1:

        if self.tools[self.current_tool][0] == "LINE":

          x,y = self.snap_to_grid1(event.x,event.y)
          self.pos_x = x
          self.pos_y = y

          tuple_points = goocanvas.Points([(x , y),
                                           (self.pos_x, self.pos_y)])

          if self.gcomprisBoard.mode == 'draw':
            dist = {'x' : 'width', 'y' : 'height'}

            points = {}
            for c in ['x', 'y']:
              points[c + '1'] = eval(c) - self.draw_defaults_size['LINE'][dist[c]]/2
              points[c + '2'] = eval(c) + self.draw_defaults_size['LINE'][dist[c]]/2

            tuple_points = goocanvas.Points([(points['x1'], points['y1']),
                                             (points['x2'], points['y2'])])

          self.newitem = \
            goocanvas.Polyline(
              parent = self.newitemgroup,
            points = tuple_points,
            fill_color_rgba = self.color.fill,
            line_width = 8.0
            )

        elif self.tools[self.current_tool][0] == "RECT":

          x,y = self.snap_to_grid1(event.x,event.y)
          self.pos_x = x
          self.pos_y = y

          points = {}
          for c in ['x' , 'y']:
            points[c + '1'] = eval(c)
            points[c + '2'] = eval( 'self.pos_' + c )


          if self.gcomprisBoard.mode == 'draw':
            dist = {'x' : 'width', 'y' : 'height'}

            points = {}
            for c in ['x', 'y']:
              points[c + '1'] = eval(c) - self.draw_defaults_size['LINE'][dist[c]]/2
              points[c + '2'] = eval(c) + self.draw_defaults_size['LINE'][dist[c]]/2

          self.newitem = \
            goocanvas.Rect(
              parent = self.newitemgroup,
              x = points['x1'],
              y = points['y2'],
              width = points['x2'] - points['x1'],
              height = points['y2'] - points['y1'],
              stroke_color_rgba=self.color.stroke,
              line_width=4.0
              )
          #          self.newitem.set_data('empty',True)
          gcompris.utils.canvas_set_property(self.newitem, "empty", "True")

        elif self.tools[self.current_tool][0] == "FILL_RECT":

          self.created_object = AnimItemFillRect(self,
                                                 event.x, event.y)

        elif self.tools[self.current_tool][0] == "CIRCLE":

          x,y = self.snap_to_grid1(event.x,event.y)
          self.pos_x = x
          self.pos_y = y

          points = {}
          for c in ['x' , 'y']:
            points[c + '1'] = eval(c)
            points[c + '2'] = eval( 'self.pos_' + c )


          if self.gcomprisBoard.mode == 'draw':
            dist = {'x' : 'width', 'y' : 'height'}

            points = {}
            for c in ['x', 'y']:
              points[c + '1'] = eval(c) - self.draw_defaults_size['LINE'][dist[c]]/2
              points[c + '2'] = eval(c) + self.draw_defaults_size['LINE'][dist[c]]/2

          self.newitem = \
            goocanvas.Ellipse(
              parent = self.newitemgroup,
              center_x = points['x1'],
              center_y = points['y1'],
              radius_x = points['x2'] - points['x1'],
              radius_y = points['y2'] - points['y1'],
              stroke_color_rgba = self.color.stroke,
              line_width = 5.0
            )
          #          self.newitem.set_data('empty',True)
          gcompris.utils.canvas_set_property(self.newitem, "empty", "True")

        elif self.tools[self.current_tool][0] == "FILL_CIRCLE":

          x,y = self.snap_to_grid1(event.x,event.y)
          self.pos_x = x
          self.pos_y = y


          points = {}
          for c in ['x' , 'y']:
            points[c + '1'] = eval(c)
            points[c + '2'] = eval( 'self.pos_' + c )


          if self.gcomprisBoard.mode == 'draw':
            dist = {'x' : 'width', 'y' : 'height'}

            points = {}
            for c in ['x', 'y']:
              points[c + '1'] = eval(c) - self.draw_defaults_size['LINE'][dist[c]]/2
              points[c + '2'] = eval(c) + self.draw_defaults_size['LINE'][dist[c]]/2

          self.newitem = \
            goocanvas.Ellipse(
              parent = self.newitemgroup,
            center_x = points['x1'],
            center_y = points['y1'],
            radius_x = points['x2'] - points['x1'],
            radius_y = points['y2'] - points['y1'],
            fill_color_rgba = self.color.fill,
            stroke_color_rgba = self.color.stroke,
            line_width = 1.0
            )

        elif self.tools[self.current_tool][0] == "TEXT":

          x,y = self.snap_to_grid1(event.x,event.y)
          self.pos_x = x
          self.pos_y = y

          self.newitem = \
            goocanvas.Text(
              parent = self.newitemgroup,
              x = self.pos_x,
              y = self.pos_y,
              fill_color_rgba = self.color.fill,
              font = gcompris.FONT_BOARD_BIG_BOLD,
              text = u'?',
              anchor=gtk.ANCHOR_CENTER
              )

      return True

    #
    # MOTION EVENT
    # ------------
    if (event.type == gtk.gdk.MOTION_NOTIFY
        and self.created_object):
      # That's used only in item creation.
      # In draw mode, item creation does not use drag&drop
      if self.gcomprisBoard.mode == 'draw':
        return False

      if event.state & gtk.gdk.BUTTON1_MASK:
        x = event.x
        y = event.y

        self.created_object.resize_item_event(item,
                                              target,
                                              event)

    #
    # MOUSE DRAG STOP
    # ---------------
    if event.type == gtk.gdk.BUTTON_RELEASE:
      # That's used only in item creation.
      # In draw mode, item creation does not use drag&drop
      if self.created_object:
        self.created_object = False
        return True

      if ((self.tools[self.current_tool][0] == "IMAGE") or
          (self.tools[self.current_tool][0] == "TEXT")):
        return False

    return False

  def playing_start(self):
    if not self.running:
      self.running = True
      self.root_coloritem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.root_toolitem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.root_playingitem.props.visibility = goocanvas.ITEM_VISIBLE
      #self.Anim2Run()

  def playing_event(self, item, target, event, state):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if state:
        self.playing_start()
      else:
        self.playing_stop()

  # Display the animation tools
  def draw_animtools(self):
    x_left = 8
    y_top  = 472
    minibutton_width = 32
    minibutton_height = 20

    if self.gcomprisBoard.mode == 'draw':
      return

    # Draw the background area

      goocanvas.Image(
        parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/counter.png")),
      x=x_left - -11,
      y=y_top - 2,
      width=70.0,
      height=34.0,
      )

    # Image Number
    self.item_frame_counter = \
        goocanvas.Text(
      parent = self.rootitem,
      text = self.current_frame + 1,
      x = x_left + minibutton_width + 14,
      y = y_top + 15,
      font = gcompris.skin.get_font("gcompris/board/medium"),
      fill_color = "white")



  def object_set_size_and_pos(self, object, x1, y1, x2, y2):
    if gobject.type_name(object.get_child(0)) == "GooCanvasLine":
      object.get_child(0).set_properties(
        points = goocanvas.Points([(x1,y1) ,(x2,y2)])
        )
    elif gobject.type_name(object.get_child(0)) == "GooCanvasImage":
      item = object.get_child(0)
      item.props.transform = None
      sx = (x2 - x1) / item.props.width
      sy = (y2 - y1) / item.props.height
      item.translate(x1, y1)
      item.scale(sx, sy)
    elif gobject.type_name(object.get_child(0)) == "GooCanvasText":
      object.get_child(0).set_properties(
        x = (x1+x2)/2,
        y = (y1+y2)/2
        )
    elif gobject.type_name(object.get_child(0)) == "GooCanvasRect":
      object.get_child(0).set_properties(
        x = x1,
        y = y1,
        width = x2 - x1,
        height = y2 - y1
        )
    elif gobject.type_name(object.get_child(0)) == "GooCanvasEllipse":
      object.get_child(0).set_properties(
        center_x = x1 + (x2-x1)/2,
        center_y = y1 + (y2-y1)/2,
        radius_x = (x2 - x1)/2,
        radius_y = (y2 - y1)/2
        )

    if(object.get_n_children() < 2):
      print "Warning: Should not happens, uncomplete object"
      return

    for i in range(0, object.get_child(1).get_n_children()):
      anchor = object.get_child(1).get_child(i)
      anchor_type = anchor.get_data('anchor_type')

      if anchor_type == self.ANCHOR_N:
        anchor.set_properties(
          x= (x1 + x2 - self.DEFAULT_ANCHOR_SIZE)/2,
          y= y1 - self.DEFAULT_ANCHOR_SIZE,
          width = self.DEFAULT_ANCHOR_SIZE,
          height = self.DEFAULT_ANCHOR_SIZE,
          )
      elif anchor_type == self.ANCHOR_T:
        anchor.set_properties(
          x= (x1 + x2 - self.DEFAULT_ANCHOR_SIZE*3)/2,
          y= y2,
          width = self.DEFAULT_ANCHOR_SIZE,
          height = self.DEFAULT_ANCHOR_SIZE,
          )
      elif anchor_type == self.ANCHOR_NE:
        anchor.set_properties(
          x= x2,
          y= y1 - self.DEFAULT_ANCHOR_SIZE,
          width = self.DEFAULT_ANCHOR_SIZE,
          height = self.DEFAULT_ANCHOR_SIZE,
          )
      elif anchor_type == self.ANCHOR_E:
        anchor.set_properties(
          x= x2,
          y= (y1 + y2 - self.DEFAULT_ANCHOR_SIZE)/2,
          width = self.DEFAULT_ANCHOR_SIZE,
          height = self.DEFAULT_ANCHOR_SIZE,
          )
      elif anchor_type == self.ANCHOR_SE:
        anchor.set_properties(
          x= x2,
          y= y2,
          width = self.DEFAULT_ANCHOR_SIZE,
          height = self.DEFAULT_ANCHOR_SIZE,
          )
      elif anchor_type == self.ANCHOR_S:
        anchor.set_properties(
          x= (x1 + x2 - self.DEFAULT_ANCHOR_SIZE)/2,
          y= y2,
          width = self.DEFAULT_ANCHOR_SIZE,
          height = self.DEFAULT_ANCHOR_SIZE,
          )
      elif anchor_type == self.ANCHOR_SW:
        anchor.set_properties(
          x= x1 - self.DEFAULT_ANCHOR_SIZE,
          y= y2,
          width = self.DEFAULT_ANCHOR_SIZE,
          height = self.DEFAULT_ANCHOR_SIZE,
          )
      elif anchor_type == self.ANCHOR_W:
        anchor.set_properties(
          x= x1 - self.DEFAULT_ANCHOR_SIZE,
          y= (y1 + y2 - self.DEFAULT_ANCHOR_SIZE)/2,
          width = self.DEFAULT_ANCHOR_SIZE,
          height = self.DEFAULT_ANCHOR_SIZE,
          )
      elif anchor_type == self.ANCHOR_NW:
        anchor.set_properties(
          x= x1 - self.DEFAULT_ANCHOR_SIZE,
          y= y1 - self.DEFAULT_ANCHOR_SIZE,
          width = self.DEFAULT_ANCHOR_SIZE,
          height = self.DEFAULT_ANCHOR_SIZE,
          )


  def resize_item_event(self, item, target, event, anchor_type):
    if self.running:
      return

    # Right button is a shortcup to Shot
    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 3:
      self.Anim2Shot()
      return False

    if event.state & gtk.gdk.BUTTON1_MASK:
      # warning: anchor is in a group of anchors, which is in the object group
      parent = item.get_property("parent").get_property("parent")
      real_item = parent.get_child(0)

      x = event.x
      y = event.y

      if gobject.type_name(real_item)=="GooCanvasLine":
        points = real_item.get_property("points")
        x1 = points[0]
        y1 = points[1]
        x2 = points[2]
        y2 = points[3]
      elif gobject.type_name(real_item)=="GooCanvasImage":
        bounds = real_item.get_bounds()
        (x1, y1)= \
            real_item.get_canvas().convert_to_item_space(parent,
                                                         bounds.x1, bounds.y1)
        (x2, y2)= \
            real_item.get_canvas().convert_to_item_space(parent,
                                                         bounds.x2, bounds.y2)

        mx1 = min(x1, x2)
        my1 = min(y1, y2)
        x2 = max(x1, x2)
        y2 = max(y1, y2)
        x1 = mx1
        y1 = my1
#         x1 = real_item.get_property("x")
#         y1 = real_item.get_property("y")
#         x2 = x1 + real_item.get_property("width")
#         y2 = y1 + real_item.get_property("height")
      elif gobject.type_name(real_item)=="GooCanvasEllipse":
        x1 = real_item.get_property("center_x") - real_item.get_property("radius_x")
        y1 = real_item.get_property("center_y") - real_item.get_property("radius_y")
        x2 = x1 + real_item.get_property("radius_x") * 2
        y2 = y1 + real_item.get_property("radius_y") * 2
      elif gobject.type_name(real_item)=="GooCanvasRect":
        x1 = real_item.get_property("x")
        y1 = real_item.get_property("y")
        x2 = x1 + real_item.get_property("width")
        y2 = y1 + real_item.get_property("height")
      elif gobject.type_name(real_item)=="GooCanvasText":
        y1 = y
        y2 = y + 20
        pass

      if (anchor_type == self.ANCHOR_N):
        self.object_set_size_and_pos(parent,
                                     x1=x1,
                                     y1=y,
                                     x2=x2,
                                     y2=y2
                                     )
      elif (anchor_type == self.ANCHOR_T):
        self.object_set_size_and_pos(parent,
                                     x1=x,
                                     y1=y1,
                                     x2=x,
                                     y2=y2
                                     )
      elif (anchor_type == self.ANCHOR_NE):
        self.object_set_size_and_pos(parent,
                                     x1=x1,
                                     y1=y,
                                     x2=x,
                                     y2=y2
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
                                     y1=y1,
                                     x2=x,
                                     y2=y
                                     )
      elif (anchor_type == self.ANCHOR_S):
        self.object_set_size_and_pos(parent,
                                     x1=x1,
                                     y1=y1,
                                     x2=x2,
                                     y2=y
                                     )
      elif (anchor_type == self.ANCHOR_SW):
        self.object_set_size_and_pos(parent,
                                     x1=x,
                                     y1=y1,
                                     x2=x2,
                                     y2=y
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
                                     y1=y,
                                     x2=x2,
                                     y2=y2
                                     )


  def rotate_relative(self, item, angle):
    bounds = item.get_bounds()
    #    print "Item bounds : ", bounds

    #bds = item.get_property("parent").get_bounds()
    #    print "Item parent bounds : ", bounds

    (cx, cy) = ( bounds.x1 + (bounds.x2-bounds.x1)/2,
                 bounds.y1 + (bounds.y2-bounds.y1)/2 )


    t = math.radians(angle)

    # This matrix rotate around ( cx, cy )

    #     This is the result of the product:


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

    #item.get_property("parent").set_transform(mat)
    item.rotate(angle, cx, cy)

    return

###############################################
#
#             GLOBAL functions
#
###############################################
def general_save(filename, filetype):
  global fles
  print "filename=%s filetype=%s" %(filename, filetype)


def general_restore(filename, filetype):
  print "general_restore : ", filename, " type ",filetype




def image_selected(image):
  #fles is used because self is not passed through callback
  global fles

  pixmap = gcompris.utils.load_pixmap(image)

  fles.newitem = None
  fles.newitemgroup = goocanvas.Group(
    parent = fles.root_anim
    )

  x = fles.pos_x
  y = fles.pos_y

  fles.newitem = goocanvas.Image(
    parent = fles.newitemgroup,
    pixbuf = pixmap,
    )



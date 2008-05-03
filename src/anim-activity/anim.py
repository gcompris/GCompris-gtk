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

    # The main list of items
    # this parameter is used in svg save, to know where to get the list
    self.itemlist = { 'draw' : 'framelist',
		      'anim': 'animlist'
                      }

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

    # These are used to let us restart only after the bonux is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    self.MAX_TEXT_CHAR = 50

    # kind of beautiful blue
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
    self.ANCHOR_T  = 9

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
                     'TEXT': [ self.ANCHOR_T ]
                     }
    self.anchors ['FILL_RECT'] =  self.anchors ['RECT']
    self.anchors ['CIRCLE'] =  self.anchors ['RECT']
    self.anchors ['FILL_CIRCLE'] =  self.anchors ['RECT']
    self.anchors ['IMAGE'] =  self.anchors ['RECT']

    # goocanvas type corresponding
    self.types = { 'RECT' : goocanvas.Rect,
                   'FILL_RECT' : goocanvas.Rect,
                   'CIRCLE' : goocanvas.Ellipse,
                   'FILL_CIRCLE' : goocanvas.Ellipse,
                   'TEXT' : goocanvas.Text,
                   'IMAGE' : goocanvas.Image,
                   'LINE' : goocanvas.Polyline
                   }

    # mutable goocanvas attributs
    self.attributs = { 'LINE' : [ "points",
                                  "stroke_color_rgba",
                                  ],
                       'RECT' : [ "x",
                                  "y",
                                  "width",
                                  "height",
                                  "stroke_color_rgba",
                                  ],
                       'FILL_RECT' : [ "x",
                                       "y",
                                       "width",
                                       "height",
                                       "fill_color_rgba",
                                       ],
                       'CIRCLE' : [ "center_x",
                                    "center_y",
                                    "radius_x",
                                    "radius_y",
                                    "stroke_color_rgba",
                                    ],
                       'FILL_CIRCLE' : [ "center_x",
                                         "center_y",
                                         "radius_x",
                                         "radius_y",
                                         "fill_color_rgba",
                                         ],
                       'TEXT' : [ "x",
                                  "y",
                                  "text",
                                  "fill_color_rgba",
                                  ],
                       'IMAGE' : [ "x",
                                   "y",
                                   "width",
                                   "height",
                                   ]
                       }

    # non mutable goocanvas attributs
    self.fixedattributs = { 'LINE' : { 'line_width': 8.0
                                       },
                            'RECT' : { 'line_width': 4.0
                                       },
                            'FILL_RECT' : { 'line_width': 1.0,
                                            'stroke_color_rgba': 0x000000FFL
                                            },
                            'CIRCLE' : { 'line_width': 4.0 },
                            'FILL_CIRCLE' : { 'line_width': 1.0,
                                              'stroke_color_rgba': 0x000000FFL
                                              },
                            'TEXT' : { 'font': gcompris.FONT_BOARD_BIG_BOLD,
                                       'anchor' : gtk.ANCHOR_CENTER
                                       },
                            'IMAGE' : {
                                        }
                       }


    # events handled by each type
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

    self.events ['FILL_RECT']    = self.events ['LINE']
    self.events ['FILL_CIRCLE']  = self.events ['LINE']
    self.events ['CIRCLE']       = self.events ['RECT']


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

    # Part of UI: colors buttons
    # COLOR SELECTION
    # RGBA unsigned long. A is always FF.
    # keep in mind if you change that to change the svg export: it does not pass A.
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

    # keep the current color here
    self.current_color = 0

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
    self.draw_created_object = False

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
    self.draw_colors()
    self.draw_drawing_area(self.grid_step)
    self.draw_playing_area()
    self.pause(0)


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
    #used to stop the event reception at the end?
    self.board_paused = pause
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
      #if self.gcomprisBoard.mode == 'draw':
        # We can keep in draw2, svg export will be pure svg.
        #self.ps_print(self.get_drawing(self.current_image))
      #else:
        #print "Sorry i can't print an animation"

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

    if ( (not self.selected) and
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
            self.anim_item_unselect()

            self.playing_start()
            return False

        elif (self.tools[tool][0] != "SELECT") and (self.selected):
          self.anim_item_unselect()

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


  # Display the color selector
  def draw_colors(self):

    pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/color-selector.png"))

    x = (self.drawing_area[2] - self.drawing_area[0]
         - pixmap.get_width())/2 + self.drawing_area[0]

    color_pixmap_height = pixmap.get_height()

    y = gcompris.BOARD_HEIGHT - color_pixmap_height - 30

    c = 0

    self.root_coloritem = \
      goocanvas.Group(
        parent = self.rootitem,
      )


    goocanvas.Image(
      parent = self.root_coloritem,
      pixbuf = pixmap,
      x=x,
      y=y,
      )

    for i in range(0,10):
      x1=x+26+i*56

      for j in range(0,4):
        c = i*4 +j
        item = \
          goocanvas.Rect(
            parent = self.root_coloritem,
            x = x1 + 26*(j%2),
            y = y + 8 + (color_pixmap_height/2 -6)*(j/2),
            width = 24,
            height = color_pixmap_height/2 - 8,
            fill_color_rgba = self.colors[c],
            stroke_color_rgba = 0x07A3E0FFL
            )

        item.connect("button_press_event", self.color_item_event, c)
        if (c==0):
          self.current_color = c
          self.old_color_item = item
          self.old_color_item.props.line_width = 4.0
          self.old_color_item.stroke_color_rgba = 0x16EC3DFFL

  # Color event
  def color_item_event(self, item, target, event, color):
    if self.running:
      return

    if event.type == gtk.gdk.BUTTON_PRESS:
      gcompris.sound.play_ogg("sounds/drip.wav")
      if event.button == 1:
        # Deactivate old button
        self.old_color_item.props.line_width = 0.0
        self.old_color_item.props.stroke_color_rgba = 0x144B9DFFL

        # Activate new button
        self.current_color = color
        self.old_color_item = item
        self.old_color_item.props.line_width = 4.0
        self.old_color_item.props.stroke_color_rgba= 0x16EC3DFFL


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
    item.connect("button_press_event", self.create_item_event)
    item.connect("button_release_event", self.create_item_event)
    item.connect("motion_notify_event", self.create_item_event)

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
    self.run_anim2()

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
      item.connect("button_press_event", self.create_item_event)
      item.connect("button_release_event", self.create_item_event)
      item.connect("motion_notify_event", self.create_item_event)

    for i in range(int(y1), int(y2), step):
      item = \
        goocanvas.Polyline(
          parent = self.grid,
        points= goocanvas.Points([(x1, i), (x2 , i)]),
        fill_color_rgba=color,
        line_width=1.0,
        )
      item.connect("button_press_event", self.create_item_event)
      item.connect("button_release_event", self.create_item_event)
      item.connect("motion_notify_event", self.create_item_event)

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
  def move_item_event(self, item, target, event):

    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
      gcompris.sound.play_ogg("sounds/bleep.wav")
      self.anim_item_unselect()

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
        self.z_raise(item.get_data("AnimItem"))
        return True
      else:
        return False

    if self.tools[self.current_tool][0] == "LOWER":
      if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
        item.get_property("parent").lower(None)
        self.z_lower(item.get_data("AnimItem"))
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
        if self.draw_created_object:
          self.draw_created_object = False
          return True

        # activate the anchors
        self.anim_item_select(item.get_property("parent"))

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

  # Event when a click on an item happen on fill in type object
  def fillin_item_event(self, item, target, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/paint1.wav")
        if self.tools[self.current_tool][0] == "FILL":
          item.set_properties(fill_color_rgba=self.colors[self.current_color])
          return True
    return False

  # Event when a click on an item happen on border fill type object
  def fillout_item_event(self, item, target, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/paint1.wav")
        if self.tools[self.current_tool][0] == "FILL":
          item.set_properties(stroke_color_rgba=self.colors[self.current_color])
          return True
    return False

  # Del an item and internal struct cleanup
  def del_item(self, item):
    item.get_property("parent").remove()
    self.del_AnimItem(item.get_data("AnimItem"))

  # Event when a click on an item happen
  def del_item_event(self, item, target, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "DEL":
          gcompris.sound.play_ogg("sounds/eraser1.wav",
                                  "sounds/eraser2.wav")
          self.del_item(item);
          return True
    return False

  # Event when an event on the drawing area happen
  def create_item_event(self, item, target, event):
    if(event.type == gtk.gdk.BUTTON_PRESS and self.running==True):
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
        gcompris.sound.play_ogg("sounds/bleep.wav")
        self.newitem = None
        #print "----------------------------------------"
        #print "Current image = " + str(self.current_frame)
        #self.dump_group(self.root_anim)

        # All items are formatted this way:
        #     ItemGroup:
        #        Item (line, rect, ellipse, text)
        #        AnchorsGroup
        #           ANCHOR_SE
        #           .....

        self.newitemgroup = \
          goocanvas.Group(
            parent = self.root_anim,
          )


        if self.tools[self.current_tool][0] == "LINE":

          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y

          tuple_points = (x , y,  self.pos_x, self.pos_y)

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
            fill_color_rgba = self.colors[self.current_color],
            line_width = 8.0
            )

        elif self.tools[self.current_tool][0] == "RECT":

          x,y = self.snap_to_grid(event.x,event.y)
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
              stroke_color_rgba=self.colors[self.current_color],
              line_width=4.0
              )
          #          self.newitem.set_data('empty',True)
          gcompris.utils.canvas_set_property(self.newitem, "empty", "True")

        elif self.tools[self.current_tool][0] == "FILL_RECT":

          x,y = self.snap_to_grid(event.x,event.y)
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
            y = points['y1'],
            width = points['x2'] - points['x1'],
            height = points['y2'] - points['y1'],
            fill_color=self.colors[self.current_color],
            fill_color_rgba=self.colors[self.current_color],
            stroke_color_rgba=0x000000FFL,
            line_width=1.0
            )

        elif self.tools[self.current_tool][0] == "CIRCLE":

          x,y = self.snap_to_grid(event.x,event.y)
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
              stroke_color_rgba = self.colors[self.current_color],
              line_width = 5.0
            )
          #          self.newitem.set_data('empty',True)
          gcompris.utils.canvas_set_property(self.newitem, "empty", "True")

        elif self.tools[self.current_tool][0] == "FILL_CIRCLE":

          x,y = self.snap_to_grid(event.x,event.y)
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
            fill_color_rgba = self.colors[self.current_color],
            stroke_color_rgba = 0x000000FFL,
            line_width = 1.0
            )

        elif self.tools[self.current_tool][0] == "TEXT":

          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y

          self.newitem = \
            goocanvas.Text(
              parent = self.newitemgroup,
              x = self.pos_x,
              y = self.pos_y,
              fill_color_rgba = self.colors[self.current_color],
              font = gcompris.FONT_BOARD_BIG_BOLD,
              text = u'?',
              anchor=gtk.ANCHOR_CENTER
              )

        if self.newitem != 0:
          self.anchorize(self.newitemgroup)
          anAnimItem = self.AnimItem()
          anAnimItem.z = self.new_z()
          anAnimItem.canvas_item = self.newitem
          anAnimItem.type = self.tools[self.current_tool][0]
          anAnimItem.canvas_item.set_data("AnimItem", anAnimItem)
          self.framelist.append(anAnimItem)
          self.list_z_actual.append(anAnimItem.z)
          self.draw_created_object = True

          if self.gcomprisBoard.mode == 'draw':
            # needed because used to set the anchors.
            # The item has already the right size
            (x1, x2, y1, y2) = self.get_bounds(self.newitem)
            self.object_set_size_and_pos(self.newitemgroup, x1, x2, y1, y2)

            self.anim_item_select(self.newitemgroup)
            # in draw creation is finished. Object is selected.
            self.newitem = None
            self.newitemgroup = None
          elif self.tools[self.current_tool][0] == "TEXT":
            (x1, x2, y1, y2) = self.get_bounds(self.newitem)
            self.object_set_size_and_pos(self.newitemgroup, x1, x2, y1, y2)
            self.anim_item_select(self.newitemgroup)
            self.newitem = None
            self.newitemgroup = None

      return True

    #
    # MOTION EVENT
    # ------------
    if event.type == gtk.gdk.MOTION_NOTIFY:
      # That's used only in item creation.
      # In draw mode, item creation does not use drag&drop
      if self.gcomprisBoard.mode == 'draw':
        return False

      if ((self.tools[self.current_tool][0] == "IMAGE") or
          (self.tools[self.current_tool][0] == "TEXT")):
        return False

      if event.state & gtk.gdk.BUTTON1_MASK:
        if (self.tools[self.current_tool][0] == "RAISE" or
            self.tools[self.current_tool][0] == "LOWER"):
          return False

        x = event.x
        y = event.y
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
#          self.newitem.set_properties( points=( self.pos_x, self.pos_y, x, y) )
#        elif (self.tools[self.current_tool][0] == "RECT" or
#              self.tools[self.current_tool][0] == "FILL_RECT" or
#              self.tools[self.current_tool][0] == "CIRCLE" or
#              self.tools[self.current_tool][0] == "FILL_CIRCLE"):
#          self.newitem.set_properties(
#            x2=x,
#            y2=y)

        if self.tools[self.current_tool][0] == "LINE":
          points= self.newitem.get_property("points")
          x1 = points[0]
          y1 = points[1]
        elif (self.tools[self.current_tool][0] == "RECT" or
              self.tools[self.current_tool][0] == "FILL_RECT"):
          x1 = self.newitem.get_property("x")
          y1 = self.newitem.get_property("y")
        elif (self.tools[self.current_tool][0] == "CIRCLE" or
              self.tools[self.current_tool][0] == "FILL_CIRCLE"):
          x1 = self.newitem.get_property("center_x") \
              - self.newitem.get_property("radius_x")
          y1 = self.newitem.get_property("center_y") \
              - self.newitem.get_property("radius_y")

        self.object_set_size_and_pos(self.newitemgroup,
                                     x1 = x1,
                                     y1 = y1,
                                     x2 = x,
                                     y2 = y
                                     )
    #
    # MOUSE DRAG STOP
    # ---------------
    if event.type == gtk.gdk.BUTTON_RELEASE:
      # That's used only in item creation.
      # In draw mode, item creation does not use drag&drop
      if self.draw_created_object:
        self.draw_created_object = False
        return True

      if ((self.tools[self.current_tool][0] == "IMAGE") or
          (self.tools[self.current_tool][0] == "TEXT")):
        return False

      if event.button == 1:
        if (self.tools[self.current_tool][0] == "RAISE" or
            self.tools[self.current_tool][0] == "LOWER"):
          return False
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

        return True
    return False

  def snapshot_event(self, item, target, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      self.Anim2Shot()

  def run_flash(self):
    self.flash.props.visibility = goocanvas.ITEM_INVISIBLE
    return False

  def playing_start(self):
    if not self.running:
      self.running = True
      self.root_coloritem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.root_toolitem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.root_playingitem.props.visibility = goocanvas.ITEM_VISIBLE
      self.Anim2Run()

  def playing_event(self, item, target, event, state):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if state:
        self.playing_start()
      else:
        self.playing_stop()

  # Display the animation tools
  def draw_animtools(self):
    # Desactived for the moment

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
        y2 = y + real_item.get_property("text_height")
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


  def get_bounds(self, item):

    bounds = item.get_bounds()

    return (bounds.x1, bounds.y1, bounds.x2, bounds.y2)


  def item_type(self, item):

    item_type = ''

    if gobject.type_name(item)=="GooCanvasGroup":
      item_type='GROUP'
    elif gobject.type_name(item)=="GooCanvasLine":
      item_type='LINE'
    elif gobject.type_name(item)=="GooCanvasImage":
      item_type='IMAGE'
    elif gobject.type_name(item)=="GooCanvasRect":
      try:
        # Can't do it here because it needs to be C compatible for the svgexport
        empty = gcompris.utils.canvas_get_property(item, "empty")
        #empty = item.get_data('empty')
        if not empty:
          empty = False
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

    elif gobject.type_name(item)=="GooCanvasEllipse":
      try:
        #empty = item.get_data('empty')
        # Can't do it here because it needs to be C compatible for the svgexport
        empty = gcompris.utils.canvas_get_property(item, "empty")

        if not empty:
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

    elif gobject.type_name(item)=="GooCanvasText":
      item_type='TEXT'


    return item_type


  #
  # Call anchorize recursively on each item of the group
  #
  def recursive_anchorize(self, root_item):
    for item in root_item.item_list:
      if gobject.type_name(item)=="GooCanvasGroup":
        self.recursive_anchorize(item)
      else:
        self.anchorize(item.get_property("parent"))

  #
  # Add the anchors and callbacks on an item
  #
  def anchorize(self, group):
    # group contains normal items.

    item = group.get_child(0)

    item_type = self.item_type(item)

    if item_type == "GROUP" or not item_type:
      return

    for event in self.events[item_type]:
      item.connect("button_press_event", event)
      item.connect("button_release_event", event)
      item.connect("motion_notify_event", event)

    anchorgroup = \
      goocanvas.Group(
        parent = group,
      )
    anchorgroup.set_data('anchors',True)
    anchorgroup.props.visibility = goocanvas.ITEM_INVISIBLE

    for anchor_type in self.anchors[item_type]:
      anchor = \
          goocanvas.Rect(
        parent = anchorgroup,
        fill_color_rgba=self.ANCHOR_COLOR,
        stroke_color_rgba=0x000000FFL,
        line_width=1,
        )
      anchor.set_data('anchor_type', anchor_type)
      anchor.connect("button_press_event", self.resize_item_event, anchor_type)
      anchor.connect("button_release_event", self.resize_item_event, anchor_type)
      anchor.connect("motion_notify_event", self.resize_item_event, anchor_type)

  def select_tool_TBD(self):
    # Deactivate old button
    self.old_tool_item.set_properties(
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(
          self.tools[self.current_tool][1])))

    # Activate new button
    self.current_tool = self.select_tool_number
    self.old_tool_item = self.select_tool
    self.old_tool_item.set_properties(
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[self.current_tool][2])))
    gcompris.set_cursor(self.tools[self.current_tool][3]);


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

  def item_flip(self, item):
    bounds = self.get_bounds(item)
    (cx, cy) = ( (bounds[2]+bounds[0])/2 , (bounds[3]+bounds[1])/2)

    mat = ( -1, 0, 0, 1, 2*cx, 0)

    item.get_property("parent").set_transform(mat)


###########################################
# Anim 2 specific
###########################################

  class AnimItem:
    def __init__(self):
      self.z = None
      self.frames_info = {}
      self.canvas_item = None
      self.z_previous = None

  def new_z(self):
    if self.list_z_actual != []:
      return int(self.list_z_actual[-1] + 1 )
    else:
      return 1

  def del_AnimItem(self, AnimItem):
    # AnimItem is really deleted only on shot.
    self.list_z_actual.remove(AnimItem.z)
    AnimItem.z = None
    #AnimItem.frames_info[self.current_frame]['deleted']=True

  def z_raise(self, anAnimItem):
    index = self.list_z_actual.index(anAnimItem.z)
    if index < len(self.list_z_actual) -1 :
      if index < len(self.list_z_actual) - 2 :
        anAnimItem.z = (self.list_z_actual[index + 1] + self.list_z_actual[index + 2])/2.0
      else:
        anAnimItem.z = self.list_z_actual[-1] + 1
      self.list_z_actual.pop(index)
      self.list_z_actual.insert(index+1, anAnimItem.z)

  def z_lower(self, anAnimItem):
    index = self.list_z_actual.index(anAnimItem.z)
    if index > 0:
      if index > 1:
         anAnimItem.z = (self.list_z_actual[index - 1] + self.list_z_actual[index - 2])/2.0
      else:
        anAnimItem.z = self.list_z_actual[0] /2.0
      self.list_z_actual.pop(index)
      self.list_z_actual.insert(index-1, anAnimItem.z)

  # Version 2: compare attributes and put those with difference in frames_info
  #
  # self.attributs is list of specific attributs usable for animation
  # There is matrice (rotation, flip) and z position to check too


  def get_animitem_properties(self, anAnimItem):
    properties = {'matrice' : anAnimItem.canvas_item.get_transform() }
    for property_name in self.attributs[anAnimItem.type]:
      properties [property_name] = anAnimItem.canvas_item.get_property(property_name)
      if property_name == 'text':
        properties [property_name] = properties [property_name].decode('UTF-8')
    return properties

  def z_reinit(self):
    for anAnimItem in self.framelist:
      anAnimItem.z = self.list_z_actual.index(anAnimItem.z)+1
      anAnimItem.z_previous =  anAnimItem.z

    self.list_z_last_shot= range(1, len(self.list_z_actual) + 1)
    self.list_z_actual=self.list_z_last_shot[:]

  def z_delete_on_shot(self, anAnimItem):
    if anAnimItem.z_previous:
        self.list_z_last_shot.remove(anAnimItem.z_previous)

  def get_modified_parameters(self, animItem):
        print "get_modified_parameters=", animItem
        modified= {}
        dict_properties = self.get_animitem_properties(animItem)
        print "dict_properties=", dict_properties
        frames = animItem.frames_info.keys()
        if frames != []:
          frames.sort()
          frames.reverse()

          for property in dict_properties.keys():
            for frame in frames:
              print animItem.type, property, frame, animItem.frames_info[frame]
              if animItem.frames_info[frame].has_key(property):
                if not animItem.frames_info[frame][property]==dict_properties[property]:
                  modified[property]=dict_properties[property]
                break
        else:
          modified = dict_properties
          modified.update(self.fixedattributs[animItem.type])
          if animItem.type == 'IMAGE':
            modified['image_name']= animItem.image_name
          modified['create']=True
          self.animlist.append(animItem)

        if animItem.z != animItem.z_previous:
          if animItem.z_previous:
            self.list_z_last_shot.remove(animItem.z_previous)
          modified['z'] = self.z_find_index(animItem)
          self.list_z_last_shot.insert( modified['z'], animItem.z)

        return modified

  def Anim2Shot(self):
    if self.gcomprisBoard.mode == 'draw':
      return
    self.flash.props.visibility = goocanvas.ITEM_VISIBLE
    for anAnimItem in self.framelist[:]:
      if not anAnimItem.z:
        # deleted
        self.z_delete_on_shot(anAnimItem)
        modified = { 'delete': True }
        self.framelist.remove(anAnimItem)
        if self.animlist.count(anAnimItem) == 0:
          # deleted without being in any shot
          continue
      else:
        modified = self.get_modified_parameters(anAnimItem)
        print "Anim2Shot modified", modified


      if len(modified) != 0:
        print ">>MARKING MODIFIED"
        anAnimItem.frames_info[self.current_frame] = modified
#
    self.current_frame = self.current_frame + 1
    self.frames_total =  self.current_frame
    self.z_reinit()
    self.item_frame_counter.set_properties(text=self.current_frame + 1)
    # print self.current_frame + 1
    self.timeout = gobject.timeout_add(500, self.run_flash)

  def z_find_index(self, anAnimItem):
    def f(x): return x < anAnimItem.z

    return len(filter(f, self.list_z_last_shot))

#    self.z_reinit()

  def apply_frame(self, frame):
    print "apply_frame", frame, self.playlist
    for item in self.playlist:
      print "item ",item
      if not item.frames_info.has_key(frame):
        continue
      modif = item.frames_info[frame].copy()
      print "modif ",modif
      if modif.has_key('delete'):
        item.canvas_item.remove()
        continue
      if modif.has_key('create'):
        del modif['create']
        z = modif['z']
        del modif['z']
        matrice = modif['matrice']
        del modif['matrice']
        if item.type == 'IMAGE':
          image = modif['image_name']
          del modif['image_name']
          pixmap = gcompris.utils.load_pixmap(image)
          modif['pixbuf']= pixmap
        item.canvas_item = self.types[item.type](
          parent = self.playing,
          **modif)
        #delta = len(self.playing.get_n_children()) - z -1
        #if delta != 0:
        #  item.canvas_item.lower(delta)
        if matrice:
          item.canvas_item.set_transform(matrice)
        continue
      else:
        if modif.has_key('z'):
          z = modif['z']
          del modif['z']
          index = self.playing.item_list.index(item.canvas_item)
          if index > z:
            item.canvas_item.lower(index - z)
          else:
            item.canvas_item.raise_(z - index)
        if  modif.has_key('matrice'):
          matrice = modif['matrice']
          del modif['matrice']
          if matrice:
            item.canvas_item.set_transform(matrice)
        if len(modif) != 0:
          item.canvas_item.set_properties(**modif)

  def run_anim2(self):
    if self.running:
      if self.current_frame==0:
        self.playing.remove()
        self.playing = goocanvas.Group(
          parent = self.rootitem,
          )
      self.apply_frame((self.current_frame)%(self.frames_total))
      self.current_frame=(self.current_frame+1)%(self.frames_total)
      self.item_frame_counter.set_properties(text=self.current_frame + 1)
    else:
      self.playing.remove()
      self.current_frame = self.frames_total
      self.item_frame_counter.set_properties(text=self.current_frame + 1)
      self.root_anim.props.visibility = goocanvas.ITEM_VISIBLE
      self.root_coloritem.props.visibility = goocanvas.ITEM_VISIBLE
      self.root_toolitem.props.visibility = goocanvas.ITEM_VISIBLE
      self.root_playingitem.props.visibility = goocanvas.ITEM_INVISIBLE
      gcompris.bar_hide(False)
    return self.running

  def Anim2Run(self):
    gcompris.bar_hide(True)
    if self.frames_total==0:
      #print "Mmm... Need to make shots before run anim !!"
      self.running=False
      return
    # Hide the current drawing
    self.root_anim.props.visibility = goocanvas.ITEM_INVISIBLE
    self.playing = goocanvas.Group(
      parent = self.root_anim,
      )

    self.playlist = []
    print "Creating list"
    for aItem in self.animlist:
      print aItem
      playItem = self.AnimItem()
      playItem.frames_info = aItem.frames_info.copy()
      playItem.type = aItem.type
      self.playlist.append(playItem)

    # Show the first drawing
    self.apply_frame(0)
    self.current_frame = 0

    self.timeout = gobject.timeout_add(1000/self.anim_speed, self.run_anim2)


  def anim_item_select(self, item):
    if (self.selected):
      self.anim_item_unselect()

    self.selected = item
    self.selected.get_child(1).props.visibility = goocanvas.ITEM_VISIBLE


  def anim_item_unselect(self):
    if not self.selected:
      return
    if ((gobject.type_name(self.selected.get_child(0)) == "GooCanvasText")
        and (self.last_commit)):
      #suppress preedit
      self.selected.get_child(0).set_properties(text = self.last_commit)
      self.last_commit = None
      gcompris.im_reset()

    self.selected.get_child(1).props.visibility = goocanvas.ITEM_INVISIBLE
    self.selected = None




###############################################
#
#             GLOBAL functions
#
###############################################
def general_save(filename, filetype):
  global fles

  print "filename=%s filetype=%s" %(filename, filetype)
  fles.z_reinit()

  anim2_to_file(filename)


def general_restore(filename, filetype):
  #print "general_restore : ", filename, " type ",filetype

  filename = filename + fles.file_type
  # Determine the file format by reading the first line
  file = open(filename, 'r')
  line = file.read(24)
  file.close();
  filetype = ""
  if(line == "UGCompris draw 2 cPikle"
     or line == "UGCompris anim 2 cPikle"):
    filetype = 'image/gcompris+anim'
  elif(line == "<?xml version='1.0' enco"):
    filetype = 'image/svg+xml'

  if (filetype in ['image/gcompris+anim','image/gcompris+draw']):
    file_to_anim2(filename)


def anim2_to_file(filename):
  global fles

  file = open(filename, 'wb')

  # Save the descriptif frame:
  pickle.dump(fles.format_string['gcompris'], file, True)

  # save the total of frames
  pickle.dump(fles.frames_total, file, True)

  # save the list into
  list_to = []

  # get the list
  list_from = []

  if (fles.gcomprisBoard.mode == 'draw'):
    # in draw we need to get the list in z order, because of svg.
    def get_item_at(z):
      for item in eval('fles.' + fles.itemlist[fles.gcomprisBoard.mode]):
        if item.z == z: return item
    for z in fles.list_z_actual:
      list_from.append(get_item_at(z))

    # now each item needs to get it's frames_info updated
    for anAnimItem in list_from[:]:
      modified = fles.get_modified_parameters(anAnimItem)
      if len(modified) != 0:
        anAnimItem.frames_info[fles.current_frame] = modified

  else:
    list_from = fles.animlist

  for item in list_from:
    frames_info_copied = {}
    for t, d  in item.frames_info.iteritems():
      frames_info_copied[t] = d.copy();
    Sitem = [ item.type, frames_info_copied]
    list_frames = Sitem[1].keys()
    list_frames.sort()
    if ((Sitem[0] == 'TEXT') and (Sitem[1][list_frames[0]].has_key('anchor'))):
        Sitem[1][list_frames[0]]['text-anchor'] = 'middle'
        del Sitem[1][list_frames[0]]['anchor']
    list_to.append(Sitem)

  pickle.dump(list_to, file, True)
  file.close()

def file_to_anim2(filename):
  global fles

  file =   open(filename, 'rb')
  try:
    desc = pickle.load(file)
  except:
    file.close()
    print 'Cannot load ', filename , " as a GCompris animation"
    return

  print "file_to_anim2 A"
  if type(desc) == type('str'):
    # string
    if 'desc' != fles.format_string['gcompris']:
      if (desc == 'GCompris draw 2 cPikle file'
          or desc == 'GCompris anim 2 cPikle file'):
        fles.frames_total = pickle.load(file)
      else:
        print "ERROR: Unrecognized file format, file", filename, ' has description : ', desc
        file.close()
        return
    else:
      print "ERROR: Unrecognized file format (desc), file", filename, ' has description : ', desc
      file.close()
      return

  elif type(desc) == type(1):
    print filename, 'has no description. Are you sure it\'s', fles.format_string['gcompris'],'?'
    # int
    fles.frames_total = desc

  print "file_to_anim2 B"
  picklelist = pickle.load(file)
  file.close()
  list_restore(picklelist)
  print "file_to_anim2 C"

def list_restore(picklelist):
  global fles

  print "list_restore"
  def update_anchors(item):
    global fles

    data_list = { 'LINE' : ['parent', 'points'],
                  'IMAGE' : ['parent', 'x', 'y', 'width', 'height'],
                  'TEXT' : ['parent', 'x', 'y'],
                  'RECT': ['parent', 'x', 'y', 'width', 'height'],
                  'FILL_RECT': ['parent', 'x', 'y', 'width', 'height'],
                  'CIRCLE': ['parent', 'center_x', 'center_y', 'radius_x', 'radius_y'],
                  'FILL_CIRCLE': ['parent', 'center_x', 'center_y', 'radius_x', 'radius_y'],
      }

    data = {}
    for prop in data_list[item.type]:
      print prop
      data[prop] = item.canvas_item.get_property(prop)

    if item.type == 'LINE':
      param = data['parent'], data['points'][0], data['points'][1], data['points'][2], data['points'][3],
    else:
      bounds = item.canvas_item.get_bounds()
      param = data['parent'], bounds.x1, bounds.y1, bounds.x2, bounds.y2

    fles.object_set_size_and_pos(*param)

  fles.selected = None

  for item in fles.framelist:
    print item
    try:
      # can have been removed before by a delete action. No matter
      item.canvas_item.get_property("parent").remove()
    except:
      pass

  fles.framelist = []
  fles.animlist=[]

  for Sitem in picklelist:
    print "Sitem"
    print Sitem
    AItem = fles.AnimItem()
    AItem.type = Sitem[0]
    AItem.frames_info = Sitem[1]
    fles.animlist.append(AItem)

  missing_images = []
  for fles.current_frame in range(fles.frames_total+1):
    for item in fles.animlist[:]:
      if fles.gcomprisBoard.mode == 'draw':
        item.z = fles.animlist.index(item)
      restore_item(item, fles.current_frame, missing_images)

  if missing_images:
    list_images = ''
    for im in missing_images:
      list_images = list_images + im + '\n'
      gcompris.utils.dialog(_('Warning: the following images cannot be accessed on your system.\n') +
                            list_images +
                            _('The corresponding items have been skipped.'),
                            None)
  fles.list_z_last_shot= []
  for item in fles.framelist:
    fles.list_z_last_shot.append(item.z)
    #FIXMEupdate_anchors(item)
  fles.list_z_last_shot.sort()
  fles.list_z_actual = fles.list_z_last_shot[:]
  fles.z_reinit()
  fles.current_frame = fles.frames_total

  fles.root_anim.props.visibility = goocanvas.ITEM_VISIBLE

  # now each item needs to get it's frames_info cleared
  if fles.gcomprisBoard.mode != 'draw':
    fles.item_frame_counter.set_properties(text=fles.current_frame + 1)
  else:
    for anAnimItem in fles.animlist[:]:
      anAnimItem.frames_info = {}

def restore_item(item, frame, missing):
  global fles

  print "restore_item A"
  if not item.frames_info.has_key(frame):
    return

  print "restore_item B"
  modif = item.frames_info[frame].copy()

  if modif.has_key('delete'):
    item.canvas_item.get_property("parent").remove()
    fles.framelist.remove(item)
    return False

  # To be backward compatible, delete unsupported gnomecanvas properties
  try:
    del modif['height_set']
    del modif['width_set']
  except:
    pass

  print "restore_item C"
  # To be backward compatible, rename some properties
  if ( ((item.type == 'CIRCLE') or (item.type == 'FILL_CIRCLE'))
        and (modif.has_key('x2')) ):
    modif['radius_x'] = (modif['x2'] - modif['x1'])/2
    modif['radius_y'] = (modif['y2'] - modif['y1'])/2
    modif['center_x'] = modif['x1'] + modif['radius_x']
    modif['center_y'] = modif['y1'] + modif['radius_y']
    del modif['x1']
    del modif['x2']
    del modif['y1']
    del modif['y2']

  if ( ((item.type == 'RECT') or (item.type == 'FILL_RECT'))
        and modif.has_key('x2') ):
    modif['x'] = modif['x1']
    modif['y'] = modif['y1']
    modif['width'] = modif['x2'] - modif['x1']
    modif['height'] = modif['y2'] - modif['y1']
    del modif['x1']
    del modif['x2']
    del modif['y1']
    del modif['y2']

  if (modif.has_key('width-units')):
    modif['line_width'] = modif['width-units']
    del modif['width-units']

  if (modif.has_key('outline_color_rgba')):
    modif['stroke_color_rgba'] = modif['outline_color_rgba']
    del modif['outline_color_rgba']

  if (modif.has_key('create') or (fles.gcomprisBoard.mode == 'draw')):

    if modif.has_key('create'):
      del modif['create']
    if modif.has_key('z'):
      item.z = modif['z']
      del modif['z']
    if fles.gcomprisBoard.mode == 'draw':
      modif.update(fles.fixedattributs[item.type])
    matrice = modif['matrice']
    del modif['matrice']
    if ((item.type == 'TEXT') and (modif.has_key('text-anchor'))):
      del modif['text-anchor']
      del item.frames_info[frame]['text-anchor']
      item.frames_info[frame]['anchor']= gtk.ANCHOR_CENTER
      modif['anchor']= gtk.ANCHOR_CENTER
    if item.type == 'IMAGE':
      item.image_name =  modif['image_name']

      if (not os.access(gcompris.DATA_DIR + '/' + item.image_name, os.R_OK)
          and not os.access(item.image_name, os.R_OK)):
        missing.append(item.image_name)
        fles.animlist.remove(item)
        return False
      del modif['image_name']
      pixmap = gcompris.utils.load_pixmap(item.image_name)
      modif['pixbuf']= pixmap

    print "restore_item D"
    newitemgroup = goocanvas.Group(
      parent = fles.root_anim,
        )

    print modif
    print "restore_item E %s" %(item.type,)
    item.canvas_item = fles.types[item.type](
      parent = newitemgroup,
      **modif)
    print item.canvas_item
    item.canvas_item.set_data("AnimItem", item)
    fles.anchorize(newitemgroup)
    #delta = len(fles.root_anim.item_list) - item.z -1
    #if delta != 0:
    #  newitemgroup.lower(delta)
    if matrice:
      newitemgroup.set_transform(matrice)
    fles.framelist.append(item)
    print "restore_item F"
    return True
  else:
    if modif.has_key('z'):
      item.z = modif['z']
      del modif['z']
      index = fles.root_anim.item_list.index(item.canvas_item.get_property("parent"))
      if index > item.z:
        item.canvas_item.get_property("parent").lower(index - item.z)
      else:
        item.canvas_item.get_property("parent").raise_(item.z - index)
    if  modif.has_key('matrice'):
      matrice = modif['matrice']
      del modif['matrice']
      if matrice:
        item.canvas_item.get_property("parent").set_transform(matrice)
    if len(modif) != 0:
      # Bourrin: je supprime les ancres et je les remets apres les modifs
      # Pas envie de me faire ch*** a retraiter les resize et les move
      #item.canvas_item.get_property("parent").get_child(1).remove()
      item.canvas_item.set_properties(**modif)
      #fles.anchorize(item.canvas_item.get_property("parent"))
    return True
  print "restore_item END"

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

  anAnimItem = fles.AnimItem()
  anAnimItem.z = fles.new_z()
  anAnimItem.canvas_item = fles.newitem
  anAnimItem.canvas_item.set_data("AnimItem", anAnimItem)
  anAnimItem.type = 'IMAGE'
  anAnimItem.image_name = image
  fles.framelist.append(anAnimItem)
  fles.list_z_actual.append(anAnimItem.z)

  fles.anchorize(fles.newitemgroup)
  width  = pixmap.get_width()
  height = pixmap.get_height()
  fles.object_set_size_and_pos(fles.newitemgroup,
                               x, y,
                               x + width, y + height)
  fles.anim_item_select(fles.newitemgroup)

  fles.newitem = None
  fles.newitemgroup = None


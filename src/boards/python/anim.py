#  gcompris - anim
#
# Time-stamp: <2001/08/20 00:54:45 bruno>
#
# Copyright (C) 2003 Bruno Coudoin (redraw code), 2004 Yves Combe (anim code)
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

#  Version 2 of anim
# Due to performance, the animation code as been rewriten
# For now, the animation is done keeping difference
# in parameters of graphicals object between shots?


from gettext import gettext as _
# PythonTest Board module
import gobject
import gnomecanvas
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

# Note that we only need one of these for any given version of the
# processing class.
#
python_xml = False
# python_xml = True
# try:
#   from xml.dom.DOMImplementation import implementation
#   import xml.sax.writer
#   import xml.utils
# except:
#   python_xml = False
  # Commented out, until we have a clean support for a mainstream mozilla
  #print _('You need the python xml module. Disabling SVG.')

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

    # Cool !!!
    self.empty="empty"

    # global parameter to access object structures from global fonctions
    global fles
    fles=self

    # File formats to save/restore
    #
    # svg has proprietary data to handle animation, base64 included images
    # gcompris is cPickle python saved data
    #
    # svg in draw mode is normal svg file with base64 included images
    global python_xml
    if self.gcomprisBoard.mode == 'draw':
      self.file_type = "image/gcompris+draw"
      if python_xml:
        self.file_type = self.file_type + " image/svg+xml"
    else:
      self.file_type = "image/gcompris+anim"
      #      if python_xml:
      #        self.file_type = self.file_type +"  image/svg+xml+javascript"

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

    # gnomecanvas type corresponding
    self.types = { 'RECT' : gnomecanvas.CanvasRect,
                   'FILL_RECT' : gnomecanvas.CanvasRect,
                   'CIRCLE' : gnomecanvas.CanvasEllipse,
                   'FILL_CIRCLE' : gnomecanvas.CanvasEllipse,
                   'TEXT' : gnomecanvas.CanvasText,
                   'IMAGE' : gnomecanvas.CanvasPixbuf,
                   'LINE' : gnomecanvas.CanvasLine
                   }

    # mutable gnomecanvas attributs
    self.attributs = { 'LINE' : [ "points",
                                  "fill_color_rgba",
                                  ],
                       'RECT' : [ "x1",
                                  "y1",
                                  "x2",
                                  "y2",
                                  "outline_color_rgba",
                                  ],
                       'FILL_RECT' : [ "x1",
                                       "y1",
                                       "x2",
                                       "y2",
                                       "fill_color_rgba",
                                       ],
                       'CIRCLE' : [ "x1",
                                    "y1",
                                    "x2",
                                    "y2",
                                    "outline_color_rgba",
                                    ],
                       'FILL_CIRCLE' : [ "x1",
                                         "y1",
                                         "x2",
                                         "y2",
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

    # non mutable gnomecanvas attributs
    self.fixedattributs = { 'LINE' : { 'width-units': 8.0
                                       },
                            'RECT' : { 'width-units': 4.0
                                       },
                            'FILL_RECT' : { 'width-units': 1.0,
                                            'outline_color_rgba': 0x000000FFL
                                            },
                            'CIRCLE' : { 'width-units': 4.0 },
                            'FILL_CIRCLE' : { 'width-units': 1.0,
                                              'outline_color_rgba': 0x000000FFL
                                              },
                            'TEXT' : { 'font': gcompris.FONT_BOARD_BIG_BOLD,
                                       'anchor' : gtk.ANCHOR_CENTER
                                       },
                            'IMAGE' : { 'width_set': True,
                                        'height_set': True
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

    self.events ['FILL_RECT']         = self.events ['LINE']
    self.events ['FILL_CIRCLE']       = self.events ['LINE']
    self.events ['CIRCLE']  = self.events ['RECT']


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
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    # initialisation
    self.draw_tools()
    self.draw_animtools()
    self.draw_colors()
    self.draw_drawing_area(self.grid_step)
    self.draw_playing_area()
    self.pause(0)

    global python_xml
    if not python_xml:
      #gcompris.utils.dialog(_('Python xml module not found. SVG is disabled. Install the python xml module to enable SVG Save/restore.'), None)
      #print _('Python xml module not found. SVG is disabled. Install the python xml module to enable SVG Save/restore.')
      pass

  def end(self):
    # stop the animation
    if self.running:
      self.playing_stop()

    if self.timeout:
      gobject.source_remove(self.timeout)
    self.timeout = 0

    # Remove the root item removes all the others inside it
    gcompris.set_cursor(gcompris.CURSOR_DEFAULT);
    self.rootitem.destroy()

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
      gcompris.file_selector_save( self.gcomprisBoard, self.selector_section, self.file_type, general_save)
    elif (keyval == gtk.keysyms.F2):
      gcompris.file_selector_load( self.gcomprisBoard, self.selector_section, self.file_type, general_restore)

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

    if (self.selected == None):
      return True
    elif (gobject.type_name(self.selected.item_list[0])!="GnomeCanvasText"):
      #print "Not Text object when got key !!!"
      return True

    textItem = self.selected.item_list[0]
    if (self.last_commit == None):
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
    else:
      if ((oldtext[:1] == u'?') and (len(oldtext)==1)):
        oldtext = u' '
        oldtext = oldtext.strip()

      if (commit_str != None):
        str = commit_str
        self.last_commit = oldtext + str
      if (preedit_str != None):
        str = '<span foreground="red">'+ preedit_str +'</span>'
        self.last_commit = oldtext

      if (len(oldtext) < self.MAX_TEXT_CHAR):
        newtext = oldtext + str
      else:
        newtext = oldtext

    textItem.set(markup=newtext.encode('UTF-8'))

    return True

  # Display the tools
  def draw_tools(self):

    self.root_toolitem = self.rootitem.add(
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.root_toolitem.add(
      gnomecanvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/tool-selector.png")),
      x=5,
      y=5.0,
      width=107.0,
      height=517.0,
      width_set=True,
      height_set=True
      )

    x1=11.0
    x2=56.0
    y=11.0
    stepy=45

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

      item = self.root_toolitem.add(
        gnomecanvas.CanvasPixbuf,
        pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[i][1])),
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
        self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[i][2])))
        gcompris.set_cursor(self.tools[i][3]);


  # Event when a tool is selected
  # Perform instant action or swich the tool selection
  def tool_item_event(self, item, event, tool):

    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/bleep.wav")
        # Some button have instant effects
        if (self.tools[tool][0] == "SAVE"):
#          self.Anim2Shot()
          gcompris.file_selector_save( self.gcomprisBoard, self.selector_section, self.file_type, general_save)
          return False

        elif (self.tools[tool][0] == "LOAD"):
          gcompris.file_selector_load( self.gcomprisBoard, self.selector_section, self.file_type, general_restore)
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
            self.unselect()

            self.playing_start()
            return False

        elif (self.tools[tool][0] != "SELECT") and (self.selected != None):
          self.unselect()

        #
        # Normal case, tool button switch
        # -------------------------------

        # Deactivate old button
        self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[self.current_tool][1])))

        # Activate new button
        self.current_tool = tool
        self.old_tool_item = item
        self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[self.current_tool][2])))
        gcompris.set_cursor(self.tools[self.current_tool][3]);


  # Display the color selector
  def draw_colors(self):

    pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/color-selector.png"))

    x = (self.drawing_area[2] - self.drawing_area[0]
         - pixmap.get_width())/2 + self.drawing_area[0]

    color_pixmap_height = pixmap.get_height()

    y = gcompris.BOARD_HEIGHT - color_pixmap_height - 8

    c = 0

    self.root_coloritem = self.rootitem.add(
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.root_coloritem.add(
      gnomecanvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=x,
      y=y,
      )

    for i in range(0,10):
      x1=x+26+i*56

      for j in range(0,4):
        c = i*4 +j
        item = self.root_coloritem.add(
          gnomecanvas.CanvasRect,
          x1=x1 + 26*(j%2),
          y1=y+8 + (color_pixmap_height/2 -6)*(j/2),
          x2=x1 + 24  + 26*(j%2),
          y2=y + color_pixmap_height/2  + (color_pixmap_height/2 -6)*(j/2),
          fill_color_rgba=self.colors[c],
          outline_color_rgba=0x07A3E0FFL
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
      gcompris.sound.play_ogg("sounds/drip.wav")
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
      gnomecanvas.CanvasRect,
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
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    self.draw_grid(x1,x2,y1,y2,step)

    # Create the root_anim group which contains all the drawings.
    # At root_anim root, there is a group for each drawings.
    self.root_anim = self.rootitem.add(
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    gcompris.utils.item_absolute_move(self.root_anim,
                                      int(self.playing_area[0]-self.drawing_area[0]),
                                      int(self.playing_area[1]-self.drawing_area[1])
                                      )

    # Create a group for the first drawing

    self.flash = self.rootitem.add (
      gnomecanvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/camera.png")),
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
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    self.root_playingitem.hide()

    # intervall = 1000 / anim_speed
    self.anim_speed=5

    run = self.root_playingitem.add(
      gnomecanvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/down.png")),
      x = 15,
      y = 410,
      width = 20,
      height = 20,
      width_set = 1,
      height_set = 1
      )
    run.connect("event", self.speed_event,False)

    self.speed_item = self.root_playingitem.add(
      gnomecanvas.CanvasText,
      text=self.anim_speed,
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x=52,
      y=420,
      anchor=gtk.ANCHOR_CENTER,
      )


    run = self.root_playingitem.add(
      gnomecanvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/up.png")),
      x = 70,
      y = 410,
      width = 20,
      height = 20,
      width_set = 1,
      height_set = 1
      )
    run.connect("event", self.speed_event,True)

    # And finaly a STOP icon
    run = self.root_playingitem.add(
      gnomecanvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("boardicons/draw.png")),
      x = 16,
      y = 110,
      )
    run.connect("event", self.stop_event,True)


  def stop_event(self, item, event, up):
    if event.type == gtk.gdk.BUTTON_PRESS:
      gcompris.sound.play_ogg("sounds/bleep.wav")
      self.playing_stop()

  def playing_stop(self):
    self.running=False
    gobject.source_remove(self.timeout)
    self.run_anim2()

  def speed_event(self, item, event, up):

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
      self.speed_item.set(text=self.anim_speed)

  # Draw the grid
  #
  def draw_grid(self, x1, x2, y1, y2, step):

    self.current_step = step

    color = 0x1D0DFFFFL

    self.grid = self.rootitem.add (
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    self.grid.hide()

    for i in range(int(x1), int(x2), step):
      item = self.grid.add (
        gnomecanvas.CanvasLine,
        points=(i , y1, i , y2),
        fill_color_rgba=color,
        width_units=1.0,
        )
      # Clicking on lines let you create object
      item.connect("event", self.create_item_event)

    for i in range(int(y1), int(y2), step):
      item = self.grid.add (
        gnomecanvas.CanvasLine,
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
      if ((event.type == gtk.gdk.BUTTON_PRESS) and
          (event.button == 1) and
          (gobject.type_name(item)!="GnomeCanvasText")):
        # this one seems broken
        #gcompris.utils.item_rotate_relative(item.get_property("parent"),-10)
        self.rotate_relative(item,-10)
        return True
      else:
        return False

    if self.tools[self.current_tool][0] == "CW":
      if ((event.type == gtk.gdk.BUTTON_PRESS) and
          (event.button == 1) and
          (gobject.type_name(item)!="GnomeCanvasText")):
        self.rotate_relative(item,10)
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
        item.get_property("parent").raise_(1)
        self.z_raise(item.get_data("AnimItem"))
        return True
      else:
        return False

    if self.tools[self.current_tool][0] == "LOWER":
      if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
        item.get_property("parent").lower(1)
        self.z_lower(item.get_data("AnimItem"))
        return True
      else:
        return False

    if self.tools[self.current_tool][0] != "SELECT":
      return False

    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/bleep.wav")
        self.unselect()

    #
    # MOUSE DRAG STOP
    # ---------------
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        if self.draw_created_object:
          self.draw_created_object = False
          return True
        # activate the anchors
        self.selected=item.get_property("parent")
        self.selected.item_list[1].show()

        # Reset the in_select_ofx ofset
        self.in_select_ofx = -1
        self.in_select_ofy = -1

        return True

    if event.state & gtk.gdk.BUTTON1_MASK:
      wx=event.x
      wy=event.y
      #pass in item relative coordinate
      (x, y)= item.w2i( wx, wy)

      bounds = item.get_bounds()
      #bounds = self.get_bounds(item)

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

      return True

    return False

  # Event when a click on an item happen on fill in type object
  def fillin_item_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/paint1.wav")
        if self.tools[self.current_tool][0] == "FILL":
          item.set(fill_color_rgba=self.colors[self.current_color])
          return True
    return False

  # Event when a click on an item happen on border fill type object
  def fillout_item_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/paint1.wav")
        if self.tools[self.current_tool][0] == "FILL":
          item.set(outline_color_rgba=self.colors[self.current_color])
          return True
    return False

  # Del an item and internal struct cleanup
  def del_item(self, item):
    item.get_property("parent").destroy()
    self.del_AnimItem(item.get_data("AnimItem"))

  # Event when a click on an item happen
  def del_item_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "DEL":
          gcompris.sound.play_ogg("sounds/eraser1.wav",
                                  "sounds/eraser2.wav")
          self.del_item(item);
          return True
    return False

  # Event when an event on the drawing area happen
  def create_item_event(self, item, event):
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
        self.newitemgroup = self.root_anim.add(
          gnomecanvas.CanvasGroup,
          x=0.0,
          y=0.0
          )


        if (self.tools[self.current_tool][0] == "DEL" or
            self.tools[self.current_tool][0] == "SELECT" or
            self.tools[self.current_tool][0] == "FILL"):
          # This event is treated in del_item_event to avoid
          # operating on background item and grid
          return False

        elif self.tools[self.current_tool][0] == "LINE":

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
            tuple_points = ( points['x1'], points['y1'], points['x2'], points['y2'])

#     ItemGroup:
#        AnchorsGroup
#           ANCHOR_SE
#           .....
#        Item

          self.newitem = self.newitemgroup.add(
            gnomecanvas.CanvasLine,
            points=tuple_points,
            fill_color_rgba=self.colors[self.current_color],
            width_units=8.0
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

          self.newitem = self.newitemgroup.add(
            gnomecanvas.CanvasRect,
            x1=points['x1'],
            y1=points['y1'],
            x2=points['x2'],
            y2=points['y2'],
            outline_color_rgba=self.colors[self.current_color],
            width_units=4.0
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

          self.newitem = self.newitemgroup.add(
            gnomecanvas.CanvasRect,
            x1=points['x1'],
            y1=points['y1'],
            x2=points['x2'],
            y2=points['y2'],
            fill_color=self.colors[self.current_color],
            fill_color_rgba=self.colors[self.current_color],
            outline_color_rgba=0x000000FFL,
            width_units=1.0
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

          self.newitem = self.newitemgroup.add(
            gnomecanvas.CanvasEllipse,
            x1=points['x1'],
            y1=points['y1'],
            x2=points['x2'],
            y2=points['y2'],
             outline_color_rgba=self.colors[self.current_color],
            width_units=5.0
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

          self.newitem = self.newitemgroup.add(
            gnomecanvas.CanvasEllipse,
            x1=points['x1'],
            y1=points['y1'],
            x2=points['x2'],
            y2=points['y2'],
            fill_color_rgba=self.colors[self.current_color],
            outline_color_rgba=0x000000FFL,
            width_units=1.0
            )

        elif self.tools[self.current_tool][0] == "TEXT":

          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y

          self.newitem = self.newitemgroup.add(
            gnomecanvas.CanvasText,
            x=self.pos_x,
            y=self.pos_y,
            fill_color_rgba=self.colors[self.current_color],
            font=gcompris.FONT_BOARD_BIG_BOLD,
            text=u'?',
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


          if self.tools[self.current_tool][0] == "TEXT":
            (x1, x2, y1, y2) = self.get_bounds(self.newitem)
            self.object_set_size_and_pos(self.newitemgroup, x1, x2, y1, y2)
            self.select_item(self.newitemgroup)
            self.newitem = None
            self.newitemgroup = None

          elif self.gcomprisBoard.mode == 'draw':
            # needed because used to set the anchors.
            # The item has already the right size
            self.object_set_size_and_pos(self.newitemgroup,
                                          x1=points['x1'],
                                          y1=points['y1'],
                                          x2=points['x2'],
                                          y2=points['y2']
                                          )

            self.select_item(self.newitemgroup)
            # in draw creation is finished. Object is selected.
            self.newitem = None
            self.newitemgroup = None

      return True

    #
    # MOTION EVENT
    # ------------
    if event.type == gtk.gdk.MOTION_NOTIFY:
      # That's used only in itel creation.
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

  def snapshot_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      self.Anim2Shot()

  def run_flash(self):
    self.flash.hide()
    return False

  def playing_start(self):
    if not self.running:
      self.running=True
      self.root_coloritem.hide()
      self.root_toolitem.hide()
      self.root_playingitem.show()
      self.Anim2Run()

  def playing_event(self, item, event, state):
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
    self.rootitem.add(
      gnomecanvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/counter.png")),
      x=x_left - -11,
      y=y_top - 2,
      width=70.0,
      height=34.0,
      width_set=True,
      height_set=True
      )

    # First
    #item = self.rootitem.add(
    #  gnomecanvas.CanvasPixbuf,
    #  pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("anim/minibutton.png")),
    #  x = x_left,
    #  y = y_top,
    #  )
    #item.connect("event", self.image_select_event, "first")
    #item = self.rootitem.add(
    #  gnomecanvas.CanvasText,
    #  text = "<<",
    #  x = x_left + 14,
    #  y = y_top + 7,
    #  )
    #item.connect("event", self.image_select_event, "first")

    # Image Number
    self.item_frame_counter = self.rootitem.add(
      gnomecanvas.CanvasText,
      text = self.current_frame + 1,
      x = x_left + minibutton_width + 14,
      y = y_top + 15,
      font = gcompris.skin.get_font("gcompris/board/medium"))

    # Last
    #item = self.rootitem.add(
    #  gnomecanvas.CanvasPixbuf,
    #  pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("anim/minibutton.png")),
    #  x = x_left + 2*minibutton_width,
    #  y = y_top,
    #  )
    #item.connect("event", self.image_select_event, "last")
    #item = self.rootitem.add(
    #  gnomecanvas.CanvasText,
    #  text = ">>",
    #  x = x_left + 2*minibutton_width + 14,
    #  y = y_top + 7,
    #  )
    #item.connect("event", self.image_select_event, "last")

    # Next line
    #y_top += minibutton_height

    # Previous
    #item = self.rootitem.add(
    #  gnomecanvas.CanvasPixbuf,
    #  pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("anim/minibutton.png")),
    #  x = x_left,
    #  y = y_top,
    #  )
    #item.connect("event", self.image_select_event, "previous")
    ##item = self.rootitem.add(
    #  gnomecanvas.CanvasText,
    #  text = "<",
    #  x = x_left + 14,
    #  y = y_top + 7,
    #  )
    #item.connect("event", self.image_select_event, "previous")

    # Next
    #item = self.rootitem.add(
    #  gnomecanvas.CanvasPixbuf,
    #  pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("anim/minibutton.png")),
    #  x = x_left + 2*minibutton_width,
    #  y = y_top,
    #  )
    #item.connect("event", self.image_select_event, "next")
    #item = self.rootitem.add(
    #  gnomecanvas.CanvasText,
    #  text = ">",
    #  x = x_left + 2*minibutton_width + 14,
    #  y = y_top + 7,
    #  )
    #item.connect("event", self.image_select_event, "next")

    # Last button line
    #y_top += minibutton_height

  def object_move(self,object,dx,dy):
    # Unfortunately object.move is broken for 'TEXT' group.

    if gobject.type_name(object.item_list[0])=="GnomeCanvasText":
      (x1,y1,x2,y2) = object.get_bounds()
      (idx, idy) =  object.w2i( dx, dy )
      self.object_set_size_and_pos(object, x1+idx, y1+idy, x2+idx, y2+idy)
    else:
      object.move(dx, dy)

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
      elif anchor_type == self.ANCHOR_T:
        anchor.set(
          x1= (x1 + x2 - self.DEFAULT_ANCHOR_SIZE*3)/2,
          x2= (x1 + x2 + self.DEFAULT_ANCHOR_SIZE*3)/2,
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
      self.Anim2Shot()
      return False

    if event.state & gtk.gdk.BUTTON1_MASK:
      # warning: anchor is in a group of anchors, which is in the object group
      parent=item.get_property("parent").get_property("parent")
      real_item=parent.item_list[0]

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
      elif gobject.type_name(real_item)=="GnomeCanvasText":
        y1=y
        y2=y+real_item.get_property("text_height")
        pass
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

    item_type = ''

    if gobject.type_name(item)=="GnomeCanvasGroup":
      item_type='GROUP'
    elif gobject.type_name(item)=="GnomeCanvasLine":
      item_type='LINE'
    elif gobject.type_name(item)=="GnomeCanvasPixbuf":
      item_type='IMAGE'
    elif gobject.type_name(item)=="GnomeCanvasRect":
      try:
        # Can't do it here because it needs to be C compatible for the svgexport
        empty = gcompris.utils.canvas_get_property(item, "empty")
        #empty = item.get_data('empty')
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
        #empty = item.get_data('empty')
        # Can't do it here because it needs to be C compatible for the svgexport
        empty = gcompris.utils.canvas_get_property(item, "empty")

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


  #
  # Call anchorize recursively on each item of the group
  #
  def recursive_anchorize(self, root_item):
    for item in root_item.item_list:
      if gobject.type_name(item)=="GnomeCanvasGroup":
        self.recursive_anchorize(item)
      else:
        self.anchorize(item.get_property("parent"))

  #
  # Add the anchors and callbacks on an item
  #
  def anchorize(self, group):
    # group contains normal items.

    item = group.item_list[0]

    item_type = self.item_type(item)

    if item_type == "GROUP" or not item_type:
      return

    for event in self.events[item_type]:
      item.connect("event", event)

    anchorgroup=group.add(
      gnomecanvas.CanvasGroup,
      x=0,
      y=0
      )
    anchorgroup.set_data('anchors',True)
    anchorgroup.hide()

    for anchor_type in self.anchors[item_type]:
      anchor=anchorgroup.add(
        gnomecanvas.CanvasRect,
        fill_color_rgba=self.ANCHOR_COLOR,
        outline_color_rgba=0x000000FFL,
        width_pixels=1,
        )
      anchor.set_data('anchor_type', anchor_type)
      anchor.connect("event", self.resize_item_event,anchor_type)

  def select_item(self, group):
    if (self.selected != None):
      self.unselect()

    # Deactivate old button
    self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[self.current_tool][1])))

    # Activate new button
    self.current_tool = self.select_tool_number
    self.old_tool_item = self.select_tool
    self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[self.current_tool][2])))
    gcompris.set_cursor(self.tools[self.current_tool][3]);

    self.selected = group
    self.selected.item_list[1].show()

  def rotate_relative(self, item, angle):
    bounds = item.get_bounds()
    #    print "Item bounds : ", bounds

    #bds = item.get_property("parent").get_bounds()
    #    print "Item parent bounds : ", bounds

    (cx, cy) = ( (bounds[2]+bounds[0])/2 , (bounds[3]+bounds[1])/2)


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

    item.get_property("parent").affine_relative(mat)


    return

  def item_flip(self, item):
    bounds = self.get_bounds(item)
    (cx, cy) = ( (bounds[2]+bounds[0])/2 , (bounds[3]+bounds[1])/2)

    mat = ( -1, 0, 0, 1, 2*cx, 0)

    item.get_property("parent").affine_relative(mat)


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

  # Version 2: compare attributs and put those with difference in frames_info
  #
  # self.attributs is list of specific attributs usable for animation
  # There is matrice (rotation, flip) and z position to check too


  def get_animitem_properties(self, anAnimItem):
    properties = {'matrice' : anAnimItem.canvas_item.i2c_affine((0,0,0,0,0,0)) }
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
    if anAnimItem.z_previous != None:
        self.list_z_last_shot.remove(anAnimItem.z_previous)

  def get_modified_parameters(self, animItem):

        modified= {}
        dict_properties = self.get_animitem_properties(animItem)
        frames = animItem.frames_info.keys()
        if frames != []:
          frames.sort()
          frames.reverse()

          for property in dict_properties.keys():
            for frame in frames:
#              print animItem.type, property, frame, animItem.frames_info[frame]
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
          if animItem.z_previous != None:
            self.list_z_last_shot.remove(animItem.z_previous)
          modified['z'] = self.z_find_index(animItem)
          self.list_z_last_shot.insert( modified['z'], animItem.z)

        return modified

  def Anim2Shot(self):
    if self.gcomprisBoard.mode == 'draw':
      return
    self.flash.show()
    for anAnimItem in self.framelist[:]:
      if anAnimItem.z == None:
        # deleted
        self.z_delete_on_shot(anAnimItem)
        modified = { 'delete': True }
        self.framelist.remove(anAnimItem)
        if self.animlist.count(anAnimItem) == 0:
          # deleted without being in any shot
          continue
      else:
#
        modified = self.get_modified_parameters(anAnimItem)


      if len(modified) != 0:
        anAnimItem.frames_info[self.current_frame] = modified
#
    self.current_frame = self.current_frame + 1
    self.frames_total =  self.current_frame
    self.z_reinit()
    self.item_frame_counter.set(text=self.current_frame + 1)
    # print self.current_frame + 1
    self.timeout = gobject.timeout_add(500, self.run_flash)

  def z_find_index(self, anAnimItem):
    def f(x): return x < anAnimItem.z

    return len(filter(f, self.list_z_last_shot))

#    self.z_reinit()

#  def anim2Run(self):

  def apply_frame(self, frame):
    for item in self.playlist:
      if not item.frames_info.has_key(frame):
        continue
      modif = item.frames_info[frame].copy()
      if modif.has_key('delete'):
        item.canvas_item.destroy()
        continue
      if modif.has_key('create'):
        del modif['create']
        z = modif['z']
        del modif['z']
        matrice = modif['matrice']
        del modif['matrice']
        if item.type == 'IMAGE':
          image =  modif['image_name']
          del modif['image_name']
          pixmap = gcompris.utils.load_pixmap(image)
          modif['pixbuf']= pixmap
        item.canvas_item = self.playing.add(self.types[item.type], **modif)
        delta = len(self.playing.item_list) - z -1
        if delta != 0:
          item.canvas_item.lower(delta)
        item.canvas_item.affine_absolute(matrice)
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
          item.canvas_item.affine_absolute(matrice)
        if len(modif) != 0:
          item.canvas_item.set(**modif)

  def run_anim2(self):
    if self.running:
      if self.current_frame==0:
        self.playing.destroy()
        self.playing = self.rootitem.add(
          gnomecanvas.CanvasGroup,
          x=0.0,
          y=0.0
          )
      self.apply_frame((self.current_frame)%(self.frames_total))
      self.current_frame=(self.current_frame+1)%(self.frames_total)
      self.item_frame_counter.set(text=self.current_frame + 1)
    else:
      self.playing.destroy()
      self.current_frame = self.frames_total
      self.item_frame_counter.set(text=self.current_frame + 1)
      self.root_anim.show()
      self.root_coloritem.show()
      self.root_toolitem.show()
      self.root_playingitem.hide()
      gcompris.bar_hide(False)
    return self.running

  def Anim2Run(self):
    gcompris.bar_hide(True)
    if self.frames_total==0:
      #print "Mmm... Need to make shots before run anim !!"
      self.running=False
      return
    # Hide the current drawing
    self.root_anim.hide()
    self.playing = self.root_anim.add(
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.playlist = []
    for aItem in self.animlist:
      playItem = self.AnimItem()
      playItem.frames_info = aItem.frames_info.copy()
      playItem.type = aItem.type
      self.playlist.append(playItem)

    # Show the first drawing
    self.apply_frame(0)
    self.current_frame = 0

    self.timeout=gobject.timeout_add(1000/self.anim_speed, self.run_anim2)


  def unselect(self):
    if not self.selected:
      return
    if ((gobject.type_name(self.selected.item_list[0])=="GnomeCanvasText")
        and
        (self.last_commit != None)):
      #suppress preedit
      self.selected.item_list[0].set(markup=self.last_commit)
      self.last_commit = None
      gcompris.im_reset()
    self.selected.item_list[1].hide()
    self.selected = None





###############################################
#
#             GLOBAL functions
#
###############################################
def general_save(filename, filetype):
  global fles

  fles.z_reinit()

  #print "general_save : ", filename, " type ",filetype
  if filetype == None:
    filetype = filename.split('.')[-1]
  if (filetype in ['image/svg+xml+javascript','image/svg+xml']):
    anim2_to_svg(filename)
    return
  if (filetype in ['image/gcompris+anim','image/gcompris+draw']):
    anim2_to_file(filename)
    return
  #print "Error File selector return unknown filetype :",'|' + filetype + '|', "!!!"

def general_restore(filename, filetype):
  #print "general_restore : ", filename, " type ",filetype
  if filetype == None:
    filetype = filename.split('.')[-1]

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

  #   print " Detected type ",filetype

  if (filetype in ['image/svg+xml+javascript','image/svg+xml']):
    global python_xml
    if python_xml:
      svg_to_anim2(filename)
    else:
      gcompris.utils.dialog(_('SVG is disabled. Install python xml module to enable it'),None)
    return
  if (filetype in ['image/gcompris+anim','image/gcompris+draw']):
    file_to_anim2(filename)
    return
  #print "Error File selector return unknown filetype :",filetype, "!!!"


def anim2_to_file(filename):
  global fles

  file =   open(filename, 'wb')

  # Save the descriptif frame:
  pickle.dump(fles.format_string['gcompris'],file,True)

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
        Sitem[1][list_frames[0]]['text-anchor']='middle'
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

  picklelist = pickle.load(file)
  file.close()
  list_restore(picklelist)

def list_restore(picklelist):
  global fles

  def update_anchors(item):
    global fles

    data_list = { 'LINE' : ['parent', 'points'],
                  'IMAGE' : ['parent', 'x', 'y', 'width', 'height'],
                  'TEXT' : ['parent', 'x', 'y'],
                  'RECT': ['parent', 'x1', 'y1', 'x2', 'y2'],
                  'FILL_RECT': ['parent', 'x1', 'y1', 'x2', 'y2'],
                  'CIRCLE': ['parent', 'x1', 'y1', 'x2', 'y2'],
                  'FILL_CIRCLE': ['parent', 'x1', 'y1', 'x2', 'y2']
      }

    data = {}
    for prop in data_list[item.type]:
      data[prop]=item.canvas_item.get_property(prop)

    if item.type == 'LINE':
      param = data['parent'], data['points'][0], data['points'][1], data['points'][2], data['points'][3],
    elif item.type == 'TEXT':
      bounds = item.canvas_item.get_bounds()
      param = data['parent'], bounds[0],bounds[1],bounds[2],bounds[3]
    elif item.type == 'IMAGE':
      param = data['parent'], data['x'], data['y'], data['x']+data['width'], data['y']+data['height']
    else:
      param = data['parent'], data['x1'], data['y1'], data['x2'], data['y2']


    fles.object_set_size_and_pos(*param)

  # Historic strate
  #fles.current_image = 0

  fles.selected = None

  for item in fles.framelist:
    try:
      # can have been destroyed before by a delete action. No matter
      item.canvas_item.get_property("parent").destroy()
    except:
      pass

  fles.framelist = []
  fles.animlist=[]

  for Sitem in picklelist:
    AItem = fles.AnimItem()
    AItem.type = Sitem[0]
    AItem.frames_info = Sitem[1]
    fles.animlist.append(AItem)

  missing_images = []
  for fles.current_frame in range(fles.frames_total+1):
    for item in fles.animlist[:]:
      if fles.gcomprisBoard.mode == 'draw':
        item.z = fles.animlist.index(item)
      restore_item( item, fles.current_frame, missing_images)

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
    update_anchors(item)
  fles.list_z_last_shot.sort()
  fles.list_z_actual = fles.list_z_last_shot[:]
  fles.z_reinit()
  fles.current_frame = fles.frames_total

  fles.root_anim.show()

  # now each item needs to get it's frames_info cleared
  if fles.gcomprisBoard.mode != 'draw':
    fles.item_frame_counter.set(text=fles.current_frame + 1)
  else:
    for anAnimItem in fles.animlist[:]:
      anAnimItem.frames_info = {}

def restore_item(item, frame, missing):
  global fles
  if not item.frames_info.has_key(frame):
    return
  modif = item.frames_info[frame].copy()
  if modif.has_key('delete'):
    item.canvas_item.get_property("parent").destroy()
    fles.framelist.remove(item)
    return False
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
    newitemgroup = fles.root_anim.add(
        gnomecanvas.CanvasGroup,
        x=0.0,
        y=0.0
        )
    item.canvas_item = newitemgroup.add(fles.types[item.type], **modif)
    item.canvas_item.set_data("AnimItem", item)
    fles.anchorize(newitemgroup)
    delta = len(fles.root_anim.item_list) - item.z -1
    if delta != 0:
      newitemgroup.lower(delta)
    newitemgroup.affine_absolute(matrice)
    fles.framelist.append(item)
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
      item.canvas_item.get_property("parent").affine_absolute(matrice)
    if len(modif) != 0:
      # Bourrin: je supprime les ancres et je les remets apres les modifs
      # Pas envie de me faire ch*** a retraiter les resize et les move
      #item.canvas_item.get_property("parent").item_list[1].destroy()
      item.canvas_item.set(**modif)
      #fles.anchorize(item.canvas_item.get_property("parent"))
    return True

##############################################
#
#  SVG anim 2 export
#
##############################################

def anim2_to_svg(filename):

    processor_class = DOMProcess

    outfp = open(filename,'w')

    processor = processor_class(outfp)
    processor.run()

    outfp.close()


class BaseProcess:
    """Base class for the conversion processors.  Each concrete subclass
    must provide the following methods:

    initOutput()
        Initialize the output stream and any internal data structures
        that the conversion process needs.

    addRecord(lname, fname, type)
        Add one record to the output stream (or the internal structures)
        where lname is the last name, fname is the first name, and type
        is either 'manager' or 'employee'.

    finishOutput()
        Finish all output generation.  If all work has been on internal
        data structures, this is where they should be converted to text
        and written out.
    """
    def __init__(self, outfp):
        """Store the input and output streams for later use."""
        self.types = { 'RECT' : 'rect',
                       'FILL_RECT' : 'rect',
                       'CIRCLE' : 'ellipse',
                       'FILL_CIRCLE' : 'ellipse',
                       'TEXT' : 'text',
                       'IMAGE' : 'use',
                       'LINE' : 'line'
                   }

        global fles

        self.outfp = outfp
        self.images_list = {}

        self.frames_total = fles.frames_total


        # save the list into
        self.list_to = []

        # get the list
        self.list_from = []

        if (fles.gcomprisBoard.mode == 'draw'):
          # in draw we need to get the list in z order, because of svg.
          def get_item_at(z):
            for item in eval('fles.' + fles.itemlist[fles.gcomprisBoard.mode]):
              if item.z == z: return item
          for z in fles.list_z_actual:
            self.list_from.append(get_item_at(z))

          # now each item needs to get it's frames_info updated
          for anAnimItem in self.list_from[:]:
            modified = fles.get_modified_parameters(anAnimItem)
            if len(modified) != 0:
              anAnimItem.frames_info[fles.current_frame] = modified
        else:
          self.list_from = fles.animlist

        for item in self.list_from:
          frames_info_copied = {}
          for t, d  in item.frames_info.iteritems():
            frames_info_copied[t] = d.copy();
          Sitem = [ item.type, frames_info_copied]
          list_frames = Sitem[1].keys()
          list_frames.sort()
#          if ((Sitem[0] == 'TEXT') and (Sitem[1][list_frames[0]].has_key('anchor'))):
#            Sitem[1][list_frames[0]]['text-anchor']='middle'
#            del Sitem[1][list_frames[0]]['anchor']
          self.list_to.append(Sitem)


    def get_last_rectel_bounds(self, item, frame_no):
        listkeys = item[1].keys()
        listkeys.sort()


        def f(x): return x < frame_no

        #print "rectel last", item, frame_no, filter(f,listkeys)

        for frame in filter(f,listkeys):
          if item[1][frame].has_key('x1'):
             x1 = item[1][frame]['x1']
          if item[1][frame].has_key('x2'):
             x2 = item[1][frame]['x2']
          if item[1][frame].has_key('y1'):
             y1 = item[1][frame]['y1']
          if item[1][frame].has_key('y2'):
             y2 = item[1][frame]['y2']

        return (x1,y1,x2,y2)

    def get_last_line_points(self, item, frame_no):
        listkeys = item[1].keys()
        listkeys.sort()

        def f(x): return x < frame_no

        for frame in filter(f,listkeys):
            if  item[1][frame].has_key('points'):
                points = item[1][frame]['points']
        return points

    def rgb_write(self, rgba):
        red = int ( ( rgba >> 24 ) & 255 )
        green = int ( ( rgba >> 16 ) & 255 )
        blue = int ( ( rgba >> 8 ) & 255 )
        return 'rgb(' + str(red) +',' + str(green) + ',' + str(blue) + ')'

    def run(self):
        """Perform the complete conversion process.

        This method is responsible for parsing the input and calling the
        subclass-provided methods in the right order.
        """
        self.initOutput()

        global fles

        for item in self.list_to:
          self.element = self.document.createElement(self.types[item[0]])
          self.svg.appendChild(self.element)
          listkeys = item[1].keys()
          listkeys.sort()
          for frame_no in listkeys:
            # if draw there is only one key.
            # in this case parameters are put in self.element
            # and not in self.frame

            if fles.gcomprisBoard.mode == 'draw':
              self.frame = self.element
            else:
             self.frame = self.document.createElement("gcompris:frame")
             self.frame.setAttribute('time',str(frame_no))
             self.element.appendChild(self.frame)

            for attr in item[1][frame_no].keys():
              if (self.types[item[0]] == 'rect'):

                if (item[0] == 'RECT') and item[1][frame_no].has_key('create'):
                  self.frame.setAttribute('fill', 'none')
                if (attr == 'x2'):
                  if item[1][frame_no].has_key('x1'):
                    self.frame.setAttribute('width', str(item[1][frame_no]['x2']-item[1][frame_no]['x1']))
                  else:
               	    points = self.get_last_rectel_bounds(item, frame_no)
                    self.frame.setAttribute('width', str(item[1][frame_no]['x2']- points[0]))
                  continue
                if (attr == 'y2'):
                  if item[1][frame_no].has_key('y1'):
                    self.frame.setAttribute('height', str(item[1][frame_no]['y2']-item[1][frame_no]['y1']))
                  else:
               	    points = self.get_last_rectel_bounds(item, frame_no)
                    self.frame.setAttribute('height', str(item[1][frame_no]['y2']- points[1]))
                  continue
                if (attr == 'x1'):
                  self.frame.setAttribute('x', str(item[1][frame_no]['x1']))
                  if not item[1][frame_no].has_key('x2'):
                    points = self.get_last_rectel_bounds(item, frame_no)
                    self.frame.setAttribute('width', str( - item[1][frame_no]['x1'] + points[2]))
                  continue
                if (attr == 'y1'):
                  self.frame.setAttribute('y', str(item[1][frame_no]['y1']))
                  if not item[1][frame_no].has_key('y2'):
                    points = self.get_last_rectel_bounds(item, frame_no)
                    self.frame.setAttribute('width', str( - item[1][frame_no]['y1']+ points[3]))
                  continue
                if (attr == 'fill_color_rgba'):
                  self.frame.setAttribute(
                      'fill',
                      self.rgb_write(item[1][frame_no]['fill_color_rgba']))
                  continue
                if (attr == 'outline_color_rgba'):
                  self.frame.setAttribute(
                      'stroke',
                      self.rgb_write(item[1][frame_no]['outline_color_rgba']))
                  continue
                if (attr == 'width-units'):
                  self.frame.setAttribute(
                      'stroke-width',
                      str(item[1][frame_no]['width-units']))
                  continue

              if (self.types[item[0]] == 'ellipse'):
                if (attr == 'width-units'):
                  self.frame.setAttribute(
                      'stroke-width',
                      str(item[1][frame_no]['width-units']))
                  continue

                if (attr == 'outline_color_rgba'):
                  self.frame.setAttribute(
                      'stroke',
                      self.rgb_write(item[1][frame_no]['outline_color_rgba']))
                  continue
                if (item[0] == 'CIRCLE') and item[1][frame_no].has_key('create'):
                  self.frame.setAttribute('fill', 'none')

                if (attr == 'fill_color_rgba'):
                  self.frame.setAttribute(
                      'fill',
                      self.rgb_write(item[1][frame_no]['fill_color_rgba']))
                  continue

                if (attr == 'x2'):
                  if item[1][frame_no].has_key('x1'):
                    cx = (item[1][frame_no]['x2']+item[1][frame_no]['x1'])/2
                  else:
               	    points = self.get_last_rectel_bounds(item, frame_no)
                    cx = (item[1][frame_no]['x2']+ points[0])/2
                  rx = item[1][frame_no]['x2']-cx
                  self.frame.setAttribute('cx',str(cx))
                  self.frame.setAttribute('rx',str(rx))
                  continue
                if (attr == 'x1'):
                  if item[1][frame_no].has_key('x2'):
                    continue
                  else:
               	    points = self.get_last_rectel_bounds(item, frame_no)
                    cx = (item[1][frame_no]['x1']+ points[2])/2
                    rx = cx - item[1][frame_no]['x1']
                    self.frame.setAttribute('cx',str(cx))
                    self.frame.setAttribute('rx',str(rx))
                  continue

                if (attr == 'y2'):
                  if item[1][frame_no].has_key('y1'):
                    cy = (item[1][frame_no]['y2']+item[1][frame_no]['y1'])/2
                  else:
               	    points = self.get_last_rectel_bounds(item, frame_no)
                    cy = (item[1][frame_no]['y2']+ points[1])/2
                  ry = item[1][frame_no]['y2']-cy
                  self.frame.setAttribute('cy',str(cy))
                  self.frame.setAttribute('ry',str(ry))
                  continue
                if (attr == 'y1'):
                  if item[1][frame_no].has_key('y2'):
                    continue
                  else:
               	    points = self.get_last_rectel_bounds(item, frame_no)
                    cy = (item[1][frame_no]['y1']+ points[3])/2
                    ry = cy - item[1][frame_no]['y1']
                    self.frame.setAttribute('cy',str(cy))
                    self.frame.setAttribute('ry',str(ry))
                  continue

              if (self.types[item[0]] == 'line'):
                if (attr == 'fill_color_rgba'):
                  self.frame.setAttribute(
                      'stroke',
                      self.rgb_write(item[1][frame_no]['fill_color_rgba']))
                  continue
                if (attr == 'width-units'):
                  self.frame.setAttribute(
                      'stroke-width',
                      str(item[1][frame_no]['width-units']))
                  continue
                if (attr == 'points'):
                    if item[1][frame_no].has_key('create'):
                        self.frame.setAttribute('x1', str(item[1][frame_no]['points'][0]))
                        self.frame.setAttribute('y1', str(item[1][frame_no]['points'][1]))
                        self.frame.setAttribute('x2', str(item[1][frame_no]['points'][2]))
                        self.frame.setAttribute('y2', str(item[1][frame_no]['points'][3]))
                    else:
                        last_points = self.get_last_line_points(item, frame_no)
                        points = item[1][frame_no]['points']

                        if points[0] != last_points[0]:
                            self.frame.setAttribute('x1', str(points[0]))
                        if points[1] != last_points[1]:
                            self.frame.setAttribute('y1', str(points[1]))
                        if points[2] != last_points[2]:
                            self.frame.setAttribute('x2', str(points[2]))
                        if points[3] != last_points[3]:
                            self.frame.setAttribute('y2',str( points[3]))
                    continue

              if (self.types[item[0]] == 'text'):
                if (attr == 'fill_color_rgba'):
                  self.frame.setAttribute(
                      'fill',
                      self.rgb_write(item[1][frame_no]['fill_color_rgba']))
                  continue
                if (attr == 'anchor'):
                    self.frame.setAttribute(
                        'text-anchor',
                        'middle')
                    continue
#                if (attr == 'text'):
#                  self.frame.appendChild(self.document.createTextNode(item[1][frame_no]['text'].encode('UTF-8')))
#                  continue

              if ( attr == 'font' ):
                  font = item[1][frame_no]['font']
                  list = font.split()

                  self.frame.setAttribute(
                      'font-size',list[-1] + 'pt')
                  self.frame.setAttribute(
                      'font-family',list[0] + ' ' + list[1])


              if (item[0] == 'IMAGE'):
                if (attr == 'image_name'):
                  image_name=item[1][frame_no]['image_name']
                  list_image_name = image_name.split('/')
                  if self.images_list.has_key(image_name):
                    self.element.setAttribute(
                        'xlink:href',self.images_list[image_name])
                  else:
                    self.symbol = self.document.createElement('symbol')
                    self.defel.appendChild(self.symbol)
                    self.image = self.document.createElement('image')
                    self.symbol.appendChild(self.image)
                    self.symbol.setAttribute(
                        'id', 'image' + str(len(self.images_list)))
                    self.element.setAttribute(
                        'xlink:href', '#image' + str(len(self.images_list)))
                    self.images_list[image_name]= 'image' + str(len(self.images_list))
                    # Base64 included image, to get all in one file
                    #
                    # that's dirty, but i want something simple for kids
                    #
                    # anyway image can be used multiple time,
                    # it will be included only once
                    #
                    # Maybe put file and image in same directory ?
                    #
                    imagefile = open(gcompris.DATA_DIR + '/' + image_name)
                    base64string = base64.encodestring(imagefile.read())
                    self.image.setAttribute(
                        'xlink:href','data:image/png;base64,' + base64string)

                    # get real size of the image.
                    pixmap = gcompris.utils.load_pixmap(image_name)
                    width = pixmap.get_width()
                    height = pixmap.get_height()

                    # Pass the <symbol> with image included.
                    self.image.setAttribute(
                        'x','0')
                    self.image.setAttribute(
                        'y','0')
                    self.image.setAttribute(
                        'width', str(width))
                    self.image.setAttribute(
                        'height',str(height))
                    self.symbol.setAttribute(
                        'viewBox','0 0 '+ str(width) + ' ' + str(height))
                    self.symbol.setAttribute(
                        'preserveAspectRatio','none')
                    self.gcompris_name = self.document.createElement('gcompris:image_name')
                    # Pass the image_name info in private child
                    self.image.appendChild(self.gcompris_name)
                    self.gcompris_name.setAttribute('value',image_name)
                  continue

                if ((attr == 'height_set') or  (attr == 'width_set')):
                  continue

              if (attr == 'matrice'):
                  self.frame.setAttribute(
                      'transform',
                      'matrix' + str(item[1][frame_no]['matrice']))
                  continue

              if fles.gcomprisBoard.mode == 'draw':
                if (attr != 'create'):
                  self.frame.setAttribute(attr,str(item[1][frame_no][attr]))
              else:
                self.frame.setAttribute(attr,str(item[1][frame_no][attr]))
        self.finishOutput()


class DOMProcess(BaseProcess):
    """Concrete conversion process which uses a DOM structure as an
    internal data structure.

    Content is added to the DOM tree for each input record, and the
    entire tree is serialized and written to the output stream in the
    finishOutput() method.
    """
    def initOutput(self):
        global fles
        # Create a new document with no namespace uri, qualified name,
        # or document type
        self.document = implementation.createDocument(None,None,None)
        self.svg = self.document.createElement("svg")
        self.svg.setAttribute("id","svgroot")
        self.svg.setAttribute("width","800")
        self.svg.setAttribute("height","550")
        self.svg.setAttribute("version","1.1")
        self.svg.setAttribute("xmlns","http://www.w3.org/2000/svg")
        self.svg.setAttribute("xmlns:xlink","http://www.w3.org/1999/xlink")
        self.svg.setAttribute("xmlns:html","http://www.w3.org/1999/xhtml")
        self.svg.setAttribute("xmlns:gcompris","http://www.ofset.org/gcompris")
        self.svg.setAttribute("onload","init();")
        self.document.appendChild(self.svg)

        self.metadata = self.document.createElement("metadata")
        self.svg.appendChild(self.metadata)
        self.gc_desc = self.document.createElement("gcompris:description")
        self.metadata.appendChild(self.gc_desc)
        self.gc_desc.setAttribute('value',fles.format_string['svg'])

        if fles.gcomprisBoard.mode != 'draw':
          self.script = self.document.createElement("script")
          self.svg.appendChild(self.script)
          self.gcompris_frames_total = self.document.createElement("gcompris:frames_total")
          self.svg.appendChild(self.gcompris_frames_total)
          self.gcompris_frames_total.setAttribute("value",str(self.frames_total))
          scriptfile = open(gcompris.DATA_DIR + "/anim/animation.js")
          t = self.document.createCDATASection(scriptfile.read())
          self.script.appendChild(t)

        self.defel = self.document.createElement("defs")
        self.svg.appendChild(self.defel)

        if fles.gcomprisBoard.mode != 'draw':
          # html buttons included
          self.foreign = self.document.createElement("foreignObject")
          self.svg.appendChild(self.foreign)
          self.foreign.setAttribute("x","300")
          self.foreign.setAttribute("y","520")
          self.foreign.setAttribute("width","800")
          self.foreign.setAttribute("height","30")
          self.foreign.setAttribute("requiredExtensions","http://www.mozilla.org/SVGExtensions/EmbeddedXHTML")
          self.button1 = self.document.createElement("html:button")
          self.foreign.appendChild(self.button1)
          self.button1.setAttribute("onclick", "start_animation();")
          self.button1text = self.document.createTextNode(u'>'.encode('UTF-8'))
          self.button1.appendChild(self.button1text)

          self.button2 = self.document.createElement("html:button")
          self.foreign.appendChild(self.button2)
          self.button2.setAttribute("onclick", "speed_down();")
          self.button2text = self.document.createTextNode(u'<<'.encode('UTF-8'))
          self.button2.appendChild(self.button2text)

          self.speedtext = self.document.createElement("html:input")
          self.foreign.appendChild(self.speedtext)
          self.speedtext.setAttribute("id","speed_text")
          self.speedtext.setAttribute("type","TEXT")
          self.speedtext.setAttribute("size","6")
          self.speedtext.setAttribute("maxlength","6")
          self.speedtext.setAttribute("value","4 fps")

          self.ratetext = self.document.createElement("html:input")
          self.foreign.appendChild(self.ratetext)
          self.ratetext.setAttribute("id","rate_text")
          self.ratetext.setAttribute("type","TEXT")
          self.ratetext.setAttribute("size","6")
          self.ratetext.setAttribute("maxlength","6")
          self.ratetext.setAttribute("value","")

          self.button3 = self.document.createElement("html:button")
          self.foreign.appendChild(self.button3)
          self.button3.setAttribute("onclick", "speed_up();")
          self.button3text = self.document.createTextNode(u'>>'.encode('UTF-8'))
          self.button3.appendChild(self.button3text)

          self.button4 = self.document.createElement("html:button")
          self.foreign.appendChild(self.button4)
          self.button4.setAttribute("onclick", "stop_animation();")
          self.button4text = self.document.createTextNode(u'||'.encode('UTF-8'))
          self.button4.appendChild(self.button4text)


    def finishOutput(self):
#        t = self.document.createTextNode("\n")
#        self.svg.appendChild(t)
        # XXX toxml not supported by 4DOM
        # self.outfp.write(self.document.toxml())
        xml.dom.ext.PrettyPrint(self.document, self.outfp)
        self.outfp.write("\n")

        # now each item needs to get it's frames_info cleared
        global fles
        if fles.gcomprisBoard.mode == 'draw':
          for anAnimItem in self.list_from[:]:
            del anAnimItem.frames_info[fles.current_frame]






##############################################
#
#  SVG anim 2 import
#
##############################################


#try:
#    import xml.parsers.expat
#except ImportError:
#    import pyexpat

#from xml.parsers import expat

def svg_to_anim2(filename):
    """Process command line parameters and run the conversion."""

    infp = open(filename)
    global fles

    fles.filename = filename
    out = Outputter()
    parser = expat.ParserCreate()

    HANDLER_NAMES = [
        'StartElementHandler', 'EndElementHandler',
        'CharacterDataHandler', 'ProcessingInstructionHandler',
        'UnparsedEntityDeclHandler', 'NotationDeclHandler',
        'StartNamespaceDeclHandler', 'EndNamespaceDeclHandler',
        'CommentHandler', 'StartCdataSectionHandler',
        'EndCdataSectionHandler',
        'DefaultHandler', 'DefaultHandlerExpand',
        #'NotStandaloneHandler',
        'ExternalEntityRefHandler', 'SkippedEntityHandler',
        ]
    parser.returns_unicode = 1

    for name in HANDLER_NAMES:
        setattr(parser, name, getattr(out, name))

    try:
        parser.ParseFile(infp)
    except expat.error:
        print '** Error', parser.ErrorCode, expat.ErrorString(parser.ErrorCode)
        print '** Line', parser.ErrorLineNumber
        print '** Column', parser.ErrorColumnNumber
        print '** Byte', parser.ErrorByteIndex


    infp.close()
    return

class Outputter:
    global fles

    def __init__(self):
       self.fixedattributs = fles.fixedattributs

       # used to check the element coming is the right one
       self.wait_element_list = ['svg']

       # keep where we are in the tree
       self.in_element = []

       # elements constituting the draws.
       self.svg_element = ['use', 'rect', 'ellipse', 'line', 'text']

       # dict with id : image_name pairs
       self.images = {}

       # Format of output in the gcompris anim2 pickle format,
       # close to the anim2 internal format
       self.picklelist = []

       # Item we are looking in. In fact we keep here the item we actually read the frames information.
       self.item_getting = None

       # id of image we are looking in
       self.image_getting = None

       # used to skip elements we are not interested in (foreignObject, script)
       self.wait_end_of = None


    def StartElementHandler(self, name, attrs):
        global fles
        def get_attrs(attrs):
          global fles

          def rgb(r,g,b):
             return (r<<24L) + (g<<16) + (b<<8) + 255

          def matrix(a, b, c, d, e, f):
            return (a , b, c, d, e, f)

          frame_info = {}

          keys = attrs.keys()
          for k in keys:
             if (k == 'create'):
               if (self.item_getting[0] == 'IMAGE'):
                 frame_info['image_name'] = self.image_getting
               if ('fill' in keys):
                 if (attrs['fill']=='none'):
                   if (self.item_getting[0] == 'FILL_RECT'):
                     self.item_getting[0] = 'RECT'
                   if (self.item_getting[0] == 'FILL_CIRCLE'):
                     self.item_getting[0] = 'CIRCLE'
               frame_info.update(self.fixedattributs[self.item_getting[0]])
             if (k == 'transform'):
                 frame_info['matrice'] =  eval(attrs[k])
                 continue
             if (k == 'stroke'):
               # used in CIRCLE LINE and RECT
               if (self.item_getting[0] in ['CIRCLE','RECT','LINE','FILL_RECT','FILL_CIRCLE']):
                 # CIRCLE RECT -> outline_color_rgba
                 # LINE -> fill_color_rgba
                 if (self.item_getting[0] == 'LINE'):
                   frame_info['fill_color_rgba'] = eval(attrs[k])
                 else:
                   frame_info['outline_color_rgba'] = eval(attrs[k])
                 continue
             if (k == 'fill'):
               #used in FILL_CIRCLE and FILL_RECT
               if (self.item_getting[0] in ['FILL_CIRCLE','FILL_RECT','TEXT']):
                 frame_info['fill_color_rgba'] = eval(attrs[k])
               continue
             if (k in ['stroke-width', 'font-size', 'font-family', 'font', 'text-anchor']):
               continue
             if (k in ['x1', 'y1', 'x2', 'y2','x','y','width','height', 'cx', 'cy', 'rx', 'ry']):
               self.points[k] =  eval(attrs[k])
               continue

             if (k == 'text'):
                frame_info['text']=attrs[k]
                continue
             if (k == 'xlink:href'):
               # in draw, this is in attrs becaus the frame is directly in element.
               frame_info['image_name'] = self.image_getting
               continue

             if (not (k in ['x1', 'y1', 'x2', 'y2','x','y','width','height', 'cx', 'cy', 'rx', 'ry'])):
               #print u'Attribut non trait\xe9 :', self.item_getting[0], " ", k, "=", attrs[k]
               frame_info[k] = eval(attrs[k])

          if (self.points != {}):
            if (self.item_getting[0] == 'LINE'):
              for coord in ['x1', 'y1', 'x2', 'y2']:
                if (not self.points.has_key(coord)):
                  self.points[coord] = self.last_points[coord]
              frame_info['points'] = ( self.points['x1'],
                                         self.points['y1'],
                                         self.points['x2'],
                                         self.points['y2'])
              self.last_points.update(self.points)
              self.points = {}

            if (self.item_getting[0] == 'IMAGE'):
              for j in self.points.keys():
                  frame_info[j] = self.points[j]
              self.points = {}

            if (self.item_getting[0] in ['RECT', 'FILL_RECT']):
              dist = { 'x' : 'width', 'y': 'height'}
              for c in ['x', 'y']:
                if (self.points.has_key(c)):
                  b1 = self.points[c]
                  if (self.points.has_key(dist[c])):
                    # x and w changed
                    b2 = b1 + self.points[dist[c]]
                  else:
                    # x changed but not w
                    b2 = b1 + self.last_points[c + '2'] - self.last_points[c + 1]
                else:
                  b1 = self.last_points[c + '1']

                  if (self.points.has_key(dist[c])):
                    # x not changed but w
                    b2 = b1 + self.points[dist[c]]
                  else:
                    # x and w not changed. normally never here
                    b2 = self.last_points[c + '2']

                if (b1 != self.last_points[c+'1']):
                  frame_info[c+'1'] = b1
                  self.last_points[c+'1'] = b1
                if (b2 != self.last_points[c+'2']):
                  frame_info[c+'2'] = b2
                  self.last_points[c+'2'] = b2

            if (self.item_getting[0] in ['CIRCLE', 'FILL_CIRCLE']):
              dist = { 'x' : 'rx', 'y': 'ry'}
              for c in ['x', 'y']:
                if (self.points.has_key('c' + c)):
                  if (self.points.has_key(dist[c])):
                    # c and r change
                    b1 = self.points['c' + c] - self.points[dist[c]]
                    b2 = self.points['c' + c] + self.points[dist[c]]
                  else:
                    # c changed but not r
                    b1 = self.points['c' + c] - (self.last_points[c +'2'] - self.last_points[c +'1'])/2
                    b2 = self.points['c' + c] + (self.last_points[c +'2'] - self.last_points[c +'1'])/2
                else:
                  if (self.points.has_key(dist[c])):
                    # c not changed , r changed
                    b1 = (self.last_points[c + '1'] + self.last_points[c + '2'])/2 - self.points[dist[c]]
                    b2 = b1 + 2 * self.points[dist[c]]
                  else:
                    # c and r not changed
                    b1 = self.last_points[c + '1']
                    b2 = self.last_points[c + '2']

                if (b1 != self.last_points[c+'1']):
                  frame_info[c+'1'] = b1
                  self.last_points[c+'1'] = b1
                if (b2 != self.last_points[c+'2']):
                  frame_info[c+'2'] = b2
                  self.last_points[c+'2'] = b2

            if (self.item_getting[0] in ['TEXT']):
              for c in ['x', 'y']:
                if (self.points[c] != self.last_points[c+'1']):
                  frame_info[c] = self.points[c]
                  self.last_points[c+'1'] = self.points[c]

          return frame_info

        if self.wait_end_of != None:
           # ignore all childs of that element .
           return

        if not (name in self.wait_element_list):
           #print "Error : wait ", self.wait_element_list, " get ", name
           return
        self.in_element.append(name)
        if (name == 'svg'):
          if (fles.gcomprisBoard.mode == 'draw'):
            self.wait_element_list = [ 'defs', 'metadata' ]
          else:
            self.wait_element_list = [ 'script', 'metadata' ]
          return
        if (name == 'metadata'):
           self.wait_element_list = ['gcompris:description']
           return
        if (name == 'gcompris:description'):
           desc = attrs['value']
           return
        if (name == 'script'):
           self.wait_end_of = name
           return
        if (name == 'foreignObject'):
           self.wait_end_of = name
           return
        if (name == 'gcompris:frames_total'):
           fles.frames_total = eval(attrs['value'])
           return
        if (name == 'defs'):
           self.wait_element_list = ['symbol']
           return
        if (name == 'symbol'):
           # just get the id.
           self.wait_element_list = ['image']
           self.image_getting = attrs['id']
        if (name == 'image'):
           #the only interresting thing is the name in gcompris tree of this image. This the child element <gcompris:image_name /> value attribut.
           self.wait_element_list = ['gcompris:image_name']
           return
        if (name == 'gcompris:image_name'):
           #the only interresting thing is the name in gcompris tree of this image. This the child element <gcompris:image_name /> value attribut.
           image_id = attrs['value']
           self.images['#' + self.image_getting] =  image_id
           return
        if (name in self.svg_element):
           self.wait_element_list = ['gcompris:frame']
           # used to check modification in x, y, w, h positions
           self.points = {}
           self.last_points = { 'x1' : None,
                                'y1' : None,
                                'x2' : None,
                                'y2' : None
                                }

           if (name == 'use'):
             self.item_getting = ['IMAGE',{}]
             # We will put image_name when we meet 'create' attr, in frame_info spec. For that we need to keep the name of that image.
             self.image_getting = self.images[attrs['xlink:href']]
           if (name == 'text'):
             self.item_getting = ['TEXT',{}]
           if (name == 'line'):
             self.item_getting = ['LINE',{}]
           if (name == 'rect'):
             # Warning ! Will be changed in RECT
             # if fill='none' found with create attr.
             self.item_getting = ['FILL_RECT',{}]
           if (name == 'ellipse'):
             # Warning ! Will be changed in CIRCLE
             # if fill='none'  found with create attr.
             self.item_getting = ['FILL_CIRCLE',{}]

           if (fles.gcomprisBoard.mode == 'draw'):
             self.item_getting[1][0] = get_attrs(attrs)

        if (name == 'gcompris:frame'):
          self.item_getting[1][eval(attrs['time'])] = {}
          frame_info = self.item_getting[1][eval(attrs['time'])]
          del attrs['time']

          frame_info.update(get_attrs(attrs))

    def EndElementHandler(self, name):
        if (self.wait_end_of != None):
          if (name == self.wait_end_of):
            self.wait_end_of = None
          else: return

        if (name != self.in_element[-1]):
          # Let this print it can handle error
          print "Error close ", name, " but ", self.in_element[-1], " waited."
          return
        self.in_element.pop()

        if (name == 'svg'):
            list_restore(self.picklelist)
            return
        if (name == 'metadata'):
          if (fles.gcomprisBoard.mode == 'draw'):
            self.wait_element_list = [ 'defs' ]
          else:
            self.wait_element_list = [ 'script' ]
          return
        if (name == 'script'):
            self.wait_element_list = [ 'gcompris:frames_total' ]
            return
        if (name == 'gcompris:frames_total'):
            self.wait_element_list = ['defs']
            return
        if (name == 'symbol'):
            self.wait_element_list = ['symbol']
            return
        if (name == 'defs'):
          if (fles.gcomprisBoard.mode == 'draw'):
            self.wait_element_list = self.svg_element
          else:
            self.wait_element_list = ['foreignObject']
            return
        if (name == 'foreignObject'):
            self.wait_element_list = self.svg_element
            return
        if (name in self.svg_element):
            self.wait_element_list = self.svg_element
            self.picklelist.append([self.item_getting[0],self.item_getting[1].copy()])
            self.item_getting = None
            return

    def CharacterDataHandler(self, data):
      pass

    def ProcessingInstructionHandler(self, target, data):
      pass

    def StartNamespaceDeclHandler(self, prefix, uri):
      pass

    def EndNamespaceDeclHandler(self, prefix):
      pass

    def StartCdataSectionHandler(self):
      pass

    def EndCdataSectionHandler(self):
      pass

    def CommentHandler(self, text):
      pass

    def NotationDeclHandler(self, *args):
      pass

    def UnparsedEntityDeclHandler(self, *args):
      pass

    def NotStandaloneHandler(self, userData):
      return 1

    def ExternalEntityRefHandler(self, *args):
      return 1

    def SkippedEntityHandler(self, *args):
      pass

    def DefaultHandler(self, userData):
      pass

    def DefaultHandlerExpand(self, userData):
      pass



def image_selected(image):
  #fles is used because self is not passed through callback
  global fles

  pixmap = gcompris.utils.load_pixmap(image)

  fles.newitem = None
  fles.newitemgroup = fles.root_anim.add(
    gnomecanvas.CanvasGroup,
    x=0.0,
    y=0.0
    )

  x= fles.pos_x
  y= fles.pos_y
  width  = pixmap.get_width()
  height = pixmap.get_height()

  fles.newitem = fles.newitemgroup.add(
    gnomecanvas.CanvasPixbuf,
    pixbuf = pixmap,
    x=x,
    y=y,
    width=width,
    height=height,
    width_set = True,
    height_set = True
    )

  # Tell svg_save the filename
  # Write "filename=image" in the property of newitem
  # Can't do it here because in C python string are unreadable
  # gcompris.utils.canvas_set_property(fles.newitem, "filename", image)

  anAnimItem = fles.AnimItem()
  anAnimItem.z = fles.new_z()
  anAnimItem.canvas_item = fles.newitem
  anAnimItem.canvas_item.set_data("AnimItem", anAnimItem)
  anAnimItem.type = 'IMAGE'
  anAnimItem.image_name = image
  fles.framelist.append(anAnimItem)
  fles.list_z_actual.append(anAnimItem.z)

  fles.anchorize(fles.newitemgroup)
  fles.object_set_size_and_pos(fles.newitemgroup, x, y, x+width, y+height)
  fles.select_item(fles.newitemgroup)

  fles.newitem = None
  fles.newitemgroup = None


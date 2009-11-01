#  gcompris - anim
#
# Copyright (C) 2003, 2009 Bruno Coudoin
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
# Rewrote to support a timeline. Now each object type has its own
# python class.
#
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

    # The list of all the user's objects
    self.animlist = []

    # There is two board in the same code
    # here the diff in parameters
    self.pickle_protocol = 2
    if self.gcomprisBoard.mode == 'draw':
      self.format_string = { 'gcompris' : 'GCompris draw 3 cPikle file',
                             }
    else:
      self.format_string = { 'gcompris' : 'GCompris anim 3 cPikle file',
                             }

    if self.gcomprisBoard.mode == 'draw':
      # DRAW
      #
      # draw is adapted to little kids : big anchors
      self.DEFAULT_ANCHOR_SIZE	= 14

      # draw specific UI
      self.selector_section = "draw3"
    else:
      # Anim
      #
      # Normal anchors
      self.DEFAULT_ANCHOR_SIZE	= 10

      # anim specific UI
      self.selector_section = "anim3"

    # Initialisation. Should not change in draw.
    self.running = False

    # global parameter to access object structures from global fonctions
    global fles
    fles=self

    self.file_type = ".gcanim"

    self.MAX_TEXT_CHAR = 50

    # Part of UI : tools buttons
    # TOOL SELECTION
    self.tools = [
      ["SAVE",           "anim/tool-save.png",            "anim/tool-save.png",                  gcompris.CURSOR_SELECT],
      ["LOAD",           "anim/tool-load.png",            "anim/tool-load.png",                  gcompris.CURSOR_SELECT],
      ["MOVIE",          "anim/tool-movie.png",           "anim/tool-movie_on.png",              gcompris.CURSOR_SELECT],
      ["SELECT",         "anim/tool-select.png",          "anim/tool-select_on.png",             gcompris.CURSOR_SELECT],
      ["RECT",           "anim/tool-rectangle.png",       "anim/tool-rectangle_on.png",          gcompris.CURSOR_RECT],
      ["FILL_RECT",      "anim/tool-filledrectangle.png", "anim/tool-filledrectangle_on.png",    gcompris.CURSOR_FILLRECT],
      ["CIRCLE",         "anim/tool-circle.png",          "anim/tool-circle_on.png",             gcompris.CURSOR_CIRCLE],
      ["FILL_CIRCLE",    "anim/tool-filledcircle.png",    "anim/tool-filledcircle_on.png",       gcompris.CURSOR_FILLCIRCLE],
      ["LINE",           "anim/tool-line.png",            "anim/tool-line_on.png",               gcompris.CURSOR_LINE],
      ["TEXT",           "anim/tool-text.png",            "anim/tool-text_on.png",               gcompris.CURSOR_LINE],
      ["IMAGE",          "anim/tool-image.png",           "anim/tool-image_on.png",              gcompris.CURSOR_DEFAULT],
      ["FILL",           "anim/tool-fill.png",            "anim/tool-fill_on.png",               gcompris.CURSOR_FILL],
      ["DEL",            "anim/tool-del.png",             "anim/tool-del_on.png",                gcompris.CURSOR_DEL],
      ["FLIP",           "anim/tool-flip.png",            "anim/tool-flip_on.png",               gcompris.CURSOR_DEFAULT],
      ["RAISE",          "anim/tool-up.png",              "anim/tool-up_on.png",                 gcompris.CURSOR_DEFAULT],
      ["LOWER",          "anim/tool-down.png",            "anim/tool-down_on.png",               gcompris.CURSOR_DEFAULT],
# FIXME NEED A CLEAN IMPLEMENTATION OF THE ROTATION
#      ["CCW",            "anim/tool-rotation-ccw.png",    "anim/tool-rotation-ccw_on.png",       gcompris.CURSOR_DEFAULT],
#      ["CW",             "anim/tool-rotation-cw.png",     "anim/tool-rotation-cw_on.png",        gcompris.CURSOR_DEFAULT],
      ]

    # keep the tool selected
    self.current_tool=0

    # selected object
    self.selected = None

    # Part of UI : drawing_area is the drawing zone
    # when anim is played, it's masked and playing_area is displayed
    #
    # Drawing area is editing image area
    # Palying area is playing map
    self.drawing_area = [124.0, 20.0, gcompris.BOARD_WIDTH - 15, gcompris.BOARD_HEIGHT - 40]
    self.playing_area = [124.0, 20.0, gcompris.BOARD_WIDTH - 15, gcompris.BOARD_HEIGHT - 40]

    # The frame counter
    # TODO : check if used
    self.item_frame_counter = []

    # Not used for the moment in anim2
    # TODO : fix that
    #self.current_image = 0

    # Part of UI
    # The root items
    self.root_toolitem  = []

    # used to handle draw creation of object
    self.created_object = None

  def start(self):

    self.last_commit = None

    # GCompris initialisation
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=0
    self.gcomprisBoard.number_of_sublevel=0

    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(
      parent =  self.gcomprisBoard.canvas.get_root_item())

    # initialisation
    self.draw_tools()
    self.draw_animtools()

    self.timeline = Timeline(self)
    self.timeline.draw()
    if self.gcomprisBoard.mode == 'draw':
      self.timeline.hide()

    self.color = Color(self.rootitem, self.drawing_area)
    self.color.draw()

    self.draw_drawing_area()
    self.draw_playing_area()

    gcompris.bar_set(0)
    gcompris.bar_location(10, -1, 0.6)

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
                                   general_save, self)
    elif (keyval == gtk.keysyms.F2):
      gcompris.file_selector_load( self.gcomprisBoard,
                                   self.selector_section, self.file_type,
                                   general_restore, self)
    elif (keyval == gtk.keysyms.Left):
      self.timeline.previous()
      return True
    elif (keyval == gtk.keysyms.Right):
      self.timeline.next()
      return True

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
      pixbuf = gcompris.utils.load_pixmap("anim/tool-selector.png"),
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
        pixbuf = gcompris.utils.load_pixmap(self.tools[i][1]),
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
            gcompris.utils.load_pixmap(self.tools[i][2])
        gcompris.set_cursor(self.tools[i][3]);


  # Event when a tool is selected
  # Perform instant action or swich the tool selection
  def tool_item_event(self, item, target, event, tool):

    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/bleep.wav")
        # Some button have instant effects
        if (self.tools[tool][0] == "SAVE"):
          print "SAVE"
          gcompris.file_selector_save( self.gcomprisBoard,
                                       self.selector_section, self.file_type,
                                       general_save, self)
          return False

        elif (self.tools[tool][0] == "LOAD"):
          gcompris.file_selector_load( self.gcomprisBoard,
                                       self.selector_section, self.file_type,
                                       general_restore, self)
          return False

        elif (self.tools[tool][0] == "IMAGE"):
          self.pos_x = gcompris.BOARD_WIDTH/2
          self.pos_y = gcompris.BOARD_HEIGHT/2

          gcompris.images_selector_start(self.gcomprisBoard,
                                         "dataset",
                                         image_selected,
                                         self);
          return False

        elif (self.tools[tool][0] == "MOVIE"):
          if not self.running:
            self.deselect()

            self.playing_start()
            return False

        elif (self.tools[tool][0] != "SELECT"):
          self.deselect()

        #
        # Normal case, tool button switch
        # -------------------------------

        # Deactivate old button
        self.old_tool_item.props.pixbuf = \
            gcompris.utils.load_pixmap(self.tools[self.current_tool][1])

        # Activate new button
        self.current_tool = tool
        self.old_tool_item = item
        self.old_tool_item.props.pixbuf = \
            gcompris.utils.load_pixmap(self.tools[self.current_tool][2])
        gcompris.set_cursor(self.tools[self.current_tool][3]);


  # Display the drawing area
  def draw_drawing_area(self):

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
          line_width=2.0,
          fill_color_rgba=0xFFFFFFFFL,
          stroke_color_rgba=0x111199FFL,
          radius_x=5.0,
          radius_y=5.0)
    item.connect("button_press_event", self.item_event)
    item.connect("button_release_event", self.item_event)
    item.connect("motion_notify_event", self.item_event)

    # The CanvasGroup for the edit space.
    self.root_drawingitem = \
      goocanvas.Group(
        parent = self.rootitem,
      )

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
      pixbuf = gcompris.utils.load_pixmap("anim/down.png"),
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
      pixbuf = gcompris.utils.load_pixmap("anim/up.png"),
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
    run.connect("button_press_event", self.stop_event, True)


  def stop_event(self, item, target, event, up):
    if event.type == gtk.gdk.BUTTON_PRESS:
      gcompris.sound.play_ogg("sounds/bleep.wav")
      self.playing_stop()

  def playing_stop(self):
    self.running = False
    gobject.source_remove(self.timeout)
    self.root_toolitem.props.visibility = goocanvas.ITEM_VISIBLE
    self.root_playingitem.props.visibility = goocanvas.ITEM_INVISIBLE

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
      self.timeout = gobject.timeout_add(1000/self.anim_speed,
                                         self.refresh_loop)
      self.speed_item.set_properties(text = self.anim_speed)

  def deselect(self):
    if self.selected:
      self.selected.deselect()
      self.selected = None

  # Delete the item from the unique list
  # This is called by animitem itself when the object
  # is no more displayed on any time lines.
  def deleteItem(self, item):
    self.animlist.remove(item)

  # Main callback on item comes here first
  # And are then dispatched to the proper functions
  def item_event(self, item, target, event):
    if(event.type == gtk.gdk.BUTTON_PRESS and self.running==True):
      self.playing_stop()
      return False

    if event.type == gtk.gdk.BUTTON_PRESS:

      if event.button == 1:
        gcompris.sound.play_ogg("sounds/bleep.wav")

        animItem = item.get_data("AnimItem")
        if not animItem:
          self.deselect()
        else:
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
            self.selected.move_item_event(item,
                                          target,
                                          event)
          elif self.tools[self.current_tool][0] == "RAISE":
            animItem.raise_()
          elif self.tools[self.current_tool][0] == "LOWER":
            animItem.lower()
          elif self.tools[self.current_tool][0] == "CCW":
            animItem.rotate(-10)
          elif self.tools[self.current_tool][0] == "CW":
            animItem.rotate(10)
          elif self.tools[self.current_tool][0] == "FLIP":
            animItem.flip()

        self.created_object = None
        if self.tools[self.current_tool][0] == "FILL_RECT":
          self.created_object = AnimItemRect(self,
                                             event.x, event.y,
                                             self.color.fill, self.color.stroke, 2)
        elif self.tools[self.current_tool][0] == "RECT":
          self.created_object = AnimItemRect(self,
                                             event.x, event.y,
                                             None, self.color.stroke, 7)
        elif self.tools[self.current_tool][0] == "FILL_CIRCLE":
          self.created_object = AnimItemEllipse(self,
                                                event.x, event.y,
                                                self.color.fill, self.color.stroke, 2)
        elif self.tools[self.current_tool][0] == "CIRCLE":
          self.created_object = AnimItemEllipse(self,
                                                event.x, event.y,
                                                None, self.color.stroke, 7)
        elif self.tools[self.current_tool][0] == "LINE":
          self.created_object = AnimItemLine(self,
                                             event.x, event.y,
                                             None, self.color.stroke, 7)

        if self.created_object:
          self.created_object.create_item_event(item,
                                                target)

          # We keep all object in a unique list
          self.animlist.append(self.created_object)

    #
    # MOTION EVENT
    # ------------
    elif (event.type == gtk.gdk.MOTION_NOTIFY
          and event.state & gtk.gdk.BUTTON1_MASK
          and self.created_object):
        self.created_object.create_item_drag_event(item,
                                                   target,
                                                   event)

    elif (event.type == gtk.gdk.MOTION_NOTIFY
          and event.state & gtk.gdk.BUTTON1_MASK
          and self.tools[self.current_tool][0] == "SELECT"
          and self.selected):
        self.selected.move_item_event(item,
                                      target,
                                      event)

    #
    # MOUSE DRAG STOP
    # ---------------
    elif (event.type == gtk.gdk.BUTTON_RELEASE):
      if self.created_object:
        self.created_object.create_item_drag_event(item,
                                                   target,
                                                   event)
        self.created_object = None
        return True
      else:
        if self.selected:
          self.selected.move_item_event(item,
                                        target,
                                        event)

    return False

  def refresh(self, time):
    # We keep all object in a unique list
    # Here we order them to refresh them at the given time
    for item in self.animlist:
      item.display_at_time(time)

  def refresh_loop(self):
    self.timeline.next()
    return True

  def playing_start(self):
    if not self.running:
      self.running = True
      self.root_toolitem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.root_playingitem.props.visibility = goocanvas.ITEM_VISIBLE
      self.timeout = gobject.timeout_add(1000/self.anim_speed,
                                         self.refresh_loop)


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
      pixbuf = gcompris.utils.load_pixmap("anim/counter.png"),
      x=x_left - -11,
      y=y_top - 2,
      width=70.0,
      height=34.0,
      )

    # Image Number
    self.item_frame_counter = \
        goocanvas.Text(
      parent = self.rootitem,
      text = 1,
      x = x_left + minibutton_width + 14,
      y = y_top - 15,
      font = gcompris.skin.get_font("gcompris/board/medium"),
      fill_color = "white")

  def anim_to_file(self, filename):

    file = open(filename, 'wb')

    # Save the descriptif frame:
    pickle.dump(fles.format_string['gcompris'], file, self.pickle_protocol)

    # Save the last mark
    pickle.dump(self.timeline.get_lastmark(), file, self.pickle_protocol)

    # Save the animation
    pickle.dump(self.animlist, file, self.pickle_protocol)

    file.close()


  def file_to_anim(self, filename):

    file = open(filename, 'rb')
    try:
      desc = pickle.load(file)
    except:
      file.close()
      print 'Cannot load ', filename , " as a GCompris animation"
      return

    if type(desc) == type('str'):
      # string
      if 'desc' != fles.format_string['gcompris']:
        if (desc == 'GCompris draw 3 cPikle file'
            or desc == 'GCompris anim 3 cPikle file'):

          for item in self.animlist:
            item.delete()

          self.timeline.set_lastmark(pickle.load(file))
          self.animlist = pickle.load(file)
          for item in self.animlist:
            item.restore(self)

          self.refresh(self.timeline.get_time())
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

    file.close()


###############################################
#
#             GLOBAL functions
#
###############################################
def general_save(filename, filetype, fles):
  #print "filename=%s filetype=%s" %(filename, filetype)
  fles.anim_to_file(filename)

def general_restore(filename, filetype, fles):
  #print "general_restore : ", filename, " type ",filetype
  fles.file_to_anim(filename)

def image_selected(image, fles):
  #fles is used because self is not passed through callback
  print "image selected %s" %(image,)
  pixmap = gcompris.utils.load_pixmap(image)

  # Always display the image at the center of the drawing area
  cx = (fles.drawing_area[2] - fles.drawing_area[0]) / 2
  cy = (fles.drawing_area[3] - fles.drawing_area[1]) / 2

  fles.created_object = AnimItemPixmap(fles, cx, cy, pixmap)

  if fles.created_object:
    fles.created_object.create_item_event(fles.root_drawingitem,
                                          fles.root_drawingitem)

  # We keep all object in a unique list
  fles.animlist.append(fles.created_object)


#  gcompris - louis_braille.py
#
# Copyright (C) 2011 Bruno Coudoin | Srishti Sethi
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
#
# louis_braille activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import goocanvas
import pango
import random
import ConfigParser
from BrailleChar import *

from gcompris import gcompris_gettext as _

COLOR_ON = 0XFFFFFFFFL
COLOR_OFF= 0X000000FFL
CIRCLE_FILL = "white"
CELL_WIDTH = 30

LOUIS_BRAILLE_NAME = ['L','O','U','I','S','B','R','A','I','L','L','E']

class Gcompris_louis_braille:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    self.gcomprisBoard.maxlevel = 12

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    # Set the buttons we want in the bar
    gcompris.bar_set (0)

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "louis_braille/background.svgz")

    #Initialize variables
    self.won = 0
    self.counter = 0
    self.gamewon = 0
    self.item = 0

    self.groupitem_array = []
    self.coorditem_array = []

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.

    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())


    self.read_data()
    self.display_game(self.gcomprisBoard.level)

  def read_data(self):
    '''Load the activity data'''
    config = ConfigParser.RawConfigParser()
    p = gcompris.get_properties()
    filename = gcompris.DATA_DIR + '/' + self.gcomprisBoard.name + '/activity.desktop'
    try:
      gotit = config.read(filename)
      if not gotit:
         gcompris.utils.dialog(_("Cannot find the file '{filename}'").format(filename=filename),None)
         return False

    except ConfigParser.Error as error:
      gcompris.utils.dialog(_("Failed to parse data set '{filename}' with error:\n{error}").
                              format(filename=filename, error=error), None)
      return False

    self.dataset = config
    return True

  def display_game(self, level):
      if(level == 12):
          gcompris.bar_location(gcompris.BOARD_WIDTH - 120, -1, 0.8)

          n_lines = 11
          self.reordering = Reordering(self, n_lines)

          # Insert the lines in the correct order
          for index in range(n_lines):
              self.reordering.add_line( str(self.dataset.get(str(index + 1), _("story"))))

          ok = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#OK",
                         tooltip = _("Click to confirm your sequence")
                         )
          ok.translate(200,-70)
          ok.connect("button_press_event", self.ok_event)
          gcompris.utils.item_focus_init(ok, None)

      else :
          gcompris.bar_location(gcompris.BOARD_WIDTH - 140, 350, 0.8)

          goocanvas.Rect( parent = self.rootitem,
                          x = 50,
                          y = 20,
                          width = 700,
                          height = 95,
                          radius_x = 10,
                          radius_y = 10,
                          stroke_color_rgba = 0x666666FFL,
                          fill_color_rgba = 0x33333333L,
                          line_width = 2.0)


          #Previous Button
          ok = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#PREVIOUS",
                         tooltip = _("Click to move to previous story page")
                         )
          ok.translate(-300,50)
          ok.connect("button_press_event", self.previous_event, self.gcomprisBoard.level)
          gcompris.utils.item_focus_init(ok, None)

          #Next Button
          ok = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#NEXT",
                         tooltip = _("Click to move to next story page")
                         )
          ok.translate(75,-58)
          ok.connect("button_press_event", self.next_event, self.gcomprisBoard.level)
          gcompris.utils.item_focus_init(ok, None)

          #Display name of louis braille
          for index in range(5):
              BrailleChar(self.rootitem,(index+1) *(CELL_WIDTH + 20)+30,
                             20, 50, LOUIS_BRAILLE_NAME[index] ,COLOR_ON, COLOR_OFF,
                             CIRCLE_FILL, CIRCLE_FILL, True,False ,False, None)
          for index in range(5,12):
              BrailleChar(self.rootitem,(index+1) *(CELL_WIDTH + 20)+65,
                             20, 50, LOUIS_BRAILLE_NAME[index] ,COLOR_ON, COLOR_OFF,
                             CIRCLE_FILL, CIRCLE_FILL,True,False ,False, None)

          story = self.dataset.get(str(level),_("story"))

          #Rectangle for YEAR
          goocanvas.Rect(parent=self.rootitem,
                          x=330,
                          y=382,
                          width=180,
                          height=25,
                          stroke_color="orange",
                          fill_color="white",
                          line_width=2.0)

          #Displaying the YEAR
          goocanvas.Text(parent = self.rootitem,
                   x=420.0,
                   y=395.0,
                   text=str(self.dataset.get(str(level) , "year")),
                   fill_color="black",
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = 'SANS 17'
                   )

          #Rectangle for STORY
          goocanvas.Rect(parent=self.rootitem,
                          x=50,
                          y=415,
                          width=720,
                          height=80,
                          stroke_color="orange",
                          fill_color="white",
                          line_width=2.0)

          #Displaying the STORY
          goocanvas.Text(parent = self.rootitem,
                   x=400.0,
                   y=455.0,
                   text=str(story),
                   fill_color="black",
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   width = 700,
                   font = 'SANS 17'
                   )

          #Displaying the IMAGE
          goocanvas.Image(parent = self.rootitem,
                                 pixbuf = gcompris.utils.load_pixmap
                                 (str(self.dataset.get(str(level),"image"))),
                                 x = 300,
                                 y = 120,
                                 )

  def ok_event(self, event ,target ,item):
      if ( self.reordering.is_done() ):
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.TUX)
          self.gamewon = 1
      else :
          self.reordering.is_not_done()

  def previous_event(self, event, target,item, level):
      if (self.gcomprisBoard.level == 1):
          self.gcomprisBoard.level = self.gcomprisBoard.maxlevel
      else :
          self.gcomprisBoard.level = level - 1
      self.end()
      self.start()

  def next_event(self, event, target, item, level):
      if (self.gcomprisBoard.level == self.gcomprisBoard.maxlevel):
          self.gcomprisBoard.level = 1
      else :
          self.gcomprisBoard.level = level + 1
      self.end()
      self.start()

  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()

  def ok(self):
    print("louis_braille ok.")


  def repeat(self):
    print("louis_braille repeat.")


  def config(self):
    print("louis_braille config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

  def pause(self, pause):
    print("louis_braille pause. %i" % pause)
    if(self.gamewon == 1):
        self.gcomprisBoard.level = 1
        self.end()
        self.start()

  def set_level(self, level):
    print("louis_braille set level. %i" % level)

class Reordering:
  """This class manages the reordering activity"""

  def __init__(self, louisbraille, max_item):
    self.louisbraille = louisbraille
    self.rootitem = louisbraille.rootitem
    self.rectbox_array = []
    self.index = 0
    self.randoms = range(max_item)
    random.shuffle(self.randoms)
    self.orders = range(max_item)

  def dump(self):
    for index, item in enumerate(self.orders):
      print index, item.get_data("index")

  # Return True if all the items are properly placed
  def is_done(self):
     self.dump()
     for index, item in enumerate(self.orders):
       group_index = item.get_data("index")
       if ( group_index != index ):
         return False
     return True

  #To indicate correct and wrong lines
  def is_not_done(self):
      for index , item in enumerate (self.orders):
          group_index = item.get_data("index")
          if ( group_index != index ):
              self.rectbox_array[index].set_property("fill_color","#F95234")
          else :
              self.rectbox_array[index].set_property("fill_color","#5DF934")

  def add_line(self, text):
    position = self.randoms[ self.index ]
    y = (position + 0.5) * 43
    group_item = goocanvas.Group(parent = self.rootitem,
                                 y = y)
    # Save in the item itself where is its correct position
    group_item.set_data("index", self.index)
    # Create Rounded Rectangles for each story
    rect_box = goocanvas.Rect(parent = group_item,
                   x = 100,
                   y = 0,
                   width = 550,
                   height = 40,
                   radius_x = 17,
                   radius_y = 17,
                   stroke_color = "orange",
                   fill_color = "white",
                   line_width = 2.0)

    self.rectbox_array.append(rect_box)
    self.orders[ position ] = group_item

    # Displaying the STORY
    goocanvas.Text(parent = group_item,
                   x = 370.0,
                   y = 20,
                   text = text,
                   fill_color = "black",
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = 'SANS 9',
                   width = 500,
                   )

    # It is hard to manage focus when we move the item
    # gcompris.utils.item_focus_init(self.dragText, self.dragRect)
    group_item.connect("button_press_event", self.component_drag)
    group_item.connect("motion_notify_event", self.component_drag)
    group_item.connect("button_release_event", self.component_drag)

    self.index += 1

  # Return the index in self.orders of the given group
  def get_group_index(self, group):
    return self.orders.index(group)

  def move_group(self, from_index, to_index):
    from_item = self.orders[from_index]
    (from_x, from_y) = from_item.get_properties('x', 'y')
    print "from", from_x, from_y
    to_item = self.orders[to_index]
    to_x = to_item.get_data('xref')
    to_y = to_item.get_data('yref')
    print "to", to_x, to_y
    from_item.set_properties(x = to_x, y = to_y)
    to_item.set_data("xref", from_x)
    to_item.set_data("yref", from_y)
    self.orders[from_index], self.orders[to_index] = self.orders[to_index], self.orders[from_index]

  def swap_with_group_near(self, group, y):
    for index, item in enumerate(self.orders):
      bounds = item.get_bounds()
      if ( group != item and
           y < bounds.y2 and y > bounds.y1 ):
        print "got it at index " + str(index)
        print self.get_group_index(group)
        self.move_group( index, self.get_group_index(group) )
        return

  def component_drag(self, widget, target, event):
      groupitem = target.get_parent()
      groupitem.raise_(None)

      if event.type == gtk.gdk.BUTTON_PRESS:
        bounds = groupitem.get_bounds()
        self.offset_y = event.y
        # We save the x and y coord in xref and yref for future use
        (x, y) = groupitem.get_properties('x', 'y')
        groupitem.set_data("xref", x)
        groupitem.set_data("yref", y)

      elif ( event.type == gtk.gdk.MOTION_NOTIFY
             and event.state & gtk.gdk.BUTTON1_MASK ):
          groupitem.translate(0, event.y - self.offset_y)

          (x, y) = self.louisbraille.gcomprisBoard.canvas.\
              convert_from_item_space(groupitem, event.x, event.y)
          self.swap_with_group_near(groupitem, y)

      elif event.type == gtk.gdk.BUTTON_RELEASE:
        self.dump()
        groupitem.set_transform(None)
        to_x = groupitem.get_data('xref')
        to_y = groupitem.get_data('yref')
        groupitem.set_properties(x = to_x, y = to_y)

      return True
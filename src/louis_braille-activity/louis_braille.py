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
import gettext
from gcompris import gcompris_gettext as _
from gettext import dgettext as D_

COLOR_ON = 0XFF0000FFL
COLOR_OFF= 0XFFFFFFFFL
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

    self.read_data()
    self.display_story()

  def read_data(self):
    '''Load the activity data'''
    config = ConfigParser.RawConfigParser()
    p = gcompris.get_properties()
    filename = gcompris.DATA_DIR + '/' + self.gcomprisBoard.name + '/activity.desktop'
    try:
      gotit = config.read(filename)
      if not gotit:
         gcompris.utils.dialog(D_(gcompris.GETTEXT_ERROR,"Cannot find the file '{filename}'").format(filename=filename),None)
         return False

    except ConfigParser.Error, error:
      gcompris.utils.dialog(D_(gcompris.GETTEXT_ERROR,"Failed to parse data set '{filename}' with error:\n{error}").
                              format(filename=filename, error=error), None)
      return False

    self.dataset = config
    return True

  def display_game(self):
    gcompris.bar_location(gcompris.BOARD_WIDTH - 120, -1, 0.8)

    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    n_lines = 11
    self.reordering = Reordering(self, n_lines)

    # Insert the lines in the correct order
    for index in range(n_lines):
      self.reordering.add_line( _(str(self.dataset.get(str(index + 1),
                                                       "story"))) )

      ok = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#OK",
                         tooltip = _("Click to confirm your sequence")
                         )
      ok.translate(200,-70)
      ok.connect("button_press_event", self.ok_event)
      gcompris.utils.item_focus_init(ok, None)


  def display_story(self):
    gcompris.bar_location(gcompris.BOARD_WIDTH - 140, 350, 0.8)

    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    goocanvas.Rect( parent = self.rootitem,
                    x = 50,
                    y = 20,
                    width = 700,
                    height = 95,
                    radius_x = 10,
                    radius_y = 10,
                    stroke_color_rgba = 0x666666FFL,
                    fill_color_rgba = 0x33333333L,
                    line_width = 2.0 )


    # Previous Button
    ok = goocanvas.Svg( parent = self.rootitem,
                        svg_handle = gcompris.skin.svg_get(),
                        svg_id = "#PREVIOUS",
                        )
    ok.translate(-300,50)
    ok.connect("button_press_event", self.previous_event, None)
    gcompris.utils.item_focus_init(ok, None)

    # Next Button
    ok = goocanvas.Svg(parent = self.rootitem,
                       svg_handle = gcompris.skin.svg_get(),
                       svg_id = "#NEXT",
                       )
    ok.translate(75,-58)
    ok.connect("button_press_event", self.next_event, None)
    gcompris.utils.item_focus_init(ok, None)

    # Display name of louis braille
    for index in range(5):
      BrailleChar(self.rootitem,(index+1) *(CELL_WIDTH + 20)+30,
                  20, 50, LOUIS_BRAILLE_NAME[index] ,COLOR_ON, COLOR_OFF,
                  CIRCLE_FILL, CIRCLE_FILL, True,False ,False, None)
    for index in range(5,12):
      BrailleChar(self.rootitem,(index+1) *(CELL_WIDTH + 20)+65,
                  20, 50, LOUIS_BRAILLE_NAME[index] ,COLOR_ON, COLOR_OFF,
                  CIRCLE_FILL, CIRCLE_FILL,True,False ,False, None)

    # Displaying the YEAR
    self.year_item = \
        goocanvas.Text(parent = self.rootitem,
                       x = 420.0,
                       y = 400.0,
                       fill_color="black",
                       anchor = gtk.ANCHOR_CENTER,
                       alignment = pango.ALIGN_CENTER,
                       font = gcompris.skin.get_font("gcompris/title")
                       )

    # Rectangle for YEAR
    self.year_rect = \
        goocanvas.Rect(parent=self.rootitem,
                       x = 40,
                       y = 425,
                       width = 720,
                       height = 85,
                       stroke_color = "orange",
                       fill_color = "white",
                       line_width = 2.0)
    self.year_rect.lower(None)

    # Rectangle for STORY
    goocanvas.Rect(parent=self.rootitem,
                   x = 20,
                   y = 425,
                   width = 760,
                   height = 85,
                   stroke_color = "orange",
                   fill_color = "white",
                   line_width = 2.0)

    # Displaying the STORY
    self.storyitem = \
        goocanvas.Text(parent = self.rootitem,
                       x=400.0,
                       y=465.0,
                       fill_color="black",
                       anchor = gtk.ANCHOR_CENTER,
                       alignment = pango.ALIGN_CENTER,
                       width = 750,
                       font = gcompris.skin.get_font("gcompris/subtitle")
                       )

    self.imageitem = \
        goocanvas.Image(parent = self.rootitem,
                        x = 300,
                        y = 120,
                        )
    self.imageitem.connect("button_press_event", self.next_event, None)
    self.display_image()

  def display_image(self):
    pixbuf = \
        gcompris.utils.load_pixmap( (str(self.dataset.get(str(self.gcomprisBoard.level),
                                                          "image"))) )
    self.imageitem.set_properties(pixbuf = pixbuf)
    story = _(self.dataset.get( str(self.gcomprisBoard.level), "story"))
    self.storyitem.set_properties(text = story)
    year = _(self.dataset.get( str(self.gcomprisBoard.level) , "year"))
    self.year_item.set_properties(text = year)
    bounds = self.year_item.get_bounds()
    gapx = 10
    gapy = 4
    self.year_rect.set_properties(
      x = bounds.x1 - gapx,
      y = bounds.y1 - gapy,
      width = (bounds.x2 - bounds.x1) + gapx * 2,
      height = (bounds.y2 - bounds.y1) + gapy * 2,
      )


  def ok_event(self, event ,target ,item):
    if ( self.reordering.is_done() ):
      gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.TUX)
      self.gamewon = 1
    else :
      self.reordering.is_not_done()

  def refresh_level(self):
    if ( self.gcomprisBoard.level == 1 ):
      self.end()
      self.display_story()
    elif ( self.gcomprisBoard.level != 12 ):
      self.display_image()
    else:
      self.end()
      self.display_game()

  def previous_event(self, event, target,item, dummy):
      if (self.gcomprisBoard.level == 1):
          self.gcomprisBoard.level = self.gcomprisBoard.maxlevel
      else :
          self.gcomprisBoard.level -= 1
      self.refresh_level()

  def next_event(self, event, target, item, dummy):
      if (self.gcomprisBoard.level == self.gcomprisBoard.maxlevel):
          self.gcomprisBoard.level = 1
      else :
          self.gcomprisBoard.level += 1
      self.refresh_level()

  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()

  def ok(self):
    pass


  def repeat(self):
    pass


  def config(self):
    pass


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

  def pause(self, pause):
    if(self.gamewon == 1):
        self.gcomprisBoard.level = 1
        self.end()
        self.start()

  def set_level(self, level):
    pass

class Reordering:
  """This class manages the reordering activity"""

  def __init__(self, louisbraille, max_item):
    self.louisbraille = louisbraille
    self.rootitem = louisbraille.rootitem
    self.index = 0
    self.randoms = range(max_item)
    random.shuffle(self.randoms)
    self.orders = range(max_item)

  def dump(self):
    for index, item in enumerate(self.orders):
      print index, item.get_data("index")

  # Return True if all the items are properly placed
  def is_done(self):
     for index, item in enumerate(self.orders):
       group_index = item.get_data("index")
       if ( group_index != index ):
         return False
     return True

  # To indicate correct and wrong lines
  def is_not_done(self):
      for index , item in enumerate (self.orders):
          group_index = item.get_data("index")
          rectbox = item.get_data("rectbox")
          if ( group_index != index ):
              rectbox.set_property("fill_color_rgba", 0xed6d6deeL)
          else :
              rectbox.set_property("fill_color_rgba", 0x80e072eeL)

  # Reset the lines to the original color, no good or bad position mark
  def reset_selection(self):
      for index , item in enumerate (self.orders):
          group_index = item.get_data("index")
          rectbox = item.get_data("rectbox")
          rectbox.set_property("fill_color_rgba", 0xEEEEEEEEL)

  def add_line(self, text):
    position = self.randoms[ self.index ]
    y = (position + 0.5) * 43
    group_item = goocanvas.Group(parent = self.rootitem,
                                 y = y)
    # Save in the item itself where is its correct position
    group_item.set_data("index", self.index)
    # Create Rounded Rectangles for each story
    rect_box = goocanvas.Rect(parent = group_item,
                   x = 30,
                   y = 0,
                   width = 640,
                   height = 40,
                   radius_x = 5,
                   radius_y = 5,
                   stroke_color = "black",
                   fill_color_rgba = 0xEEEEEEEEL,
                   line_width = 2.0)
    group_item.set_data("rectbox", rect_box)
    self.orders[ position ] = group_item

    # Displaying the STORY
    goocanvas.Text(parent = group_item,
                   x = 350.0,
                   y = 20,
                   text = text,
                   fill_color = "black",
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = gcompris.skin.get_font("gcompris/board/tiny"),
                   width = 635,
                   )

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
    to_item = self.orders[to_index]
    to_x = to_item.get_data('xref')
    to_y = to_item.get_data('yref')
    from_item.set_properties(x = to_x, y = to_y)
    to_item.set_data("xref", from_x)
    to_item.set_data("yref", from_y)
    self.orders[from_index], self.orders[to_index] = self.orders[to_index], self.orders[from_index]

  def swap_with_group_near(self, group, y):
    for index, item in enumerate(self.orders):
      bounds = item.get_bounds()
      if ( group != item and
           y < bounds.y2 and y > bounds.y1 ):
        self.move_group( index, self.get_group_index(group) )
        return

  def component_drag(self, widget, target, event):
      groupitem = target.get_parent()
      groupitem.raise_(None)

      if event.type == gtk.gdk.BUTTON_PRESS:
        self.reset_selection()
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
        groupitem.set_transform(None)
        to_x = groupitem.get_data('xref')
        to_y = groupitem.get_data('yref')
        groupitem.set_properties(x = to_x, y = to_y)

      return True

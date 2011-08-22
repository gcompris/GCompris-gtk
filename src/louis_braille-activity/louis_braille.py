#  gcompris - louis_braille.py
#
# Copyright (C) 2003, 2008 Bruno Coudoin
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

COLOR_ON = 0X00FFFFL
COLOR_OFF= 0X00000000L
CIRCLE_FILL = "red"
CELL_WIDTH = 30

LOUIS_BRAILLE_NAME = ['L','O','U','I','S','B','R','A','I','L','L','E']
NUMBERS = [12,13,14,15,16,17,18,19,20,21,22]
SEQUENCE = ['a','b','c','d','e','f','g','h','i','j','k']
NUMBER_SEQUENCE = [[1,'a'],[2,'b'],[3,'c'],[4,'d'],[5,'e'],[6,'f'],
                   [7,'g'],[8,'h'],[9,'i'],[10,'j'],[11,'k']]
random.shuffle(NUMBERS)
random.shuffle(NUMBER_SEQUENCE)

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
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "louis_braille/back.png")

    #Initialize variables
    self.won = 0
    self.counter = 0
    self.gamewon = 0
    self.offset_x = self.offset_y = 0
    self.init_coord = {}


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

          for index in range(11):
              #Create Rounded Rectangles for each story
              self.dragRect = goocanvas.Rect(parent=self.rootitem,
                          x = 100,
                          y = (index + 0.5) * 43,
                          width = 550,
                          height = 40,
                          radius_x = 17,
                          radius_y = 17,
                          stroke_color = "orange",
                          fill_color = "white",
                          line_width = 2.0)

              #Displaying the STORY
              self.dragText = goocanvas.Text(parent = self.rootitem,
                   x = 370.0,
                   y = (index + 1) * 43,
                   text = str(self.dataset.get(str(NUMBER_SEQUENCE[index][0]),"_story")),
                   fill_color = "black",
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = 'SANS 9',
                   width = 500,
                   )
              gcompris.utils.item_focus_init(self.dragText, self.dragRect)
              self.dragText.connect("button_press_event", self.component_drag)
              self.dragText.connect("motion_notify_event", self.component_drag)
              self.dragText.connect("button_release_event", self.component_drag)


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

          story = self.dataset.get(str(level),"_story")

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
                   text=str(self.dataset.get(str(level),"_year")),
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

  def component_drag(self, widget, target, event):
      if event.type == gtk.gdk.MOTION_NOTIFY:
        if event.state & gtk.gdk.BUTTON1_MASK:
          # Save the click to image offset
          if self.offset_x == 0:
            bounds = widget.get_bounds()
            self.offset_x = (event.x - bounds.x1)
            self.offset_y = (event.y - bounds.y1)

          widget.set_properties(x = 360,
                                y = event.y - self.offset_y)

      if event.type == gtk.gdk.BUTTON_RELEASE:
        if event.button == 1:
          bounds = widget.get_bounds()

        return True

  def enter_callback(self, event, widget, index):
      print self.widget_array[index].get_text()

  def ok_event(self, event ,target ,item):
      for index in range(11):
          if(self.widget_array[index].get_text() == NUMBER_SEQUENCE[index][1]):
              self.won +=1
      if (self.won == 11):
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.TUX)
          self.gamewon = 1
      else :
          gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.TUX)

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
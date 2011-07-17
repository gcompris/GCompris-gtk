#  gcompris - braille_objects.py
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
# braille_objects activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import goocanvas
import pango
import ConfigParser
from BrailleChar import *
from braille_alphabets import *

from gcompris import gcompris_gettext as _

#Constants Declaration
COLOR_ON = 0xFF0000FFL
COLOR_OFF = 0X000000
CIRCLE_STROKE = "black"
CIRCLE_FILL = "#DfDfDf"

class Gcompris_braille_objects:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    self.gcomprisBoard.maxlevel = 8

    #Boolean variable declaration
    self.mapActive = False

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    # Set a background image
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())


    #Creating a Red Border
    goocanvas.Rect(
      parent = self.rootitem,
      x = 10,
      y = 10,
      width = 780,
      height = 500,
      stroke_color = "dark red",
      line_width=40)

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(300,-1,0.6)
    gcompris.bar_set_level(self.gcomprisBoard)

    #REPEAT ICON
    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT_ICON)
    gcompris.bar_location(300,-1,0.7)

    goocanvas.Text(
      parent = self.rootitem,
      x=400.0,
      y=50.0,
      text=_("Braille these Objects"),
      fill_color="black",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER,
      font = 'SANS 20'
      )
    goocanvas.Text(
      parent = self.rootitem,
      x=320.0,
      y=100.0,
      text=_("Category : "),
      fill_color="black",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER,
      font = 'SANS 15'
      )
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
      #Displaying the Category
      goocanvas.Text(parent = self.rootitem,
                   x=450.0,
                   y=100.0,
                   text=str(self.dataset.get(str(level),"category")),
                   fill_color="black",
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = 'SANS 15'
                   )
      #Displaying the OK Button
      goocanvas.Image(parent = self.rootitem,
                                 pixbuf = gcompris.utils.load_pixmap("braille_objects/ok.svg"),
                    x = 30,
                    y = 380,
                    )


      #Displaying the CONTENT
      k = 0
      i = 0
      j = 2
      while k < 2:
        spacing = 0

        #Displaying the OBJECTS
        for index in range(i,j):
            goocanvas.Image(parent = self.rootitem,
                                 pixbuf = gcompris.utils.load_pixmap
                                 (str(self.dataset.get(str(level),"image_" + str(index+1)))),
                    x = 370 * spacing + 130,
                    y = 150 * k + 130
                    )


            answer_length = len(str(self.dataset.get(str(level),"ans_" + str(index+1))))

            #Displaying the Braille Tile
            for index in range(answer_length):
                BrailleChar(self.rootitem,( 370 * spacing + 130 ) + (40 * index + 1 ), 180 * k + 200, 50 ,
                             '', COLOR_ON, COLOR_OFF ,CIRCLE_FILL, CIRCLE_STROKE, True, True ,False, None)

            spacing = spacing + 1
        i += 2
        j += 2
        k += 1


  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("braille_objects ok.")


  def repeat(self):
    if(self.mapActive):
          self.end()
          self.start()
          self.mapActive = False
          self.pause(0)
    else :
          self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
          self.rootitem = goocanvas.Group(parent=
                                   self.gcomprisBoard.canvas.get_root_item())
          gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
          map_obj = BrailleMap(self.rootitem, COLOR_ON, COLOR_OFF, CIRCLE_FILL, CIRCLE_STROKE)
          self.mapActive = True
          self.pause(1)


  def config(self):
    print("braille_objects config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_braille_objects key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    print("braille_objects pause. %i" % pause)


  def set_level(self, level):
    gcompris.sound.play_ogg("sounds/receive.wav")
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()

  def increment_level(self):
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.gcomprisBoard.sublevel += 1
    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
        self.gcomprisBoard.sublevel=1
        self.gcomprisBoard.level += 1
        if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
            self.gcomprisBoard.level = 1
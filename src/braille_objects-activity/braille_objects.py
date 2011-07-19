#  gcompris - braille_objects.py
#
# Copyright (C) 2003, 2008 Bruno Coudoin | Srishti Sethi
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

    #Constant Declarations
    self.answer_string = ""
    self.counter = 0
    self.tile_counter = 0

    #Defining an Object Array
    self.tile_array = []

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    # Set a background image
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.root = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())


    #Creating a Red Border
    goocanvas.Rect(
      parent = self.root,
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
      parent = self.root,
      x=400.0,
      y=50.0,
      text=_("Braille these Objects"),
      fill_color="black",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER,
      font = 'SANS 20'
      )
    goocanvas.Text(
      parent = self.root,
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
      goocanvas.Text(parent = self.root,
                   x=450.0,
                   y=100.0,
                   text=str(self.dataset.get(str(level),"category")),
                   fill_color="black",
                   anchor = gtk.ANCHOR_CENTER,
                   alignment = pango.ALIGN_CENTER,
                   font = 'SANS 15'
                   )
      #Displaying the OK Button
      ok_button = goocanvas.Image(parent = self.root,
                                 pixbuf = gcompris.utils.load_pixmap("braille_objects/ok.svg"),
                    x = 630,
                    y = 50,
                    tooltip =_("Click for Correction of Braille Objects"),
                    )
      ok_button.connect("button_press_event", self.ok_event)
      gcompris.utils.item_focus_init(ok_button, None)


      #Displaying the CONTENT
      k = 0
      i = 0
      j = 2
      while k < 2:
        spacing = 0

        #Displaying the OBJECTS
        for index in range(i,j):
            goocanvas.Image(parent = self.root,
                                 pixbuf = gcompris.utils.load_pixmap
                                 (str(self.dataset.get(str(level),"image_" + str(index+1)))),
                    x = 370 * spacing + 130,
                    y = 150 * k + 130
                    )

            answer = str(self.dataset.get(str(level),"ans_" + str(index+1)))

            #Create a string of all answers
            self.answer_string = self.answer_string + answer
            answer_length = len(str(self.dataset.get(str(level),"ans_" + str(index+1))))

            #Displaying the Braille Tile
            for index in range(answer_length):
                #Naming Objects from Variable Class Names
                self.obj = "self.obj" + str(self.tile_counter)
                #Appending a new object name to an array with tile_counter
                self.tile_array.append(self.obj)
                #Defining Object to BrailleChar Instance to produce braille_tile
                self.tile_array[self.tile_counter] = BrailleChar(self.root,( 370 * spacing + 130 ) + (40 * index + 1 ),
                                         180 * k + 200, 50 ,'', COLOR_ON, COLOR_OFF ,CIRCLE_FILL, CIRCLE_STROKE, True,
                                         True ,False, callback = self.letter_change )
                self.tile_counter +=1
            spacing = spacing + 1
        i += 2
        j += 2
        k += 1

  def letter_change(self, letter):
      self.correct_letter = letter

  def ok_event(self, item, target, event):
      self.list_array =  list(self.answer_string)
      for index in range(self.tile_counter):
          dot_letter = self.tile_array[index].get_letter()
          if self.list_array[index].upper() == dot_letter:
              self.counter += 1
      if (self.counter == self.tile_counter) :
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.SMILEY)
      else :
          self.gamewon = 0
          gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.SMILEY)

  def end(self):
    # Remove the root item removes all the others inside it
    self.root.remove()


  def ok(self):
    print("braille_objects ok.")


  def repeat(self):
    if(self.mapActive):
          self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
          self.root.props.visibility = goocanvas.ITEM_VISIBLE
          self.mapActive = False
    else :
          self.root.props.visibility = goocanvas.ITEM_INVISIBLE
          self.rootitem = goocanvas.Group(parent=
                                   self.gcomprisBoard.canvas.get_root_item())
          gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
          map_obj = BrailleMap(self.rootitem, COLOR_ON, COLOR_OFF, CIRCLE_FILL, CIRCLE_STROKE)
          self.mapActive = True

  def config(self):
    print("braille_objects config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_braille_objects key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    self.board_paused = pause
    if(self.board_paused) and (self.gamewon == 1):
          self.gamewon = 0
          self.declare()
          self.increment_level()
          self.end()
          self.start()
    if(self.board_paused) and (self.gamewon == 0):
        self.declare()
        self.end()
        self.start()

  #This function is to reinitialize string arrays and counters
  def declare(self):
      self.tile_counter = 0
      self.tile_array = []
      self.list_array = []
      self.answer_string = ""

  def set_level(self, level):
    self.declare()
    gcompris.sound.play_ogg("sounds/receive.wav")
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()

  def increment_level(self):
    self.declare()
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.gcomprisBoard.sublevel += 1
    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
        self.gcomprisBoard.sublevel=1
        self.gcomprisBoard.level += 1
        if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
            self.gcomprisBoard.level = 1
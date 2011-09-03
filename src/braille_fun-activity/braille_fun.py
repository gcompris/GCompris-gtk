#  gcompris - braille_fun.py
#
# Copyright (C) 2011 Srishti Sethi
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
# braille_fun activity.
import gtk
import gtk.gdk
import gcompris
import gobject
import gcompris.utils
import gcompris.skin
import gcompris.sound
import gcompris.bonus
import gcompris.score
import goocanvas
import pango
import random
import string
from BrailleChar import *
from BrailleMap import *
from gcompris import gcompris_gettext as _


COLOR_ON = 0X00FFFF
COLOR_OFF = 0X000000
CIRCLE_FILL = "light green"
CIRCLE_STROKE = "black"
CELL_WIDTH = 30

class Gcompris_braille_fun:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel = 3
    self.gcomprisBoard.maxlevel = 5
    self.text_color = 0X0000FFFFL
    self.counter = 0
    self.timerAnim = 0
    self.delay = 40

    #Boolean variable declaration
    self.mapActive = False

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    #Array declaration
    self.letter_array = []
    self.alphabet_array = []
    self.tile_array = []
    self.tile_index_array = []

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(300,-1,0.6)
    gcompris.bar_set_level(self.gcomprisBoard)

    #REPEAT ICON
    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT_ICON)
    gcompris.bar_location(300,-1,0.7)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.root = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())
    #To create a group item for horizontal and vertical text
    self.horizontalTextRoot = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())
    self.verticalTextRoot = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    #Display the sublevel
    gcompris.score.start(gcompris.score.STYLE_NOTE, 530, 460,
                         self.gcomprisBoard.number_of_sublevel)
    gcompris.score.set(self.gcomprisBoard.sublevel)

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "braille_fun/hillside.svg")

    self.display_game(self.gcomprisBoard.level)

  def display_game(self, level):
      #SVG Handle for TUX Plane
      svghandle = gcompris.utils.load_svg("braille_fun/plane.svg")
      self.tuxplane = goocanvas.Svg(
                                     parent = self.root,
                                     svg_handle = svghandle,
                                     svg_id = "#PLANE",
                                     tooltip = _("I am TUX PLANE")
                                     )
      self.tuxplane.translate(50 * level, 40 )

      #Animated Tux plane.Move from left to right
      self.tuxplane.animate(900,
                            20 ,
                           1,
                           1,
                           True,
                           25000 + level * 9000,
                           250,
                           goocanvas.ANIMATE_FREEZE)

      for index in range (level) :
      	   #Select a random letter and append it to self.letter_array
           if (level == 1):
                #Letters from A to J
           	letter = random.choice(string.letters[random.randint(0,9)])
           elif(level == 2):
           	#Letters from K to T
           	letter = random.choice(string.letters[random.randint(10,18)])
           elif(level == 3):
           	#Letters from U to Z
           	letter = random.choice(string.letters[random.randint(19,25)])
           else :
           	letter = random.choice(string.letters[random.randint(0,25)])

           self.letter_array.append(letter.upper())
           self.animateString = "".join(self.letter_array)

           #Display rectangle for braille tile
           goocanvas.Rect(parent=self.root,
                          x = 300 + 90 * index,
                          y = 155,
                          width = 80 ,
                          height = 100,
                          stroke_color = "black",
                          fill_color = "#DfDfDf",
                          line_width=2.0)

           #Naming Objects from Variable Class Names and appending
           #to array self.tile_array
           self.obj = "self.obj" + str(index)
           self.tile_array.append(self.obj)

           #Defining Object to BrailleChar Instance to produce braille_tile
           self.tile_array[index] = BrailleChar(self.root, 300 + 90 * index , 150, 80,
                                        '',COLOR_ON ,COLOR_OFF, "#DfDfDf","black",
                                         True, True ,True, callback = self.letter_change)

      #Display alphabets for TUX_PLANE horizontally and vertically
      lengthString = len(self.animateString)
      for index in range(lengthString):
          self.alphabet_horizontal = goocanvas.Text(
                         parent = self.horizontalTextRoot,
                         x =  50 * index  ,
                         y = 60.0 ,
                         text = self.animateString[index],
                         fill_color="black",
                         anchor = gtk.ANCHOR_CENTER,
                         alignment = pango.ALIGN_CENTER,
                         font = 'SANS 50'
                         )
          self.alphabet_vertical = goocanvas.Text(
                         parent = self.verticalTextRoot,
                         x=50.0 + 50 * index ,
                         y=130.0 ,
                         text=self.animateString[index],
                         fill_color="black",
                         anchor = gtk.ANCHOR_CENTER,
                         alignment = pango.ALIGN_CENTER,
                         font = 'SANS 50'
                         )
          self.alphabet_array.append(self.alphabet_vertical)

      #Animate or move letters horizontally and vertically
      self.horizontalTextRoot.animate(900,
                           5,
                           1,
                           1,
                           True,
                           20000 + level * 16000,
                           250,
                           goocanvas.ANIMATE_FREEZE)

      self.verticalTextRoot.animate(5,
                           415,
                           1,
                           1,
                           True,
                           30000 + level * 5000,
                           250,
                           goocanvas.ANIMATE_FREEZE)

      #To call a function when animation finishes
      self.verticalTextRoot.connect("animation-finished", self.animationFinished)


  def letter_change(self, letter):
      self.letter = letter
      for index in range(self.gcomprisBoard.level):
          #Change the text color of alphabet correctly identified in the braille tile
          if (self.tile_array[index].get_letter() == self.letter_array[index]):
              self.alphabet_array[index].set_property("fill_color_rgba",self.text_color)

          #If an alphabet's color changes to blue the counter is increased
          #and the index number of tile is appended to a self.tile_index_array
          if self.alphabet_array[index].get_property("fill_color_rgba") == self.text_color \
              and index not in self.tile_index_array :
              self.tile_index_array.append(index)
              self.counter +=1

      #If counter equals level number then call a timer_loop
      if (self.counter == self.gcomprisBoard.level) :
          self.timerAnim = gobject.timeout_add(40, self.timer_loop)

  #This timer loop is to wait for milliseconds before calling a
  #Bonus API
  def timer_loop(self):
      self.delay -= 1
      if(self.delay == 0):
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.SMILEY)
          self.delay = 40
      if self.delay < 40 :
          gobject.timeout_add(self.delay,self.timer_loop)

  def animationFinished(self, item, status):
      if (self.counter != self.gcomprisBoard.level) :
          gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.SMILEY)

  def end(self):
    # Remove the root item removes all the others inside it
    self.root.remove()
    self.horizontalTextRoot.remove()
    self.verticalTextRoot.remove()

  def ok(self):
    print("braille_fun ok.")

  def repeat(self):
    if(self.mapActive):
          self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
          gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "braille_fun/hillside.svg")
          self.root.props.visibility = goocanvas.ITEM_VISIBLE
          self.horizontalTextRoot.props.visibility = goocanvas.ITEM_VISIBLE
          self.verticalTextRoot.props.visibility = goocanvas.ITEM_VISIBLE
          self.mapActive = False
    else :
          self.root.props.visibility = goocanvas.ITEM_INVISIBLE
          self.horizontalTextRoot.props.visibility = goocanvas.ITEM_INVISIBLE
          self.verticalTextRoot.props.visibility = goocanvas.ITEM_INVISIBLE
          self.rootitem = goocanvas.Group(parent=
                                   self.gcomprisBoard.canvas.get_root_item())
          gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
          map_obj = BrailleMap(self.rootitem, COLOR_ON, COLOR_OFF, CIRCLE_FILL, CIRCLE_STROKE)
          self.mapActive = True

  def config(self):
    print("braille_fun config.")

  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_braille_fun key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    self.board_paused = pause
    #This is to hide the Repeat board
    if self.mapActive == True:
        self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
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

  def declare(self):
    self.letter_array = []
    self.tile_array = []
    self.alphabet_array = []
    self.tile_index_array = []
    self.counter = 0

  def increment_level(self):
    self.declare()
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.gcomprisBoard.sublevel += 1
    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
        self.gcomprisBoard.sublevel=1
        self.gcomprisBoard.level += 1
        if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
            self.gcomprisBoard.level = 1

  def set_level(self, level):
    #This is to hide the Repeat board in case set_level button gets pressed
    if self.mapActive == True:
        self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.declare()
    gcompris.sound.play_ogg("sounds/receive.wav")
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()

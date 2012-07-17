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
from BrailleChar import *
from BrailleMap import *
from gcompris import gcompris_gettext as _


class Gcompris_braille_fun:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel = 26
    self.gcomprisBoard.maxlevel = 3
    self.text_color = 0X0000FFFFL
    self.counter = 0
    self.timerAnim = 0

    #Boolean variable declaration
    self.mapActive = False

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = False

    #Array declaration
    self.letter_array = []
    self.alphabet_array = []
    self.tile_array = []
    self.tile_index_array = []

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    self.reseted = False
    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(300,-1,0.6)
    gcompris.bar_set_level(self.gcomprisBoard)

    #REPEAT ICON
    pixmap = gcompris.utils.load_svg("braille_alphabets/target.svg")
    gcompris.bar_set_repeat_icon(pixmap)
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

    # The root item for the help
    self.map_rootitem = \
        goocanvas.Group( parent = self.gcomprisBoard.canvas.get_root_item() )
    BrailleMap(self.map_rootitem, self.move_back)
    self.map_rootitem.props.visibility = goocanvas.ITEM_INVISIBLE

    # Display the sublevel
    gcompris.score.start(gcompris.score.STYLE_NOTE, 530, 460,
                         self.gcomprisBoard.number_of_sublevel)

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "braille_fun/hillside.svg")

    self.display_game(self.gcomprisBoard.level)

  def move_back(self,event,target,item):
    self.map_rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.mapActive = False

  def display_game(self, level):

      gcompris.score.set(self.gcomprisBoard.sublevel)

      self.planeroot = goocanvas.Group( parent =
                                        self.root )
      # SVG Handle for TUX Plane
      svghandle = gcompris.utils.load_svg("braille_fun/plane.svg")
      self.tuxplane = goocanvas.Svg(
                                     parent = self.planeroot,
                                     svg_handle = svghandle,
                                     svg_id = "#PLANE"
                                     )
      self.tuxplane.translate(50 * level, 40 )

      # Animated Tux plane.Move from left to right
      self.planeroot.animate(900,
                             20 ,
                             1,
                             1,
                             True,
                             25000 + level * 9000,
                             100,
                             goocanvas.ANIMATE_FREEZE)

      string = "abcdefghijklmnopqrstuvwxyz"
      for index in range (level):
      	   # Select a random letter and append it to self.letter_array
           if (level == 1):
           	letter = string[self.gcomprisBoard.sublevel - 1]
           elif(level == 2):
           	letter = random.choice(string[random.randint(0, 9)])
           elif(level == 3):
           	letter = random.choice(string[random.randint(10,19)])
           elif(level == 4):
           	letter = random.choice(string[random.randint(20, 25)])
           else :
           	letter = random.choice(string[random.randint(0,25)])

           self.play_letter(letter)
           self.letter_array.append( letter.upper() )

           # Defining Object to BrailleChar Instance to produce braille_tile
           self.tile_array.append( \
               BrailleChar(self.root,
                           300 + 90 * index , 150, 80,
                           '', 0xFF0000FFL, 0xFFFFFFFFL,
                           "white", "black",
                           True, True, True, callback = self.letter_change) )

      # Display alphabets for TUX_PLANE horizontally and vertically
      animateString = "".join(self.letter_array)
      for index in range( len(animateString) ):
          goocanvas.Text(
            parent = self.planeroot,
            x =  50 * index  ,
            y = 60.0 ,
            text = animateString[index],
            fill_color="black",
            anchor = gtk.ANCHOR_CENTER,
            alignment = pango.ALIGN_CENTER,
            font = 'SANS 50'
            )
          self.alphabet_vertical = goocanvas.Text(
                         parent = self.verticalTextRoot,
                         x = 50.0 + 50 * index ,
                         y = 130.0 ,
                         text = animateString[index],
                         fill_color = "black",
                         anchor = gtk.ANCHOR_CENTER,
                         alignment = pango.ALIGN_CENTER,
                         font = 'SANS 50'
                         )
          self.alphabet_array.append(self.alphabet_vertical)

      # Animate or move letters vertically
      self.verticalTextRoot.animate( 5,
                                     415,
                                     1,
                                     1,
                                     True,
                                     30000 + level * 5000,
                                     100,
                                     goocanvas.ANIMATE_FREEZE )

      # To call a function when animation finishes
      self.verticalTextRoot.connect("animation-finished", self.animationFinished)


  def letter_change(self, letter):
      self.letter = letter
      for index in range(self.gcomprisBoard.level):
          # Change the text color of alphabet correctly identified in the braille tile
          if (self.tile_array[index].get_letter() == self.letter_array[index]):

              # The user just found this letter, let's play it to give a
              # success feedback
              if self.alphabet_array[index].get_property("fill_color_rgba") != self.text_color:
                self.play_letter(self.letter_array[index])

              self.alphabet_array[index].set_property("fill_color_rgba",
                                                      self.text_color)

          # If an alphabet's color changes to blue the counter is increased
          # and the index number of tile is appended to a self.tile_index_array
          if self.alphabet_array[index].get_property("fill_color_rgba") == self.text_color \
              and index not in self.tile_index_array :
              self.tile_index_array.append(index)
              self.counter +=1

      # If counter equals level number then call a timer_loop
      if (self.counter == self.gcomprisBoard.level) :
          self.timerAnim = gobject.timeout_add(1000, self.timer_loop)

  # This timer loop is to wait for milliseconds before calling a Bonus API
  def timer_loop(self):
    self.gamewon = True
    gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.SMILEY)

  def animationFinished(self, item, status):
      if self.reseted:
        return
      print "animationFinished"
      if (self.counter != self.gcomprisBoard.level) :
          self.gamewon = False
          gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.SMILEY)

  def end(self):
    self.reseted = True
    # Remove the root item removes all the others inside it
    self.root.remove()
    self.map_rootitem.remove()
    self.horizontalTextRoot.remove()
    self.verticalTextRoot.remove()
    gcompris.score.end()

  def ok(self):
    pass

  def repeat(self):
      if(self.mapActive):
          self.map_rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
          self.mapActive = False
      else :
          self.map_rootitem.props.visibility = goocanvas.ITEM_VISIBLE
          self.mapActive = True


  def config(self):
    pass

  def key_press(self, keyval, commit_str, preedit_str):
    pass

  def pause(self, pause):
    self.board_paused = pause
    # This is to hide the Repeat board
    if self.mapActive == True:
        self.root.props.visibility = goocanvas.ITEM_INVISIBLE

    if (self.board_paused):
        self.verticalTextRoot.stop_animation()
        self.planeroot.stop_animation()
        return

    self.reset_level()
    if self.gamewon:
      self.increment_level()
    self.gamewon = False
    self.end()
    self.start()

  def reset_level(self):
    self.letter_array = []
    self.tile_array = []
    self.alphabet_array = []
    self.tile_index_array = []
    self.counter = 0

  def increment_level(self):
    self.reset_level()
    self.gcomprisBoard.sublevel += 1
    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
        self.gcomprisBoard.sublevel=1
        self.gcomprisBoard.level += 1
        if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
            self.gcomprisBoard.level = 1

  def set_level(self, level):
    #This is to hide the Repeat board in case set_level button gets pressed
    if self.mapActive == True:
        self.root.props.visibility = goocanvas.ITEM_INVISIBLE
    self.reset_level()
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()

  def play_letter(self, letter):
    # Play the letter
    filename = 'voices/$LOCALE/alphabet/U%04X.ogg' % ord(letter.lower())
    gcompris.sound.play_ogg(filename)


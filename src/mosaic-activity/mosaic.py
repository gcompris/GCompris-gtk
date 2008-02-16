#  gcompris - mosaic
#
# Copyright (C) 2005 Bruno Coudoin / Clara Coudoin
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

import goocanvas
import cairo
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gcompris.bonus
import gcompris.sound
import gtk
import gtk.gdk
import random
import pango

from gcompris import gcompris_gettext as _

class Gcompris_mosaic:
  """Tux hide a number, you must guess it"""


  def __init__(self, gcomprisBoard):

    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.disable_im_context = True

    # A text canvas item use to indicate it's over or lower
    self.indicator = None
    self.indicator_s = None

    # The current selected color
    self.current_index = None

    # The min and max value that must be found
    self.min = -1
    self.max = -1

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    self.orig_x = 35
    self.orig_y = 50
    self.target_x = gcompris.BOARD_WIDTH/2 + 17
    self.target_y = 50

    self.palette_x = 40
    self.palette_y = 330

    # In the palette png, the size of each image
    self.palette_item_width  = 50
    self.palette_item_height = 50

    # In the palette png, the number of image in x and y
    self.palette_number_of_item_x = 8
    self.palette_number_of_item_y = 3

    # Index of the grey item in the palette
    self.palette_grey_item_x = 0
    self.palette_grey_item_y = 3

    # What the user have to find
    self.target_list = []

    # The user answer
    self.user_list   = []

  def start(self):

    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=6
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(gcompris.BAR_LEVEL)

    gcompris.bar_set_level(self.gcomprisBoard)

    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "mosaic/mosaic_bg.png")

    self.display_game()


  def end(self):

    gcompris.reset_locale()

    # Remove the root item removes all the others inside it
    self.cleanup_game()

  def ok(self):
    pass

  def repeat(self):
    print("Gcompris_mosaic repeat.")


  def key_press(self, keyval, commit_str, preedit_str):
    # Return  True  if you did process a key
    # Return  False if you did not processed a key
    #         (gtk need to send it to next widget)
    return False

  def pause(self, pause):
    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(pause == 0 and self.gamewon):
      self.increment_level()
      self.gamewon = 0
      self.cleanup_game()
      self.display_game()

    return


  def set_level(self, level):
    self.gcomprisBoard.level=level;
    self.gcomprisBoard.sublevel=1;

    # Set the level in the control bar
    gcompris.bar_set_level(self.gcomprisBoard);

    self.cleanup_game()
    self.display_game()

  #
  # End of Initialisation
  # ---------------------
  #

  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return 1 if continue, 0 if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      gcompris.bar_set_level(self.gcomprisBoard)

      if(self.gcomprisBoard.level>self.gcomprisBoard.maxlevel):
        # the current board is finished : bail out
        gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
        return 0

    return 1

  # Display the board game
  def cleanup_game(self):

      self.current_index = None
      # Remove the root item removes all the others inside it
      self.rootitem.remove()

  # Display the board game
  def display_game(self):

      # What the user have to find
      self.target_list = []
      # The user answer
      self.user_list   = []

      # Create our rootitem. We put each canvas item in it so at the end we
      # only have to kill it. The canvas deletes all the items it contains automaticaly.
      self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

      text = _("Rebuild the same mosaic on the right area")
      goocanvas.Text(
        parent = self.rootitem,
        x=gcompris.BOARD_WIDTH/2,
        y=gcompris.BOARD_HEIGHT - 30,
        font=gcompris.skin.get_font("gcompris/title"),
        text=(text),
        fill_color="white",
        anchor=gtk.ANCHOR_CENTER
        )

      self.number_item_x = 4
      self.number_item_y = 2
      self.number_max_of_color_x = 8
      self.number_max_of_color_y = 1
      if(self.gcomprisBoard.level == 2):
        self.number_item_x = 6
        self.number_item_y = 2
        self.number_max_of_color_y = 1
      elif(self.gcomprisBoard.level == 3):
        self.number_item_x = 6
        self.number_item_y = 3
        self.number_max_of_color_y = 2
      elif(self.gcomprisBoard.level == 4):
        self.number_item_x = 6
        self.number_item_y = 3
        self.number_max_of_color_y = 3
      elif(self.gcomprisBoard.level == 5):
        self.number_item_x = 6
        self.number_item_y = 4
        self.number_max_of_color_y = 3
      elif(self.gcomprisBoard.level == 6):
        self.number_item_x = 6
        self.number_item_y = 4
        self.number_max_of_color_y = 3


      self.current_index_x = -1
      self.current_index_y = -1

      palette = gcompris.utils.load_pixmap("mosaic/mosaic_palette.png")

      self.display_mosaic(self.orig_x, self.orig_y, True, palette)

      self.display_mosaic(self.target_x, self.target_y, False, palette)

      self.display_palette(self.palette_x, self.palette_y, palette)

      # Create the check button to show the selected color
      self.checked_color_item = goocanvas.Image(
        parent = self.rootitem,
        pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_checked.png")) )

      self.checked_color_item.props.visibility = goocanvas.ITEM_INVISIBLE

  #
  # Display the mosaic itself (colored or not)
  #
  def display_mosaic(self, orig_x, orig_y, colored, palette):

      gap_x = 10
      gap_y = 10

      i = 0
      for y in range(0, self.number_item_y):
        for x in range(0, self.number_item_x):

              # Select an image in the palette
              color_index_x = random.randint(0, (self.number_max_of_color_x-1))
              color_index_y = random.randint(0, (self.number_max_of_color_y-1))

              if colored:
                image  = palette.subpixbuf(color_index_x * self.palette_item_width,
                                           color_index_y * self.palette_item_height,
                                           self.palette_item_width, self.palette_item_height)
              else:
                image  = palette.subpixbuf(self.palette_grey_item_x * self.palette_item_width,
                                           self.palette_grey_item_y * self.palette_item_height,
                                           self.palette_item_width, self.palette_item_height)

              item = goocanvas.Image(
                parent = self.rootitem,
                pixbuf = image,
                x = orig_x + x * (self.palette_item_width  + gap_x),
                y = orig_y + y * (self.palette_item_height + gap_y))

              if not colored:
                item.connect("button_press_event",
                             self.set_focus_item_event, (i, palette))
                self.user_list.append((-1, -1))
              else:
                self.target_list.append((color_index_x, color_index_y))

              i += 1



  #
  # Display the color palette
  #
  def display_palette(self, orig_x, orig_y, palette):

      gap_x = 10
      gap_y = 10

      number_item_x = 12
      number_item_y = 2

      for i in range(0, self.number_max_of_color_x * self.number_max_of_color_y):

            x         = i % number_item_x
            y         = i / number_item_x

            palette_x = i % self.palette_number_of_item_x
            palette_y = i / self.palette_number_of_item_x

            image  = palette.subpixbuf(palette_x * self.palette_item_width,
                                       palette_y * self.palette_item_height,
                                       self.palette_item_width, self.palette_item_height)

            coord_x = orig_x + x * (self.palette_item_width  + gap_x)
            coord_y = orig_y + y * (self.palette_item_height + gap_y)
            item = goocanvas.Image(
              parent = self.rootitem,
              pixbuf = image,
              x = coord_x,
              y = coord_y)

            gcompris.utils.item_focus_init(item, None)
            item.connect("button_press_event",
                         self.set_color_item_event, (palette_x, palette_y,
                                                     coord_x, coord_y))


  # Event when a target square is selected
  def set_focus_item_event(self, item, target, event, data):
    (index, palette)  = data

    if event.type == gtk.gdk.BUTTON_PRESS:
      # A color is selected
      gcompris.sound.play_ogg("sounds/bleep.wav");
      if(self.current_index_x >= 0):

        image = palette.subpixbuf(self.current_index_x * self.palette_item_width,
                                  self.current_index_y * self.palette_item_height,
                                  self.palette_item_width, self.palette_item_height)

        item.props.pixbuf = image

        self.user_list[index] = (self.current_index_x,
                                 self.current_index_y)

        if(self.target_list == self.user_list):
          self.gamewon = True
          gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.GNU)



  # Event when a color square is selected
  def set_color_item_event(self, item, target, event, data):

    gcompris.sound.play_ogg("sounds/paint1.wav");
    (color_index_x, color_index_y, coord_x, coord_y) = data

    self.current_index_x = color_index_x
    self.current_index_y = color_index_y

    index = color_index_y * self.palette_number_of_item_x + color_index_x

    self.checked_color_item.set_transform(cairo.Matrix(1, 0, 0, 1, 5, -5))
    self.checked_color_item.translate(coord_x,
                                      coord_y + self.palette_item_height/3)
    self.checked_color_item.props.visibility = goocanvas.ITEM_VISIBLE

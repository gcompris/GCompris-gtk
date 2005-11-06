#  gcompris - mosaic
# 
# Copyright (C) 2005 Bruno Coudoin / Clara Coudoin
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

import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gcompris.bonus
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
    self.previous_color_item = None
    
    # The min and max value that must be found
    self.min = -1
    self.max = -1
    
    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    self.orig_x = 40
    self.orig_y = 50
    self.target_x = gcompris.BOARD_WIDTH/2 + 40
    self.target_y = 50

    self.palette_x = 40
    self.palette_y = 330
    
    self.colors = [
        0x878686FF,
        0xc08b8bFF,
        0xe57979FF,
        0xfa4242FF,
        0xeb09ffFF,
        0xdaa7d6FF,
        0x1c0cf7FF,
        0xc6c4eaFF,
        0x0ffdefFF,
        0xb3e4e5FF,
        0x34fd42FF,
        0xa5e0afFF,
        0xe8fd05FF,
        0xe3e086FF,
        0x878686FF,
        0xc08b8bFF,
        0xe57979FF,
        0xfa4242FF,
        0xeb09ffFF,
        0xdaa7d6FF,
        0x1c0cf7FF,
        0xc6c4eaFF,
        0x0ffdefFF,
        0xb3e4e5FF,
        0x34fd42FF,
        0xa5e0afFF,
        0xe8fd05FF,
        0xe3e086FF,
        ]

    self.alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    # What the user have to find
    self.target_list = []

    # The user answer
    self.user_list   = []
    
  def start(self):
    
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=5
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(gcompris.BAR_OK|gcompris.BAR_LEVEL)
    
    gcompris.bar_set_level(self.gcomprisBoard)

    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            "images/mosaic_bg.png")

    self.display_game()
    

  def end(self):

    gcompris.reset_locale()
    
    # Remove the root item removes all the others inside it
    self.cleanup_game()

  def ok(self):
    if(self.target_list == self.user_list):
      self.gamewon = True
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.GNU)
    else:
      self.gamewon = False
      gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.GNU)

  def repeat(self):
    print("Gcompris_mosaic repeat.")


  def config(self):
    print("Gcompris_mosaic config.")


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
    
  ###################################################
  # Configuration system
  ###################################################
  
  #mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    pass

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
      self.previous_color_item = None
      # Remove the root item removes all the others inside it
      self.rootitem.destroy()

  # Display the board game
  def display_game(self):

      # What the user have to find
      self.target_list = []
      # The user answer
      self.user_list   = []
      
      # Create our rootitem. We put each canvas item in it so at the end we
      # only have to kill it. The canvas deletes all the items it contains automaticaly.
      self.rootitem = self.gcomprisBoard.canvas.root().add(
          gnome.canvas.CanvasGroup,
          x=0.0,
          y=0.0
          )

      text = _("Rebuild the same mosaic on the right area")
      self.rootitem.add(
          gnome.canvas.CanvasText,
          x=gcompris.BOARD_WIDTH/2 + 1.0,
          y=gcompris.BOARD_HEIGHT - 50 + 1.0,
          font=gcompris.skin.get_font("gcompris/title"),
          text=(text),
          fill_color="black",
          justification=gtk.JUSTIFY_CENTER
          )
      self.rootitem.add(
          gnome.canvas.CanvasText,
          x=gcompris.BOARD_WIDTH/2,
          y=gcompris.BOARD_HEIGHT - 50,
          font=gcompris.skin.get_font("gcompris/title"),
          text=(text),
          fill_color="white",
          justification=gtk.JUSTIFY_CENTER
          )



      self.number_item_x = 4
      self.number_item_y = 2
      self.number_max_of_colors = 8
      if(self.gcomprisBoard.level == 2):
          self.number_item_x = 4
          self.number_item_y = 3
          self.number_max_of_colors = 12
      elif(self.gcomprisBoard.level == 3):
          self.number_item_x = 5
          self.number_item_y = 3
          self.number_max_of_colors = 15
      elif(self.gcomprisBoard.level == 4):
          self.number_item_x = 5
          self.number_item_y = 4
          self.number_max_of_colors = 28
      elif(self.gcomprisBoard.level == 5):
          self.number_item_x = 6
          self.number_item_y = 4
          self.number_max_of_colors = 28


      self.display_mosaic(self.orig_x, self.orig_y, True)

      self.display_mosaic(self.target_x, self.target_y, False)

      self.display_palette(self.palette_x, self.palette_y)

  #
  # Display the mosaic itself (colored or not)
  #
  def display_mosaic(self, orig_x, orig_y, colored):
          
      gap_x = 10
      gap_y = 10
      box_size_x = 40
      box_size_y = 40

      i = 0
      for x in range(0, self.number_item_x):
          for y in range(0, self.number_item_y):
              color_index = random.randint(0, (self.number_max_of_colors)-1)
              if colored:
                fill_color  = self.colors[color_index]
                fill_letter = self.alphabet[color_index]
              else:
                  fill_color  = 0xF0F0F0FF
                  fill_letter = " "
                  
              item = self.rootitem.add(
                  gnome.canvas.CanvasRect,
                  x1 = orig_x + x * (box_size_x + gap_x),
                  y1 = orig_y + y * (box_size_y + gap_y),
                  x2 = orig_x + x * (box_size_x + gap_x) + box_size_x,
                  y2 = orig_y + y * (box_size_y + gap_y) + box_size_y,
                  fill_color_rgba=fill_color,
                  outline_color_rgba=0x000000FF,
                  width_units=0.0
                  )

              item2 = self.rootitem.add(
                  gnome.canvas.CanvasText,
                  x = orig_x + x * (box_size_x + gap_x) + box_size_x/2 + 0.9,
                  y = orig_y + y * (box_size_y + gap_y) + box_size_y/2 + 0.9,
                  text = fill_letter,
                  fill_color="black",
                  )

              item3 = self.rootitem.add(
                  gnome.canvas.CanvasText,
                  x = orig_x + x * (box_size_x + gap_x) + box_size_x/2,
                  y = orig_y + y * (box_size_y + gap_y) + box_size_y/2,
                  text = fill_letter,
                  fill_color="white",
                  )
              if not colored:
                item.connect("event", self.set_focus_item_event,  (i, item, item2, item3))
                item2.connect("event", self.set_focus_item_event, (i, item, item2, item3))
                item3.connect("event", self.set_focus_item_event, (i, item, item2, item3))
                self.user_list.append(-1)
              else:
                self.target_list.append(color_index)

              i += 1
              


  #
  # Display the color palette
  #
  def display_palette(self, orig_x, orig_y):
      
      gap_x = 10
      gap_y = 10
      box_size_x = 40
      box_size_y = 40

      x = 0
      y = 0
      for i in range(0, self.number_max_of_colors):
            fill_color  = self.colors[i]
            fill_letter = self.alphabet[i]

            item = self.rootitem.add(
              gnome.canvas.CanvasRect,
              x1 = orig_x + x * (box_size_x + gap_x),
              y1 = orig_y + y * (box_size_y + gap_y),
              x2 = orig_x + x * (box_size_x + gap_x) + box_size_x,
              y2 = orig_y + y * (box_size_y + gap_y) + box_size_y,
              fill_color_rgba=fill_color,
              outline_color_rgba=0x000000FF,
              width_units=0.0
              )
            item.connect("event", self.set_color_item_event, (item, i))

            item2 = self.rootitem.add(
              gnome.canvas.CanvasText,
              x = orig_x + x * (box_size_x + gap_x) + box_size_x/2 + 0.9,
              y = orig_y + y * (box_size_y + gap_y) + box_size_y/2 + 0.9,
              text = fill_letter,
              fill_color="black",
              )
            item2.connect("event", self.set_color_item_event, (item, i))

            item2 = self.rootitem.add(
              gnome.canvas.CanvasText,
              x = orig_x + x * (box_size_x + gap_x) + box_size_x/2,
              y = orig_y + y * (box_size_y + gap_y) + box_size_y/2,
              text = fill_letter,
              fill_color="white",
              )
            item2.connect("event", self.set_color_item_event, (item, i))

            # Move position
            x += 1
            if(x>13):
              x = 0
              y += 1
              
  # Event when a target square is selected
  def set_focus_item_event(self, item, event, data):
    box_index  = data[0]
    box_item   = data[1]
    text1_item = data[2]
    text2_item = data[3]
    
    if event.type == gtk.gdk.BUTTON_PRESS:
      if(self.current_index >= 0):
        box_item.set(fill_color_rgba = self.colors[self.current_index])
        text1_item.set(text = self.alphabet[self.current_index])
        text2_item.set(text = self.alphabet[self.current_index])
        self.user_list[box_index] = self.current_index

  # Event when a color square is selected
  def set_color_item_event(self, item, event, data):
    box_item = data[0]
    color_index = data[1]
    
    if event.type == gtk.gdk.BUTTON_PRESS:
      self.current_index = color_index

      if self.previous_color_item:
        self.previous_color_item.set(width_units = 1.0)

      box_item.set(width_units = 5.0)
      self.previous_color_item = box_item

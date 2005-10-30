#  gcompris - mosaic
# 
# Copyright (C) 2005 Bruno Coudoin / Clement Coudoin
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
    self.current_color = None
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

    self.palette_x = 240
    self.palette_y = 300
    
    self.colors = [
        0x759aaaFF,
        0xd05a4dFF,
        0x50a45cFF,
        0xd87369FF,
        0xb7b2bfFF,
        0xebe539FF,
        0xf7f495FF,
        0x88ad80FF,
        0x6e34c5FF,
        0x663535FF,
        0xa09292FF,
        0x206423FF,
        0xff0808FF,
        0xc06969FF,
        0x6814e7FF,
        0x1f9acdFF,
        0x716e1fFF,
        0xfbf54eFF
        ]
    
  def start(self):
    
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=4
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(gcompris.BAR_OK|gcompris.BAR_LEVEL)
    
    gcompris.bar_set_level(self.gcomprisBoard)

    #compris.set_background(self.gcomprisBoard.canvas.root(), "images/cave.png")

    self.display_game()
    

  def end(self):

    gcompris.reset_locale()
    
    # Remove the root item removes all the others inside it
    self.cleanup_game()

  def ok(self):
    print("Gcompris_mosaic ok.")


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
    if(pause == 0):
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
      
      self.current_color = None
      self.previous_color_item = None
      # Remove the root item removes all the others inside it
      self.rootitem.destroy()

  # Display the board game
  def display_game(self):

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
      if(self.gcomprisBoard.level == 2):
          self.number_item_x = 5
          self.number_item_y = 3
      elif(self.gcomprisBoard.level == 3):
          self.number_item_x = 5
          self.number_item_y = 4
      elif(self.gcomprisBoard.level == 4):
          self.number_item_x = 5
          self.number_item_y = 5


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
      
      for x in range(0, self.number_item_x):
          for y in range(0, self.number_item_y):
              if colored:
                  fill_color = self.colors[random.randint(0, len(self.colors)-1)]
              else:
                  fill_color = 0xF0F0F0FF
                  
              print "x=%d y=%d" %(x, y)
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
              if not colored:
                  item.connect("event", self.set_focus_item_event)


  #
  # Display the color palette
  #
  def display_palette(self, orig_x, orig_y):
      
      gap_x = 10
      gap_y = 10
      box_size_x = 40
      box_size_y = 40

      i = 0
      for x in range(0, 5):
          for y in range(0, 3):
              fill_color = self.colors[i]
              i += 1
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
              item.connect("event", self.set_color_item_event, fill_color)
                  
  # Event when a target square is selected
  def set_focus_item_event(self, item, event):

      if event.type == gtk.gdk.BUTTON_PRESS:
          if(self.current_color):
              item.set(fill_color_rgba = self.current_color)

  # Event when a color square is selected
  def set_color_item_event(self, item, event, color):

      if event.type == gtk.gdk.BUTTON_PRESS:
          self.current_color = color
          
          if self.previous_color_item:
              self.previous_color_item.set(width_units = 1.0)
              
          item.set(width_units = 5.0)
          self.previous_color_item = item

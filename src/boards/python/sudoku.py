#  gcompris - sudoku
# 
# Copyright (C) 2005 Bruno Coudoin
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
import gcompris.bonus
import gtk
import gtk.gdk
import random
from gettext import gettext as _

class Gcompris_sudoku:
  """Sudoku game"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;

    # It holds the canvas items for each square
    self.sudo_square = []       # The square Rect Item
    self.sudo_number = []       # The square Text Item

    self.valid_chars = []       # The valid chars for the sudoku are calculated from the dataset

    # Holds the coordinate of the current square
    self.cursqre = None

    self.normal_square_color = 0xbebbc9ffL
    self.focus_square_color  = 0x8b83a7ffL
    self.error_square_color  = 0xff77a7ffL
    self.lines_color         = 0xebe745ffL
    
    self.fixed_number_color  = 0xff2100ffL
    self.user_number_color   = 0x000bffffL

    self.root_sudo = None

    self.sudoku = None          # The current sudoku data
    self.sudo_size = 0          # the size of the current sudoku
    self.sudo_region = None     # the modulo region in the current sudoku


  def start(self):

    # Init the sudoku dataset
    self.sudoku = self.init_item_list()

    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=len(self.sudoku)
    self.gcomprisBoard.sublevel=1
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.next_level()
    
    self.pause(0);



  def end(self):

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()
    self.rootitem = None

  def ok(self):
    print("Gcompris_sudoku ok.")


  def repeat(self):
    print("Gcompris_sudoku repeat.")


  def config(self):
    print("Gcompris_sudoku config.")


  def key_press(self, keyval):

    if(self.cursqre == None):
      return False

    if (keyval == gtk.keysyms.KP_0):
      keyval= gtk.keysyms._0
    if (keyval == gtk.keysyms.KP_1):
      keyval= gtk.keysyms._1
    if (keyval == gtk.keysyms.KP_2):
      keyval= gtk.keysyms._2
    if (keyval == gtk.keysyms.KP_2):
      keyval= gtk.keysyms._2
    if (keyval == gtk.keysyms.KP_3):
      keyval= gtk.keysyms._3
    if (keyval == gtk.keysyms.KP_4):
      keyval= gtk.keysyms._4
    if (keyval == gtk.keysyms.KP_5):
      keyval= gtk.keysyms._5
    if (keyval == gtk.keysyms.KP_6):
      keyval= gtk.keysyms._6
    if (keyval == gtk.keysyms.KP_7):
      keyval= gtk.keysyms._7
    if (keyval == gtk.keysyms.KP_8):
      keyval= gtk.keysyms._8
    if (keyval == gtk.keysyms.KP_9):
      keyval= gtk.keysyms._9

    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    if(strn in self.valid_chars):

      if self.is_possible(strn):
        self.sudo_number[self.cursqre[0]][self.cursqre[1]].set(
          text = strn.encode('UTF-8'),
          )

        # Maybe it's all done
        if self.is_solved():
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)

    else:
      # Erase the old number there
      if ((keyval == gtk.keysyms.BackSpace) or
          (keyval == gtk.keysyms.Delete) or
          (keyval == gtk.keysyms.space)):
        self.sudo_number[self.cursqre[0]][self.cursqre[1]].set(
          text = "",
          )

      else:
        # No key processing done
        return False
      
    # Return  True  if you did process a key
    # Return  False if you did not processed a key
    #         (gtk need to send it to next widget)
    return True

  def pause(self, pause):

    self.board_paused = pause
    
    # When the bonus is displayed, it call us first
    # with pause(1) and then with pause(0)
    # the game is won
    if(self.gamewon == 1 and pause == 0):
      self.gamewon = 0
      if(self.increment_level()):
        self.next_level()

    return
                  


  def set_level(self, level):
    self.gcomprisBoard.level = level;
    self.gcomprisBoard.sublevel = 1;
    gcompris.bar_set_level(self.gcomprisBoard)
    self.next_level()

# ---- End of Initialisation

  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return True if continue, False if bail out
  def next_level(self):

    self.display_sudoku(self.sudoku[self.gcomprisBoard.level-1][self.gcomprisBoard.sublevel-1])
    return True

  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return True if continue, False if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel > len(self.sudoku[self.gcomprisBoard.level-1])):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      if(self.gcomprisBoard.level>self.gcomprisBoard.maxlevel):
        # the last board is finished : bail out
        gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
        return False
      
    return True
        

  #
  # This function is being called uppon a click on any little square
  #
  def square_item_event(self, widget, event, square):
    
    if (event and event.type == gtk.gdk.BUTTON_PRESS):
      if event.button == 1:
        # Check it's a user editable square
        oldcolor = self.sudo_number[square[0]][square[1]].get_property('fill_color_rgba')
        if oldcolor == self.fixed_number_color:
          return

        if(self.cursqre):
          self.sudo_square[self.cursqre[0]][self.cursqre[1]].set(
            fill_color_rgba = self.normal_square_color,            
            )
        self.cursqre = square
        self.sudo_square[square[0]][square[1]].set(
          fill_color_rgba = self.focus_square_color,            
          )
        return True

    return False

  # Highlight the given item to show it's the one that causes the resufal
  # to pose a number
  def set_on_error(self, items):
    for item in items:
      item.set(
        fill_color_rgba= self.error_square_color
        )
      
    gtk.timeout_add(3000, self.unset_on_error, items)
    
  def unset_on_error(self, items):
    if(self.rootitem):
      for item in items:
        item.set(
          fill_color_rgba= self.normal_square_color
          )
    
  
  # Return True or False if the given number is possible
  #
  def is_possible(self, number):

    possible = True
    bad_square = []
    
    if(self.cursqre == None):
      return possible

    # Check this number is not already in a row
    for x in range(0,self.sudo_size):

      if x == self.cursqre[0]:
         continue

      item = self.sudo_number[x][self.cursqre[1]]
      othernumber = item.get_property('text').decode('UTF-8')

      if(number == othernumber):
        bad_square.append(self.sudo_square[x][self.cursqre[1]])
        possible = False

    # Check this number is not already in a column
    for y in range(0,self.sudo_size):

      if y == self.cursqre[1]:
        continue
       
      item = self.sudo_number[self.cursqre[0]][y]
      othernumber = item.get_property('text').decode('UTF-8')

      if(number == othernumber):
        bad_square.append(self.sudo_square[self.cursqre[0]][y])
        possible = False

    #
    # Check this number is in a region
    #

    # Region is the modulo place to set region if defined
    region = None
    if(self.sudo_region.has_key(self.sudo_size)):
      region=self.sudo_region[self.sudo_size]

    if(region):
      # First, find the top-left square of the region
      top_left=[self.cursqre[0]/region*region, self.cursqre[1]/region*region]
      for x in range(0,region):
        for y in range(0,region):
          # Do not check the current square
          if (top_left[0] + x == self.cursqre[0] and
          top_left[1] + y == self.cursqre[1]):
            continue

          item = self.sudo_number[top_left[0] + x][top_left[1] + y]
          othernumber = item.get_property('text').decode('UTF-8')

          if(number == othernumber):
            bad_square.append(self.sudo_square[top_left[0] + x][top_left[1] + y])
            possible = False
        
    if not possible:
      self.set_on_error(bad_square)

    return possible
 
  # Return True or False if the given sudoku is solved
  # We don't really check it's solved, only that all squares
  # have a value. This works because only valid numbers can
  # be entered up front.
  #
  def is_solved(self):

    for x in range(0,self.sudo_size):
      for y in range(0,self.sudo_size):
        item = self.sudo_number[x][y]
        number = item.get_property('text').decode('UTF-8')
        if(number == ""):
          return False
        
    return True

  #
  # Display the board area
  #
  def display_board(self, sudo_size):

    if(self.root_sudo):
      self.root_sudo.destroy()
      
    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.root_sudo = self.rootitem.add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )


    # Region is the modulo place to set region if defined
    region = None
    if(self.sudo_region.has_key(sudo_size)):
      region=self.sudo_region[sudo_size]
      
    square_width = (gcompris.BOARD_HEIGHT-50)/sudo_size
    square_height = square_width
    x_init = (gcompris.BOARD_WIDTH - square_width*sudo_size)/2
    y_init = (gcompris.BOARD_HEIGHT - square_height*sudo_size)/2
    
    for x in range(0,sudo_size):
      line_square = []
      line_number = []

      for y in range(0,sudo_size):

        item = self.root_sudo.add(
          gnome.canvas.CanvasRect,
          fill_color_rgba = self.normal_square_color,
          x1= x_init + square_width * x,
          y1= y_init + square_height * y,
          x2= x_init + square_width * (x+1),
          y2= y_init + square_height * (y+1),
          width_units=1.0,
          outline_color_rgba= 0x144B9DFFL
          )
        line_square.append(item)
        item.connect("event", self.square_item_event, (x,y))

        item = self.root_sudo.add(
          gnome.canvas.CanvasText,
          x= x_init + square_width * x + square_width/2,
          y= y_init + square_height * y + square_height/2,
          text= "",
          font=gcompris.skin.get_font("gcompris/content"),
          )
        line_number.append(item)

        if(region):
          if(y>0 and y%region==0):
            self.root_sudo.add (
              gnome.canvas.CanvasLine,
              points=(x_init - 5,
                      y_init + square_height * y,
                      x_init + (square_width * sudo_size) + 5,
                      y_init + square_height * y),
              fill_color_rgba=self.lines_color,
              width_units=2.5,
              )

      self.sudo_square.append(line_square)
      self.sudo_number.append(line_number)

      if(region):
        if(x>0 and x%region==0):
          self.root_sudo.add (
            gnome.canvas.CanvasLine,
            points=(x_init + square_width * x,
                    y_init - 5,
                    x_init + square_width * x,
                    y_init + square_height * sudo_size + 5),
            fill_color_rgba=self.lines_color,
            width_units=2.5,
            )

  #
  # Display valid number (or chars)
  #
  def display_valid_chars(self, sudo_size, valid_chars):
    
    square_width = (gcompris.BOARD_HEIGHT-50)/sudo_size
    square_height = square_width
    
    x_init = 20
    y_init = (gcompris.BOARD_HEIGHT - square_height*sudo_size)/2
    
    for y in range(0,sudo_size):
      # Shadow
      self.root_sudo.add(
        gnome.canvas.CanvasText,
        x= x_init+1.5,
        y= y_init + square_height * y + square_height/2 + 1.5,
        text= valid_chars[y],
        font=gcompris.skin.get_font("gcompris/title"),
        fill_color="black"
        )
      # Text
      self.root_sudo.add(
        gnome.canvas.CanvasText,
        x= x_init,
        y= y_init + square_height * y + square_height/2,
        text= valid_chars[y],
        font=gcompris.skin.get_font("gcompris/title"),
        fill_color="white"
        )


  #
  # Display the given sudoku
  #
  def display_sudoku(self, sudoku):

    # Reinit the sudoku globals
    self.sudo_square = []       # The square Rect Item
    self.sudo_number = []       # The square Text Item

    self.valid_chars = []       # The valid chars for the sudoku are calculated from the dataset

    self.sudo_size = len(sudoku[0])

    self.display_board(self.sudo_size)

    for x in range(0,self.sudo_size):
      for y in range(0,self.sudo_size):
        text  = sudoku[y][x]
        color = self.fixed_number_color
        if text == '.':
          text  = ""
          color = self.user_number_color
        else:
          if(not text in self.valid_chars):
            self.valid_chars.append(text)
          
        self.sudo_number[x][y].set(
          text= text,
          fill_color_rgba= color,
          )

    self.valid_chars.sort()
    self.display_valid_chars(self.sudo_size, self.valid_chars)
    
  # return the list of items (data) for this game
  def init_item_list(self):

    # It's hard coded that sudoku of size x have y region.
    # If not defined there, it means no region
    #
    # Sudoku size / Number of region
    #
    self.sudo_region = {
      9: 3
      }

    # It's a list of level which is a list of sudoku data
    sudoku_list = \
      [
       [ # Level 1
        [
         ['A','C','.'],
         ['.','B','A'],
         ['B','.','.']
        ],
        [
         ['C','A','.'],
         ['.','.','C'],
         ['B','.','.']
        ],
       ],
       [ # Level 2
        [
         ['A','B','C','D','E'],
         ['.','A','B','C','D'],
         ['.','.','A','B','C'],
         ['.','.','.','A','B'],
         ['.','.','.','.','A']
        ],
       ],
       [ # Level 3
        [
         ['3','.','.','.','.','5','6','.','2'],
         ['.','6','2','7','1','.','.','4','.'],
         ['1','.','.','9','.','.','.','5','.'],
         ['8','.','.','.','.','.','2','6','.'],
         ['.','3','.','.','8','.','.','7','.'],
         ['.','9','7','.','.','.','.','.','1'],
         ['.','5','.','.','.','.','.','.','6'],
         ['.','8','.','.','6','7','4','2','.'],
         ['6','.','3','1','.','.','.','.','8']
        ],
        [
         ['9','3','1','.','.','7','.','2','8'],
         ['.','.','.','.','3','6','.','.','.'],
         ['4','.','6','.','.','2','.','.','.'],
         ['6','2','.','.','.','.','9','1','.'],
         ['1','.','.','.','2','.','.','.','4'],
         ['.','4','8','.','.','.','.','6','5'],
         ['.','.','.','9','.','.','3','.','2'],
         ['.','.','.','2','7','.','.','.','.'],
         ['3','7','.','4','.','.','5','9','1']
        ],
        [
         ['.','.','9','.','6','.','.','.','.'],
         ['2','.','6','4','.','.','1','.','.'],
         ['.','.','3','1','8','.','5','.','.'],
         ['.','.','2','.','.','.','.','.','1'],
         ['7','9','.','.','.','.','.','6','8'],
         ['1','.','.','.','.','.','3','.','.'],
         ['.','.','7','.','9','2','8','.','.'],
         ['.','.','1','.','.','8','6','.','5'],
         ['.','.','.','.','3','.','4','.','.']
         ]
       ]
      ]


    # Randomize each level in the list
    for l in range(0, len(sudoku_list)):
      for j in range(0, len(sudoku_list[l])):
        # Select a random new position to set the J sudoku
        old_sudo = sudoku_list[l][j]
        new_pos = random.randint(0,len(sudoku_list[l])-1)
        sudoku_list[l][j] = sudoku_list[l][new_pos]
        sudoku_list[l][new_pos] = old_sudo
        

      
    return sudoku_list
  

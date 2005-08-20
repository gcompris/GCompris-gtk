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

    # Holds the coordinate of the current square
    self.cursqre = None

    self.normal_square_color = 0xbebbc9ffL
    self.focus_square_color  = 0x8b83a7ffL
    self.lines_color         = 0xebe745ffL
    
    self.fixed_number_color  = 0xff2100ffL
    self.user_number_color   = 0x000bffffL
    
    print("Gcompris_sudoku __init__.")


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
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

    # Init the sudoku dataset
    self.sudoku = self.init_item_list()
    self.current_sudoku = -1

    # Display the board area
    square_width = 50
    square_height = 50
    x_init = (gcompris.BOARD_WIDTH - square_width*9)/2
    y_init = (gcompris.BOARD_HEIGHT - square_height*9)/2
    
    for x in range(0,9):
      line_square = []
      line_number = []

      for y in range(0,9):

        item = self.rootitem.add(
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

        item = self.rootitem.add(
          gnome.canvas.CanvasText,
          x= x_init + square_width * x + square_width/2,
          y= y_init + square_height * y + square_height/2,
          text= "",
          font=gcompris.skin.get_font("gcompris/content"),
          )
        line_number.append(item)
        
        if(y>0 and y%3==0):
          self.rootitem.add (
            gnome.canvas.CanvasLine,
            points=(x_init - 5,
                    y_init + square_height * y,
                    x_init + (square_width * 9) + 5,
                    y_init + square_height * y),
            fill_color_rgba=self.lines_color,
            width_units=2.5,
            )

      self.sudo_square.append(line_square)
      self.sudo_number.append(line_number)

      if(x>0 and x%3==0):
        self.rootitem.add (
          gnome.canvas.CanvasLine,
          points=(x_init + square_width * x,
                  y_init - 5,
                  x_init + square_width * x,
                  y_init + square_height * 9 + 5),
          fill_color_rgba=self.lines_color,
          width_units=2.5,
          )

    self.next_level()
    
    self.pause(0);

    print("Gcompris_sudoku start.")


  def end(self):

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()


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

    if(keyval >= gtk.keysyms._1 and
       keyval <= gtk.keysyms._9):

      utf8char = gtk.gdk.keyval_to_unicode(keyval)
      strn = u'%c' % utf8char

      if self.is_possible(strn):
        self.sudo_number[self.cursqre[0]][self.cursqre[1]].set(
          text = strn.encode('UTF-8'),
          )

        # Maybe it's all done
        if self.is_solved():
          print "is solved"
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
      self.next_level()

    return
                  


  def set_level(self, level):
    print("Gcompris_sudoku set level. %i" % level)

# ---- End of Initialisation

  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return True if continue, False if bail out
  def next_level(self):

    self.current_sudoku += 1

    if(not self.sudoku[self.current_sudoku]):
      gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
      return False

    self.display_sudoku(self.sudoku[self.current_sudoku])
    return True
      
  def square_item_event(self, widget, event, square):
    if event.type == gtk.gdk.BUTTON_PRESS:
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

  # Return True or False if the given number is possible
  #
  def is_possible(self, number):

    if(self.cursqre == None):
      return False

    # Check this number if not already in a row
    for x in range(0,9):
      if x == self.cursqre[0]:
         continue

      item = self.sudo_number[x][self.cursqre[1]]
      othernumber = item.get_property('text').decode('UTF-8')

      if(number == othernumber):
        return False

    # Check this number if not already in a column
    for y in range(0,9):
      if y == self.cursqre[1]:
         continue

      item = self.sudo_number[self.cursqre[0]][y]
      othernumber = item.get_property('text').decode('UTF-8')

      if(number == othernumber):
        return False

    #
    # Check this number is in a mini sqare
    #

    # First, find the top-left mini square
    
    return True
 
  # Return True or False if the given sudoku is solved
  # We don't really check it's solved, only that all squares
  # have a value. This worls because only valid numbers can
  # be entered up front.
  #
  def is_solved(self):

    for x in range(0,9):
      for y in range(0,9):
        item = self.sudo_number[x][y]
        number = item.get_property('text').decode('UTF-8')
        if(number<1 or number>9):
          return False
        
    return True

  # Display the given sudoku
  def display_sudoku(self, sudoku):
    
    for x in range(0,9):
      for y in range(0,9):
        text  = sudoku[y][x]
        color = self.fixed_number_color
        if text == 0:
          text  = ""
          color = self.user_number_color
        
        self.sudo_number[x][y].set(
          text= text,
          fill_color_rgba= color,
          )
    
  # return the list of items (data) for this game
  def init_item_list(self):

      return \
      [
       [
        [3,0,0,0,0,5,6,0,2],
        [0,6,2,7,1,0,0,4,0],
        [1,0,0,9,0,0,0,5,0],
        [8,0,0,0,0,0,2,6,0],
        [0,3,0,0,8,0,0,7,0],
        [0,9,7,0,0,0,0,0,1],
        [0,5,0,0,0,0,0,0,6],
        [0,8,0,0,6,7,4,2,0],
        [6,0,3,1,0,0,0,0,8]
       ],
       [
        [9,3,1,0,0,7,0,2,8],
        [0,0,0,0,3,6,0,0,0],
        [4,0,6,0,0,2,0,0,0],
        [6,2,0,0,0,0,9,1,0],
        [1,0,0,0,2,0,0,0,4],
        [0,4,8,0,0,0,0,6,5],
        [0,0,0,9,0,0,3,0,2],
        [0,0,0,2,7,0,0,0,0],
        [3,7,0,4,0,0,5,9,1]
       ]
      ]

#  gcompris - sudoku
#
# Copyright (C) 2005, 2008 Bruno Coudoin
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
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.score
import gobject
import gtk
import gtk.gdk
import random
from gcompris import gcompris_gettext as _

class Gcompris_sudoku:
  """Sudoku game"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.disable_im_context = True

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;

    # It holds the canvas items for each square
    self.sudo_square = []       # The square Rect Item
    self.sudo_number = []       # The square Text Item
    self.sudo_symbol = []       # The square Symbol Item

    self.valid_chars = []       # The valid chars for the sudoku are calculated from the dataset

    # Holds the coordinate of the current square
    self.cursqre = None

    self.normal_square_color = 0x33CCFFffL
    self.highl_square_color  = 0xFFCC33ffL
    self.focus_square_color  = 0xFF6633ffL
    self.fixed_square_color  = 0x3366FFffL
    self.error_square_color  = 0xFF3366ffL
    self.lines_color         = 0xFF3366ffL

    self.fixed_number_color  = 0xFFFFFFFFL
    self.user_number_color   = 0xFFFFFFFFL

    self.root_sudo = None

    self.sudoku = None          # The current sudoku data
    self.sudo_size = 0          # the size of the current sudoku
    self.sudo_region = None     # the modulo region in the current sudoku

    self.timer = 0              # The timer that highlights errors

    self.symbolize_level_max = 4 # Last level in which we set symbols
    self.symbols = [
      (gcompris.utils.load_pixmap("sudoku/rectangle.png"),
       gcompris.utils.load_pixmap("sudoku/rectangle_grey.png")),
      (gcompris.utils.load_pixmap("sudoku/circle.png"),
       gcompris.utils.load_pixmap("sudoku/circle_grey.png")),
      (gcompris.utils.load_pixmap("sudoku/rhombus.png"),
       gcompris.utils.load_pixmap("sudoku/rhombus_grey.png")),
      (gcompris.utils.load_pixmap("sudoku/triangle.png"),
       gcompris.utils.load_pixmap("sudoku/triangle_grey.png")),
      (gcompris.utils.load_pixmap("sudoku/star.png"),
       gcompris.utils.load_pixmap("sudoku/star_grey.png"))
      ]

  def start(self):

    # Init the sudoku dataset
    self.sudoku = self.init_item_list()

    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=len(self.sudoku)
    self.gcomprisBoard.sublevel=1

    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT)

    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())

    self.next_level()

    self.pause(0);



  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()
    self.rootitem = None
    gcompris.score.end()

  def ok(self):
    print("Gcompris_sudoku ok.")


  def repeat(self):
    self.display_sudoku(self.sudoku[self.gcomprisBoard.level-1][self.gcomprisBoard.sublevel-1])

  def config(self):
    print("Gcompris_sudoku config.")


  def key_press(self, keyval, commit_str, preedit_str):

    if(self.cursqre == None):
      return False

    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    if(strn in self.valid_chars):

      if self.is_legal(strn):
        self.sudo_number[self.cursqre[0]][self.cursqre[1]].props.text = \
            strn.encode('UTF-8')

        # Maybe it's all done
        if self.is_solved():
          self.cursqre = None
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)

    else:
      # Erase the old number there
      if ((keyval == gtk.keysyms.BackSpace) or
          (keyval == gtk.keysyms.Delete) or
          (keyval == gtk.keysyms.space)):
        self.sudo_number[self.cursqre[0]][self.cursqre[1]].props.text = ""

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

    # Randomize symbols
    for j in range(0, len(self.symbols)):
        # Select a random new position to set the J symbol
        old_symbol = self.symbols[j]
        new_pos = random.randint(0,len(self.symbols)-1)
        self.symbols[j] = self.symbols[new_pos]
        self.symbols[new_pos] = old_symbol

    self.display_sudoku(self.sudoku[self.gcomprisBoard.level-1][self.gcomprisBoard.sublevel-1])

    gcompris.score.start(gcompris.score.STYLE_NOTE, 610, 485,
                         len(self.sudoku[self.gcomprisBoard.level-1]))
    gcompris.score.set(self.gcomprisBoard.sublevel)

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
      # Set the level in the control bar
      gcompris.bar_set_level(self.gcomprisBoard);

      if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
        self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

    return True

  #
  # Set a symbol in the sudoku
  #
  def set_sudoku_symbol(self, text, x, y):
    (pixmap, pixmap_grey) = self.get_pixmap_symbol(self.valid_chars, text)

    self.sudo_symbol[x][y].props.pixbuf = pixmap
    self.sudo_symbol[x][y].props.visibility = goocanvas.ITEM_VISIBLE

    self.sudo_number[x][y].props.text = text

  #
  # Event on a placed symbol. Means that we remove it
  #
  def hide_symbol_event(self, item, target, event, data):

    if(self.gamewon):
      return False

    if event.type == gtk.gdk.BUTTON_PRESS:
      item.props.visibility = goocanvas.ITEM_INVISIBLE
      self.sudo_number[data[0]][data[1]].props.text = ""

  #
  # This function is being called uppon a click on a symbol on the left
  # If a square has the focus, then the clicked square is assigned there
  #
  def symbol_item_event(self, item, target, event, text):

    if(self.gamewon):
      return False

    if self.selected_item == item:
      return False

    item.props.pixbuf = item.get_data("pixmap")

    if self.selected_item:
      self.selected_item.props.pixbuf = self.selected_item.get_data("pixmap_grey")

    self.selected_item = item

    return False

  #
  # This function is being called uppon a click on any little square
  #
  def square_item_event(self, widget, target, event, square):

    if(self.gamewon):
      return False

    # At level change this may be undefined
    try:
      self.sudo_square[square[0]][square[1]]
    except:
      return False

    if event.type == gtk.gdk.ENTER_NOTIFY:
      if self.cursqre != square:
        self.sudo_square[square[0]][square[1]].props.fill_color_rgba = \
            self.highl_square_color

    elif event.type == gtk.gdk.LEAVE_NOTIFY:
      if ((self.gcomprisBoard.level >= self.symbolize_level_max)
          and (self.cursqre == square)):
        self.sudo_square[square[0]][square[1]].props.fill_color_rgba = \
            self.focus_square_color
      else:
        self.sudo_square[square[0]][square[1]].props.fill_color_rgba = \
            self.normal_square_color

    elif event.type == gtk.gdk.BUTTON_PRESS:
      # For low levels, we mark the selected item
      if(self.gcomprisBoard.level < self.symbolize_level_max):
        self.cursqre = (square[0], square[1])
        text = self.selected_item.get_data("text")
        if self.is_legal(text):
          self.set_sudoku_symbol(text, square[0], square[1])

          # Maybe it's all done
          if self.is_solved():
            self.gamewon = 1
            gcompris.bonus.display(gcompris.bonus.WIN,
                                   gcompris.bonus.FLOWER)

      else:
        if(self.cursqre):
          self.sudo_square[self.cursqre[0]][self.cursqre[1]].props.fill_color_rgba = \
              self.normal_square_color
        self.cursqre = square
        self.sudo_square[square[0]][square[1]].props.fill_color_rgba = \
            self.focus_square_color
        return True

    return False

  # Highlight the given item to show it's the one that causes the refusal
  # to place a number
  def unset_on_error(self, items):
    if(self.rootitem):
      for item in items:
        item.props.fill_color_rgba = self.fixed_square_color


  def set_on_error(self, items):
    for item in items:
      item.props.fill_color_rgba = self.error_square_color

    self.timer = gobject.timeout_add(3000, self.unset_on_error, items)

  # Return True or False if the given number is possible
  #
  def is_legal(self, number):

    if((self.cursqre[0] == None) or
       (self.cursqre[1] == None)):
      return

    possible = True
    bad_square = []

    if(self.cursqre == None):
      return possible

    # Check this number is not already in a row
    for x in range(0, self.sudo_size):

      if x == self.cursqre[0]:
         continue

      item = self.sudo_number[x][self.cursqre[1]]
      othernumber = item.get_property('text').decode('UTF-8')

      if(number == othernumber):
        bad_square.append(self.sudo_square[x][self.cursqre[1]])
        possible = False

    # Check this number is not already in a column
    for y in range(0, self.sudo_size):

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
  # Display valid number (or chars)
  #
  def display_valid_chars(self, sudo_size, valid_chars):

    square_width = (gcompris.BOARD_HEIGHT-50)/sudo_size
    square_height = square_width

    x_init = 20
    y_init = (gcompris.BOARD_HEIGHT - square_height*sudo_size)/2

    for y in range(0, sudo_size):

      # For low levels, we symbolize the letters
      if(self.gcomprisBoard.level < self.symbolize_level_max):
        (pixmap, pixmap_grey) = self.get_pixmap_symbol(valid_chars, valid_chars[y])
        zoom = (square_width*0.5) / pixmap.get_width()

        cx = int(x_init + (pixmap.get_width() * zoom)/2)
        cy = int(y_init + square_height * y + (square_height - pixmap.get_height()*zoom)/2)

        item = goocanvas.Image(
          parent = self.root_sudo,
          pixbuf = pixmap_grey,
          x = cx,
          y = cy / zoom)
        item.scale(zoom, zoom)
        item.set_data("pixmap_grey", pixmap_grey)
        item.set_data("pixmap", pixmap)
        item.set_data("text", valid_chars[y])

        item.connect("button_press_event", self.symbol_item_event, valid_chars[y])
        # This item is clickeable and it must be seen
        gcompris.utils.item_focus_init(item, None)

        # Select the first item
        if y == 0:
          self.selected_item = item
          item.props.pixbuf = pixmap

      else:
        # Text
        goocanvas.Text(
          parent = self.root_sudo,
          x= x_init,
          y= y_init + square_height * y + square_height/2,
          text= valid_chars[y],
          font=gcompris.skin.get_font("gcompris/title"),
          fill_color = "black"
          )

  # Return a symbol pixmap
  def get_pixmap_symbol(self, valid_chars, text):
    if (not text in valid_chars):
      # Return our first item. It will be hidden anyway.
      return self.symbols[0]

    return self.symbols[valid_chars.index(text)]

  #
  # Display the given sudoku (the board and the data)
  #
  def display_sudoku(self, sudoku):

    # Reinit the sudoku globals
    if self.timer:
      gobject.source_remove(self.timer)

    self.sudo_square = []       # The square Rect Item
    self.sudo_number = []       # The square Text Item
    self.sudo_symbol = []       # The square Symbol Item

    # The valid chars for the sudoku are calculated from the dataset
    self.valid_chars = []

    self.cursqre = None

    self.sudo_size = len(sudoku[0])

    if(self.root_sudo):
      self.root_sudo.remove()

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.root_sudo = goocanvas.Group(parent = self.rootitem)


    # Region is the modulo place to set region (if defined)
    region = None
    if(self.sudo_region.has_key(self.sudo_size)):
      region = self.sudo_region[self.sudo_size]

    square_width = (gcompris.BOARD_HEIGHT-80) / self.sudo_size
    square_height = square_width
    x_init = (gcompris.BOARD_WIDTH - square_width*self.sudo_size)/2 - 40
    y_init = (gcompris.BOARD_HEIGHT - 45 - square_height*self.sudo_size)/2

    for x in range(0,self.sudo_size):
      line_square = []
      line_number = []
      line_symbol = []

      for y in range(0,self.sudo_size):

        # Get the data from the dataset
        text  = sudoku[y][x]
        if text == '.':
          text  = ""
          color = self.user_number_color
          square_color = self.normal_square_color
        else:
          color = self.fixed_number_color
          square_color = self.fixed_square_color
          if(not text in self.valid_chars):
            self.valid_chars.append(text)

        # The background of the square
        item = goocanvas.Rect(
            parent = self.root_sudo,
            fill_color_rgba = square_color,
            x = x_init + square_width * x,
            y = y_init + square_height * y,
            width = square_width,
            height = square_height,
            line_width = 1.0,
            stroke_color_rgba = 0x144B9DFFL
            )
        line_square.append(item)
        if text == '':
          item.connect("button_press_event", self.square_item_event, (x,y))
          item.connect("enter_notify_event", self.square_item_event, (x,y))
          item.connect("leave_notify_event", self.square_item_event, (x,y))

        # Save it's sudo coord for the drag and drop
        item.set_data('sudo_x', str(x))
        item.set_data('sudo_y', str(y))

        # For low levels, we symbolize the letters
        if(self.gcomprisBoard.level < self.symbolize_level_max):
          (pixmap, pixmap_grey) = self.get_pixmap_symbol(self.valid_chars, text)
          zoom = (square_width*0.835) / pixmap.get_width()

          item = goocanvas.Image(
            parent = self.root_sudo,
            pixbuf = pixmap,
            x = (x_init + square_width * x + square_width/2
            - (pixmap.get_width() * zoom)/2) / zoom,
            y = (y_init + square_height * y + square_height/2
                 - (pixmap.get_height() * zoom)/2) / zoom,
            )
          item.scale(zoom, zoom)
          line_symbol.append(item)

          # Save it's sudo coord for the drag and drop
          item.set_data('sudo_x', str(x))
          item.set_data('sudo_y', str(y))

          if(not text):
            item.props.visibility = goocanvas.ITEM_INVISIBLE
            # This item is clickeable and it must be seen
            gcompris.utils.item_focus_init(item, None)
            item.connect("button_press_event", self.hide_symbol_event, (x, y))

        #
        #
        #

        item = goocanvas.Text(
          parent = self.root_sudo,
          x= x_init + square_width * x + square_width/2 - 5,
          y= y_init + square_height * y + square_height/2 - 5,
          text = text,
          fill_color_rgba= color,
          font=gcompris.skin.get_font("gcompris/content"),
          )
        if(self.gcomprisBoard.level<self.symbolize_level_max):
          item.props.visibility = goocanvas.ITEM_INVISIBLE

        line_number.append(item)

        if(region):
          if(y>0 and y%region==0):
            goocanvas.Polyline(
              parent = self.root_sudo,
              points = goocanvas.Points([(x_init - 5,
                                          y_init + square_height * y),
                                         (x_init + (square_width * self.sudo_size) + 5,
                                          y_init + square_height * y)]),
              stroke_color_rgba = self.lines_color,
              line_width = 2.5,
              )

      self.sudo_square.append(line_square)
      self.sudo_number.append(line_number)
      self.sudo_symbol.append(line_symbol)

      if(region):
        if(x>0 and x%region==0):
          goocanvas.Polyline(
            parent = self.root_sudo,
            points = goocanvas.Points([(x_init + square_width * x,
                                        y_init - 5),
                                       (x_init + square_width * x,
                                        y_init + square_height * self.sudo_size + 5)]),
            stroke_color_rgba = self.lines_color,
            line_width = 2.5,
            )

    if(self.gcomprisBoard.level >= self.symbolize_level_max):
      self.valid_chars.sort()

    self.display_valid_chars(self.sudo_size, self.valid_chars)

  # return the list of items (data) for this game
  def init_item_list(self):

    # It's hard coded that sudoku of size x have y region.
    # If not defined there, it means no region
    #
    # Sudoku size : Number of region
    #
    self.sudo_region = {
      9: 3,
      4: 2
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
        [
         ['A','B','.'],
         ['C','.','B'],
         ['B','.','A']
        ],
        [
         ['.','B','.'],
         ['.','C','.'],
         ['.','A','.']
        ],
       ],
       [ # Level 2
        [
         ['C','.','.','D'],
         ['.','.','B','.'],
         ['A','.','.','.'],
         ['.','.','D','.']
        ],
        [
         ['.','B','.','A'],
         ['.','.','B','.'],
         ['C','.','D','.'],
         ['.','.','.','C']
        ],
        [
         ['A','.','B','.'],
         ['.','C','.','A'],
         ['.','.','.','D'],
         ['D','.','C','.']
        ],
        [
         ['.','A','.','.'],
         ['C','.','A','B'],
         ['.','.','C','.'],
         ['D','.','.','A']
        ],
        [
         ['C','.','.','D'],
         ['B','.','A','.'],
         ['.','B','.','A'],
         ['.','.','.','.']
        ],
        [
         ['.','A','C','.'],
         ['.','.','.','D'],
         ['C','.','.','A'],
         ['.','B','.','.']
        ],
        [
         ['.','C','.','D'],
         ['B','.','.','.'],
         ['.','.','.','.'],
         ['C','A','.','B']
        ],
        [
         ['B','.','.','C'],
         ['.','A','.','.'],
         ['.','.','D','.'],
         ['.','B','.','.']
        ],
       ],
       [ # Level 3
        [
         ['A','B','C','D','E'],
         ['.','A','B','C','D'],
         ['.','.','A','B','C'],
         ['.','.','.','A','B'],
         ['.','.','.','.','A']
        ],
        [
         ['A','B','.','D','.'],
         ['.','.','D','E','A'],
         ['C','.','.','A','.'],
         ['D','E','.','.','C'],
         ['.','A','B','.','D']
        ],
        [
         ['.','C','.','A','.'],
         ['A','.','B','.','C'],
         ['.','B','.','C','.'],
         ['D','.','C','.','A'],
         ['.','A','E','.','B']
        ],
        [
         ['C','B','.','.','D'],
         ['.','.','D','C','.'],
         ['D','.','B','.','E'],
         ['.','A','.','D','C'],
         ['E','.','.','B','.']
        ],
        [
         ['D','.','.','B','E'],
         ['.','E','A','.','.'],
         ['A','C','.','.','B'],
         ['.','.','B','C','.'],
         ['C','B','.','A','.']
        ],
        [
         ['.','.','C','D','.'],
         ['B','.','.','.','C'],
         ['.','C','.','B','D'],
         ['C','.','D','A','.'],
         ['D','E','.','.','A']
        ],
       ],
       [ # Level 4
        [
         ['1','2','3','4','5'],
         ['.','1','2','3','4'],
         ['.','.','1','2','3'],
         ['.','.','.','1','2'],
         ['.','.','.','.','1']
        ],
        [
         ['1','2','.','4','.'],
         ['.','.','4','5','1'],
         ['3','.','.','1','.'],
         ['4','5','.','.','3'],
         ['.','1','2','.','4']
        ],
        [
         ['.','3','.','1','.'],
         ['1','.','2','.','3'],
         ['.','2','.','3','.'],
         ['4','.','3','.','1'],
         ['.','1','5','.','2']
        ],
        [
         ['3','2','.','.','4'],
         ['.','.','4','3','.'],
         ['4','.','2','.','5'],
         ['.','1','.','4','3'],
         ['5','.','.','2','.']
        ],
        [
         ['4','.','.','2','5'],
         ['.','5','1','.','.'],
         ['1','3','.','.','2'],
         ['.','.','2','3','.'],
         ['3','2','.','1','.']
        ],
        [
         ['.','.','3','4','.'],
         ['2','.','.','.','3'],
         ['.','3','.','2','4'],
         ['3','.','4','1','.'],
         ['4','5','.','.','1']
        ],
       ],
       [ # Level 5
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
         ],
        [
         ['.','7','6','.','.','5','.','.','8'],
         ['.','8','.','.','.','.','.','5','.'],
         ['.','.','2','.','6','1','4','3','.'],
         ['8','.','.','.','1','.','.','.','.'],
         ['.','.','.','6','.','3','.','.','.'],
         ['.','.','.','.','7','.','.','.','1'],
         ['.','9','5','4','8','.','3','.','.'],
         ['.','2','.','.','.','.','.','8','.'],
         ['7','.','.','9','.','.','1','6','.']
        ],
        [
         ['.','7','6','3','.','.','9','.','.'],
         ['3','8','.','7','4','.','.','5','.'],
         ['9','.','2','.','.','.','.','.','.'],
         ['.','.','9','.','.','.','.','.','.'],
         ['.','1','.','6','.','3','.','4','.'],
         ['.','.','.','.','.','.','6','.','.'],
         ['.','.','.','.','.','.','3','.','2'],
         ['.','2','.','.','3','7','.','8','9'],
         ['.','.','8','.','.','2','1','6','.']
        ],
        [
         ['.','7','6','.','.','5','.','1','.'],
         ['.','8','.','.','4','.','.','.','6'],
         ['9','.','2','.','.','.','4','.','.'],
         ['.','.','.','5','.','.','.','2','.'],
         ['.','1','7','.','.','.','8','4','.'],
         ['.','4','.','.','.','8','.','.','.'],
         ['.','.','5','.','.','.','3','.','2'],
         ['6','.','.','.','3','.','.','8','.'],
         ['.','3','.','9','.','.','1','6','.']
        ],
        [
         ['.','7','.','.','2','5','.','1','8'],
         ['3','.','.','.','.','9','2','.','.'],
         ['.','.','.','.','6','.','4','3','.'],
         ['.','.','.','.','.','4','7','2','.'],
         ['.','.','.','.','.','.','.','.','.'],
         ['.','4','3','2','.','.','.','.','.'],
         ['.','9','5','.','8','.','.','.','.'],
         ['.','.','4','1','.','.','.','.','9'],
         ['7','3','.','9','5','.','.','6','.']
        ],
        [
         ['.','.','.','3','.','5','9','.','.'],
         ['3','8','.','.','.','.','2','.','6'],
         ['9','5','.','.','6','.','.','.','.'],
         ['.','.','9','.','1','4','.','.','3'],
         ['.','.','.','.','.','.','.','.','.'],
         ['5','.','.','2','7','.','6','.','.'],
         ['.','.','.','.','8','.','.','7','2'],
         ['6','.','4','.','.','.','.','8','9'],
         ['.','.','8','9','.','2','.','.','.']
        ],
        [
         ['.','.','6','.','2','5','.','.','8'],
         ['.','8','.','.','.','9','.','.','6'],
         ['9','.','.','.','.','.','4','.','.'],
         ['.','6','.','.','1','.','.','2','3'],
         ['.','.','.','6','.','3','.','.','.'],
         ['5','4','.','.','7','.','.','9','.'],
         ['.','.','5','.','.','.','.','.','2'],
         ['6','.','.','1','.','.','.','8','.'],
         ['7','.','.','9','5','.','1','.','.']
        ],
        [
         ['.','.','6','.','.','5','9','1','8'],
         ['.','.','.','7','4','.','.','5','.'],
         ['9','.','.','.','6','.','.','3','.'],
         ['.','.','.','5','.','.','7','.','.'],
         ['2','.','.','.','.','.','.','.','5'],
         ['.','.','3','.','.','8','.','.','.'],
         ['.','9','.','.','8','.','.','.','2'],
         ['.','2','.','.','3','7','.','.','.'],
         ['7','3','8','9','.','.','1','.','.']
        ],
        [
         ['.','.','.','5','.','4','.','.','.'],
         ['2','3','4','8','.','.','.','1','.'],
         ['.','.','.','.','1','3','8','.','2'],
         ['.','5','.','4','7','2','.','.','1'],
         ['.','7','.','.','.','.','.','6','.'],
         ['4','.','.','6','9','5','.','2','.'],
         ['7','.','1','3','5','.','.','.','.'],
         ['.','9','.','.','.','1','4','5','7'],
         ['.','.','.','7','.','9','.','.','.']
        ],
        [
         ['.','.','.','.','.','.','6','.','.'],
         ['2','.','.','8','6','.','9','.','5'],
         ['5','.','7','.','1','3','8','.','2'],
         ['6','.','8','.','7','.','.','.','1'],
         ['.','7','.','.','3','.','.','6','.'],
         ['4','.','.','.','9','.','7','.','8'],
         ['7','.','1','3','5','.','2','.','9'],
         ['3','.','6','.','8','1','.','.','7'],
         ['.','.','5','.','.','.','.','.','.']
        ],
        [
         ['.','8','9','5','.','4','.','7','3'],
         ['2','.','.','.','6','7','.','.','5'],
         ['.','.','.','9','.','.','.','4','2'],
         ['6','5','.','.','.','.','.','.','.'],
         ['.','7','2','1','3','8','5','6','.'],
         ['.','.','.','.','.','.','.','2','8'],
         ['7','4','.','.','.','6','.','.','.'],
         ['3','.','.','2','8','.','.','.','7'],
         ['8','2','.','7','.','9','1','3','.']
        ],
        [
         ['.','8','9','.','2','4','.','.','3'],
         ['.','.','.','.','.','.','.','1','.'],
         ['5','.','.','.','1','3','8','.','2'],
         ['6','.','.','4','.','2','.','.','.'],
         ['.','7','2','1','.','8','5','6','.'],
         ['.','.','.','6','.','5','.','.','8'],
         ['7','.','1','3','5','.','.','.','9'],
         ['.','9','.','.','.','.','.','.','.'],
         ['8','.','.','7','4','.','1','3','.']
        ],
        [
         ['.','.','.','.','.','.','.','9','8'],
         ['.','.','.','.','.','4','1','.','.'],
         ['8','4','3','6','.','.','2','.','.'],
         ['9','.','5','.','.','3','.','.','.'],
         ['3','2','.','.','.','.','.','8','7'],
         ['.','.','.','9','.','.','3','.','1'],
         ['.','.','2','.','.','5','7','3','6'],
         ['.','.','7','1','.','.','.','.','.'],
         ['5','9','.','.','.','.','.','.','.']
        ],
        [
         ['2','.','.','3','5','7','.','.','.'],
         ['7','.','9','2','.','.','.','6','.'],
         ['.','.','.','.','.','.','.','.','.'],
         ['9','.','.','.','.','.','.','2','4'],
         ['.','.','6','5','4','1','9','.','.'],
         ['4','7','.','.','.','.','.','.','1'],
         ['.','.','.','.','.','.','.','.','.'],
         ['.','3','.','.','.','8','5','.','9'],
         ['.','.','.','7','3','6','.','.','2']
        ],
        [
         ['.','.','1','.','5','7','4','.','.'],
         ['.','.','9','2','.','.','.','.','.'],
         ['8','4','.','6','.','.','.','7','.'],
         ['.','.','5','.','.','3','.','2','.'],
         ['3','.','.','.','.','.','.','.','7'],
         ['.','7','.','9','.','.','3','.','.'],
         ['.','8','.','.','.','5','.','3','6'],
         ['.','.','.','.','.','8','5','.','.'],
         ['.','.','4','7','3','.','8','.','.']
        ],
        [
         ['2','6','.','3','.','.','.','9','.'],
         ['7','.','.','2','.','4','.','6','.'],
         ['.','.','.','.','.','9','.','.','.'],
         ['9','.','5','.','7','.','6','.','4'],
         ['.','.','.','.','.','.','.','.','.'],
         ['4','.','8','.','6','.','3','.','1'],
         ['.','.','.','4','.','.','.','.','.'],
         ['.','3','.','1','.','8','.','.','9'],
         ['.','9','.','.','.','6','.','1','2']
        ],
        [
         ['4','9','.','.','.','.','.','1','.'],
         ['.','8','.','.','.','2','5','.','.'],
         ['1','.','5','.','.','.','6','.','.'],
         ['7','.','.','6','4','.','.','.','.'],
         ['5','.','.','7','.','3','.','.','8'],
         ['.','.','.','.','2','1','.','.','4'],
         ['.','.','8','.','.','.','3','.','1'],
         ['.','.','1','9','.','.','.','7','.'],
         ['.','7','.','.','.','.','.','2','5']
        ],
        [
         ['9','6','.','4','1','7','.','2','3'],
         ['.','.','8','.','9','5','6','.','1'],
         ['.','.','.','6','.','.','.','.','9'],
         ['.','.','.','.','2','.','9','.','.'],
         ['4','7','.','.','5','.','.','8','6'],
         ['.','.','6','.','4','.','.','.','.'],
         ['8','.','.','.','.','3','.','.','.'],
         ['6','.','3','1','7','.','5','.','.'],
         ['1','9','.','5','8','4','.','6','2']
        ],
        [
         ['.','.','.','4','.','7','8','.','.'],
         ['7','3','.','.','.','5','6','.','.'],
         ['.','1','4','.','3','.','7','.','.'],
         ['5','8','.','.','.','6','.','.','.'],
         ['4','.','.','3','.','1','.','.','6'],
         ['.','.','.','8','.','.','.','7','5'],
         ['.','.','2','.','6','.','4','1','.'],
         ['.','.','3','1','.','.','.','9','8'],
         ['.','.','7','5','.','4','.','.','.']
        ],
        [
         ['9','.','5','.','.','7','8','2','.'],
         ['7','.','.','2','.','.','.','4','.'],
         ['.','1','.','6','3','.','.','.','9'],
         ['.','.','1','7','2','.','9','.','.'],
         ['.','.','.','3','.','1','.','.','.'],
         ['.','.','6','.','4','9','1','.','.'],
         ['8','.','.','.','6','3','.','1','.'],
         ['.','4','.','.','.','2','.','.','8'],
         ['.','9','7','5','.','.','3','.','2']
        ],
        [
         ['9','6','.','4','1','7','8','.','3'],
         ['.','.','8','2','9','5','.','.','.'],
         ['2','.','.','6','3','.','.','.','9'],
         ['.','.','.','7','.','.','.','3','4'],
         ['4','.','.','.','5','.','.','.','6'],
         ['3','2','.','.','.','9','.','.','.'],
         ['8','.','.','.','6','3','.','.','7'],
         ['.','.','.','1','7','2','5','.','.'],
         ['1','.','7','5','8','4','.','6','2']
        ],
        [
         ['.','.','1','.','.','4','7','.','3'],
         ['7','.','.','.','6','.','.','5','.'],
         ['.','.','2','7','.','8','.','.','6'],
         ['.','9','.','.','4','.','.','.','.'],
         ['.','.','.','3','.','9','.','.','.'],
         ['.','.','.','.','5','.','.','2','.'],
         ['6','.','.','9','.','7','4','.','.'],
         ['.','7','.','.','8','.','.','.','1'],
         ['4','.','8','6','.','.','3','.','.']
        ],
        [
         ['.','.','1','.','.','.','.','9','.'],
         ['7','.','.','1','.','3','.','5','.'],
         ['.','.','.','.','9','8','1','.','6'],
         ['.','9','.','.','4','.','.','.','.'],
         ['.','8','4','.','.','.','6','1','.'],
         ['.','.','.','.','5','.','.','2','.'],
         ['6','.','5','9','3','.','.','.','.'],
         ['.','7','.','4','.','2','.','.','1'],
         ['.','2','.','.','.','.','3','.','.']
        ],
        [
         ['8','.','.','.','2','4','.','.','.'],
         ['7','.','9','1','.','.','.','.','.'],
         ['.','5','.','.','9','.','.','4','.'],
         ['.','.','6','.','.','.','8','3','.'],
         ['2','.','4','.','.','.','6','.','5'],
         ['.','3','7','.','.','.','9','.','.'],
         ['.','1','.','.','3','.','.','8','.'],
         ['.','.','.','.','.','2','5','.','1'],
         ['.','.','.','6','1','.','.','.','9']
        ],
        [
         ['.','.','1','.','2','.','.','.','.'],
         ['.','.','9','.','6','3','.','.','.'],
         ['3','.','.','.','.','8','1','4','.'],
         ['.','9','.','.','.','.','8','3','.'],
         ['.','.','4','.','7','.','6','.','.'],
         ['.','3','7','.','.','.','.','2','.'],
         ['.','1','5','9','.','.','.','.','2'],
         ['.','.','.','4','8','.','5','.','.'],
         ['.','.','.','.','1','.','3','.','.']
        ],
        [
         ['.','6','.','5','.','4','7','.','3'],
         ['.','.','9','.','.','.','.','5','.'],
         ['3','.','2','.','9','.','.','.','.'],
         ['5','.','6','.','.','.','8','.','.'],
         ['.','8','.','.','.','.','.','1','.'],
         ['.','.','7','.','.','.','9','.','4'],
         ['.','.','.','.','3','.','4','.','2'],
         ['.','7','.','.','.','.','5','.','.'],
         ['4','.','8','6','.','5','.','7','.']
        ],
        [
         ['.','.','.','.','2','.','.','9','.'],
         ['.','.','.','.','6','3','.','.','8'],
         ['3','.','.','.','.','8','1','4','.'],
         ['.','.','.','.','4','.','8','.','7'],
         ['.','8','4','.','.','.','6','1','.'],
         ['1','.','7','.','5','.','.','.','.'],
         ['.','1','5','9','.','.','.','.','2'],
         ['9','.','.','4','8','.','.','.','.'],
         ['.','2','.','.','1','.','.','.','.']
        ],
        [
         ['8','.','1','.','.','4','.','.','.'],
         ['.','.','.','.','6','.','.','5','.'],
         ['3','.','2','7','.','.','.','.','6'],
         ['.','9','.','.','4','.','.','.','7'],
         ['2','.','.','3','.','9','.','.','5'],
         ['1','.','.','.','5','.','.','2','.'],
         ['6','.','.','.','.','7','4','.','2'],
         ['.','7','.','.','8','.','.','.','.'],
         ['.','.','.','6','.','.','3','.','9']
        ],
        [
         ['.','.','.','.','2','.','9','4','3'],
         ['5','3','.','9','.','.','.','8','.'],
         ['.','.','.','.','.','1','6','.','7'],
         ['.','.','.','3','.','.','8','.','4'],
         ['.','.','.','2','7','9','.','.','.'],
         ['9','.','3','.','.','5','.','.','.'],
         ['7','.','9','6','.','.','.','.','.'],
         ['.','6','.','.','.','4','.','7','5'],
         ['1','4','5','.','8','.','.','.','.']
        ],
        [
         ['.','7','1','5','.','.','9','4','.'],
         ['5','.','.','9','6','.','.','8','2'],
         ['.','9','.','.','.','.','.','.','.'],
         ['2','5','.','3','.','.','8','9','.'],
         ['.','.','.','2','7','9','.','.','.'],
         ['.','1','3','.','.','5','.','2','6'],
         ['.','.','.','.','.','.','.','1','.'],
         ['3','6','.','.','9','4','.','.','5'],
         ['.','4','5','.','.','2','3','6','.']
        ],
        [
         ['6','7','1','5','2','.','.','.','.'],
         ['5','.','.','9','6','7','.','.','2'],
         ['.','.','.','.','.','.','6','.','.'],
         ['.','.','7','3','.','6','8','9','.'],
         ['4','.','6','2','.','9','5','.','1'],
         ['.','1','3','8','.','5','7','.','.'],
         ['.','.','9','.','.','.','.','.','.'],
         ['3','.','.','1','9','4','.','.','5'],
         ['.','.','.','.','8','2','3','6','9']
        ],
        [
         ['.','7','.','5','2','.','.','4','3'],
         ['5','.','.','.','.','7','1','8','2'],
         ['.','9','.','.','.','1','6','.','.'],
         ['.','5','.','3','1','6','8','9','.'],
         ['.','.','.','.','.','.','.','.','.'],
         ['.','1','3','8','4','5','.','2','.'],
         ['.','.','9','6','.','.','.','1','.'],
         ['3','6','8','1','.','.','.','.','5'],
         ['1','4','.','.','8','2','.','6','.']
        ],
        [
         ['.','.','.','1','6','.','.','2','7'],
         ['.','.','7','.','8','9','.','6','.'],
         ['1','.','6','.','5','.','.','8','9'],
         ['.','.','.','5','.','.','2','3','8'],
         ['.','.','.','8','2','3','.','.','.'],
         ['8','2','3','.','.','1','.','.','.'],
         ['5','1','.','.','3','.','9','.','2'],
         ['.','8','.','7','1','.','4','.','.'],
         ['6','7','.','.','9','5','.','.','.']
        ],
        [
         ['.','.','8','1','.','.','.','2','7'],
         ['.','.','7','.','8','9','.','.','.'],
         ['.','.','6','2','.','.','3','8','.'],
         ['7','.','1','5','.','6','.','3','8'],
         ['.','.','.','.','.','.','.','.','.'],
         ['8','2','.','9','.','1','6','.','5'],
         ['.','1','4','.','.','8','9','.','.'],
         ['.','.','.','7','1','.','4','.','.'],
         ['6','7','.','.','.','5','8','.','.']
        ],
        [
         ['.','3','.','.','.','4','.','2','.'],
         ['2','.','.','.','8','9','1','.','4'],
         ['1','.','.','2','.','.','3','8','.'],
         ['7','.','1','.','.','6','.','.','8'],
         ['.','6','.','.','.','.','.','9','.'],
         ['8','.','.','9','.','.','6','.','5'],
         ['.','1','4','.','.','8','.','.','2'],
         ['3','.','9','7','1','.','.','.','6'],
         ['.','7','.','4','.','.','.','1','.']
        ],
        [
         ['9','.','8','.','.','.','5','2','.'],
         ['.','.','7','3','.','.','.','6','.'],
         ['.','4','.','.','.','.','.','8','9'],
         ['7','.','.','5','.','6','2','3','.'],
         ['.','.','5','.','2','.','7','.','.'],
         ['.','2','3','9','.','1','.','.','5'],
         ['5','1','.','.','.','.','.','7','.'],
         ['.','8','.','.','.','2','4','.','.'],
         ['.','7','2','.','.','.','8','.','3']
        ],
        [
         ['8','.','.','2','4','.','.','.','.'],
         ['7','.','.','.','1','3','.','8','2'],
         ['6','5','.','.','8','9','1','.','4'],
         ['.','.','.','.','.','7','2','1','6'],
         ['.','1','7','.','3','.','4','5','.'],
         ['5','9','6','1','.','.','.','.','.'],
         ['3','.','5','8','9','.','.','4','1'],
         ['1','7','.','3','6','.','.','.','8'],
         ['.','.','.','.','7','1','.','.','5']
        ],
        [
         ['8','.','.','2','.','6','.','.','.'],
         ['.','.','9','.','1','3','.','.','2'],
         ['6','.','2','7','8','.','.','3','.'],
         ['.','.','3','.','.','.','.','1','6'],
         ['.','.','7','6','.','8','4','.','.'],
         ['5','9','.','.','.','.','8','.','.'],
         ['.','6','.','.','9','2','7','.','1'],
         ['1','.','.','3','6','.','9','.','.'],
         ['.','.','.','4','.','1','.','.','5']
        ],
        [
         ['8','3','.','2','.','.','.','9','.'],
         ['.','4','.','.','.','3','.','8','.'],
         ['.','.','.','.','8','9','1','.','4'],
         ['.','8','.','9','.','.','2','.','.'],
         ['2','.','.','6','.','8','.','.','9'],
         ['.','.','6','.','.','4','.','7','.'],
         ['3','.','5','8','9','.','.','.','.'],
         ['.','7','.','3','.','.','.','2','.'],
         ['.','2','.','.','.','1','.','6','5']
        ],
        [
         ['.','3','1','2','4','.','.','.','.'],
         ['.','.','9','5','1','.','6','8','.'],
         ['6','.','.','.','.','.','.','3','.'],
         ['.','8','.','9','.','7','2','.','.'],
         ['2','.','7','.','.','.','4','.','9'],
         ['.','.','6','1','.','4','.','7','.'],
         ['.','6','.','.','.','.','.','.','1'],
         ['.','7','4','.','6','5','9','.','.'],
         ['.','.','.','.','7','1','3','6','.']
        ],
        [
         ['.','.','7','.','.','.','.','5','.'],
         ['1','.','4','.','.','3','7','.','6'],
         ['.','6','5','7','4','2','9','.','3'],
         ['2','.','.','4','.','5','6','3','9'],
         ['.','.','.','3','.','7','.','.','.'],
         ['4','8','3','9','.','6','.','.','5'],
         ['9','.','8','2','5','4','1','6','.'],
         ['5','.','2','6','.','.','3','.','8'],
         ['.','1','.','.','.','.','5','.','.']
        ],
        [
         ['.','.','7','.','.','.','.','.','2'],
         ['.','2','.','.','.','3','.','8','6'],
         ['.','6','5','7','4','.','9','.','3'],
         ['.','.','.','4','.','.','6','.','.'],
         ['.','5','9','.','.','.','8','4','.'],
         ['.','.','3','.','.','6','.','.','.'],
         ['9','.','8','.','5','4','1','6','.'],
         ['5','4','.','6','.','.','.','9','.'],
         ['7','.','.','.','.','.','5','.','.']
        ],
        [
         ['.','.','7','1','.','8','4','5','2'],
         ['1','.','4','.','.','3','.','.','6'],
         ['.','.','5','.','.','2','9','.','3'],
         ['.','.','.','.','.','5','.','3','.'],
         ['.','.','9','3','.','7','8','.','.'],
         ['.','8','.','9','.','.','.','.','.'],
         ['9','.','8','2','.','.','1','.','.'],
         ['5','.','.','6','.','.','3','.','8'],
         ['7','1','6','8','.','9','5','.','.']
        ],
        [
         ['3','.','7','.','.','.','.','5','.'],
         ['.','2','.','5','9','.','.','.','.'],
         ['8','6','.','.','.','2','.','1','.'],
         ['2','.','.','.','8','5','.','.','9'],
         ['.','5','.','.','2','.','.','4','.'],
         ['4','.','.','9','1','.','.','.','5'],
         ['.','3','.','2','.','.','.','6','7'],
         ['.','.','.','.','7','1','.','9','.'],
         ['.','1','.','.','.','.','5','.','4']
        ],
        [
         ['6','8','.','2','.','7','.','3','.'],
         ['.','2','9','.','.','6','8','7','.'],
         ['5','.','.','.','8','4','.','.','.'],
         ['3','.','5','.','.','.','.','8','.'],
         ['.','.','.','7','.','8','.','.','.'],
         ['.','7','.','.','.','.','4','.','3'],
         ['.','.','.','8','4','.','.','.','7'],
         ['.','5','8','1','.','.','9','4','.'],
         ['.','6','.','5','.','9','.','1','8']
        ],
        [
         ['6','.','4','2','1','7','.','.','.'],
         ['.','2','.','.','.','6','.','.','.'],
         ['5','3','7','9','.','.','2','.','1'],
         ['3','.','.','.','.','.','7','8','6'],
         ['.','4','6','.','.','.','1','9','.'],
         ['8','7','1','.','.','.','.','.','3'],
         ['9','.','3','.','.','2','6','5','7'],
         ['.','.','.','1','.','.','.','4','.'],
         ['.','.','.','5','7','9','3','.','8']
        ],
        [
         ['6','8','.','.','1','.','.','3','.'],
         ['1','.','.','3','.','6','.','7','4'],
         ['.','.','7','9','.','4','2','.','1'],
         ['3','.','5','4','.','.','.','.','6'],
         ['.','.','.','.','.','.','.','.','.'],
         ['8','.','.','.','.','5','4','.','3'],
         ['9','.','3','8','.','2','6','.','.'],
         ['7','5','.','1','.','3','.','.','2'],
         ['.','6','.','.','7','.','.','1','8']
        ],
        [
         ['6','.','4','.','.','.','5','.','9'],
         ['1','.','.','.','.','.','8','7','.'],
         ['5','3','.','9','8','.','.','.','1'],
         ['.','.','5','.','.','1','.','8','.'],
         ['.','4','.','7','3','8','.','9','.'],
         ['.','7','.','6','.','.','4','.','.'],
         ['9','.','.','.','4','2','.','5','7'],
         ['.','5','8','.','.','.','.','.','2'],
         ['4','.','2','.','.','.','3','.','8']
        ],
        [
         ['.','.','.','.','5','6','1','.','.'],
         ['.','.','.','8','1','.','.','6','4'],
         ['6','4','.','7','9','.','.','5','.'],
         ['9','.','7','.','8','.','5','4','.'],
         ['4','.','.','.','.','.','.','.','8'],
         ['.','5','6','.','2','.','7','.','3'],
         ['.','6','.','.','3','7','.','8','5'],
         ['3','9','.','.','4','8','.','.','.'],
         ['.','.','2','5','6','.','.','.','.']
        ],
        [
         ['.','.','.','.','.','.','1','.','9'],
         ['5','.','9','.','.','2','.','6','.'],
         ['6','.','.','7','9','.','8','5','.'],
         ['9','.','7','3','8','.','5','.','6'],
         ['.','.','.','6','.','5','.','.','.'],
         ['8','.','6','.','2','4','7','.','3'],
         ['.','6','4','.','3','7','.','.','5'],
         ['.','9','.','1','.','.','6','.','7'],
         ['7','.','2','.','.','.','.','.','.']
        ],
        [
         ['2','.','8','4','.','.','.','.','9'],
         ['5','.','9','8','1','2','.','.','.'],
         ['.','.','1','.','9','.','.','5','.'],
         ['.','.','7','.','.','.','5','4','6'],
         ['.','1','3','6','.','5','2','9','.'],
         ['8','5','6','.','.','.','7','.','.'],
         ['.','6','.','.','3','.','9','.','.'],
         ['.','.','.','1','4','8','6','.','7'],
         ['7','.','.','.','.','9','4','.','1']
        ],
        [
         ['2','.','8','4','5','.','.','7','.'],
         ['.','.','9','.','1','2','3','.','4'],
         ['6','.','.','7','.','.','8','5','.'],
         ['.','.','.','.','8','.','5','4','6'],
         ['4','1','.','.','.','.','.','9','8'],
         ['8','5','6','.','2','.','.','.','.'],
         ['.','6','4','.','.','7','.','.','5'],
         ['3','.','5','1','4','.','6','.','.'],
         ['.','8','.','.','6','9','4','.','1']
        ],
        [
         ['7','9','.','5','.','1','.','.','.'],
         ['.','5','1','.','.','.','.','.','6'],
         ['.','8','.','.','.','9','.','.','2'],
         ['.','.','.','2','5','.','.','8','1'],
         ['.','.','.','.','.','.','.','.','.'],
         ['8','4','.','.','7','3','.','.','.'],
         ['1','.','.','6','.','.','.','7','.'],
         ['5','.','.','.','.','.','8','1','.'],
         ['.','.','.','3','.','2','.','6','5']
        ],
        [
         ['.','.','.','.','6','.','.','.','.'],
         ['.','.','1','.','.','.','.','9','6'],
         ['.','.','4','.','3','9','.','.','.'],
         ['.','6','7','.','.','4','3','8','.'],
         ['2','.','.','8','.','6','.','.','7'],
         ['.','4','5','1','.','.','6','2','.'],
         ['.','.','.','6','8','.','2','.','.'],
         ['5','2','.','.','.','.','8','.','.'],
         ['.','.','.','.','1','.','.','.','.']
        ],
        [
         ['.','.','.','.','6','1','.','3','.'],
         ['.','.','.','.','.','.','7','9','.'],
         ['.','.','4','7','3','.','.','.','2'],
         ['.','6','.','.','5','.','.','8','.'],
         ['2','.','3','.','.','.','5','.','7'],
         ['.','4','.','.','7','.','.','2','.'],
         ['1','.','.','.','8','5','2','.','.'],
         ['.','2','6','.','.','.','.','.','.'],
         ['.','7','.','3','1','.','.','.','.']
        ],
        [
         ['.','.','.','.','.','1','4','3','8'],
         ['.','.','.','.','.','.','.','9','6'],
         ['.','.','.','.','.','9','1','.','2'],
         ['.','.','.','.','.','4','3','.','1'],
         ['2','.','.','8','.','6','.','.','7'],
         ['8','.','5','1','.','.','.','.','.'],
         ['1','.','9','6','.','.','.','.','.'],
         ['5','2','.','.','.','.','.','.','.'],
         ['4','7','8','3','.','.','.','.','.']
        ],
        [
         ['.','7','1','6','.','.','9','.','.'],
         ['4','.','.','.','.','.','3','.','1'],
         ['.','.','.','8','.','2','.','7','.'],
         ['.','5','.','.','.','.','4','.','.'],
         ['2','.','.','1','.','5','.','.','3'],
         ['.','.','3','.','.','.','.','9','.'],
         ['.','1','.','2','.','4','.','.','.'],
         ['6','.','8','.','.','.','.','.','2'],
         ['.','.','4','.','.','9','1','5','.']
        ],
        [
         ['2','.','7','.','6','.','.','.','.'],
         ['4','3','.','.','.','9','2','.','.'],
         ['.','6','5','4','.','.','.','.','.'],
         ['.','7','.','.','.','.','1','.','2'],
         ['.','.','6','8','1','7','5','.','.'],
         ['5','.','3','.','.','.','.','9','.'],
         ['.','.','.','.','.','8','3','5','.'],
         ['.','.','1','9','.','.','.','2','7'],
         ['.','.','.','.','3','.','6','.','1']
        ],
        [
         ['.','9','.','.','.','1','4','8','5'],
         ['.','.','8','7','.','.','2','.','6'],
         ['.','6','.','.','.','2','9','.','3'],
         ['8','7','.','.','9','.','1','.','.'],
         ['.','2','.','8','.','7','.','3','.'],
         ['.','.','3','.','2','.','.','9','8'],
         ['6','.','2','1','.','.','.','5','.'],
         ['3','.','1','.','.','6','8','.','.'],
         ['7','8','9','2','.','.','.','4','.']
        ],
        [
         ['2','.','.','3','.','.','.','8','.'],
         ['.','.','.','7','.','.','2','.','6'],
         ['.','6','.','4','8','.','9','7','3'],
         ['8','.','.','.','9','.','1','.','2'],
         ['9','.','.','8','1','7','.','.','4'],
         ['5','.','3','.','2','.','.','.','8'],
         ['6','4','2','.','7','8','.','5','.'],
         ['3','.','1','.','.','6','.','.','.'],
         ['.','8','.','.','.','5','.','.','1']
        ],
        [
         ['.','.','.','.','.','1','.','8','.'],
         ['4','.','.','.','.','.','2','1','6'],
         ['.','6','.','4','8','.','.','.','3'],
         ['.','.','4','5','9','.','.','6','.'],
         ['9','.','6','.','1','.','5','.','4'],
         ['.','1','.','.','2','4','7','.','.'],
         ['6','.','.','.','7','8','.','5','.'],
         ['3','5','1','.','.','.','.','.','7'],
         ['.','8','.','2','.','.','.','.','.']
        ],
        [
         ['.','.','9','2','.','.','.','.','4'],
         ['.','.','6','.','.','3','1','.','.'],
         ['4','.','.','1','.','.','5','.','.'],
         ['.','3','.','8','.','.','9','.','1'],
         ['.','.','.','3','.','4','.','.','.'],
         ['1','.','4','.','.','6','.','5','.'],
         ['.','.','7','.','.','2','.','.','3'],
         ['.','.','1','7','.','.','6','.','.'],
         ['9','.','.','.','.','1','4','.','.']
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
        ],
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


#  gcompris - guessnumber
#
# Copyright (C) 2005, 2008 Bruno Coudoin / Clement Coudoin
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

import gobject
import goocanvas
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

class Gcompris_guessnumber:
  """Tux hide a number, you must guess it"""


  def __init__(self, gcomprisBoard):

    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.disable_im_context = True

    # A text canvas item use to indicate it's over or lower
    self.indicator = None

    # The text entry for the number
    self.entry = None

    # The min and max value that must be found
    self.min = -1
    self.max = -1

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0
    self.stopped       = True

    # Manage the helico move
    self.move_stepnum = 0
    self.x = self.y = self.x_old = self.y_old = -1
    self.moving = False
    self.action_start = 0
    self.anim = None
    self.velocity = []
    self.sw = self.sh = 1
    self.move_tick = 30
    self.num_moveticks = 20
    self.rotation = 0
    self.movestep_timer = 0

    self.orig_x = 0
    self.orig_y = gcompris.BOARD_HEIGHT/2
    self.target_x = 640
    self.target_y = self.orig_y

    self.helico_width = -1
    self.helico_height = -1

  def start(self):

    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=4
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(gcompris.BAR_LEVEL)

    gcompris.bar_set_level(self.gcomprisBoard)
    gcompris.bar_location(gcompris.BOARD_WIDTH - 160, -1, 0.7)

    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "guessnumber/cave.png")

    self.stopped = False

    self.display_game()


  def end(self):

    self.stopped = True
    # Remove the root item removes all the others inside it
    self.cleanup_game()

  def ok(self):
    pass


  def repeat(self):
    print("Gcompris_guessnumber repeat.")


  def key_press(self, keyval, commit_str, preedit_str):
    # Return  True  if you did process a key
    # Return  False if you did not processed a key
    #         (gtk need to send it to next widget)
    return False

  def pause(self, pause):
    self.board_paused = pause

    # Hack for widget that can't be covered by bonus and/or help
    if pause:
       self.entry.props.visibility = goocanvas.ITEM_INVISIBLE
    else:
      self.entry.props.visibility = goocanvas.ITEM_VISIBLE

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

      if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
        # the current board is finished : stay on the last level
        self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

    return 1

  # Display the board game
  def cleanup_game(self):
    self.gamewon = False
    if self.movestep_timer != 0:
      gtk.timeout_remove(self.movestep_timer)
      self.movestep_timer = 0

    # Remove the root item removes all the others inside it
    self.rootitem.remove()

  # Display the board game
  def display_game(self):

      # Create our rootitem. We put each canvas item in it so at the end we
      # only have to kill it. The canvas deletes all the items it contains automaticaly.
      self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

      self.min = 1
      self.max = 10
      if(self.gcomprisBoard.level == 2):
          self.max = 100
      elif(self.gcomprisBoard.level == 3):
          self.max = 500
      elif(self.gcomprisBoard.level == 4):
          self.max = 1000

      # Select the number to find
      self.solution = random.randint(self.min, self.max)

      text = _("Guess a number between {0} and {1}").format(self.min, self.max)

      goocanvas.Text(
        parent = self.rootitem,
        x=10.0,
        y=30.0,
        font=gcompris.skin.get_font("gcompris/subtitle"),
        text=(text),
        fill_color_rgba=0x1514c4ffL,
        )

      self.indicator =goocanvas.Text(
        parent = self.rootitem,
        x=400.0,
        y=70.0,
        width = 700,
        font=gcompris.skin.get_font("gcompris/subtitle"),
        text=(""),
        fill_color_rgba=0xff0006ffL,
        anchor = gtk.ANCHOR_CENTER,
        alignment = pango.ALIGN_CENTER,
        )

      text_item = self.entry_text()

      #
      # Display the helico
      #
      pixmap = gcompris.utils.load_pixmap("guessnumber/tuxhelico.png")
      self.helico_width = pixmap.get_width()
      self.helico_height = pixmap.get_height()
      self.orig_x = self.x_old = self.x = 20
      self.y_old = self.y = self.orig_y

      self.anim = goocanvas.Image(
        parent = self.rootitem,
        pixbuf = pixmap,
        x = self.x,
        y = self.y,
        )

      # The OK Button
      item = goocanvas.Svg(parent = self.rootitem,
                           svg_handle = gcompris.skin.svg_get(),
                           svg_id = "#OK"
                           )
      item.translate(item.get_bounds().x1 * -1
                     + gcompris.BOARD_WIDTH - (item.get_bounds().x2 - item.get_bounds().x1) - 30,
                     item.get_bounds().y1 * -1
                     + 65)

      item.connect("button_press_event", self.ok_event, text_item)
      gcompris.utils.item_focus_init(item, None)

  def entry_text(self):
    self.entry = gtk.Entry()

    self.entry.modify_font(pango.FontDescription("sans bold 24"))
    text_color = gtk.gdk.color_parse("blue")
    text_color_selected = gtk.gdk.color_parse("green")

    self.entry.modify_text(gtk.STATE_NORMAL, text_color)
    self.entry.modify_text(gtk.STATE_SELECTED, text_color_selected)

    self.entry.set_max_length(4)
    self.entry.connect("activate", self.enter_callback)
    self.entry.connect("changed", self.enter_char_callback)

    self.entry.props.visibility = goocanvas.ITEM_VISIBLE

    self.widget = goocanvas.Widget(
      parent = self.rootitem,
      widget=self.entry,
      x=730,
      y=30,
      width=100,
      height=46,
      anchor=gtk.ANCHOR_CENTER,
      )

    self.widget.raise_(None)

    #self.widget.grab_focus()
    self.entry.grab_focus()

    return self.entry

  def enter_char_callback(self, widget):
      text = widget.get_text()
      widget.set_text(text.decode('utf8').upper().encode('utf8'))

  def enter_callback(self, widget):
    text = widget.get_text()

    # Find a number game
    if str(self.solution) == text:
      self.indicator.props.text = ""
      self.gamewon = True
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)
    else:
      try:
        number = int(text)
      except:
        self.indicator.props.text = \
            _("Please enter a number between {0} and {1}").format(self.min, self.max)
        widget.set_text('')
        return

      if number > self.max or number <= 0:
        self.indicator.props.text = _("Out of range")
      else:
        max_distance = max(self.max - self.solution, self.solution)
        distance_x = self.target_x - abs(self.solution - number) * float(self.target_x - self.orig_x) / max_distance
        distance_y = self.orig_y + float(((self.solution - number) * 170) / max_distance)
        if(number > self.solution):
          self.indicator.props.text=_("Too high")
        else:
          self.indicator.props.text = _("Too low")

        self.move(self.x_old, self.y_old,
                  distance_x,
                  distance_y)

    widget.set_text('')


  def move_step(self):

    if self.stopped:
      return

    if self.move_stepnum < self.num_moveticks-1:
      self.move_stepnum += 1
      x = self.anim.get_property("x") + self.velocity[0]/self.num_moveticks
      y = self.anim.get_property("y") + self.velocity[1]/self.num_moveticks
      self.anim.props.x = x
      self.anim.props.y = y
      return True

    else:

      self.move_stepnum = 0
      self.moving = False
      self.movestep_timer = 0
      (self.x_old, self.y_old) = self.gcomprisBoard.canvas.\
          convert_from_item_space(self.anim, self.x_old, self.y_old)
      self.anim.props.transform = None
      self.anim.props.x = self.x_old
      self.anim.props.y = self.y_old
      self.entry.set_editable(True)
      return False


  def move(self, x_old, y_old, x, y):

    if x == x_old and y == y_old:
      return

    self.entry.set_editable(False)

    if(x > x_old):
      self.rotation = 5
    elif (x<x_old):
      self.rotation = -5
    else:
      self.rotation = 0
    gcompris.utils.item_rotate(self.anim, self.rotation)

    (self.x_old, self.y_old) = self.gcomprisBoard.canvas.\
        convert_to_item_space(self.anim, x, y)
    (x_old, y_old) = self.gcomprisBoard.canvas.\
        convert_to_item_space(self.anim, x_old, y_old)

    (self.x, self.y) = (x, y)

    self.velocity = [float(self.x_old - x_old),
                     float(self.y_old - y_old)]

    self.moving = True
    self.move_stepnum = 0

    # it takes self.num_moveticks iterations of duration self.move_tick to move squares
    self.movestep_timer = gobject.timeout_add(self.move_tick, self.move_step)

  def ok_event(self, widget, target, event, data):
    if self.stopped:
      return
    self.enter_callback(data)

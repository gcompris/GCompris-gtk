#  gcompris - mining.py
#
# Copyright (C) 2012 Peter Albrecht 
#   based on template by Bruno Coudoin
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
# mining activity.

import math
import gobject
import gtk
import gtk.gdk
import gcompris
import gcompris.bonus
import gcompris.utils
import gcompris.skin
import gcompris.sound
import goocanvas
import pango
import random
import cairo

from gcompris import gcompris_gettext as _

class Gcompris_mining:
  """ GCompis Mining-Activity """

  # the number of nuggets, we need to collect in this level
  nuggets_to_collect = 0

  # the number of nuggets, we already have colleced
  nugget_count = 0

  # used to trigger the creation of a new nugget at next opportunity (only at max zoomed out)
  need_new_nugget = False

  # used to start new game, after game was won and bonus is displayed
  is_game_won = False

  # The factor to shrink the source image with, in order to make it fit on the screen.
  # This has to be larger than 1 (= source image has higher resolution than screen),
  # so the image looks still nice, if we zoom in a bit.
  source_image_scale = 3.0


  def __init__(self, gcomprisBoard):
    """ Constructor """

    print "mining init"

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True


  def start(self):
    """ Load data and start the activity """

    print "mining start"

    self.gcomprisBoard.maxlevel = 9
    self.gcomprisBoard.sublevel = 1
    self.gcomprisBoard.number_of_sublevel = 1

    # Set the buttons we want in the bar.
    # We need to set BAR_LEVEL, in order to see the "help" icon!?!
    gcompris.bar_set(gcompris.BAR_LEVEL)

    gcompris.bar_location(0, -1, 0.8)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())
    self.rootitem.connect("button_press_event", self.on_button_press)

    svghandle = gcompris.utils.load_svg("mining/rockwall.svgz")

    self.viewport = Viewport(self.gcomprisBoard, self.rootitem, self.on_zoom_change, self.source_image_scale)

    rockwall_img = goocanvas.Svg(
      parent = self.viewport.get_gc_group(),
      svg_handle = svghandle,
      svg_id = "#BACKGROUND"
      )

    # the further zoom logic highly depends on the following conditions
    assert(gcompris.BOARD_WIDTH == (rockwall_img.get_bounds().x2 / self.source_image_scale))
    assert(gcompris.BOARD_HEIGHT == (rockwall_img.get_bounds().y2 / self.source_image_scale))

    self.nugget = Nugget(svghandle, self.viewport.get_gc_group())

    # create sparkling last, so it is on above the nugget
    self.sparkling = Sparkling(svghandle, self.viewport.get_gc_group())

    # initialize the level, to start with
    self.set_level(1)


  def set_level(self, level):
    """ Initialize new level and start it. """

    print("mining set level: %i" % level)

    self.end_level()

    # store new level
    self.gcomprisBoard.level = level
    gcompris.bar_set_level(self.gcomprisBoard);

    # load level specific values
    self.nuggets_to_collect = 3

    # prepare new game
    self.is_game_won = False
    self.nugget_count = 0
    self.need_new_nugget = False

    # TODO: set level-function:
    #  generate nice level with random stones, bones, ...
    #  and place gold nuggest according to no stone/bone bounding boxes
    #  perhaps different backgrounds?

    self.viewport.reset()

    self.place_new_nugget()


  def place_new_nugget(self):
    """ Place a new nugget to collect on the rockwall """
    
    # TODO: check for stones/bones
    # TODO: nugget width / height

    margin = 50
    x = random.randrange(margin, gcompris.BOARD_WIDTH - margin) * self.source_image_scale
    y = random.randrange(margin, gcompris.BOARD_HEIGHT - margin) * self.source_image_scale

    self.nugget.reset(x, y)
    (x, y) = self.nugget.get_sparkling_coordinates()
    self.sparkling.reset(x, y)
    self.sparkling.animation_start()
    self.need_new_nugget = False


  def on_zoom_change(self, state):
    """ Do something according to specific zoom states (E.g. the nugget is only visible at maximum zoom.) """
    # As of 2012-08-11 there seems to be no "gcomrpis way" to change the mouse cursor to
    # a individual png. So we can't change to a pickaxe. :(

    if state == 'min':
      self.nugget.hide()

      if self.need_new_nugget:
        self.place_new_nugget()

    elif state == 'mid':
      self.nugget.hide()

    elif state == 'max':
      self.nugget.show()

    else:
      assert(False)


  def on_button_press(self, item, target_item, event = None):
    """
    The user clicked somewhere.

    Since the spark sometimes hides the nugget, click events might be misleaded to the spark-image.
    So we can't use "nugget.connect()". :(
    """

    # don't react on double clicks
    if event.type != gtk.gdk.BUTTON_PRESS:
      return

    # coordinate space fix
    # Event-coordinates are in the coordinate space of that item, which lies direct under the cursor.
    # To "normalize" them, we translate those coordinates to canvas coordinate space.
    (x, y) = self.gcomprisBoard.canvas.convert_from_item_space(target_item, event.x, event.y)

    if self.nugget.is_visible():
      # Was the mouse-click over the nugget?
      # An alternative would be:
      #    gcomprisBoard.canvas.get_items_at(x, y, True)
      # But we don't have the nugget_img object here.
      if self.nugget.is_hit(x, y):
        self.collect_nugget()


  def collect_nugget(self):
    """ The nugget was clicked, so collect it """

    self.nugget_count += 1
    self.sparkling.animation_stop()
    self.nugget.hide()

    if self.nugget_count >= self.nuggets_to_collect:
      self.on_level_won()
    else:
      # we need to collect more nuggets, so lets place a new one
      self.need_new_nugget = True


  def on_level_won(self):
    """ The user collected enough nuggets """

    self.is_game_won = True;

    # display the lion bonus (I have no idea, why it has the GNU-label)
    gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.GNU)


  def pause(self, pause):
    """ Called by GCompris, when the game is paused (e.g. config or bonus display) """

    # When the bonus is displayed, this function is called first with pause(1) and then with pause(0)
    if(pause == 0 and self.is_game_won):
      # the bonus has been shown, so start a new game in the next level
      self.set_level(self.get_next_level())


  def get_next_level(self):
    """ Determines the next level """

    next_level = self.gcomprisBoard.level + 1
    if(next_level > self.gcomprisBoard.maxlevel):
      next_level = self.gcomprisBoard.maxlevel

    return next_level


  def end_level(self):
    """ Terminate the current level """
    self.sparkling.end()


  def end(self):
    """ Terminate the activity """
    print "mining end"

    self.end_level()

    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  # -----------------------
  # GCompris interface functions, not needed by mining-activity

  def config_start(self, profile):
    print("config_start() not needed by mining-activity.")

  def config_stop(self):
    print("config_stop() not needed by mining-activity.")

  def key_press(self, keyval, commit_str, preedit_str):
    print("key_press() not needed by mining-activity.")

  def ok(self):
    print("ok() not needed by mining-activity.")

  def repeat(self):
    print("repeat() not needed by mining-activity.")



class Viewport:
  """ The viewport handles zooming in and out with the appropriate translation """

  # viewport transformation
  x = 0
  y = 0
  scale = 1.0


  # zooming

  # The limit to max zoom out, while still filling all the screen with the rockwall.
  # This value is set up in the constructor.
  scale_min = None

  # The limit to max zoom in.
  # Try to keep scale_max reachable by  scale_min * zoom_factor ^ n  (with n in [1, 2, 3, 4, ...[)
  scale_max = 0.9520

  # the factor to zoom on each zoom event
  zoom_factor = 1.3


  # The GooCanvas group, which holds everything that is affected by zooming
  gc_group = None

  # see documentation in Gcompris_mining
  source_image_scale = None


  def __init__(self, gcomprisBoard, parent, cb_zoom_change, source_image_scale):
    """
    Constructor:
      gcomprisBoard      : the GCompris board
      parent             : the parent GooCanvas item to add our gc_group
      cb_zoom_change     : callback function for "zoom changed"-event
      source_image_scale : see documentation in Gcompris_mining
    """
    self.gcomprisBoard = gcomprisBoard
    self.gc_group = goocanvas.Group(parent = parent)
    self.gc_group.connect("scroll_event", self.__on_scroll)
    self.cb_zoom_change = cb_zoom_change
    self.source_image_scale = source_image_scale
    self.scale_min = 1.0 / source_image_scale


  def reset(self):
    """ Reset the viewport """

    # zoom out max
    self.scale = self.scale_min
    self.x = 0
    self.y = 0

    self.__update_transformation()


  def get_gc_group(self):
    """ get the GooCanvas group, which holds everything that is affected by zooming """
    return self.gc_group


  def __on_scroll(self, item, target_item, event = None):
    """ Called at scroll (= zoom) events """

    # item - The element connected with this callback function
    # target_item - The element under the cursor
    # event  - gtk.gdk.Event

    assert(event.type == gtk.gdk.SCROLL)


    ###
    # coordinate space fix

    # Event-coordinates are in the coordinate space of that item, which lies direct under the cursor.
    # So if the user scrolls over the spark, for example, we have to translate those coordinates to 
    # the coordinate space of our gc_group.
    (x, y) = self.gcomprisBoard.canvas.convert_from_item_space(target_item, event.x, event.y)
    (x, y) = self.gcomprisBoard.canvas.convert_to_item_space(self.gc_group, x, y)


    ###
    # remember old values for calculation

    old_scale = self.scale
    old_x = self.x
    old_y = self.y


    ###
    # calculate the new scale factor

    if event.direction == gtk.gdk.SCROLL_DOWN:
      # zoom out
      self.scale /= self.zoom_factor
      if self.scale < self.scale_min:
        self.scale = self.scale_min
    elif event.direction == gtk.gdk.SCROLL_UP:
      # zoom in
      self.scale *= self.zoom_factor
      if self.scale > self.scale_max:
        self.scale = self.scale_max
    else:
      assert(False)


    ###
    # The pixel under the cursor shall be fixed, while everything else zooms in/out 
    # (we zoom at the mouse position), so we have to calculate a new position for 
    # gc_group (displaying primarily the rockwall image).

    new_x = old_x + x * (old_scale - self.scale)
    new_y = old_y + y * (old_scale - self.scale)


    ###
    # Checks to prevent empty areas on any side of the rockwall

    min_x = gcompris.BOARD_WIDTH * (1 - self.scale * self.source_image_scale)
    min_y = gcompris.BOARD_HEIGHT * (1 - self.scale * self.source_image_scale)

    # block uncovered area on the left
    if new_x > 0:
      new_x = 0

    # block uncovered area on the top
    if new_y > 0:
      new_y = 0

    # block uncovered area on the right
    if new_x < min_x:
      new_x = min_x

    # block uncovered area on the bottom
    if new_y < min_y:
      new_y = min_y


    ###
    # apply new values

    self.x = new_x
    self.y = new_y

    self.__update_transformation()


    ###
    # inform main class about zoom change
    
    if old_scale != self.scale:
      if self.scale == self.scale_max:
        state = 'max'
      elif self.scale == self.scale_min:
        state = 'min'
      else:
        state = 'mid'

      # call the callback function of main class
      self.cb_zoom_change(state)


  def __update_transformation(self):
    """ Update the viewport's transformation matrix """
    
    self.gc_group.set_simple_transform(self.x, self.y, self.scale, 0)



class Nugget:
  """ The gold nugget """

  # position of the nugget (in the rockwall/gc_group)
  x = 0.0
  y = 0.0

  # center of the spark in the svg file (used for rotation and positioning)
  pivot_x = 1000
  pivot_y = 800

  # picture of the nugget
  nugget_img = None


  def __init__(self, svghandle, parent):
    """ 
    Constructor:
      svghandle      : handle to the svg file, holding the pictures
      parent         : GooCanvas parent item of the gold nugget
    """

    self.nugget_img = goocanvas.Svg(
      parent = parent,
      svg_handle = svghandle,
      svg_id = "#NUGGET",

      # start invisible, since x/y are not set properly yet
      visibility = goocanvas.ITEM_INVISIBLE
      )


  def reset(self, x, y):
    """ Move nugget to a new position. """
    self.x = x
    self.y = y
    self.nugget_img.set_simple_transform(self.x - self.pivot_x, self.y - self.pivot_y, 1.0, 0)


  def show(self):
    """ Display the nugget """
    self.nugget_img.props.visibility = goocanvas.ITEM_VISIBLE


  def hide(self):
    """ Hide the nugget """
    self.nugget_img.props.visibility = goocanvas.ITEM_INVISIBLE


  def is_visible(self):
    """ Tells whether the nugget is visible (True) or hidden (False) """
    return self.nugget_img.props.visibility == goocanvas.ITEM_VISIBLE


  def is_hit(self, x, y):
    """ 
    Determines whether the given coordinates are in the nugget's bounding box.
      x and y are the coordinates in the canvas coordinate space. 
    """

    # get the nugget's bounding box in canvas coordinate space
    bounds = self.nugget_img.get_bounds()

    if bounds.x1 <= x and x <= bounds.x2 and bounds.y1 <= y and y <= bounds.y2:
      return True

    return False


  def get_sparkling_coordinates(self):
    """ Get the coordinates, where the sparkling should appear """
    return (self.x - 10, self.y - 10)


class Sparkling:
  """
  Handles all "sparkling"-stuff of the gold nugget.

  The sparkling animation consists of "sparkphases", the time when the spark can be seen,
  and pauses, the time when the spark is hidden.
  """

  # position of the spark (in the rockwall/gc_group)
  x = 0.0
  y = 0.0

  # rotation (in degrees)
  angle = 0
  rot_delta = 0
  rot_delta_init = 6

  # size
  scale = 0
  scale_factor = 0.90
  scale_min = 0.4
  scale_max = 1.0

  # animation
  timer = None
  timer_milliseconds = 30

  # the number of timer-events, a pause lasts
  pause_ticks_total = 25
  pause_ticks_current = 0
  pause_tick_variation = 10

  # spark image
  spark = None

  # center of the spark in the svg file (used for rotation and positioning)
  pivot_x = 600
  pivot_y = 600


  def __init__(self, svghandle, parent):
    """ 
    Constructor:
      svghandle : handle to the svg file, holding the pictures
      parent    : GooCanvas parent item of this spark
    """

    self.spark = goocanvas.Svg(
      parent = parent,
      svg_handle = svghandle,
      svg_id = "#SPARK",

      # start invisible, since x/y are not set properly yet
      visibility = goocanvas.ITEM_INVISIBLE
      )


  def end(self):
    """ Our "destructor" """
    if self.__is_animation_playing():
      self.animation_stop()


  def reset(self, x, y):
    """ Prepare a new sparkling animation. """

    self.x = x
    self.y = y

    self.pause_ticks_current = 0

    # we turn rotation direction at every sparkphase
    self.rot_delta = self.rot_delta_init

    self.__sparkphase_start()


  def __update_transformation(self):
    """ Updates the transformation matrix of the spark (= calculate new picture of animation). """

    # we need those values more than once, so lets remember them
    a = math.radians(self.angle)
    cos_a = math.cos(a)
    sin_a = math.sin(a)

    # create the transformation matrices
    m_center_to_origin = cairo.Matrix(1, 0, 0, 1, -self.pivot_x, -self.pivot_y)
    m_scale = cairo.Matrix(self.scale, 0, 0, self.scale, 0, 0)
    m_rotate = cairo.Matrix(cos_a, sin_a, -sin_a, cos_a, 0, 0)
    m_to_destination = cairo.Matrix(1, 0, 0, 1, self.x, self.y)

    # combine all transformation matrices to one
    matrix = m_center_to_origin * m_scale * m_rotate * m_to_destination

    self.spark.set_transform(matrix)


  def __show_spark(self):
    """ Display the spark """
    self.spark.props.visibility = goocanvas.ITEM_VISIBLE


  def __hide_spark(self):
    """ Hide the spark """
    self.spark.props.visibility = goocanvas.ITEM_INVISIBLE


  def __sparkphase_start(self):
    """ Start a new sparkphase, showing the spark """

    # setup new values for this sparkphase
    self.angle += random.randrange(0, 359)
    self.rot_delta *= -1
    self.scale = self.scale_max

    # show the spark at the new state
    self.__update_transformation()
    self.__show_spark()

    # make some noise ;)
    #TODO: gcompris.sound.play_ogg("sounds/sparkle.ogg")


  def __calculate_new_state(self):
    """ Calculate next animation step """
    self.scale *= self.scale_factor
    self.angle += self.rot_delta


  def __animate(self):
    """ Called every x milliseconds, to animate the sparkling """

    if self.pause_ticks_current > 0:
      # pause this tick
      self.pause_ticks_current -= 1

      if self.pause_ticks_current <= 0:
        # pause ends
        self.__sparkphase_start()

      return True

    # no pause

    if self.scale < self.scale_min:
      # start pause
      self.pause_ticks_current = self.pause_ticks_total + random.randrange(-self.pause_tick_variation, self.pause_tick_variation)
      self.__hide_spark()
      return True

    # normal sparkle animation
    self.__calculate_new_state()
    self.__update_transformation()

    # call timeout again
    return True


  def animation_start(self):
    """ Starts the sparkling animation """
    assert(not self.__is_animation_playing())

    self.timer = gobject.timeout_add(self.timer_milliseconds, self.__animate)


  def animation_stop(self):
    """ Stops the sparkling animation """
    assert(self.__is_animation_playing())

    gobject.source_remove(self.timer)
    self.timer = None

    self.__hide_spark()


  def __is_animation_playing(self):
    """ Tells us, if there is an animation running at the moment """
    return self.timer != None

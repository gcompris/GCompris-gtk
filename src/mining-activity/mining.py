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

  # the number of nuggets, we already have collected
  nugget_count = 0

  # used to trigger the creation of a new nugget at next opportunity (only at max zoomed out)
  need_new_nugget = False

  # used to start new game, after game was won and bonus is displayed
  is_game_won = False

  # used to avoid input (like scrolling) during game pause
  __is_game_paused = False

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

    # Setup a nugget-blocking-area for the GCompris bar, to avoid placing the nugget behind
    # the bar.
    # The bar is in the lower, left corner of the screen with dimensions: width = 196; height = 50
    self.gc_bar_blocker = BlockingArea(0, 470, 196, 520)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automatically.
    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())
    self.rootitem.connect("button_press_event", self.on_button_press)

    svghandle = gcompris.utils.load_svg("mining/rockwall.svgz")

    self.viewport = Viewport(self, self.rootitem)

    rockwall_img = goocanvas.Svg(
      parent = self.viewport.get_gc_group(),
      svg_handle = svghandle,
      svg_id = "#BACKGROUND"
      )

    # the further zoom logic highly depends on the following conditions
    assert(gcompris.BOARD_WIDTH == (rockwall_img.get_bounds().x2 / self.source_image_scale))
    assert(gcompris.BOARD_HEIGHT == (rockwall_img.get_bounds().y2 / self.source_image_scale))

    self.lorry = Lorry(svghandle, self.rootitem)

    self.placer = Placer(self)

    self.decorations = Decorations(svghandle, self.viewport.get_gc_group(), self.placer)

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
    #  perhaps different backgrounds?

    self.update_lorry()
    self.viewport.reset()

    self.placer.add_blocker(self.gc_bar_blocker)
    self.placer.add_blocker(self.lorry)
    for blocking_area in self.viewport.get_nugget_blocker():
      self.placer.add_blocker(blocking_area)

    self.decorations.decorate_viewport(10)

    self.place_new_nugget()


  def place_new_nugget(self):
    """ Place a new nugget to collect on the rockwall """

    self.placer.place(self.nugget, self.nugget.reset)

    (x, y) = self.nugget.get_sparkling_coordinates()
    self.sparkling.reset(x, y)
    self.sparkling.animation_start()
    self.need_new_nugget = False

    # The following sound was copied form "Tuxpaint" (GPL)
    gcompris.sound.play_ogg("mining/realrainbow.ogg")


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

    # ignore input while game paused
    if self.is_game_paused():
      return

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

    self.placer.remove_blocker(self.nugget)
    self.nugget_count += 1
    self.sparkling.animation_stop()
    self.nugget.hide()

    self.update_lorry()

    if self.nugget_count >= self.nuggets_to_collect:
      self.on_level_won()
    else:
      # The following sound is based on "metalpaint.wav" from "Tuxpaint" (GPL)
      gcompris.sound.play_ogg("mining/pickaxe.ogg")

      # we need to collect more nuggets, so lets place a new one
      self.need_new_nugget = True


  def update_lorry(self):
    """ Updates the nugget-collect-counter of the lorry in the lower right corner """
    self.lorry.update_text(str(self.nugget_count) + "/" + str(self.nuggets_to_collect))


  def on_level_won(self):
    """ The user collected enough nuggets """
    self.is_game_won = True;
    gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.LION)


  def pause(self, pause):
    """ Called by GCompris, when the game is paused (e.g. config or bonus display) """

    # When the bonus is displayed, this function is called first with pause(1) and then with pause(0)
    if pause == 0:
      # pause finished
      self.__is_game_paused = False

      if self.is_game_won:
        # the bonus has been shown, so start a new game in the next level
        self.set_level(self.get_next_level())

    else:
      # pause started
      self.__is_game_paused = True


  def is_game_paused(self):
    """ Determine if the game is currently paused """
    return self.__is_game_paused


  def get_next_level(self):
    """ Determines the next level """

    next_level = self.gcomprisBoard.level + 1
    if(next_level > self.gcomprisBoard.maxlevel):
      next_level = self.gcomprisBoard.maxlevel

    return next_level


  def end_level(self):
    """ Terminate the current level """
    self.sparkling.end()
    self.placer.remove_all_blocker()
    self.decorations.cleanup_viewport()


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



class Lorry:
  """ The lorry in the lower right corner of the screen, showing the number of collected nuggets """

  # position of the lorry (in screen coordinates (800/520))
  x = 730.0
  y = 470.0

  # center of the lorry in the svg file (used for rotation and positioning)
  pivot_x = 2220
  pivot_y = 1432

  # the svg file is scaled with a factor of 3 (to enable nice zoom in) so we
  # have to correct this for the lorry, which is not affected by zooming.
  # Plus: the lorry designed in the svg file is a little bit to small for the
  # text in default font to fit in.
  scale = 0.4


  def __init__(self, svghandle, rootitem):
    """
    Constructor:
      svghandle          : handle of the svg file, containing graphics data
      rootitem           : the root item to attach goo-object to
    """

    self.lorry_img = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#LORRY",
      )

    self.text = goocanvas.Text(
      parent = rootitem,
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x = self.x + 9,
      y = self.y + 2,
      anchor = gtk.ANCHOR_CENTER,
      fill_color = "white",
      text = "-/-"
      )

    self.__update_transformation()


  def get_bounds(self):
    """ Get the bounds of the lorry image on the canvas """
    return self.lorry_img.get_bounds()


  def update_text(self, text):
    """ Set a new text for the lorry to display """
    self.text.set_properties(text = text)


  def __update_transformation(self):
    """ Updates the transformation matrix of the lorry """

    m_center_to_origin = cairo.Matrix(1, 0, 0, 1, -self.pivot_x, -self.pivot_y)
    m_scale = cairo.Matrix(self.scale, 0, 0, self.scale, 0, 0)
    m_to_destination = cairo.Matrix(1, 0, 0, 1, self.x, self.y)

    # combine all transformation matrices to one
    matrix = m_center_to_origin * m_scale * m_to_destination

    self.lorry_img.set_transform(matrix)



class Placer:
  """ This class randomly places items on the screen and assures, that they do not overlap """

  # the internal list of blocking areas
  blocking_areas = []


  def __init__(self, activity):
    """
    Constructor:
      activity : the main activity class
    """
    self.activity = activity


  def place(self, item, place_callback):
    """
    Place "item" on the screen.
      item            : the item to place (needs to implement "get_bounds()")
      place_callback  : The callback function to actually place the item (and modify, like rotate/scale).
                        Is called with parameters (item, x, y).
    """
    area = Area(item.get_bounds())
    width_half = int(area.width / 2.0)
    height_half = int(area.height / 2.0)

    safety_counter = 0

    while True:
      # get new random position for the item
      x = random.randrange(width_half, gcompris.BOARD_WIDTH - width_half) * self.activity.source_image_scale
      y = random.randrange(height_half, gcompris.BOARD_HEIGHT - height_half) * self.activity.source_image_scale

      # place the item to this position
      place_callback(item, x, y)

      # check for overlapping objects
      if not self.__does_object_collide_with_registered_ones(item):
        # we found a valid position without collisions
        # lets remember the positioned item...
        self.add_blocker(item);
        # ... and end the search for a valid position
        break

      safety_counter += 1
      if safety_counter > 20:
        # We tried to place this object many times, but could not find a valid position.
        # Seems to be very difficult with this amount of objects.
        # Since an invalid position (= overlapping objects) is way better than a frozen
        # application, we exit this while loop with an "invalid" position.
        print("Warning: safety_counter reached maximum!")
        self.add_blocker(item);
        break


  def add_blocker(self, blocking_area):
    """ Add a new blocking area to the internal list of blocking areas """
    self.blocking_areas.append(blocking_area)


  def remove_blocker(self, blocking_area):
    """ Removes the given blocking area from the internal list of blocking areas """
    for i in range(len(self.blocking_areas)):
      if self.blocking_areas[i] == blocking_area:
        # this is the blocking_area, we are looking for, so remove it from the list
        self.blocking_areas.pop(i)
        return

    print("Warning: blocking-area not in list: " + str(blocking_area))


  def remove_all_blocker(self):
    """ Removes all blocker from the internal list """
    self.blocking_areas = []


  def __does_object_collide_with_registered_ones(self, asked_object):
    """ Tests whether any registered object collides with the asked_object """
    area_a = Area(asked_object.get_bounds())

    for blocking_area in self.blocking_areas:
      area_r = Area(blocking_area.get_bounds())

      # collision on x-axis?
      if math.fabs(area_a.center_x - area_r.center_x) <= (area_r.width + area_a.width) / 2.0:
        # collision on y-axis?
        if math.fabs(area_a.center_y - area_r.center_y) <= (area_r.height + area_a.height) / 2.0:
          # collision!
          return True

    # no collision
    return False



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


  def __init__(self, activity, parent):
    """
    Constructor:
      activity           : the main activity object
      parent             : the parent GooCanvas item to add our gc_group
    """
    self.gcomprisBoard = activity.gcomprisBoard
    self.gc_group = goocanvas.Group(parent = parent)
    self.gc_group.connect("scroll_event", self.__on_scroll)
    self.cb_zoom_change = activity.on_zoom_change
    self.source_image_scale = activity.source_image_scale
    self.is_game_paused = activity.is_game_paused
    self.scale_min = 1.0 / self.source_image_scale

    self.nugget_blocker = (
      BlockingArea(0, 0, 800, 42), # top
      BlockingArea(758, 0, 800, 520), # right
      BlockingArea(0, 510, 800, 520), # bottom
      BlockingArea(0, 0, 42, 520)     # left
    )


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


  def get_nugget_blocker(self):
    """ Get the viewport's blocking areas, where no nugget should be placed """
    return self.nugget_blocker


  def __on_scroll(self, item, target_item, event = None):
    """ Called at scroll (= zoom) events """

    # item - The element connected with this callback function
    # target_item - The element under the cursor
    # event  - gtk.gdk.Event

    # ignore input while game paused
    if self.is_game_paused():
      return

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



class Decorations:
  """
  This class handles decorations, like stones. They have the meaning of:
    - make every level look a bit different
    - help orienting, while scrolling
  """

  # our decoration types in the svg file
  decoration_types = (
    {
      'svg_id': '#STONE1',
      'svg_x': 500,
      'svg_y': 1300
    },
    {
      'svg_id': '#STONE2',
      'svg_x': 1000,
      'svg_y': 1300
    },
    {
      'svg_id': '#STONE3',
      'svg_x': 1500,
      'svg_y': 1300
    },
    {
      'svg_id': '#STONE4',
      'svg_x': 2000,
      'svg_y': 1300
    },
  )

  # A goocanvas group, that holds all our decoration, so we can easily
  # remove them, by removing only this group.
  decoration_group = None

  # ID of the decoration type, currently being placed. (Used to overcome callback bounderies)
  current_decoration_id = None


  def __init__(self, svghandle, gc_group, placer):
    """
    Constructor:
      - svghandle : handle to the svg file, to load the decoration pictures
      - gc_group  : our viewport's gc_group, the decorations are attached to
      - placer    : reference to the Placer object
    """
    self.svghandle = svghandle
    self.viewport_gc_group = gc_group
    self.placer = placer


  def decorate_viewport(self, number_of_decorations):
    """ Fill the viewport with some decorations """

    assert(self.decoration_group == None)
    self.decoration_group = goocanvas.Group(parent = self.viewport_gc_group)

    for i in range(number_of_decorations):
      # select random decoration
      self.current_decoration_id = random.randrange(4)
      svg_id = self.decoration_types[self.current_decoration_id]['svg_id']

      # create decoration
      decoration = goocanvas.Svg(
        parent = self.decoration_group,
        svg_handle = self.svghandle,
        svg_id = svg_id,
        )

      self.placer.place(decoration, self.__place_decoration)


  def __place_decoration(self, decoration, x, y):
    """ Updates the transformation of the decoration to the new coordinates. Rotation and scale are varied. """

    svg_x = self.decoration_types[self.current_decoration_id]['svg_x']
    svg_y = self.decoration_types[self.current_decoration_id]['svg_y']


    # scale between 0.5 and 2.0
    scale = math.pow(1.2, random.randrange(-4, +4))

    rot = random.randrange(-20, +20)


    # we need those values more than once, so lets remember them
    a = math.radians(rot)
    cos_a = math.cos(a)
    sin_a = math.sin(a)

    # create the transformation matrices
    m_center_to_origin = cairo.Matrix(1, 0, 0, 1, -svg_x, -svg_y)
    m_scale = cairo.Matrix(scale, 0, 0, scale, 0, 0)
    m_rotate = cairo.Matrix(cos_a, sin_a, -sin_a, cos_a, 0, 0)
    m_to_destination = cairo.Matrix(1, 0, 0, 1, x, y)

    # combine all transformation matrices to one
    matrix = m_center_to_origin * m_scale * m_rotate * m_to_destination

    decoration.set_transform(matrix)


  def cleanup_viewport(self):
    """ Remove all decorations from the viewport """
    if self.decoration_group != None:
      self.decoration_group.remove()
      self.decoration_group = None



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


  def reset(self, nugget, x, y):
    """
    Move nugget to a new position.
      nugget : we don't need it in this case, since there is only one nugget image in this class,
                but the callback interface defines this parameter
      x      : new x position for the nugget image
      y      : new y position for the nugget image
    """
    self.x = x
    self.y = y
    self.nugget_img.set_simple_transform(self.x - self.pivot_x, self.y - self.pivot_y, 1.0, 0)


  def get_bounds(self):
    """ Get the bounds of the nugget image on the canvas """
    return self.nugget_img.get_bounds()


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


class Area:
  """ A class for comfortable working with goocanvas.Bounds """

  def __init__(self, bounds):
    self.x1 = bounds.x1
    self.x2 = bounds.x2
    self.y1 = bounds.y1
    self.y2 = bounds.y2

    self.width = self.x2 - self.x1
    self.height = self.y2 - self.y1

    self.center_x = self.x1 + self.width / 2.0
    self.center_y = self.y1 + self.height / 2.0


class BlockingArea:
  """ This class defines a blocking area, where no nugget should be put """

  def __init__(self, x1, y1, x2, y2):
    """
    Constructor:
      x1, y1, x2, y2 : bounding box of the blocker in canvas coordinates (0 - 800 / 0 - 520)
    """
    self.bounds = goocanvas.Bounds(x1, y1, x2, y2)

  def get_bounds(self):
    """ Return the bounds, defined in the constructor """
    return self.bounds

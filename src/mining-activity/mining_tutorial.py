#  gcompris - mining_tutorial.py
#
# Copyright (C) 2012 Peter Albrecht
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

import math
import gobject
import gcompris
import goocanvas
import cairo

from mining_tools import BlockingArea


class MiningTutorial:
  """ This class provides tutorial information to the user """

  # the current tutorial state
  current_state = None


  def __init__(self, rootitem):
    """
    Constructor
      rootitem: the root item, to add tutorial images to
      (loads graphics)
    """

    # we create an own GooCanvas group to add tutorial items, so we can
    # remove them easier
    self.tutorial_rootitem = goocanvas.Group(parent = rootitem)

    svghandle = gcompris.utils.load_svg("mining/tutorial.svgz")

    self.mouse = TutorialMouse(self.tutorial_rootitem, svghandle, 300, 440)
    self.cursor = TutorialCursor(self.tutorial_rootitem, svghandle)
    self.touchpad = TutorialTouchpad(self.tutorial_rootitem, svghandle, 500, 440)


  def get_blocking_area(self):
    """
    Returns the blocking area for the tutorial, to avoid placing the nugget behind
    the tutorial mouse or the touchpad.
    """
    return BlockingArea(250, 360, 640, 520)


  def start(self):
    """ Start the tutorial (display graphics) """
    self.current_state = 'start'


  def get_tutorial_state(self):
    """ Returns the current tutorial state """
    return self.current_state


  def set_tutorial_state(self, state, *special_state_arguments):
    """
    Set the current state of the tutorial
      state: the state to set, valid values:
        - 'move to'
        - 'zoom in'
        - 'click'
        - 'zoom out'
      special_state_arguments: special arguments for the particular state
    """

    if state == self.current_state:
      return


    # advance to next state, if current state matches
    if state == 'move to':
      if self.current_state == 'start':
        self.cursor.start(*special_state_arguments)

        self.current_state = state

    elif state == 'zoom in':
      if self.current_state == 'move to':
        self.cursor.stop()

        self.mouse.start_zoom('in')
        self.touchpad.start_zoom('in')

        self.current_state = state

    elif state == 'click':
      if self.current_state == 'zoom in':
        self.mouse.stop()
        self.touchpad.stop()

        self.mouse.start_click()
        self.touchpad.start_click()

        self.current_state = state

    elif state == 'zoom out':
      if self.current_state == 'click':
        self.mouse.stop()
        self.touchpad.stop()

        self.mouse.start_zoom('out')
        self.touchpad.start_zoom('out')

        self.current_state = state

    else:
      # invalid state
      assert(False)


  def restart_tutorial_step(self, *special_state_arguments):
    """
    Restarts the current tutorial step
      special_state_arguments: special arguments for the particular state
    """
    if self.current_state == 'move to':
      self.cursor.stop()
      self.cursor.start(*special_state_arguments)

    else:
      # restart not supported
      assert(False)


  def stop(self):
    """ Stop the tutorial (hide graphics) """
    self.cursor.stop()
    self.mouse.stop()
    self.touchpad.stop()


  def end(self):
    """ Our "destructor" (remove graphics) """
    self.cursor.end()
    self.mouse.end()
    self.touchpad.end()

    # remove all tutorial graphics from the canvas
    self.tutorial_rootitem.remove()



class TutorialCursor:
  """ This class demonstrates to move the cursor to a specific position """

  # position of the center of the cursor target in screen coordinates (800 x 520)
  circle_x = 0.0
  circle_y = 0.0

  # position of the center of the ghost mouse cursor in screen coordinates (800 x 520)
  cursor_x = 0.0
  cursor_y = 0.0

  # center of the cursor target in the svg file
  pivot_circle_x = 400.0
  pivot_circle_y = 370.0

  # center of the mouse cursor in the svg file
  pivot_cursor_x = 200.0
  pivot_cursor_y = 370.0

  # the number of milliseconds the total "move cursor to" animation lasts
  animation_time_total = 3000

  # the time between each animation step, in milliseconds
  animation_time_step = 100


  def __init__(self, rootitem, svghandle):
    """
    Constructor
        rootitem    : The goocanvas item to add the mouse to
        svghandle   : Handle of the svg file, containing the graphics
    """

    self.circle_img = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#CURSOR_TARGET",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      )

    self.cursor_img = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#MOUSE_CURSOR",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      )


  def start(self, cx, cy, tx, ty):
    """
    Start the cursor move tutorial: show graphics
      cx, cy: x-, y-position of the mouse cursor
      tx, ty: x-, y-position of the target
    """

    self.circle_x = tx
    self.circle_y = ty

    self.cursor_x = cx
    self.cursor_y = cy

    self.circle_img.props.visibility = goocanvas.ITEM_VISIBLE
    self.cursor_img.props.visibility = goocanvas.ITEM_VISIBLE

    self.__update_circle_transformation()
    self.__update_cursor_transformation()

    self.__animation_start()


  def __update_circle_transformation(self):
    """ Updates the transformation matrix of the cursor target """

    matrix = cairo.Matrix(1, 0, 0, 1, self.circle_x - self.pivot_circle_x, self.circle_y - self.pivot_circle_y)
    self.circle_img.set_transform(matrix)


  def __update_cursor_transformation(self):
    """ Updates the transformation matrix of the ghost mouse cursor """

    matrix = cairo.Matrix(1, 0, 0, 1, self.cursor_x - self.pivot_cursor_x, self.cursor_y - self.pivot_cursor_y)
    self.cursor_img.set_transform(matrix)


  def __animation_start(self):
    # start moving the ghost mouse cursor to the sparkle
    self.cursor_img.animate(
      self.circle_x - self.cursor_x, # destination x position in root coordinates (800x520)
      self.circle_y - self.cursor_y, # destination y position in root coordinates (800x520)
      1, 0, # no scale and no rotation
      False, # destination position is relative
      self.animation_time_total, # see variable definition
      self.animation_time_step, # see variable definition
      goocanvas.ANIMATE_RESTART        # loop the animation
      )


  def stop(self):
    """ Stop the cursor move tutorial: hide graphics """

    self.cursor_img.stop_animation()

    self.circle_img.props.visibility = goocanvas.ITEM_INVISIBLE
    self.cursor_img.props.visibility = goocanvas.ITEM_INVISIBLE


  def end(self):
    """ Our "destructor" """
    pass



class TutorialMouse:
  """ Displays a mouse with mouse wheel animation """

  # time period between two animation steps
  timer_scroll_milliseconds = 300
  timer_click_milliseconds = 800

  # timer
  timer_scroll = None
  timer_click = None

  # position of the center of the mouse in screen coordinates (800 x 520)
  x = 0.0
  y = 0.0

  # center of the mouse in the svg file
  pivot_mouse_x = 200
  pivot_mouse_y = 220

  # position of the mouse wheel center relative to the mouse center
  center_mouse_to_center_wheel_x = 0
  center_mouse_to_center_wheel_y = -14.2

  # center of the mouse wheel pivots in the svg file
  pivot_wheels = (
    {
      'pivot_x': 100,
      'pivot_y': 220
      },
    {
      'pivot_x': 120,
      'pivot_y': 220
      },
    {
      'pivot_x': 140,
      'pivot_y': 220
      }
    )

  # center of the mouse button in the svg file
  pivot_button_x = 120
  pivot_button_y = 270

  # position of the mouse button center relative to the mouse center
  center_mouse_to_center_button_x = -16
  center_mouse_to_center_button_y = -12

  # the current wheel displayed
  current_wheel = None

  # define the scroll direction:
  #   +1: zoom in
  #   -1: zoom out
  scroll_direction = None

  # the number of different wheels
  number_of_wheels = 3

  # list of mouse wheel images
  wheel_imgs = []


  def __init__(self, rootitem, svghandle, x, y):
    """
    Constructor
        rootitem    : The goocanvas item to add the mouse to
        svghandle   : Handle of the svg file, containing the graphics
        x, y        : Position of the center of the mouse in screen coordinates (800 x 520)
    """

    self.x = x
    self.y = y

    self.mouse_img = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#MOUSE",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      )

    self.button_img = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#MOUSE_BUTTON",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      )

    # GooCanvas does not support to add SVG-items to other SVG-items, so we have to add
    # the wheels to the rootitem.

    self.wheel_imgs.append(goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#MOUSEWHEEL01",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      ))

    self.wheel_imgs.append(goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#MOUSEWHEEL02",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      ))

    self.wheel_imgs.append(goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#MOUSEWHEEL03",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      ))

    self.current_wheel = 0
    self.__update_transformation()


  def start_zoom(self, zoom_direction):
    """
    Start the mouse zoom tutorial: show graphics, start animation
      zoom_direction: "in" or "out"
    """

    self.mouse_img.props.visibility = goocanvas.ITEM_VISIBLE

    self.current_wheel = 0
    self.__display_current_wheel()

    if zoom_direction == 'in':
      self.scroll_animation_start(+1)

    elif zoom_direction == 'out':
      self.scroll_animation_start(-1)

    else:
      # invalid zoom direction
      assert(False)


  def start_click(self):
    """ Start the mouse click tutorial: show graphics, start animation """

    self.mouse_img.props.visibility = goocanvas.ITEM_VISIBLE

    self.current_wheel = 0
    self.__display_current_wheel()

    self.click_animation_start()


  def stop(self):
    """ Stop the mouse tutorial: hide graphics, stop animation """

    if self.__is_scroll_animation_playing():
      self.scroll_animation_stop()

    if self.__is_click_animation_playing():
      self.click_animation_stop()

    self.button_img.props.visibility = goocanvas.ITEM_INVISIBLE
    self.mouse_img.props.visibility = goocanvas.ITEM_INVISIBLE
    self.__hide_current_wheel()


  def end(self):
    """ Our "destructor" """
    pass


  def __display_current_wheel(self):
    """ Displays the current mouse wheel """
    self.wheel_imgs[self.current_wheel].props.visibility = goocanvas.ITEM_VISIBLE


  def __hide_current_wheel(self):
    """ Hides the current mouse wheel """
    self.wheel_imgs[self.current_wheel].props.visibility = goocanvas.ITEM_INVISIBLE


  def __display_next_wheel(self):
    """ Displays the next mouse wheel on top of the mouse """

    # hide the old one
    self.__hide_current_wheel()

    # determine next mouse wheel to display
    self.current_wheel += self.scroll_direction
    if self.current_wheel >= self.number_of_wheels:
      self.current_wheel = 0
    elif self.current_wheel < 0:
      self.current_wheel = self.number_of_wheels - 1

    # show the new one
    self.__display_current_wheel()


  def __update_transformation(self):
    """ Updates the transformation matrix of the mouse, all wheels and the button """

    matrix = cairo.Matrix(1, 0, 0, 1, self.x - self.pivot_mouse_x, self.y - self.pivot_mouse_y)
    self.mouse_img.set_transform(matrix)

    for i in range(self.number_of_wheels):
      wheel = self.pivot_wheels[i]
      matrix = cairo.Matrix(1, 0, 0, 1,
        self.x - wheel['pivot_x'] + self.center_mouse_to_center_wheel_x,
        self.y - wheel['pivot_y'] + self.center_mouse_to_center_wheel_y
        )
      self.wheel_imgs[i].set_transform(matrix)

    matrix = cairo.Matrix(1, 0, 0, 1,
      self.x - self.pivot_button_x + self.center_mouse_to_center_button_x,
      self.y - self.pivot_button_y + self.center_mouse_to_center_button_y
      )
    self.button_img.set_transform(matrix)


  def __scroll_animate(self):
    """ Displays the next scrolling animation step """

    self.__display_next_wheel()

    # call timeout again
    return True


  def __is_scroll_animation_playing(self):
    """ Tells us, if there is a scroll animation running at the moment """
    return self.timer_scroll != None


  def scroll_animation_start(self, scroll_direction):
    """
    Starts the mouse wheel scroll animation
      scroll_direction:
         +1: zoom in
         -1: zoom out
    """
    assert(not self.__is_scroll_animation_playing())

    self.scroll_direction = scroll_direction

    self.timer_scroll = gobject.timeout_add(self.timer_scroll_milliseconds, self.__scroll_animate)


  def scroll_animation_stop(self):
    """ Stops the mouse wheel scroll animation """
    assert(self.__is_scroll_animation_playing())

    gobject.source_remove(self.timer_scroll)
    self.timer_scroll = None


  def __click_animate(self):
    """ Displays the click animation step """

    if self.click_visible:
      # switch to invisible
      self.click_visible = False
      self.button_img.props.visibility = goocanvas.ITEM_INVISIBLE

    else:
      # switch to visible
      self.click_visible = True
      self.button_img.props.visibility = goocanvas.ITEM_VISIBLE

    # call timeout again
    return True


  def __is_click_animation_playing(self):
    """ Tells us, if there is a click animation running at the moment """
    return self.timer_click != None


  def click_animation_start(self):
    """ Starts the mouse click animation """
    assert(not self.__is_click_animation_playing())
    self.click_visible = False
    self.timer_click = gobject.timeout_add(self.timer_click_milliseconds, self.__click_animate)


  def click_animation_stop(self):
    """ Stops the mouse click animation """
    assert(self.__is_click_animation_playing())

    gobject.source_remove(self.timer_click)
    self.timer_click = None



class TutorialTouchpad:
  """ Displays a touchpad showing the zoom animation """

  # position of the center of the touchpad in screen coordinates (800 x 520)
  touchpad_x = 0.0
  touchpad_y = 0.0

  # center of the touchpad in the svg file
  pivot_touchpad_x = 400
  pivot_touchpad_y = 220

  # center of the finger in the svg file
  pivot_finger_x = 600
  pivot_finger_y = 220

  # center of the touch effect in the svg file
  pivot_touch_effect_x = 700
  pivot_touch_effect_y = 220

  # position of the touch effect center relative to the touchpad center
  center_touchpad_to_center_touch_effect_x = -3
  center_touchpad_to_center_touch_effect_y = 0

  # the number of milliseconds one zoom finger animation lasts
  zoom_animation_time_total = 3000

  # the time between each zoom animation step, in milliseconds
  zoom_animation_time_step = 100

  # 1: show "1"-finger animation
  # 2: show "2"-finger animation
  zoom_number_of_fingers = None

  # zoom "in" or "out"
  zoom_direction = None

  # the finger's movement on the y-axis during one finger animation
  zoom_finger_movement_y = 40

  # one time timer to start the zoom animation again
  zoom_animation_start_timer = None

  # we remember the handler id to be able to disconnect it again
  zoom_animation_finished_handler_id = None

  # the number of milliseconds one finger click animation lasts (only the lowering part)
  click_animation_time_total = 1000

  # the time between each click animation step, in milliseconds
  click_animation_time_step = 100

  # the factor, the finger is scaled down by, during the click animation
  click_animation_finger_scale_down = 0.667

  # the number of milliseconds, the touch-effect is shown during a click animation
  click_show_effect_milliseconds = 750

  # one time timer to restart the click animation, after showing the touch effect
  click_animation_show_effect_timer = None

  # we remember the handler id to be able to disconnect it again
  click_animation_finished_handler_id = None


  def __init__(self, rootitem, svghandle, x, y):
    """
    Constructor
        rootitem    : The goocanvas item to add the mouse to
        svghandle   : Handle of the svg file, containing the graphics
        x, y        : Position of the center of the touchpad in screen coordinates (800 x 520)
    """

    self.touchpad_x = x
    self.touchpad_y = y

    self.touchpad_img = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#TOUCHPAD",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      )

    self.finger01_img = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#FINGER",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      )

    self.finger02_img = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#FINGER",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      )

    self.touch_effect_img = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = "#TOUCHPAD_PRESS_EFFECT",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE
      )

    self.__update_touchpad_transformation()


  def start_zoom(self, zoom_direction):
    """
    Start the touchpad zoom tutorial: show graphics, start animation
      zoom_direction: "in" or "out"
    """

    self.zoom_animation_finished_handler_id = self.finger01_img.connect("animation-finished", self.__zoom_animation_finished)

    self.zoom_direction = zoom_direction
    self.touchpad_img.props.visibility = goocanvas.ITEM_VISIBLE
    self.zoom_number_of_fingers = 1
    self.__zoom_animation_start()


  def start_click(self):
    """ Start the touchpad click tutorial: show graphics, start animation """

    self.click_animation_finished_handler_id = self.finger01_img.connect("animation-finished", self.__click_animation_finished)

    self.touchpad_img.props.visibility = goocanvas.ITEM_VISIBLE
    self.finger01_img.props.visibility = goocanvas.ITEM_VISIBLE

    self.__click_animation_start()


  def stop(self):
    """ Stop the touchpad tutorials: hide graphics, stop animation """

    self.finger01_img.stop_animation()
    self.finger02_img.stop_animation()

    if self.zoom_animation_finished_handler_id is not None:
      self.finger01_img.disconnect(self.zoom_animation_finished_handler_id)
      self.zoom_animation_finished_handler_id = None

    if self.click_animation_finished_handler_id is not None:
      self.finger01_img.disconnect(self.click_animation_finished_handler_id)
      self.click_animation_finished_handler_id = None

    if self.zoom_animation_start_timer is not None:
      # do not start another zoom animation
      gobject.source_remove(self.zoom_animation_start_timer)
      self.zoom_animation_start_timer = None

    if self.click_animation_show_effect_timer is not None:
      # do not continue the click animation
      gobject.source_remove(self.click_animation_show_effect_timer)
      self.click_animation_show_effect_timer = None

    self.touch_effect_img.props.visibility = goocanvas.ITEM_INVISIBLE
    self.touchpad_img.props.visibility = goocanvas.ITEM_INVISIBLE
    self.__hide_fingers()


  def end(self):
    """ Our "destructor" """
    pass


  def __hide_fingers(self):
    """ Hide all fingers """
    self.finger01_img.props.visibility = goocanvas.ITEM_INVISIBLE
    self.finger02_img.props.visibility = goocanvas.ITEM_INVISIBLE


  def __zoom_animation_start(self):
    """ Starts the touchpad zoom animation """

    self.__zoom_position_fingers()


    if self.zoom_direction == 'in':
      destination_y = -self.zoom_finger_movement_y

    elif self.zoom_direction == 'out':
      destination_y = +self.zoom_finger_movement_y

    else:
      # wrong zoom direction
      assert(False)


    self.finger01_img.props.visibility = goocanvas.ITEM_VISIBLE

    if self.zoom_number_of_fingers == 1:
      self.finger02_img.props.visibility = goocanvas.ITEM_INVISIBLE

      self.finger01_img.animate(
        0, # destination x position in root coordinates (800x520)
        destination_y, # destination y position in root coordinates (800x520)
        1, 0, # no scale and no rotation
        False, # destination position is relative
        self.zoom_animation_time_total, # see variable definition
        self.zoom_animation_time_step, # see variable definition
        goocanvas.ANIMATE_FREEZE  # stop at animation end
        )

    elif self.zoom_number_of_fingers == 2:
      self.finger02_img.props.visibility = goocanvas.ITEM_VISIBLE

      self.finger01_img.animate(
        0,
        destination_y,
        1, 0,
        False,
        self.zoom_animation_time_total,
        self.zoom_animation_time_step,
        goocanvas.ANIMATE_FREEZE
        )

      self.finger02_img.animate(
        0,
        destination_y,
        1, 0,
        False,
        self.zoom_animation_time_total,
        self.zoom_animation_time_step,
        goocanvas.ANIMATE_FREEZE
        )

    else:
      # Wrong number of fingers!
      assert(False)


  def __zoom_animation_finished(self, item, stopped):
    """
    Called, when the zoom finger animation is finished
      item - The element connected with this callback function
      stopped - True if the animation was stopped explicitly
    """

    if stopped:
      # animation has been stopped explicitly (tutorial.stop())
      # => do not start another one
      return True


    ##
    # start animation with the other number of fingers

    # switch number of fingers
    if self.zoom_number_of_fingers == 1:
      self.zoom_number_of_fingers = 2

    elif self.zoom_number_of_fingers == 2:
      self.zoom_number_of_fingers = 1

    else:
      # Wrong number of fingers!
      assert(False)

    # Restart the animation again after a short period, to make sure the other animation
    # has finished, too. And I would have a bad feeling, starting a new animation while being
    # in the animation-finished function of the same object.
    self.zoom_animation_start_timer = gobject.timeout_add(int(self.zoom_animation_time_step * 1.5), self.__zoom_animation_start_by_onetime_timer)

    return True


  def __zoom_animation_start_by_onetime_timer(self):
    """ Start the finger zoom animation again, called by a timer, once """

    # this timer is consumed
    self.zoom_animation_start_timer = None

    self.__zoom_animation_start()

    # do not call this timer again (this return is the reason for this function)
    return False


  def __update_touchpad_transformation(self):
    """ Updates the transformation matrix of the touchpad and the touch effect """

    matrix = cairo.Matrix(1, 0, 0, 1, self.touchpad_x - self.pivot_touchpad_x, self.touchpad_y - self.pivot_touchpad_y)
    self.touchpad_img.set_transform(matrix)

    matrix = cairo.Matrix(1, 0, 0, 1,
      self.touchpad_x - self.pivot_touch_effect_x + self.center_touchpad_to_center_touch_effect_x,
      self.touchpad_y - self.pivot_touch_effect_y + self.center_touchpad_to_center_touch_effect_y
      )
    self.touch_effect_img.set_transform(matrix)


  def __zoom_position_fingers(self):
    """ Updates the position of the fingers for zoom animation to start """

    if self.zoom_direction == 'in':
      finger_y = 85

    elif self.zoom_direction == 'out':
      finger_y = 45

    else:
      # wrong zoom direction
      assert(False)


    if self.zoom_number_of_fingers == 1:
      self.__update_finger_transformation(self.finger01_img, 35, finger_y, 1.0, -1.0)

    elif self.zoom_number_of_fingers == 2:
      self.__update_finger_transformation(self.finger01_img, -20, finger_y, 1.0, -1.0)
      self.__update_finger_transformation(self.finger02_img, 2, finger_y - 2, 1.0, -2.5)

    else:
      # Wrong number of fingers!
      assert(False)


  def __click_animation_start(self):
    """ Starts the touchpad click animation """

    # position finger (with reverse "scale down", so at the end of the animation, we have scale = 1.0)
    self.__update_finger_transformation(self.finger01_img, -2.1, 68, 1.0 / self.click_animation_finger_scale_down, -1.0)

    # start "lowering finger" animation
    self.finger01_img.animate(
      # shrinking (scale down) affects our pivot correction, so we have to compensate this
      # by relative movement:
      self.pivot_finger_x * (1 - self.click_animation_finger_scale_down), # position correction on x axis
      self.pivot_finger_y * (1 - self.click_animation_finger_scale_down), # position correction on y axis

      self.click_animation_finger_scale_down, # scale to 1.0
      0, # no rotation
      False, # destination position is relative
      self.click_animation_time_total, # see variable definition
      self.click_animation_time_step, # see variable definition
      goocanvas.ANIMATE_FREEZE  # stop at animation end
      )


  def __click_animation_finished(self, item, stopped):
    """
    Called, when the click finger animation is finished
      item - The element connected with this callback function
      stopped - True if the animation was stopped explicitly
    """

    if stopped:
      # animation has been stopped explicitly (tutorial.stop())
      # => do not continue this animation
      return True

    # show "touch"-effect
    self.touch_effect_img.props.visibility = goocanvas.ITEM_VISIBLE

    # start with the "lowering finger" animation again, in a few milliseconds
    self.click_animation_show_effect_timer = gobject.timeout_add(self.click_show_effect_milliseconds, self.__click_animation_start_by_onetime_timer)

    return True


  def __click_animation_start_by_onetime_timer(self):
    """ Start the finger click animation again, called by a timer, once """

    # this timer is consumed
    self.click_animation_show_effect_timer = None

    # hide the "touch"-effect again for new animation
    self.touch_effect_img.props.visibility = goocanvas.ITEM_INVISIBLE

    self.__click_animation_start()

    # do not call this timer again (this return is the reason for this function)
    return False


  def __update_finger_transformation(self, finger_img, x, y, scale, angle):
    """
    Updates the transformation matrix of the given finger
      finger_img: the finger to move
      x, y: position of the finger, relative to the touchpad center
      scale: finger scale
      angle: finger rotation in degrees
    """

    # we need those values more than once, so lets remember them
    a = math.radians(angle)
    cos_a = math.cos(a)
    sin_a = math.sin(a)

    # create the transformation matrices
    m_center_to_origin = cairo.Matrix(1, 0, 0, 1, -self.pivot_finger_x, -self.pivot_finger_y)
    m_scale = cairo.Matrix(scale, 0, 0, scale, 0, 0)
    m_rotate = cairo.Matrix(cos_a, sin_a, -sin_a, cos_a, 0, 0)
    m_to_destination = cairo.Matrix(1, 0, 0, 1, x + self.touchpad_x, y + self.touchpad_y)

    # combine all transformation matrices to one
    matrix = m_center_to_origin * m_scale * m_rotate * m_to_destination

    finger_img.set_transform(matrix)

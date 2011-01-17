#  gcompris - watercycle
#
# Copyright (C) 2003, 2008 Bruno Coudoin
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
import gcompris.anim
import gcompris.skin
import gcompris.sound
import gcompris.bonus
import gtk
import gtk.gdk

from gcompris import gcompris_gettext as _

class Gcompris_watercycle:
  """The Water Cycle activity"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.boat_is_arrived = False

  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    # The basic duration factor for object animations
    # Higher is longer animation (slower)
    self.timerinc = 20
    self.step_time = 100

    # Need to manage the timers to quit properly
    self.waterlevel_timer = 0

    # Used to display the bonus a single time
    self.you_won = False

    gcompris.sound.play_ogg("sounds/Harbor1.wav", "sounds/Harbor3.wav")

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

    svghandle = gcompris.utils.load_svg("watercycle/watercycle.svgz")
    goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#BACKGROUND",
      pointer_events = goocanvas.EVENTS_NONE
      )

    # Take care, the items are stacked on each other in the order you add them.
    # If you need, you can reorder them later with raise and lower functions.

    # The River
    self.riveritem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#RIVERWATER",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )
    self.riverfull = False

    # The dirty water
    self.dirtywater = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#DIRTYWATER",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # The clean water
    self.cleanwateritem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#CLEANWATER",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # The Sun
    self.sunitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SUN",
      tooltip = "\n\n\n" + \
        _("The sun heats the water and creates water vapor. "
          "Water vapor combines into small water droplets which "
          "becomes clouds.")
      )
    self.sunitem.connect("button_press_event", self.sun_item_event)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.sunitem, None)
    self.sun_on = 0
    self.sunitem_orig_y1 = self.sunitem.get_bounds().y1
    self.sunitem_target_y1 = 10
    self.sun_connect_handler = 0

    # The Snow
    self.snowitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SNOW",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # The rain
    self.rainitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#RAIN",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # The cloud
    self.clouditem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#CLOUD",
      visibility = goocanvas.ITEM_INVISIBLE,
      tooltip = "\n\n\n" + \
        _("As a cloud matures, the dense water droplets may combine "
          "to produce larger droplets, which may combine to form "
          "droplets large enough to fall as rain")
      )
    self.clouditem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.clouditem.connect("animation-finished", self.cloud_arrived)
    self.cloud_on = False
    self.cloud_is_arrived = False
    self.clouditem_bounds = self.clouditem.get_bounds()

    # The vapor
    self.vaporitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#VAPOR",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )
    self.vaporitem.connect("animation-finished", self.vapor_arrived)

    # The Waterpump
    self.waterpumpitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#PUMPSTATION",
      tooltip = "\n\n\n" + \
        _("This is a water pump station.")
      )
    self.waterpumpitem.connect("button_press_event", self.waterpump_item_event)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.waterpumpitem, None)
    self.waterpump_on = 0

    # The WaterCleaning
    self.watercleaningitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#CLEANSTATION",
      tooltip = "\n\n\n" + \
        _("This is a water cleanup station.")
      )
    self.watercleaningitem.connect("button_press_event", self.watercleaning_item_event)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.watercleaningitem, None)
    self.watercleaning_on = 0

    # The tuxboat
    self.boatitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#BOAT"
      )
    self.boatitem.translate(-100, 0);
    self.boatitem_parked = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#BOAT_PARKED",
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # Tux in the shower (without water)
    self.tuxshoweritem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SHOWER",
      visibility = goocanvas.ITEM_INVISIBLE
      )

    self.shower_tux = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SHOWER_TUX",
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # Tux in the shower with the water
    self.tuxshowerwateritem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SHOWER_ON",
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # The shower on/off button (I need to get the 2 buttons to manage the focus)
    self.showerbuttonitem_on = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SHOWER_BUTTON_ON",
      visibility = goocanvas.ITEM_INVISIBLE
      )
    self.showerbuttonitem_on.connect("button_press_event", self.showerbutton_item_event,
                                     True)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.showerbuttonitem_on, None)
    self.showerbutton = False

    self.showerbuttonitem_off = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SHOWER_BUTTON_OFF",
      visibility = goocanvas.ITEM_INVISIBLE
      )
    self.showerbuttonitem_off.connect("button_press_event", self.showerbutton_item_event,
                                      False)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.showerbuttonitem_off, None)

    goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#FOREGROUND",
      pointer_events = goocanvas.EVENTS_NONE
      )

    goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#WATER_TOWER",
      tooltip = "\n\n\n" + \
        _("A water tower or elevated water tower is a large elevated water storage container constructed to hold a water supply at a height sufficient to pressurize a water distribution system.")
      )

    # The level of water in the tower
    self.waterlevel_max = 12
    self.waterlevel_min = 1
    self.waterlevel_x_ratio = 1.3
    self.waterlevel = 1
    self.waterlevel_item = goocanvas.Ellipse(
                parent = self.rootitem,
                center_x = 635,
                center_y = 142,
                radius_x = 1,
                radius_y = 1,
                fill_color_rgba = 0x0033FFC0L,
                line_width = 0)
    self.waterlevel_timer = gobject.timeout_add(1000, self.update_waterlevel)

    # Some item ordering
    self.rainitem.raise_(None)
    self.clouditem.raise_(None)
    self.vaporitem.raise_(None)

    # Ready GO
    target_x = 700
    trip_x = int(target_x - self.boatitem.get_bounds().x1)
    self.boatitem.connect("animation-finished", self.boat_arrived)
    self.boatitem.animate(target_x,
                          0,
                          1,
                          1,
                          True,
                          self.timerinc*trip_x,
                          self.step_time,
                          goocanvas.ANIMATE_FREEZE)

    gcompris.bar_set(0)
    gcompris.bar_location(5, -1, 0.6)
    gcompris.bar_set_level(self.gcomprisBoard)


  def end(self):
    # Remove all the timer first
    if self.waterlevel_timer :
      gobject.source_remove(self.waterlevel_timer)

    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    pass


  def repeat(self):
    pass


  def config(self):
    pass

  def key_press(self, keyval, commit_str, preedit_str):
    return False

  # This is called each second to update to castle water level
  def update_waterlevel(self):
    old_waterlevel = self.waterlevel

    if self.waterpump_on and self.waterlevel < self.waterlevel_max :
      self.waterlevel += 1

    # It tux is in the shower and it works, then remove some water
    if (self.showerbutton
        and self.waterlevel > self.waterlevel_min) :
      self.waterlevel -= 2

    if self.waterlevel < 0:
      self.waterlevel = self.waterlevel_min

    # Redisplay the level of water if needed
    if old_waterlevel != self.waterlevel :
      self.waterlevel_item.props.radius_x = self.waterlevel * self.waterlevel_x_ratio
      self.waterlevel_item.props.radius_y = self.waterlevel

    if self.waterlevel < self.waterlevel_min + 3 :
      # In this case, de activate the shower
      self.showerbutton_update(False)

    if self.waterlevel == self.waterlevel_max:
      self.cloud_reset()

    self.waterlevel_timer = gobject.timeout_add(1000, self.update_waterlevel)

  def boat_arrived(self, item, status):
    # park the boat, change the boat to remove tux
    self.boat_is_arrived = True
    self.boatitem_parked.props.visibility = goocanvas.ITEM_VISIBLE
    self.boatitem.props.visibility = goocanvas.ITEM_INVISIBLE
    gcompris.sound.play_ogg("sounds/Harbor3.wav")

    # Now display tux in the shower
    self.tuxshoweritem.props.visibility = goocanvas.ITEM_VISIBLE
    self.shower_tux.props.visibility = goocanvas.ITEM_VISIBLE
    self.showerbuttonitem_on.props.visibility = goocanvas.ITEM_VISIBLE

  def sun_down(self):
    # Move the sun down
    trip_y = self.sunitem_orig_y1 - self.sunitem_target_y1
    if self.sun_connect_handler:
      self.sunitem.disconnect(self.sun_connect_handler)
    self.sun_connect_handler = self.sunitem.connect("animation-finished",
                                                    self.sun_down_arrived)
    self.sunitem.animate(0,
                         0,
                         1,
                         1,
                         True,
                         int(self.timerinc*abs(trip_y)),
                         self.step_time,
                         goocanvas.ANIMATE_FREEZE)
    return False

  def sun_up_arrived(self, item, status):
    # Start the vapor
    self.init_vapor()

  def sun_down_arrived(self, item, status):
    gcompris.utils.item_focus_init(self.sunitem, None)
    # Stop the sun
    self.sun_on = 0

  def cloud_reset(self):
    if (not self.cloud_is_arrived or not self.riverfull):
      return

    self.clouditem.translate(self.clouditem_bounds.x1 - self.clouditem.get_bounds().x1,
                             0)
    self.clouditem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.rainitem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.snowitem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.cloud_is_arrived = False
    self.cloud_on = False

  def cloud_arrived(self, item, status):
    self.sun_down()
    self.clouditem.connect("button_press_event", self.cloud_item_event)
    gcompris.utils.item_focus_init(self.clouditem, None)
    self.cloud_is_arrived = True

  def move_cloud(self):
    if (self.cloud_on):
      return
    self.cloud_on = True
    self.clouditem.props.visibility = goocanvas.ITEM_VISIBLE
    trip_x = 270
    self.clouditem.animate(trip_x,
                           0,
                           1,
                           1,
                           True,
                           self.timerinc*trip_x,
                           self.step_time,
                           goocanvas.ANIMATE_FREEZE)

  def vapor_arrived(self, item, state):
    self.vapor_on = 0
    self.vaporitem.props.visibility = goocanvas.ITEM_INVISIBLE
    # Start the cloud
    self.move_cloud()

  def init_vapor(self):
    self.vapor_on = 1
    self.vaporitem.props.visibility = goocanvas.ITEM_VISIBLE
    trip_y = 80
    self.vaporitem.animate(0,
                           trip_y * -1,
                           1,
                           1,
                           True,
                           self.timerinc*trip_y,
                           self.step_time,
                           goocanvas.ANIMATE_RESET)



  def pause(self, pause):
    pass

  def set_level(self, level):
    pass

  def sun_item_event(self, widget, target, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if not self.sun_on and not self.cloud_on:
          gcompris.utils.item_focus_remove(self.sunitem, None)
          gcompris.sound.play_ogg("sounds/bleep.wav")
          trip_y = self.sunitem_target_y1 - self.sunitem_orig_y1
          if self.sun_connect_handler:
            self.sunitem.disconnect(self.sun_connect_handler)
          self.sun_connect_handler = self.sunitem.connect("animation-finished", self.sun_up_arrived)
          self.sunitem.animate(0,
                               trip_y,
                               1,
                               1,
                               True,
                               int(self.timerinc*abs(trip_y)),
                               self.step_time,
                               goocanvas.ANIMATE_FREEZE)

          self.sun_on = 1
        return True
    return False

  def cloud_item_event(self, widget, target, event=None):
    gcompris.sound.play_ogg("sounds/Water5.wav")
    self.rainitem.props.visibility = goocanvas.ITEM_VISIBLE
    self.snowitem.props.visibility = goocanvas.ITEM_VISIBLE
    self.riveritem.props.visibility = goocanvas.ITEM_VISIBLE
    self.riverfull = True
    return True

  def waterpump_item_event(self, widget, target, event=None):
    if self.riverfull:
      gcompris.sound.play_ogg("sounds/bubble.wav")
      self.cleanwateritem.props.visibility = goocanvas.ITEM_VISIBLE
      self.waterpump_on = 1
      return True

  def watercleaning_item_event(self, widget, target, event=None):
    if self.riverfull:
      gcompris.sound.play_ogg("sounds/bubble.wav")
      self.watercleaning_on = 1
      self.dirtywater.props.visibility = goocanvas.ITEM_VISIBLE
    return True


  # If Tux is in the shower, we must display the water if needed
  def shower_water_update(self):

    if (self.waterlevel > self.waterlevel_min) and self.showerbutton:
      self.tuxshowerwateritem.props.visibility = goocanvas.ITEM_VISIBLE
      if not self.you_won:
        gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)
        self.you_won = True
    else:
      self.tuxshowerwateritem.props.visibility = goocanvas.ITEM_INVISIBLE

  def showerbutton_update(self, state):

    if not self.watercleaning_on:
      return

    self.showerbutton = state
    if not self.showerbutton:
      gcompris.sound.play_ogg("sounds/bleep.wav")
      self.showerbuttonitem_on.props.visibility = goocanvas.ITEM_VISIBLE
      self.showerbuttonitem_off.props.visibility = goocanvas.ITEM_INVISIBLE
    else:
      gcompris.sound.play_ogg("sounds/apert2.wav")
      self.showerbuttonitem_on.props.visibility = goocanvas.ITEM_INVISIBLE
      self.showerbuttonitem_off.props.visibility = goocanvas.ITEM_VISIBLE

    self.shower_water_update()


  def showerbutton_item_event(self, widget, target, event, state):

    self.showerbutton_update(state)


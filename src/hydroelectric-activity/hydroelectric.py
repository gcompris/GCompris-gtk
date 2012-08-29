#  gcompris - hydroelectric
#
# Copyright (C) 2010 Bruno Coudoin
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

class Gcompris_hydroelectric:
  """The Hydroelectric activity"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=3

    # Just a tick counter
    self.tick = 0

    self.boat_is_arrived = False
    self.reset_state = True
    self.gamewon = False
    # The basic duration factor for object animations
    # Higher is longer animation (slower)
    self.timerinc = 20
    self.step_time = 100

    # Need to manage the timers to quit properly
    self.waterlevel_timer = 0

    gcompris.sound.play_ogg("sounds/Harbor1.wav", "sounds/Harbor3.wav")

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

    self.svghandle = gcompris.utils.load_svg("hydroelectric/hydroelectric.svgz")
    goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#BACKGROUND",
      pointer_events = goocanvas.EVENTS_NONE
      )

    # Take care, the items are stacked on each other in the order you add them.
    # If you need, you can reorder them later with raise and lower functions.

    # The River
    self.riveritem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#RIVERWATER",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # The Sun
    self.sunitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
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
    self.sunitem_bounds = self.sunitem.get_bounds()
    self.sunitem_target_y1 = 10
    self.sun_connect_handler = 0

    # The Snow
    self.snowitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#SNOW",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # The rain
    self.rainitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#RAIN",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # The cloud
    self.clouditem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
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
    self.its_raining = False

    # The vapor
    self.vaporitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#VAPOR",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )
    self.vaporitem.connect("animation-finished", self.vapor_arrived)
    self.vaporitem_bounds = self.vaporitem.get_bounds()

    # The reservoir level
    self.reservoirlevel = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#RESERVOIR1",
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # The Dam
    goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#DAM",
      pointer_events = goocanvas.EVENTS_NONE,
      )

    # The DAM'S TURBINE
    self.dam_turbine = \
      Producer(self.rootitem, self.svghandle,
               Counter( self.rootitem, self.svghandle,
                        "#DAM_PROD_COUNT",
                        #Translator : "meter" here means a measuring device
                        _("This is the meter for electricity produced by the turbine. ") + \
                        _("The electricity power is measured in watts (W)."),
                        303, 224 ),
               self.update_prod_count,
               None,
               [ "#TURBINE" ],
               _("Flowing water is directed on to the blades of a turbine runner, "
                 "creating a force on the blades. In this way, energy is transferred "
                 "from the water flow to the turbine"),
               "#TURBINE_CABLE_ON",
               "#TRANSFORMER_DAM",
               "#TRANSFORMER_DAM_TO_USERS",
               1000)

    # The Wind
    self.winditem_off = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#WIND_OFF",
      tooltip = "\n\n\n" + \
        _("This cloud simulates the wind, click on it to have wind.")
      )
    self.winditem_off.connect("button_press_event", self.wind_item_event)
    # This item is clickable and it must be seen
    gcompris.utils.item_focus_init(self.winditem_off, None)

    self.winditem_on = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#WIND_ON",
      visibility = goocanvas.ITEM_INVISIBLE,
      pointer_events = goocanvas.EVENTS_NONE,
      )
    self.wind = False

    # The Cable from transformer 2 to Town
    self.cable_to_town_on = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#CABLE_TO_TOWN_ON",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )
    # The Cable from transformer 2 to Tux
    self.cable_to_tux_on = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#CABLE_TO_TUX_ON",
      pointer_events = goocanvas.EVENTS_NONE,
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # TRANSFORMER2
    self.transformer2item = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#TRANSFORMER2",
      tooltip = "\n\n\n" + \
        _("This is a step down transformer. Electricity is transformed "
          "in low voltage, ready to be used by the customers.")
      )
    self.transformer2item.connect("button_press_event", self.transformer2_item_event)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.transformer2item, None)
    self.transformer2_on = 0

    # The tuxboat
    self.boatitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#BOAT"
      )
    self.boatitem.translate(-100, 0);
    self.boatitem_bounds = self.boatitem.get_bounds()
    self.boatitem.connect("animation-finished", self.boat_arrived)
    self.boatitem_parked = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#BOAT_PARKED",
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # Tux in his saloon
    self.tuxsaloonitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#SALOON",
      visibility = goocanvas.ITEM_INVISIBLE
      )

    goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = self.svghandle,
      svg_id = "#FOREGROUND",
      pointer_events = goocanvas.EVENTS_NONE
      )

    # The level of water in the reservoir
    self.waterlevel_max = 5
    self.waterlevel_min = 1
    self.waterlevel = 0
    self.waterlevel_timer = gobject.timeout_add(1000, self.update)

    # We have 2 counters, the production one and the consumption
    # The children have to make sure there is no more consumption
    # than production.
    self.prod_count = 0
    self.conso_count = 0

    self.production_counter = \
        Counter( self.rootitem, self.svghandle,
                 "#PROD_COUNT",
                 #Translator : "meter" here means a measuring device
                 _("This is the meter for all the electricity produced. ") + \
                 _("The electricity power is measured in watts (W)."),
                 525, 226 )

    self.consumers_counter = \
        Counter( self.rootitem, self.svghandle,
                 "#CONSO_COUNT",
                 #Translator : "meter" here means a measuring device
                 _("This is the meter for electricity consumed by the users. ") + \
                 _("The electricity power is measured in watts (W)."),
                 590, 203 )

    self.consumers = []
    self.conso_tux = Consumer(self.rootitem, self.svghandle,
                              self.update_conso_count,
                              "#LIGHT_BUTTON_ON",
                              "#LIGHT_BUTTON_OFF",
                              "#TUX_ON",
                              "#TUX_OFF",
                              100)
    self.conso_house = Consumer(self.rootitem, self.svghandle,
                                self.update_conso_count,
                                "#LIGHT_HOUSE_ON",
                                "#LIGHT_HOUSE_OFF",
                                "#HOUSE_LIGHTS_ON",
                                None, 400)
    self.conso_building = Consumer(self.rootitem, self.svghandle,
                                   self.update_conso_count,
                                   "#LIGHT_BUILDING_ON",
                                   "#LIGHT_BUILDING_OFF",
                                   "#BUILDING_LIGHTS_ON",
                                   None, 800)

    # The solar panel
    self.solar_array = \
      Producer(self.rootitem, self.svghandle,
               Counter( self.rootitem, self.svghandle,
                        "#SOLAR_PANEL_COUNT",
                        #Translator : "meter" here means a measuring device
                        _("This is the meter for electricity produced by the solar panels. ") + \
                        _("The electricity power is measured in watts (W)."),
                        697, 177 ),
               self.update_prod_count,
               "#SOLAR_PANEL_BG",
               [ "#SOLAR_PANEL_OFF", "#SOLAR_PANEL_ON" ],
               _("Solar panels use light energy (photons) from the sun to "
                 "generate electricity through the photovoltaic effect."),
               "#SOLAR_PANEL_CABLE_ON",
               "#TRANSFORMER_SOLAR_PANEL",
               "#TRANSFORMER_SOLAR_PANEL_TO_USERS",
               400)

    # The Wind farm
    self.windfarmitems = []
    for i in range(1,4):
      self.windfarmitems.append("#WIND_FARM_" + str(i))

    self.wind_farm = \
      Producer(self.rootitem, self.svghandle,
               Counter( self.rootitem, self.svghandle,
                        "#WIND_FARM_COUNT",
                        #Translator : "meter" here means a measuring device
                        _("This is the meter for electricity produced by the wind turbines. ") + \
                        _("The electricity power is measured in watts (W)."),
                        650, 137 ),
               self.update_prod_count,
               "#WIND_FARM_BG",
               self.windfarmitems,
               _("A wind turbine is a device that converts wind motion energy "
                 "into electricity generation. It is called a wind generator or "
                 "wind charger. "),
               "#WINDFARM_CABLE_ON",
               "#TRANSFORMER_WINDFARM",
               "#TRANSFORMER_WINDFARM_TO_USERS",
               600)

    # Some item ordering
    self.rainitem.raise_(None)
    self.clouditem.raise_(None)
    self.vaporitem.raise_(None)

    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(5, -1, 0.6)
    gcompris.bar_set_level(self.gcomprisBoard)
    self.next_level()

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

  # This is called each second to update the whole activity
  def update(self):
    old_waterlevel = self.waterlevel
    self.tick += 1

    if self.its_raining and self.waterlevel < self.waterlevel_max :
      self.waterlevel += 1

    if not self.cloud_on \
       and self.waterlevel == self.waterlevel_max \
       and self.tick % 200 == 0:
      # Simulate a miss of water
      self.waterlevel -= 1

    # Redisplay the level of water if needed
    if old_waterlevel != self.waterlevel :
      # Represent the water level
      self.reservoirlevel.set_properties(
        svg_id = "#RESERVOIR%d" %self.waterlevel,
        visibility = goocanvas.ITEM_VISIBLE)

    if self.waterlevel == self.waterlevel_max \
       and self.tick % 40 == 0 \
       and self.cloud_is_arrived:
      self.cloud_reset()

    if self.wind:
      self.wind += 1
      if self.wind % 60 == 0:
        self.set_wind_state(0)


    # Manage the consumers ability to produce energy
    if self.wind_farm.enabled:
      if self.wind:
        self.wind_farm.set_energy(True)
      else:
        self.wind_farm.set_energy(False)

    if self.solar_array.enabled:
      if self.sun_on:
        self.solar_array.set_energy(True)
      else:
        self.solar_array.set_energy(False)

    if self.waterlevel == self.waterlevel_max:
      self.dam_turbine.set_energy(True)
    else:
      self.dam_turbine.set_energy(False)

    self.waterlevel_timer = gobject.timeout_add(1000, self.update)

  def waterlevel_reset(self):
    self.waterlevel = 0
    self.reservoirlevel.props.visibility = goocanvas.ITEM_INVISIBLE
    self.riveritem.props.visibility = goocanvas.ITEM_INVISIBLE

  def boat_start(self):
    target_x = 700
    trip_x = int(target_x - self.boatitem.get_bounds().x1)
    self.boatitem.animate(target_x,
                          0,
                          1,
                          1,
                          True,
                          self.timerinc*trip_x,
                          self.step_time,
                          goocanvas.ANIMATE_FREEZE)


  def boat_arrived(self, item, status):
    if self.reset_state: return
    # park the boat, change the boat to remove tux
    self.boat_is_arrived = True
    self.boatitem_parked.props.visibility = goocanvas.ITEM_VISIBLE
    self.boatitem.props.visibility = goocanvas.ITEM_INVISIBLE
    gcompris.sound.play_ogg("sounds/Harbor3.wav")

    # Now display tux in the saloon
    self.tuxsaloonitem.props.visibility = goocanvas.ITEM_VISIBLE

    # Display all consumers
    map(lambda s: s.off(), self.consumers)

  def boat_reset(self):
    self.boat_is_arrived = False
    self.boatitem.stop_animation()
    self.boatitem_parked.props.visibility = goocanvas.ITEM_INVISIBLE
    self.boatitem.props.visibility = goocanvas.ITEM_VISIBLE
    self.tuxsaloonitem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.boatitem.translate( \
      self.boatitem_bounds.x1 - self.boatitem.get_bounds().x1,
      0)


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
                         int(self.timerinc*abs(trip_y)) * 10,
                         self.step_time,
                         goocanvas.ANIMATE_FREEZE)
    return False

  def sun_up_arrived(self, item, status):
    if self.reset_state: return
    # Start the vapor
    self.init_vapor()

  def sun_down_arrived(self, item, status):
    gcompris.utils.item_focus_init(self.sunitem, None)
    # Stop the sun
    self.sun_on = 0

  def sun_reset(self):
    self.sunitem.stop_animation()
    self.sunitem.translate( 0,
      self.sunitem_bounds.y1 - self.sunitem.get_bounds().y1 )
    self.sun_down_arrived(None, None)

  def cloud_reset(self):
    self.clouditem.stop_animation()
    self.clouditem.translate( \
      self.clouditem_bounds.x1 - self.clouditem.get_bounds().x1,
      0)
    self.clouditem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.rainitem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.snowitem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.cloud_is_arrived = False
    self.cloud_on = False
    self.its_raining = False

  def cloud_arrived(self, item, status):
    if self.reset_state: return
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
    if self.reset_state: return
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


  def vapor_reset(self):
    self.vaporitem.stop_animation()
    self.vaporitem.translate( \
      self.vaporitem_bounds.x1 - self.vaporitem.get_bounds().x1,
      0)
    self.vapor_on = 0
    self.vaporitem.props.visibility = goocanvas.ITEM_INVISIBLE

  def pause(self, pause):
    self.board_paused = pause

    # When the bonus is displayed, it call us first
    # with pause(1) and then with pause(0)
    # the game is won
    if(self.gamewon and pause == 0):
      self.gamewon = False
      self.increment_level()
      self.next_level()

    return


  def set_level(self, level):
    self.gcomprisBoard.level = level;
    gcompris.bar_set_level(self.gcomprisBoard)
    self.next_level()

  def next_level(self):
    self.producers = None
    self.reset()
    if self.gcomprisBoard.level == 1:
      self.producers = [self.dam_turbine]
      self.wind_farm.disable()
      self.solar_array.disable()
      self.winditem_on.props.visibility = goocanvas.ITEM_INVISIBLE
      self.winditem_off.props.visibility = goocanvas.ITEM_INVISIBLE
      self.consumers = [self.conso_tux]
      self.conso_tux.disable()
      self.conso_house.disable()
      self.conso_building.disable()
    if self.gcomprisBoard.level == 2:
      self.producers = [self.dam_turbine, self.wind_farm]
      self.wind_farm.enable()
      self.winditem_on.props.visibility = goocanvas.ITEM_INVISIBLE
      self.winditem_off.props.visibility = goocanvas.ITEM_VISIBLE
      self.solar_array.disable()
      self.consumers = [self.conso_tux, self.conso_house]
      self.conso_tux.disable()
      self.conso_house.disable()
      self.conso_building.disable()
    elif self.gcomprisBoard.level == 3:
      self.producers = [self.dam_turbine, self.wind_farm,
                        self.solar_array]
      self.wind_farm.enable()
      self.solar_array.enable()
      self.consumers = [self.conso_tux, self.conso_house,
                        self.conso_building]
      self.conso_tux.disable()
      self.conso_house.disable()
      self.conso_building.disable()
    self.boat_start()

  def reset(self):
    self.reset_state = True
    self.sun_reset()
    self.cloud_reset()
    self.vapor_reset()
    self.boat_reset()
    self.wind_reset()
    self.waterlevel_reset()
    self.transformer2_disable()
    self.prod_count = 0
    self.production_counter.set(self.prod_count)
    self.conso_count = 0
    self.consumers_counter.set(self.conso_count)
    self.reset_state = False

  # Code that increments the level
  # And bail out if no more levels are available
  def increment_level(self):
    # Try the next level
    self.gcomprisBoard.level += 1
    # Set the level in the control bar
    if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
      self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

    gcompris.bar_set_level(self.gcomprisBoard);

  def set_wind_state(self, status):
    if status:
      self.winditem_on.props.visibility = goocanvas.ITEM_VISIBLE
      self.winditem_off.props.visibility = goocanvas.ITEM_INVISIBLE
    else:
      self.winditem_on.props.visibility = goocanvas.ITEM_INVISIBLE
      self.winditem_off.props.visibility = goocanvas.ITEM_VISIBLE
    self.wind = status

  def wind_item_event(self, widget, target, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        self.set_wind_state(1)

  def wind_reset(self):
    self.set_wind_state(False)

  def sun_item_event(self, widget, target, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if not self.sun_on and not self.cloud_on:
          gcompris.utils.item_focus_remove(self.sunitem, None)
          gcompris.sound.play_ogg("sounds/bleep.wav")
          trip_y = self.sunitem_target_y1 - self.sunitem_orig_y1
          if self.sun_connect_handler:
            self.sunitem.disconnect(self.sun_connect_handler)
          self.sun_connect_handler = self.sunitem.connect("animation-finished",
                                                          self.sun_up_arrived)
          self.sunitem.animate(0,
                               trip_y,
                               1,
                               1,
                               True,
                               int(self.timerinc*abs(trip_y)) * 3,
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
    self.its_raining = True
    self.clouditem.disconnect_by_func(self.cloud_item_event)
    gcompris.utils.item_focus_remove(self.clouditem, None)
    return True

  def update_prod_count(self):
    self.prod_count = 0
    self.prod_count = reduce(lambda x, y: x + y,
                             map(lambda x: x.production, self.producers) )

    self.production_counter.set(self.prod_count)
    self.check_balance()

  def update_conso_count(self):
    self.conso_count = 0
    if self.transformer2_on:
      self.conso_count = reduce(lambda x, y: x + y,
                           map(lambda x: x.consumption, self.consumers) )
      self.check_win()

    self.consumers_counter.set(self.conso_count)
    self.check_balance()

  def check_balance(self):
    # If there a miss of power
    if self.conso_count > self.prod_count:
      self.set_transformer2_state(False)
      gcompris.utils.dialog( \
        _("It is not possible to consume more electricity "
          "than what is produced. This is a key limitation in the "
          "distribution of electricity, with minor exceptions, "
          "electrical energy cannot be stored, and therefore it "
          "must be generated as it is needed. A sophisticated "
          "system of control is therefore required to ensure electric "
          "generation very closely matches the demand. If supply and demand "
          "are not in balance, generation plants and transmission equipment "
          "can shut down which, in the worst cases, can lead to a major "
          "regional blackout."), None)

  def check_win(self):
    if self.gamewon or self.gcomprisBoard.level == 3:
      return

    values = map(lambda x: x.consumption, self.consumers)
    values += map(lambda x: x.production, self.producers)

    if not 0 in values:
      self.gamewon = True
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)

  def transformer2_item_event(self, widget, target, event=None):
    if self.transformer2_on:
      self.set_transformer2_state(False)
    else:
      self.set_transformer2_state(True)
    return True

  def transformer2_enable(self):
    self.transformer2_on = True
    self.cable_to_town_on.props.visibility = goocanvas.ITEM_VISIBLE
    self.cable_to_tux_on.props.visibility = goocanvas.ITEM_VISIBLE

  def transformer2_disable(self):
    self.transformer2_on = False
    self.cable_to_town_on.props.visibility = goocanvas.ITEM_INVISIBLE
    self.cable_to_tux_on.props.visibility = goocanvas.ITEM_INVISIBLE

  def set_transformer2_state(self, state):
    if not self.boat_is_arrived:
      return
    if state and self.prod_count > 0:
      gcompris.sound.play_ogg("sounds/bubble.wav")
      self.transformer2_enable()
      map(lambda s: s.power_on(), self.consumers)
    elif self.transformer2_on:
      self.transformer2_disable()
      map(lambda s: s.power_off(), self.consumers)

    self.update_conso_count()


#
class Consumer:
  # Pass the SVG IDs of the stuff to act on
  def __init__(self, rootitem, svghandle,
               update_conso_count,
               switch_on, switch_off,
               target_on, target_off, power):
    self.power_count = power
    self.consumption = 0
    self.update_conso_count = update_conso_count
    self.switch_on = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = switch_on,
      visibility = goocanvas.ITEM_INVISIBLE
      )
    self.switch_on.connect("button_press_event",
                           self.lightbutton_item_event_on)
    gcompris.utils.item_focus_init(self.switch_on, None)
    self.switch_off = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = switch_off,
      visibility = goocanvas.ITEM_INVISIBLE
      )
    self.switch_off.connect("button_press_event",
                           self.lightbutton_item_event_off)
    gcompris.utils.item_focus_init(self.switch_off, None)
    self.target_on = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = target_on,
      visibility = goocanvas.ITEM_INVISIBLE
      )
    if target_off:
      self.target_off = goocanvas.Svg(
        parent = rootitem,
        svg_handle = svghandle,
        svg_id = target_off,
        visibility = goocanvas.ITEM_INVISIBLE
        )
    else:
      self.target_off = None
    # Is there power comming in
    self.power = False
    # Is the light is switched on
    self.is_on = False

  def enable(self):
    self.power_off()
    self.off()

  def disable(self):
    if self.target_off:
      self.target_off.props.visibility = goocanvas.ITEM_INVISIBLE
    self.target_on.props.visibility = goocanvas.ITEM_INVISIBLE
    self.switch_on.props.visibility = goocanvas.ITEM_INVISIBLE
    self.switch_off.props.visibility = goocanvas.ITEM_INVISIBLE
    self.power = False
    # Is the light is switched on
    self.is_on = False

  def update_light(self):
    if self.power and self.is_on:
      self.target_on.props.visibility = goocanvas.ITEM_VISIBLE
      if self.target_off:
        self.target_off.props.visibility = goocanvas.ITEM_INVISIBLE
      self.consumption = self.power_count
    else:
      if self.target_off:
        self.target_off.props.visibility = goocanvas.ITEM_VISIBLE
      self.target_on.props.visibility = goocanvas.ITEM_INVISIBLE
      self.consumption = 0
    self.update_conso_count()

  def on(self):
    self.switch_on.props.visibility = goocanvas.ITEM_VISIBLE
    self.switch_off.props.visibility = goocanvas.ITEM_INVISIBLE
    self.is_on = True
    self.update_light()

  def off(self):
    self.switch_off.props.visibility = goocanvas.ITEM_VISIBLE
    self.switch_on.props.visibility = goocanvas.ITEM_INVISIBLE
    self.switch_off.raise_(None)
    self.switch_on.raise_(None)
    self.is_on = False
    self.update_light()

  def lightbutton_item_event_on(self, widget, target, event):
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.off()

  def lightbutton_item_event_off(self, widget, target, event):
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.on()

  def power_off(self):
    self.power = False
    self.update_light()

  def power_on(self):
    self.power = True
    self.update_light()

#
class Producer:
  # Pass the SVG IDs of the stuff to act on
  def __init__(self, rootitem, svghandle, counter,
               update_prod_count,
               background_id,
               prod_items, tooltip,
               prod_item_on,
               transformer, transformer_on, power):
    self.power_count = power
    self.counter = counter
    self.production = 0
    self.update_prod_count = update_prod_count

    # Is there enough renewable energy to run this producer
    self.energy = False

    self.background_item = None
    if background_id:
      self.background_item = goocanvas.Svg(
        parent = rootitem,
        svg_handle = svghandle,
        svg_id = background_id,
        pointer_events = goocanvas.EVENTS_NONE
        )

    done = False
    self.current_prod_item = 0
    self.prod_items = []
    for svg_id in prod_items:
      item = goocanvas.Svg(
        parent = rootitem,
        svg_handle = svghandle,
        svg_id = svg_id,
        visibility = \
          goocanvas.ITEM_VISIBLE if not done else goocanvas.ITEM_INVISIBLE,
        tooltip = "\n\n\n" + tooltip
      )
      done = True
      item.connect("button_press_event",
                   self.runbutton_item_event)
      gcompris.utils.item_focus_init(item, None)
      self.prod_items.append(item)

    self.prod_item_on = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = prod_item_on,
      visibility = goocanvas.ITEM_INVISIBLE
      )

    self.transformer = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = transformer,
      tooltip = "\n\n\n" + \
        _("This is a step up transformer. Electricity is transmitted "
          "at high voltages (110 kV or above) "
          "to reduce the energy lost in long distance transmission.")
      )
    self.transformer.connect("button_press_event",
                 self.powerbutton_item_event)
    gcompris.utils.item_focus_init(self.transformer, None)

    self.transformer_on = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = transformer_on,
      visibility = goocanvas.ITEM_INVISIBLE
      )

    # Is the power on
    self.power = False
    # Is the run is switched on
    self.is_on = False
    # The animation timer
    self.timer = 0
    self.enabled = True

  def __del__(self):
    if self.timer:
      gobject.source_remove(self.timer)

  def enable(self):
    if self.background_item:
      self.background_item.props.visibility = goocanvas.ITEM_VISIBLE
    self.transformer_on.props.visibility = goocanvas.ITEM_INVISIBLE
    self.transformer.props.visibility = goocanvas.ITEM_VISIBLE
    self.prod_item_on.props.visibility = goocanvas.ITEM_INVISIBLE
    done = False
    self.current_prod_item = 0
    for item in self.prod_items:
      item.props.visibility = \
          goocanvas.ITEM_VISIBLE if not done else goocanvas.ITEM_INVISIBLE
      done = True
    self.counter.enable()
    # Is the power on
    self.power = False
    # Is the run is switched on
    self.is_on = False
    self.enabled = True

  def disable(self):
    if self.background_item:
      self.background_item.props.visibility = goocanvas.ITEM_INVISIBLE
    self.transformer_on.props.visibility = goocanvas.ITEM_INVISIBLE
    self.transformer.props.visibility = goocanvas.ITEM_INVISIBLE
    self.prod_item_on.props.visibility = goocanvas.ITEM_INVISIBLE
    for item in self.prod_items:
      item.props.visibility = goocanvas.ITEM_INVISIBLE
    self.counter.disable()
    self.enabled = False

  def update_run(self):
    if self.is_on and self.energy \
          and len(self.prod_items) > 2 \
          and self.timer == 0:
      self.anim_item()
    if self.is_on and self.power:
      self.production = self.power_count
    else:
      self.production = 0
    self.counter.set(self.production)

    self.update_prod_count()

  def on(self):
    if not self.energy:
      return
    self.prod_item_on.props.visibility = goocanvas.ITEM_VISIBLE
    self.is_on = True
    if len(self.prod_items) == 2:
      self.prod_items[0].props.visibility = goocanvas.ITEM_INVISIBLE
      self.prod_items[1].props.visibility = goocanvas.ITEM_VISIBLE

    self.update_run()

  def off(self):
    self.prod_item_on.props.visibility = goocanvas.ITEM_INVISIBLE
    self.is_on = False
    if len(self.prod_items) == 2:
      self.prod_items[0].props.visibility = goocanvas.ITEM_VISIBLE
      self.prod_items[1].props.visibility = goocanvas.ITEM_INVISIBLE
    self.power_off()
    self.update_run()

  def runbutton_item_event(self, widget, target, event):
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.off() if self.is_on else self.on()

  def power_off(self):
    self.transformer_on.props.visibility = goocanvas.ITEM_INVISIBLE
    self.power = False
    self.update_run()

  def power_on(self):
    if not self.is_on:
      return
    self.transformer_on.props.visibility = goocanvas.ITEM_VISIBLE
    self.power = True
    self.update_run()

  def powerbutton_item_event(self, widget, target, event):
    gcompris.sound.play_ogg("sounds/bleep.wav")
    if not self.energy:
      return
    self.power_off() if self.power else self.power_on()

  def set_energy(self, state):
    self.energy = state
    if not self.energy:
      self.off()

  # Simulate an animation
  def anim_item(self):
    self.prod_items[self.current_prod_item].props.visibility = \
        goocanvas.ITEM_INVISIBLE
    self.current_prod_item += 1
    if self.current_prod_item == len(self.prod_items):
      self.current_prod_item = 0
    self.prod_items[self.current_prod_item].props.visibility = \
        goocanvas.ITEM_VISIBLE
    if self.is_on and self.energy:
      self.timer = gobject.timeout_add(1000, self.anim_item)
    else:
      self.timer = 0

class Counter:
  # Pass the SVG IDs of the stuff to act on
  def __init__(self, rootitem, svghandle, svg_id, tooltip, x, y):
    self.item = goocanvas.Svg(
      parent = rootitem,
      svg_handle = svghandle,
      svg_id = svg_id,
      tooltip = "\n\n\n" + tooltip
      )
    self.text = goocanvas.Text(
      parent = rootitem,
      x = x,
      y = y,
      font = "Sans 8",
      text = "0W",
      tooltip = "\n\n\n" + tooltip
      )

  def set(self, value):
    self.text.set_properties(text = str(value) + "W")

  def enable(self):
    self.item.props.visibility = goocanvas.ITEM_VISIBLE
    self.text.props.visibility = goocanvas.ITEM_VISIBLE

  def disable(self):
    self.item.props.visibility = goocanvas.ITEM_INVISIBLE
    self.text.props.visibility = goocanvas.ITEM_INVISIBLE


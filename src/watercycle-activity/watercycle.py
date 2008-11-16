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
import gtk
import gtk.gdk
import rsvg

class Gcompris_watercycle:
  """The Water Cycle activity"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    # The basic tick for object moves
    self.timerinc = 50

    # Need to manage the timers to quit properly
    self.boat_timer = 0
    self.sun_timer = 0
    self.vapor_timer = 0
    self.cloud_timer = 0
    self.waterlevel_timer = 0

    gcompris.bar_set(0)
    gcompris.bar_location(gcompris.BOARD_WIDTH - 140, -1, 0.7)
    gcompris.bar_set_level(self.gcomprisBoard)

    gcompris.sound.play_ogg("sounds/Harbor1.wav", "sounds/Harbor3.wav")

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

    svghandle = gcompris.utils.load_svg("watercycle/watercycle.svgz")
    goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#BACKGROUND",
      hit_detection = False
      )

    # Take care, the items are stacked on each other in the order you add them.
    # If you need, you can reorder them later with raise and lower functions.

    # The River
    self.riveritem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#RIVERWATER"
      )
    self.riveritem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.riverfull = 0

    # The dirty water
    self.dirtywater = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#DIRTYWATER"
      )
    self.dirtywater.props.visibility = goocanvas.ITEM_INVISIBLE

    # The clean water
    self.cleanwateritem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#CLEANWATER"
      )
    self.cleanwateritem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.cleanwaterstatus = 0

    # The Sun
    self.sunitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SUN",
      )
    self.sunitem.connect("button_press_event", self.sun_item_event)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.sunitem, None)
    self.sun_direction = -1
    self.sun_on = 0

    # The Snow
    self.snowitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SNOW"
      )
    self.snowitem.props.visibility = goocanvas.ITEM_INVISIBLE

    # The rain
    self.rainitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#RAIN"
      )
    self.rainitem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.rain_on = 0

    # The cloud
    self.clouditem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#CLOUD"
      )
    self.clouditem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.clouditem.connect("button_press_event", self.cloud_item_event)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.clouditem, None)
    self.cloud_on = 0

    # The vapor
    self.vaporitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#VAPOR"
      )
    self.vaporitem.props.visibility = goocanvas.ITEM_INVISIBLE

    # The Waterpump
    self.waterpumpitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#PUMPSTATION"
      )
    self.waterpumpitem.connect("button_press_event", self.waterpump_item_event)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.waterpumpitem, None)
    self.waterpump_on = 0

    # The WaterCleaning
    self.watercleaningitem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#CLEANSTATION"
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
      svg_id = "#BOAT_PARKED"
      )
    self.boatitem_parked.props.visibility = goocanvas.ITEM_INVISIBLE

    # Tux in the shower (without water)
    self.tuxshoweritem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SHOWER"
      )
    self.tuxshoweritem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.shower_tux = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SHOWER_TUX"
      )
    self.shower_tux.props.visibility = goocanvas.ITEM_INVISIBLE

    # Tux in the shower with the water
    self.tuxshowerwateritem = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SHOWER_ON"
      )
    self.tuxshowerwateritem.props.visibility = goocanvas.ITEM_INVISIBLE

    # The shower on/off button (I need to get the 2 buttons to manage the focus)
    self.showerbuttonitem_on = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SHOWER_BUTTON_ON"
      )

    self.showerbuttonitem_on.connect("button_press_event", self.showerbutton_item_event)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.showerbuttonitem_on, None)
    self.showerbuttonitem_on.props.visibility = goocanvas.ITEM_INVISIBLE
    self.showerbutton = 0

    self.showerbuttonitem_off = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#SHOWER_BUTTON_OFF"
      )
    self.showerbuttonitem_off.connect("button_press_event", self.showerbutton_item_event)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.showerbuttonitem_off, None)

    # The level of water in the castle
    self.waterlevel_max = 65
    self.waterlevel_min = 85
    self.waterlevel     = self.waterlevel_min
    self.waterlevel_timer = gobject.timeout_add(1000, self.update_waterlevel)

    self.waterlevel_item = \
        goocanvas.Polyline(
      parent = self.rootitem,
      points = goocanvas.Points([(655 , self.waterlevel),
                                 (655 , self.waterlevel_min)]),
      stroke_color_rgba = 0x0033FFFFL,
      line_width = 20.0
      )

    goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#FOREGROUND",
      hit_detection = False
      )

    # Some item ordering
    self.rainitem.raise_(None)
    self.clouditem.raise_(None)

    # Ready GO
    target_x = 700
    trip_x = int(target_x - self.boatitem.get_bounds().x1)
    print self.boatitem.get_bounds().x1
    print trip_x
    self.boatitem.animate(target_x,
                          0,
                          1,
                          1,
                          True,
                          40*trip_x,
                          40,
                          goocanvas.ANIMATE_FREEZE)
    self.boat_timer = gobject.timeout_add(40 * trip_x + 4000, self.boat_arrived)


  def end(self):
    # Remove all the timer first
    if self.boat_timer :
      gobject.source_remove(self.boat_timer)
    if self.sun_timer :
      gobject.source_remove(self.sun_timer)
    if self.vapor_timer :
      gobject.source_remove(self.vapor_timer)
    if self.cloud_timer :
      gobject.source_remove(self.cloud_timer)
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

    if self.waterpump_on and self.waterlevel > self.waterlevel_max :
      self.waterlevel -= 1

    # It tux is in the shower and it works, then remove some water
    if (self.cleanwaterstatus and self.showerbutton
        and self.waterlevel < self.waterlevel_min) :
      self.waterlevel += 2

    # Redisplay the level of water if needed
    if old_waterlevel != self.waterlevel :
      self.waterlevel_item.props.points = goocanvas.Points([(655 , self.waterlevel),
                                                            (655 , self.waterlevel_min)])
    # Update the clean water tubes
    if self.waterlevel < self.waterlevel_min - 5 :
      self.set_cleanwater(1)
    else:
      self.set_cleanwater(0)
      # In this case, de activate the shower to avoid blinking tubes
      self.showerbuttonitem_on.props.visibility = goocanvas.ITEM_INVISIBLE
      self.showerbuttonitem_off.props.visibility = goocanvas.ITEM_VISIBLE
      self.showerbutton = 0
      self.shower_water_update()

    self.waterlevel_timer = gobject.timeout_add(1000, self.update_waterlevel)


  def set_cleanwater(self, status):
    if(status == self.cleanwaterstatus):
      return

    if status:
      self.cleanwateritem.props.pixbuf = gcompris.utils.load_pixmap("watercycle/cleanwater.png");
    else:
      self.cleanwateritem.props.pixbuf = gcompris.utils.load_pixmap("watercycle/cleanwater_off.png");
    self.cleanwaterstatus = status


  def boat_arrived(self):
    # We are parked, change the boat to remove tux
    self.boatitem_parked.props.visibility = goocanvas.ITEM_VISIBLE
#    self.boatitem.props.visibility = goocanvas.ITEM_INVISIBLE
    gcompris.sound.play_ogg("sounds/Harbor3.wav")

    # Now display tux in the shower
    self.tuxshoweritem.props.visibility = goocanvas.ITEM_VISIBLE
    self.shower_tux.props.visibility = goocanvas.ITEM_VISIBLE


  def move_cloud(self):
    if(self.cloud_on):
      self.clouditem.props.visibility = goocanvas.ITEM_VISIBLE
    else:
      self.clouditem.props.visibility = goocanvas.ITEM_INVISIBLE

    if(self.rain_on):
      self.rainitem.props.visibility = goocanvas.ITEM_VISIBLE
      # The snow appear if we are close to the left mountain
      if( self.clouditem.get_bounds().x1 < 250):
          self.snowitem.props.visibility = goocanvas.ITEM_VISIBLE
      # The river
      if(not self.riverfull):
        self.riveritem.props.pixbuf = gcompris.utils.load_pixmap("watercycle/riverfull.png");
        self.riverfull = 1

    else:
      self.rainitem.props.visibility = goocanvas.ITEM_INVISIBLE

    self.clouditem.translate(1, 0);
    gcompris.utils.item_focus_init(self.clouditem, None)
    self.rainitem.translate(1, 0);
    if( self.clouditem.get_bounds().x1 > 800 ) :
      self.clouditem.translate(-800, 0);
      self.rainitem.translate(-800, 0);
      self.cloud_on = 0
      self.rain_on = 0
      self.clouditem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.rainitem.props.visibility = goocanvas.ITEM_INVISIBLE
    else:
      self.cloud_timer = gobject.timeout_add(self.timerinc, self.move_cloud)

  def init_vapor(self):
    self.vapor_on = 1
    self.vaporitem.props.visibility = goocanvas.ITEM_VISIBLE
    self.move_vapor()

  def move_vapor(self):
    self.vaporitem.translate(0, -1);
    if( self.vaporitem.get_bounds().y1 < 20 ) :
      self.vaporitem.translate(0, +100);

    self.vapor_timer = gobject.timeout_add(self.timerinc, self.move_vapor)

  def move_sun(self):
    gcompris.utils.item_focus_init(self.sunitem, None)
    self.sunitem.translate(0, self.sun_direction);
    if( (self.sunitem.get_bounds().y1 > 0 and
         self.sunitem.get_bounds().y1 < 70 ) ) :
      self.sun_timer = gobject.timeout_add(self.timerinc, self.move_sun)
    else :
      # The sun is at is top
      if(self.sun_direction < 0) :
        # Stop the sun
        self.sun_timer = gobject.timeout_add(15000, self.move_sun)
        # Start the vapor
        self.vapor_timer = gobject.timeout_add(5000 , self.init_vapor)
        self.vapor_on = 1
        # Start the cloud
        if(not self.cloud_on):
          self.cloud_timer = gobject.timeout_add(10000, self.move_cloud)
          self.cloud_on = 1
        # Remove the snow
        self.snowitem.props.visibility = goocanvas.ITEM_INVISIBLE

      else :
        # Stop the vapor
        self.vapor_on = 0
        self.vaporitem.props.visibility = goocanvas.ITEM_INVISIBLE
        # Stop the sun
        self.sun_on = 0

      self.sun_direction = self.sun_direction * -1

  def pause(self, pause):
    pass

  def set_level(self, level):
    pass

  def sun_item_event(self, widget, target, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if not self.sun_on :
          gcompris.sound.play_ogg("sounds/bleep.wav")
          self.sun_timer = gobject.timeout_add(self.timerinc, self.move_sun)
          self.sun_on = 1
        return True
    return False

  def cloud_item_event(self, widget, target, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/Water5.wav")
        self.rain_on = 1
        return True
    return False

  def waterpump_item_event(self, widget, target, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.riverfull:
          gcompris.sound.play_ogg("sounds/bubble.wav")
          self.waterpump_on = 1
          self.pumpwateritem.props.pixbuf = gcompris.utils.load_pixmap("watercycle/pumpwater.png");
        return True
    return False

  def watercleaning_item_event(self, widget, target, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.waterpump_on:
          gcompris.sound.play_ogg("sounds/bubble.wav")
          self.watercleaning_on = 1
          self.dirtywater.props.pixbuf = gcompris.utils.load_pixmap("watercycle/badwater.png");
        return True
    return False


  # If Tux is in the shower, we must display the water if needed
  def shower_water_update(self):

    if self.cleanwaterstatus and self.showerbutton:
      self.tuxshoweritem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.tuxshowerwateritem.props.visibility = goocanvas.ITEM_VISIBLE
    else:
      self.tuxshoweritem.props.visibility = goocanvas.ITEM_VISIBLE
      self.tuxshowerwateritem.props.visibility = goocanvas.ITEM_INVISIBLE


  def showerbutton_item_event(self, widget, target, event=None):

    if not self.watercleaning_on:
      return

    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.showerbutton:
          gcompris.sound.play_ogg("sounds/bleep.wav")
          self.showerbuttonitem_on.props.visibility = goocanvas.ITEM_INVISIBLE
          self.showerbuttonitem_off.props.visibility = goocanvas.ITEM_VISIBLE
        else:
          gcompris.sound.play_ogg("sounds/apert2.wav")
          self.showerbuttonitem_on.props.visibility = goocanvas.ITEM_VISIBLE
          self.showerbuttonitem_off.props.visibility = goocanvas.ITEM_INVISIBLE

        self.showerbutton = not self.showerbutton

        self.shower_water_update()

#  gcompris - watercycle
# 
# Time-stamp: <2001/08/20 00:54:45 bruno>
# 
# Copyright (C) 2003 Bruno Coudoin
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
import gtk
import gtk.gdk

class Gcompris_watercycle:
  """The Cycle Water activity"""
  

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    
    print("Gcompris_watercycle __init__.")
  

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
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            "watercycle/background.png")
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    # Take care, the items are stacked on each other in the order you add them.
    # If you need, you can reorder them later with raise and lower functions.

    # The River
    self.riveritem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/riverempty.png"),
      x=150.0,
      y=50.0
      )
    self.riverfull = 0

    # The bad water
    self.badwateritem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/badwater_off.png"),
      x=360.0,
      y=292.0
      )

    # The clean water
    self.cleanwateritem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/cleanwater_off.png"),
      x=470.0,
      y=130.0
      )
    self.cleanwaterstatus = 0
    
    # The Sun
    self.sunitem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/sun.png"),
      x=10.0,
      y=70.0
      )
    self.sunitem.connect("event", self.sun_item_event)
    # This item is clickeable and it must be seen
    self.sunitem.connect("event", gcompris.utils.item_event_focus)
    self.sun_direction = -1
    self.sun_on = 0

    # The sun mask object to simulate the see
    self.rootitem.add(
      gnome.canvas.CanvasRect,
      x1=10.0,
      y1=89.0,
      x2=90.0,
      y2=155.0,
      fill_color_rgba=0x0099FFFFL,
      width_units=0.0
      )

    # The Snow
    self.snowitem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/snow.png"),
      x=180.0,
      y=3.0
      )
    self.snowitem.hide()

    # The rain
    self.rainitem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/rain.png"),
      x=40.0,
      y=70.0
      )
    self.rainitem.hide()
    self.rain_on = 0
    
    # The cloud
    self.clouditem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("gcompris/misc/cloud.png"),
      x=10.0,
      y=10.0
      )
    self.clouditem.hide()
    self.clouditem.connect("event", self.cloud_item_event)
    # This item is clickeable and it must be seen
    self.clouditem.connect("event", gcompris.utils.item_event_focus)
    self.cloud_on = 0
    
    # The vapor
    self.vaporitem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/vapor.png"),
      x=35.0,
      y=150.0
      )
    self.vaporitem.hide()

    # The Waterpump
    self.waterpumpitem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/waterpump.png"),
      x=165.0,
      y=120.0
      )
    self.waterpumpitem.connect("event", self.waterpump_item_event)
    # This item is clickeable and it must be seen
    self.waterpumpitem.connect("event", gcompris.utils.item_event_focus)
    self.waterpump_on = 0
    
    # The pump water
    self.pumpwateritem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/pumpwater_off.png"),
      x=270.0,
      y=133.0
      )

    # The WaterCleaning
    self.watercleaningitem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/watercleaning.png"),
      x=520.0,
      y=380.0
      )
    self.watercleaningitem.connect("event", self.watercleaning_item_event)
    # This item is clickeable and it must be seen
    self.watercleaningitem.connect("event", gcompris.utils.item_event_focus)
    self.watercleaning_on = 0

    # The tuxboat
    self.tuxboatitem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("gcompris/misc/tuxboat.png"),
      x=10.0,
      y=470.0
      )

    # Tux in the shower (without water)
    self.tuxshoweritem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("gcompris/misc/minitux.png"),
      x=569.0,
      y=239.0
      )
    self.tuxshoweritem.hide()
    self.tuxisinshower = 0
    
    # Tux in the shower with the water
    self.tuxshowerwateritem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/showerwater.png"),
      x=561.0,
      y=231.0
      )
    self.tuxshowerwateritem.hide()
    
    # The shower itself
    self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/shower.png"),
      x=560.0,
      y=283.0
      )

    # The shower on/off button (I need to get the 2 buttons to manage the focus)
    self.showerbuttonitem_on = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/shower_on.png"),
      x=622.0,
      y=260.0
      )
    self.showerbuttonitem_on.connect("event", self.showerbutton_item_event)
    # This item is clickeable and it must be seen
    self.showerbuttonitem_on.connect("event", gcompris.utils.item_event_focus)
    self.showerbuttonitem_on.hide()
    self.showerbutton = 0

    self.showerbuttonitem_off = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/shower_off.png"),
      x=622.0,
      y=260.0
      )
    self.showerbuttonitem_off.connect("event", self.showerbutton_item_event)
    # This item is clickeable and it must be seen
    self.showerbuttonitem_off.connect("event", gcompris.utils.item_event_focus)

    # The level of water in the castle
    self.waterlevel_max = 65
    self.waterlevel_min = 85
    self.waterlevel     = self.waterlevel_min
    self.waterlevel_timer = gtk.timeout_add(1000, self.update_waterlevel)

    self.waterlevel_item = self.rootitem.add(
          gnome.canvas.CanvasLine,
          points = (655 , self.waterlevel,
                    655 , self.waterlevel_min),
          fill_color_rgba = 0x0033FFFFL,
          width_units = 20.0
          )

    # Some item ordering
    self.rainitem.raise_to_top()
    self.clouditem.raise_to_top()
    
    # Ready GO
    self.move_boat()
    
    print("Gcompris_watercycle start.")

    
  def end(self):
    # Remove all the timer first
    if self.boat_timer :
      gtk.timeout_remove(self.boat_timer)
    if self.sun_timer :
      gtk.timeout_remove(self.sun_timer)
    if self.vapor_timer :
      gtk.timeout_remove(self.vapor_timer)
    if self.cloud_timer :
      gtk.timeout_remove(self.cloud_timer)
    if self.waterlevel_timer :
      gtk.timeout_remove(self.waterlevel_timer)
    
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

    print("Gcompris_watercycle end.")
        

  def ok(self):
    print("Gcompris_watercycle ok.")
          

  def repeat(self):
    print("Gcompris_watercycle repeat.")
            

  def config(self):
    print("Gcompris_watercycle config.")
              
  def key_press(self, keyval):
    print("got key %i" % keyval)
    return

  # This is called each second to update to castle water level
  def update_waterlevel(self):
    old_waterlevel = self.waterlevel
    
    if self.waterpump_on and self.waterlevel > self.waterlevel_max :
      self.waterlevel -= 1

    # It tux is in the shower and it works, then remove some water
    if (self.tuxisinshower and self.cleanwaterstatus and self.showerbutton
        and self.waterlevel < self.waterlevel_min) :
      self.waterlevel += 2

    # Redisplay the level of water if needed
    if old_waterlevel != self.waterlevel :
      self.waterlevel_item.set(
        points = (655, self.waterlevel,
                  655, self.waterlevel_min),
        )

    # Update the clean water tubes
    if self.waterlevel < self.waterlevel_min - 5 :
      self.set_cleanwater(1)
    else:
      self.set_cleanwater(0)
      # In this case, de activate the shower to avoid blinking tubes
      self.showerbuttonitem_on.hide()
      self.showerbuttonitem_off.show()
      self.showerbutton = 0
      self.shower_water_update()

    self.waterlevel_timer = gtk.timeout_add(1000, self.update_waterlevel)


  def set_cleanwater(self, status):
    if(status == self.cleanwaterstatus):
      return
    
    if status:
      self.cleanwateritem.set(pixbuf = gcompris.utils.load_pixmap("watercycle/cleanwater.png"));
    else:
      self.cleanwateritem.set(pixbuf = gcompris.utils.load_pixmap("watercycle/cleanwater_off.png"));
    self.cleanwaterstatus = status
    
    
  def move_boat(self):
    self.tuxboatitem.move(1, 0)
    if( self.tuxboatitem.get_bounds()[2] < 700 ) :
      self.boat_timer = gtk.timeout_add(self.timerinc, self.move_boat)
    else :
      if self.tuxboatitem.get_bounds()[2] < 790 :
        # Park the boat
        self.tuxboatitem.move(1, -1)
        self.boat_timer = gtk.timeout_add(self.timerinc, self.move_boat)
      else :
        # We are parked, change the boat to remove tux
        self.tuxboatitem.set(
          pixbuf = gcompris.utils.load_pixmap("gcompris/misc/fishingboat.png"),
          width = 100.0,
          height = 48.0,
          width_set = 1, 
          height_set = 1,
          )
        # Now display tux in the shower
        self.tuxshoweritem.show()
        self.tuxisinshower = 1

        
  def move_cloud(self):
    if(self.cloud_on):
      self.clouditem.show()
    else:
      self.clouditem.hide()
      
    if(self.rain_on):
      self.rainitem.show()
      # The snow appear if we are close to the left mountain
      if( self.clouditem.get_bounds()[0] < 250):
          self.snowitem.show()
      # The river
      if(not self.riverfull):
        self.riveritem.set(pixbuf = gcompris.utils.load_pixmap("watercycle/riverfull.png"));
        self.riverfull = 1
        
    else:
      self.rainitem.hide()

    self.clouditem.move(1, 0);
    self.rainitem.move(1, 0);
    if( self.clouditem.get_bounds()[0] > 800 ) :
      self.clouditem.move(-800, 0);
      self.rainitem.move(-800, 0);
      self.cloud_on = 0
      self.rain_on = 0
      self.clouditem.hide()
      self.rainitem.hide()
    else:
      self.cloud_timer = gtk.timeout_add(self.timerinc, self.move_cloud)
      
  def init_vapor(self):
    self.vapor_on = 1
    self.vaporitem.show()
    self.move_vapor()
    
  def move_vapor(self):
    self.vaporitem.move(0, -1);
    if( self.vaporitem.get_bounds()[1] < 20 ) :
      self.vaporitem.move(0, +100);

    self.vapor_timer = gtk.timeout_add(self.timerinc, self.move_vapor)
      
  def move_sun(self):
    self.sunitem.move(0, self.sun_direction);
    if( (self.sunitem.get_bounds()[1] > 0 and
         self.sunitem.get_bounds()[1] < 70 ) ) :
      self.sun_timer = gtk.timeout_add(self.timerinc, self.move_sun)
    else :
      # The sun is at is top
      if(self.sun_direction < 0) :
        # Stop the sun
        self.sun_timer = gtk.timeout_add(15000, self.move_sun)
        # Start the vapor
        self.vapor_timer = gtk.timeout_add(5000 , self.init_vapor)
        self.vapor_on = 1
        # Start the cloud
        if(not self.cloud_on):
          self.cloud_timer = gtk.timeout_add(10000, self.move_cloud)
          self.cloud_on = 1
        # Remove the snow
        self.snowitem.hide()

      else :
        # Stop the vapor
        self.vapor_on = 0
        self.vaporitem.hide()
        # Stop the sun
        self.sun_on = 0

      self.sun_direction = self.sun_direction * -1
      
  def pause(self, pause):  
    print("Gcompris_watercycle pause. %i" % pause)
                  
  def set_level(self, level):  
    print("Gcompris_watercycle set level. %i" % level)

  def sun_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if not self.sun_on :
          self.sun_timer = gtk.timeout_add(self.timerinc, self.move_sun)
          self.sun_on = 1
        return gtk.TRUE
    return gtk.FALSE

  def cloud_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        self.rain_on = 1
        return gtk.TRUE
    return gtk.FALSE

  def waterpump_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.riverfull:
          self.waterpump_on = 1
          self.pumpwateritem.set(pixbuf = gcompris.utils.load_pixmap("watercycle/pumpwater.png"));
        return gtk.TRUE
    return gtk.FALSE

  def watercleaning_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.waterpump_on:
          self.watercleaning_on = 1
          self.badwateritem.set(pixbuf = gcompris.utils.load_pixmap("watercycle/badwater.png"));
        return gtk.TRUE
    return gtk.FALSE


  # If Tux is in the shower, we must display the water if needed
  def shower_water_update(self):
    
    if not self.tuxisinshower:
      return

    if self.cleanwaterstatus and self.showerbutton:
      self.tuxshoweritem.hide()
      self.tuxshowerwateritem.show()
    else:
      self.tuxshoweritem.show()
      self.tuxshowerwateritem.hide()

  
  def showerbutton_item_event(self, widget, event=None):

    # Not active until tux is in the shower and the watercleaning station is running
    if not self.tuxisinshower:
      return

    if not self.watercleaning_on:
      return
    
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.showerbutton:
          self.showerbuttonitem_on.hide()
          self.showerbuttonitem_off.show()
        else:
          self.showerbuttonitem_on.show()
          self.showerbuttonitem_off.hide()
          
        self.showerbutton = not self.showerbutton

        self.shower_water_update()

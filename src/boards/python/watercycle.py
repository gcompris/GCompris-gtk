# PythonTest Board module
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
    self.canvasitems = {}
    
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
      fill_color_rgba=0x0099FFFF,
      width_units=0.0
      )

    # The tuxboat
    self.tuxboatitem = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("gcompris/misc/tuxboat.png"),
      x=10.0,
      y=470.0
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
      y=40.0
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
    return
  
  def move_boat(self):
    self.tuxboatitem.move(1, 0)
    if( self.tuxboatitem.get_bounds()[2] < 790 ) :
      self.boat_timer = gtk.timeout_add(self.timerinc, self.move_boat)
      
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
      
  def set_cleanwater(self):
    self.cleanwateritem.set(pixbuf = gcompris.utils.load_pixmap("watercycle/cleanwater.png"));


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
          # Set the water to the house after a timeout to simulate the water castle refill
          self.cleanwater_timer = gtk.timeout_add(1000, self.set_cleanwater)
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


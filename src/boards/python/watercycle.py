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

    # The Sun
    self.canvasitems[1] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/sun.png"),
      x=10.0,
      y=70.0
      )
    self.canvasitems[1].connect("event", self.sun_item_event)
    self.sun_direction = -1
    self.sun_on = 0

    # The sun mask object
    self.canvasitems[2] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasRect,
      x1=10.0,
      y1=89.0,
      x2=90.0,
      y2=155.0,
      fill_color_rgba=0x0099FFFF,
      width_units=0.0
      )

    # The tuxboat
    self.canvasitems[3] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("gcompris/misc/tuxboat.png"),
      x=10.0,
      y=470.0
      )

    # The rain
    self.canvasitems[5] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/rain.png"),
      x=40.0,
      y=40.0
      )
    self.canvasitems[5].hide()
    self.rain_on = 0
    
    # The cloud
    self.canvasitems[4] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("gcompris/misc/cloud.png"),
      x=10.0,
      y=10.0
      )
    self.canvasitems[4].hide()
    self.canvasitems[4].connect("event", self.cloud_item_event)
    self.cloud_on = 0
    
    # The vapor
    self.canvasitems[6] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("watercycle/vapor.png"),
      x=35.0,
      y=150.0
      )
    self.canvasitems[6].hide()

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
    
    # Remove the canvas item we added during this plugin
    for item in self.canvasitems.values():
      item.destroy()
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
    self.canvasitems[3].move(1, 0)
    if( self.canvasitems[3].get_bounds()[2] < 790 ) :
      self.boat_timer = gtk.timeout_add(self.timerinc, self.move_boat)
      
  def move_cloud(self):
    if(self.cloud_on):
      self.canvasitems[4].show()
    else:
      self.canvasitems[4].hide()
      
    if(self.rain_on):
      self.canvasitems[5].show()
    else:
      self.canvasitems[5].hide()

    self.canvasitems[4].move(1, 0);
    self.canvasitems[5].move(1, 0);
    if( self.canvasitems[4].get_bounds()[0] > 800 ) :
      self.canvasitems[4].move(-800, 0);
      self.canvasitems[5].move(-800, 0);
      self.cloud_on = 0
      self.show_rain = 0
      self.canvasitems[4].hide()
      self.canvasitems[5].hide()
    else:
      self.cloud_timer = gtk.timeout_add(self.timerinc, self.move_cloud)
      
  def move_vapor(self):
    if(self.vapor_on):
      self.canvasitems[6].show()
    else:
      self.canvasitems[6].hide()
      
    self.canvasitems[6].move(0, -1);
    if( self.canvasitems[6].get_bounds()[1] < 20 ) :
      self.canvasitems[6].move(0, +100);
    self.vapor_timer = gtk.timeout_add(self.timerinc, self.move_vapor)
      
  def move_sun(self):
    self.canvasitems[1].move(0, self.sun_direction);
    if( (self.canvasitems[1].get_bounds()[1] > 0 and
         self.canvasitems[1].get_bounds()[1] < 70 ) ) :
      self.sun_timer = gtk.timeout_add(self.timerinc, self.move_sun)
    else :
      if(self.sun_direction < 0) :
        # Stop the sun
        self.sun_timer = gtk.timeout_add(15000, self.move_sun)
        # Start the vapor
        self.vapor_timer = gtk.timeout_add(5000 , self.move_vapor)
        self.vapor_on = 1
        # Start the cloud
        if(not self.cloud_on):
          self.cloud_timer = gtk.timeout_add(10000, self.move_cloud)
          self.cloud_on = 1

      else :
        self.vapor_on = 0
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


# Ballcatch Board module
import gobject
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
from gettext import gettext as _

# ----------------------------------------
# Hit left shift and right shift together to send the ball straight

class Gcompris_ballcatch:
  """catch the ball"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    print("Gcompris_ballcatch __init__.")


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=9
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)

    # Event loop timer for the ball move
    self.ballinc = 20
    
    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    # Tux
    self.lefthand = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("gcompris/misc/tux.png"),
      x=gcompris.BOARD_WIDTH/2 - 60,
      y=60.0
      )

    # Balloon
    self.balloon_item = self.rootitem.add(
      gnome.canvas.CanvasEllipse,
      x1=0.0,
      y1=0.0,
      x2=0.0,
      y2=0.0
      )
    self.init_balloon()

    # The Left Hand
    self.lefthand = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("gcompris/misc/hand.png"),
      x=gcompris.BOARD_WIDTH/2-150.0,
      y=gcompris.BOARD_HEIGHT - 150
      )

    # The Right Hand
    item = self.lefthand = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("gcompris/misc/hand.png"),
      x=gcompris.BOARD_WIDTH/2+100.0,
      y=gcompris.BOARD_HEIGHT - 150.0
      )
    bounds = self.get_bounds(item)
    (cx, cy) = ( (bounds[2]+bounds[0])/2 , (bounds[3]+bounds[1])/2)
    mat = ( -1, 0, 0, 1, 2*cx, 0)
    item.affine_relative(mat)
    
    # For the game status WIN/LOOSE
    self.status = self.rootitem.add(
      gnome.canvas.CanvasText,
      x=gcompris.BOARD_WIDTH / 2,
      y=gcompris.BOARD_HEIGHT - 200,
      font=gcompris.skin.get_font("gcompris/content"),
      fill_color_rgba=0x123456FFL,
      justification=gtk.JUSTIFY_CENTER
      )

    # The basic tick for object moves
    self.timerinc = 1000
    
    self.timer_inc  = gtk.timeout_add(self.timerinc, self.timer_inc_display)

    self.counter_left  = 0
    self.counter_right = 0

    self.speed = self.rootitem.add(
      gnome.canvas.CanvasText,
      x=gcompris.BOARD_WIDTH / 2,
      y=gcompris.BOARD_HEIGHT - 280,
      font=gcompris.skin.get_font("gcompris/content"),
      text="Speed="+str(self.timerinc)+" ms",
      fill_color="white",
      justification=gtk.JUSTIFY_CENTER
      )

    self.textitem_left = self.rootitem.add(
      gnome.canvas.CanvasText,
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH / 3,
      y=gcompris.BOARD_HEIGHT - 240,
      fill_color_rgba=0xFFFFFFFFL
      )
    
    self.textitem_right = self.rootitem.add(
      gnome.canvas.CanvasText,
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH / 1.5,
      y=gcompris.BOARD_HEIGHT - 240,
      fill_color_rgba=0xFFFFFFFFL
      )

    self.left_continue  = True
    self.right_continue = True
    
    print("Gcompris_ballcatch start.")


  def end(self):

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

    if self.timer_inc :
      gtk.timeout_remove(self.timer_inc)


  def ok(self):
    print("Gcompris_ballcatch ok.")


  def repeat(self):
    print("Gcompris_ballcatch repeat.")


  def config(self):
    print("Gcompris_ballcatch config.")


  def key_press(self, keyval):
    print("Gcompris_ballcatch key press. %i" % keyval)

    win = False
    
    if (keyval == gtk.keysyms.Shift_L):
      self.left_continue  = False
    
    if (keyval == gtk.keysyms.Shift_R):
      self.right_continue = False

    if(not self.left_continue and not self.right_continue):
      if(self.counter_left == self.counter_right):
        self.status.set(text="GAGNE",
                        fill_color_rgba=0x2bf9f2FFL)
        win=True
      else:
        self.status.set(text="PERDU",
                        fill_color_rgba=0xFF0000FFL)
        
    if ((keyval == gtk.keysyms.BackSpace) or
        (keyval == gtk.keysyms.Delete)):
      self.timerinc = 1100
      keyval = 32
      
    if (keyval == 32):
      self.init_balloon()
      self.left_continue  = True
      self.right_continue = True
      self.counter_left  = 0
      self.counter_right = 0
      if(win):
        if(self.timerinc>200):
          self.timerinc -= 200
        elif(self.timerinc>100):
          self.timerinc -= 50
        elif(self.timerinc>10):
          self.timerinc -= 10
        elif(self.timerinc>1):
          self.timerinc -= 1

      if(self.timerinc<1):
          self.timerinc = 1
          
      self.status.set(text="")
      
      # Restart the timer
      self.timer_inc  = gtk.timeout_add(self.timerinc, self.timer_inc_display)

    self.speed.set(text="Speed="+str(self.timerinc)+" ms")
      
  def pause(self, pause):
    print("Gcompris_ballcatch pause. %i" % pause)


  def set_level(self, level):
    print("Gcompris_ballcatch set level. %i" % level)

# ---- End of Initialisation

  def timer_inc_display(self):

    if(self.left_continue):
      self.textitem_left.set(text=str(self.counter_left))
      self.counter_left += self.timer_inc

    if(self.right_continue):
      self.textitem_right.set(text=str(self.counter_right))
      self.counter_right += self.timer_inc

    if(self.left_continue or self.right_continue):
      self.timer_inc  = gtk.timeout_add(self.timerinc, self.timer_inc_display)
    else:
      # Send the ball now
      self.timer_diff = self.counter_right/1000-self.counter_left/1000
      self.timer_inc  = gtk.timeout_add(self.ballinc, self.ball_move)
      
  def ball_move(self):

    # The move simulation
    self.balloon_size -= 3
    self.balloon_x    += self.timer_diff
    self.balloon_y    -= 7

    if(self.balloon_width_units>1.0):
      self.balloon_width_units -= 0.5
    
    self.balloon_item.set(
      x1=self.balloon_x - self.balloon_size/2,
      y1=self.balloon_y - self.balloon_size/2,
      x2=self.balloon_x + self.balloon_size/2,
      y2=self.balloon_y + self.balloon_size/2,
      width_units=self.balloon_width_units
      )

    if(self.balloon_size>48):
      self.timer_inc  = gtk.timeout_add(self.ballinc, self.ball_move)


  def get_bounds(self, item):
    if gobject.type_name(item)=="GnomeCanvasPixbuf":
      x1=item.get_property("x")
      y1=item.get_property("y")
      x2=item.get_property("x")+item.get_property("width")
      y2=item.get_property("y")+item.get_property("height")
    return (min(x1,x2),min(y1,y2),max(x1,x2),max(y1,y2))

  def init_balloon(self):
    self.balloon_size = 160
    self.balloon_width_units = 5.0
    self.balloon_x    = gcompris.BOARD_WIDTH/2-20
    self.balloon_y    = gcompris.BOARD_HEIGHT - 130

    self.balloon_item.set(
      x1=self.balloon_x - self.balloon_size/2,
      y1=self.balloon_y - self.balloon_size/2,
      x2=self.balloon_x + self.balloon_size/2,
      y2=self.balloon_y + self.balloon_size/2,
      fill_color_rgba=0xc98d68FFL,
      outline_color_rgba=0x000000FFL,
      width_units=self.balloon_width_units
      )

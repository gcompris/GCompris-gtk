# PythonTest Board module
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
from gettext import gettext as _

class Gcompris_pythontest:
  """Testing gcompris python class"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.canvasitems = {}

    self.colors = {}
    self.colors['circle_in'] = gcompris.skin.get_color("pythontest/circle in")
    self.colors['circle_out'] = gcompris.skin.get_color("pythontest/circle out")
    self.colors['line'] = gcompris.skin.get_color("pythontest/line")

    self.movingline='none'

    print("Gcompris_pythontest __init__.")


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.canvasitems[1] = self.rootitem.add(
      gnome.canvas.CanvasEllipse,
      x1=300.0,
      y1=200.0,
      x2=280.0,
      y2=220.0,
      fill_color_rgba= self.colors['circle_in'],
      outline_color_rgba= self.colors['circle_out'],
      width_units=1.0
      )
    self.canvasitems[1].connect("event", self.circle_item_event)

    self.canvasitems[2] = self.rootitem.add(
      gnome.canvas.CanvasEllipse,
      x1=500.0,
      y1=200.0,
      x2=520.0,
      y2=220.0,
      fill_color_rgba= self.colors['circle_in'],
      outline_color_rgba= self.colors['circle_out'],
      width_units=1.0
      )
    self.canvasitems[2].connect("event", self.circle_item_event)

    self.canvasitems[3] = self.rootitem.add(
      gnome.canvas.CanvasText,
      x=400.0,
      y=100.0,
      text=_("This is the first plugin in gcompris coded in the Python\nProgramming language."),
      fill_color="white",
      justification=gtk.JUSTIFY_CENTER
      )

    self.canvasitems[4] = self.rootitem.add(
      gnome.canvas.CanvasText,
      x=400.0,
      y=140.0,
      text=_("It is now possible to develop gcompris activies in C or in Python.\n Thanks to Olivier Samys who makes this possible."),
      fill_color="white",
      justification=gtk.JUSTIFY_CENTER
      )

    self.canvasitems[5] = self.rootitem.add(
      gnome.canvas.CanvasText,
      x=400.0,
      y=250.0,
      text=_("This activity is not playable yet, just a test"),
      fill_color="white",
      justification=gtk.JUSTIFY_CENTER
      )

    #----------------------------------------
    # A simple game.
    # Try to hit left shift and right shift together. The peed increases
    self.rootitem.add(
          gnome.canvas.CanvasRect,
          x1=20,
          y1=gcompris.BOARD_HEIGHT-180,
          x2=gcompris.BOARD_WIDTH-20,
          y2=gcompris.BOARD_HEIGHT-10,
          fill_color_rgba=0x101077FFL,
          outline_color_rgba=0x07A3E080L,
          width_units=2.0)

    # For the game status WIN/LOOSE
    self.canvasitems[6] = self.rootitem.add(
      gnome.canvas.CanvasText,
      x=gcompris.BOARD_WIDTH / 2,
      y=gcompris.BOARD_HEIGHT - 40,
      font=gcompris.skin.get_font("gcompris/content"),
      fill_color_rgba=0x123456FFL,
      justification=gtk.JUSTIFY_CENTER
      )

    self.rootitem.add(
      gnome.canvas.CanvasText,
      x=400.0,
      y=400.0,
      text=("Test your reflex with the counter. Hit the 2 shifts key together.\nHit space to reset the counter and increase the speed.\nBackspace to reset the speed"),
      fill_color="white",
      justification=gtk.JUSTIFY_CENTER
      )

    # The basic tick for object moves
    self.timerinc = 1000
    
    self.timer_inc  = gtk.timeout_add(self.timerinc, self.timer_inc_display)

    self.counter_left  = 0
    self.counter_right = 0

    self.canvasitems[7] = self.rootitem.add(
      gnome.canvas.CanvasText,
      x=gcompris.BOARD_WIDTH / 2,
      y=gcompris.BOARD_HEIGHT - 80,
      font=gcompris.skin.get_font("gcompris/content"),
      text="Speed="+str(self.timerinc)+" ms",
      fill_color="white",
      justification=gtk.JUSTIFY_CENTER
      )

    self.textitem_left = self.rootitem.add(
      gnome.canvas.CanvasText,
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH / 3,
      y=gcompris.BOARD_HEIGHT - 40,
      fill_color_rgba=0xFFFFFFFFL
      )
    
    self.textitem_right = self.rootitem.add(
      gnome.canvas.CanvasText,
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH / 1.5,
      y=gcompris.BOARD_HEIGHT - 40,
      fill_color_rgba=0xFFFFFFFFL
      )

    self.left_continue  = True
    self.right_continue = True
    
    print("Gcompris_pythontest start.")


  def end(self):

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

    if self.timer_inc :
      gtk.timeout_remove(self.timer_inc)


  def ok(self):
    print("Gcompris_pythontest ok.")


  def repeat(self):
    print("Gcompris_pythontest repeat.")


  def config(self):
    print("Gcompris_pythontest config.")


  def key_press(self, keyval):
    print("Gcompris_pythontest key press. %i" % keyval)

    win = False
    
    if (keyval == gtk.keysyms.Shift_L):
      self.left_continue  = False
    
    if (keyval == gtk.keysyms.Shift_R):
      self.right_continue = False

    if(not self.left_continue and not self.right_continue):
      if(self.counter_left == self.counter_right):
        self.canvasitems[6].set(text="WIN",
                                fill_color_rgba=0x2bf9f2FFL)
        win=True
      else:
        self.canvasitems[6].set(text="LOOSE",
                                fill_color_rgba=0xFF0000FFL)
        
    if ((keyval == gtk.keysyms.BackSpace) or
        (keyval == gtk.keysyms.Delete)):
      self.timerinc = 1100
      keyval = 32
      
    if (keyval == 32):
      self.left_continue  = True
      self.right_continue = True
      self.counter_left  = 0
      self.counter_right = 0
      if(win):
        if(self.timerinc>500):
          self.timerinc -= 100
        elif(self.timerinc>200):
          self.timerinc -= 50
        elif(self.timerinc>10):
          self.timerinc -= 10
        elif(self.timerinc>1):
          self.timerinc -= 1

      if(self.timerinc<1):
          self.timerinc = 1
          
      self.canvasitems[3].set(text="")
      self.canvasitems[6].set(text="")

    self.canvasitems[7].set(text="Speed="+str(self.timerinc)+" ms")

    # Return  True  if you did process a key
    # Return  gtk.FALSE if you did not processed a key
    #         (gtk need to send it to next widget)
    return True

  def pause(self, pause):
    print("Gcompris_pythontest pause. %i" % pause)


  def set_level(self, level):
    print("Gcompris_pythontest set level. %i" % level)

# ---- End of Initialisation

  def timer_inc_display(self):

    if(self.left_continue):
      self.textitem_left.set(text=str(self.counter_left))
      self.counter_left += self.timer_inc

    if(self.right_continue):
      self.textitem_right.set(text=str(self.counter_right))
      self.counter_right += self.timer_inc

    self.timer_inc  = gtk.timeout_add(self.timerinc, self.timer_inc_display)


  def circle_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        bounds = widget.get_bounds()
        self.pos_x = (bounds[0]+bounds[2])/2
        self.pos_y = (bounds[1]+bounds[3])/2
        if 'line 1' in self.canvasitems:
          self.canvasitems['line 1'].destroy()
        self.canvasitems['line 1'] = self.rootitem.add(
          gnome.canvas.CanvasLine,
          points=( self.pos_x, self.pos_y, event.x, event.y),
          fill_color_rgba=self.colors['circle_in'],
          width_units=5.0
          )
        self.movingline='line 1'
        print "Button press"
        return True
    if event.type == gtk.gdk.MOTION_NOTIFY:
      if event.state & gtk.gdk.BUTTON1_MASK:
        self.canvasitems[self.movingline].set(
          points=( self.pos_x, self.pos_y, event.x, event.y)
          )
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        self.movingline='line 1'
        print "Button release"
        return True
    return gtk.FALSE


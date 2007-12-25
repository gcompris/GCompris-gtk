# Ballcatch Board module
import gobject
import goocanvas
import cairo
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.sound
import gtk
import gtk.gdk
from gettext import gettext as _

# ----------------------------------------
# Hit left shift and right shift together to send the ball straight

class Gcompris_ballcatch:
  """catch the ball"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.disable_im_context = True

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=9
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),"ballcatch/beach1.png")
    gcompris.bar_set_level(self.gcomprisBoard)


    self.ballinc    = 20        # Event loop timer for the ball move
    self.timer_diff = 0         # Store the time diff between left and right key

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

    # Tux
    goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("ballcatch/tux.png"),
      x=gcompris.BOARD_WIDTH/2 - 60,
      y=135.0
      )

    # Balloon
    self.balloon_item =goocanvas.Ellipse(
      parent = self.rootitem,
      )
    self.init_balloon()

    # The Left Hand
    goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("ballcatch/hand.png"),
      x = gcompris.BOARD_WIDTH/2 - 150.0,
      y = gcompris.BOARD_HEIGHT - 150
      )

    # The Right Hand (invert the left hand)
    item = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("ballcatch/hand.png"),
      )
    item.set_transform(cairo.Matrix( -1, 0, 0, 1,
                                      gcompris.BOARD_WIDTH/2 + 100.0,
                                      gcompris.BOARD_HEIGHT - 150))

    # The Left Shift KEY
    self.leftkey =goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("ballcatch/shift_key.png"),
      x=gcompris.BOARD_WIDTH/2-240.0,
      y=gcompris.BOARD_HEIGHT - 80
      )

    # The Right Shift KEY
    self.rightkey =goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("ballcatch/shift_key.png"),
      x=gcompris.BOARD_WIDTH/2+100.0,
      y=gcompris.BOARD_HEIGHT - 80
      )

    # The basic tick for object moves
    self.timerinc = 1000

    self.left_continue  = True
    self.right_continue = True

    self.counter_left  = 0
    self.counter_right = 0

    self.timer_inc  = gobject.timeout_add(self.timerinc,
                                          self.timer_inc_display)

  def end(self):

    if self.timer_inc:
      gobject.source_remove(self.timer_inc)

    # Remove the root item removes all the others inside it
    self.rootitem.remove()

  def ok(self):
    pass


  def repeat(self):
    pass


  def config(self):
    pass


  def key_press(self, keyval, commit_str, preedit_str):

    if (keyval == gtk.keysyms.Shift_L):
      self.left_continue  = False

    if (keyval == gtk.keysyms.Shift_R):
      self.right_continue = False

    return False

  # Called by gcompris core
  def pause(self, pause):

    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(pause == 0):
      self.next_level()
      self.gamewon = 0

    return


  # Called by gcompris when the user click on the level icon
  def set_level(self, level):
    self.gcomprisBoard.level=level
    self.gcomprisBoard.sublevel=1
    self.next_level()

  # End of Initialisation
  # ---------------------

  def next_level(self):

    # Set the level in the control bar
    gcompris.bar_set_level(self.gcomprisBoard);
    self.init_balloon()
    self.left_continue  = True
    self.right_continue = True
    self.counter_left  = 0
    self.counter_right = 0
    if self.timer_inc:
      gobject.source_remove(self.timer_inc)
    self.timer_inc = 0

    if(self.gcomprisBoard.level == 1):
      self.timerinc = 900
      gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),"ballcatch/beach1.png")
    elif(self.gcomprisBoard.level == 2):
      self.timerinc = 350
    elif(self.gcomprisBoard.level == 3):
      self.timerinc = 300
      gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),"ballcatch/beach2.png")
    elif(self.gcomprisBoard.level == 4):
      self.timerinc = 200
    elif(self.gcomprisBoard.level == 5):
      self.timerinc = 150
      gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),"ballcatch/beach3.png")
    elif(self.gcomprisBoard.level == 6):
      self.timerinc = 100
    elif(self.gcomprisBoard.level == 7):
      self.timerinc = 60
      gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),"ballcatch/beach4.png")
    elif(self.gcomprisBoard.level == 8):
      self.timerinc = 30
    elif(self.gcomprisBoard.level == 9):
      self.timerinc = 15

    if(self.timerinc<1):
      self.timerinc = 1

    # Restart the timer
    self.timer_inc  = gobject.timeout_add(self.timerinc,
                                          self.timer_inc_display)


  def timer_inc_display(self):

    if(self.left_continue):
      self.counter_left += self.timer_inc

    if(self.right_continue):
      self.counter_right += self.timer_inc

    if(self.left_continue or self.right_continue):
      self.timer_inc  = gobject.timeout_add(self.timerinc,
                                            self.timer_inc_display)
    else:
      gcompris.sound.play_ogg("sounds/brick.wav")
      # Send the ball now
      self.timer_diff = self.counter_right/1000 - self.counter_left/1000
      # Make some adjustment so that it cannot be too or too far close from the target
      # In between, the calculated value stay proportional to the error.
      if(self.timer_diff < -6):
        self.timer_diff = -6
      elif(self.timer_diff > 6):
        self.timer_diff = 6
      elif(self.timer_diff > -1.5 and self.timer_diff < 0 ):
        self.timer_diff = -1.5
      elif(self.timer_diff < 1.5 and self.timer_diff > 0 ):
        self.timer_diff = 1.5

      self.timer_inc = gobject.timeout_add(self.ballinc, self.ball_move)

  def ball_move(self):

    # The move simulation
    self.balloon_size -= 3
    self.balloon_x    += self.timer_diff
    self.balloon_y    -= 5

    if(self.balloon_line_width>1.0):
      self.balloon_line_width -= 0.5

    self.balloon_item.props.center_x = self.balloon_x
    self.balloon_item.props.center_y = self.balloon_y
    self.balloon_item.props.radius_x = self.balloon_size/2
    self.balloon_item.props.radius_y = self.balloon_size/2
    self.balloon_item.props.line_width = self.balloon_line_width


    if(self.balloon_size>48):
      self.timer_inc  = gobject.timeout_add(self.ballinc,
                                            self.ball_move)
    else:
      # We are done with the ballon move
      if(self.counter_left == self.counter_right):
        # This is a win
        if (self.increment_level() == 1):
          gcompris.sound.play_ogg("sounds/tuxok.wav")
          self.gamewon = 1
          gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)
      else:
        # This is a loose
        gcompris.sound.play_ogg("sounds/youcannot.wav")
        self.gamewon = 0
        gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.TUX)

  def get_bounds(self, item):
    if gobject.type_name(item) == "GooCanvasImage":
      x1 = item.get_property("x")
      y1 = item.get_property("y")
      x2 = item.get_property("x")+item.get_property("width")
      y2 = item.get_property("y")+item.get_property("height")
      return (min(x1,x2),min(y1,y2),max(x1,x2),max(y1,y2))
    return(None)

  def init_balloon(self):
    self.balloon_size = 160
    self.balloon_line_width = 5.0
    self.balloon_x    = gcompris.BOARD_WIDTH/2-20
    self.balloon_y    = gcompris.BOARD_HEIGHT - 130

    self.balloon_item.props.center_x = self.balloon_x
    self.balloon_item.props.center_y = self.balloon_y
    self.balloon_item.props.radius_x = self.balloon_size/2
    self.balloon_item.props.radius_y = self.balloon_size/2
    self.balloon_item.props.fill_color_rgba = 0xFF1212FFL
    self.balloon_item.props.stroke_color_rgba = 0x000000FFL
    self.balloon_item.props.line_width = self.balloon_line_width


  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return 1 if continue, 0 if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
        self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

    return 1


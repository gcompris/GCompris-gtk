# Follow Line Board module
import gobject
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gtk
import gtk.gdk
import random
from gettext import gettext as _

# ----------------------------------------
# 

class Gcompris_followline:
  """follow the line"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;

    print("Gcompris_followline __init__.")


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=9
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)


    self.ballinc    = 20        # Event loop timer for the ball move
    self.timer_diff = 0         # Store the time diff between left and right key
    
    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    # Start spot
    self.start_item = self.rootitem.add(
      gnome.canvas.CanvasEllipse,
      x1=0.0,
      y1=0.0,
      x2=0.0,
      y2=0.0
      )

    # Stop spot
    self.stop_item = self.rootitem.add(
      gnome.canvas.CanvasEllipse,
      x1=0.0,
      y1=0.0,
      x2=0.0,
      y2=0.0
      )
    
    # Line path
    self.path_item = self.rootitem.add(
      gnome.canvas.CanvasBpath
      )
    
    self.init_board()    
    
    print("Gcompris_followline start.")


  def end(self):

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()


  def ok(self):
    print("Gcompris_followline ok.")


  def repeat(self):
    print("Gcompris_followline repeat.")


  def config(self):
    print("Gcompris_followline config.")


  def key_press(self, keyval):
    print("Gcompris_followline key press. %i" % keyval)

      
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
    
    self.init_board()
    self.left_continue  = True
    self.right_continue = True
    self.counter_left  = 0
    self.counter_right = 0

    if(self.gcomprisBoard.level == 1):
      self.timerinc = 900
    elif(self.gcomprisBoard.level == 2):
      self.timerinc = 350
    elif(self.gcomprisBoard.level == 3):
      self.timerinc = 300
    elif(self.gcomprisBoard.level == 4):
      self.timerinc = 200
    elif(self.gcomprisBoard.level == 5):
      self.timerinc = 150
    elif(self.gcomprisBoard.level == 6):
      self.timerinc = 100
    elif(self.gcomprisBoard.level == 7):
      self.timerinc = 60
    elif(self.gcomprisBoard.level == 8):
      self.timerinc = 30
    elif(self.gcomprisBoard.level == 9):
      self.timerinc = 15

    if(self.timerinc<1):
      self.timerinc = 1
          

  def init_board(self):
    self.start_size = 160
    self.start_width_units = 5.0
    self.start_x    = self.start_size - 40
    self.start_y    = self.start_size/2 + random.randint(0, gcompris.BOARD_HEIGHT-self.start_size)

    self.stop_size = self.start_size
    self.stop_width_units = self.start_width_units
    self.stop_x    = gcompris.BOARD_WIDTH - self.stop_size + 20
    self.stop_y    = self.start_size/2 + random.randint(0, gcompris.BOARD_HEIGHT-self.start_size)

    # Line path
    self.path_def = gnome.canvas.path_def_new([(gnome.canvas.MOVETO_OPEN, self.stop_x, self.stop_y),
                                               (gnome.canvas.CURVETO,
                                                self.start_x + random.randint(10, gcompris.BOARD_WIDTH - 20),
                                                self.start_y + random.randint(10, gcompris.BOARD_HEIGHT - 20),
                                                self.start_x + random.randint(10, 50),
                                                self.start_y + random.randint(10, 50),
                                                self.start_x, self.start_y),
                                               (gnome.canvas.MOVETO_OPEN, self.start_x, self.start_y),])
    self.path_item.set(
      outline_color_rgba = 0x0C1010FFL,
      width_pixels  =  30
      )
    self.path_item.set_bpath(self.path_def)

    self.start_item.set(
      x1=self.start_x - self.start_size/2,
      y1=self.start_y - self.start_size/2,
      x2=self.start_x + self.start_size/2,
      y2=self.start_y + self.start_size/2,
      fill_color_rgba=0xFF1212FFL,
      outline_color_rgba=0x000000FFL,
      width_units=self.start_width_units
      )

    self.stop_item.set(
      x1=self.stop_x - self.stop_size/2,
      y1=self.stop_y - self.stop_size/2,
      x2=self.stop_x + self.stop_size/2,
      y2=self.stop_y + self.stop_size/2,
      fill_color_rgba=0xFF1212FFL,
      outline_color_rgba=0x000000FFL,
      width_units=self.stop_width_units
      )



  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return 1 if continue, 0 if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      if(self.gcomprisBoard.level>self.gcomprisBoard.maxlevel):
        # the current board is finished : bail out
        gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
        return 0
      
    return 1
        

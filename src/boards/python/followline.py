# Follow Line Board module
import gobject
import gnome
import gnome.canvas
import gcompris
import gcompris.skin
import gcompris.bonus
import gtk
import gtk.gdk
import random
import math
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
    self.background_item = gcompris.set_background(self.gcomprisBoard.canvas.root(),
                                                  gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    self.background_item_connect_id = self.background_item.connect("event", self.loosing_item_event)

    gcompris.bar_set_level(self.gcomprisBoard)

    self.init_board()    
    
    print("Gcompris_followline start.")


  def end(self):

    # Remove the root item removes all the others inside it
    self.cleanup()

    # Disconnect from the background item
    self.background_item.disconnect(self.background_item_connect_id)

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

  def cleanup(self):
    # Remove the root item removes all the others inside it
    self.state = "Done"

    self.rootitem.destroy()
    self.lines_group.destroy()
    
  def next_level(self):

    self.cleanup()
    
    # Set the level in the control bar
    gcompris.bar_set_level(self.gcomprisBoard);
    
    self.init_board()

          

  def init_board(self):

    # Number of hit outside of the line are allowed
    self.loosing_count = 20 - self.gcomprisBoard.level

    self.state = "Ready"

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    
    # Another group where we put each canvas line item in it
    self.lines_group = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    
    self.start_x    = 40
    self.start_y    = gcompris.BOARD_HEIGHT/2

    self.stop_x     = gcompris.BOARD_WIDTH - self.start_x
    self.stop_y     = gcompris.BOARD_HEIGHT/2

    # Line path (create several little line)
    min_boundary = 40
    y            = self.start_y
    line_width   = 45 - self.gcomprisBoard.level*2
    step         = (self.stop_x-self.start_x)/(50)

    frequency = 1 + int(self.gcomprisBoard.level/4)
      
    xpi = math.pi/2*frequency
    y   = self.start_y + math.cos(xpi)*(self.gcomprisBoard.level*10)
    for x in range(self.start_x, self.stop_x, step):
      
      xpi += (math.pi/2*frequency)/step
      y2 = self.start_y + math.cos(xpi)*(self.gcomprisBoard.level*10)
      
      # Check we stay within boundaries
      if(y2>=gcompris.BOARD_HEIGHT-min_boundary):
        y2=gcompris.BOARD_HEIGHT-min_boundary
      elif(y2<=min_boundary):
        y2=min_boundary

      item = self.lines_group.add(
        gnome.canvas.CanvasLine,
        points          =( x,
                           y,
                           x + step,
                           y2),
        fill_color_rgba = 0x0AA0F0FFL,
        width_units     = line_width,
        cap_style       = gtk.gdk.CAP_ROUND
        )
      item.connect("event", self.line_item_event)
      y = y2

    self.highlight_next_line()

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

  def highlight_next_line(self):
    for item in self.lines_group.item_list:
      if(item.get_data("gotit") != True):
        item.set(
          fill_color_rgba=0x11F212FFL,
          )
        item.set_data("iamnext", True);
        return

  def is_done(self):
    done = True
    for item in self.lines_group.item_list:
      if(item.get_data("gotit") != True):
        done = False

    if(done):
      # This is a win
      if (self.increment_level() == 1):
        self.state = "Done"
        self.gamewon = 1
        gcompris.bonus.display(1, gcompris.bonus.FLOWER)
      
    return done

  def loosing_item_event(self, widget, event=None):
    if(self.state == "Started"):
      self.loosing_count -= 1
      if(self.loosing_count<=0):
        self.state = "Done"
        self.gamewon = 0
        gcompris.bonus.display(0, gcompris.bonus.FLOWER)
        
      print self.loosing_count
      
    return gtk.FALSE


  def line_item_event(self, widget, event=None):
    if not self.board_paused and widget.get_data("iamnext") == True:
      # The first line touch means the game is started
      self.state = "Started"
      widget.set(
        fill_color_rgba=0x11F2F2FFL,
        )
      widget.set_data("gotit", True);
      widget.set_data("iamnext", False);
      self.highlight_next_line()
      self.is_done()
      
    return gtk.FALSE


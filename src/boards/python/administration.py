# Administration Board module
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
from gettext import gettext as _

# To add a panel, add a python module in the admin subdir
# This module must have a start() and end() function
class Gcompris_administration:
  """Administrating GCompris"""


  def __init__(self, gcomprisBoard):
    print "Gcompris_administration __init__"
    self.gcomprisBoard = gcomprisBoard

    # The panel being displayed
    self.current_panel = None

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
                    
    gap = 10
    panel_x = 120
    self.select_area = (gap , gap, panel_x + gap , gcompris.BOARD_HEIGHT-gap)
    self.panel_area  = (panel_x + 2*gap , gap, gcompris.BOARD_WIDTH-gap, gcompris.BOARD_HEIGHT-gap)

    print("Gcompris_administration __init__.")


  def start(self):

    # Find the list of modules in admin/
    # that means all files, but __init__.py and module.py
    list_modules = []
    
    import glob
    m_list = glob.glob(gcompris.DATA_DIR+'/../python/admin/module_*.py')
    for file in m_list:
      m_name = file.split('/')[-1].split('.')[0].split('_')[1]
      list_modules.append(m_name)

    print "Modules List : ", list_modules

    # Now import modules,
    # and get the list of string 'name.Name' for initialisation below.
    modules_init = []

    for module in list_modules:
      exec('from admin import module_' + module + ' as ' + module)
      modules_init.append(module+ '.' + module[0].upper() + module[1:])

    del list_modules
  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)

    self.rootitem.add(
      gnome.canvas.CanvasRect,
      x1=self.select_area[0],
      y1=self.select_area[1],
      x2=self.select_area[2],
      y2=self.select_area[3],
      fill_color="white",
      outline_color_rgba=0x111199FFL,
      width_units=1.0
      )

    self.rootitem.add(
      gnome.canvas.CanvasRect,
      x1=self.panel_area[0],
      y1=self.panel_area[1],
      x2=self.panel_area[2],
      y2=self.panel_area[3],
      fill_color="white",
      outline_color_rgba=0x111199FFL,
      width_units=1.0
      )

    # Display the menu in the selection area
    # The list of modules
    i = 0
    for module in modules_init:
      print module
      print module +'(self.rootitem).init(i, self.select_area, self.select_event)'
      exec(module +'(self.rootitem).init(i, self.select_area, self.select_event)')
      i+=1

    print("Gcompris_administration start.")


  def end(self):
    print("Gcompris_administration end.")


  def ok(self):
    print("Gcompris_administration ok.")


  def repeat(self):
    print("Gcompris_administration repeat.")


  def config(self):
    print("Gcompris_administration config.")


  def key_press(self, keyval):
    #print("Gcompris_administration key press. %i" % keyval)
    return gtk.FALSE

  # ---- End of Initialisation

  # Event when a tool is selected
  def select_event(self, item, event, module):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:

        # Do not reload it
        if(self.current_panel ==  module):
          return
        
        # Stop previous panel if any
        if(self.current_panel != None):
          self.current_panel.stop()
          
        self.current_panel = module

        # Start the new panel
        module.start(self.panel_area)
        

#  gcompris - administration.py
#
# Copyright (C) 2005, 2008 Bruno Coudoin and Yves Combe
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
# Administration Board module
import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
from gcompris import gcompris_gettext as _

# To add a panel, add a python module in the admin subdir
# This module must have a start() and end() function
class Gcompris_administration:
  """Administering GCompris"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    # The panel being displayed
    self.current_panel = None
    self.current_panel_stopped = False

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

    gapx = 6
    gapy = 10
    panel_x = 120
    self.select_area = (gapx, gapy, panel_x + gapx , gcompris.BOARD_HEIGHT-gapy)
    self.panel_area  = (panel_x + 2*gapx ,
                        gapy,
                        gcompris.BOARD_WIDTH-gapx,
                        gcompris.BOARD_HEIGHT-gapy)



  def pause(self, pause):
    if(self.current_panel != None):
      if pause:
        # Stop current panel
        self.current_panel.stop()
        self.current_panel_stopped = True
      else:
        # Restart the current panel
        self.current_panel.start(self.panel_area)
        self.current_panel_stopped = False


  def start(self):

    # Find the list of modules in admin/
    # that means all files, but __init__.py and module.py
    list_modules = []

    import glob
    m_list = glob.glob('admin/module_*.py')
    if m_list == []:
      m_list = glob.glob(gcompris.PYTHON_PLUGIN_DIR+'/admin/module_*.py')
    for file in m_list:
      m_name = file.split('/')[-1].split('.')[0].split('_')[1]
      list_modules.append(m_name)

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
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)
    gcompris.bar_location(25, -1, 0.6)

    goocanvas.Rect(
      parent = self.rootitem,
      x = self.select_area[0],
      y = self.select_area[1],
      width = self.select_area[2] - self.select_area[0],
      height = self.select_area[3] - self.select_area[1],
      fill_color = "white",
      stroke_color_rgba=0x111199FFL,
      line_width=1.0
      )

    # A White background in the panel area

    goocanvas.Rect(
      parent = self.rootitem,
      x = self.panel_area[0],
      y = self.panel_area[1],
      width = self.panel_area[2] - self.panel_area[0],
      height = self.panel_area[3] - self.panel_area[1],
      fill_color="white",
      stroke_color_rgba=0x111199FFL,
      line_width=1.0
      )

    # Get the menu position for each module
    # The list of modules
    modules_ordered = []
    for module in modules_init:
      p = eval(module +'(self.rootitem).position()')
      modules_ordered.append((p, module))

    # Display the menu in the selection area
    # The list of modules
    i = 0
    modules_ordered.sort()
    for tmodule in modules_ordered:
      module = tmodule[1]
      eval(module +'(self.rootitem).init(i, self.select_area, self.select_event)')
      i+=1


  def end(self):
    # Stop current panel
    if(self.current_panel != None and not self.current_panel_stopped):
      self.current_panel.stop()
      self.current_panel_stopped = False

  def ok(self):
    pass


  def repeat(self):
    pass


  def config(self):
    pass


  def key_press(self, keyval, commit_str, preedit_str):
    return False

  # ---- End of Initialisation

  # Event when a tool is selected
  def select_event(self, item, target, event, module):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:

        # Do not reload it
        if(self.current_panel ==  module):
          return

        # Stop previous panel if any
        if(self.current_panel != None):
          self.current_panel.stop()
          self.current_panel_stopped = False

        self.current_panel = module

        # Start the new panel
        module.start(self.panel_area)


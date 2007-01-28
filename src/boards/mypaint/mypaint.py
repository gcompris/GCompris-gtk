# MyPaint Board module
import gnomecanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gtk
import gtk.gdk
import random

from gcompris import gcompris_gettext as _

class Gcompris_mypaint:
  """MyPaint draw board"""

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    pass

  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(gcompris.BAR_CONFIG)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)

    import sys, os
    sys.path.insert(0, os.path.join(gcompris.get_properties().package_plugin_dir , 'pythonlib'))
    sys.path.append(os.path.join(gcompris.get_properties().package_python_plugin_dir , 'mypaint'))

    print sys.path
    import mydrawwidget

  def end(self):
    pass

  def ok(self):
    pass
    
  def config(self):
    print("Gcompris_pythontest config.")


  def key_press(self, keyval, commit_str, preedit_str):
    print("Gcompris_pythontest key press. %i %s %s" % (keyval, commit_str, preedit_str))

  def pause(self, pause):
    pass

  ###################################################
  # Configuration system
  ###################################################

  #mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    # keep profile in mind
    # profile can be Py_None
    self.configuring_profile = profile


import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
from gettext import gettext as _

import module

class Profiles(module.Module):
  """Administrating GCompris Profiles"""


  def __init__(self, canvas):
    print("Gcompris_administration __init__ profiles panel.")
    module.Module.__init__(self, canvas, "profiles", _("Profiles list"))

  def start(self, area):
    print "starting profiles panel"

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.

    self.rootitem = self.canvas.add(
        gnome.canvas.CanvasGroup,
        x=0.0,
        y=0.0
        )

    module.Module.start(self)

    item = self.rootitem.add (
        gnome.canvas.CanvasText,
        text=_(self.module_label + " Panel"),
        font=gcompris.skin.get_font("gcompris/content"),
        x = area[0] + (area[2]-area[0])/2,
        y = area[1] + 50,
        fill_color="black"
        )

  def stop(self):
    print "stopping profiles panel"
    module.Module.stop(self)

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()



import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
from gettext import gettext as _

import module

class Boards(module.Module):
  """Administrating GCompris Boards"""


  def __init__(self, canvas):
    print("Gcompris_administration __init__ boards panel.")
    module.Module.__init__(self, canvas, "boards", _("Boards"))

  def start(self, area):
    print "starting boards panel"

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

    boards = []
    for board in gcompris.get_boards_list():
      if board.is_configurable:
        boards.append(board)

    print "Configurable boards :",
    for board in boards:
      print board.section, board.name, board.id

  def stop(self):
    print "stopping boards panel"
    module.Module.stop(self)

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()


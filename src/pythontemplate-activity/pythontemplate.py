#  gcompris - pythontemplate.py
#
# Copyright (C) 2003, 2008 Bruno Coudoin
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
# pythontemplate activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas
import pango

from gcompris import gcompris_gettext as _

#
# The name of the class is important. It must start with the prefix
# 'Gcompris_' and the last part 'pythontemplate' here is the name of
# the activity and of the file in which you put this code. The name of
# the activity must be used in your menu.xml file to reference this
# class like this: type="python:pythontemplate"
#
class Gcompris_pythontemplate:
  """Empty gcompris Python class"""


  def __init__(self, gcomprisBoard):
    print "pythontemplate init"

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    print "pythontemplate start"

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)

    # Set a background image
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    goocanvas.Text(
      parent = self.rootitem,
      x=400.0,
      y=100.0,
      text=_("This is the first plugin in GCompris coded in the Python\n"
             "Programming language."),
      fill_color="black",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

  def end(self):
    print "pythontemplate end"
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("pythontemplate ok.")


  def repeat(self):
    print("pythontemplate repeat.")


  #mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    print("pythontemplate config_start.")

  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_pythontemplate key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    print("pythontemplate pause. %i" % pause)


  def set_level(self, level):
    print("pythontemplate set level. %i" % level)


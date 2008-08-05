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
# PythonTemplate Board module
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas

class Gcompris_pythontemplate:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    print "template init"

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    print "template start"

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))


  def end(self):
    print "template end"


  def ok(self):
    print("template ok.")


  def repeat(self):
    print("template repeat.")


  def config(self):
    print("template config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_pythontemplate key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    print("template pause. %i" % pause)


  def set_level(self, level):
    print("template set level. %i" % level)


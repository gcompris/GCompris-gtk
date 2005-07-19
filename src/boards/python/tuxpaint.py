#  gcompris - Tuxpaint Launcher
# 
#  Copyright (C) 2004  Yves Combe
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.bonus
import gcompris.skin
import gcompris.sound
import gcompris.admin
import gtk
import os

#import gobject
from gettext import gettext as _

pid = None

class Gcompris_tuxpaint:
  """TuxPaint Launcher"""
  global pid
  
  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    pass
    
  def start(self):
#    line = os.sys.stdin.readline()
#    while (len(line) >1):
#      exec(line)
#      line = os.sys.stdin.readline()

    Prop = gcompris.get_properties()
    
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    options = ['tuxpaint']
    if Prop.fullscreen:
      options.append('--fullscreen')

    gcompris.sound.close()

    try:
      # gobject.SPAWN_DO_NOT_REAP_CHILD|gobject.SPAWN_SEARCH_PATH = 2 | 4
      pid,stdin, stdout, stderr = gcompris.spawn_async(
        options, flags=2|4)
    except:
      gcompris.utils.dialog(_("Cannot find Tuxpaint. \n Install it to use this board !"),stop_board)
      return 
    
    gcompris.child_watch_add(pid, child_callback, None)

    gcompris.bar_set(0)
    gcompris.bar_hide(1)

    gcompris.set_background(self.rootitem,
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))

    textItem = self.rootitem.add(
      gnome.canvas.CanvasText,
      text = _("Waiting Tuxpaint Finished"), 
      x = gcompris.BOARD_WIDTH/2, 
      y = 185,
      fill_color_rgba = 0x000000ffL, 
      font = gcompris.skin.get_font("gcompris/board/huge bold"),
      )

  def end(self):
    gcompris.sound.reopen()
    global pid
    # force kill - data loss
    if pid != None:
      print "Tuxpaint not killed", self.pid
    #import os
    #os.kill(self.pid, signal.SIGKILL)
    if self.rootitem != None: 
      self.rootitem.destroy()
      self.rootitem = None

  def set_level(self,level):
    pass
        
  def ok(self):
    pass

  def key_press(self, keyval):
    return False

  def repeat(self):
    pass

  def pause(self, pause):
    pass

  def config(self):
    print "Config"
    pass

  def config_start(self, canvasgroup, x, y, w, h):
    print "config_start", self, canvasgroup, x, y, w, h
    print "Configurables boards:"
    for board in gcompris.get_boards_list():
      if board.is_configurable:
        print "    ", board.name
    print dir(gcompris)
    pass

  def config_stop(self):
    print "config_stop", self
    pass

def child_callback(fd,  cond, data):
  gcompris.sound.reopen()
  global pid
  pid = None
  gcompris.bar_hide(0)
  gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
  
def stop_board():
  gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)

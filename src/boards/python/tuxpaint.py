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
#board = None

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

    self.window = self.gcomprisBoard.canvas.get_toplevel()

    #global board
    #board = self
    
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


    #self.window.set_property("accept-focus", 0)
    #self.window.set_keep_below(False)
    
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


  def config_stop(self):
    print "config_stop", self
    self.config_window.destroy()
    pass

  def config_start(self, profile, window):
    self.config_window = window
    print "config_start", self.gcomprisBoard.name, profile.name, window
    
    button_close = gtk.Button(stock=gtk.STOCK_CLOSE)
    button_close.connect("clicked", self.configuration_close)
    button_close.show()
    
    button_apply = gtk.Button(stock=gtk.STOCK_APPLY)
    button_apply.connect("clicked", self.configuration_apply)
    button_apply.show()

    main_box = gtk.VBox(False, 8)
    main_box.show()
    window.add(main_box) 
    
    box_bottom = gtk.HBox(False, 0)
    box_bottom.show()
    main_box.pack_end(box_bottom, False, False, 0)

    box_bottom.pack_end(button_close, False, False, 0)
    box_bottom.pack_start(button_apply, False, False, 0)
    
    pass

  def configuration_close(self, button):
    self.config_window.destroy()

  def configuration_apply(self, button):
    pass


def child_callback(fd,  cond, data):
  #global board
  #board.window.set_property("accept-focus", 1)
  #board.window.set_keep_above(False)
  gcompris.sound.reopen()
  global pid
  pid = None
  gcompris.bar_hide(0)
  gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
  
def stop_board():
  gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)

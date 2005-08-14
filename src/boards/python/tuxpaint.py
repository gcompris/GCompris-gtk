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

import pango

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

  def configuration(self, value):
    if self.config_dict.has_key(value):
      return eval(self.config_dict[value])
    else:
      return False

  def start(self):
#    line = os.sys.stdin.readline()
#    while (len(line) >1):
#      exec(line)
#      line = os.sys.stdin.readline()

    self.window = self.gcomprisBoard.canvas.get_toplevel()

    #global board
    #board = self
    
    Prop = gcompris.get_properties()

    self.config_dict = gcompris.get_board_conf()
    
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    options = ['tuxpaint']

    if (Prop.fullscreen and self.configuration('fullscreen')):
      options.append('--fullscreen')

    if self.configuration('disable_shape_rotation'):
      options.append('--simpleshapes')

    if self.configuration('uppercase_text'):
      options.append('--uppercase')

    if self.configuration('disable_stamps'):
      options.append('--nostamps')

    if self.configuration('disable_stamps_control'):
      options.append('--nostampcontrols')
  
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

  def config_start(self, profile):
    self.configure_profile = profile
    
    self.config_values = {}
    self.main_vbox = gcompris.configuration_window(_('<b>%s</b> configuration\n for profile <b>%s</b>') % ('Tuxpaint', profile.name ),
                                                   self.apply_callback)

    self.config_dict = gcompris.get_conf(profile, self.gcomprisBoard)

    gcompris.boolean_box('Disable shape rotation', 'disable_shape_rotation', self.configuration('disable_shape_rotation'))
    gcompris.boolean_box('Follow gcompris fullscreen', 'fullscreen', self.configuration('fullscreen'))
    gcompris.boolean_box('Show Uppercase text only', 'uppercase_text', self.configuration('uppercase_text'))
    stamps = gcompris.boolean_box('Disable stamps', 'disable_stamps', self.configuration('disable_stamps'))
    
    self.stamps_control = gcompris.boolean_box('Disable stamps control', 'disable_stamps_control', self.configuration('disable_stamps_control'))
    self.stamps_control.set_sensitive(self.configuration('disable_stamps'))
      

    gcompris.combo_box('ComboBox Test ', ['red', 'blue', 'green'], 'combo_test', 1)

    gcompris.radio_buttons('<b>Radio Buttons</b> Test',
                           'radio_test',
                           { 'red': 'Red Carpet',
                             'blue': 'Blue Hotel',
                             'green': 'Yellow Submarine'
                             },
                           'green'
                           )

    stamps.connect("toggled", self.stamps_changed)

    gcompris.spin_int('<b>Spin Button</b> Test',
                      'spin_test',
                      -12,
                      12,
                      2,
                      0)
                     
  def stamps_changed(self, button):
    self.stamps_control.set_sensitive(button.get_active())

  def apply_callback(self,table):
    print table
    for key,value in table.iteritems():
      gcompris.set_board_conf(self.configure_profile, self.gcomprisBoard, key, value)


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
  

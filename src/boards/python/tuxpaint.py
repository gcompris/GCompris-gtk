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

  def configuration(self, value, init):
    if self.config_dict.has_key(value):
      return eval(self.config_dict[value])
    else:
      return init

  def start(self):
#    line = os.sys.stdin.readline()
#    while (len(line) >1):
#      exec(line)
#      line = os.sys.stdin.readline()

    self.window = self.gcomprisBoard.canvas.get_toplevel()

    #global board
    #board = self
    
    Prop = gcompris.get_properties()

    #get default values
    self.config_dict = self.init_config()

    #replace configured values
    self.config_dict.update(gcompris.get_board_conf())
    
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    options = ['tuxpaint']

    if (Prop.fullscreen and eval(self.config_dict['fullscreen'])):
      options.append('--fullscreen')

    # tuxpaint size are 800x600 and 640x480 in 9.14
    # in cvs (future 9.15) it will be
    # [--640x480   | --800x600   | --1024x768 |
    #  --1280x1024 | --1400x1050 | --1600x1200]
    if (Prop.screensize and eval(self.config_dict['size'])):
      if (Prop.screensize >=1):
        print 'Prop.screensize', Prop.screensize
        options.append('--800x600')
      else:
        options.append('--640x480')

    if eval(self.config_dict['disable_shape_rotation']):
      options.append('--simpleshapes')

    if eval(self.config_dict['uppercase_text']):
      options.append('--uppercase')

    if eval(self.config_dict['disable_stamps']):
      options.append('--nostamps')

    if eval(self.config_dict['disable_stamps_control']):
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
      text = _("Waiting for Tuxpaint finishes"), 
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

  def key_press(self, keyval, commit_str, preedit_str):
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
    pass

  def config_start(self, profile):
    self.configure_profile = profile

    #get default values
    self.config_dict = self.init_config()

    #set already configured values
    self.config_dict.update(gcompris.get_conf(profile, self.gcomprisBoard))

    self.main_vbox = gcompris.configuration_window(_('<b>%s</b> configuration\n for profile <b>%s</b>') % ('Tuxpaint', profile.name ),
                                                   self.apply_callback)


    gcompris.boolean_box(_('Inherit fullscreen setting from GCompris'), 'fullscreen', eval(self.config_dict['fullscreen']))

    gcompris.separator()

    gcompris.boolean_box(_('Inherit size setting from GCompris (800x600, 640x480)'), 'size', eval(self.config_dict['size']))

    gcompris.separator()

    gcompris.boolean_box(_('Disable shape rotation'), 'disable_shape_rotation', eval(self.config_dict['disable_shape_rotation']))

    gcompris.separator()

    gcompris.boolean_box(_('Show Uppercase text only'), 'uppercase_text', eval(self.config_dict['uppercase_text']))

    gcompris.separator()

    stamps = gcompris.boolean_box(_('Disable stamps'), 'disable_stamps', eval(self.config_dict['disable_stamps']))
    stamps.connect("toggled", self.stamps_changed)
    
    self.stamps_control = gcompris.boolean_box('Disable stamps control', 'disable_stamps_control', eval(self.config_dict['disable_stamps_control']))
    self.stamps_control.set_sensitive(not eval(self.config_dict['disable_stamps']))
     
  def stamps_changed(self, button):
    self.stamps_control.set_sensitive(not button.get_active())

  def apply_callback(self,table):
    print table
    for key,value in table.iteritems():
      gcompris.set_board_conf(self.configure_profile, self.gcomprisBoard, key, value)

  def init_config(self):
    default_config_dict = { 'fullscreen'             : 'True',
                            'disable_shape_rotation' : 'False',
                            'uppercase_text'         : 'False',
                            'disable_stamps'         : 'False',
                            'disable_stamps_control' : 'False',
                            'size'                   : 'True'
      }
    return default_config_dict

def child_callback(fd,  cond, data):
  #global board
  #board.window.set_property("accept-focus", 1)
  #board.window.set_keep_above(False)
  gcompris.sound.reopen()

  #a bug in tuxpaint 9.14: it does not suppress it lockfile
  lockfile = os.getenv('HOME') + '/.tuxpaint/lockfile.dat'

  try:
    os.remove(lockfile)
  except:
    print lockfile, 'not removed.'

  global pid
  pid = None
  gcompris.bar_hide(0)
  gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
  
def stop_board():
  gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
  

#  gcompris - Tuxpaint Launcher
#
#  Copyright (C) 2004, 2008  Yves Combe
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
import goocanvas
import gcompris
import gcompris.utils
import gcompris.bonus
import gcompris.skin
import gcompris.sound
import gcompris.admin
import gtk
import os
import gobject
import pango
import platform

#gettext support
import gettext
from gcompris import gcompris_gettext as _
from gettext import dgettext as D_

pid = None
fles = None

class Gcompris_tuxpaint:
  """TuxPaint Launcher"""

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.rootitem = None
    # global parameter to access object structures from global fonctions
    global fles
    fles=self
    pass

  def configuration(self, value, init):
    if self.config_dict.has_key(value):
      return eval(self.config_dict[value])
    else:
      return init

  def start(self):
    progname='tuxpaint'
    tuxpaint_dir = None
    flags = gobject.SPAWN_DO_NOT_REAP_CHILD | gobject.SPAWN_SEARCH_PATH

    print platform.platform(), platform.platform().split('-')[0]
    if (platform.platform().split('-')[0] == 'Windows'):
      progname = 'tuxpaint.exe'

      try:
         import _winreg

         tuxpaint_key = _winreg.OpenKey( _winreg.HKEY_LOCAL_MACHINE,
                                         "Software\\TuxPaint" )
         tuxpaint_dir, type = _winreg.QueryValueEx(tuxpaint_key, "Install_Dir")
         flags = gobject.SPAWN_DO_NOT_REAP_CHILD
         # escape mandatory in Win pygtk2.6
         tuxpaint_dir = '"' + tuxpaint_dir + '"'

      except:
	   pass

    self.window = self.gcomprisBoard.canvas.get_toplevel()

    Prop = gcompris.get_properties()

    #get default values
    self.config_dict = self.init_config()

    #replace configured values
    self.config_dict.update(gcompris.get_board_conf())

    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())

    options = [progname]

    if (Prop.fullscreen and eval(self.config_dict['fullscreen'])):
      options.append('--fullscreen')

    if eval(self.config_dict['disable_shape_rotation']):
      options.append('--simpleshapes')

    if eval(self.config_dict['uppercase_text']):
      options.append('--uppercase')

    if eval(self.config_dict['disable_stamps']):
      options.append('--nostamps')

    if eval(self.config_dict['disable_stamps_control']):
      options.append('--nostampcontrols')

    gcompris.sound.close()

    global pid
    try:
       # bug in working_directory=None ?
       if (tuxpaint_dir):
          pid, stdin, stdout, stderr = gobject.spawn_async(
            argv=options,
            flags=flags,
            working_directory=tuxpaint_dir)

       else:
          pid, stdin, stdout, stderr = gobject.spawn_async(
            argv=options,
            flags=flags)

    except:
       gcompris.utils.dialog(D_(gcompris.GETTEXT_ERRORS,"Cannot find Tuxpaint.\nInstall it to use this activity !"),stop_board)
       return

    gobject.child_watch_add(pid, child_callback, data=self, priority=gobject.PRIORITY_HIGH)

    gcompris.bar_set(gcompris.BAR_CONFIG)
    gcompris.bar_hide(1)

    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    textItem = goocanvas.Text(
      parent = self.rootitem,
      text = _("Waiting for Tuxpaint to finish"),
      x = gcompris.BOARD_WIDTH/2,
      y = 185,
      fill_color_rgba = 0x000000ffL,
      anchor = gtk.ANCHOR_CENTER,
      font = gcompris.skin.get_font("gcompris/board/title bold"),
      )

  def end(self):
    gcompris.sound.reopen()
    global pid
    # force kill - data loss
    if pid != None:
      print "Tuxpaint not killed", pid
    #import os
    #os.kill(self.pid, signal.SIGKILL)
    if self.rootitem != None:
      self.rootitem.remove()
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
    pass

  def config_stop(self):
    pass

  def config_start(self, profile):
    self.configure_profile = profile

    #get default values
    self.config_dict = self.init_config()

    #set already configured values
    self.config_dict.update(gcompris.get_conf(profile, self.gcomprisBoard))

    bconfig = gcompris.configuration_window ( \
      D_(gcompris.GETTEXT_GUI, \
        '<b>{activity}</b> configuration\n for profile <b>{profile}</b>'.format( \
                        activity=_('Tuxpaint'),
                        # This is the name of the Default user profile
                        profile=profile.name if profile else D_(gcompris.GETTEXT_GUI,"Default")),
        ),
        self.apply_callback
      )


    gcompris.boolean_box(bconfig, D_(gcompris.GETTEXT_GUI,'Inherit fullscreen setting from GCompris'), 'fullscreen', eval(self.config_dict['fullscreen']))

    gcompris.separator(bconfig)

    gcompris.boolean_box(bconfig, D_(gcompris.GETTEXT_GUI,'Inherit size setting from GCompris (800x600, 640x480)'), 'size', eval(self.config_dict['size']))

    gcompris.separator(bconfig)

    gcompris.boolean_box(bconfig, D_(gcompris.GETTEXT_GUI,'Disable shape rotation'), 'disable_shape_rotation', eval(self.config_dict['disable_shape_rotation']))

    gcompris.separator(bconfig)

    gcompris.boolean_box(bconfig, D_(gcompris.GETTEXT_GUI,'Show Uppercase text only'), 'uppercase_text', eval(self.config_dict['uppercase_text']))

    gcompris.separator(bconfig)

    stamps = gcompris.boolean_box(bconfig, D_(gcompris.GETTEXT_GUI,'Disable stamps'), 'disable_stamps', eval(self.config_dict['disable_stamps']))
    stamps.connect("toggled", self.stamps_changed)

    self.stamps_control = gcompris.boolean_box(bconfig, D_(gcompris.GETTEXT_GUI,'Disable stamps control'), 'disable_stamps_control', eval(self.config_dict['disable_stamps_control']))
    self.stamps_control.set_sensitive(not eval(self.config_dict['disable_stamps']))

  def stamps_changed(self, button):
    self.stamps_control.set_sensitive(not button.get_active())

  def apply_callback(self,table):
    if table:
      for key,value in table.iteritems():
        gcompris.set_board_conf(self.configure_profile, self.gcomprisBoard,
                              key, value)
    return True

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
  global pid
  pid = None

  global fles
  fles.end()
  gcompris.end_board()

def stop_board():
  global fles
  fles.end()
  gcompris.end_board()


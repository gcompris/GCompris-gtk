#  gcompris - pythontest
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
# PythonTest Board module
import gobject
import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gtk
import gtk.gdk
import random
import cairo
import pango

from gcompris import gcompris_gettext as _

class Gcompris_pythontest:
  """Testing gcompris python class"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.disable_im_context = True

    self.canvasitems = {}

    self.colors = {}
    self.colors['circle_in'] = 0x8080FFFF
    self.colors['circle_out'] = 0xFFFFFFFF
    self.colors['line'] = 0xFFFF40FF

    # Just for config demo
    self.config_colors = { 'red' : 0xFF0000FFL,
                           'green' : 0x00FF00FFL,
                           'blue' : 0x0000FFFFL }

    # dict .keys() list of keys has random order
    self.config_colors_list = [ 'red', 'green', 'blue']

    self.movingline='none'

    # Find a number game
    self.solution = random.randint(0,9)

    print("Gcompris_pythontest __init__.")

    #initialisation to default values. Some of them will be replaced by
    #the configured values.

  def start(self):
    logged = gcompris.admin.get_current_user()

    # API Test, return the absolute path to this file
    print gcompris.utils.find_file_absolute("sounds/bleep.wav")
#    wordlist = gcompris.get_wordlist('wordslevel_max_pt_BR')
#    print wordlist
#    print wordlist.filename
#    print wordlist.level
#    print wordlist.locale
#    print wordlist.description
#    print wordlist.words


    if logged:
      print "User Logged in:"
      print "   ", logged.login, logged.firstname, logged.lastname

    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    # init config to default values
    self.config_dict = self.init_config()

    print "init self.config_dict :", self.config_dict

    # change configured values
    print "gcompris.get_board_conf() : ", gcompris.get_board_conf()
    self.config_dict.update(gcompris.get_board_conf())

    print "self.config_dict final :", self.config_dict

    if self.config_dict.has_key('locale'):
      gcompris.set_locale(self.config_dict['locale'])

    # self.colors['line'] s set in init.
    # I put here the configuration use

    color_name = self.config_dict['color_line']
    self.colors['line'] = self.config_colors[color_name]

    gcompris.bar_set(gcompris.BAR_CONFIG)
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())

    # distance is used to demo of gcompris.spin_int
    distance = eval(self.config_dict['distance_circle'])

    # pattern is for gcompris.radio_buttons
    pattern = self.config_dict['pattern']

    patterns = { 'circle': goocanvas.Ellipse,
                 'rectangle': goocanvas.Rect
                 }

    #error check
    if not patterns.has_key(pattern):
      pattern = 'circle'

    self.canvasitems[1] = patterns[pattern](
      parent = self.rootitem,
      fill_color_rgba= self.colors['circle_in'],
      stroke_color_rgba= self.colors['circle_out'],
      line_width=1.0
      )
    self.canvasitems[1].connect("button_press_event", self.circle_item_event)
    self.canvasitems[1].connect("button_release_event", self.circle_item_event)
    self.canvasitems[1].connect("motion_notify_event", self.circle_item_event)

    if(pattern == 'circle'):
      self.canvasitems[1].set_properties(
        center_x = 400.0 - distance,
        center_y = 200.0,
        radius_x = 20,
        radius_y = 20)
    else:
      self.canvasitems[1].set_properties(
        x = 400.0 - distance,
        y = 200.0,
        width = 20,
        height = 20)

    self.canvasitems[2] = patterns[pattern](
      parent = self.rootitem,
      fill_color_rgba= self.colors['circle_in'],
      stroke_color_rgba= self.colors['circle_out'],
      line_width=1.0
      )
    self.canvasitems[2].connect("button_press_event", self.circle_item_event)
    self.canvasitems[2].connect("button_release_event", self.circle_item_event)
    self.canvasitems[2].connect("motion_notify_event", self.circle_item_event)

    if(pattern == 'circle'):
      self.canvasitems[2].set_properties(
        center_x = 400.0 + distance,
        center_y = 200.0,
        radius_x = 20,
        radius_y = 20)
    else:
      self.canvasitems[2].set_properties(
        x = 400.0 + distance,
        y = 200.0,
        width = 20,
        height = 20)

    self.canvasitems[3] = goocanvas.Text(
      parent = self.rootitem,
      x=400.0,
      y=100.0,
      text=_("This is the first plugin in GCompris coded in the Python\nProgramming language."),
      fill_color="black",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    self.canvasitems[4] = goocanvas.Text(
      parent = self.rootitem,
      x=400.0,
      y=140.0,
      text=_("It is now possible to develop GCompris activities in C or in Python.\nThanks to Olivier Samys who makes this possible."),
      fill_color="black",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    self.canvasitems[5] = goocanvas.Text(
      parent = self.rootitem,
      x=400.0,
      y=250.0,
      text=_("This activity is not playable, just a test"),
      fill_color="black",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    #----------------------------------------
    # A simple game.
    # Try to hit left shift and right shift together. The peed increases
    goocanvas.Rect(
      parent = self.rootitem,
      x = 20,
      y = gcompris.BOARD_HEIGHT-220,
      width = gcompris.BOARD_WIDTH-40,
      height = 150,
      fill_color_rgba=0xFF663333L,
      stroke_color_rgba=0xFF33CCAAL,
      line_width=2.0)

    # For the game status WIN/LOOSE
    self.canvasitems[6] = goocanvas.Text(
      parent = self.rootitem,
      x=gcompris.BOARD_WIDTH / 2,
      y=gcompris.BOARD_HEIGHT - 100,
      font=gcompris.skin.get_font("gcompris/content"),
      fill_color_rgba=0x102010FFL,
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    goocanvas.Text(
      parent = self.rootitem,
      x=400.0,
      y=360.0,
      text=("Test your reflex with the counter. Hit the 2 shifts key together.\nHit space to reset the counter and increase the speed.\nBackspace to reset the speed"),
      fill_color="black",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    # The basic tick for object moves
    self.timerinc = 1000

    self.timer_inc  = gobject.timeout_add(self.timerinc, self.timer_inc_display)

    self.counter_left  = 0
    self.counter_right = 0

    self.canvasitems[7] =goocanvas.Text(
      parent = self.rootitem,
      x=gcompris.BOARD_WIDTH / 2,
      y=gcompris.BOARD_HEIGHT - 120,
      font=gcompris.skin.get_font("gcompris/content"),
      text="Speed="+str(self.timerinc)+" ms",
      fill_color="black",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    self.textitem_left = goocanvas.Text(
      parent = self.rootitem,
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH / 3,
      y=gcompris.BOARD_HEIGHT - 100,
      fill_color_rgba=0xFF000FFFL
      )

    self.textitem_right =goocanvas.Text(
      parent = self.rootitem,
      font=gcompris.skin.get_font("gcompris/content"),
      x=gcompris.BOARD_WIDTH / 1.5,
      y=gcompris.BOARD_HEIGHT - 100,
      fill_color_rgba=0xFF000FFFL
      )

    self.left_continue  = True
    self.right_continue = True

    print("Gcompris_pythontest start.")


  def end(self):

    gcompris.set_locale( "" )

    # Remove the root item removes all the others inside it
    self.rootitem.remove()

    if self.timer_inc :
      gobject.source_remove(self.timer_inc)


  def ok(self):
    print("Gcompris_pythontest ok.")


  def repeat(self):
    print("Gcompris_pythontest repeat.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_pythontest key press keyval=%i %s" % (keyval, strn))

    win = False

    if (keyval == gtk.keysyms.Shift_L):
      self.left_continue  = False

    if (keyval == gtk.keysyms.Shift_R):
      self.right_continue = False

    if(not self.left_continue and not self.right_continue):
      if(self.counter_left == self.counter_right):
        self.canvasitems[6].set_properties(text = "WIN",
                                           fill_color_rgba = 0x002EB8FFL)
        win=True
      else:
        self.canvasitems[6].set_properties(text = "LOOSE",
                                           fill_color_rgba = 0xFF0000FFL)

    if ((keyval == gtk.keysyms.BackSpace) or
        (keyval == gtk.keysyms.Delete)):
      self.timerinc = 1100
      keyval = 32

    if (keyval == 32):
      self.left_continue  = True
      self.right_continue = True
      self.counter_left  = 0
      self.counter_right = 0
      if(win):
        if(self.timerinc>500):
          self.timerinc -= 100
        elif(self.timerinc>200):
          self.timerinc -= 50
        elif(self.timerinc>10):
          self.timerinc -= 10
        elif(self.timerinc>1):
          self.timerinc -= 1

      if(self.timerinc<1):
          self.timerinc = 1

      self.canvasitems[3].set_properties(text="")
      self.canvasitems[6].set_properties(text="")

    self.canvasitems[7].set_properties(text="Speed="+str(self.timerinc)+" ms")

    # Find a number game
    if str(self.solution) == strn:
      print "WIN"
    else:
      print "LOST"

    # Return  True  if you did process a key
    # Return  False if you did not processed a key
    #         (gtk need to send it to next widget)
    return True

  def pause(self, pause):
    print("Gcompris_pythontest pause. %i" % pause)


  def set_level(self, level):
    print("Gcompris_pythontest set level. %i" % level)

# ---- End of Initialisation

  def timer_inc_display(self):

    if(self.left_continue):
      self.textitem_left.set_properties(text=str(self.counter_left))
      self.counter_left += self.timer_inc

    if(self.right_continue):
      self.textitem_right.set_properties(text=str(self.counter_right))
      self.counter_right += self.timer_inc

    self.timer_inc  = gobject.timeout_add(self.timerinc, self.timer_inc_display)

  def circle_item_event(self, widget, target, event=None):
    if eval(self.config_dict['disable_line']):
      return False

    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        bounds = widget.get_bounds()
        self.pos_x = (bounds.x1+bounds.x2)/2
        self.pos_y = (bounds.y1+bounds.y2)/2
        if 'line 1' in self.canvasitems:
          self.canvasitems['line 1'].remove()
        self.canvasitems['line 1'] =goocanvas.Polyline(
          parent = self.rootitem,
          points = goocanvas.Points([(self.pos_x, self.pos_y),
                                     (event.x, event.y)]),
          fill_color_rgba = self.colors['line'],
          line_cap = cairo.LINE_CAP_ROUND,
          line_width = 6.0
          )
        self.movingline='line 1'
        print "Button press"
        return True
    if event.type == gtk.gdk.MOTION_NOTIFY:
      if event.state & gtk.gdk.BUTTON1_MASK:
        self.canvasitems[self.movingline].set_properties(
          points = goocanvas.Points([(self.pos_x, self.pos_y),
                                     (event.x, event.y)])
          )
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        self.movingline='line 1'
        print "Button release"
        return True
    return False

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

    # init with default values
    self.config_dict = self.init_config()

    #get the configured values for that profile
    self.config_dict.update(gcompris.get_conf(profile, self.gcomprisBoard))

    # Init configuration window:
    # all the configuration functions will use it
    # all the configuration functions returns values for their key in
    # the dict passed to the apply_callback
    # the returned value is the main GtkVBox of the window,
    #we can add what you want in it.

    bconf = gcompris.configuration_window ( \
      _('<b>%s</b> configuration\n for profile <b>%s</b>')
      % ('Pythontest', (profile.name if profile else "") ),
      self.ok_callback
      )

    # toggle box
    control_line = gcompris.boolean_box(bconf, _('Disable line drawing in circle'),
                                        'disable_line',
                                        eval(self.config_dict['disable_line'])
                                        )
    # sample of control in python
    control_line.connect("toggled", self.color_disable)

    # combo box
    self.color_choice = \
       gcompris.combo_box(bconf, _('Color of the line'),
                          self.config_colors_list,
                          'color_line',
                          self.config_dict['color_line']
                          )
    self.color_choice.set_sensitive(not eval(self.config_dict['disable_line']))

    gcompris.separator(bconf)

    #spin button for int
    self.distance_box = \
       gcompris.spin_int(bconf, _('Distance between circles'),
                         'distance_circle',
                         20,
                         200,
                         20,
                         eval(self.config_dict['distance_circle'])
                         )

    gcompris.separator(bconf)

    #radio buttons for circle or rectangle
    patterns = { 'circle': _('Use circles'),
                 'rectangle': _('Use rectangles')
                 }

    # FIXME radio button makes the configuration unstable
    #       the problem is perhaps in the C code but since
    #       no one use it, let's forget it for now.
    # gcompris.radio_buttons(bconf, _('Choice of pattern'),
    #                        'pattern',
    #                        patterns,
    #                        self.config_dict['pattern']
    #                        )

    print "List of locales shown in gcompris.combo_locale :"
    print gcompris.get_locales_list()

    gcompris.separator(bconf)

    gcompris.combo_locales(bconf, self.config_dict['locale'])

    gcompris.separator(bconf)

    print "List of locales shown in gcompris.combo_locales_asset :"
    locales_purple = gcompris.get_locales_asset_list( "purple.ogg" )
    print locales_purple

    gcompris.combo_locales_asset(bconf, _("Select sound locale"),
                                 self.config_dict['locale_sound'],
                                 "voices/$LOCALE/colors/red.ogg")

  def color_disable(self, button):
    self.color_choice.set_sensitive(not button.get_active())

  # Callback when the "OK" button is clicked in configuration window
  # this get all the _changed_ values
  def ok_callback(self, table):
    if (table == None):
      print 'Configuration returns None'
      return True

    print "Keys and values returned by PythonTest config window:"

    if (len(table) == 0):
        print '%20s' % 'None'

    for key,value in table.iteritems():
      print '%20s:%20s    ' % (key, value)
      gcompris.set_board_conf(self.configuring_profile, self.gcomprisBoard, key, value)

    return True;

  def init_config(self):
    default_config = { 'disable_line'    : 'False',
                       'color_line'      : 'red',
                       'distance_circle' : '100',
                       'pattern'         : 'circle',
                       'locale'          : 'NULL',
                       'locale_sound'    : 'NULL'
                       }
    return default_config



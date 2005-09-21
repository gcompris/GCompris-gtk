#  gcompris - login
# 
# Copyright (C) 2005 Bruno Coudoin
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
import gcompris.skin
import gcompris.admin
import gtk
import gtk.gdk
import pango
from gettext import gettext as _

import math

# Database
from pysqlite2 import dbapi2 as sqlite

# Background screens
backgrounds = [
  "images/scenery2_background.png",
  "images/scenery3_background.png",
  "images/scenery4_background.png",
  "images/scenery5_background.png",
  "images/scenery6_background.png"
]

MAX_USERS_AT_ONCE = 10

class Gcompris_login:
  """Login screen for gcompris"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.disable_im_context = True

    print("Gcompris_login __init__.")


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_reload.png"))
    if(pixmap):
      gcompris.bar_set_repeat_icon(pixmap)
      gcompris.bar_set(gcompris.BAR_REPEAT_ICON)
    else:
      gcompris.bar_set(gcompris.BAR_REPEAT)

    gcompris.bar_set_level(self.gcomprisBoard)

    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            backgrounds[self.gcomprisBoard.level-1])

    # Get the default profile
    self.Prop = gcompris.get_properties()

    # Connect to our database
    self.con = sqlite.connect(gcompris.get_database())
    self.cur = self.con.cursor()

    # init config to default values
    self.config_dict = self.init_config()

    print gcompris.get_board_conf(), self.gcomprisBoard.board_id, self.gcomprisBoard.name

    # change configured values
    self.config_dict.update(gcompris.get_board_conf())

    # Create and Initialize the rootitem.
    self.init_rootitem(self.Prop)

    # Get the user list
    #users = self.get_users(self.con, self.cur,
    #                       Prop.profile.profile_id)

    users = []
    for group_id in self.Prop.profile.group_ids:
      users.extend( gcompris.admin.get_users_from_group(group_id))
      
    self.users = self.check_unique_id(users)

    if eval(self.config_dict['entry_text']):
      self.entry_text()
    else:
      self.display_user_by_letter(self.users, "")

    print("Gcompris_login start.")


  def init_rootitem(self, Prop):
    print "__rootitem__"
    # Create our rootitem.
    # We put each canvas item in it so at the end we only have to kill it.
    # The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    # Display the profile name
    x = gcompris.BOARD_WIDTH-100
    y = 20.0
    text = _("Profile: ") + Prop.profile.name

    # Shadow
    self.rootitem.add(
      gnome.canvas.CanvasText,
      x= x + 1,
      y= y + 1,
      text= text,
      fill_color="black",
      font=gcompris.skin.get_font("gcompris/board/small"),
      justification=gtk.JUSTIFY_RIGHT
      )

    # Profile name
    self.rootitem.add(
      gnome.canvas.CanvasText,
      x= x,
      y= y,
      text= text,
      fill_color="white",
      font=gcompris.skin.get_font("gcompris/board/small"),
      justification=gtk.JUSTIFY_RIGHT
      )


  def check_unique_id(self, users):
    passed = {}
    for user in users:
      passed[user.login] = user

    result = []
    keys = passed.keys()
    keys.sort()
    for login in keys:
      result.append(passed[login])

    return result
    
  def end(self):

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

    # Close the database
    self.cur.close()
    self.con.close()

  def ok(self):
    print("Gcompris_login ok.")


  def repeat(self):
    print("Gcompris_login repeat.")
    self.rootitem.destroy()

    self.init_rootitem(self.Prop)

    if eval(self.config_dict['entry_text']):
      self.entry_text()
    else:
      self.display_user_by_letter(self.users, "")

  def config(self):
    print("Gcompris_login config.")


  def key_press(self, keyval, commit_str, preedit_str):
    print("Gcompris_login key press. %i" % keyval)

    # Return  True  if you did process a key
    # Return  False if you did not processed a key
    #         (gtk need to send it to next widget)
    return False

  def pause(self, pause):
    print("Gcompris_login pause. %i" % pause)


  def set_level(self, level):
    print("Gcompris_login set level. %i" % level)

  # -------------------------------
  # ---- End of Initialisation ----
  # -------------------------------

  # Display user by letter
  # The first letter of the users is displayed
  # If the remaining list of users with this letter is
  # too large, then the next letter is proposed.
  # Once the user list is not too long, it is displayed
  #
  # param: users is the sorted list of users to display
  # param: start_filter is a string filter to apply on
  #        the first letters of the user name
  # 
  def display_user_by_letter(self, users, start_filter):

    print "display_user_by_letter start_filter=" + start_filter
          
    first_letters = []
    current_letter = None
    remaining_users=0
    
    for user in users:
      if eval(self.config_dict['uppercase_only']):
        login = user.login.decode('utf8').upper().encode('utf8')
      else:
        login = user.login
      if login.startswith(start_filter):
        remaining_users += 1
        if(len(start_filter)<len(login)):
          if(not current_letter or
             current_letter != login[len(start_filter)]):
            current_letter = login[len(start_filter)]
            first_letters.append(current_letter)

    # Fine we have the list of first letters
    if(remaining_users<MAX_USERS_AT_ONCE):
      # We now can display the list of users
      self.display_user_list(users, start_filter)
    else:
      # Display only the letters
      self.display_letters(first_letters, users, start_filter)

  #
  # Display the letters in 'letters'
  #
  # param users and start_filter are just need to let
  # this function pass it to it's event function
  # 
  def display_letters(self, letters, users, start_filter):

    # Create a group for the letters
    self.letter_rootitem = self.rootitem.add(
      self.rootitem,
      x=0.0,
      y=0.0
      )

    #
    # Display the current filter
    #

    x = gcompris.BOARD_WIDTH/2
    y = 20

    # The shadow
    self.letter_rootitem.add(
      gnome.canvas.CanvasText,
      x= x + 1.5,
      y= y + 1.5,
      text= _("Login: ") + start_filter + "...",
      fill_color="black",
      font=gcompris.skin.get_font("gcompris/board/huge"),
      )
    # The text
    self.letter_rootitem.add(
      gnome.canvas.CanvasText,
      x= x,
      y= y,
      text= _("Login: ") + start_filter + "...",
      fill_color="white",
      font=gcompris.skin.get_font("gcompris/board/huge"),
      )


    # Tricky but does the job, try to make the layout as large as
    # possible
    step_x = 480/math.sqrt(len(letters))
    
    start_x = 100
    x = start_x
    y = 100
    i = 0
    step_y = step_x
    current_line = 0
    max_letter_by_line = (gcompris.BOARD_WIDTH-start_x*2)/step_x
    letter_by_line = max_letter_by_line
    button_pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("ok.png"))
    
    for letter in letters:

      # Display both cases for the letter
      if eval(self.config_dict['uppercase_only']):
        text = letter
      else:
        text = letter.upper() + letter.lower()

      item = self.letter_rootitem.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = button_pixbuf,
        x = x -  button_pixbuf.get_width()/2,
        y = y -  button_pixbuf.get_height()/2,
        )
      # This item is clickeable and it must be seen
      item.connect("event", gcompris.utils.item_event_focus)
      item.connect("event", self.letter_click_event,
                   (users, start_filter + letter))
      
      # The shadow
      item = self.letter_rootitem.add(
        gnome.canvas.CanvasText,
        x= x + 1.5,
        y= y + 1.5,
        text= text,
        fill_color="black",
        font=gcompris.skin.get_font("gcompris/board/huge"),
        )
      item.connect("event", self.letter_click_event,
                   (users, start_filter + letter))

      # The text
      item = self.letter_rootitem.add(
        gnome.canvas.CanvasText,
        x= x,
        y= y,
        text= text,
        fill_color="white",
        font=gcompris.skin.get_font("gcompris/board/huge"),
        )
      item.connect("event", self.letter_click_event,
                   (users, start_filter + letter))
      x += step_x

      i += 1

      # Perform the column switch
      if(i>letter_by_line):
        i = 0
        if(current_line%2):
          x = start_x
          letter_by_line = max_letter_by_line
        else:
          x = start_x + step_x/2 
          letter_by_line = max_letter_by_line-1
         
        y += step_y
        current_line += 1


  # Display the user list so the user can click on it's name
  #
  # param users is the list of users to display
  #
  def display_user_list(self, users, start_filter):

    y_start = 80
    y = y_start
    x = gcompris.BOARD_WIDTH/4
    i = 0
    step_y = 90
    button_pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_large2.png"))
    
    for user in users:
      if eval(self.config_dict['uppercase_only']):
        login = user.login.decode('utf8').upper().encode('utf8')
      else:
        login = user.login

      if not login.startswith(start_filter):
        continue
      
      item = self.rootitem.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = button_pixbuf,
        x = x -  button_pixbuf.get_width()/2,
        y = y -  button_pixbuf.get_height()/2,
        )
      # This item is clickeable and it must be seen
      item.connect("event", gcompris.utils.item_event_focus)
      item.connect("event", self.name_click_event, user)

      
      # The shadow
      item = self.rootitem.add(
        gnome.canvas.CanvasText,
        x= x + 1.5,
        y= y + 1.5,
        text= login,
        fill_color="black",
        font=gcompris.skin.get_font("gcompris/board/huge"),
        )
      item.connect("event", self.name_click_event, user)

      # The text
      item = self.rootitem.add(
        gnome.canvas.CanvasText,
        x= x,
        y= y,
        text= login,
        fill_color="white",
        font=gcompris.skin.get_font("gcompris/board/huge"),
        )
      item.connect("event", self.name_click_event, user)

      y += step_y
      i += 1

      # Perform the column switch
      if(i==5):
        y = y_start
        x = 3*gcompris.BOARD_WIDTH/4


  #
  # Event when a click happen on a letter
  # data[0] is the user list we work on
  # data[1] is the start filter
  def letter_click_event(self, widget, event, data):
    if event.type == gtk.gdk.BUTTON_PRESS:
      self.letter_rootitem.destroy()
      self.display_user_by_letter(data[0], data[1])
      return True
    
    return False

  #
  # Event when a click happen on a user name
  #
  def name_click_event(self, widget, event, user):
    if event.type == gtk.gdk.BUTTON_PRESS:
      print "selected user = " + user.login

      self.logon(user)
      return True
    
    return False

  def logon(self, user):
    gcompris.admin.set_current_user(user)
    gcompris.admin.board_run_next(self.Prop.menu_board)
 
 
  def init_config(self):
    default_config = { 'uppercase_only'    : 'False',
                       'entry_text'      : 'False'
                       }
    return default_config


  def entry_text(self):
    print "__entry__"
    entry = gtk.Entry()

    entry.modify_font(pango.FontDescription("sans bold 36"))
    text_color = gtk.gdk.color_parse("white")
    text_color_selected = gtk.gdk.color_parse("green")
    bg_color = gtk.gdk.color_parse("blue")

    entry.modify_text(gtk.STATE_NORMAL, text_color)
    entry.modify_text(gtk.STATE_SELECTED, text_color_selected)
    entry.modify_base(gtk.STATE_NORMAL, bg_color)

    entry.set_max_length(50)
    entry.connect("activate", self.enter_callback)
    entry.connect("changed", self.enter_char_callback)

    entry.show()

    self.widget = self.rootitem.add(
      gnome.canvas.CanvasWidget,
      widget=entry,
      x=400,
      y=400,
      width=400,
      height=72,
      anchor=gtk.ANCHOR_CENTER,
      size_pixels=False
      )

    self.widget.raise_to_top()

    # does not work. Why ?
    #self.gcomprisBoard.canvas.grab_focus()
    self.widget.grab_focus()
    entry.grab_focus()
   
  def enter_char_callback(self, widget):
    if eval(self.config_dict['uppercase_only']):
      text = widget.get_text()
      widget.set_text(text.decode('utf8').upper().encode('utf8'))
    
  def enter_callback(self, widget):
    text = widget.get_text()

    found = False
    for user in self.users:
      if eval(self.config_dict['uppercase_only']):
        login = user.login.decode('utf8').upper().encode('utf8')
      else:
        login = user.login
      if text == login:
        self.widget.destroy()
        self.logon(user)
        found = True

    if not found:
      widget.set_text('')

  def config_start(self, profile):
    print '__login config__'
    # keep profile in mind
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

    self.main_vbox = gcompris.configuration_window ( \
      _('<b>%s</b> configuration\n for profile <b>%s</b>') % ('Login', profile.name ),
      self.ok_callback
      )

    # toggle box
    uppercase = gcompris.boolean_box(_('Uppercase only text'),
                                        'uppercase_only',
                                        eval(self.config_dict['uppercase_only'])
                                        )

    #uppercase.set_sensitive(False)

    gcompris.separator()

    # toggle box
    entry_text = gcompris.boolean_box(_('Enter login to log in'),
                                        'entry_text',
                                        eval(self.config_dict['entry_text'])
                                        )


  def config_stop(self):
    pass

  def ok_callback(self, table):
    for key,value in table.iteritems():
      gcompris.set_board_conf(self.configuring_profile, self.gcomprisBoard, key, value)
    

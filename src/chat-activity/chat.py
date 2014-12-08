#  gcompris - Chat
#
# Copyright (C) 2006, 2008  Bruno Coudoin
# Copyright (C) 2009, 2010  Bruno Coudoin, Fionn Ziegler
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
# Drawboard functionality by Fionn Ziegler <fionnziegler@gmail.com>
#
#

import goocanvas
import gcompris
from gcompris import gcompris_gettext as _
import gcompris.utils
import gcompris.bonus
import gcompris.skin
import gcompris.sound
import gtk
import gtk.gdk
import gobject
import random
import cairo

import socket
import struct
import pango
import sys
import uuid

from socket import gethostname

class Gcompris_chat:
  """The chat activity"""

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.rootitem = None
    #contains the drawboard items (points/lines)
    self.drawitems = []
    # Adress and port
    self.mcast_adress = "227.234.253.9"
    self.port = 15922
    self.mcast_timer = 0
    self.sock = None
    # Used to recognize our own network message
    self.uuid = uuid.uuid1().hex

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;


  def start(self):
    gcompris.bar_set (0)
    gcompris.bar_location(gcompris.BOARD_WIDTH - 160, -1, 0.6)
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
    self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

    # The global area
    # ---------------
    self.global_area_sw = gtk.ScrolledWindow()
    self.global_area_sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
    self.global_area_sw.set_shadow_type(gtk.SHADOW_ETCHED_OUT)

    w = gcompris.BOARD_WIDTH - 240.0
    h = gcompris.BOARD_HEIGHT - 300.0
    y = 20.0 # The upper limit of the text boxes
    x  = 20.0

    self.global_area_tb = gtk.TextBuffer()
    self.global_area_tv = gtk.TextView(self.global_area_tb)
    self.global_area_tv.set_editable(False)
    self.global_area_sw.add(self.global_area_tv)

    self.global_area_tb.set_text(_("All messages will be displayed here.\n"))

    self.global_area_tv.set_wrap_mode(gtk.WRAP_CHAR)

    goocanvas.Widget(
      parent = self.rootitem,
      widget=self.global_area_sw,
      x=x,
      y=y,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_NW)
    self.global_area_tv.show()
    self.global_area_sw.show()

    # The friend area
    # ---------------
    self.friend_area_sw = gtk.ScrolledWindow()
    self.friend_area_sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
    self.friend_area_sw.set_shadow_type(gtk.SHADOW_ETCHED_OUT)

    w = 160.0
    h = gcompris.BOARD_HEIGHT - 240.0
    y = 20.0 # The upper limit of the text boxes
    x = gcompris.BOARD_WIDTH - w - 20.0

    self.friend_area_tb = gtk.TextBuffer()
    self.friend_area_tv = gtk.TextView(self.friend_area_tb)
    self.friend_area_tv.set_editable(False)
    self.friend_area_sw.add(self.friend_area_tv)
    # save name and selected color in a map
    self.friend_map = {}

    self.friend_area_tb.set_text("")

    self.friend_area_tv.set_wrap_mode(gtk.WRAP_CHAR)

    goocanvas.Widget(
      parent = self.rootitem,
      widget=self.friend_area_sw,
      x=x,
      y=y,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_NW)
    self.friend_area_tv.show()
    self.friend_area_sw.show()

    # A label for the friend area

    goocanvas.Text(
      parent = self.rootitem,
      text=_("Your Friends"),
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x=x+(w/2),
      y=y+h+15,
      anchor=gtk.ANCHOR_CENTER,
      )

    # The channel area
    # ---------------
    self.channel = gtk.Entry()
    w = 160.0
    h = 30.0
    y = gcompris.BOARD_HEIGHT - 160.0
    x = x


    goocanvas.Widget(
      parent = self.rootitem,
      widget=self.channel,
      x=x,
      y=y,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_NW)

    self.channel.show()
    self.channel.set_text("")


    # A label for the channel area

    goocanvas.Text(
      parent = self.rootitem,
      text=_("Your Channel"),
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x=x+(w/2),
      y=y+h+15,
      anchor=gtk.ANCHOR_CENTER,
      )

    # the entry area
    self.entry = gtk.Entry()
    self.entry.connect("activate", self.enter_callback, self.entry)
    x = 20.0
    w = gcompris.BOARD_WIDTH - x * 2
    h = 30.0
    y = gcompris.BOARD_HEIGHT - 70.0


    goocanvas.Widget(
      parent = self.rootitem,
      widget=self.entry,
      x=x,
      y=y,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_NW)

    self.entry.show()
    self.entry.set_text(_("Type your message here, to send to other GCompris users on your local network."))

    # drawing area
    x = 20.0
    w = gcompris.BOARD_WIDTH - 300
    h = 200.0
    y = gcompris.BOARD_HEIGHT - 270.0
    self.colors = {}
    self.colors['bg'] = 0xFFFFFFFFL
    self.colors['border'] = 0x000000FFL

    self.drawboard = goocanvas.Rect(
      parent = self.rootitem,
      fill_color_rgba = self.colors['bg'],
      stroke_color_rgba = self.colors['border'],
      line_width = 1.0,
      x = x,
      y = y,
      width = w,
      height = h)
    self.drawboard.connect("button_press_event", self.draw_item_event)
    self.drawboard.connect("button_release_event", self.draw_item_event)
    self.drawboard.connect("motion_notify_event", self.draw_item_event)
    r = random.randint(0,200) # Prevent the possibility of white
    g = random.randint(0,255)
    b = random.randint(0,255)

    # colorpicker
    self.colorpicker = gtk.ColorButton(gtk.gdk.color_parse(self.tohex(r, g, b)))

    x = gcompris.BOARD_WIDTH - 270
    y = gcompris.BOARD_HEIGHT - 270.0
    w = 50
    h = 50

    goocanvas.Widget(
      parent = self.rootitem,
      widget = self.colorpicker,
      x = x,
      y = y,
      width = w,
      height = h,
      anchor = gtk.ANCHOR_NW)


    self.colorpicker.show()
    self.colorpicker.set_tooltip_text(_("color"))
    # line and draw radiobuttons
    self.buttonline = gtk.RadioButton()
    self.buttondraw = gtk.RadioButton(group=self.buttonline)

    x = gcompris.BOARD_WIDTH - 270
    y = gcompris.BOARD_HEIGHT - 220.0
    w = 70
    h = 50

    goocanvas.Widget(
      parent = self.rootitem,
      widget = self.buttonline,
      x = x,
      y = y,
      width = w,
      height = h,
      anchor = gtk.ANCHOR_NW)

    goocanvas.Widget(
      parent = self.rootitem,
      widget = self.buttondraw,
      x = x,
      y = y + 50,
      width = w,
      height = h,
      anchor = gtk.ANCHOR_NW)

    image_line = gtk.Image()
    image_draw = gtk.Image()

    pixbuf = gcompris.utils.load_pixmap("chat/tool-line.png")
    image_line.set_from_pixbuf(pixbuf)
    pixbuf = gcompris.utils.load_pixmap("chat/tool-draw.png")
    image_draw.set_from_pixbuf(pixbuf)

    self.buttonline.set_image(image_line)
    self.buttondraw.set_image(image_draw)

    # erase all button
    self.delAll = gtk.Button()
    x = gcompris.BOARD_WIDTH - 270
    y = gcompris.BOARD_HEIGHT - 100.0
    w = 50
    h = 30


    goocanvas.Widget(
      parent = self.rootitem,
      widget = self.delAll,
      x = x,
      y = y,
      width = w,
      height = h,
      anchor = gtk.ANCHOR_NW)
    self.delAll.show()
    # don't delete this, it's used to change font...
    self.delAll.set_label("")

    if self.delAll.get_use_stock():
      label = self.delAll.child.get_children()[1]
    elif isinstance(self.delAll.child, gtk.Label):
      label = self.delAll.child

    label.set_markup("<span foreground='red' weight='ultrabold' size='x-large'>X</span>")
    self.delAll.connect("button_press_event", self.delAllEvent)
    try:
      # Start the server
      self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
      self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
      self.sock.bind(('', self.port))
      mreq = struct.pack('4sl', socket.inet_aton(self.mcast_adress), socket.INADDR_ANY)

      self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
      self.sock.setblocking(0)
      self.timer_interval = 10
      self.mcast_timer = gobject.timeout_add(self.timer_interval, self.mcast_read)
    except:
      self.display_message("GCompris",
                           _("ERROR: Failed to initialize the network interface. You cannot communicate."))


  def end(self):
    self.cleanup()

  def ok(self):
    print("Gcompris_chat ok.")

  def key_press(self, keyval, commit_str, preedit_str):
    #print("got key %i" % keyval)

    return False

  # Called by gcompris core
  def pause(self, pause):

    self.board_paused = pause

    # There is a problem with GTK widgets, they are not covered by the help
    # We hide/show them here
    if(pause):
      self.global_area_tv.hide()
      self.global_area_sw.hide()
      self.friend_area_tv.hide()
      self.friend_area_sw.hide()
      self.entry.hide()
      self.channel.hide()
    else:
      self.global_area_tv.show()
      self.global_area_sw.show()
      self.friend_area_tv.show()
      self.friend_area_sw.show()
      self.entry.show()
      self.channel.show()

    return


  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------


  def cleanup(self):
    #tell the others to remove me from the friend-lists of the other clients:
    prop = gcompris.get_properties()
    self.send_message("GCOMPRIS:LEAVE:%s:%s:%s" % (self.channel.get_text(),
                                                   prop.logged_user.login,
                                                   self.get_selectedcolor() ))
    if self.mcast_timer :
      gobject.source_remove(self.mcast_timer)
      self.mcast_timer = 0

    if self.sock:
      self.sock.close()

    # Remove the root item removes all the others inside it
    if self.rootitem != None:
     self.rootitem.remove()
     self.rootitem = None


  def display_message(self, nick, message):

    self.global_area_tb.insert(self.global_area_tb.get_end_iter(),
                               nick + " => " + message + "\n")

    self.global_area_tv.scroll_to_iter(self.global_area_tb.get_end_iter(),
                                       0.0,
                                       True,
                                       0,
                                       0)
    self.set_colors(self.global_area_tb)

  def mcast_read(self):

      if not self.mcast_timer:
          return

      self.mcast_timer = gobject.timeout_add(self.timer_interval,
                                             self.mcast_read)

      text = ""
      try:
          text = self.sock.recv(10240)
          #print "Received text: %s\n" % text
      except:
          return

      # Parse it
      textl = text.split(":", 10)

      # Is this a message for us
      if(textl[0] != "GCOMPRIS"):
          return

      if(textl[1] != "CHAT" and textl[1] != "DRAW" and textl[1] != "LEAVE"):
          return
      if(textl[2] != self.channel.get_text()):
          return
      # if color is available if not use black (e.g. chat mode)
      try:
          color = textl[4]
      except:
          color = "#000000"
      # map->name:color
      self.friend_map[textl[3]] = color
      # Build the friend list
      if textl[1] == "LEAVE":
          self.display_message(textl[3], _("Has left the chat."))
          del self.friend_map[textl[3]]

      friends=""
      for name in self.friend_map.keys():
          friends = "%s%s\n" % (friends, name)

      self.friend_area_tb.set_text(friends)
      self.set_colors(self.friend_area_tb)

      gcompris.sound.play_ogg("sounds/receive.wav")

      if (textl[1] == "CHAT"):
          # Display the message - user: message
          self.display_message(textl[3], textl[5])
      if (textl[1] == "DRAW"):
        # don't paint our own drawing
        if self.uuid == textl[5]:
          return False

        # draw received point or line...
        x = self.convertStr(textl[7])
        y = self.convertStr(textl[8])
        # check type
        if textl[6] == "point":
          self.draw_point(x, y, color)
        if textl[6] == "line":
          x2 = self.convertStr(textl[9])
          y2 = self.convertStr(textl[10])
          self.draw_line(x, y, x2, y2, color)

      return False

  def set_colors(self, widget):
      """sets the name in the list with the selected color."""
      # Get text buffer from the end and mark last entry
      start_iter = widget.get_end_iter()

      for name, color in self.friend_map.iteritems():
        # Always just search the latest occurrence of 'name' to avoid that we
        # Search every time the whole text!
        # This would be a lot of cpu work
        result = start_iter.backward_search(name, 0,None)
        if not result:
            return
        match_start_iter, match_end_iter = result

        match_start_iter.starts_word()
        match_end_iter.ends_word()

        # Create color tag
        ctag = widget.get_tag_table().lookup(color)
        if ctag == None:
           self.color_tag = widget.create_tag(color)
           self.color_tag.set_property("foreground", color)
           self.color_tag.set_property("weight", pango.WEIGHT_BOLD)
        else:
           self.color_tag = ctag

        widget.apply_tag(self.color_tag ,match_start_iter,
               match_end_iter)


  def convertStr(self,s):
    """Convert string to either int or float."""
    try:
        ret = int(s)
    except ValueError:
        #Try float.
        ret = float(s)
    return ret


  def enter_callback(self, widget, entry):
    gcompris.sound.play_ogg("sounds/bleep.wav")

    if(not self.channel.get_text()):
      self.display_message(
        "GCompris",
        _("You must set a channel in your channel entry box first.\n") +
        _("Your friends must set the same channel in order to communicate with you") )
      return
    Prop = gcompris.get_properties()
    entry_text = entry.get_text()

    # format the message
    entry_text = ("GCOMPRIS:CHAT:" +
                  self.channel.get_text() + ":" +
                  Prop.logged_user.login + ":" + self.get_selectedcolor() +
                  ":" + entry_text)
    self.send_message(entry_text)
    entry.set_text("")

  def send_drawboard_content(self, x, y, linex=None, liney=None, color=""):
    """send either line or point depending the given attributes."""

    Prop = gcompris.get_properties()
    type = ""
    if linex == None:
        type = "point"
        linex = ""
        liney = ""
    else:
        type = "line"


    message = ("GCOMPRIS:DRAW:" + self.channel.get_text() + ":" +
               Prop.logged_user.login + ":" + color + ":" +
               self.uuid + ":" +
               type + ":" + repr(x) + ":" + repr(y) +
               ":" +str(linex) + ":"+ str(liney))
    self.send_message(message)


  def send_message(self, message):
    """sends the given message."""

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM,
                         socket.IPPROTO_UDP)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)

    sock.sendto(message, (self.mcast_adress, self.port))
    sock.close()

  def delAllEvent(self, widget, target, event=None):
     """removes the draw board content."""
     for point in self.drawitems:
         point.remove()

  def draw_item_event(self, widget, target, event=None):
    bounds = self.drawboard.get_bounds()

    if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
      self.pos_x = event.x
      self.pos_y = event.y
      # -------- LINE
      if self.buttonline.get_active():
        self.line = self.draw_line(self.pos_x, self.pos_y,
                                   event.x, event.y,
                                   self.get_selectedcolor())
        return True
      if self.buttondraw.get_active():
        self.lastx =self.pos_x
        self.lasty =self.pos_y
        self.draw_point(self.pos_x, self.pos_y, self.get_selectedcolor())
        self.send_drawboard_content(self.pos_x, self.pos_y,
                                    None, None, self.get_selectedcolor())
    # do action while mouse button is pressed and in movement
    elif event.type == gtk.gdk.MOTION_NOTIFY \
          and event.state & gtk.gdk.BUTTON1_MASK:
      if event.x < bounds.x2 and event.x > bounds.x1 \
            and  event.y < bounds.y2 and event.y > bounds.y1:
        if self.buttonline.get_active(): # line selected
          # to draw the line while mouse button is pressed
          self.line.set_properties(points=goocanvas.Points
                                   ([(self.pos_x, self.pos_y),
                                     (event.x, event.y)]))
        elif self.buttondraw.get_active():
          self.difx = event.x - self.lastx
          self.dify = event.y - self.lasty
          # because there is a space between points
          # if you draw moving the mouse fast,
          # lets make a line between them:
          if self.difx > 1 or self.difx < -1 \
                or self.dify > 1 or self.dify < -1:
            self.draw_line(self.lastx, self.lasty,
                           event.x, event.y,
                           self.get_selectedcolor())
            self.send_drawboard_content(self.lastx, self.lasty,
                                        event.x, event.y,
                                        self.get_selectedcolor())

            self.draw_point(event.x, event.y, self.get_selectedcolor())
            self.send_drawboard_content(event.x, event.y,
                                        None, None,
                                        self.get_selectedcolor())
            self.lastx = event.x
            self.lasty = event.y

    elif event.type == gtk.gdk.BUTTON_RELEASE \
          and self.buttonline.get_active() \
          and event.button == 1:
      if event.x < bounds.x2 and event.x > bounds.x1 \
            and event.y < bounds.y2 and event.y > bounds.y1:
        # send line when mouse button released
        self.send_drawboard_content(self.pos_x, self.pos_y,
                                    event.x, event.y,
                                    self.get_selectedcolor())
        return True

    return False

  def draw_point(self, x, y, color):
    """draws the given point."""
    point = goocanvas.Rect(parent = self.rootitem,
                           x=x,
                           y=y,
                           width=1,
                           height=1,
                           stroke_color = color)
    self.drawitems.append(point)
    #to be able to draw by clicking on the new point
    point.connect("button_press_event", self.draw_item_event)
    point.connect("button_release_event", self.draw_item_event)
    point.connect("motion_notify_event", self.draw_item_event)
    return point

  def draw_line(self, x, y, destx, desty, color):
    line =goocanvas.Polyline(parent = self.rootitem,
                             points = goocanvas.Points([(x, y),
                                                        (destx, desty)]),
                             stroke_color = color,
                             line_cap = cairo.LINE_CAP_ROUND,
                             line_width = 4.0)
    self.drawitems.append(line)
    #to be able to draw by clicking on the new line
    line.connect("button_press_event", self.draw_item_event)
    line.connect("button_release_event", self.draw_item_event)
    line.connect("motion_notify_event", self.draw_item_event)
    return line


  def get_selectedcolor(self):
    """returns the selected color of the GDK ColorButton in hex form."""
    color = self.colorpicker.get_color();
    g = int((color.green / 65535.0) *255)
    r = int((color.red   / 65535.0) *255)
    b = int((color.blue  / 65535.0) *255)
    return self.tohex(r, g, b)

  def tohex(self,r,g,b):
    """rgb to hex string."""
    hexchars = "0123456789ABCDEF"
    return "#" + hexchars[r / 16] + hexchars[r % 16] + hexchars[g / 16] \
        + hexchars[g % 16] + hexchars[b / 16] + hexchars[b % 16]


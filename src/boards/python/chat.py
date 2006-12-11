#  gcompris - Chat
#
# Copyright (C) 2006  Bruno Coudoin
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
from gettext import gettext as _
import gnomecanvas
import gcompris
import gcompris.utils
import gcompris.bonus
import gcompris.skin
import gtk
import gtk.gdk
import gobject
import random
import math

import socket
import struct
import sys

from socket import gethostname

class Gcompris_chat:
  """The chat activity"""

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.rootitem = None

    # Randon adress
    self.mcast_adress = "227.234.253.9"

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = False;


  def start(self):
    gcompris.bar_set (0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    self.rootitem = self.gcomprisBoard.canvas.root().add(
     gnomecanvas.CanvasGroup,
     x=0.0,
     y=0.0
     )

    # The global area
    # ---------------
    self.global_area_sw = gtk.ScrolledWindow()
    self.global_area_sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
    self.global_area_sw.set_shadow_type(gtk.SHADOW_ETCHED_OUT)

    w = gcompris.BOARD_WIDTH - 240.0
    h = gcompris.BOARD_HEIGHT - 100.0
    y = 20.0 # The upper limit of the text boxes
    x  = 20.0

    self.global_area_tb = gtk.TextBuffer()
    self.global_area_tv = gtk.TextView(self.global_area_tb)
    self.global_area_tv.set_editable(False)
    self.global_area_sw.add(self.global_area_tv)

    self.global_area_tb.set_text(_("All messages will be displayed here.\n"))

    self.global_area_tv.set_wrap_mode(gtk.WRAP_CHAR)
    self.rootitem.add(
      gnomecanvas.CanvasWidget,
      widget=self.global_area_sw,
      x=x,
      y=y,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_NW,
      size_pixels=False)
    self.global_area_tv.show()
    self.global_area_sw.show()

    # The friend area
    # ---------------
    self.friend_area_sw = gtk.ScrolledWindow()
    self.friend_area_sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
    self.friend_area_sw.set_shadow_type(gtk.SHADOW_ETCHED_OUT)

    w = 160.0
    h = gcompris.BOARD_HEIGHT - 100.0
    y = 20.0 # The upper limit of the text boxes
    x = gcompris.BOARD_WIDTH - w - 20.0

    self.friend_area_tb = gtk.TextBuffer()
    self.friend_area_tv = gtk.TextView(self.friend_area_tb)
    self.friend_area_tv.set_editable(False)
    self.friend_area_sw.add(self.friend_area_tv)

    self.friend_list = []
    self.friend_area_tb.set_text("")

    self.friend_area_tv.set_wrap_mode(gtk.WRAP_CHAR)
    self.rootitem.add(
      gnomecanvas.CanvasWidget,
      widget=self.friend_area_sw,
      x=x,
      y=y,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_NW,
      size_pixels=False)
    self.friend_area_tv.show()
    self.friend_area_sw.show()

    # A label for the friend area
    self.rootitem.add(
      gnomecanvas.CanvasText,
      text=_("Your Friends"),
      font = gcompris.skin.get_font("gcompris/board/medium"),
      x=x+(w/2),
      y=y+h+15,
      anchor=gtk.ANCHOR_CENTER,
      )

    # the entry area
    entry = gtk.Entry()
    entry.connect("activate", self.enter_callback, entry)
    w = gcompris.BOARD_WIDTH - 240
    h = 30.0
    y = gcompris.BOARD_HEIGHT - 60.0
    x = 20.0

    self.rootitem.add(
      gnomecanvas.CanvasWidget,
      widget=entry,
      x=x,
      y=y,
      width=w,
      height= h,
      anchor=gtk.ANCHOR_NW,
      size_pixels=False)

    entry.show()
    entry.set_text(_("Type here you message to GCompris users in your local network."))

    # Start the server
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    self.sock.bind(('', 6543))
    mreq = struct.pack('4sl', socket.inet_aton(self.mcast_adress), socket.INADDR_ANY)

    self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    self.sock.setblocking(0)

    self.timer_interval = 500
    self.mcast_timer = gobject.timeout_add(self.timer_interval, self.mcast_read)

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

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(pause == 0 and self.gamewon):
      self.finished()
      self.gamewon = 0

    return


  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------
  # ----------------------------------------------------------------------


  def cleanup(self):

    self.gamewon       = False;

    if self.mcast_timer :
      gobject.source_remove(self.mcast_timer)
      self.mcast_timer = 0

    self.sock.close()

    # Remove the root item removes all the others inside it
    if self.rootitem != None:
     self.rootitem.destroy()
     self.rootitem = None


  def finished(self):
    gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)

  def mcast_read(self):

      if not self.mcast_timer:
          return

      self.mcast_timer = gobject.timeout_add(self.timer_interval, self.mcast_read)

      text = ""
      try:
          text = self.sock.recv(10240)
          #print "Received text: %s\n" % text
      except:
          return

      # Parse it
      textl = text.split(":", 3)

      # Is this a message for us
      if(textl[0] != "GCOMPRIS"):
          return

      if(textl[1] != "CHAT"):
          return

      # Build the friend list
      gotit = False
      for name in self.friend_list:
          if name == textl[2]:
              gotit = True

      if not gotit:
          self.friend_list.append(textl[2])
          self.friend_list.sort()

      friends="\n"
      friends = friends.join(self.friend_list)
      self.friend_area_tb.set_text(friends)

      # Display the message
      self.global_area_tb.insert(self.global_area_tb.get_end_iter(),
                                 textl[2] + " => " + textl[3] + "\n")

      self.global_area_tv.scroll_to_iter(self.global_area_tb.get_end_iter(),
                                         0.0,
                                         True,
                                         0,
                                         0)
      return False

  def enter_callback(self, widget, entry):
      entry_text = entry.get_text()
      sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM,
                           socket.IPPROTO_UDP)
      sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)

      # format the message
      entry_text = "GCOMPRIS:CHAT:" + gethostname() + ":" + entry_text
      sock.sendto(entry_text, (self.mcast_adress, 6543))
      entry.set_text("")
      sock.close()

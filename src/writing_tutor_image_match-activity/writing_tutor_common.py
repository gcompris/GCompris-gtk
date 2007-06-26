#   gcompris/tutor/common.py - Writing Tutor project
#
#   Author: Jozef Cerven, 5mi, (c) 2006/2007
#   FACULTY OF MATHEMATICS, PHYSICS AND INFORMATICS COMENIUS UNIVERSITY BRATISLAVA, Informatics Teaching and Basic of Informatics department
#   http://www.edi.fmph.uniba.sk
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

import gnomecanvas
import gcompris
import gcompris.utils
import gcompris.bonus
import gcompris.skin
import gcompris.anim
import gcompris.sound
import gcompris.admin
import gcompris.score
import gtk
import gtk.gdk
import random
import pango
import time
import os
import socket
import fcntl
import struct
import re
import glob
import string
import time




# BEGIN OF class GtkInputDialog ************************************
class GtkInputDialog:

	inputd = None

	def __init__(self, Gcompris_template_match_instance):

		self.gd_instance = Gcompris_template_match_instance

		self.inputd = gtk.InputDialog()
		self.inputd.move((gcompris.BOARD_WIDTH)/2, (gcompris.BOARD_HEIGHT)/2)  # !
		self.inputd.connect("enable-device", self.clb_enable_device)
		self.inputd.connect("disable-device", self.clb_disable_device)
		self.inputd.connect("response", self.clb_close_dialog, None)
		self.inputd.run()

	def clb_close_dialog(self, inputdialog, deviceid, x=None):
		self.inputd.hide()


	def clb_enable_device(self, inputdialog, deviceid, x=None):
		print "clb_enable_device", deviceid.name

		if (deviceid.source == gtk.gdk.SOURCE_PEN) and (deviceid.mode == gtk.gdk.MODE_SCREEN or deviceid.mode == gtk.gdk.MODE_WINDOW):
			self.gd_instance.set_tablet_indicator(1)

		self.clb_close_dialog(inputdialog, deviceid)

	def clb_disable_device(self, inputdialog, deviceid, x=None):
		print "clb_disable_device", deviceid.name

		if (deviceid.source == gtk.gdk.SOURCE_PEN):
			self.gd_instance.set_tablet_indicator(0)

		self.clb_close_dialog(inputdialog, deviceid)

# END OF class GtkInputDialog ************************************



def draw_guiding_lines(_pixmap, _area, _pattern, _right, _color='SlateGrey'):
		"""
		Description:
		Paint guiding lines according to type of shape (lower, upper, digit, script,...)

	   	Arguments:
    	_pixmap  -- gtk.gdk.Pixmap
		_area    -- gtk.DrawingArea
		_pattern -- pattern to be written
		_color   -- foreground color of lines, color string specified in the X11 rgb.txt
    	"""

		left = 60
		right = _right
		top = 60
		bottom = 450

		colormap = _area.get_colormap()
		_color_ = colormap.alloc_color(_color)

		gc_solid = _pixmap.new_gc()
		gc_solid.copy(_area.get_style().black_gc)
		gc_solid.set_foreground(_color_)
		gc_solid.set_line_attributes(2, gtk.gdk.LINE_SOLID, gtk.gdk.CAP_BUTT, gtk.gdk.JOIN_MITER)

		gc_dash = _pixmap.new_gc()
		gc_dash.copy(gc_solid)
		gc_dash.set_foreground(_color_)
		gc_dash.set_line_attributes(1, gtk.gdk.LINE_ON_OFF_DASH, gtk.gdk.CAP_BUTT, gtk.gdk.JOIN_MITER)

		_pixmap.draw_line(gc_dash, left, top, right, top)				# top line
		_pixmap.draw_line(gc_dash, left, bottom, right, bottom)		# bottom line


		if _pattern[0] == '_':		# tlacene pismena..(in english?)
			_pixmap.draw_line(gc_solid, left, top+40, right, top+40)				# top1 line
			if _pattern[1] in ['a','b','c','d','e','f','h','i','k','l','m','n','o','r','s','t','u','v','z']:
				_pixmap.draw_line(gc_dash, left, 200, right, 200)
				_pixmap.draw_line(gc_solid, left, bottom-40, right, bottom-40)
			elif _pattern[1] in ['g','p']:
				_pixmap.draw_line(gc_dash, left, 320, right, 320)
				_pixmap.draw_line(gc_solid, left, bottom-40, right, bottom-40)
			elif _pattern[1] in ['j']:
				_pixmap.draw_line(gc_dash, left, 210, right, 210)
				_pixmap.draw_line(gc_solid, left, bottom-50, right, bottom-50)
			elif _pattern[1] in ['A','B','E','F','G','H','J','K','P','R',]:
				_pixmap.draw_line(gc_dash, left, 260, right, 260)
				_pixmap.draw_line(gc_solid, left, bottom-40, right, bottom-40)
			elif _pattern[1] in ['C','D','I','L','M','N','O','S','T','U','V','Z']:
				_pixmap.draw_line(gc_dash, left, 200, right, 200)
				_pixmap.draw_line(gc_solid, left, bottom-40, right, bottom-40)


		elif _pattern[0] == '@':	# geometric shapes
			pass

		else:				# pisane pismena male aj velke, a cislice 0-9
			_pixmap.draw_line(gc_solid, left, top+40, right, top+40)				# top1 line
			if _pattern[0] not in ['f','g','G','j','J','p']:
				_pixmap.draw_line(gc_dash, left, 240, right, 240)
				_pixmap.draw_line(gc_solid, left, bottom-40, right, bottom-40)
			else:
				_pixmap.draw_line(gc_dash, left, 210, right, 210)
				_pixmap.draw_line(gc_solid, left, bottom-110, right, bottom-110)





def get_ip_address(ifname):
	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		return socket.inet_ntoa(fcntl.ioctl(s.fileno(), 0x8915, struct.pack('256s', ifname[:15]))[20:24])
	except:
		return "127.0.0.1"




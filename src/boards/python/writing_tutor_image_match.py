#   gcompris/tutor/image_match.py - Writing Tutor project
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


#import gobject
from gettext import gettext as _

from writing_tutor_common import *

import DTW.dtw


_global_rootitem = None		# for config dialog
_global_tablet_indicator= None





# BEGIN OF class Gcompris_image_match ************************************
class Gcompris_writing_tutor_image_match:

	_ap = None
	ap = None
	_sb = None
	sb = None

	rootitem = None
	board_paused = 0

	save_path = None
	sequence = None
	pattern = None
	pattern_image = None
	show_template = 100*[None]		# if show_template[0]==1: show_template in level 1

	area = None
	notepad_area = None
	label = None
	won_level = 0

	dat_filename = None
	pixmap = None						# drawing area pixmap
	filename = None
	file_handle = None
	writing_status = None
	last_written_point_index = None
	motions_count = None
	points =  [None] * 2000
	cas_start = None
	button_pixbuf = None

	def __init__(self, gcomprisBoard):
		self.gcomprisBoard = gcomprisBoard

		_notepad_width = 485
		_notepad_height = 485

		#self.notepad_area  = ((gcompris.BOARD_WIDTH/2 - _notepad_width/2), 20, _notepad_width, _notepad_height)	 #	centered
		self.notepad_area  = (280, 10, _notepad_width, _notepad_height)



	def start(self):
		global _global_rootitem

		self.window = self.gcomprisBoard.canvas.get_toplevel()

		self.config_dict = self.init_config()
		self.config_dict.update(gcompris.get_board_conf())

		self.save_path = self.config_dict['save_path']
		self.sequence = self.config_dict['image_match_sequence']

		self.parse_sequence()

		self.gcomprisBoard.level=1
		self.gcomprisBoard.maxlevel=len(self.sequence)
		self.gcomprisBoard.sublevel=1
		self.gcomprisBoard.number_of_sublevel = len(self.sequence[self.gcomprisBoard.level-1])


		_pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_reload.png"))
		if(_pixmap):
			gcompris.bar_set_repeat_icon(_pixmap)
			gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_OK|gcompris.BAR_REPEAT_ICON|gcompris.BAR_CONFIG)
		else:
			gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_OK|gcompris.BAR_REPEAT|gcompris.BAR_CONFIG)


		#gcompris.set_background(self.rootitem, gcompris.skin.image_to_skin("gcompris-bg.jpg"))
		gcompris.set_background(self.rootitem, "writing_tutor/wood_pine.png")


		# Create our rootitem. We put each canvas item in it so at the end we
		# only have to kill it. The canvas deletes all the items it contains automaticaly.
		self.rootitem = self.gcomprisBoard.canvas.root().add(gnomecanvas.CanvasGroup, x=0.0, y=0.0)
		_global_rootitem = self.rootitem


		# show tablet indicator icon

		_t_active = 0
		_devices = gtk.gdk.devices_list()
		for _d in _devices:
			if _d.source == gtk.gdk.SOURCE_PEN:
				_t_active = 1
				if _d.mode == gtk.gdk.MODE_DISABLED:	# if tablet is disabled
					_d.set_mode(gtk.gdk.MODE_SCREEN)	# enable it (first in sequence)
					break
				else:									# already enabled..
					break

		if _t_active == 1:
			self.set_tablet_indicator(1)
		else:
			self.set_tablet_indicator(0)


		self.set_level(1)




	def recreate_drawing_area(self):

		# BEGIN OF drawing_area ************************************

		if self.area != None:
			self.area.destroy()

		self.area = gtk.DrawingArea()

		# Signals used to handle backing pixmap
		self.area.connect("expose_event", self.expose_event)
		self.area.connect("configure_event", self.configure_event)

		# Event signals
		self.area.connect("motion_notify_event", self.motion_notify_event)
		#area.connect("button_press_event", self.button_press_event)

		self.area.set_events(gtk.gdk.EXPOSURE_MASK | gtk.gdk.LEAVE_NOTIFY_MASK | gtk.gdk.BUTTON_PRESS_MASK
                            | gtk.gdk.POINTER_MOTION_MASK |	gtk.gdk.PROXIMITY_IN | gtk.gdk.PROXIMITY_OUT_MASK)

		# The following call enables tracking and processing of extension
		# events for the drawing area
		#self.area.set_extension_events(gtk.gdk.EXTENSION_EVENTS_CURSOR)
		self.area.set_extension_events(gtk.gdk.EXTENSION_EVENTS_ALL)

		self.rootitem.add(gnomecanvas.CanvasWidget, widget=self.area, x=self.notepad_area[0], y=self.notepad_area[1],
							width=self.notepad_area[2], height=self.notepad_area[3])

		self.area.show()
		# END OF drawing_area ************************************

		#print "recreate_drawing_area()"



	def start_2(self, recreate="yes"):

		self.pattern = self.sequence[self.gcomprisBoard.level-1][self.gcomprisBoard.sublevel-1]
		gcompris.score.set(self.gcomprisBoard.sublevel)
		self.draw_image(30, 150, self.pattern)
		self.won_level = 0

		if recreate == "yes":
			self.recreate_drawing_area()

		if self.show_template[self.gcomprisBoard.level-1] == 1:		# level with "show_template"
			predlohy_dir = self.save_path + "/writing_tutor/predlohy/" + self.pattern + "/"
			if not os.path.exists(predlohy_dir):
				print "%s DOES NOT EXISTS!", predlohy_dir
			else:
				arglist = []
				os.path.walk(predlohy_dir,self.walk_callback,arglist)

		ip = get_ip_address('eth0')  # eth0
		datum = time.strftime('%d.%m.%Y_%X').replace('/','.')
		filename1 = self.save_path + "/writing_tutor/predloha_test/" + self.pattern
		self.filename = filename1 + "/" + ip + "__" + datum + ".dat"
		# musi existovat cesta => adresare!
		if not os.path.exists(filename1):
			os.makedirs(filename1)

		self.file_handle = open(self.filename,"w")

		self.show_pen_animation(1)




	def walk_callback(self, arg, dirname, fnames ):
		global area

		# print "dirname: ", dirname
		if len(fnames) < 1:
			print "walk_callback: len(fnames)<1 !! : ", len(fnames)

		for _filename in fnames:
			if os.path.getsize(dirname+_filename) > 0:
				# print "df: ", dirname , _filename
				_filehandle = open(dirname + _filename, "r")
				for line in _filehandle:
					l = line.split()
					#print "x=",l[0]," y=",l[1], " pressure=", l[2]
					self.draw_brush(self.area, float(l[0]), float(l[1]), float(l[2]),'predloha')
					time.sleep(0.010)
				_filehandle.close()
				break # berieme len prvu predlohu v adresari



	# BEGIN OF drawing_area HANDLERS ************************************
	# Create a new backing pixmap of the appropriate size
	def configure_event(self, widget, event):
		x, y, width, height = widget.get_allocation()
		self.pixmap = gtk.gdk.Pixmap(widget.window, width, height)
		#self.pixmap.draw_rectangle(widget.get_style().white_gc, True, 0, 0, width, height)

		#pixbuf = gcompris.utils.load_pixmap("boardicons/notepad.png")
		tmp = self.gcomprisBoard.filename.split("/")
		_file = tmp[len(tmp)-1]
		tmp.remove(_file)
		_board_dir = "/".join(tmp)

		pixbuf =  gtk.gdk.pixbuf_new_from_file(_board_dir + "/writing_tutor/wood_pine.png")	# transparent png
		self.pixmap.draw_pixbuf(widget.get_style().fg_gc[gtk.STATE_NORMAL], pixbuf, 0, 0, 0, 0, width, height)
		pixbuf =  gtk.gdk.pixbuf_new_from_file(_board_dir + "/writing_tutor/notepad.png")	# transparent png
		self.pixmap.draw_pixbuf(widget.get_style().fg_gc[gtk.STATE_NORMAL], pixbuf, 0, 0, 0, 0, width, height)

		draw_guiding_lines(self.pixmap, self.area, self.pattern, 450)

		return True


	# Redraw the screen from the backing pixmap
	def expose_event(self, widget, event):
		x , y, width, height = event.area
		widget.window.draw_drawable(widget.get_style().fg_gc[gtk.STATE_NORMAL], self.pixmap, x, y, x, y, width, height)
		return False


	def draw_brush(self, widget, x, y, pressure, _type='vzorka'):

		colormap = self.area.get_colormap()
		if _type == 'predloha':
			color = colormap.alloc_color(86*256, 153*256, 215*256)  # slabo modra
			rect = (int(x-4*pressure), int(y-4*pressure), int(8*pressure), int(8*pressure))
		else:
			color = colormap.alloc_color(97*256, 105*256, 141*256)  # tmavo seda
			rect = (int(x-5*pressure), int(y-5*pressure), int(10*pressure), int(10*pressure))


		gc = self.area.get_style().fg_gc[gtk.STATE_NORMAL]
		gc.foreground = color

		self.pixmap.draw_rectangle(widget.get_style().fg_gc[gtk.STATE_NORMAL], True, rect[0], rect[1], rect[2], rect[3])
		widget.window.draw_drawable(widget.get_style().fg_gc[gtk.STATE_NORMAL], self.pixmap, rect[0], rect[1], rect[0], rect[1], rect[2], rect[3])

		while gtk.events_pending(): gtk.main_iteration()	# refreshing widget

		# nastavim naspat ciernu
		color = colormap.alloc_color(0,0,0)  # slabo modra
		gc = self.area.get_style().fg_gc[gtk.STATE_NORMAL]
		gc.foreground = color



	def motion_notify_event(self, widget, event):
		#if not self.can_write:		#!!
		#	return True
    	# is_hint => mys
		if event.is_hint:
			x, y, state = event.window.get_pointer()
			pressure = 0.0
			x_tilt = 0.0
			y_tilt = 0.0
		else: # pero tabletu
			state = event.state
			x = event.get_axis(gtk.gdk.AXIS_X)
			y = event.get_axis(gtk.gdk.AXIS_Y)
			pressure = event.get_axis(gtk.gdk.AXIS_PRESSURE)
			x_tilt = event.get_axis(gtk.gdk.AXIS_XTILT)   # musi to podporovat tablet,napr "Wacom Intuos"
			if x_tilt == None:
				x_tilt = 0.0
			y_tilt = event.get_axis(gtk.gdk.AXIS_YTILT)
			if y_tilt == None:
				y_tilt = 0.0

		#wheel = event.get_axis(gtk.gdk.AXIS_WHEEL) #?
		#print "x=",x," y=",y," pressure=",pressure, " x_tilt=", x_tilt," y_tilt=",y_tilt


		if (not (x >= 10 and x <= 485  and  y >= 10	and y<= 485)): 	    #if not in drawing_area_rect:
			if self.writing_status == 1:
				self.writing_status = None
			return True


		if state & gtk.gdk.BUTTON1_MASK and self.pixmap != None:

			if event.is_hint:
				pressure = 1.0
			if pressure == None:
				pressure = 1.0

			if self.writing_status == None and pressure > 0.0:
				self.show_pen_animation(0)
				self.writing_status = 1	# zacal som pisat pismeno/zaznamenavat udaje
				self.motions_count = 1
				self.cas_start = time.time()  # pri zaciatku pisania pismena sa poznaci cas



			if pressure > 0.0:
				self.draw_brush(widget, x, y, pressure)
				self.last_written_point_index = self.motions_count

		if self.writing_status == 1:
			#points[motions_count] = str(int(x)) + " " + str(int(y)) + " " + str(int(pressure)) + "\n"
			self.points[self.motions_count] = str(float(x)) + " " + str(float(y)) + " " + str(pressure) + " " + str(x_tilt) + " " + str(y_tilt) + " " + str((time.time()-self.cas_start)*1000.0) + "\n"
			#print "points[",motions_count,"]=", points[motions_count]
			self.motions_count = self.motions_count + 1

		return True



  # END OF drawing_area HANDLERS ************************************



	#def onBoard(self, x, y):
	#	return x >= 0 and x < self.width and y >= 0 and y < self.height



	def draw_image(self, x, y, _pattern):
		_pixmap = gcompris.utils.load_pixmap("writing_tutor/image_match/" + self.pattern + ".png")

		if self.pattern_image != None:
			self.pattern_image.destroy()

		self.pattern_image = self.rootitem.add(gnomecanvas.CanvasPixbuf, pixbuf = _pixmap, x = 20, y = 150)
		self.pattern_image.show()



	def end(self):
		#gcompris.sound.reopen()

		if self.rootitem != None:
			self.rootitem.destroy()
			self.rootitem = None
			gcompris.score.end()
		pass


	def get_datfiles_list(self, _pattern):

		dat_list = glob.glob(self.save_path + "/writing_tutor/predlohy/" + _pattern + "/*.dat")
		res_dat_list = []
		for _file in dat_list:
			if os.stat(_file).st_size > 0:
				res_dat_list.append(_file)			# v zozname necha len neprazdne subory

		return res_dat_list


	def _remove_empty_list_item(self, _list):
		if '' in _list:
			_list.remove('')

		return _list


	def parse_sequence(self):
		""" parse sequence string into levels and sublevels """
		# sequence:
		# a|b|@kruh$  - $ :level separator
		# x|@tvar

		_sequence = re.sub(r'\s', '', self.sequence)		# remove white spaces
		_sequence = _sequence.split("$")			# divide into levels (this activity has 2 levels)
		_sequence = self._remove_empty_list_item(_sequence)	# remove empty level string if extsts
		self.sequence = []


		tmp = self.gcomprisBoard.filename.split("/")
		_file = tmp[len(tmp)-1]
		tmp.remove(_file)
		_board_dir = "/".join(tmp)

		_level = 0
		for _s in _sequence:					# for each level (should be max 2)
			_s_tmp = _s
			_dat_list = self._remove_empty_list_item(_s_tmp.split("|"))
			_non_empty_dat_list = _dat_list[:]
			for s in _dat_list:										# for each pattern in level
				if s != '*':
					img_path = _board_dir + "/writing_tutor/image_match/" + s + ".png"
					if (len(self.get_datfiles_list(s)) < 1) or (not os.path.exists(img_path)):	# iba ak existuju .dat subor predlohy a obrazok
						_non_empty_dat_list.remove(s)

			if _non_empty_dat_list.count('*') > 0:
				_non_empty_dat_list.remove('*')
				self.show_template[_level] = 1
			else:
				self.show_template[_level] = 0

			_level += 1

			if len(_non_empty_dat_list) > 0:
				self.sequence.append(_non_empty_dat_list)





	# Called by gcompris when the user click on the level icon
	def set_level(self, level):
		#print "set_level: level=",level

		if self.gcomprisBoard.sublevel-1 == self.gcomprisBoard.number_of_sublevel and self.gcomprisBoard.level == self.gcomprisBoard.maxlevel:
			gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
			self.won_level = 1
			return

		self.gcomprisBoard.level=level
		self.gcomprisBoard.sublevel=1
		self.gcomprisBoard.number_of_sublevel = len(self.sequence[self.gcomprisBoard.level-1])
		gcompris.bar_set_level(self.gcomprisBoard)		# nastavi kocku na cislo levelu

		gcompris.score.start(gcompris.score.STYLE_NOTE, 10, 50, len(self.sequence[self.gcomprisBoard.level-1]))

		self.start_2("yes")



	def increment_level(self):
		self.gcomprisBoard.sublevel += 1
		if self.gcomprisBoard.sublevel > self.gcomprisBoard.number_of_sublevel:
			self.set_level(self.gcomprisBoard.level % self.gcomprisBoard.maxlevel + 1)
		else:
			self.start_2("yes")


  # stlacenie "ruky"
	def ok(self):

		self.writing_status = None

		# zapise do suboru vsetky body od prveho prilozenia pera, az po posledny napisany bod..

		if self.last_written_point_index != None:		# if something was written...
			for i in range(self.last_written_point_index):
				self.file_handle.write(self.points[i+1])

			self.file_handle.close()

			self.show_sablier_animation(1)

			_dtw = DTW.dtw.DTW(self.save_path, self.pattern)
			dtw_res = _dtw.dtw(self.filename, 1)
			print "ok: dtw_res=", str(dtw_res)

			self.show_sablier_animation(0)

			if self.pattern[0] == '@':
				limit = 13
			else:
				limit = 9

			if self.show_template[self.gcomprisBoard.level-1] == 1:
				limit = 5

			if dtw_res < limit:
				gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.RANDOM)
				self.won_level = 1
			else:
				gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.RANDOM)
				self.won_level = 1

		else:	# none was written
			gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.RANDOM)
			self.won_level = 1


		self.last_written_point_index = None






	def key_press(self, keyval, commit_str, preedit_str):
		return False

	def repeat(self):
		print "repeat"
		self.start_2("yes")



	def pause(self, pause):			# called after config dialog, help, bonus
		print "pause"

		self.board_paused = pause

		if(pause):
			self.area.hide()
			self.show_pen_animation(0)
			if self.label != None:
				self.label.hide()
		else:
			self.area.show()
			if self.won_level:
                # if we are paused, then unpaused it means that they beat the sublevel
				self.increment_level()
			else:
				self.set_level(self.gcomprisBoard.level)

		## There is a problem with GTK widgets, they are not covered by the help, We hide/show them here
		#if(pause):
		#	self.area.hide()
		#else:
		#	self.area.show()
		#	self.repeat()


		return



	def config(self):
		print "Config"
		pass

	def config_stop(self):
		print "config_stop", self
		pass




	def show_pen_animation(self, show):
		if show == 0:
			if self.ap != None:
				self.ap.gnomecanvas.hide()
		else:
			if self.ap == None:
				self._ap = gcompris.anim.Animation("writing_tutor/anim_pen.txt")
				self.ap = gcompris.anim.CanvasItem(self._ap, self.rootitem)
				self.ap.gnomecanvas.set(x=80, y=gcompris.BOARD_HEIGHT - 70)
				self.ap.setState(0)
				self.ap.gnomecanvas.show()
			else:
				self.ap.gnomecanvas.show()


	def show_sablier_animation(self, show):
		if show == 0:
			if self.sb != None:
				self.sb.gnomecanvas.hide()
		else:
			if self.sb == None:
				self._sb = gcompris.anim.Animation("connect4/sablier.txt")
				self.sb = gcompris.anim.CanvasItem(self._sb, self.rootitem)
				self.sb.gnomecanvas.set(x=40, y= gcompris.BOARD_HEIGHT - 230)
				self.sb.setState(0)
				self.sb.gnomecanvas.show()
			else:
				self.sb.gnomecanvas.show()





	def set_tablet_indicator(self, tablet_status):
		"""
		Description:
		Set tablet indicator icon

	   	Arguments:
    	tablet_status -- 1 or 0 indicating status of tablet device (enabled or disabled)
    	"""
		global _global_rootitem
		global _global_tablet_indicator

		#print "set_tablet_indicator(", tablet_status, ")"

		if tablet_status == 1:
			_pixmap = gcompris.utils.load_pixmap("writing_tutor/tablet_active.png")
		else:
			_pixmap = gcompris.utils.load_pixmap("writing_tutor/tablet_inactive.png")

		if _global_tablet_indicator != None:
			_global_tablet_indicator.destroy()

		_global_tablet_indicator = _global_rootitem.add(gnomecanvas.CanvasPixbuf, pixbuf = _pixmap, x = 10, y = gcompris.BOARD_HEIGHT -50)
		_global_tablet_indicator.show()



	def config_start(self, profile):
		print "config_start"
		global _global_rootitem


		# show only tablet configuration. save_path, sequence only in admin mode
		if _global_rootitem != None:		# not in admin mode
			dlg = GtkInputDialog(self)
			return

		self.configure_profile = profile

		self.config_dict = self.init_config()
		self.config_dict.update(gcompris.get_conf(profile, self.gcomprisBoard))

		self.main_vbox = gcompris.configuration_window (\
		_('Hra <b>%s</b> - nastavenie pre profil <b>%s</b>') % ('Tutor', profile.name), self.save_config)

		gcompris.separator()
		self.button = gtk.Button("Nastavenie tabletu...")
		self.button.connect("clicked", self.input_dlg_callback)
		self.button.show()
		self.main_vbox.pack_start(self.button, False, False, 0)
		gcompris.separator()

		# !! ak je v admin mode, tak nemoze nastavovat tablet...
		if _global_rootitem == None:	# in admin mode
			self.button.set_state(gtk.STATE_INSENSITIVE)


		gcompris.textview(_('Adresar, kde sa budu ukladat vzorky/predlohy: '), 'save_path', 'desc', self.config_dict['save_path'],
						 self.validate_callback)
		gcompris.separator()

		gcompris.textview(_('Postupnosti tvarov pre testovanie: '), 'image_match_sequence', 'desc', 			self.config_dict['image_match_sequence'], self.validate_callback)
		gcompris.separator()


 	def validate_callback(self, key, value, z):
		#print "validate_callback: key=", key, ", value=", value, ", z=", z

		if key == 'save_path':
			if not os.path.exists(value):
				return False
		if key == 'sequence':
			pass

		return True


	def input_dlg_callback(self, table):
		dlg = GtkInputDialog()


	def dummy_enter_callback(self, widget, _entry):
		pass

	def clb_dummy(self, x=None):
		pass

	def save_config(self, table):
		print "save_config: table=", table
		global _global_rootitem

		if table == None:
			return

		for key,value in table.iteritems():
			gcompris.set_board_conf(self.configure_profile, self.gcomprisBoard, key, value)		# save config to db

		if _global_rootitem != None:
			self.start_2("no")


	def configuration(self, value, init):
		if self.config_dict.has_key(value):
			return eval(self.config_dict[value])
		else:
			return init


	def init_config(self):
		default_config_dict = { 'save_path' : gcompris.get_properties().user_dir ,
					'image_match_sequence'  : 'a|b|g $ @kruh|@stvorec'
					}
		return default_config_dict

		# sequence format:

		#a|b|c|d $ 			1. level
		#@kruh $			2. level



# END OF class Gcompris_image_match ************************************




def stop_board():
  gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)



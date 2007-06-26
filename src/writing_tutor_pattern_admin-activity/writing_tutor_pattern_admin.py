#   gcompris/tutor/pattern_admin.py -
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


from gettext import gettext as _

from  writing_tutor_common import *

_global_rootitem = None		# for config dialog
_global_tablet_indicator= None



# BEGIN OF class Gcompris_pattern_admin **************************
class Gcompris_writing_tutor_pattern_admin:

	_ap = None
	ap = None

	rootitem = None
	board_paused = False
	last_selected_pattern = None
	last_selected_writing_type = None
	pattern = None
	writing_type = "vzorky"
	save_path = None
	sequence = None
	cursor_index = 0
	patterns_count = 0
	area = None
	can_write = False
	tree_already_clicked = False
	drop_button = None
	dat_filename = None
	pixmap = None						# drawing area pixmap
	filename = None
	file_handle = None
	writing_status = None
	last_written_point_index = None
	motions_count = None
	points =  [None] * 2000
	cas_start = None


	def __init__(self, gcomprisBoard):
		# this constructor is called for every refresh of gcompris board, eg. help window, config window.. -> global variables may be needed for
		# persistence..

		self.gcomprisBoard = gcomprisBoard

		gap = 10
		panel_x = 130
		self.select_area = (gap , gap, panel_x + gap , gcompris.BOARD_HEIGHT-gap-100)
		self.panel_area  = (panel_x + 3*gap , 0, gcompris.BOARD_WIDTH-gap, gcompris.BOARD_HEIGHT-gap)



	def __del__(self):
		pass


	def start(self):
		global _global_rootitem

		self.window = self.gcomprisBoard.canvas.get_toplevel()

		#get default values
		self.config_dict = self.init_config()

		#replace configured values (..ak su v db..)
		self.config_dict.update(gcompris.get_board_conf())

		self.save_path = self.config_dict['save_path']
		self.sequence = self.config_dict['sequence']

		#print "start: save_path=",self.save_path, ", sequence=", self.sequence

		self.gcomprisBoard.level=1
		self.gcomprisBoard.maxlevel=1
		self.gcomprisBoard.sublevel=1
		self.gcomprisBoard.number_of_sublevel=1

		# Create our rootitem. We put each canvas item in it so at the end we
		# only have to kill it. The canvas deletes all the items it contains automaticaly.
		self.rootitem = self.gcomprisBoard.canvas.root().add(gnomecanvas.CanvasGroup, x=0.0, y=0.0)
		_global_rootitem = self.rootitem


		gcompris.bar_set( gcompris.BAR_LEVEL|gcompris.BAR_OK|gcompris.BAR_CONFIG )
		#gcompris.bar_hide(1)

		gcompris.set_background(self.rootitem, gcompris.skin.image_to_skin("gcompris-bg.jpg"))


		# graphics controls...

		self.left_win = gtk.ScrolledWindow()
		self.left_win.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.left_win.set_shadow_type(gtk.SHADOW_ETCHED_OUT)

		self.model = gtk.ListStore(str, str)
		self.treeview  = gtk.TreeView(self.model)
		self.treeview.connect('cursor-changed', self.clb_tree_cursor_changed)
		self.treeview.add_events(gtk.gdk.BUTTON_PRESS_MASK)
		self.treeview.connect('button_press_event', self.clb_tree_button_press)
		colnum = 2
		colwidths = [90, 20]
		coltexts  = ['Tvar', 'Pocet']

		for col in range(colnum):
			renderer = gtk.CellRendererText()
			renderer.set_property('ellipsize', True)
			#renderer.set_property('editable', True)
			renderer.set_property('xpad', 5)

			#renderer.connect('edited', on_edit_item, col)
			treecol = self.treeview.insert_column_with_attributes(-1, coltexts[col], renderer, text=col)
			treecol.set_spacing(10)
			treecol.set_resizable(True)
			treecol.set_alignment(0.5)
			treecol.set_data('id', col)
			treecol.set_data('renderer', renderer)
			treecol.set_min_width(colwidths[col])


		self.update_tree_data()

		self.left_win.add(self.treeview)

		self.rootitem.add(gnomecanvas.CanvasWidget, widget=self.left_win, x=self.select_area[0], y=self.select_area[1], width=self.select_area[2],
							 height=self.select_area[3])
		self.treeview.show()
		self.left_win.show()



		# BEGIN OF radio buttons ************************************
		self.rbgr = gtk.RadioButton(label="vzorky")
		self.rbgr.connect("toggled", self.rb_callback, "vzorky")
		self.rootitem.add(gnomecanvas.CanvasWidget, widget=self.rbgr, x=10, y=430, width=80, height=20 )
		self.rbgr.show()

		self.rb1 = gtk.RadioButton(group=self.rbgr, label="predlohy")
		self.rb1.connect("toggled", self.rb_callback, "predlohy")
		self.rootitem.add(gnomecanvas.CanvasWidget, widget=self.rb1, x=80, y=430, width=80, height=20 )
		self.rb1.show()
		# END OF radio buttons ************************************

		gcompris.set_background(self.gcomprisBoard.canvas.root(),
					gcompris.skin.image_to_skin("gcompris-bg.jpg"))
		#BC_pixmap = gcompris.utils.load_pixmap("writing_tutor/bg.png")
		#BCself.bg_pixmap = self.rootitem.add(gnomecanvas.CanvasPixbuf, pixbuf = _pixmap,  x=0, y=0)


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

		self.treeview.set_cursor(self.cursor_index)

		self.start_2()



	def clb_tree_cursor_changed(self, treeview, user_param1=None):
		if self.tree_already_clicked:
			self.tree_already_clicked = False
		else:
			#print "clb_tree_cursor_changed:"
			self.show_pen_animation(0)
			treeselection = treeview.get_selection()
			(model, iter) = treeselection.get_selected()
			self.pattern =  model.get(iter, 0)[0]
			pocet =  model.get(iter, 1)[0]

			self.cursor_index = model.get_path(iter)[0]
			self.show_drop_button(0)
			self.start_2("yes")
			self.can_write = True


	def clb_tree_button_press(self, widget, event):

		#print "clb_tree_button_press: widget=", widget, ", event.button=",event.button, ", event.type=", event.type
		if event.button == 3:

			self.show_pen_animation(0)
			self.can_write = False
			treeselection = self.treeview.get_selection()
			(model, iter) = treeselection.get_selected()
			if model == None or iter == None:
				return
			self.pattern =  model.get(iter, 0)[0]
			pocet =  model.get(iter, 1)[0]

			self.cursor_index = model.get_path(iter)[0]
			#print "self.cursor_index=", self.cursor_index

			if self.last_selected_pattern != self.pattern or self.last_selected_writing_type != self.writing_type:
				self.pattern_arr_index = 0
				self.last_selected_pattern = self.pattern
				self.last_selected_writing_type = self.writing_type
				self.last_selected_pattern_count = len(self.get_datfiles_list(self.pattern))
			else:
				if self.last_selected_pattern_count > 0:
					self.pattern_arr_index = (self.pattern_arr_index + 1) % self.last_selected_pattern_count

			if self.last_selected_pattern_count > 0:
				self.dat_filename = self.get_datfiles_list(self.pattern)[self.pattern_arr_index]

				#print "paint:self.patern=", self.pattern, self.dat_filename
				self.configure_event(self.area, None)
				self.paint_pattern(self.dat_filename)

				# zobrazi cestu k .dat suboru
				tmp = self.dat_filename.split("/")
				_dat_filename_file = tmp[len(tmp)-1]
				tmp.remove(_dat_filename_file)
				_dat_filename_dir = "/".join(tmp) + "/"
				context = self.area.create_pango_context()
				desc = pango.FontDescription('Sans 9')
				gc = self.pixmap.new_gc()
				self.layout = self.area.create_pango_layout(_dat_filename_dir)		# v 1. riadku nazov adresar
				self.layout.set_font_description(desc)
				self.pixmap.draw_layout(gc, x=30, y=15, layout=self.layout)
				self.layout = self.area.create_pango_layout(_dat_filename_file)		# v 2. riadku nazov .dat suboru
				self.layout.set_font_description(desc)
				self.pixmap.draw_layout(gc, x=30, y=30, layout=self.layout)
				self.area.queue_draw_area(0, 0, 650, 500)

				# zobraz tlacitko na zmazanie
				self.show_drop_button(1)



			self.tree_already_clicked = True




	def get_datfiles_list(self, _pattern):

		dat_list = glob.glob(self.save_path + "/tutor/" + self.writing_type + "/" + _pattern + "/*.dat")
		empty_dat_list = []
		for _file in dat_list:
			if os.stat(_file).st_size <= 0:
				empty_dat_list.append(_file)			# v zozname necha len neprazdne subory

		set_dat_list = set(dat_list)
		set_empty_dat_list = set(empty_dat_list)

		res = set_dat_list - set_empty_dat_list		# rozdiel mnozin
		return list(res)


	def update_tree_data(self):
		#print "update_tree_data: writing_type=", self.writing_type

		# sequence:  a|b|c|d|A|B|C|D|ab|@kruh

		self.model.clear()
		self.patterns_count = 0

		_sequence = re.sub(r'\s', '', self.sequence)
		_sequence = _sequence.replace("$", "|")		# v tejto aktivite nie su levely, ale ak nahodou je separator $ tak -> |
		_sequence = _sequence.split("|")
		for s in _sequence:
			if len(s) > 0:
				self.model.append([s, str(len(self.get_datfiles_list(s)))])
				self.patterns_count += 1


	def tree_select_next_item(self):
		self.cursor_index = (self.cursor_index + 1 ) % self.patterns_count
		self.treeview.set_cursor(self.cursor_index)
		#print "tree_select_next_item: self.cursor_index=", self.cursor_index, ", self.patterns_count=", self.patterns_count



	def show_drop_button(self, show):
		#print "show_drop_button: show=", show

		if show == 1:
			if self.drop_button == None:
				_pixmap = gcompris.utils.load_pixmap("writing_tutor/drop.png")
				self.drop_button = self.rootitem.add(gnomecanvas.CanvasPixbuf, pixbuf = _pixmap, x = 90, y = gcompris.BOARD_HEIGHT -60)
				self.drop_button.connect("event", self.drop_button_click_event)
			self.drop_button.show()

		else:
			if self.drop_button != None:
				self.drop_button.hide()


	def drop_button_click_event(self, widget, event):
		#print "drop_button_click_event: event.type=", event.type
		if event.type == gtk.gdk.BUTTON_PRESS:		 # and je nejaky pattern ...
			# zmaz aktualny datafile + update_tree_data+ zobraz dalsie
			# dorob !!
			if os.path.exists(self.dat_filename):
				os.remove(self.dat_filename)
				#print ".dat file deleted: ", self.dat_filename

				gcompris.sound.play_ogg_cb("sounds/crash.ogg", self.clb_dummy)
				self.pattern_arr_index = 0
				self.last_selected_pattern_count = len(self.get_datfiles_list(self.pattern))
				self.update_tree_data()
				self.treeview.set_cursor(self.cursor_index)

			return True


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

		#area.size(400, 300)
		#self.area.set_size_request(550, 600)

		self.rootitem.add(gnomecanvas.CanvasWidget, widget=self.area, x=self.panel_area[0], y=self.panel_area[1],
							width=self.panel_area[2], height=self.panel_area[3])

		self.area.show()
		# END OF drawing_area ************************************

		#print "recreate_drawing_area()"



	def paint_pattern(self, pattern_dat_file):
		#print "paint_pattern: ", pattern_dat_file
		if os.path.getsize(pattern_dat_file) > 0:
			_filehandle = open(pattern_dat_file, "r")
			for line in _filehandle:
				l = line.split()
				#print "x=",l[0]," y=",l[1], " pressure=", l[2]
				if len(l) == 6:
					self.draw_brush(self.area, float(l[0]), float(l[1]), float(l[2]))
					#time.sleep(0.010)
				else:
					print "BAD FILE FORMAT: "
					print pattern_dat_file
					break

		_filehandle.close()


	def start_2(self, recreate="yes"):
		#print "start_2"

		ip = get_ip_address('eth0')

		datum = time.strftime('%d.%m.%Y_%X').replace('/','.')
		filename1 = self.save_path + "/tutor/" + self.writing_type + "/" + self.pattern
		self.filename = filename1 + "/" + ip + "__" + datum + ".dat"
		# musi existovat cesta => adresare!
		if not os.path.exists(filename1):
			os.makedirs(filename1)

		self.file_handle = open(self.filename,"w")
		if recreate == "yes":
			self.recreate_drawing_area()

		#self.draw_guiding_lines()
		self.show_pen_animation(1)



	# BEGIN OF drawing_area HANDLERS ************************************
	# Create a new backing pixmap of the appropriate size
	def configure_event(self, widget, event):

		x, y, width, height = widget.get_allocation()
		self.pixmap = gtk.gdk.Pixmap(widget.window, width, height)
		#pixmap.draw_rectangle(widget.get_style().white_gc, True, 0, 0, width, height)

		pixbuf = gcompris.utils.load_pixmap("writing_tutor/pattern_admin_bg.png")
		self.pixmap.draw_pixbuf(widget.get_style().fg_gc[gtk.STATE_NORMAL], pixbuf, 0, 0, 0, 0, width, height)


		draw_guiding_lines(self.pixmap, self.area, self.pattern, 570)

		return True


	# Redraw the screen from the backing pixmap
	def expose_event(self, widget, event):
		x , y, width, height = event.area
		widget.window.draw_drawable(widget.get_style().fg_gc[gtk.STATE_NORMAL], self.pixmap, x, y, x, y, width, height)
		return False

	# Draw a rectangle on the screen
	def draw_brush(self, widget, x, y, pressure):
		rect = (int(x-5*pressure), int(y-5*pressure), int(10*pressure), int(10*pressure))
		self.pixmap.draw_rectangle(widget.get_style().black_gc, True, rect[0], rect[1], rect[2], rect[3])
		widget.queue_draw_area(rect[0], rect[1], rect[2], rect[3])



	def motion_notify_event(self, widget, event):
		if not self.can_write:
			return True

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


		if (not (x >= 10 and x <= 550  and  y >= 10	and y<= 500)): 	    #if not in drawing_area_rect:
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
			self.motions_count += 1

		return True

# END OF drawing_area HANDLERS ************************************


	def rb_callback(self, widget, data):
		# zavola sa len pri aktivovani rb, nie pri deaktivovani
		if not widget.get_active():
			return

		#print "rb_callback"

		self.show_drop_button(0)
		self.show_pen_animation(0)
		self.writing_type = data
		self.update_tree_data()
		self.cursor_index = 0
		self.treeview.set_cursor(self.cursor_index)
		self.start_2("no")




	def end(self):
		#gcompris.sound.reopen()

		if self.rootitem != None:
			self.rootitem.destroy()
			self.rootitem = None
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




	def ok(self):
		#print "ok"

		self.writing_status = None

		# od naposledy napisaneho bodu vsetky vymaz (s tlakom=0)
		# zapise do suboru vsetky body od prveho prilozenia pera, az po posledny napisany bod..
		if self.last_written_point_index != None:
			for i in range(self.last_written_point_index):
				#print "i=",i," , last_written_point_index=",last_written_point_index
				self.file_handle.write(self.points[i+1])
			self.file_handle.close()

			_empty_dat_list = glob.glob(self.save_path + "/tutor/" + self.writing_type + "/" + self.pattern + "/*.dat")
			for _file in _empty_dat_list:
				if os.stat(_file).st_size <= 0:
					os.remove(_file)			 # zmaze .dat subory 0-vej dlzky
					#print "empty file deleted: ", _file
			#if os.path.getsize(filename) < 1:

			#self.start_2() # !!
			self.update_tree_data()
			self.can_write = False
			self.set_level(1)


	def set_level(self, level):
		#print "set_level"

		self.tree_select_next_item()
		self.show_pen_animation(1)
		self.start_2("yes")




	def key_press(self, keyval, commit_str, preedit_str):
		return False

	def repeat(self):
		print "repeat"

	def pause(self, pause):
		self.board_paused = pause

		# There is a problem with GTK widgets, they are not covered by the help, We hide/show them here
		if(pause):
			self.left_win.hide()
			self.area.hide()
			self.rbgr.hide()
			self.rb1.hide()

		else:
			self.left_win.show()
			self.area.show()
			self.rbgr.show()
			self.rb1.show()

			self.repeat()
		return


	def config(self):
		print "Config"
		pass

	def config_stop(self):
		print "config_stop", self
		pass


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

		gcompris.textview(_('Postupnost tvarov pre zbieranie vzoriek: '), 'sequence', 'desc', self.config_dict['sequence'],
						 self.validate_callback)
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
		dlg = GtkInputDialog(self)


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


	def init_config(self):
		default_config_dict = { 'save_path' : gcompris.get_properties().user_dir,
					'sequence'  : 'a|b|c|d|e|A|B|C|D|E|_A|_B|1|2|@kruh|@stvorec'
					}
		return default_config_dict

		# sequence format:
		#a|b|c|d|A|B|C|D|ab|@kruh




# END OF class Gcompris_pattern_admin ************************************



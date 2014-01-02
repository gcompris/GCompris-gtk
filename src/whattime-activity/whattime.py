#  gcompris - whattime.py
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
# whattime activity.
import sys
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.sound
import gobject
import goocanvas
import pango
import string
import random
import math

from gcompris import gcompris_gettext as _

#Array Declaration
#hour array
hour_arr = ['1','2','3','4','5','6','7','8','9','10','11','12']
random.shuffle(hour_arr)

#minute -level 2 array
minute_arr_one = ['0','15','30','45']
random.shuffle(minute_arr_one)

#minute -level 3 array
minute_arr_two = ['0','1','2','3','4','5','6','7','8','9',
		  '10','11','12','13','14','15','16','17','18','19',
		  '20','21','22','23','24','25','26','27','28','29',
		  '30','31','32','33','34','35','36','37','38','39',
		  '40','41','42','43','44','45','46','47','48','49',
		  '50','51','52','53','54','55','56','57','58','59']
random.shuffle(minute_arr_two)

#
# The name of the class is important. It must start with the prefix
# 'Gcompris_' and the last part 'whattime' here is the name of
# the activity and of the file in which you put this code. The name of
# the activity must be used in your menu.xml file to reference this
# class like this: type="python:whattime"
#
class Gcompris_whattime:
  """Empty gcompris Python class"""
  random_hour=hour_arr[0]
  random_minute=minute_arr_one[0]

  def __init__(self, gcomprisBoard):
    print "whattime init"

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.counter = 0
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level = 1
    self.gcomprisBoard.maxlevel = 4
    self.gcomprisBoard.sublevel = 1
    self.gcomprisBoard.number_of_sublevel = 1

    #Boolean variable decaration
    self.mapActive = False
    self.stopped = True
    self.wrongcount = 0
    self.correctcount = 0
    self.hrdone=0
    
    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    print "whattime start"

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_set_level(self.gcomprisBoard)
    
    pixmap = gcompris.utils.load_svg("whattime/target.svg")
    gcompris.bar_set_repeat_icon(pixmap)
    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT_ICON)
    gcompris.bar_location(300,-1,0.8)
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())
    self.stopped = False
    self.board_upper(self.gcomprisBoard.level)

  def end(self):
    print "whattime end"
    self.stopped = True
    # Remove the root item removes all the others inside it
    self.rootitem.remove()

  def ok(self):
    print("whattime ok.")

  def repeat(self):
    print("whattime repeat.")
    if(self.mapActive):
          self.end()
          self.start()
          self.mapActive = False
    else:
          self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
          self.rootitem = goocanvas.Group(parent=
                                   self.gcomprisBoard.canvas.get_root_item())
          gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
          
          #text displaying how to read clock
          goocanvas.Text(
	    parent = self.rootitem,
	    x=400.0,
	    y=40.0,
	    text=_("Learn to Read Clock"),
	    fill_color="black",
	    width=5000,
	    anchor = gtk.ANCHOR_CENTER,
	    alignment = pango.ALIGN_CENTER
	    )
          
          #text displaying hour hand for left hand clock
          goocanvas.Text(
	    parent = self.rootitem,
	    x=200.0,
	    y=80.0,
	    text=_("Hour Hand"),
	    fill_color="black",
	    width=5000,
	    anchor = gtk.ANCHOR_CENTER,
	    alignment = pango.ALIGN_CENTER
	    )
          
          #text displaying hour hand for right hand clock
          goocanvas.Text(
	    parent = self.rootitem,
	    x=550.0,
	    y=80.0,
	    text=_("Minute Hand"),
	    fill_color="black",
	    width=5000,
	    anchor = gtk.ANCHOR_CENTER,
	    alignment = pango.ALIGN_CENTER
	    )
	    
          #clock on right
          self.Clockstudy1 = goocanvas.Image(
	    parent = self.rootitem,
	    pixbuf = gcompris.utils.load_pixmap("whattime/clock_study1.png"),
	    x = 400,
	    y = 100,
	    width = 300,
	    height = 300,
	    tooltip = "Reading different positions of minute hand on clock"
	  )
          gcompris.utils.item_focus_init(self.Clockstudy1,None)
          
          #clock on left
          self.Clockstudy2 = goocanvas.Image(
	    parent = self.rootitem,
	    pixbuf = gcompris.utils.load_pixmap("whattime/clock_study2.png"),
	    x = 50,
	    y = 100,
	    width = 300,
	    height = 300,
	    tooltip = "Reading different time of hour hand on clock"
	  )
          gcompris.utils.item_focus_init(self.Clockstudy2,None)
          
          #text displaying go back
          self.goback = goocanvas.Text(
	    parent = self.rootitem,
	    x=700.0,
	    y=450.0,
	    text=_("Move Back"),
	    fill_color="green",
	    width=5000,
	    anchor = gtk.ANCHOR_CENTER,
	    alignment = pango.ALIGN_CENTER
	    )
	  self.goback.connect("button_press_event", self.move_back)
          gcompris.utils.item_focus_init(self.goback,None)
          
          #Move back item
          self.backitem = goocanvas.Image(parent = self.rootitem,
                    pixbuf = gcompris.utils.load_pixmap("whattime/back.svg"),
                    x = 600,
                    y = 425,
                    tooltip = "Move Back"
                    )
          self.backitem.connect("button_press_event", self.move_back)
          gcompris.utils.item_focus_init(self.backitem, None)

          self.mapActive = True

  def move_back(self,event,target,item):
    print("In move_back function.")
    self.end()
    self.start()

  #mandatory but unused yet

  def config_stop(self):
    pass

  # Configuration function.
  
  def config_start(self, profile):
    print("whattime config_start.")

  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_whattime key press keyval=%i %s" % (keyval, strn))
    
  def pause(self, pause):
    print("whattime pause. %i" % pause)
    # Hack for widget that can't be covered by bonus and/or help
    if pause:
       self.hour.props.visibility = goocanvas.ITEM_INVISIBLE
    else:
      self.hour.props.visibility = goocanvas.ITEM_VISIBLE

    if(pause == 0):
      self.counter += 1
      if(self.counter == self.sublevel):
	  self.increment_level()
      self.end()
      self.start()

  def set_level(self, level):
    gcompris.sound.play_ogg("sounds/recieve.wav")
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()
    print("whattime set level. %i" % level)

  def increment_level(self):
    self.counter = 0
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
        self.gcomprisBoard.sublevel=1
        self.gcomprisBoard.level += 1
        if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
            self.gcomprisBoard.level = 1
            
  def board_upper(self, level):
    
    if(level == 1):
	  # Set a background image
	  gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),"whattime/background.jpg")
    
	  goocanvas.Text(parent=self.rootitem,
			x=390,
			y=100,
			fill_color="dark blue",
			font="Sans 15",
			anchor=gtk.ANCHOR_CENTER,
			text="Clock-Activity : What is the Time ?")
		  
	  #Activity Description
	  goocanvas.Text(parent=self.rootitem,
			  x=520,
			  y=260,
			  fill_color="dark blue",
			  font="Sans 15",
			  anchor=gtk.ANCHOR_CENTER,
			  text="Reading a clock is the basic\n"
				"thing a child must know.\n"
				"The clock has 12 basic marks on it,\n"
				"relative to which we have 60 different\n"
				"positions indicating various time\n"
				"Moreover,there are 24 hours in a day\n"
				"First 12 hours indicated by AM.\n"
				"And the later half of day by PM.\n"
				"We will play with 12-hour format.\n"
			)
	
	  #displaying the main clock
	  self.Clock = goocanvas.Image(
	    parent = self.rootitem,
	    pixbuf = gcompris.utils.load_pixmap("whattime/Eternal_clock.png"),
	    x = 50,
	    y = 150,
	    tooltip = "This is just a representation of clock",
	    width = 250,
	    height = 200    
	  )
	  
	  self.nextlev = goocanvas.Text(parent=self.rootitem,
			  x=215,
			  y=400,
			  fill_color="dark red",
			  font="Sans 15",
			  anchor=gtk.ANCHOR_CENTER,
			  text="Okay. Got it !\n"
			       "Next Level Please ->"
			)  
	  self.nextlev.connect("button_press_event", self.next_level)
          gcompris.utils.item_focus_init(self.nextlev,None)
          
    elif(level == 2):
	  #selecting a random minute hand position
	  #for level three, it would be either 00 to 03
	  self.random_minute = minute_arr_one[random.randint(0,3)]
	  self.sublevel = 5
    
    elif(level == 3):
	  #selecting a random minute hand position
	  #for level three, it would be either 00 to 59 
	  self.random_minute = minute_arr_two[random.randint(0,59)]
	  self.sublevel = 10
	  
    elif(level == 4):
          self.sublevel = 1
          goocanvas.Text(parent=self.rootitem,
			  x=400,
			  y=220,
			  fill_color="black",
			  font="Sans 20",
			  anchor=gtk.ANCHOR_CENTER,
			  text="ScoreBoard: \n\n\n"
			  "Correct Attempts:"+str(self.correctcount)+"\n"
			  "Wrong Attempts:"+str(self.wrongcount)+"\n"
			  "Total Attempts:"+str(self.correctcount+self.wrongcount)+"\n"
			)  
    
    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),"whattime/background.jpg")

    if ( level == 3 or level == 2):
	goocanvas.Text(
	  parent = self.rootitem,
	  x=450.0,
	  y=50.0,
	  text=_("Can You Guess \n"
		"What Time Is It ? "),
	  fill_color="dark blue",
	  font="Sans 15",
	  anchor = gtk.ANCHOR_CENTER,
	  alignment = pango.ALIGN_CENTER
	  )
	
	#displaying the main clock
	self.Clock = goocanvas.Image(
	  parent = self.rootitem,
	  pixbuf = gcompris.utils.load_pixmap("whattime/alarm-clock.png"),
	  x = 250,
	  y = 60,
	  width = 400,
	  height = 400    
	)
	
	#selecting a random hour hand position
	self.random_hour = hour_arr[random.randint(0,11)] 
	print("Random hour is "+self.random_hour)
	print("Random minute is "+self.random_minute)
	
	#yellow line indicating hour
	goocanvas.Polyline(
	  parent = self.rootitem,
	  points = goocanvas.Points([(450,280),
			(450+75*math.cos(math.radians(int(self.random_hour)*30 - 90 + int(self.random_minute)/2)),280+75*math.sin(math.radians(int(self.random_hour)*30 - 90 + int(self.random_minute)/2)))]),
	  stroke_color = "yellow", 
	  line_width = 5.0)
	  
	#blue line indicating hour
	goocanvas.Polyline(
	  parent = self.rootitem,
	  points = goocanvas.Points([(450,280), (450+120*math.cos(math.radians(int(self.random_minute)*6 - 90)),280+120*math.sin(math.radians(int(self.random_minute)*6 - 90)))]),
	  stroke_color = "blue", 
	  line_width = 2.5)
	  
	#displaying if the answer is wrong    
	self.indicator =goocanvas.Text(
        parent = self.rootitem,
        x=100.0,
        y=200.0,
        width = 700,
        font=gcompris.skin.get_font("gcompris/subtitle"),
        text=(""),
        fill_color="dark blue",
        anchor = gtk.ANCHOR_CENTER,
        alignment = pango.ALIGN_CENTER,
        )
        
        #displaying ":" between minute and hour
        goocanvas.Text(
	  parent = self.rootitem,
	  x=100.0,
	  y=400.0,
	  text=_(" :"),
	  fill_color="dark blue",
	  font="Sans 15",
	  anchor = gtk.ANCHOR_CENTER,
	  alignment = pango.ALIGN_CENTER
	  )

        #OK Button
	ok = goocanvas.Svg(parent = self.rootitem,
			  svg_handle = gcompris.skin.svg_get(),
			  svg_id = "#OK",
			  tooltip = "Click to confirm your answer"
			  )
	ok.translate(-400,0)

	hour_item = self.hour_text()
	minute_item = self.minute_text()
	
	ok.connect("button_press_event", self.ok_event, hour_item, minute_item )
	gcompris.utils.item_focus_init(ok, None)

  def minute_text(self):
    self.minute = gtk.Entry()

    self.minute.modify_font(pango.FontDescription(gcompris.skin.get_font("gcompris/board/big bold")))
    text_color = gtk.gdk.color_parse("blue")
    text_color_selected = gtk.gdk.color_parse("green")

    self.minute.modify_text(gtk.STATE_NORMAL, text_color)
    self.minute.modify_text(gtk.STATE_SELECTED, text_color_selected)

    # allow only 2 number of digits 
    self.minute.set_max_length(2)
    self.minute.connect("activate", self.enter_callback_2)
    self.minute.connect("changed", self.enter_char_callback_2)

    self.minute.props.visibility = goocanvas.ITEM_VISIBLE

    self.widget2 = goocanvas.Widget(
      parent = self.rootitem,
      widget=self.minute,
      x=150,
      y=400,
      width=40,
      height=46,
      anchor=gtk.ANCHOR_CENTER,
      )

    self.widget2.raise_(None)

    return self.minute
    
  def hour_text(self):
    self.hour = gtk.Entry()

    self.hour.modify_font(pango.FontDescription(gcompris.skin.get_font("gcompris/board/big bold")))
    text_color = gtk.gdk.color_parse("blue")
    text_color_selected = gtk.gdk.color_parse("green")

    self.hour.modify_text(gtk.STATE_NORMAL, text_color)
    self.hour.modify_text(gtk.STATE_SELECTED, text_color_selected)

    # allow only 2 number of digits 
    self.hour.set_max_length(2)
    self.hour.connect("activate", self.enter_callback)
    self.hour.connect("changed", self.enter_char_callback)

    self.hour.props.visibility = goocanvas.ITEM_VISIBLE

    self.widget = goocanvas.Widget(
      parent = self.rootitem,
      widget=self.hour,
      x=60,
      y=400,
      width=40,
      height=46,
      anchor=gtk.ANCHOR_CENTER,
      )

    self.widget.raise_(None)

    self.hour.grab_focus()

    return self.hour
    

  def enter_char_callback(self, widget):
      text = widget.get_text()
      widget.set_text(text.decode('utf8').upper().encode('utf8'))

  def enter_callback(self, widget):
    text = widget.get_text()
    self.hrdone = 0
    
    # Check if the hour on the click matches
    if self.random_hour == text:
      self.indicator.props.text = ""
      self.hrdone = 1
    else:
      self.wrongcount=self.wrongcount+1
      if text > 12 or number <= 0:
        self.indicator.props.text = _("Out of range Hour.\nTry Again.")
      else:
	self.indicator.props.text = \
	  _("Wrong Hour.\nTry Again")
      self.hour.grab_focus()
      widget.set_text('')
      return

      
  def enter_char_callback_2(self, widget2):
      text = widget2.get_text()
      widget2.set_text(text.decode('utf8').upper().encode('utf8'))

  def enter_callback_2(self, widget2):
    text = widget2.get_text()

    # Check if the minute on the clock matches
    if self.random_minute == text and self.hrdone == 1:
      self.correctcount=self.correctcount+1
      self.indicator.props.text = ""
      gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.TUX)
      self.next_level()
    elif self.hrdone == 1:
      self.wrongcount=self.wrongcount+1
      if text > 59 or number < 0:
        self.indicator.props.text = _("Out of range Minute.\nTry Again")
      else:
	self.indicator.props.text = \
	  _("Wrong Minute.\nTry Again")
      self.minute.grab_focus()
      widget2.set_text('')
      return
      
  def ok_event(self, item, target, event, data_hr, data_min):
    if self.stopped:
      return
    self.enter_callback(data_hr)
    self.enter_callback_2(data_min)
    
  def next_level(self,event,target,item):
      print("In next_level function\n");
      self.increment_level()
      self.end()
      self.start()
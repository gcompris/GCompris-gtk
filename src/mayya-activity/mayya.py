#  gcompris - mayya.py
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
# mayya activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas
import pango

from gcompris import gcompris_gettext as _

#
# The name of the class is important. It must start with the prefix
# 'Gcompris_' and the last part 'mayya' here is the name of
# the activity and of the file in which you put this code. The name of
# the activity must be used in your menu.xml file to reference this
# class like this: type="python:mayya"
#
class Gcompris_mayya:
  """Empty gcompris Python class"""


  def __init__(self, gcomprisBoard):
    print "mayya init"

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    print "mayya start"

    # Set the buttons we want in the bar
    #gcompris.bar_set(gcompris.BAR_LEVEL)

    # Set a background image
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())
    
    self.titleText=goocanvas.Text(
      parent = self.rootitem,
      x=10.0,
      y=10.0,
      text=_("The Three Little Pigs"),
      fill_color="black",
      anchor = gtk.ANCHOR_NW,
      alignment = pango.ALIGN_CENTER
      )
    
    storyRect=goocanvas.Rect(parent = self.rootitem,
      x = 8,
      y = 48,
      height = 200,
      width = 404,
      stroke_color = "red",
      fill_color = "yellow",
      radius_x = 3, radius_y = 3,
      line_width = 1.0)

    storyText=goocanvas.Text(
      parent = self.rootitem,
      x=10.0,
      y=50.0,
      text=_("Once upon a time there were three little pigs" 
	" and the time came for them to leave home and seek their fortunes."
	" Before they left, their mother told them 'Whatever you do,"
	" do it the best that you can because that's the way to get along in the world.'......."),
      fill_color="black",
      anchor = gtk.ANCHOR_NW,
      alignment = pango.ALIGN_LEFT,
      width=400
      )
    storyText.raise_(storyRect)
    p_points = goocanvas.Points([(418.0,6), (418.0, 460.0)])
    polyline = goocanvas.Polyline(
      parent = self.rootitem,
      points=p_points, 
      stroke_color="red", 
      line_width=1)

    questionNText=goocanvas.Text(
      parent = self.rootitem,
      x=430.0,
      y=10.0,
      text=_("Question 1"),
      fill_color="black",
      anchor = gtk.ANCHOR_NW,
      alignment = pango.ALIGN_LEFT
      )

    questionText=goocanvas.Text(
      parent = self.rootitem,
      x=430.0,
      y=35.0,
      text=_("How many pigs were in the story?"),
      fill_color="black",
      anchor = gtk.ANCHOR_NW,
      alignment = pango.ALIGN_LEFT
      )

    ansText1=goocanvas.Text(
      parent = self.rootitem,
      x=560.0,
      y=80,
      text=_("2"),
      fill_color="red",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_LEFT
      )
    ansEllipse1 = goocanvas.Ellipse(parent=self.rootitem,
      center_x=560,
      center_y=80,
      radius_x=26,
      radius_y=14,
      stroke_color="red",
      fill_color="yellow",
      line_width=1.0)
    ansText1.raise_(ansEllipse1)
    ansEllipse1.connect("button_press_event",self.wrongAnswer_event, None)
    
    ansText2=goocanvas.Text(
      parent = self.rootitem,
      x=560.0,
      y=120,
      text=_("3"),
      fill_color="red",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_LEFT
      )
    ansEllipse2 = goocanvas.Ellipse(parent=self.rootitem,
      center_x=560,
      center_y=120,
      radius_x=26,
      radius_y=14,
      stroke_color="red",
      fill_color="yellow",
      line_width=1.0)
    ansText2.raise_(ansEllipse2)
    ansEllipse2.connect("button_press_event",self.rightAnswer_event, None)
    ansText3=goocanvas.Text(
      parent = self.rootitem,
      x=560.0,
      y=160,
      text=_("4"),
      fill_color="red",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_LEFT
      )
    ansEllipse3 = goocanvas.Ellipse(parent=self.rootitem,
      center_x=560,
      center_y=160,
      radius_x=26,
      radius_y=14,
      stroke_color="red",
      fill_color="yellow",
      line_width=1.0)
    ansText3.raise_(ansEllipse3)
    ansEllipse3.connect("button_press_event",self.wrongAnswer_event, None)
    self.wrongAnsTxt=goocanvas.Text(
      parent = self.rootitem,
      x=550.0,
      y=205,
      text=_("Wrong answer! Try again!"),
      fill_color="red",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )
    self.wrongAnsImg = goocanvas.Image(
        parent = self.rootitem,
        x = 540,
        y = 219,
        height = 48,
        width = 48,
        pixbuf = gcompris.utils.load_pixmap('mayya-activity/weather-rain.png')
    )

    self.rightAnsTxt=goocanvas.Text(
      parent = self.rootitem,
      x=550.0,
      y=205,
      text=_("Great job! Way to go!"),
      fill_color="red",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )
    self.rightAnsImg = goocanvas.Image(
    	parent = self.rootitem,
        x = 540,
        y = 219,
        height = 48,
        width = 48,
        pixbuf = gcompris.utils.load_pixmap('mayya-activity/weather-sun.png')
    )
    self.wrongAnsTxt.props.visibility=goocanvas.ITEM_INVISIBLE
    self.wrongAnsImg.props.visibility=goocanvas.ITEM_INVISIBLE
    self.rightAnsTxt.props.visibility=goocanvas.ITEM_INVISIBLE
    self.rightAnsImg.props.visibility=goocanvas.ITEM_INVISIBLE
	
  def wrongAnswer_event(self, widget, target, event, button):
    self.wrongAnsTxt.props.visibility=goocanvas.ITEM_VISIBLE
    self.wrongAnsImg.props.visibility=goocanvas.ITEM_VISIBLE
    self.rightAnsTxt.props.visibility=goocanvas.ITEM_INVISIBLE
    self.rightAnsImg.props.visibility=goocanvas.ITEM_INVISIBLE
    return None

  def rightAnswer_event(self, widget, target, event, button):
    self.wrongAnsTxt.props.visibility=goocanvas.ITEM_INVISIBLE
    self.wrongAnsImg.props.visibility=goocanvas.ITEM_INVISIBLE
    self.rightAnsTxt.props.visibility=goocanvas.ITEM_VISIBLE
    self.rightAnsImg.props.visibility=goocanvas.ITEM_VISIBLE
    return None


  def end(self):
    print "mayya end"
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("mayya ok.")


  def repeat(self):
    print("mayya repeat.")


  #mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    print("mayya config_start.")

  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_mayya key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    print("mayya pause. %i" % pause)


  def set_level(self, level):
    print("mayya set level. %i" % level)



#  gcompris - intro_gravity.py
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
# intro_gravity activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.anim
import goocanvas
import pango
import gcompris.bonus
import gobject
from gcompris import gcompris_gettext as _


class Gcompris_intro_gravity:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    print "intro_gravity init"

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    self.gcomprisBoard.maxlevel = 3

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    print "intro_gravity start"

    # Set the buttons we want in the bar
    gcompris.bar_set(0)


    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())
    
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),"intro_gravity/solar_system.svgz")

    #Load the solar system image
    svghandle = gcompris.utils.load_svg("intro_gravity/solar_system.svgz")
    self.selection = goocanvas.Svg(
      parent = self.rootitem,
      svg_handle = svghandle,
      svg_id = "#selected"
      )
    
    #connect the selected label to the next screen  
    self.selection.connect("button_press_event", self.set_level)
    gcompris.utils.item_focus_init(self.selection, None)
    
    self.text = goocanvas.Text(parent=self.rootitem,
      x = 400,
      y = 400,
      fill_color = "white",
      font = gcompris.skin.get_font("gcompris/title"),
      text = _("The Solar System"))

  def game(self):
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    gcompris.utils.item_focus_remove(self.selection, None)
    self.text.remove()

    goocanvas.Text(
      parent = self.rootitem,
      x=400.0,
      y=100.0,
      text=_("Mass is directly proportional to gravitational force"),
      font = gcompris.skin.get_font("gcompris/subtitle"),
      fill_color="white",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                           "intro_gravity/background.svg")

    #Load the middle planet - Uranus
    pixbuf = gcompris.utils.load_pixmap("intro_gravity/uranus.png")
    self.planet_mid = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf,
      height = 50,
      width = 50,
      x = 375,
      y = 198)

    #Load planet on the left - saturn 
    self.planet_left = fixed_planet(self.rootitem)
    self.planet_left.load_planet("saturn.png",45,160)
      
    #line joining increase and decrease button
    points = goocanvas.Points([(27,190),(27,270)])
    self.slider_left = slider(points,self.rootitem)

    #Increase button for planet_left
    incr = self.slider_left.increase_button(9)
    incr.connect('button_press_event',self.increase_planet_left)
      
    #Decrease button for planet_left
    decr = self.slider_left.decrease_button(9)
    decr.connect("button_press_event",self.decrease_planet_left)

    #sliding bar for planet_left  
    points = goocanvas.Points([(21,247),(33,247)])
    self.slider_left.sliding_bar(points)
    self.yBar_planet_left = 247  
      
    #Planet on right - neptune
    self.planet_right = fixed_planet(self.rootitem)
    self.planet_right.load_planet("neptune.png",660,165)


    #Line joining increase and decrease button
    points = goocanvas.Points([(788,190),(788,270)])
    self.slider_right = slider(points,self.rootitem)


    #Increase button for planet_right
    incr = self.slider_right.increase_button(770)
    incr.connect("button_press_event",self.increase_planet_right)
      
    #Decrease button for planet_right
    decr = self.slider_right.decrease_button(770)
    decr.connect("button_press_event",self.decrease_planet_right)
    
    #sliding bar  
    points = goocanvas.Points([(782,247),(794,247)])
    self.slider_right.sliding_bar(points)
    
    #declaring variables     
    self.timer_on = False  
    self.planet_left_mass = self.planet_right_mass = 1000  
    self.velocity = 1

  def end(self):
    print "intro_gravity end"
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("intro_gravity ok.")


  def repeat(self):
    print 'repeat'


  #mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    print("intro_gravity config_start.")

  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_intro_gravity key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if pause == 0:
      self.set_level(1,2,3)

  def set_level(self,a,b,c):
    if self.gcomprisBoard.level == 1:
      self.level = 100
    elif self.gcomprisBoard.level == 2:
      self.level = 100
    elif self.gcomprisBoard.level == 3:
      self.level = 50

    self.game()   
  
  def increase_planet_right(self,a,b,c):
    self.timer()
    self.slider_right.move_bar(782,-8,1)
    self.move_planet()
    
  def increase_planet_left(self,a,b,c):
    self.timer()    
    self.planet_left_mass = self.planet_left.set_mass(1.1,-8,-20,500,1)
    self.slider_left.move_bar(21,-8,1)
    self.move_planet()
     
  def decrease_planet_right(self,a,b,c):
    self.timer()    
    self.planet_right_mass = self.planet_right.set_mass(0.9,78,25,-500,2)
    self.slider_right.move_bar(782,8,2)
    self.move_planet()
    
  def decrease_planet_left(self,a,b,c):
    self.timer()
    self.planet_left_mass = self.planet_left.set_mass(0.9,10,25,-500,2)
    self.slider_left.move_bar(21,8,2)
    self.move_planet()
      
  def timer(self):
    if self.timer_on == False:
      self.t = gobject.timeout_add(15000,self.next_level)
      self.timer_on = True


  def next_level(self):
      gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.TUX)
      self.end()
      self.gcomprisBoard.level += 1
      self.timer_on = False    
      self.board_paused = 1

  def crash(self):
      self.end()
      self.board_paused = 1
      self.timer_on = False    
      gobject.source_remove(self.t)  
      gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.TUX)
      self.board_paused = 1

  def move_planet(self):
    x = self.planet_mid.get_bounds().x1
    position = x + self.velocity
    self.planet_mid.set_properties(x=position,y=198)
    if position < 615 and position > 175:
      gobject.timeout_add(30,self.force)
    else:
      self.crash()
      
  def force(self):    
    if self.planet_right_mass == self.planet_left_mass:
      print self.velocity
      self.move_planet()
      
    elif self.planet_right_mass > self.planet_left_mass:
      self.count +=1
      if self.count%self.level==0 or self.velocity ==0:
        self.velocity +=1
        print self.count
      self.move_planet()

    else:
      self.count +=1
      if self.count%self.level==0 or self.velocity ==0:
        self.velocity -=1  
        print self.count
      self.move_planet()
    

class fixed_planet:
  """ Fixed planets """

  def __init__(self,rootitem):
    self.rootitem = rootitem
    self.planet_mass = 1000 
     
  def load_planet(self,planet,x,y):  
    image = "intro_gravity/"+planet
    pixbuf = gcompris.utils.load_pixmap(image)
    self.planet = goocanvas.Image(
      parent = self.rootitem,
      pixbuf=pixbuf,
      height=120,
      width=120,
      x=x,
      y=y)

  def set_mass(self,scale,x,y,mass,button):
    if (self.planet_mass < 3500 and button ==1) or (self.planet_mass > 1000 and button ==2):
      self.planet.scale(scale,scale)
      self.planet.translate(x,y)
      self.planet_mass += mass 
    
    return self.planet_mass
        
class slider:
  """ class for scale slider"""

  def __init__(self,points,rootitem):
    self.rootitem = rootitem
    self.line = goocanvas.Polyline(
      parent = rootitem,
      points=points,
      stroke_color="grey",
      width=2.0)

  def increase_button(self,x):
    pixbuf = gcompris.utils.load_pixmap("/intro_gravity/plus.png")
    button = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf,
      x = x,
      y = 175
      )
    return button
    
  def decrease_button(self,x):
    pixbuf = gcompris.utils.load_pixmap("intro_gravity/minus.png")
    button = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf,
      x = x,
      y = 250
      )  
    return button
    
  def sliding_bar(self,points):
    self.bar = goocanvas.Polyline(
      parent = self.rootitem,
      points=points,
      stroke_color="grey",
      line_width=5.0)
    
  def move_bar(self,x,y,button):
    y_old = self.bar.get_bounds().y1
    if (y_old > 207 and button ==1) or (y_old < 244 and button ==2):
      y_new = int(y_old + y)
      gcompris.utils.item_absolute_move(self.bar,x,y_new)

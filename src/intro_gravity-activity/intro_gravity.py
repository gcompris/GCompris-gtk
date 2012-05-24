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
import gcompris.timer
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
    self.selection.connect("button_press_event", self.game)
    gcompris.utils.item_focus_init(self.selection, None)
    
    self.text = goocanvas.Text(parent=self.rootitem,
      x = 400,
      y = 400,
      fill_color = "white",
      font = gcompris.skin.get_font("gcompris/title"),
      text = _("The Solar System"))

  def game(self,a,b,c):
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

    #Load the middle planet
    pixbuf = gcompris.utils.load_pixmap("intro_gravity/uranus.png")
    self.mid_planet = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf,
      height = 50,
      width = 50,
      x = 375,
      y = 200)

    #Load planet on left  
    pixbuf = gcompris.utils.load_pixmap("intro_gravity/saturn.png")
    self.saturn = goocanvas.Image(
      parent = self.rootitem,
      pixbuf=pixbuf,
      height=130,
      width=130,
      x=45,
      y=160)
      
    #line joining increase and decrease button
    points = goocanvas.Points([(27,190),(27,270)])
    self.line = goocanvas.Polyline(
      parent = self.rootitem,
      points=points,
      stroke_color="grey",
      width=2.0)


    #Increase button for saturn
    pixbuf_plus = gcompris.utils.load_pixmap("/intro_gravity/plus.png")
    plus_saturn = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf_plus,
      x = 9,
      y = 175
      )
    plus_saturn.connect("button_press_event",self.increase_saturn)
      
    #Decrease button for saturn
    pixbuf_minus = gcompris.utils.load_pixmap("intro_gravity/minus.png")
    minus_saturn = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf_minus,
      x = 9,
      y = 250
      )  
    minus_saturn.connect("button_press_event",self.decrease_saturn)

    #sliding bar for saturn  
    points = goocanvas.Points([(21,247),(33,247)])
    self.bar_saturn = goocanvas.Polyline(
      parent = self.rootitem,
      points=points,
      stroke_color="grey",
      line_width=5.0)
    self.yBar_saturn = 247  
      
    #Planet on right
    pixbuf = gcompris.utils.load_pixmap("intro_gravity/neptune.png")
    self.neptune = goocanvas.Image(
      parent = self.rootitem,
      pixbuf=pixbuf,
      height=50,
      width=50,
      x=690,
      y=200)

    #Line joining increase and decrease button
    points = goocanvas.Points([(788,190),(788,270)])
    self.line = goocanvas.Polyline(
      parent = self.rootitem,
      points=points,
      stroke_color="grey",
      width=2.0)

    #Increase button for neptune
    pixbuf_plus = gcompris.utils.load_pixmap("/intro_gravity/plus.png")
    plus_neptune = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf_plus,
      x = 770,
      y = 175
      )

    plus_neptune.connect("button_press_event",self.increase_neptune)
      
    #Decrease button for neptune
    pixbuf_minus = gcompris.utils.load_pixmap("intro_gravity/minus.png")
    minus_neptune = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf_minus,
      x = 770,
      y = 250
      )  
    
    minus_neptune.connect("button_press_event",self.decrease_neptune)
    
    #sliding bar  
    points = goocanvas.Points([(782,247),(794,247)])
    self.bar_neptune = goocanvas.Polyline(
      parent = self.rootitem,
      points=points,
      stroke_color="grey",
      line_width=5.0)
    self.yBar_neptune = 247  
    
    #declaring variables     
    self.timer_on = False  
    self.force_neptune = self.force_saturn = 10
    self.mass_neptune = self.mass_saturn = 10000 
#    self.distance_neptune = self.distance_saturn = 1000
    self.velocity = 0.05
    self.direction = None
    self.crash = False
      
  def end(self):
    print "intro_gravity end"
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("intro_gravity ok.")


  def repeat(self):
    print 'repeat'
    gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.SMILEY)
    self.game(1,2,3)
    self.neptune.set_transform(None)


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
      self.game(1,2,3)

  def set_level(self, level):
    print("intro_gravity set level. %i" % level)
  
  
  def increase_neptune(self,a,b,c):
#    q,w = goocanvas.Canvas.convert_to_item_space(self.neptune,630,170)
#    print q,w 
#    x = self.neptune.get_bounds().x1
#    print x
    self.timer()
    #increase planet if not maximum and move bar
    if self.yBar_neptune > 207:
      self.neptune.scale(1.1,1.1)
      self.neptune.translate(-63,-20)
      self.yBar_neptune -=8
      gcompris.utils.item_absolute_move(self.bar_neptune, 782,self.yBar_neptune)
      if self.direction == None:
        self.direction = 2
        self.move_mid_planet()
      else:
        self.move_mid_planet()
      self.mass_neptune += 5000
    
  def increase_saturn(self,a,b,c):
    self.timer()    
    #increase planet if not maximum and move bar
    if self.yBar_saturn > 207: 
      self.saturn.scale(1.1,1.1)
      self.saturn.translate(-8,-20)  	
      self.yBar_saturn -=8
      gcompris.utils.item_absolute_move(self.bar_saturn,21,self.yBar_saturn)
      if self.direction == None:
        self.direction = 1
        self.move_mid_planet()
      else:
        self.move_mid_planet()
      self.mass_saturn += 5000
      
  def decrease_neptune(self,a,b,c):
    self.timer()    
    #decrease planet size if not minimum and move bar  
    if self.yBar_neptune < 247:
      self.neptune.scale(0.9,0.9) 
      self.neptune.translate(80,23)
      self.yBar_neptune +=8
      gcompris.utils.item_absolute_move(self.bar_neptune, 782,self.yBar_neptune)
      if self.direction == None:
        self.direction = 1
        self.move_mid_planet()
      else:
        self.move_mid_planet()
      self.mass_neptune -= 5000

  def decrease_saturn(self,a,b,c):
    self.timer()
    #decrease planet size if not minimum and move bar 
    if self.yBar_saturn < 247:
      self.saturn.scale(0.9,0.9)
      self.saturn.translate(10,25)
      self.yBar_saturn += 8
      gcompris.utils.item_absolute_move(self.bar_saturn,21,self.yBar_saturn)
      if self.direction == None:
        self.direction = 2
        self.move_mid_planet()
      else:
        self.move_mid_planet()
      self.mass_saturn -= 5000
      
  def timer(self):
    #start timer if not initiated
    #if planets don't crash during this time then level is won
    if self.timer_on == False:
      self.t = gobject.timeout_add(30000,self.stopGame)
      self.timer_on = True


  def stopGame(self):
    if self.crash == False:
      gcompris.bonus.display(gcompris.bonus.WIN,gcompris.bonus.TUX)
      self.end()
      self.gcomprisBoard.level += 1
      self.timer_on = False    
      self.board_paused = 1
    else:
      gcompris.bonus.display(gcompris.bonus.LOOSE,gcompris.bonus.TUX)
      self.end()
      self.board_paused = 1
      self.crash = False
      self.timer_on = False    
      gobject.source_remove(self.t)  
   

  def move_mid_planet(self):
    if self.direction == 1:
      x = self.mid_planet.get_bounds().x1
      self.position = int(x - self.velocity)
#      gcompris.utils.item_absolute_move(self.mid_planet,self.position,200)
      self.mid_planet.set_properties(x=self.position,y=200)
#      self.distance_neptune += 1
#      self.distance_saturn -= 1
      if self.position > 200:
        gobject.timeout_add(30,self.force)
      else:
        self.crash = True
        self.stopGame()

    elif self.direction ==2:
      x = self.mid_planet.get_bounds().x1
      self.position = int(x + self.velocity)
#      gcompris.utils.item_absolute_move(self.mid_planet,self.position,200)
      self.mid_planet.set_properties(x=self.position,y=200)
#      self.distance_neptune -= 1
#      self.distance_saturn += 1
      if self.position < 615:
        gobject.timeout_add(30,self.force)
      else:
        self.crash = True
        self.stopGame()
      
      
  def force(self):
#    print 'before cal neptune',self.force_neptune
#    print 'before cal saturn',self.force_saturn

#    print 'neptune distance',self.distance_neptune
#    print 'saturn distance',self.distance_saturn

#    print 'mass of neptune',self.mass_neptune
#    print 'mass of saturn',self.mass_saturn

#    self.force_neptune = self.mass_neptune/self.distance_neptune 
#    self.force_saturn = self.mass_saturn/self.distance_saturn
#    print 'neptune force',self.force_neptune
#    print 'saturn force',self.force_saturn
    if self.mass_neptune == self.mass_saturn:
      self.velocity = 0.10
      print 'equal',self.direction
      self.move_mid_planet() 
    elif self.mass_neptune > self.mass_saturn:
      self.velocity += 0.01
      self.direction = 2
      self.move_mid_planet()
      print 'neptune greater',self.direction
    else:
      self.velocity += 0.01
      self.move_mid_planet()
      self.direction = 1
      print 'saturn greater',self.direction
#      print self.velocity
      
    


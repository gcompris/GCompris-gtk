#  gcompris - melody
# 
# Time-stamp: <2003/12/09 20:54:45 jose>
# 
# Copyright (C) 2003 Jose Jorge
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
import gcompris.sound
import gcompris.bonus
import gtk
import gtk.gdk
import random

class Gcompris_melody:
  """The melody activity"""
  

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    print("Gcompris_melody __init__.")

    # These are used to let us restart only after the bonux is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;


  def start(self):  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1 
    self.gcomprisBoard.number_of_sublevel=1
    self.timers = []
    self.solution = []
    self.kidstry = []
    self.in_repeat = 0;

    # Play an intro sound
    gcompris.sound.play_ogg("melody/melody")
    
    #
    # This list contains the 'theme' for each melody level.
    #
    self.melodylist = \
                      [
      # Kitchen
      [
      {'background': "melody/background.jpg", 'hittool': "melody/cursor.png", 'hitofset_x': 50, 'hitofset_y': 50},
      [ {'x': 150.0, 'y': 50.0,  'image': "melody/son1.png", 'sound': "melody/son1"},
        {'x': 550.0, 'y': 50.0,  'image': "melody/son2.png", 'sound': "melody/son2"},
        {'x': 150.0, 'y': 250.0, 'image': "melody/son3.png", 'sound': "melody/son3"},
        {'x': 550.0, 'y': 250.0, 'image': "melody/son4.png", 'sound': "melody/son4"} ] ],
      
      # Kitchen2
      [
      {'background': "melody/background.jpg", 'hittool': "melody/cursor.png", 'hitofset_x': 50, 'hitofset_y': 50},
      [ {'x': 150.0, 'y': 50.0,  'image': "melody/son1.png", 'sound': "melody/son1"},
        {'x': 550.0, 'y': 50.0,  'image': "melody/son2.png", 'sound': "melody/son2"},
        {'x': 150.0, 'y': 250.0, 'image': "melody/son3.png", 'sound': "melody/son3"},
        {'x': 550.0, 'y': 250.0, 'image': "melody/son4.png", 'sound': "melody/son4"} ] ],

      # Kitchen3
      [
      {'background': "melody/background.jpg", 'hittool': "melody/cursor.png", 'hitofset_x': 50, 'hitofset_y': 50},
      [ {'x': 150.0, 'y': 50.0,  'image': "melody/son1.png", 'sound': "melody/son1"},
        {'x': 550.0, 'y': 50.0,  'image': "melody/son2.png", 'sound': "melody/son2"},
        {'x': 150.0, 'y': 250.0, 'image': "melody/son3.png", 'sound': "melody/son3"},
        {'x': 550.0, 'y': 250.0, 'image': "melody/son4.png", 'sound': "melody/son4"} ] ],
      
      # Kitchen4
      [
      {'background': "melody/background.jpg", 'hittool': "melody/cursor.png", 'hitofset_x': 50, 'hitofset_y': 50},
      [ {'x': 150.0, 'y': 50.0,  'image': "melody/son1.png", 'sound': "melody/son1"},
        {'x': 550.0, 'y': 50.0,  'image': "melody/son2.png", 'sound': "melody/son2"},
        {'x': 150.0, 'y': 250.0, 'image': "melody/son3.png", 'sound': "melody/son3"},
        {'x': 550.0, 'y': 250.0, 'image': "melody/son4.png", 'sound': "melody/son4"} ] ],
      
      # Kitchen5
      [
      {'background': "melody/background.jpg", 'hittool': "melody/cursor.png", 'hitofset_x': 50, 'hitofset_y': 50},
      [ {'x': 150.0, 'y': 50.0,  'image': "melody/son1.png", 'sound': "melody/son1"},
        {'x': 550.0, 'y': 50.0,  'image': "melody/son2.png", 'sound': "melody/son2"},
        {'x': 150.0, 'y': 250.0, 'image': "melody/son3.png", 'sound': "melody/son3"},
        {'x': 550.0, 'y': 250.0, 'image': "melody/son4.png", 'sound': "melody/son4"} ] ],
      
      # Kitchen6
      [
      {'background': "melody/background.jpg", 'hittool': "melody/cursor.png", 'hitofset_x': 50, 'hitofset_y': 50},
      [ {'x': 150.0, 'y': 50.0,  'image': "melody/son1.png", 'sound': "melody/son1"},
        {'x': 550.0, 'y': 50.0,  'image': "melody/son2.png", 'sound': "melody/son2"},
        {'x': 150.0, 'y': 250.0, 'image': "melody/son3.png", 'sound': "melody/son3"},
        {'x': 550.0, 'y': 250.0, 'image': "melody/son4.png", 'sound': "melody/son4"} ] ],
      
      # Kitchen7
      [
      {'background': "melody/background.jpg", 'hittool': "melody/cursor.png", 'hitofset_x': 50, 'hitofset_y': 50},
      [ {'x': 150.0, 'y': 50.0,  'image': "melody/son1.png", 'sound': "melody/son1"},
        {'x': 550.0, 'y': 50.0,  'image': "melody/son2.png", 'sound': "melody/son2"},
        {'x': 150.0, 'y': 250.0, 'image': "melody/son3.png", 'sound': "melody/son3"},
        {'x': 550.0, 'y': 250.0, 'image': "melody/son4.png", 'sound': "melody/son4"} ] ],
      
      # Kitchen8
      [
      {'background': "melody/background.jpg", 'hittool': "melody/cursor.png", 'hitofset_x': 50, 'hitofset_y': 50},
      [ {'x': 150.0, 'y': 50.0,  'image': "melody/son1.png", 'sound': "melody/son1"},
        {'x': 550.0, 'y': 50.0,  'image': "melody/son2.png", 'sound': "melody/son2"},
        {'x': 150.0, 'y': 250.0, 'image': "melody/son3.png", 'sound': "melody/son3"},
        {'x': 550.0, 'y': 250.0, 'image': "melody/son4.png", 'sound': "melody/son4"} ] ],
      
      ]

    self.gcomprisBoard.maxlevel = len(self.melodylist)

    gcompris.bar_set(gcompris.BAR_REPEAT|gcompris.BAR_LEVEL)

    self.display_current_level()
    self.pause(0);
    print("Gcompris_melody start.")
    
  def end(self):
    self.cleanup()
    print("Gcompris_melody end.")
        

  def ok(self):
    print("Gcompris_melody ok.")


  def cleanup(self):

    # Clear all timer
    for i in self.timers :
      gtk.timeout_remove(i)

    self.timers = []

    self.in_repeat = 0;
    
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()
    self.rootitem = None
    
  def display_current_level(self):

    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            self.melodylist[self.gcomprisBoard.level-1][0]['background'])
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.sound_list = self.melodylist[self.gcomprisBoard.level-1][1]
    
    for i in self.sound_list:
      self.sound_item = self.rootitem.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = gcompris.utils.load_pixmap(i['image']),
        x=i['x'],
        y=i['y']
        )
      self.sound_item.connect("event", self.sound_item_event, i)
      # This item is clickeable and it must be seen
      self.sound_item.connect("event", gcompris.utils.item_event_focus)

    
    self.bang_item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap(self.melodylist[self.gcomprisBoard.level-1][0]['hittool']),
      x=0,
      y=0
      )
    self.bang_item.hide()
    
    self.hitofset_x = self.melodylist[self.gcomprisBoard.level-1][0]['hitofset_x']
    self.hitofset_y = self.melodylist[self.gcomprisBoard.level-1][0]['hitofset_y']

    self.populate(self.sound_list)


  # records the try of the child
  def tried(self, a):
    if len(self.kidstry) >= len(self.solution) :
      self.kidstry.pop(0)
    self.kidstry.append(a)
    #level finished?
    if self.kidstry == self.solution :
      if (self.increment_level() == 1):
        self.gamewon = 1
        gcompris.bonus.display(1, gcompris.bonus.FLOWER)

  
  # Ready to play for the kid
  def ready(self):

    self.in_repeat = 0
    self.timers.pop(0)

    
  # Shows and plays the thing clicked
  def show_bang_stop(self, a):

    if self.board_paused or self.rootitem == None:
      return

    self.bang_item.hide()
    self.timers.pop(0)
    
  # Shows and plays the thing clicked
  def show_bang(self, a):

    if self.board_paused or self.rootitem == None:
      return
    
    self.bang_item.set(x=a['x'] + self.hitofset_x, y=a['y'] + self.hitofset_y)

    self.bang_item.show()
    
    gcompris.sound.play_ogg(a['sound'])
    self.timers.pop(0)


  def repeat(self):
    print("Gcompris_melody repeat.")
    # Important to use a timer here to keep self.timers up todate
    self.timers.append(gtk.timeout_add(50, self.repeat_it))
    
  def repeat_it(self):
    print("Gcompris_melody repeat it.")
    if self.in_repeat:
      return
    
    # We are like paused until the last sound is played
    self.in_repeat = 1;

    if self.timers :
      self.timers.pop(0)

    self.kidstry = []
    timer = 0

    for i in self.solution:
      self.timers.append(gtk.timeout_add(timer, self.show_bang, i))
      timer = timer + 1000
      self.timers.append(gtk.timeout_add(timer, self.show_bang_stop, i))
      timer = timer + 500
      
    self.timers.append(gtk.timeout_add(timer, self.ready))

    
  def config(self):
    print("Gcompris_melody config.")
  
  #randomize the sequence and plays it one first time
  def populate(self, sound_struct):
    print("Gcompris_melody populate.")
    self.solution = []

    for i in range(self.gcomprisBoard.level+2):
      self.solution.append(sound_struct[random.randint(0,len(sound_struct)-1)])
      
    self.timers.append(gtk.timeout_add(1300, self.repeat_it))
              
  def key_press(self, keyval):
    print("got key %i" % keyval)
    return


  def pause(self, pause):  
    self.board_paused = pause
    
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(self.gamewon == 1 and pause == 0):
      self.cleanup()
      self.display_current_level()
      self.gamewon = 0

    return
                  

  def set_level(self, level):  
    print("Gcompris_melody set level. %i" % level)
    self.gcomprisBoard.level=level;
    self.gcomprisBoard.sublevel=1;
    self.cleanup()
    self.display_current_level()
  
  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return 1 if continue, 0 if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      if(self.gcomprisBoard.level>self.gcomprisBoard.maxlevel):
        # the current board is finished : bail out
        gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
        return 0
      
    return 1

            
  # ---------------- sound on click events -----------------------
  def sound_item_event(self, widget, event, sound_struct):

    if self.board_paused or self.in_repeat:
      return
    
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
          gcompris.sound.play_ogg(sound_struct['sound'])
          self.tried(sound_struct)
    return gtk.FALSE
  

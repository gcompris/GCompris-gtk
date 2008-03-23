#  gcompris - melody
#
# Copyright (C) 2003, 2008 Jose Jorge
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
import gobject
import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.sound
import gcompris.bonus
import gtk
import gtk.gdk
import random

from gcompris import gcompris_gettext as _

class Gcompris_melody:
  """The melody activity"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.disable_im_context = True

    #print("Gcompris_melody __init__.")

    # These are used to let us restart only after the bonux is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;


  def start(self):
    self.saved_policy = gcompris.sound.policy_get()
    gcompris.sound.policy_set(gcompris.sound.PLAY_AND_INTERRUPT)

    self.gcomprisBoard.level=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    # pause the bg music
    #print "pause sound"
    gcompris.sound.pause()

    self.timers = []
    self.solution = []
    self.kidstry = []
    self.in_repeat = 0
    self.theme = 0

    #
    # This list contains the 'theme' for each melody level.
    #
    self.melodylist = \
                      [
      # xylophon
      [
      {'theme': "xylofon", 'background': "melody/xylofon_background.png", 'hittool': "melody/xylofon_cursor.png", 'hitofset_x': 50, 'hitofset_y': 50},
      [ {'x': 150.0, 'y': 101.0,  'image': "melody/xylofon_son1.png", 'sound': "melody/xylofon_son1.ogg"},
        {'x': 284.0, 'y': 118.0,  'image': "melody/xylofon_son2.png", 'sound': "melody/xylofon_son2.ogg"},
        {'x': 412.0, 'y': 140.0, 'image': "melody/xylofon_son3.png", 'sound': "melody/xylofon_son3.ogg"},
        {'x': 546.0, 'y': 157.0, 'image': "melody/xylofon_son4.png", 'sound': "melody/xylofon_son4.ogg"} ] ],

      # guitar
      [
      {'theme': "guitar", 'background': "melody/guitar_background.jpg", 'hittool': "melody/guitar_cursor.png", 'hitofset_x': 400, 'hitofset_y': -5},
      [ {'x': 0, 'y': 170.0,  'image': "melody/guitar_son1.png", 'sound': "melody/guitar_son1.ogg"},
        {'x': 0, 'y': 230.0,  'image': "melody/guitar_son2.png", 'sound': "melody/guitar_son2.ogg"},
        {'x': 0, 'y': 290.0, 'image': "melody/guitar_son3.png", 'sound': "melody/guitar_son3.ogg"},
        {'x': 0, 'y': 350.0, 'image': "melody/guitar_son4.png", 'sound': "melody/guitar_son4.ogg"} ] ],

      # Kitchen
      [
      {'theme': "tachos", 'background': "melody/tachos_background.jpg", 'hittool': "melody/tachos_cursor.png", 'hitofset_x': 50, 'hitofset_y': 50},
      [ {'x': 150.0, 'y': 50.0,  'image': "melody/tachos_son1.png", 'sound': "melody/tachos_son1.ogg"},
        {'x': 550.0, 'y': 50.0,  'image': "melody/tachos_son2.png", 'sound': "melody/tachos_son2.ogg"},
        {'x': 150.0, 'y': 250.0, 'image': "melody/tachos_son3.png", 'sound': "melody/tachos_son3.ogg"},
        {'x': 550.0, 'y': 250.0, 'image': "melody/tachos_son4.png", 'sound': "melody/tachos_son4.ogg"} ] ] ]

    self.maxtheme = len(self.melodylist)-1
    self.gcomprisBoard.maxlevel = 9

    #
    pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_reload.png"))
    if(pixmap):
      gcompris.bar_set_repeat_icon(pixmap)
      gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT_ICON)
    else:
      gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT)


    self.pause(1);
    self.display_current_level()

    # Play an intro sound
    gcompris.sound.play_ogg_cb("melody/" + self.melodylist[self.theme][0]['theme'] +
                               "_melody.ogg", self.intro_cb)

    Prop = gcompris.get_properties()

    if(not Prop.fx):
      gcompris.utils.dialog(_("Error: this activity cannot be played with the\nsound effects disabled.\nGo to the configuration dialogue to\nenable the sound"), stop_board)

  def end(self):
    self.cleanup()
    #print("Gcompris_melody end.")
    gcompris.sound.policy_set(self.saved_policy)
    gcompris.sound.resume()


  def ok(self):
    #print("Gcompris_melody ok.")
    pass

  def cleanup(self):

    # Clear all timer
    for i in self.timers :
      gobject.source_remove(i)

    self.timers = []

    self.in_repeat = 0;

    # Remove the root item removes all the others inside it
    if(self.rootitem):
      self.rootitem.remove()
    self.rootitem = None

  def display_current_level(self):

    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            self.melodylist[self.theme][0]['background'])
    gcompris.bar_set_level(self.gcomprisBoard)

    gcompris.sound.policy_set(gcompris.sound.PLAY_AND_INTERRUPT)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

    # Put the theme switcher button
    self.switch_item =goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("melody/switch.png"),
      x=10,
      y=10
      )
    self.switch_item.connect("button_press_event", self.switch_item_event)
    # This item is clickeable and it must be seen
    gcompris.utils.item_focus_init(self.switch_item, None)


    # Put the sound buttons
    self.sound_list = self.melodylist[self.theme][1]

    for i in self.sound_list:
      self.sound_item =goocanvas.Image(
        parent = self.rootitem,
        pixbuf = gcompris.utils.load_pixmap(i['image']),
        x=i['x'],
        y=i['y']
        )
      self.sound_item.connect("button_press_event", self.sound_item_event, i)
      # This item is clickeable and it must be seen
      gcompris.utils.item_focus_init(self.sound_item, None)


    self.bang_item = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap(self.melodylist[self.theme][0]['hittool']),
      )
    self.bang_item.props.visibility = goocanvas.ITEM_INVISIBLE

    self.hitofset_x = self.melodylist[self.theme][0]['hitofset_x']
    self.hitofset_y = self.melodylist[self.theme][0]['hitofset_y']

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
        gcompris.sound.policy_set(self.saved_policy)
        gcompris.bonus.display(1, gcompris.bonus.FLOWER)


  # Ready to play for the kid
  def ready(self):

    self.in_repeat = 0
    self.timers.pop(0)


  # Shows and plays the thing clicked
  def show_bang_stop(self, a):

    if self.board_paused or self.rootitem == None:
      return

    self.bang_item.props.visibility = goocanvas.ITEM_INVISIBLE
    self.timers.pop(0)

  # Shows and plays the thing clicked
  def show_bang(self, a):

    if self.board_paused or self.rootitem == None:
      return

    self.bang_item.props.transform = None
    self.bang_item.translate(a['x'] + self.hitofset_x,
                             a['y'] + self.hitofset_y)
    self.bang_item.props.visibility = goocanvas.ITEM_VISIBLE

    gcompris.sound.play_ogg_cb(a['sound'], self.sound_played)
    self.timers.pop(0)


  def repeat(self):
    # Important to use a timer here to keep self.timers up todate
    self.timers.append(gobject.timeout_add(50, self.repeat_it))

  def repeat_it(self):
    if self.in_repeat:
      return

    # We are like paused until the last sound is played
    self.in_repeat = 1;

    if self.timers :
      self.timers.pop(0)

    self.kidstry = []
    timer = 0

    for i in self.solution:
      self.timers.append(gobject.timeout_add(timer, self.show_bang, i))
      timer = timer + 1000
      self.timers.append(gobject.timeout_add(timer, self.show_bang_stop, i))
      timer = timer + 500

    self.timers.append(gobject.timeout_add(timer, self.ready))


  def config(self):
    pass

  #randomize the sequence and plays it one first time
  def populate(self, sound_struct):
    self.solution = []

    for i in range(self.gcomprisBoard.level+2):
      self.solution.append(sound_struct[random.randint(0,len(sound_struct)-1)])

    self.timers.append(gobject.timeout_add(1300, self.repeat_it))

  def key_press(self, keyval, commit_str, preedit_str):
    # Play sounds with the keys
    if ((keyval == gtk.keysyms.KP_1) or (keyval == gtk.keysyms._1)):
      #print "son1"
      self.sound_play(self.melodylist[self.theme][1][0])
      return True
    if ((keyval == gtk.keysyms.KP_2) or (keyval == gtk.keysyms._2)):
      #print "son2"
      self.sound_play(self.melodylist[self.theme][1][1])
      return True
    if ((keyval == gtk.keysyms.KP_3) or (keyval == gtk.keysyms._3)):
      #print "son3"
      self.sound_play(self.melodylist[self.theme][1][2])
      return True
    if ((keyval == gtk.keysyms.KP_4) or (keyval == gtk.keysyms._4)):
      #print "son4"
      self.sound_play(self.melodylist[self.theme][1][3])
      return True

    return False


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
  def sound_item_event(self, widget, target, event, sound_struct):

    if self.board_paused or self.in_repeat:
      return

    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
          self.sound_play(sound_struct)
    return False

  # ---------------- sound is effectively played -----------------------
  def sound_play(self, sound_struct):
    gcompris.sound.play_ogg_cb(sound_struct['sound'], self.sound_played)
    self.tried(sound_struct)
    return

  # ---------------- theme change on switch events -----------------------
  def switch_item_event(self, widget, target, event):

    if self.board_paused or self.in_repeat:
      return
    # switch the theme
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
      	if self.theme < self.maxtheme:
	  self.theme += 1
	else:
	  self.theme = 0

	#print("New melody theme : " + self.melodylist[self.theme][0]['theme'] + ".")
    # Apply the changes
      	self.cleanup()
      	self.display_current_level()


    return False

  def sound_played(self, file):
    #print "python sound played :", file
    pass

  def intro_cb(self, file):
    #print "intro passed. go play"
    self.pause(0)
    self.populate(self.sound_list)

def stop_board():
  gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)


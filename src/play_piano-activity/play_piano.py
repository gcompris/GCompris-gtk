#  gcompris - play_piano.py
#
# Copyright (C) 2012 Beth Hadley
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
# play_piano activity.

import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas
import pango
import time
from gcompris import gcompris_gettext as _
from gcomprismusic import *
from random import randint
import random


class Gcompris_play_piano:

    def __init__(self, gcomprisBoard):
        # Save the gcomprisBoard, it defines everything we need
        # to know from the core
        self.gcomprisBoard = gcomprisBoard

        # Needed to get key_press
        gcomprisBoard.disable_im_context = True

        self.gcomprisBoard.level = 1
        self.gcomprisBoard.maxlevel = 12

        self.metronomePlaying = False

        self.timers = []
        self.afterBonus = None

    def start(self):
        self.recordedHits = []
        self.saved_policy = gcompris.sound.policy_get()
        gcompris.sound.policy_set(gcompris.sound.PLAY_AND_INTERRUPT)
        gcompris.sound.pause()
        gcompris.sound.play_ogg('//boards/sounds/silence1s.ogg')

        # Set the buttons we want in the bar
        gcompris.bar_set(gcompris.BAR_LEVEL)

        # Set a background image
        gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

        # Create our rootitem. We put each canvas item in it so at the end we
        # only have to kill it. The canvas deletes all the items it contains
        # automaticaly.
        self.rootitem = goocanvas.Group(parent=
                                       self.gcomprisBoard.canvas.get_root_item())


        self.display_level(self.gcomprisBoard.level)

    def display_level(self, level):
        self.score = 0
        if hasattr(self, 'staff'):
            self.staff.clear()
            self.staff.eraseAllNotes()
        drawBasicPlayHomePagePart1(self)

        gcompris.bar_set(gcompris.BAR_LEVEL)
        gcompris.bar_set_level(self.gcomprisBoard)
        gcompris.bar_location(20, -1, 0.6)

        if level <= 6:
            self.staff = TrebleStaff(100, 80, self.rootitem, numStaves=1)
            self.staff.noteSpacingX = 32
        else:
            self.staff = BassStaff(100, 90, self.rootitem, numStaves=1)
            self.staff.noteSpacingX = 28

        self.staff.endx = 200

        if level not in [6, 12]:
            self.colorCodeNotesButton = textButton(100, 215, _("Color code notes?"),
                                                   self.rootitem, 'green')

            self.colorCodeNotesButton.connect("button_press_event", self.color_code_notes)
            gcompris.utils.item_focus_init(self.colorCodeNotesButton, None)
        else:
            self.staff.colorCodeNotes = False

        self.staff.drawStaff()
        self.staff.rootitem.scale(2.0, 2.0)

        self.givenOption = []
        self.show_melody()
        self.kidsNoteList = []
        self.piano = PianoKeyboard(250, 300, self.rootitem)
        if level in [4, 5, 6, 12, 11, 10]:
            self.piano.blackKeys = True

        self.piano.draw(300, 175, self.keyboard_click)

        textBox(_("Click the piano keys that match the written notes."),
                388, 60, self.rootitem, fill_color_rgba = 0x666666AAL, width=200)

        drawBasicPlayHomePagePart2(self)

    def keyboard_click(self, widget=None, target=None, event=None, numID=None):

        if not numID:
            numID = target.numID
        if self.gcomprisBoard.level <= 6:
            n = QuarterNote(numID, 'trebleClef', self.staff.rootitem)
        else:
            n = QuarterNote(numID, 'bassClef', self.staff.rootitem)
        n.play()
        self.kidsNoteList.append(numID)

    def generateMelody(self):
        level = self.gcomprisBoard.level
        options = [1, 2, 3]
        notenum = 3
        if level in [2, 3, 4, 5, 6, 8, 9, 10, 11, 12]:
            options.extend([4, 5, 6])
            notenum = 3
        if level in [3, 4, 5, 6, 9, 10, 11, 12]:
            options.extend([7, 8])
            notenum = 4
        if level in [4, 5, 6, 10, 11, 12]:
            options.extend([-1, -2])
        if level in [5, 6, 11, 12]:
            options.extend([-3, -4, -5])

        newmelody = []
        for ind in range(0, notenum):
            newmelody.append(options[randint(0, len(options) - 1)])
        if newmelody == self.givenOption:

            return self.generateMelody()
        else:
            return newmelody

    def show_melody(self):
        self.givenOption = self.generateMelody()

        for item in self.givenOption:
            if self.gcomprisBoard.level <= 6:
                note = QuarterNote(item, 'trebleClef', self.staff.rootitem)
            else:
                note = QuarterNote(item, 'bassClef', self.staff.rootitem)
            self.staff.drawNote(note)

        self.timers.append(gobject.timeout_add(500, self.staff.playComposition))

    def ok_event(self, widget=None, target=None, event=None):
        if self.kidsNoteList == self.givenOption:
            self.afterBonus = self.nextChallenge
            gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.NOTE)
            self.score += 1
        else:
            self.afterBonus = self.tryagain
            gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.NOTE)
            self.score -= 1

    def tryagain(self):
        self.kidsNoteList = []
        self.staff.playComposition

    def nextChallenge(self):
        self.kidsNoteList = []
        self.staff.eraseAllNotes()
        l = self.gcomprisBoard.level
        if self.score >= 5:
            self.set_level(l + 1)
            return
        self.show_melody()

    def erase_entry(self, widget, target, event):
        self.kidsNoteList = []

    def end(self):

        self.staff.eraseAllNotes()
        # Remove the root item removes all the others inside it
        self.rootitem.remove()


    def ok(self):
        pass

    def repeat(self):
        pass

    #mandatory but unused yet
    def config_stop(self):
        pass

    # Configuration function.
    def config_start(self, profile):
        pass

    def key_press(self, keyval, commit_str, preedit_str):

        utf8char = gtk.gdk.keyval_to_unicode(keyval)

        if keyval == gtk.keysyms.BackSpace:

            self.erase_entry()
        elif keyval == gtk.keysyms.Delete:
            self.erase_entry()
        elif keyval == gtk.keysyms.Return:
            self.ok_event()
        elif keyval == gtk.keysyms.space:
            self.staff.playComposition()
        else:
            pianokeyBindings(keyval, self)
        return True

    def pause(self, pause):
        if not pause and self.afterBonus:
            self.afterBonus()
            self.afterBonus = None

    def set_level(self, level):
        '''
        updates the level for the game when child clicks on bottom
        left navigation bar to increment level
        '''
        self.gcomprisBoard.level = level
        gcompris.bar_set_level(self.gcomprisBoard)
        self.display_level(self.gcomprisBoard.level)


    def color_code_notes(self, widget, target, event):
        if self.staff.colorCodeNotes:
            self.staff.colorCodeNotes = False
            self.staff.colorAllNotes('black')
        else:
            self.staff.colorCodeNotes = True
            self.staff.colorCodeAllNotes()

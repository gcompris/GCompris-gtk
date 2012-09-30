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

        if not (gcompris.get_properties().fx):
            gcompris.utils.dialog(_("Error: This activity cannot be \
played with the\nsound effects disabled.\nGo to the configuration \
dialogue to\nenable the sound."), None)

    def display_level(self, level):
        self.score = 0
        if hasattr(self, 'staff'):
            self.staff.clear()
            self.staff.eraseAllNotes()

        if self.rootitem:
            self.rootitem.remove()

        self.rootitem = goocanvas.Group(parent=
                                       self.gcomprisBoard.canvas.get_root_item())

        # set background
        goocanvas.Image(
            parent=self.rootitem,
            x=0, y=0,
            pixbuf=gcompris.utils.load_pixmap('piano_composition/playActivities/background/' + str(randint(1, 6)) + '.jpg')
            )

        if hasattr(self, 'staff'):
            self.staff.clear()

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
                                                   self.rootitem, 0x990011FFL)

            self.colorCodeNotesButton.connect("button_press_event", self.color_code_notes)
            gcompris.utils.item_focus_init(self.colorCodeNotesButton, None)
        else:
            self.staff.colorCodeNotes = False

        self.staff.drawStaff()
        self.staff.rootitem.scale(2.0, 2.0)

        self.givenOption = []
        self.show_melody()
        self.kidsNoteList = []
        self.piano = PianoKeyboard(250, 305, self.rootitem)
        if level in [4, 5, 6, 12, 11, 10]:
            self.piano.blackKeys = True

        self.piano.draw(300, 175, self.keyboard_click)

        textBox(_("Click the piano keys that match the written notes."),
                388, 60, self.rootitem, fill_color_rgba = 0x999999AAL, width=200)

        # PLAY BUTTON
        self.playButton = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/playActivities/playbutton.png'),
                x=170,
                y=50,
                tooltip = "\n\n\n" + _('Play')
                )
        self.playButton.connect("button_press_event", self.staff.playComposition)

        gcompris.utils.item_focus_init(self.playButton, None)

        # ERASE BUTTON
        self.eraseButton = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/playActivities/erase.png'),
                x=650,
                y=170,
                tooltip = "\n\n\n" + _("Erase Attempt")
                )
        self.eraseButton.connect("button_press_event", self.erase_entry)
        gcompris.utils.item_focus_init(self.eraseButton, None)
        self.show_erase(False)

    def show_erase(self, status):
        goostatus = goocanvas.ITEM_VISIBLE if status else goocanvas.ITEM_INVISIBLE
        self.eraseButton.props.visibility = goostatus

    def clear_pass_or_fail(self):
        for note in self.staff.noteList:
            note.statusNone()

    def mark_pass_or_fail(self, note_played):
        ''' Find the displayed note matching the current play '''
        ''' and mark it as pass or fail '''
        current_note_number = len(self.kidsNoteList) - 1
        if len(self.staff.noteList) > current_note_number:
            note_on_staff = self.staff.noteList[current_note_number]
            if note_played.numID == note_on_staff.numID:
                note_on_staff.statusPassed(self.rootitem)
            else:
                note_on_staff.statusFailed(self.rootitem)
                self.show_erase(True)

        if len(self.staff.noteList) - 1 == current_note_number:
            gobject.timeout_add(500, self.display_bonus)

    def keyboard_click(self, widget=None, target=None, event=None, numID=None):
        if not numID:
            numID = target.numID
        if self.gcomprisBoard.level <= 6:
            n = QuarterNote(numID, 'trebleClef', self.staff.rootitem)
        else:
            n = QuarterNote(numID, 'bassClef', self.staff.rootitem)
        n.play()
        self.kidsNoteList.append(numID)
        self.mark_pass_or_fail(n)

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

        gobject.timeout_add(500, self.staff.playComposition)

    def display_bonus(self):
        if self.kidsNoteList == self.givenOption:
            self.afterBonus = self.nextChallenge
            gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.NOTE)
            self.score += 1
        else:
            self.afterBonus = self.tryagain
            gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.NOTE)
            self.score -= 1

    def tryagain(self):
        self.erase_entry()
        self.staff.playComposition

    def nextChallenge(self):
        self.kidsNoteList = []
        self.staff.eraseAllNotes()
        l = self.gcomprisBoard.level
        if self.score >= 5:
            self.set_level(l + 1)
            return
        self.show_melody()

    def erase_entry(self, widget=None, target=None, event=None):
        self.kidsNoteList = []
        self.show_erase(False)
        self.clear_pass_or_fail()

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
        elif keyval == gtk.keysyms.space:
            self.staff.playComposition()
        else:
            pianokeyBindings(keyval, self.keyboard_click)
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

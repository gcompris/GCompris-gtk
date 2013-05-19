#  gcompris - play_rhythm.py
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
# play_rhythm activity.

import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas
import pango
import time
import gettext
from gcompris import gcompris_gettext as _
from gettext import dgettext as D_
from gcomprismusic import *
from random import randint
import random


class Gcompris_play_rhythm:

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

        # Used to skip double clicks
        self.record_click_time = 0

        # Keep the timers we create to delete it before they happen
        # when no more needed.
        self.metronomeTimer = 0
        self.updateTimer = 0

        self.first_run = True 

    def start(self):

        self.running = True
        self.saved_policy = gcompris.sound.policy_get()
        gcompris.sound.policy_set(gcompris.sound.PLAY_AND_INTERRUPT)
        gcompris.sound.pause()

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
            gcompris.utils.dialog(D_(gcompris.GETTEXT_ERROR,"Error: This activity cannot be \
played with the\nsound effects disabled.\nGo to the configuration \
dialogue to\nenable the sound."), None)

        readyButton = TextButton(400, 455, ' ' * 20 + _('I am Ready') + ' ' * 20,
                                 self.rootitem, 0x11AA11FFL)
        readyButton.getBackground().connect("button_press_event",
                                            self.ready_event, readyButton)

    def display_level(self, level):

        self.recordedHits = []

        if self.updateTimer :
            gobject.source_remove(self.updateTimer)
            self.updateTimer = 0

        if hasattr(self, 'staff'):
            self.staff.clear()

        if self.rootitem:
            self.rootitem.remove()

        self.rootitem = goocanvas.Group(parent=
                                       self.gcomprisBoard.canvas.get_root_item())

        # set background
        goocanvas.Image(
            parent=self.rootitem,
            x=0, y=0,
            pixbuf = gcompris.utils.load_pixmap('piano_composition/playActivities/background/'
                                                + str( level % 6 + 1) + '.jpg')
            )

        goocanvas.Rect(parent=self.rootitem,
                       x=200, y=160, width=400, height=30,
                       stroke_color="black", fill_color='white',
                       line_width=3.0,
                       radius_x = 1.0, radius_y = 1.0)
        textBox(_("Beat Count:"), 260, 175, self.rootitem, noRect=True)
        gcompris.bar_set(gcompris.BAR_LEVEL)
        gcompris.bar_set_level(self.gcomprisBoard)
        gcompris.bar_location(20, -1, 0.6)

        self.staff = TrebleStaff(450, 175, self.rootitem, numStaves=1)

        self.staff.noteSpacingX = 36
        self.staff.endx = 200
        self.staff.labelBeatNumbers = True
        self.staff.drawStaff()
        self.staff.rootitem.scale(2.0, 2.0)
        self.staff.rootitem.translate(-350, -75)

        self.remainingOptions = self.getInitialRhythmOptions()

        self.givenOption = []

        # RECORD BUTTON
        self.drum = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('play_rhythm/drumhead.png'),
                x=300,
                y=60,
                tooltip = "\n\n\n" + _('Beat the rhythm on this drum.')
                )
        self.drum.connect("button_press_event", self.record_click)
        gcompris.utils.item_focus_init(self.drum, None)

        # METRONOME BUTTON
        self.metronomeButton = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('play_rhythm/metronome.png'),
                x=40,
                y=150,
                tooltip = "\n\n\n" + _('For a little help, click the metronome to hear the tempo.')
                )
        self.metronomeButton.connect("button_press_event", self.play_metronome)
        gcompris.utils.item_focus_init(self.metronomeButton, None)

        bx = 650
        by = 230

        goocanvas.Rect(parent = self.rootitem,
                       x = bx - 10,
                       y = by - 8,
                       width = 120,
                       height = 63,
                       stroke_color = "black",
                       fill_color_rgba = 0x33333399L,
                       line_width = 2.0,
                       radius_x = 3, radius_y = 3)
        # PLAY BUTTON
        self.playButton = goocanvas.Image(
                parent=self.rootitem,
                pixbuf = gcompris.utils.load_pixmap('piano_composition/play.svg'),
                x = bx,
                y = by,
                tooltip = "\n\n" + _('Play')
                )
        self.playButton.connect("button_press_event", self.staff.playComposition)
        self.playButton.connect("button_press_event", self.stopMetronome)

        gcompris.utils.item_focus_init(self.playButton, None)

        # ERASE BUTTON
        self.eraseButton = goocanvas.Image(
                parent=self.rootitem,
                pixbuf = gcompris.utils.load_pixmap('piano_composition/edit-clear.svg'),
                x = bx + 50,
                y = by,
                tooltip = "\n\n" + _("Erase Attempt")
                )
        self.eraseButton.connect("button_press_event", self.erase_entry)
        gcompris.utils.item_focus_init(self.eraseButton, None)

        self.metronomePlaying = False

        if level in [1, 3, 5, 7, 9, 11]:
            # show playing line, but no metronome
            self.playingLine = True
        else:
            # show metronome, but no playing line
            self.playingLine = False
        self.staff.drawPlayingLine = self.playingLine

        self.readyForFirstDrumBeat = True

        if not self.first_run:
            self.show_rhythm()
        self.updateBoard(1)

        self.playButton.connect("button_press_event", self.compositionIsPlaying)

    def stopMetronome(self, widget=None, target=None, event=None):
        self.metronomePlaying = False
        if self.metronomeTimer :
            gobject.source_remove(self.metronomeTimer)
            self.metronomeTimer = 0

    def play_metronome(self, widget=None, target=None, event=None):
        if not self.metronomePlaying:
            self.metronomeTime = gobject.timeout_add(500, self.playClick)
            self.metronomePlaying = True
        else:
            self.metronomePlaying = False
            gcompris.sound.play_ogg('boards/sounds/silence1s.ogg')

    def playClick(self):
        if self.metronomePlaying:
            gcompris.sound.play_ogg('play_rhythm/click.wav')
            self.metronomeTime = gobject.timeout_add(500, self.playClick)

    def getInitialRhythmOptions(self):
        level = self.gcomprisBoard.level

        if level in [ 1, 2]:
            options = [[4, 4, 4], [2, 2, 2], [1, 1] ]
        elif level in [ 3, 4]:
            options = [ [4, 2], [2, 4], [1, 4], [1, 2], [4, 1]]
        elif level in [5, 6]:
            options = [ [4, 2, 4], [4, 4, 4], [2, 4, 2], [4, 4, 4], [4, 2, 4], [2, 4, 2]]
        elif level in [7, 8]:
            options = [ [4, 2, 4, 2], [2, 4, 4, 4], [4, 2, 2, 4], [4, 2, 4], [2, 2, 2] ]
        elif level in [9, 10]:
            options = [ [8, 8, 8, 8], [4, 4, 4, 4], [2, 2, 2, 2], [4, 4, 4, 4] ]
        elif level in [11, 12]:
            options = [ [4, 8, 8, 4], [8, 8, 4, 4], [4, 4, 8, 4], [4, 8, 4, 8]]
        return options

    def generateRhythm(self):
        newrhythm = self.remainingOptions[randint(0, len(self.remainingOptions) - 1)]
        if newrhythm == self.givenOption and len(self.remainingOptions) > 1:
            return self.generateRhythm()
        else:
            return newrhythm


    def show_rhythm(self):
        self.givenOption = self.generateRhythm()
        self.songDuration = 0
        for item in self.givenOption:
            if item == 8:
                note = EighthNote(1, 'trebleClef', self.staff.rootitem)
            elif item == 4:
                note = QuarterNote(1, 'trebleClef', self.staff.rootitem)
            elif item == 2:
                note = HalfNote(1, 'trebleClef', self.staff.rootitem)
            elif item == 1:
                note = WholeNote(1, 'trebleClef', self.staff.rootitem)
            self.songDuration += note.millisecs
            self.staff.drawNote(note)
        if self.gcomprisBoard.level in [1, 3, 5, 7, 9, 11]:
            self.staff.playComposition()
            self.compositionIsPlaying()
        self.readyForFirstDrumBeat = True


    def compositionIsPlaying(self, x=None, y=None, z=None):
        self.updateBoard(1)
        self.updateTimer = gobject.timeout_add(self.songDuration,
                                               self.updateBoard, 2)

    def checkTiming(self, rhythmItem, recordedHit):
        ''' returns true if the it is within the range for a rhythmItem '''
        def nearlyEqual(inputNum, correctNum, amountOfError):
            return abs(inputNum - correctNum) <= amountOfError

        if rhythmItem == 8 and not nearlyEqual(recordedHit, 0.25, 0.2):
            return False
        elif rhythmItem == 4 and not nearlyEqual(recordedHit, 0.5, 0.2):
            return False
        elif rhythmItem == 2 and not nearlyEqual(recordedHit, 1.0, 0.2):
            return False
        elif rhythmItem == 1 and not nearlyEqual(recordedHit, 2.0, 0.2):
            return False
        return True


    def check_and_win(self):
        '''Check the answer is complete and if so display the bonus'''

        if len(self.recordedHits) != len(self.givenOption) \
                or not reduce(lambda x, y: x and y, self.recordedHits):
            return False

        self.afterBonus = self.nextChallenge
        gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.NOTE)
        self.remainingOptions.remove(self.givenOption)
        return True

    def tryagain(self):
        self.readyForFirstDrumBeat = True
        self.recordedHits = []
        self.updateBoard(2)

        for note in self.staff.noteList:
            note.statusNone()

    def updateBoard(self, currentStep):

        if not self.running:
            return

        if self.gcomprisBoard.level in [2, 4, 6, 8, 10, 12] and currentStep == 1:
            currentStep = 2
        if currentStep == 1: # the rhythm is being played
            self.makePlayButtonVisible(False)
            self.makeDrumButtonVisible(False)
            self.makeEraseButtonVisible(False)

        elif currentStep == 2: # the student should enter the rhythm into the drum
            self.makeDrumButtonVisible(True)
            self.makePlayButtonVisible(True)
            self.makeEraseButtonVisible(False)

        elif currentStep == 3: # the erase options should appear
            self.makePlayButtonVisible(False)
            if not self.check_and_win():
                self.makeEraseButtonVisible(True)

        if self.gcomprisBoard.level in [2, 4, 6, 8, 10, 12]:
            self.makeMetronomeButtonVisible(True)
        else:
            self.makeMetronomeButtonVisible(False)

        if self.gcomprisBoard.level in [1, 3, 5, 7, 9, 11]:
            if currentStep == 1:
                text = _("Listen to the rhythm and follow the moving line.")
            elif currentStep == 2:
                text = _("Click the drum to the tempo. Watch the vertical line when you start.") \
                    + "\n" + _("You can use the space bar to drum the tempo.")

        else:
            text = _("Now, read the rhythm. It won't be played for you. Then, beat the rhythm on the drum.")

        if currentStep == 3:
            text = _("Click erase to try again.")

        if hasattr(self, 'text'):
            self.text.remove()
            self.rect.remove()
        self.text, self.rect = \
            textBox(text, 400, 400, self.rootitem, width=400,
                    fill_color_rgba = 0x666666AAL)

    def convert(self, visible):
        if visible:
            return goocanvas.ITEM_VISIBLE
        else:
            return goocanvas.ITEM_INVISIBLE


    def makeEraseButtonVisible(self, visible):
        v = self.convert(visible)
        self.eraseButton.props.visibility = v

    def makePlayButtonVisible(self, visible):
        v = self.convert(visible)
        self.playButton.props.visibility = v

    def makeDrumButtonVisible(self, visible):
        v = self.convert(visible)
        self.drum.props.visibility = v

    def makeMetronomeButtonVisible(self, visible):
        v = self.convert(visible)
        self.metronomeButton.props.visibility = v

    def nextChallenge(self):

        if self.remainingOptions == []:
            if self.gcomprisBoard.level + 1 <= self.gcomprisBoard.maxlevel:
                self.set_level(self.gcomprisBoard.level + 1)
            else:
                self.set_level(1)
            return

        self.readyForFirstDrumBeat = True
        self.recordedHits = []
        self.staff.eraseAllNotes()
        self.show_rhythm()
        self.updateBoard(1)

    def erase_entry(self, widget=None, target=None, event=None):
        self.recordedHits = []
        self.readyForFirstDrumBeat = True
        self.updateBoard(2)

        for note in self.staff.noteList:
            note.statusNone()

    def record_click(self, widget=None, target=None, event=None):

        # Skip Double clicks
        if event:
            if event.type == gtk.gdk._2BUTTON_PRESS:
                return True
            if event.time - self.record_click_time <= 200:
                self.record_click_time = event.time
                return True
            self.record_click_time = event.time

        if self.readyForFirstDrumBeat and self.playingLine:
            self.staff.playComposition(playingLineOnly=True)

        if self.readyForFirstDrumBeat:
            self.readyForFirstDrumBeat = False
            self.updateTimer = gobject.timeout_add(self.songDuration,
                                                   self.updateBoard, 3)
            self.makePlayButtonVisible(False)

        if not self.metronomePlaying:
            gcompris.sound.play_ogg(gcompris.DATA_DIR +
                                    '/piano_composition/treble_pitches/1/1.wav')

        if self.recordedHits == []:
            self.startTime = time.time()
            # By definition, the first hit is a success
            self.recordedHits.append(True)
            note_on_staff = self.staff.noteList[0]
            note_on_staff.statusPassed(self.rootitem)
        else:
            lap = time.time() - self.startTime
            self.startTime = time.time()

            if len(self.recordedHits) + 1 <= len(self.staff.noteList):
                note_on_staff = self.staff.noteList[ len(self.recordedHits) ]
                rhythmItem = self.givenOption[len(self.recordedHits) - 1]

                if self.checkTiming(rhythmItem, lap):
                    self.recordedHits.append(True)
                    note_on_staff.statusPassed(self.rootitem)
                else:
                    self.recordedHits.append(False)
                    note_on_staff.statusFailed(self.rootitem)

    def end(self):
        self.running = False
        self.stopMetronome()
        self.staff.eraseAllNotes()
        # Remove the root item removes all the others inside it
        self.rootitem.remove()
        gcompris.sound.policy_set(self.saved_policy)
        gcompris.sound.resume()

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
            self.record_click()
        elif keyval == gtk.keysyms.Return:
            pass
        elif keyval == gtk.keysyms.Tab:
            if self.gcomprisBoard.level in [1, 3, 5, 7, 9, 11]:
                self.staff.playComposition()

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

    def ready_event(self, widget, target, event, button):
        button.destroy()
        self.show_rhythm()
        self.first_run = False

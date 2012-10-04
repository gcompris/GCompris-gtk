#  gcompris - note_names.py
#
# Copyright (C) 2003, 2008 Beth Hadley
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
# note_names activity.

import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas
import pango
from random import randint
import random

from gcompris import gcompris_gettext as _

from gcomprismusic import *


class Gcompris_note_names:

    def __init__(self, gcomprisBoard):

        # Save the gcomprisBoard, it defines everything we need
        # to know from the core
        self.gcomprisBoard = gcomprisBoard

        # Needed to get key_press
        gcomprisBoard.disable_im_context = True

        self.gcomprisBoard.level = 1
        self.gcomprisBoard.maxlevel = 20

        self.colorButtons = True # toggle to choose whether or not to make the text
        # note name buttons colored
        self.pitchSoundEnabled = True # toggle to choose whether or not to
        # play the pitch sounds
        self.master_is_not_ready = False # boolean to prepare sound timing

        self.remainingNotesToIdentify = []

        self.repeatThisNoteLaterPlease = False

        self.afterBonus = None

    def start(self):
        # Set the buttons we want in the bar
        gcompris.bar_set(gcompris.BAR_LEVEL)
        gcompris.bar_set_level(self.gcomprisBoard)
        gcompris.bar_location(275, -1, 0.8)

        self.saved_policy = gcompris.sound.policy_get()
        gcompris.sound.policy_set(gcompris.sound.PLAY_AND_INTERRUPT)
        gcompris.sound.pause()
        gcompris.sound.play_ogg('//boards/sounds/silence1s.ogg')
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
        '''
        display appropriate game level based on number. Game levels are:
        1. treble clef staff
        2. note-identification: white key treble notes
        3. note-identification: sharp notes
        4. note-identificaiton: flat notes
        5. bass clef staff
        6. note-identification: white key bass notes
        7. note-identification: sharp notes
        8. note-identification: flat notes
        '''
        if hasattr(self, 'staff'):
            self.staff.eraseAllNotes()
        if hasattr(self, 'noteButtonsRootItem'):
            self.noteButtonsRootItem.remove()
        if self.rootitem:
            self.rootitem.remove()


        self.rootitem = goocanvas.Group(parent=
                                       self.gcomprisBoard.canvas.get_root_item())

        if level == 1:
            if hasattr(self, 'staff'):
                self.staff.clear()
            self.staff = TrebleStaff(380, 170, self.rootitem, numStaves=1)
            self.staff.noteSpacingX = 36
            self.staff.drawStaff()
            self.staff.rootitem.scale(2.0, 2.0)
            self.staff.rootitem.translate(-350, -75)
            self.staff.drawScale('C Major')

            staffText = _("These are the eight basic notes in treble clef. They form the C Major Scale.")
        elif level == 11:
            if hasattr(self, 'staff'):
                self.staff.clear()
            self.staff = BassStaff(380, 170, self.rootitem, numStaves=1)
            self.staff.noteSpacingX = 36
            self.staff.drawStaff()
            self.staff.rootitem.scale(2.0, 2.0)
            self.staff.rootitem.translate(-350, -75)
            self.staff.drawScale('C Major')
            staffText = _("These are the eight basic notes in bass clef. \
They also form the C Major Scale. Notice that the note positions are different than in treble clef.")
        if level == 1 or level == 11:
            goocanvas.Text(
              parent=self.rootitem,
              x=400.0,
              y=65,
              width=350,
              text=staffText,
              fill_color="black",
              anchor=gtk.ANCHOR_CENTER,
              alignment=pango.ALIGN_CENTER
              )

            self.playScaleButton = textButton(400, 136, _("Play Scale"),
                                              self.rootitem, 0xCC0033FFL)

            self.playScaleButton.connect("button_press_event", self.staff.playComposition)
            gcompris.utils.item_focus_init(self.playScaleButton, None)

            if level == 1:
                text = _("Play Treble Clef Game")
            elif level == 11:
                text = _("Play Bass Clef Game")

            self.playScaleGameButton = textButton(400, 410, text,
                                                  self.rootitem, 0x00AA33FFL)
            self.playScaleGameButton.connect("button_press_event", self.play_scale_game)
            gcompris.utils.item_focus_init(self.playScaleGameButton, None)

        if level != 1 and level != 11:
            if level in [2, 5, 8, 12, 15, 18]:
                self.colorButtons = True
            else:
                self.colorButtons = False

            if level in [4, 7, 10, 14, 17, 20]:
                self.pitchSoundEnabled = False
            else:
                self.pitchSoundEnabled = True

            self.updateGameLevel(level)
            self.selectedNoteObject = None # the note name the child has selected
            # The OK Button
            item = goocanvas.Svg(parent=self.rootitem,
                                 svg_handle=gcompris.skin.svg_get(),
                                 svg_id="#OK"
                                 )
            item.translate(90, -120)
            item.connect("button_press_event", self.ok_event)
            gcompris.utils.item_focus_init(item, None)
            self.drawNoteButtons()

        if level in [2, 3, 4, 12, 13, 14]:
            instructionText = _("Click on the note name to match the pitch. Then click ok to check.")
        elif level in [5, 6, 7, 15, 16, 17]:
            instructionText = _("Now there are sharp notes. These pitches are raised a half step.")
        elif level in [8, 9, 10, 18, 19, 20]:
            instructionText = _("Now there are flat notes. These pitches are lowered a half step.")

        if level not in [1, 11]:
            textBox(instructionText, 650, 180, self.rootitem, 200,
                    fill_color_rgba = 0x00AA33AAL)

            textBox(_("Click the note to hear it played"),
                    160, 100, self.rootitem, 200,
                    fill_color_rgba = 0xCC0033AAL)

    def prepareGame(self):
        self.staff.eraseAllNotes()
        self.drawRandomNote(self.staff.staffName)

    def updateGameLevel(self, levelNum):
        '''
        update the level by updating pitchPossibilities and displaying the correct staff
        '''

        self.rootitem.remove()
        self.rootitem = goocanvas.Group(parent=
                                        self.gcomprisBoard.canvas.get_root_item())

        if self.gcomprisBoard.level <= 10:
            self.staff = TrebleStaff(380, 160, self.rootitem, numStaves=1)
            self.staff.endx = 110
            self.staff.noteSpacingX = 36
            self.staff.drawStaff()
            self.staff.rootitem.scale(2.0, 2.0)
            self.staff.rootitem.translate(-350, -75)
        else:
            self.staff = BassStaff(380, 160, self.rootitem, numStaves=1)
            self.staff.endx = 110
            self.staff.noteSpacingX = 36
            self.staff.drawStaff()
            self.staff.rootitem.scale(2.0, 2.0)
            self.staff.rootitem.translate(-350, -75)

        if levelNum not in [2, 5, 8, 12, 15, 18]:
            self.staff.colorCodeNotes = False
        self.pitchPossibilities = [1, 2, 3, 4, 5, 6, 7, 8]
        self.sharpNotation = True
        if levelNum in [5, 6, 7, 8, 9, 10, 15, 16, 17, 18, 19, 20]:
            self.pitchPossibilities.extend([-1, -2, -3, -4, -5])

        if levelNum in [8, 9, 10, 18, 19, 20]:
            self.sharpNotation = False
        self.remainingNotesToIdentify = self.pitchPossibilities
        self.drawRandomNote(self.staff.staffName)
        self.drawNoteButtons()


    def color_button_toggle(self, widget=None, target=None, event=None):
        '''
        update text color of note buttons
        '''
        if self.colorButtons:
            self.colorButtons = False
            self.colorTextToggle.props.text = _("Need a hint? Click here for color-coded note names.")
        else:
            self.colorButtons = True
            self.colorTextToggle.props.text = _("Ready for a challenge? Color all buttons black!")
        self.drawNoteButtons()

    def turn_off_pitch_sound(self, widget=None, target=None, event=None):
        '''
        turn on or off pitch sounds on mouse-over
        '''
        if self.pitchSoundEnabled:
            self.pitchSoundEnabled = False
        else:
            self.pitchSoundEnabled = True


    def drawRandomNote(self, staffType):
        '''
        draw a random note, selected from the pitchPossibilities, and save as self.currentNote
        '''

        newNoteID = \
            self.remainingNotesToIdentify[randint(0, len(self.pitchPossibilities) - 1)]

        # don't repeat the same note twice
        if hasattr(self, 'currentNote') \
                and self.currentNote.numID == newNoteID \
                and len(self.remainingNotesToIdentify) > 1:
            self.drawRandomNote(staffType)
            return

        note = QuarterNote(newNoteID, staffType, self.staff.rootitem, self.sharpNotation)

        self.staff.drawNote(note)
        if self.pitchSoundEnabled:
            note.play()
        note.enablePlayOnClick()
        self.currentNote = note

    def play_scale_game(self, widget=None, target=None, event=None):
        '''
        button to move to the next level and have kids play the game. Also
        possible to just click the level arros at bottom of screen
        '''
        if self.gcomprisBoard.level == 1:
            self.gcomprisBoard.level = 2
        else:
            self.gcomprisBoard.level = 12
        gcompris.bar_set_level(self.gcomprisBoard)
        self.display_level(self.gcomprisBoard.level)


    def drawNoteButtons(self):
        '''
        draw the note buttons seen on the screen, optionally make them color-coded or black
        '''
        if hasattr(self, 'noteButtonsRootItem'):
            self.noteButtonsRootItem.remove()

        self.noteButtonsRootItem = goocanvas.Group(parent=
                                        self.rootitem, x=0, y=0)


        def drawNoteButton(x, y, numID, play_sound_on_click):
            '''
            local method to draw one button
            '''
            if self.colorButtons:
                color = NOTE_COLOR_SCHEME[numID]
            else:
                color = 'white'
            text = getKeyNameFromID(numID, self.sharpNotation)
            vars(self)[str(numID)] = goocanvas.Text(
              parent=self.rootitem,
              x=x,
              y=y,
              text=text,
              fill_color='black',
              anchor=gtk.ANCHOR_CENTER,
              alignment=pango.ALIGN_CENTER,
              pointer_events="GOO_CANVAS_EVENTS_NONE"
              )

            rect = goocanvas.Rect(parent=self.rootitem,
                              x=x - 12.5,
                              y=y - 10,
                              width=25,
                              height=20,
                              line_width=.5,
                              fill_color=color)
            vars(self)[str(numID)].raise_(None)
            vars(self)[str(numID)].scale(2.0, 2.0)
            vars(self)[str(numID)].translate(-250, -150)
            rect.scale(2, 2)
            rect.translate(-250, -150)
            rect.connect("button_press_event", play_sound_on_click, numID)
            rect.set_data('numID', numID)
            gcompris.utils.item_focus_init(rect, None)
            gcompris.utils.item_focus_init(vars(self)[str(numID)], None)

        x = 420
        y = 220
        random.shuffle(self.pitchPossibilities)
        for numID in self.pitchPossibilities:
            if numID != 8:
                # do not draw another C-button, since this is ambiguous, because
                # we have a c' and a c"
                drawNoteButton(x, y, numID, self.play_sound_on_click)
                y += 30
                if y > 320:
                    y = 220
                    x = x + 40

    def play_sound_on_click(self, widget, target, event, numID):
        '''
        plays the note sound when the mouse clicks on the note name
        '''
        self.selectedNoteObject = widget

        if self.pitchSoundEnabled:
            # change the sound to be played to the c" sound, if we asked for a c"
            # and the user clicked the c-button
            if self.currentNote.numID == 8 and numID == 1:
                numID = 8
            HalfNote(numID, self.staff.staffName, self.staff.rootitem).play()
        if hasattr(self, 'focusRect'):
            self.focusRect.remove()

        self.focusRect = goocanvas.Rect(parent=self.rootitem,
                                        x=self.selectedNoteObject.props.x + 6,
                                        y=self.selectedNoteObject.props.y + 8.5,
                                        width=28, height=23,
                                        radius_x=5, radius_y=5,
                                        stroke_color="black", line_width=1.0)
        self.focusRect.translate(-515, -320)
        self.focusRect.scale(2.0, 2.0)

    def readyToSoundAgain(self):
        self.master_is_not_ready = False


    def ok_event(self, widget, target, event):
        '''
        called when the kid presses a notename. Checks to see if this is the correct
        note name. displays the appropriate bonus, and resets the board if appropriate
        '''

        self.master_is_not_ready = True
        gobject.timeout_add(1500, self.readyToSoundAgain)
        g = self.selectedNoteObject.get_data('numID')
        c = self.currentNote.numID

        # since we only have one c-button, the user has to click c' (=1) if c" (=8) is asked
        if g == c or (c == 8 and g == 1):
            if not self.repeatThisNoteLaterPlease:
                self.remainingNotesToIdentify.remove(c)
            if self.remainingNotesToIdentify == []:
                self.afterBonus = lambda: self.set_level(self.gcomprisBoard.level + 1)
                gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.NOTE)
            else:
                self.afterBonus = self.prepareGame
                gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.NOTE)
            self.repeatThisNoteLaterPlease = False
        else:
            gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.NOTE)
            self.repeatThisNoteLaterPlease = True


    def end(self):
        # Remove the root item removes all the others inside it
        self.staff.eraseAllNotes()
        self.rootitem.remove()
        if hasattr(self, 'noteButtonsRootItem'):
            self.noteButtonsRootItem.remove()
        gcompris.sound.policy_set(self.saved_policy)
        gcompris.sound.resume()

    def set_level(self, level):
        '''
        updates the level for the game when child clicks on bottom
        left navigation bar to increment level
        '''
        self.staff.eraseAllNotes()
        self.gcomprisBoard.level = level
        gcompris.bar_set_level(self.gcomprisBoard)
        self.display_level(level)

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
        strn = u'%c' % utf8char

    def pause(self, pause):
        if not pause and self.afterBonus:
            self.afterBonus()
            self.afterBonus = None


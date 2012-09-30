# -*- coding: utf-8 -*-
#  gcompris - piano_composition.py
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
# piano_composition activity.


import gobject
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas
import pango
import gcompris.sound
import ConfigParser

from gcompris import gcompris_gettext as _

from gcomprismusic import *

class Gcompris_piano_composition:

    def __init__(self, gcomprisBoard):

        # Save the gcomprisBoard, it defines everything we need
        # to know from the core
        self.gcomprisBoard = gcomprisBoard

        self.gcomprisBoard.level = 1
        self.gcomprisBoard.maxlevel = 7

        # Needed to get key_press
        gcomprisBoard.disable_im_context = True

        self._bachEasterEggDone = False
        self._mozartEasterEggDone = False
        self._bananaEasterEggGone = False
        self._gsoc2012EasterEggGone = False

        self.noClefDescription = False

        self.melodyPageToDisplay = 0
    def start(self):
        # write the navigation bar to bottom left corner
        gcompris.bar_set(gcompris.BAR_LEVEL)
        gcompris.bar_set_level(self.gcomprisBoard)
        gcompris.bar_location(20, -1, 0.6)

        self.saved_policy = gcompris.sound.policy_get()
        gcompris.sound.policy_set(gcompris.sound.PLAY_AND_INTERRUPT)
        gcompris.sound.pause()

        # Set a background image
        gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

        # Create our rootitem. We put each canvas item in it so at the end we
        # only have to kill it. The canvas deletes all the items it contains
        # automaticaly.
        self.rootitem = goocanvas.Group(parent=
                                       self.gcomprisBoard.canvas.get_root_item())

        self.display_level(self.gcomprisBoard.level)

    def display_level(self, level):
        '''
        displays level contents.
        All levels: keyboard, staff, write & erase notes, play composition
        1. treble clef only
        2. bass clef only
        3. note duration choice, treble or bass choice
        4. sharp notes, note duration choice, treble or bass choice
        5. flat notes, note duration choice, treble or bass choice
        6. load and save, only sharp notes, note duration choice, treble or bass choice
        (7. , loads melodies)
        '''
        if hasattr(self,'staff'):
            self.staff.eraseAllNotes()

        if self.rootitem:
            self.rootitem.remove()

        self.rootitem = goocanvas.Group(parent=
                                       self.gcomprisBoard.canvas.get_root_item())
        if level == 8:
            self.displayMelodySelection()
            self.noClefDescription = True
            return


        '''
        create staff instance to manage music data
        treble clef image loaded by default; click button to switch to bass clef
        '''

        if level == 2:
            self.staff = BassStaff(370, 185, self.rootitem, 3)
        else:
            self.staff = TrebleStaff(370, 185, self.rootitem, 3)

        self.staff.drawStaff()
        self.staff.dynamicNoteSpacing = True

        clefDescription = keyboardDescription = not self.noClefDescription
        if level == 1:
            clefText = _("This is the Treble clef staff, for high pitched notes")
            keyboardText = _("These are the 8 \"white\" keys in an octave")
        elif level == 2:
            clefText = _("This is the Bass clef staff, for low pitched notes")
            keyboardText = _("These keys form the C Major scale")
        elif level == 3:
            clefText = _("Click on the note symbols to write different length notes")
            keyboardText = _("Notes can be many types, such as quarter notes, half notes, and whole notes")
        elif level == 4:
            clefText = _("Sharp notes have a # sign")
            keyboardText = _("The black keys are sharp and flat keys")
        elif level == 5:
            clefText = _("Flat notes have a b sign")
            keyboardText = _("Each black key has two names, one with a flat and one with a sharp")
        elif level == 6:
            clefText = _("Now you can load melodies from around the world")
            keyboardDescription = False
        elif level == 7:
            clefText = _("Compose music now! You can now load and save your work")
            keyboardDescription = False

        # CLEF DESCRIPTION
        if clefDescription:
            textBox(clefText, 550, 67, self.rootitem, 240,
                    fill_color_rgba = 0x82B22CFFL)

        # KEYBOARD DESCRIPTION
        if keyboardDescription:
            textBox(keyboardText, 200, 430, self.rootitem, 225,
                    fill_color_rgba = 0x639A0066L)

        # ADD BUTTONS

        bx = 500
        if (level == 7):
            bx = 450
        by = 455

        goocanvas.Rect(parent = self.rootitem,
                       x = 430,
                       y = by - 6,
                       width = 280,
                       height = 61,
                       stroke_color = "black",
                       fill_color_rgba = 0x33333366L,
                       line_width = 2.0,
                       radius_x = 3, radius_y = 3)

        self.eraseAllButton = goocanvas.Image(
            parent=self.rootitem,
            pixbuf=gcompris.utils.load_pixmap('piano_composition/edit-clear.svg'),
            x = bx,
            y = by,
            tooltip =  "\n\n" + _("Erase All Notes")
            )
        self.eraseAllButton.connect("button_press_event", self.askAndEraseStaff)
        gcompris.utils.item_focus_init(self.eraseAllButton, None)


        self.eraseNotesButton = goocanvas.Image(
            parent=self.rootitem,
            pixbuf=gcompris.utils.load_pixmap('piano_composition/edit-undo.svg'),
            x = bx + 50,
            y = by,
            tooltip =  "\n\n" + _("Erase Last Notes")
            )
        self.eraseNotesButton.connect("button_press_event", self.eraseOneNote)
        gcompris.utils.item_focus_init(self.eraseNotesButton, None)

        self.playCompositionButton = goocanvas.Image(
            parent=self.rootitem,
            pixbuf=gcompris.utils.load_pixmap('piano_composition/play.svg'),
            x = bx + 100,
            y = by,
            tooltip =  "\n\n" + _("Play Composition")
            )
        self.playCompositionButton.connect("button_press_event", self.playComposition)
        gcompris.utils.item_focus_init(self.playCompositionButton, None)

        if (level > 2):

            self.changeClefButton = goocanvas.Image(
                parent = self.rootitem,
                pixbuf = gcompris.utils.load_pixmap('piano_composition/bassclef_button.svg'),
                x = 90,
                y = 120,
                tooltip =  "\n\n" + _("Erase and Change Clef"),
            )
            self.changeClefButton.connect("button_press_event", self.change_clef_event)
            gcompris.utils.item_focus_init(self.changeClefButton, None)


        if (level >= 3):
            goocanvas.Text(
                parent=self.rootitem,
                x=210, y=140,
                width=100,
                text=_("Change Note Type:"),
                fill_color="black", anchor=gtk.ANCHOR_CENTER,
                alignment=pango.ALIGN_CENTER
                )

            x = 260
            y = 115

            # WRITE NOTE BUTTONS TO SELECT NOTE DURATION

            self.eighthNoteSelectedButton = goocanvas.Group(parent=
                                       self.rootitem)

            goocanvas.Image(
                parent=self.eighthNoteSelectedButton,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/quarterNote.png'),
                x=x,
                y=y,
                height=45,
                width=20,
                tooltip = "\n\n" + _("Eighth Note")
                )

            goocanvas.Image(
              parent=self.eighthNoteSelectedButton,
              pixbuf=gcompris.utils.load_pixmap("piano_composition/flag.png"),
              x=x + 17,
              y=y + 5 ,
              height=33,
              width=10
              )
            self.eighthNoteSelectedButton.connect("button_press_event", self.updateToEighth)
            gcompris.utils.item_focus_init(self.eighthNoteSelectedButton, None)


            self.quarterNoteSelectedButton = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/quarterNote.png'),
                x=x + 35,
                y=y,
                height=45,
                width=20,
                tooltip = "\n\n" + _("Quarter Note")
                )
            self.quarterNoteSelectedButton.connect("button_press_event", self.updateToQuarter)
            gcompris.utils.item_focus_init(self.quarterNoteSelectedButton, None)

            self.halfNoteSelected = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/halfNote.png'),
                x=x + 62,
                y=y,
                height=45,
                width=20,
                tooltip = "\n\n" + _("Half Note")
                )
            self.halfNoteSelected.connect("button_press_event", self.updateToHalf)
            gcompris.utils.item_focus_init(self.halfNoteSelected, None)

            self.wholeNoteSelected = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/wholeNote.png'),
                x=x + 90,
                y=y,
                height=45,
                width=20,
                tooltip = "\n\n" + _("Whole Note")
                )
            self.wholeNoteSelected.connect("button_press_event", self.updateToWhole)
            gcompris.utils.item_focus_init(self.wholeNoteSelected, None)

            # draw focus rectangle around quarter note duration, the default
            self.staff.drawFocusRect(292, 112)

        if (level in [6, 7]):
            self.makeFlatButton = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/blackflat.png'),
                x=175,
                y=410,
                height=40,
                width=30
                )
            self.makeFlatButton.connect("button_press_event", self.change_accidental_type)
            gcompris.utils.item_focus_init(self.makeFlatButton, None)

            self.makeSharpButton = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/blacksharp.png'),
                x=175,
                y=410,
                height=40,
                width=30
                )
            self.makeSharpButton.connect("button_press_event", self.change_accidental_type)
            gcompris.utils.item_focus_init(self.makeSharpButton, None)
            self.makeSharpButton.props.visibility = goocanvas.ITEM_INVISIBLE

            self.loadSongsButton = textButton(280, 430, _("Load Music"),
                                              self.rootitem, 0xE768ABFFL, 100)
            self.loadSongsButton.connect("button_press_event", self.load_songs_event)
            gcompris.utils.item_focus_init(self.loadSongsButton, None)

            textBox(_("Change Accidental Style:"), 100, 430, self.rootitem, width=150, noRect=True)

        if (level == 7):
            self.loadButton = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/open.svg'),
                x = bx + 150,
                y = by,
                tooltip = "\n\n" + "Open Composition"
                )
            self.loadButton.connect("button_press_event", self.load_file_event)
            gcompris.utils.item_focus_init(self.loadButton, None)


            self.saveButton = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/save.svg'),
                x = bx + 200,
                y = by,
                tooltip = "\n\n" + "Save Composition"
                )
            self.saveButton.connect("button_press_event", self.save_file_event)
            gcompris.utils.item_focus_init(self.saveButton, None)


        '''
        create piano keyboard for use on every level
        optionally specify to display the "black keys"
        '''
        self.keyboard = PianoKeyboard(50, 180, self.rootitem)

        if level == 5:
            self.keyboard.sharpNotation = False
            self.keyboard.blackKeys = True
        elif level in [4, 6, 7]:
            self.keyboard.blackKeys = True
            self.keyboard.sharpNotation = True

        self.keyboard.draw(300, 200, self.keyboard_click)

        if not (gcompris.get_properties().fx):
            gcompris.utils.dialog(_("Error: This activity cannot be \
played with the\nsound effects disabled.\nGo to the configuration \
dialogue to\nenable the sound."), None)


    def displayMelodySelection(self):
        '''
        parse the text file melodies.txt and display the melodies for selection
        '''

        self.noClefDescription = True

        goocanvas.Text(parent=self.rootitem,
         x=290,
         y=30,
         text='<span font_family="Arial" size="15000" \
         weight="bold">' + _('Select A Melody to Load') + '</span>',
         fill_color="black",
         use_markup=True,
         pointer_events="GOO_CANVAS_EVENTS_NONE"
         )

        self.read_data()
        if self.melodyPageToDisplay==0:
            txt = _("Next Page")
        else:
            txt = _("Previous Page")
        self.nextMelodiesButton = textButton(700,475,txt, self.rootitem, 0xE768ABFFL)
        self.nextMelodiesButton.connect("button_press_event", self.nextMelodyPage)
        gcompris.utils.item_focus_init(self.nextMelodiesButton, None)

        self.writeDataToScreen()

    def nextMelodyPage(self, x=None,y=None,z=None):
        if self.melodyPageToDisplay == 0:
            self.melodyPageToDisplay = 1

        else:
            self.melodyPageToDisplay = 0
        self.display_level(8)
    def writeDataToScreen(self):

        def displayTitle(section, x, y):
            newRoot = goocanvas.Group(parent=self.rootitem)
            self.text = goocanvas.Text(
                parent=newRoot,
                x=x, y=y,
                text='<span size="13000"> ' + self.data.get(section, 'title') + '</span>',
                fill_color="black",
                use_markup=True
                )

            self.origin = goocanvas.Text(parent=newRoot,
                 x=x + 30,
                 y=y + 18,
                 width=250,
                 text='<span font_family="URW Gothic L" size="10000" \
                 weight="bold">' + _(self.data.get(section, '_origin')) + '</span>',
                 fill_color="black",
                 use_markup=True
                 )

            self.text.connect("button_press_event", self.melodySelected, section)
            self.origin.connect("button_press_event", self.melodySelected, section)
            gcompris.utils.item_focus_init(newRoot, None)

        x = 55
        y = 75

        if self.melodyPageToDisplay == 0:
            lower = 0
            upper = (len(self.data.sections())) / 2 - 1
        else:
            lower = (len(self.data.sections())) / 2 - 1
            upper = len(self.data.sections())
        for section in self.data.sections()[lower:upper]:
            displayTitle(section, x, y)
            if y > 400:
                y = 75
                x += 275
            else:
                y += 48

    def read_data(self):
            '''
            method to read in the data from melodies.desktop.in. Saves this data as
            self.data for reference later.
            '''
            #self.data = ConfigParser.RawConfigParser() # the data that is parsed from
            config = ConfigParser.RawConfigParser()
            filename = gcompris.DATA_DIR + '/' + self.gcomprisBoard.name + '/melodies.desktop.in'
            try:
                gotit = config.read(filename)
                if not gotit:
                    gcompris.utils.dialog(_("Cannot find the file '{filename}'").\
                                        format(filename=filename),
                                    None)
                    return False
            except ConfigParser.Error, error:
                    gcompris.utils.dialog(_("Failed to parse data set '{filename}'"
                                      " with error:\n{error}").\
                                      format(filename=filename, error=error),
                                    None)
                    return False

            self.data = config


    def createBgForItem(self, item, color):
        '''
        Called with an item, get its bounds and create a grey rectangle around it
        '''
        bounds = item.get_bounds()
        gap = 5
        return goocanvas.Rect(parent = self.rootitem,
                              x = bounds.x1 - gap,
                              y = bounds.y1 - gap,
                              width = bounds.x2 - bounds.x1 + gap * 2,
                              height = bounds.y2 - bounds.y1 + gap * 2,
                              stroke_color = "black",
                              fill_color_rgba = color,
                              line_width = 2.0,
                              radius_x = 3, radius_y = 3)


    def melodySelected(self, widget, target, event, section):
        '''
        called once a melody has been selected
        writes the melody to the staff, and displays the title and lyrics
        '''
        self.display_level(self.gcomprisBoard.level)
        self.staff.stringToNotation(self.data.get(section, 'melody'))
        rootitem = self.staff.lyrics_rootitem
        item = goocanvas.Text(parent = rootitem,
                       x=150,
                       y=15,
                       width=280,
                       font = gcompris.skin.get_font("gcompris/board/medium"),
                       text = self.data.get(section, 'title'),
                       fill_color="black",
                       use_markup=True,
                       alignment=pango.ALIGN_CENTER,
                       anchor = gtk.ANCHOR_N,
                       pointer_events="GOO_CANVAS_EVENTS_NONE"
                       )
        bg = self.createBgForItem(item, 0xAA333366L)
        bg.lower(None)

        lyrics = self.data.get(section, 'lyrics').replace('\\n', '\n')
        item = goocanvas.Text(parent = rootitem,
                       x = 400,
                       y = 15,
                       width = 280,
                       text = lyrics,
                       fill_color = "black",
                       pointer_events = "GOO_CANVAS_EVENTS_NONE"
                       )
        bg = self.createBgForItem(item, 0x33AA3366L)
        bg.lower(None)

        item = goocanvas.Text(parent = rootitem,
                       x=150,
                       y=75,
                       width=280,
                       text='<span weight="bold" >' + _(self.data.get(section, '_origin')) + '</span>',
                       fill_color="black",
                       use_markup=True,
                       alignment=pango.ALIGN_CENTER,
                       anchor = gtk.ANCHOR_N,
                       font = gcompris.skin.get_font("gcompris/board/small"),
                       pointer_events="GOO_CANVAS_EVENTS_NONE"
                       )
        bg = self.createBgForItem(item, 0x3333AA66L)
        bg.lower(None)


    def load_songs_event(self, widget, target, event):
        #self.staff.eraseAllNotes()
        self.display_level(8)

    def save_file_event(self, widget, target, event):
        '''
        method called when 'save' button is pressed
        opens the gcompris file selector to save the music
        calls the general_save function, passes self.staff
        '''
        gcompris.file_selector_save(self.gcomprisBoard,
                                   'comp', '.txt',
                                   general_save, self.staff)
    def load_file_event(self, widget, target, event):
        '''
        method called when 'load' button is pressed
        opens the gcompris file selector to load the music
        calls the general_load function
        '''
        gcompris.file_selector_load(self.gcomprisBoard,
                                           'comp', '.gcmusic',
                                           general_load, self.staff)

    def change_clef_event(self, widget, target, event):
        '''
        method called when button to change clef is pressed
        load in the appropriate new staff image
        re-synchronize all buttons because rootitem changes
        all notes in staff are removed...this can be changed later
        if desired, but could confuse children if more than one clef
        exists in the piece
        '''
        current_note_type = self.staff.currentNoteType
        self.staff.clear()
        if hasattr(self.staff, 'newClef'):
            self.staff.newClef.clear()
        if self.staff.staffName == "trebleClef":
            self.changeClefButton.props.pixbuf = gcompris.utils.load_pixmap('piano_composition/trebbleclef_button.svg')
            self.staff = BassStaff(370, 185, self.rootitem, 3)
            self.staff.drawStaff()
            self.staff.dynamicNoteSpacing = True
        else:
            self.changeClefButton.props.pixbuf = gcompris.utils.load_pixmap('piano_composition/bassclef_button.svg')
            self.staff = TrebleStaff(370, 185, self.rootitem, 3)
            self.staff.drawStaff()
            self.staff.dynamicNoteSpacing = True
        self.staff.currentNoteType = current_note_type


    def eraseOneNote(self, unused1, unused2, unused3):
        self.staff.eraseOneNote()

    def playComposition(self, unused1, unused2, unused3):
        self.staff.playComposition()

    def updateToEighth(self, unused1, unused2, unused3):
        self.staff.updateToEighth()

    def updateToQuarter(self, unused1, unused2, unused3):
        self.staff.updateToQuarter()

    def updateToHalf(self, unused1, unused2, unused3):
        self.staff.updateToHalf()

    def updateToWhole(self, unused1, unused2, unused3):
        self.staff.updateToWhole()

    def askAndEraseStaff(self, unused1, unused2, unused3):
        # @FIXME, should have a dialog asking for confirmation
        self.staff.eraseAllNotes()

    def erase(self,x,y,z):
        #self.staff.eraseAllNotes()
        eraseUserPrompt(None, None, None,self)

    def change_accidental_type(self, widget, target, event):
        self.keyboard.sharpNotation = not self.keyboard.sharpNotation
        self.keyboard.draw(300, 200, self.keyboard_click)
        if self.keyboard.sharpNotation:
            self.makeSharpButton.props.visibility = goocanvas.ITEM_INVISIBLE
            self.makeFlatButton.props.visibility = goocanvas.ITEM_VISIBLE
        else:
            self.makeSharpButton.props.visibility = goocanvas.ITEM_VISIBLE
            self.makeFlatButton.props.visibility = goocanvas.ITEM_INVISIBLE

    def keyboard_click(self, widget, target, event, numID=None):
        '''
        called whenever a key rectangle is pressed; a note object is created
        with a note name, text is output to canvas, the note sound is generated,
        and the note is drawn on the staff
        '''
        if hasattr(self.staff, 'locked') and self.staff.locked:
            return
        if not numID:
            numID = target.numID
        if numID < 0 and self.gcomprisBoard.level < 4:
            return
        if self.staff.currentNoteType == 4:
            n = QuarterNote(numID, self.staff.staffName, self.staff.rootitem, self.keyboard.sharpNotation)
        elif self.staff.currentNoteType == 2:
            n = HalfNote(numID, self.staff.staffName, self.staff.rootitem, self.keyboard.sharpNotation)
        elif self.staff.currentNoteType == 1:
            n = WholeNote(numID, self.staff.staffName, self.staff.rootitem, self.keyboard.sharpNotation)
        elif self.staff.currentNoteType == 8:
            n = EighthNote(numID, self.staff.staffName, self.staff.rootitem, self.keyboard.sharpNotation)

        self.staff.drawNote(n)
        n.play()
        n.enablePlayOnClick()

        self.checkForEasterEgg()
        return False

    def checkForEasterEgg(self):
        '''
        Note From Beth:
        At GUADEC, I was talking to one of the organizers of GNOME-GSOC and he
        said that what he really missed about GNOME was all the fun easter eggs it
        used to have....apparently Cheese had a fun one, and lots of other cool things.
        So he actually encouraged us to put some in ourselves ;-) So here are mine,
        they're pretty rare so I don't expect many people to find them
        but I think they're legitimate and shouldn't be taken out.

        If you're reading this to figure out what the easter eggs are...then you
        obviously have an interest in music and code. why not code a music activyt
        for GCompris? I'll help you...just email me.

        And now, the easter egges are:

        if you enter the notes (on the American keyboard) Bb/A# - A - C - B or (on the
        German keyboard) B - A - C - H, a picture of Bach's statue in front of his
        cathedral in Leipzig will appear. This is a picture I took during the summer
        I was coding this activity, and I had the pleasure to visit Leipzig in Germany

        if you enter the following (note the note name is first, followed by thr rhythm length
        (4=quarter, 8=eighth, 2=half, 1= whole)

        B4 A4 Ab4 A4 Ab4 A4 , a banana will appear to fly across the screen. this is
        a joke from GUADEC...if you're really curious, email bethmhadley@gmail.com ;-)

        if you enter

        G2 E2 F8 A8 G8 F8 E8 E2 F4 D8 F4 D8

        then you'll see picture of the inside of a cathedral, with a girl standing
        on the right. Do you know who it is and where? It's the inside of a cathedral in
        Salzburg, Mozart's birthplace. Mozart served as music master here for a few years
        (because he was forced to leave! ;-) The girl, by the way, is me...I was lucky
        enough to visit Salzurg during the summer I spent coding this activity, and took
        this picture.

        and if you enter

        C4 C8 C8 C E4 E8 E8 E4 G4 G8 G8 G4

        you'll see the Google Summer of Code Banner move across the screen. This is in
        recognition of the wonderful program that enabled me to make this summer project
        a possibility. I really enjoyed working this summer on this code, and I am very
        grateful to my mentor, Bruno Coudoin, for his support and assistance in this process.
        I'm also thankful to the entire GUADEC and GCompris communities for their ideas, support,
        and encouragement.

        Thanks everyone, and enjoy!

        '''
        s = r = ''
        for x in self.staff.noteList:
            s += str(x.numID)
            r += str(x.numID) + str(x.noteType)

        if not self._bachEasterEggDone and '-5617' in s:
            responsePic = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/bach.jpg'),
                x=250,
                y=50
                )
            gobject.timeout_add(2000, lambda: responsePic.remove() )
            self._bachEasterEggDone = True
        if not self._mozartEasterEggDone and '523248685848383244284428' in r:
            responsePic = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/mozart.jpg'),
                x=160,
                y=80
                )
            gobject.timeout_add(2000, lambda: responsePic.remove() )
            self._mozartEasterEggDone = True
        if not self._bananaEasterEggGone and '7464-4464-4464' in r:
            responsePic = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/banana.png'),
                x=200,
                y=10
                )
            responsePic.animate(5, 5, 3.0, 0.0, \
                absolute=False, duration=800, step_time=100, type=goocanvas.ANIMATE_FREEZE)

            gobject.timeout_add(800, lambda: responsePic.remove() )

            self._bananaEasterEggGone = True
        if not self._gsoc2012EasterEggGone and '14181814343838345458585481' in r:
            responsePic = goocanvas.Image(
                parent=self.rootitem,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/gsoc2012.jpg'),
                x= -20,
                y=10
                )
            self.responsePic.animate(0, 400, 1, 0, \
                absolute=False, duration=1000, step_time=100, type=goocanvas.ANIMATE_FREEZE)

            gobject.timeout_add(1000, lambda: responsePic.remove() )
            self._gsoc2012EasterEggGone = True

    def end(self):
        # Remove the root item removes all the others inside it
        self.staff.eraseAllNotes()
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
            self.staff.eraseOneNote()
        elif keyval == gtk.keysyms.Delete:
            self.askAndEraseStaff()
        elif keyval == gtk.keysyms.space:
            self.staff.playComposition()
        else:
            pianokeyBindings(keyval, self.keyboard_click)
        return True
    def pause(self, x):
        pass

    def set_level(self, level):
        '''
        updates the level for the game when child clicks on bottom
        left navigation bar to increment level
        '''
        self.noClefDescription = False
        self.gcomprisBoard.level = level
        gcompris.bar_set_level(self.gcomprisBoard)
        self.display_level(self.gcomprisBoard.level)

def general_save(filename, filetype, staffInstance):
    '''
    called after file selection to save
    '''
    staffInstance.staff_to_file(filename)

def general_load(filename, filetype, staffInstance):
    '''
    called after file selection to load
    '''
    staffInstance.file_to_staff(filename)


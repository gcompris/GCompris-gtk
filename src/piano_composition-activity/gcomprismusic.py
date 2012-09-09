# -*- coding: utf-8 -*-
#  gcompris - gcomprismusic.py
#
# Copyright (C) 2012 Beth Hadley
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty 00of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, see <http://www.gnu.org/licenses/>.
#
#

'''
Please see
http://gcompris.net/wiki/Adding_a_music_activity_and_using_gcomprismusic.py_module
for complete documentation (with examples!) of this module
'''

import gobject
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas
import gcompris.bonus
import pango
import gcompris.sound
from gcompris import gcompris_gettext as _
import cPickle as pickle
import copy
from random import randint
import random

# Rainbow color scheme used throughout games,
# according to music research on best
# techniques to teach young children music

NOTE_COLOR_SCHEME = {1:'#FF0000',
                     - 1:'#FF6347',
                     2:'#FF7F00',
                     - 2:'#FFD700',
                     3:'#FFFF00',
                     4:'#32CD32',
                     - 3:'#20B2AA',
                     5:'#6495ED',
                     - 4:'#8A2BE2',
                     6:'#D02090',
                     - 5:'#FF00FF',
                     7:'#FF1493',
                     - 6: '#FF6347',
                     8:'#FF0000',
                     9:'#FF7F00',
                     10:'#FFFF00',
                     11:'#32CD32'
                     }
# this is unique to the English notation system
# translators must modify this dictionary for each language
WHITE_KEY_NOTATION = {1:_('C'), 2:_('D'), 3:_('E'), 4:_('F'), 5:_('G'), 6:_('A'), 7:_('B'), 8:_('C')}
SHARP_NOTATION = {-1:_('C#'), -2:_('D#'), -3:_('F#'), -4:_('G#'), -5:_('A#')}
FLAT_NOTATION = {-1:_('Db'), -2:_('Eb'), -3:_('Gb'), -4:_('Ab'), -5:_('Bb')}
'''
Translators: note that you must write the translated note name matching the
given note name in the English notation
 For example, in many European countries the correct translations would be:
 C C♯ D D♯ E F F♯ G G♯ A B H C
'''
def getKeyNameFromID(numID, sharpNotation=True):
    '''
    get the name of the key that corresponds to the numID given

    optionally set sharpNotation = True for sharp notation, or
    sharpNotation = False for flat notation

    >>> getKeyNameFromID(1)
    C
    >>> getKeyNameFromID(-3, sharpNotation=True)
    F#
    >>> getKeyNameFromID(-5, sharpNotation=False)
    Bb
    '''
    if numID > 0:
        return WHITE_KEY_NOTATION[numID]
    elif sharpNotation:
        return SHARP_NOTATION[numID]
    else:
        return FLAT_NOTATION[numID]

def getIDFromKeyName(keyName):
    '''
    returns the numID of the note that corresponds to the keyName

    >>> getIDFromKeyName('C')
    1
    >>> getIDFromKeyName('D#')
    -2
    >>> getIDFromKeyName('Eb')
    -2
    '''
    for x, y in WHITE_KEY_NOTATION.items():
        if y == keyName or y.upper() == keyName.upper():
            return x
    for x, y in SHARP_NOTATION.items():
        if y == keyName or y.upper() == keyName.upper():
            return x
    for x, y in FLAT_NOTATION.items():
        if y == keyName or y.upper() == keyName.upper():
            return x

# ---------------------------------------------------------------------------
#
#  STAFF OBJECTS
#
# ---------------------------------------------------------------------------

class Staff():
    '''
    object to track staff and staff contents, such as notes, clefs, and stafflines
    contains a goocanvas.Group of all components
    '''

    def __init__(self, x, y, canvasRoot, numStaves):

      self.originalRoot = canvasRoot
      self.x = x        #master X position
      self.y = y        #master Y position
      self.rootitem = goocanvas.Group(parent=canvasRoot, x=0, y=0)

      # STAFF FORMATTING
      # ALL LOCATIONS BELOW ARE RELATIVE TO self.x and self.y
      self.endx = 400     #width of staff lines
      self.verticalDistanceBetweenStaves = 95 #vertical distance between musical staves
      self.staffLineSpacing = 13 #vertical distance between lines in staff
      self.staffLineThickness = 2.0 # thickness of staff lines
      self.numStaves = numStaves # number of staves to draw (1,2, or 3)

      # MUSIC NOTATION FORMATTING
      self.currentNoteXCoordinate = x + 30 #starting X position of first note
      self.initialNoteX = x + 30
      self.noteSpacingX = 27 #distance between each note when appended to staff
      self.dynamicNoteSpacing = False #adjust note spacing according to space needed by each note
      self.currentLineNum = 1    #the line number (1,2,3) you're currently writing notes to
      self.currentNoteType = 4 #the note type you're currently using to write to the
      # musical staff, could be 4 (quarter), 8 (eighth), 2 (half) or 1 (whole)

      # NOTE & PLAY FORMATTING
      self.colorCodeNotes = True # optionally set to False to mark all notes black
      self.labelBeatNumbers = False # label the beat numbers above each note
      # (used in play-rhythm activity)
      self.drawPlayingLine = False # draw vertical line on staff to follow the beat
      # as the comp370osition is being played

      self.notReadyToPlay = False #set to True when staff is not ready to
      #play composition (something else is going on for example)

      self.noteList = [] #list of note objects written to staff

      self.timers = [] #list of timers to use for playing, running pictures, etc.

      # PRIVATE ATTRIBUTES
      self._beatNumLabels = []
      self._staffImages = [] # to keep references to all the staff clefs put onto the page so we can delete just these when needed

    def drawStaff(self):
        '''
        draw the staff, including staff lines and staff clefs
        >>> self.newStaff = TrebleStaff(100, 80, self.rootitem, numStaves=4)
        >>> self.newStaff.drawStaff()
        '''
        self._drawStaffLines() #draw staff lines

    def _drawStaffLines(self):
        '''
        draw staff lines according to the number of staves
        '''
        y = self.y
        for staveNum in range(0, self.numStaves):
            self._drawLines(x=self.x, y=y, length=self.endx)
            y += self.verticalDistanceBetweenStaves

        self._drawEndBars() #two lines at end of the last staff

    def _drawLines(self, x, y, length):
        '''
        draw one set of five staff lines
        '''
        x1 = x
        lineLength = length
        x2 = x1 + lineLength
        y = y

        yWidth = self.staffLineSpacing
        t = self.staffLineThickness
        goocanvas.polyline_new_line(self.rootitem, x1, y, x2, y,
                                    stroke_color="black", line_width=t)
        y += yWidth
        goocanvas.polyline_new_line(self.rootitem, x1, y, x2, y,
                                    stroke_color="black", line_width=t)
        y += yWidth
        goocanvas.polyline_new_line(self.rootitem, x1, y, x2, y,
                                    stroke_color="black", line_width=t)
        y += yWidth
        goocanvas.polyline_new_line(self.rootitem, x1, y, x2, y,
                                    stroke_color="black", line_width=t)
        y += yWidth
        goocanvas.polyline_new_line(self.rootitem, x1, y, x2, y,
                                    stroke_color="black", line_width=t)
        self.bottomYLine = y

    def _drawEndBars(self):
        '''
        draw the vertical end bars on each line, two for line 3
        '''
        y = self.y
        for num in range(0, self.numStaves - 1):
            goocanvas.polyline_new_line(self.rootitem, self.endx + self.x,
                                    y, self.endx + self.x, y + 53,
                                     stroke_color="black", line_width=3.0)
            y += self.verticalDistanceBetweenStaves


        #doublebar
        goocanvas.polyline_new_line(self.rootitem, self.endx + self.x - 7,
                                y - 1, self.endx + self.x - 7, y + 53,
                                 stroke_color="black", line_width=3.0)

        #final barline, dark
        goocanvas.polyline_new_line(self.rootitem, self.endx + self.x,
                                y - 1, self.endx + self.x, y + 53,
                                 stroke_color="black", line_width=4.0)

    def drawNote(self, note):
        '''
        determine the correct x & y coordinate for the next note, and writes
        this note as an image to the staff. An alert is triggered if no more
        room is left on the staff. Also color-codes the note if self.colorCodeNotes == True

        >>> self.newStaff = TrebleStaff(50, 50, self.rootitem, numStaves=4)
        >>> self.newStaff.drawStaff()
        >>> self.newStaff.drawNote(QuarterNote(1, 'trebleClef', self.newStaff.rootitem))
        >>> self.newStaff.drawNote(EighthNote(5, 'trebleClef', self.newStaff.rootitem))
        >>> self.newStaff.drawNote(WholeNote(-3, 'trebleClef', self.newStaff.rootitem))
        '''

        if self.dynamicNoteSpacing:
            if note.numID < 0:
                self.noteSpacingX = 28
            elif note.noteType == 1:
                self.noteSpacingX = 25
            elif note.noteType == 2:
                self.noteSpacingX = 23
            else:
                self.noteSpacingX = 22
        x = self.getNoteXCoordinate(note) #returns next x coordinate for note,
        if x == False:
            try:
                self.alert.remove()
            except:
                pass
            self.alert = goocanvas.Text(
              parent=self.rootitem,
              x=self.endx - 100 + self.x,
              y=self.numStaves * self.verticalDistanceBetweenStaves + self.y,
              width=200,
              text=_("The staff is full. Please erase some notes"),
              fill_color="black",
              anchor=gtk.ANCHOR_CENTER,
              alignment=pango.ALIGN_CENTER
              )
            return
        y = self.getNoteYCoordinate(note) #returns the y coordinate based on note name
        self.currentLineNum = self.getLineNum(y) #updates self.lineNum

        note.draw(x, y) #draws note image on canvas
        if len(self.noteList) >= 1:
            if self.noteList[-1].noteType == 8 and self.noteList[-1].isTupleBound == False and note.noteType == 8: #if previous note and current note are eighth notes, draw duple
                self.drawTupleEighth(self.noteList[-1], note)
        if self.colorCodeNotes:
            note.colorCodeNote()
        self.noteList.append(note) #adds note object to staff list

        if self.labelBeatNumbers:
            x = note.x
            for n in note.beatNums:
                if n == note.beatNums[0]:
                    size = "10000"
                else:
                    size = "5000"
                blob = goocanvas.Text(
                parent=self.rootitem,
                x=x, #x + self.x,
                y=self.y - 13, #y - 75 ,
                text='<span size="' + size + '" >' + n + '</span>',
                fill_color="black",
                anchor=gtk.ANCHOR_CENTER,
                alignment=pango.ALIGN_CENTER,
                use_markup=True)
                self._beatNumLabels.append(blob)
                x += self.noteSpacingX / len(note.beatNums)

    def drawTupleEighth(self, note1, note2):
        # don't draw the duple if it's going to be crazy long (from one line to the next for example)
        if abs(note1.x - note2.x) > 40:
            return
        note1.flag.props.visibility = goocanvas.ITEM_INVISIBLE
        note2.flag.props.visibility = goocanvas.ITEM_INVISIBLE
        if (note1.numID > 8 and note2.numID > 8) or (note1.numID <= 8 and note2.numID <= 8) or (note1.numID > 8 and note2.numID < -5):
            if note2.numID <= 8 and note2.numID >= -5:
                x2 = note2.x + 7
                y2 = note2.y - 34
            if note1.numID <= 8:
                x1 = note1.x + 7
                y1 = note1.y - 34
            if note2.numID > 8 or note2.numID < -5:
                x2 = note2.x - 7
                y2 = note2.y + 34
            if note1.numID > 8:
                x1 = note1.x - 7
                y1 = note1.y + 34
        else:
            x2 = note2.x + 7
            y2 = note2.y - 34
            x1 = note1.x + 7
            y1 = note1.y - 34
            if note2.numID > 8 or note2.numID < -5:
                goocanvas.Item.rotate(note2.noteHead, 180, note2.x, note2.y)
            if note1.numID > 8:
                goocanvas.Item.rotate(note1.noteHead, 180, note1.x, note1.y)
        note1.tupleBar = goocanvas.polyline_new_line(note1.rootitem,
                                x1, y1, x2, y2,
                                stroke_color_rgba=0x121212D0, line_width=4)

        note1.isTupleBound = True
        note2.isTupleBound = True

    def drawTwoSingleEighthNotes(self, note1, note2):
        note1.flag.props.visibility = goocanvas.ITEM_VISIBLE
        note2.flag.props.visibility = goocanvas.ITEM_VISIBLE
        if note1.isTupleBound:
            note1.tupleBar.props.visibility = goocanvas.ITEM_INVISIBLE
        note1.isTupleBound = False
        note2.isTupleBound = False


    def writeLabel(self, text, note):
        '''
        writes the text below the note, such as labeling the note name,
        in color-code if self.colorCodeNotes = True

        >>> self.newStaff = TrebleStaff(50, 50, self.rootitem, numStaves=1)
        >>> self.newStaff.endx = 200
        >>> self.newStaff.rootitem.scale(2.0, 2.0)
        >>> self.newStaff.drawStaff()
        >>> n2 = HalfNote(5, 'trebleClef', self.newStaff.rootitem)
        >>> self.newStaff.drawNote(n2)
        >>> self.newStaff.writeLabel('G', n2)
        '''
        if self.colorCodeNotes:
            color = NOTE_COLOR_SCHEME[note.numID]

        if note in self.noteList:
            text = goocanvas.Text(
                  parent=self.rootitem,
                  x=note.x,
                  y=self.y + self.staffLineSpacing * 6 + 3,
                  text=text,
                  fill_color='black',
                  anchor=gtk.ANCHOR_CENTER,
                  alignment=pango.ALIGN_CENTER
                  )
            rect = goocanvas.Rect(parent=self.rootitem,
                      x=note.x - 10,
                      y=self.y + self.staffLineSpacing * 6 - 8,
                      width=20,
                      height=20,
                      line_width=.5,
                      fill_color=color)
            text.raise_(rect)

    def eraseOneNote(self, widget=None, target=None, event=None):
        '''
        removes the last note in the staff's noteList, updates self.currentLineNumif
        necessary, and updates self.currentNoteXCoordinate

        TODO: bug: doesn't remove the label on the note if it has one (easy fix, maybe later)

        >>> self.newStaff.eraseOneNote()
        '''
        try:
            self.alert.remove()
        except:
            pass

        if len(self.noteList) > 1:
            self.currentNoteXCoordinate = self.noteList[-2].x
            remainingNoteY = self.noteList[-2].y
            self.currentLineNum = self.getLineNum(remainingNoteY)
            if len(self.noteList) >= 2:
                if self.noteList[-1].noteType == 8 and self.noteList[-1].isTupleBound and self.noteList[-2].noteType == 8:
                    self.drawTwoSingleEighthNotes(self.noteList[-2], self.noteList[-1])
            self.noteList[-1].remove()
            self.noteList.pop()
        else:
            self.eraseAllNotes()

    def eraseAllNotes(self, widget=None, target=None, event=None, ask_user=False):
        '''
        remove all notes from staff, deleting them from self.noteList, and
        restores self.currentLineNumto 1 and self.currentNoteXCoordinate to the
        starting position

        >>> self.newStaff.eraseAllNotes()
        '''
        for o in self._beatNumLabels:
            o.remove()
        for n in self.noteList:
          n.remove()
        self.currentNoteXCoordinate = self.initialNoteX
        self.noteList = []
        self.currentLineNum = 1

        try:
            self.alert.remove()
        except:
            pass
        if hasattr(self, 'texts'):
            for x in self.texts:
                x.remove()

    def clear(self):
        '''
        removes and erases all notes and clefs on staff (in preparation for
        a clef change)
        '''
        self.eraseAllNotes()
        for s in self._staffImages:
            s.remove()
        self._staffImages = []

        if hasattr(self, 'noteText'):
            self.noteText.remove()

    def setClefVisibility(self,visibile=True):
        if visibile == True:
            for o in self._staffImages:
                o.props.visibility = goocanvas.ITEM_VISIBLE
        else:
            for o in self._staffImages:
                o.props.visibility = goocanvas.ITEM_INVISIBLE
    def play_it(self, noteIndexToPlay, playingLineOnly=False):
        '''
        NOT A PUBLIC METHOD

        called to play one note. Checks to see if all notes have been played
        if not, establishes a timer for the next note depending on that note's
        duration (quarter, half, whole)
        colors the note white that it is currently sounding
        '''

        if noteIndexToPlay >= len(self.noteList):
            if hasattr(self, 'verticalPlayLine'):
                 self.verticalPlayLine.remove()
                 self.notReadyToPlay = False
            return

        note = self.noteList[noteIndexToPlay]

        if hasattr(self, 'verticalPlayLine'):
            self.verticalPlayLine.remove()

        if playingLineOnly == True or self.drawPlayingLine:
            self.verticalPlayLine = goocanvas.polyline_new_line(self.rootitem,
                                note.x, note.y, note.x, note.y - 50,
                                stroke_color_rgba=0x121212D0, line_width=2)

            self.verticalPlayLine.animate(self.noteSpacingX, 0, 1.0, 0.0, \
                absolute=False, duration=note.millisecs, step_time=50, type=goocanvas.ANIMATE_FREEZE)

        if not playingLineOnly:
            note.play()

        self.timers.append(
                           gobject.timeout_add(
                                               self.noteList[noteIndexToPlay].millisecs,
                                               self.play_it, noteIndexToPlay + 1, playingLineOnly))


    def playComposition(self, widget=None, target=None, event=None, playingLineOnly=False):
        '''
        plays entire composition. establish timers, one per note, called after
        different durations according to noteType. Only way to stop playback after
        calling this method and during play is self.eraseAllNotes()
        >>> self.newStaff.playComposition()
        '''

        #if not self.noteList or self.notReadyToPlay:
        #    return
        self.notReadyToPlay = True

        self.timers = []
        self.currentNoteIndex = 0
        self.play_it(0, playingLineOnly)
        self.timers.append(gobject.timeout_add(self.noteList[self.currentNoteIndex].millisecs,
                                            self.play_it, (self.currentNoteIndex + 1), playingLineOnly))

    def file_to_staff(self, filename):
        '''
        open text file, read contents and write to staff
        '''

        file = open(filename, 'rb')
        self.clear()
        self.stringToNotation(file.read())

    def staff_to_file(self, filename):
        '''
        convert staff to notation, write to text file, save to MyGCompris folder
        '''
        file = open(filename , 'wb')
        file.write(self.staffName + ' ')
        for note in self.noteList:
            if note.numID == 8:
                name = '2C'
            else:
                name = getKeyNameFromID(note.numID)
            file.write(name + str(note.noteType) + ' ')

        file.close()

    def stringToNotation(self, melodyString):
        '''
        parse the melody string and write the notes to the staff

        the melody must be in a very simple format. It is one line, and begins
        the clef, either 'trebleClef' or 'bassClef' Then, each following note is
        seperated with a space and the note name (English system, sharp=#,flat=b,
        C (second octave is 2C rather than just C) written first
        followed by the note duration (8=eighth,4=quarter,2=half,1=whole)

        self.staff1 = TrebleStaff(50, 30, self.rootitem, numStaves=1)
        self.staff1.endx = 200
        self.staff1.rootitem.scale(2.0, 2.0)
        self.staff1.drawStaff()
        self.staff1.stringToNotation('trebleClef C4 G2 F#4 Ab4 2C2')

        self.staff2 = BassStaff(50, 130, self.rootitem, numStaves=1)
        self.staff2.endx = 200
        self.staff2.rootitem.scale(2.0, 2.0)
        self.staff2.drawStaff()
        self.staff2.stringToNotation('bassClef Eb2 F4 C#8 Bb4 C2')
        '''
        if hasattr(self, 'newClef'):
            self.newClef.clear()

        self.eraseAllNotes()
        self.clear()
        keys = melodyString.split()
        staffName = keys[0]
        if staffName == 'trebleClef':
            self.newClef = TrebleStaff(self.x, self.y, self.originalRoot, self.numStaves)
            self.newClef._drawClefs()
            #self.newClef.rootitem.scale(2.0, 2.0) TODO: bug: if user scales staff, then calls
            # this method the new clefs will not scale as well...fix later
        elif staffName == 'bassClef':
            self.newClef = BassStaff(self.x, self.y, self.originalRoot, self.numStaves)
            self.newClef._drawClefs()
        else:
            print "please use trebleClef or bassClef"
            return
        self.positionDict = self.newClef.positionDict
        for exp in keys[1:]:
            duration = int(exp[-1])
            n = exp[0:-1]
            # these are added rather hakishly just to accommodate some new melodies
            # it's easy to improve this code...I just don't have time now
            if n == '2C':
                numID = 8
            elif n == '2D':
                numID = 9
            elif n == '2C#':
                numID = -6
            elif n == '2E':
                numID = 10
            elif n == '2F':
                numID = 11
            else:
                numID = getIDFromKeyName(str(exp[0:-1]))
                if not numID:
                    print 'ERROR: unable to parse note', exp
                    return
            if duration == 4:
                note = QuarterNote(numID, staffName, self.rootitem)
            elif duration == 2:
                note = HalfNote(numID, staffName, self.rootitem)
            elif duration == 1:
                note = WholeNote(numID, staffName, self.rootitem)
            elif duration == 8:
                note = EighthNote(numID, staffName, self.rootitem)
            else:
                print 'ERROR: unable to parse note', exp
                return
            if '#' in n:
                note.sharpNotation = True
            if 'b' in n:
                note.sharpNotation = False
            self.drawNote(note)

    def getLineNum(self, Ycoordinate):
        '''
        given the Ycoordinate, returns the correct lineNum (1,2,etc.)
        '''

        return ((Ycoordinate - self.y + 20) / self.verticalDistanceBetweenStaves) + 1

    def getNoteXCoordinate(self, note):
        '''
        determines the x coordinate of the next note to be written to the
        staff, with consideration for the maximum staff line length.
        Increments self.currentLineNumand sets self.currentNoteXCoordinate
        '''
        self.currentNoteXCoordinate += self.noteSpacingX

        if self.currentNoteXCoordinate >= (self.endx + self.x - 15) or (self.currentNoteType == 8 and
                                (self.currentNoteXCoordinate >= (self.endx + self.x + -25))):
            if self.currentLineNum == 3:
                #NO MORE STAFF LEFT!
                return False
            else:
                if note.numID < 0:
                    self.currentNoteXCoordinate = self.initialNoteX + 20
                else:
                    self.currentNoteXCoordinate = self.initialNoteX + 20
                self.currentLineNum += 1

        return self.currentNoteXCoordinate

    def getNoteYCoordinate(self, note):
        '''
        return a note's vertical coordinate based on the note's name. This is
        unique to each type of clef (different for bass and treble)
        '''

        yoffset = (self.currentLineNum - 1) * self.verticalDistanceBetweenStaves
        if note.numID < 0 and note.sharpNotation:
            numID = {-1:1, -2:2, -3:4, -4:5, -5:6, -6:8}[note.numID]
        elif note.numID < 0:
            numID = {-1:2, -2:3, -3:5, -4:6, -5:7, -6:9}[note.numID]
        else:
            numID = note.numID

        return  self.positionDict[numID] + yoffset + 36 + self.y

    def drawScale(self, scaleName, includeNoteNames=True):
        '''
        draw the scale on the staff, optionally including Note Names

        >>> self.staff2 = TrebleStaff(50, 50, self.rootitem, numStaves=1)
        >>> self.staff2.endx = 300
        >>> self.staff2.rootitem.scale(2.0, 2.0)
        >>> self.staff2.drawStaff()
        >>> self.staff2.drawScale('C Major')
        '''
        if scaleName == 'C Major':
            numIDs = [1, 2, 3, 4, 5, 6, 7, 8]
            # TODO:
            # good luck with the rest of the scales...all them exceed the C octave,
            # so you'll need to add in a feature to go beyond just 8 notes ;-)
        for id in numIDs:
            note = QuarterNote(id, self.staffName, self.rootitem)
            self.drawNote(note)
            if includeNoteNames:
                text = getKeyNameFromID(note.numID)
                self.writeLabel(text, note)
            note.enablePlayOnClick()

    def colorCodeAllNotes(self):
        '''
        color notes according to NOTE_COLOR_SCHEME

        self.newStaff.colorCodeAllNotes()
        '''
        for note in self.noteList:
            note.colorCodeNote()

    def colorAllNotes(self, color):
        '''
        color all notes a certain color ('black', 'red', 'blue', etc.)

        self.newStaff.colorAllNotes('black')
        '''
        for note in self.noteList:
            note.color(color)

    # ---------- Not - documented ------------------------ #
    def sound_played(self, file):
        pass #mandatory method

    def drawFocusRect(self, x, y, eighth=False):
        '''
        draws focus rectangle around notes (quarter/half/whole) in piano_composition game
        '''
        if hasattr(self, 'focusRect'):
            self.focusRect.remove()
        if eighth:
            width = 34
            height = 51
            radius_x = 9
            radius_y = 9
        else:
            width = 26
            height = 51

        self.focusRect = goocanvas.Rect(parent=self.rootitem,
                                    x=x,
                                    y=y,
                                    width=width, height=height,
                                    radius_x=9, radius_y=9,
                                    stroke_color="black", line_width=2.0)
    #update current note type based on button clicks
    def updateToEighth(self, widget=None, target=None, event=None):
        self.currentNoteType = 8
        self.drawFocusRect(256.5, 112, True)

    def updateToQuarter(self, widget=None, target=None, event=None):
        self.currentNoteType = 4
        self.drawFocusRect(292, 112)
    def updateToHalf(self, widget=None, target=None, event=None):
        self.currentNoteType = 2
        self.drawFocusRect(319.5, 112)
    def updateToWhole(self, widget=None, target=None, event=None):
        self.currentNoteType = 1
        self.drawFocusRect(347, 112)


class TrebleStaff(Staff):
    '''
    unique type of Staff with clef type specified and certain dimensional
    conventions maintained for certain notes
    '''
    def __init__(self, x, y, canvasRoot, numStaves=3):
        Staff.__init__(self, x, y, canvasRoot, numStaves)

        self.staffName = 'trebleClef'

         # for use in getNoteYCoordinateMethod
        self.positionDict = {1:26, 2:22, 3:16, 4:9, 5:3,
                        6:-4, 7:-10, 8:-17, 9:-23, 10:-29,11:-35}

    def drawStaff(self):
        self._drawClefs()
        Staff.drawStaff(self)

    def _drawClefs(self):
        '''
        draw all three clefs on canvas
        '''
        h = 65
        w = 30
        y = self.y
        for staveNum in range(0, self.numStaves):
            self._staffImages.append(goocanvas.Image(
                parent=self.rootitem,
                x=3 + self.x,
                y= -2 + y,
                height=h,
                width=w,
                pixbuf=gcompris.utils.load_pixmap('piano_composition/trebleClef.png')
                ))
            y += self.verticalDistanceBetweenStaves

class BassStaff(Staff):
    '''
    unique type of Staff with clef type specified and certain dimensional
    conventions maintained for certain notes
    '''
    def __init__(self, x, y, canvasRoot, numStaves=3):
        Staff.__init__(self, x, y, canvasRoot, numStaves)

        self.staffName = 'bassClef'

        # for use in getNoteYCoordinateMethod
        self.positionDict = {1:-4, 2:-11, 3:-17, 4:-24, 5:-30,
                        6:-36, 7:-42, 8:-48, 9:-52, 10:-58, 11:-64}

    def drawStaff(self, text=None):
        self._drawClefs()
        Staff.drawStaff(self)


    def _drawClefs(self):
        '''
        draw all three clefs on canvas
        '''
        h = 40
        w = 30
        y = self.y
        for staveNum in range(self.numStaves):
            if self.numStaves >= 1:
                self._staffImages.append(goocanvas.Image(
                    parent=self.rootitem,
                    x=6 + self.x,
                    y=y + 1,
                    height=h,
                    width=w,
                    pixbuf=gcompris.utils.load_pixmap('piano_composition/bassClef.png')
                    ))
            y += self.verticalDistanceBetweenStaves

# ---------------------------------------------------------------------------
#
#  NOTE OBJECTS
#
# ---------------------------------------------------------------------------


class Note():
    '''
    an object representation of a note object, containing the goocanvas image
    item as well as several instance variables to aid with identification
    '''
    def __init__(self, numID, staffType, rootitem, sharpNotation=True):
        self.numID = numID

        self.staffType = staffType #'trebleClef' or 'bassClef'

        self.x = 0
        self.y = 0
        self.rootitem = goocanvas.Group(parent=rootitem, x=self.x, y=self.y)

        self.silent = False #make note silent always?

        self.pitchDir = self._getPitchDir()

        self.timers = []
        self.sharpNotation = sharpNotation # toggle to switch note between sharp notation
        # and flat notation, if applicable

    def play(self, widget=None, target=None, event=None):
        '''
        plays the note pitch. Each pitch is stored in the resources
        folder as an .ogg file (these are not synthesized)
        '''
        # sometimes this method is called without actually having fa note
        # printed on the page (we just want to hear the pitch). Thus, only
        # highlight a note if it exists!
        if hasattr(self, 'playingLine'):
            self.highlight()
        gcompris.sound.play_ogg(self._getPitchDir())

    def color(self, color):
        '''
        default color method. colors all components, including notehead's fill
        '''
        self.colorNoteHead(color)

    def colorCodeNote(self):
        '''
        color the note the appropriate color based on the given color scheme
        '''
        self.color(NOTE_COLOR_SCHEME[self.numID])

    def colorNoteHead(self, color, fill=True, outline=True): # not documented online
        '''
        colors the notehead, by default both the fill and the outline
        '''
        if fill:
            self.noteHead.props.fill_color = color
        if outline:
            self.noteHead.props.stroke_color = "black"

        if hasattr(self, 'midLine'):
            self.midLine.props.fill_color = "black"
            self.midLine.props.stroke_color = "black"

    def enablePlayOnClick(self):
        '''
        enables the function that the note will be played when the user clicks
        on the note
        '''
        if hasattr(self, 'noteHead'):
            self.noteHead.connect("button_press_event", self.play)
            gcompris.utils.item_focus_init(self.noteHead, None)
        self.silent = False

    def disablePlayOnClick(self):
        self.silent = True

    def highlight(self):
        '''
        highlight the note for 700 milliseconds, then revert
        '''
        self.playingLine.props.visibility = goocanvas.ITEM_VISIBLE
        self.timers.append(gobject.timeout_add(self.millisecs, self.stopHighLight))

    def stopHighLight(self): # not documented online
        self.playingLine.props.visibility = goocanvas.ITEM_INVISIBLE

    def remove(self):
        '''
        removes the note from the canvas
        '''
        self.rootitem.remove()

    def drawPictureFocus(self, x, y):
        '''
        draw focus background picture (when note is played)
        '''
        self.playingLine = goocanvas.Image(
              parent=self.rootitem,
              pixbuf=gcompris.utils.load_pixmap("piano_composition/note_highlight.png"),
              x=x - 13,
              y=y - 15,
              )
        self.playingLine.props.visibility = goocanvas.ITEM_INVISIBLE

    def _getPitchDir(self):
        '''
        uses the note's raw name to find the pitch directory associate to it.
        Since only sharp pitches are stored, method finds the enharmonic name
        to flat notes using the circle of fifths dictionary
        '''

        if self.staffType == 'trebleClef':
             pitchDir = 'piano_composition/treble_pitches/' + str(self.noteType) + '/' + str(self.numID) + '.wav'
        else:
             pitchDir = 'piano_composition/bass_pitches/' + str(self.noteType) + '/' + str(self.numID) + '.wav'

        return pitchDir

    def _drawMidLine(self, x, y):
        if self.staffType == 'trebleClef' and (self.numID == 1 or  (self.numID == -1 and self.sharpNotation)) or \
           (self.staffType == 'bassClef' and (self.numID == 1 or self.numID == 8)) :
            self.midLine = goocanvas.polyline_new_line(self.rootitem, x - 9, y, x + 9, y ,
                                        stroke_color_rgba=0x121212D0, line_width=1, pointer_events="GOO_CANVAS_EVENTS_NONE")

    def _drawAlteration(self, x, y):
        '''
        draws a flat or a sharp sign in front of the note if needed
        width and height specifications needed because these images
        need to be so small that scaling any larger image to the correct
        size makes them extremely blury.
        '''
        if self.numID < 0:
            if self.sharpNotation:
                self.alteration = goocanvas.Image(
                  parent=self.rootitem,
                  pixbuf=gcompris.utils.load_pixmap("piano_composition/blacksharp.png"),
                  x=x - 23,
                  y=y - 9,
                  width=18,
                  height=18
                  )
            else:
                self.alteration = goocanvas.Image(
                  parent=self.rootitem,
                  pixbuf=gcompris.utils.load_pixmap("piano_composition/blackflat.png"),
                  x=x - 23,
                  y=y - 14,
                  width=20,
                  height=20,
                  )

class EighthNote(Note):
    '''
    an object inherited from Note, of specific duration (eighth length)
    '''
    noteType = 8
    beatNums = ['+']
    millisecs = 250
    isTupleBound = False # by default the eight appears with a flag
    def draw(self, x, y):
        '''
        places note image in canvas at x,y
        '''
        self.drawPictureFocus(x, y)

        # Thanks to Olivier Samyn for the note shape
        self.noteHead = goocanvas.Path(parent=self.rootitem,
            data="m %i %i a7,5 0 0,1 12,-3.5 v-32 h2 v35 a7,5 0 0,1 -14,0z" % (x - 7, y),
            fill_color='black',
            stroke_color='black',
            line_width=1.0
            )
        self.noteHead.raise_(None)
        self._drawAlteration(x, y)

        self._drawMidLine(x, y)

        self._drawFlag(x, y)

        self.y = y
        self.x = x

        if self.numID > 8 or self.numID < -6:
            goocanvas.Item.rotate(self.noteHead, 180, x, y)
            goocanvas.Item.rotate(self.flag, 180, x, y)
    def _drawFlag(self, x, y):
        self.flag = goocanvas.Image(
          parent=self.rootitem,
          pixbuf=gcompris.utils.load_pixmap("piano_composition/flag.png"),
          x=x + 7,
          y=y - 37,
          height=30,
          width=10
          )

class QuarterNote(Note):
    '''
    an object inherited from Note, of specific duration (quarter length)
    '''
    noteType = 4
    beatNums = ['1']
    millisecs = 500

    def draw(self, x, y):
        '''
        places note image in canvas
        '''

        self.drawPictureFocus(x, y)

        # Thanks to Olivier Samyn for the note shape
        self.noteHead = goocanvas.Path(parent=self.rootitem,
            data="m %i %i a7,5 0 0,1 12,-3.5 v-32 h2 v35 a7,5 0 0,1 -14,0z" % (x - 7, y),
            fill_color='black',
            stroke_color='black',
            line_width=1.0
            )
        self.noteHead.raise_(None)
        self._drawAlteration(x, y)

        self._drawMidLine(x, y)

        self.y = y
        self.x = x

        if self.numID > 8:
            goocanvas.Item.rotate(self.noteHead, 180, x, y)

class HalfNote(Note):
    '''
    an object inherited from Note, of specific duration (half length)
    '''
    noteType = 2
    beatNums = ['1', '2']
    millisecs = 1000

    def draw(self, x, y):
        '''
        places note image in canvas
        '''
        self.drawPictureFocus(x, y)

        # Thanks to Olivier Samyn for the note shape
        self.noteHead = goocanvas.Path(parent=self.rootitem,
            data="m %i %i a7,5 0 0,1 12,-3.5 v-32 h2 v35 a7,5 0 0,1 -14,0 z m 3,0 a 4,2 0 0 0 8,0 4,2 0 1 0 -8,0 z" % (x - 7, y),
            fill_color='black',
            stroke_color='black',
            line_width=1.0
            )
        self.noteHead.raise_(None)
        self._drawAlteration(x, y)

        self._drawMidLine(x, y)

        self.y = y
        self.x = x

        if self.numID > 8:
            goocanvas.Item.rotate(self.noteHead, 180, x, y)

class WholeNote(Note):
    '''
    an object inherited from Note, of specific duration (whole length)
    '''
    noteType = 1
    beatNums = ['1', '2', '3', '4']
    millisecs = 2000

    def draw(self, x, y):
        self.drawPictureFocus(x, y)

        # Thanks to Olivier Samyn for the note shape
        self.noteHead = goocanvas.Path(parent=self.rootitem,
            data="m %i %i a 7,5 0 1 1 14,0 7,5 0 0 1 -14,0 z m 3,0 a 4,2 0 0 0 8,0 a 4,2 0 1 0 -8,0 z" % (x - 7, y),
            fill_color='black',
            stroke_color='black',
            line_width=1.0
            )
        self.noteHead.raise_(None)
        self._drawAlteration(x, y)

        self._drawMidLine(x, y)

        self.y = y
        self.x = x

# ---------------------------------------------------------------------------
#
#  PIANO KEYBOARD
#
# ---------------------------------------------------------------------------

class PianoKeyboard():
    '''
    object representing the one-octave piano keyboard
    '''
    def __init__(self, x, y, canvasroot):
        self.rootitem = goocanvas.Group(parent=canvasroot, x=0, y=0)
        self.x = x
        self.y = y
        self.blackKeys = False # display black keys with buttons?
        self.sharpNotation = True # use sharp notation for notes, (#)
        # if False, use flat notation (b)
        self.whiteKeys = True # display white keys with buttons?

        self.colors = NOTE_COLOR_SCHEME #provide this as an instance
        # variable so future users may edit it
        self.sharpBlackKeyTexts = []
        self.flatBlackKeyTexts = []

    def draw(self, width, height, key_callback):
        '''
        create piano keyboard, with buttons for keys. Draw the keyboard
        using the width and height, and connect the key buttons to the key_callback
        method

        >>> def keyboard_button_press(self, widget=None, target=None, event=None):
        ...     pass
        >>> p = PianoKeyboard(50, 50, self.rootitem)
        >>> p.draw(300, 200, keyboard_button_press)
        '''
        self.width = width
        self.height = height
        self.key_callback = key_callback
        #piano keyboard image
        self.image = goocanvas.Image(
          parent=self.rootitem,
          pixbuf=gcompris.utils.load_pixmap("piano_composition/keyboard.png"),
          x=self.x,
          y=self.y,
          height=height,
          width=width
          )

        '''
        define colored rectangles to lay on top of piano keys for student to click on

        '''

        self.key_callback = key_callback
        w = width * 0.09
        h = height * 0.17
        y = self.y + 0.81 * height
        x = self.x + width * .02
        seperationWidth = w * 1.37

        if self.whiteKeys:
            for num in range(1, 9):
                self.drawKey(x, y, w, h, self.colors[num], num)
                x += seperationWidth


        if self.blackKeys:
            w = width * 0.07
            h = height * 0.15
            y = self.y + 0.6 * height
            x = self.x + width * .089
            seperationWidth = w * 1.780

            self.drawKey(x, y, w, h, self.colors[-1], -1)
            x += seperationWidth
            self.drawKey(x, y, w, h, self.colors[-2], -2)
            x += seperationWidth * 2
            self.drawKey(x, y, w, h, self.colors[-3], -3)
            x += seperationWidth
            self.drawKey(x, y, w, h, self.colors[-4], -4)
            x += seperationWidth
            self.drawKey(x, y, w, h, self.colors[-5], -5)





    def drawKey(self, x, y, width, height, color, numID):
        '''
        This function displays the clickable part of the key
        '''
        item = goocanvas.Rect(parent=self.rootitem, x=x, y=y,
                              width=width, height=height,
                              stroke_color="black", fill_color=color,
                              line_width=1.0)
        item.numID = numID

        if numID < 0:
            size = "11000"
            offset = 10
        else:
            size = "15000"
            offset = 13

        keyText = goocanvas.Text(
         parent=self.rootitem,
         x=x + offset,
         y=y + 18,
         width=10,
         text='<span font_family="URW Gothic L" size="' + size + '" weight="bold">' + getKeyNameFromID(numID, self.sharpNotation) + '</span>',
         fill_color="black",
         anchor=gtk.ANCHOR_CENTER,
         alignment=pango.ALIGN_CENTER,
         use_markup=True,
         pointer_events="GOO_CANVAS_EVENTS_NONE"
         )
        if numID < 0:
            if self.sharpNotation == True:
                self.sharpBlackKeyTexts.append(keyText)
            elif self.sharpNotation == False:
                self.flatBlackKeyTexts.append(keyText)
        gcompris.utils.item_focus_init(keyText, item)

        '''
        connect the piano keyboard rectangles to a button press event,
        the method keyboard_click
        '''
        item.connect("button_press_event", self.key_callback)
        gcompris.utils.item_focus_init(item, None)
        return item

    def changeAccidentalType(self, sharpNotation=False):
        if sharpNotation:
            l = self.flatBlackKeyTexts
        else:
            l = self.sharpBlackKeyTexts
        for x in l:
            x.remove()
        self.draw(self.width, self.height, self.key_callback)

# ---------------------------------------------------------------------------
#
# General UTILITY FUNCTIONS
#
# ---------------------------------------------------------------------------

def textButton(x, y, text, self, color='gray', width=100000, includeText=False):
    '''
    Add a text button to the screen with the following parameters:
    1. x: the x position of the button
    2. y: the y position of the button
    3. text: the text of the button
    4. self: the self object this button is to be written to (just pass 'self')
    5. color: the color of button you'd like to use. Unfortunately there
    are limited button colors available. I am not a designer, so you are welcome
    to improve this method, but the current colors available are listed below in the examples
    6. width: the width of the button

    textButton(200, 300, 'Hello World!', self, color='brown')
    textButton(350, 300, 'Hola', self, color='darkpurple')
    textButton(500, 300, 'Ciao', self, color='gray')
    textButton(650, 300, 'Bonjour', self, color='green')
    textButton(200, 400, 'Guten Tag', self, color='purple')
    textButton(350, 400, 'Nei Ho', self, color='red')
    textButton(500, 400, 'Zdravstvuyte', self, color='teal', width=70)
    '''
    self.textbox = goocanvas.Text(
        parent=self.rootitem,
        x=x, y=y,
        width=width,
        text=text,
        fill_color="white", anchor=gtk.ANCHOR_CENTER,
        alignment=pango.ALIGN_CENTER,
        pointer_events="GOO_CANVAS_EVENTS_NONE"
        )
    TG = 15
    bounds = self.textbox.get_bounds()

    img = goocanvas.Image(
            parent=self.rootitem,
            x=bounds.x1 - TG,
            y=bounds.y1 - TG,
            height=bounds.y2 - bounds.y1 + TG * 2,
            width=bounds.x2 - bounds.x1 + TG * 2,
            pixbuf=gcompris.utils.load_pixmap('piano_composition/buttons/' + color + '.png')
            )

    gcompris.utils.item_focus_init(img, None)
    self.textbox.raise_(img)
    if includeText:
        return img, self.textbox
    else:
        return img

def textBox(text, x, y , self, width=10000, fill_color=None, stroke_color=None, noRect=False, text_color="black"):
    '''
    write a textbox with text to the screen. By default the text is surrounded with a rectangle.
    Customize with the following parameters:
    text: the text to write
    x: the x position of the text
    y: the y position of the text
    self: the self object this text is to be written to (just pass 'self')
    width: the width limit of the text
    fill_color: the color to fill the rectangle
    stroke_color: the color to make the rectangle lines
    noRect: set to true for no rectangle to be drawn
    text_color: the color of the text

    accepted colors include string html color tags or english names

    textBox('Hello World!', 200, 300, self)
    textBox('Hola', 350, 300, self, fill_color='green')
    textBox('Ciao', 500, 300, self, stroke_color='pink')
    textBox('Bonjour', 650, 300, self, noRect=True)
    textBox('Nei Ho', 350, 400, self, text_color='red')
    textBox('Guten Tag', 200, 400, self, width=10)
    textBox('Zdravstvuyte', 500, 400, self, fill_color='#FF00FF')
    '''
    text = goocanvas.Text(
        parent=self.rootitem, x=x, y=y, width=width,
        text=text,
        fill_color=text_color, anchor=gtk.ANCHOR_CENTER,
        alignment=pango.ALIGN_CENTER,

        )
    TG = 10
    bounds = text.get_bounds()
    if not noRect:
        rect = goocanvas.Rect(parent=self.rootitem,
                              x=bounds.x1 - TG,
                              y=bounds.y1 - TG,
                              width=bounds.x2 - bounds.x1 + TG * 2,
                              height=bounds.y2 - bounds.y1 + TG * 2,
                              line_width=3.0)
        if fill_color:
            rect.props.fill_color = fill_color
        if stroke_color:
            rect.props.stroke_color = stroke_color
        text.raise_(rect)
        return text, rect
    return text

def clearResponsePic(self):
    self.responsePic.remove()

def displayHappyNote(self, nextMethod):
    '''
    displays the happy note for 900 milliseconds
    '''

    if hasattr(self, 'responsePic'):
        self.responsePic.remove()
    if not hasattr(self, 'timers'):
        self.timers = []
    gcompris.sound.play_ogg("/piano_composition/bonus.wav")
    self.responsePic = goocanvas.Image(
    parent=self.rootitem,
    pixbuf=gcompris.utils.load_pixmap('piano_composition/happyNote.png'),
    x=300,
    y=100,
    height=300,
    width=150
    )

    #self.responsePic.raise_(None)
    self.timers.append(gobject.timeout_add(900, clearResponsePic, self))
    self.timers.append(gobject.timeout_add(910, nextMethod))


def displaySadNote(self, nextMethod):
    '''
    displays the sad note for 900 milliseconds
    '''

    if hasattr(self, 'responsePic'):
        self.responsePic.remove()
    if not hasattr(self, 'timers'):
        self.timers = []
    self.responsePic = goocanvas.Image(
    parent=self.rootitem,
    pixbuf=gcompris.utils.load_pixmap('piano_composition/sadNote.png'),
    x=300,
    y=100,
    height=300,
    width=150
    )
    gcompris.sound.play_ogg("/piano_composition/bleep.wav")
    self.responsePic.raise_(None)
    self.timers.append(gobject.timeout_add(900, clearResponsePic, self))
    self.timers.append(gobject.timeout_add(910, nextMethod))


def pianokeyBindings(keyval, self):
    '''
    nice key bindings for the piano keys
    In your activity's key_press method, call this method and pass in the keyval and self

    def key_press(self, keyval, commit_str, preedit_str):

        utf8char = gtk.gdk.keyval_to_unicode(keyval)
        pianokeyBindings(keyval, self)
    '''

    if keyval == 49:
        self.keyboard_click(None, None, None, 1)
    elif keyval == 50:
        self.keyboard_click(None, None, None, 2)
    elif keyval == 51:
        self.keyboard_click(None, None, None, 3)
    elif keyval == 52:
        self.keyboard_click(None, None, None, 4)
    elif keyval == 53:
        self.keyboard_click(None, None, None, 5)
    elif keyval == 54:
        self.keyboard_click(None, None, None, 6)
    elif keyval == 55:
        self.keyboard_click(None, None, None, 7)
    elif keyval == 56:
        self.keyboard_click(None, None, None, 8)
    elif keyval == gtk.keysyms.F1:
        self.keyboard_click(None, None, None, -1)
    elif keyval == gtk.keysyms.F2:
        self.keyboard_click(None, None, None, -2)
    elif keyval == gtk.keysyms.F3:
        self.keyboard_click(None, None, None, -3)
    elif keyval == gtk.keysyms.F4:
        self.keyboard_click(None, None, None, -4)
    elif keyval == gtk.keysyms.F5:
        self.keyboard_click(None, None, None, -5)

def askUser(x, y, self):
    self.text = goocanvas.Text(
            parent=self.rootitem, x=x, y=y, width=1000,
            text='<span size="30000" > Erase All? </span>',
            fill_color='black', anchor=gtk.ANCHOR_CENTER,
            alignment=pango.ALIGN_CENTER,
            use_markup=True)
    TG = 10
    bounds = self.text.get_bounds()

    self.rect = goocanvas.Rect(parent=self.rootitem,
                          x=bounds.x1 - TG,
                          y=bounds.y1 - TG,
                          width=bounds.x2 - bounds.x1 + TG * 2,
                          height=bounds.y2 - bounds.y1 + TG * 2,
                          line_width=3.0,
                          fill_color='gray')
    self.rect.raise_(None)
    self.text.raise_(None)
    self.yesButton, self.txt1 = textButton(x - 50, y + 80, _('Yes'), self, color='green', includeText=True)
    self.noButton, self.txt2 = textButton(x + 50, y + 80, _('No'), self, color='red', includeText=True)

    return self.yesButton, self.noButton

def eraseUserPrompt(x,y,z,self):
    self.text.remove()
    self.yesButton.remove()
    self.noButton.remove()
    self.text.remove()
    self.rect.remove()
    self.txt1.remove()
    self.yesButton.remove()
    self.txt2.remove()

def drawBasicPlayHomePagePart1(self):
    '''
    Method used in play activities (play-piano and play-rhythm)
    because they have similar formats
    '''
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

    self.playText, self.playRect = textBox(_('Play'), 220, 30, self, fill_color='gray')

    self.okText, self.okRect = textBox(_('Okay'), 550, 30, self, fill_color='gray')

# ---------------------
# NOT DOCUMENTED ONLINE
# ---------------------

def drawBasicPlayHomePagePart2(self):
    '''
    Method used in play activities (play-piano and play-rhythm)
    because they have similar formats
    '''
    # PLAY BUTTON
    self.playButton = goocanvas.Image(
            parent=self.rootitem,
            pixbuf=gcompris.utils.load_pixmap('piano_composition/playActivities/playbutton.png'),
            x=170,
            y=50,
            )
    self.playButton.connect("button_press_event", self.staff.playComposition)

    gcompris.utils.item_focus_init(self.playButton, None)

    # OK BUTTON
    self.okButton = goocanvas.Svg(parent=self.rootitem,
                         svg_handle=gcompris.skin.svg_get(),
                         svg_id="#OK"
                         )
    self.okButton.scale(1.4, 1.4)
    self.okButton.translate(-170, -400)
    self.okButton.connect("button_press_event", self.ok_event)
    gcompris.utils.item_focus_init(self.okButton, None)

    # ERASE BUTTON
    self.eraseText, self.eraseRect = textBox(_("Erase Attempt"), 700, 150, self, fill_color='gray')

    self.eraseButton = goocanvas.Image(
            parent=self.rootitem,
            pixbuf=gcompris.utils.load_pixmap('piano_composition/playActivities/erase.png'),
            x=650,
            y=170,
            )
    self.eraseButton.connect("button_press_event", self.erase_entry)
    gcompris.utils.item_focus_init(self.eraseButton, None)



#  gcompris - world_explore_template.py
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
# world_explore_template

'''
HOW-TO USE THIS TEMPLATE:
Please visit http://gcompris.net/wiki/Adding_an_explore_activity#Instructions_to_Develop_an_Explore_Activity
'''

import gobject
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas
import pango
import ConfigParser
import gcompris.sound
import gcompris.bonus
from gcompris import gcompris_gettext as _

from random import randint
import random

# -----------------------------------------------------------------------------
# set to True if you'd like to record selected locations to make a new activity
# BEWARE: setting this to true will delete all your previous records!
RECORD_LOCATIONS = False
# -----------------------------------------------------------------------------

ExploreActivityResourcesFilepath = 'explore/'
class Gcompris_explore:

    def __init__(self, gcomprisBoard):

        self.gcomprisBoard = gcomprisBoard
        self.gcomprisBoard.level = 1
        self.gcomprisBoard.maxlevel = 2

        # Needed to get key_press
        gcomprisBoard.disable_im_context = True
        self.activityDataFilePath = '/' + self.gcomprisBoard.name + '/'

        self.numLocations = 0 # the total number of locations in this activity

        self.timers = []

        self.soundMatchingScore = 0
        self.textMatchingScore = 0

        self.sectionsAnsweredCorrectlySoundMatchingGame = []
        self.sectionsAnsweredCorrectlyTextMatchingGame = []

        self.soundClipsRemaining = [] # list of sound clips still to be played during level 2
        self.allSoundClips = [] # list of sound clips extracted from content.desktop.in

        self.textPromptsRemaining = []
        self.allTextPrompts = []

    def start(self):
        '''
        method called to create 'home-page', the world map with all the locations.
        This method is re-called whenever 'Go Back To Map' button is pressed
        by any of the location pages.
        '''

        gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

        # suspend system sound
        self.saved_policy = gcompris.sound.policy_get()
        gcompris.sound.policy_set(gcompris.sound.PLAY_AND_INTERRUPT)
        gcompris.sound.pause()

        self.display_level(self.gcomprisBoard.level)

    def display_level(self, x=None, y=None, z=None):

        # Create a rootitem.
        if hasattr(self, 'rootitem'):
            self.rootitem.remove()
        self.rootitem = goocanvas.Group(parent=
                                        self.gcomprisBoard.canvas.get_root_item())

        # silence any currently playing music
        gcompris.sound.play_ogg('//boards/sounds/silence1s.ogg')

        level = self.gcomprisBoard.level

        # set the game bar in the bottom left
        gcompris.bar_set(gcompris.BAR_LEVEL)
        gcompris.bar_set_level(self.gcomprisBoard)
        gcompris.bar_location(20, -1, 0.6)



        # -------------------------------------------------------------
        # Load Background Image
        # -------------------------------------------------------------
        if not hasattr(self, 'data'):
            self.read_data() # read in the data from content.desktop.in file

        # only allow second level if content file has the tag 'SoundMatchingGameText'
        if hasattr(self, 'SoundMatchingGameText'):
            self.gcomprisBoard.maxlevel = 3

        # set x and y positions for the background image
        if not hasattr(self, 'backgroundx'):
            x = 10
        else:
            x = int(self.backgroundx)
        if not hasattr(self, 'backgroundy'):
            y = 10
        else:
            y = int(self.backgroundy)

        self.map = goocanvas.Image(
            parent=self.rootitem,
            x=x, y=y,
            pixbuf=gcompris.utils.load_pixmap(self.activityDataFilePath + self.background)
            )

        if RECORD_LOCATIONS:
            self.recordLocationsForDeveloper()
        else:

            self.drawLocations()

            if level == 1:
                self.writeText(self.generalText)
            else:
                # prepare game for play
                if self.loadStatusBar() == False:
                     return
                if level == 2 and self.gcomprisBoard.maxlevel == 3:

                    self.writeText(self.SoundMatchingGameText)
                    # PLAY BUTTON
                    self.playButton = goocanvas.Image(
                            parent=self.rootitem,
                            pixbuf=gcompris.utils.load_pixmap(ExploreActivityResourcesFilepath + 'playbutton.png'),
                            x=65,
                            y=100,
                            )
                    self.playButton.connect("button_press_event", self.playCurrentMusicSelection)
                    self.writeText(_('Click to play sound'), 100, 70)
                    gcompris.utils.item_focus_init(self.playButton, None)
                    self.playRandomSong()
                elif level == 3 or level == 2:

                    self.writeText(self.TextMatchingGameText)
                    self.playRandomText()

    def writeText(self, txt, x=None, y=None):
        '''
        write text box with background rectangle to game
        '''
        if x == None:
            x = 100
        if y == None:
            y = 250
        t = goocanvas.Text(
          parent=self.rootitem,
          x=x,
          y=y,
          width=150,
          text='<span font_family="URW Gothic L" size="medium" \
          weight="bold" style="italic">' + txt + '</span>',
          anchor=gtk.ANCHOR_CENTER,
          alignment=pango.ALIGN_CENTER,
          use_markup=True
          )

        TG = 10
        bounds = t.get_bounds()

        rect = goocanvas.Rect(parent=self.rootitem,
                              x=bounds.x1 - TG,
                              y=bounds.y1 - TG,
                              width=bounds.x2 - bounds.x1 + TG * 2,
                              height=bounds.y2 - bounds.y1 + TG * 2,
                              line_width=3.0)
        rect.props.fill_color = 'gray'
        rect.props.stroke_color = 'black'
        t.raise_(rect)

    def loadStatusBar(self):
        '''
        load the home page with the background image and locations with progress bar
        '''
        txt2 = _('Explore Status:')
        goocanvas.Text(
          parent=self.rootitem,
          x=195,
          y=490,
          width=100,
          text='<span font_family="URW Gothic L" size="medium" \
          weight="bold" style="italic">' + txt2 + '</span>',
          anchor=gtk.ANCHOR_CENTER,
          alignment=pango.ALIGN_CENTER,
          use_markup=True
          )

        x = 240 # starting x position of progress bar
        self.progressBar = goocanvas.Rect(parent=self.rootitem,
                x=x, y=480, width=500, height=25,
                stroke_color="black",
                line_width=3.0)

        if self.gcomprisBoard.level == 3 or self.gcomprisBoard.maxlevel == 2:
            s = self.textMatchingScore
            sec = self.sectionsAnsweredCorrectlyTextMatchingGame
        else:
            s = self.soundMatchingScore
            sec = self.sectionsAnsweredCorrectlySoundMatchingGame

        # display the correct progress in the progress bar, according to the
        # number of locations the student has correclty answered the question for
        for num in range(0, s):
            barwidth = 500.0 / (len(self.data.sections()) - 1)
            goocanvas.Rect(parent=self.rootitem,
            x=x, y=480, width=barwidth, height=25,
            stroke_color="black",
            fill_color="#32CD32",
            line_width=3.0)

            # add a little decoration
            goocanvas.Image(
            parent=self.rootitem,
            x=x + (barwidth / 2.0) - 15,
            y=460,
            pixbuf=gcompris.utils.load_pixmap(ExploreActivityResourcesFilepath + 'ribbon.png')
            )
            x += barwidth

        # check to see if student has won game
        if s == (len(self.data.sections()) - 1) and s != 0:

            gcompris.sound.play_ogg('/boards/sounds/silence1s.ogg')

            # show congratulations image!
            goocanvas.Image(
            parent=self.rootitem,
            x=200, y=30,
            pixbuf=gcompris.utils.load_pixmap(self.gameWonPic)
            )

            # reset the game
            if self.gcomprisBoard.level == 2:

                self.soundMatchingScore = 0
                self.sectionsAnsweredCorrectlySoundMatchingGame = []
                self.soundClipsRemaining = self.allSoundClips[:]
                self.timers.append(gobject.timeout_add(3000, self.set_level, 3))
            elif self.gcomprisBoard.level == 3:
                self.textMatchingScore = 0
                self.sectionsAnsweredCorrectlyTextMatchingGame = []
                self.textPromptsRemaining = self.allTextPrompts[:]

                self.timers.append(gobject.timeout_add(3000, self.set_level, 1))
            self.timers = []


            return False
        return True


    def drawLocations(self):
        '''
        draw image on the map, one for each section in content.desktop.in at the
        location specified in the file by 'x' and 'y'. If the student
        has already visited the location and correctly answered the quetsion,
        thellipse will be colored green. Otherwise, the ellipse is red.
        '''
        if self.gcomprisBoard.level == 1:
            method = self.goto_location
        elif self.gcomprisBoard.level == 2 and self.gcomprisBoard.maxlevel == 3:
            method = self.checkAnswerSoundMatchingGame
        else:
            method = self.checkAnswerTextMatchingGame

        for section in self.sectionNames:
            filename = self.locationPic

            vars()[section] = goocanvas.Image(
            parent=self.rootitem,
            x=int(self.data.get(section, 'x')) - 20,
            y=int(self.data.get(section, 'y')) - 20,
            pixbuf=gcompris.utils.load_pixmap(filename)
            )

            vars()[section].connect("button_press_event", method)
            gcompris.utils.item_focus_init(vars()[section], None)
            vars()[section].set_data('sectionNum', section)

    def goto_location(self, widget=None, target=None, event=None):
            '''
            method called when student clicks on one of the ellipses.
            method loads the location page, including the text, picture, music, and question.
            '''

            self.rootitem.remove()
            self.rootitem = goocanvas.Group(parent=
                                            self.gcomprisBoard.canvas.get_root_item())
            sectionNum = target.get_data('sectionNum')

            goocanvas.Image(parent=self.rootitem, x=10, y=10,
                pixbuf=gcompris.utils.load_pixmap(ExploreActivityResourcesFilepath + 'border.png'))

            # draw back button
            txt = _('Back to Homepage')
            self.backButton = goocanvas.Text(
              parent=self.rootitem,
              x=260,
              y=490,
              text='<span font_family="purisa" size="medium" style="italic">' + txt + '</span>',
              anchor=gtk.ANCHOR_CENTER,
              alignment=pango.ALIGN_CENTER,
              use_markup=True
              )

            self.backButton.connect("button_press_event", self.display_level)
            gcompris.utils.item_focus_init(self.backButton, None)

            # ---------------------------------------------------------------------
            # WRITE LOCATION-SPECIFIC CONTENT TO PAGE
            # ---------------------------------------------------------------------

            name = _(self.data.get(sectionNum, '_title'))
            goocanvas.Text(
              parent=self.rootitem,
              x=410,
              y=50,
              text='<span font_family="century schoolbook L" size="x-large" weight="bold">' + name + '</span>',
              fill_color="black",
              anchor=gtk.ANCHOR_CENTER,
              alignment=pango.ALIGN_CENTER,
              use_markup=True
              )

            text = self.data.get(sectionNum, '_text')
            t = goocanvas.Text(
              parent=self.rootitem,
              x=120,
              y=190,
              width=150,
              text=_(text),
              fill_color="black",
              anchor=gtk.ANCHOR_CENTER,
              alignment=pango.ALIGN_CENTER
              )
            t.scale(1.4, 1.4)
            image = self.data.get(sectionNum, 'image')
            goocanvas.Image(
                parent=self.rootitem,
                x=300,
                y=120,
                pixbuf=gcompris.utils.load_pixmap(self.activityDataFilePath + image)
                )


            try:
                music = str(self.data.get(sectionNum, 'music'))
                gcompris.sound.play_ogg(self.activityDataFilePath + music)
            except: pass


    def checkAnswerTextMatchingGame(self, widget=None, target=None, event=None):
        '''
        check to see if the student pressed the correct answer. If so, increment
        textMatchingScore. Display appropriate face (happy or sad) for 800 ms.
        '''
        if not ready(self, timeouttime=1000):
            return
        if target.get_data('sectionNum') == self.currentTextSelection[1] and \
            self.currentTextSelection in self.textPromptsRemaining:
            self.textMatchingScore += 1
            self.sectionsAnsweredCorrectlyTextMatchingGame.append(target.get_data('sectionNum'))
            gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.SMILEY)

            self.textPromptsRemaining.remove(self.currentTextSelection)
            self.timers.append(gobject.timeout_add(2000, self.display_level))

        else:
            gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.SMILEY)

    def checkAnswerSoundMatchingGame(self, widget=None, target=None, event=None):
        '''
        check to see if the location the student chose corresponds to the
        currently playing sound clip. increment score accordingly
        '''
        if not ready(self, timeouttime=1000): # prevents kids from double clicking too quickly
            return
        if target.get_data('sectionNum') == self.currentMusicSelection[1] and \
            self.currentMusicSelection in self.soundClipsRemaining:
            self.soundMatchingScore += 1
            self.sectionsAnsweredCorrectlySoundMatchingGame.append(target.get_data('sectionNum'))
            self.soundClipsRemaining.remove(self.currentMusicSelection)
            self.timers.append(gobject.timeout_add(2000, self.display_level))
            gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.SMILEY)

        else:
            gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.SMILEY)

    def playRandomSong(self):
        '''
        play a random sound clip for use in the second level
        '''
        if self.soundClipsRemaining:
            self.currentMusicSelection = \
            self.soundClipsRemaining[randint(0, len(self.soundClipsRemaining) - 1)]
            self.timers.append(gobject.timeout_add(800, self.playCurrentMusicSelection))

    def playRandomText(self):

        if self.textPromptsRemaining:
            self.currentTextSelection = \
            self.textPromptsRemaining[randint(0, len(self.textPromptsRemaining) - 1)]

        self.text = goocanvas.Text(
            parent=self.rootitem, x=self.textBoxX, y=self.textBoxY, width=200,
            text=_(self.currentTextSelection[0]),
            fill_color='black', anchor=gtk.ANCHOR_CENTER,
            alignment=pango.ALIGN_CENTER,

            )
        TG = 10
        bounds = self.text.get_bounds()

        rect = goocanvas.Rect(parent=self.rootitem,
                              x=bounds.x1 - TG,
                              y=bounds.y1 - TG,
                              width=bounds.x2 - bounds.x1 + TG * 2,
                              height=bounds.y2 - bounds.y1 + TG * 2,
                              line_width=3.0)
        rect.props.fill_color = 'gray'
        rect.props.stroke_color = 'blach'
        self.text.raise_(rect)


    def playCurrentMusicSelection(self, x=None, y=None, z=None):
        gcompris.sound.play_ogg(self.activityDataFilePath +
                                                    self.currentMusicSelection[0])

    def set_level(self, level):
        '''
        updates the level for the game when child clicks on bottom
        left navigation bar to increment level
        '''
        if not ready(self, 500):
            return
        self.gcomprisBoard.level = level
        gcompris.bar_set_level(self.gcomprisBoard)
        self.display_level(self.gcomprisBoard.level)

# --------------------------------------------------------------------------
# METHODS TO DEAL WITH INPUT & OUTPUT OF CONTENT FILE
# --------------------------------------------------------------------------

# FYI: at first I didn't see any need to make a class to handle the data, but in
# retrospect having a data object would have been much cleaner.

    def recordLocationsForDeveloper(self):
        '''
        prepare to record the locations by connecting the entire
        background image with the record_location method
        '''
        for section in self.data.sections():
            if section != 'common':
                self.data.remove_section(section)

        self.map.connect("button_press_event", self.record_location)
        gcompris.utils.item_focus_init(self.map, None)

    def record_location(self, widget=None, target=None, event=None):
        '''
        method generates output to content.desktop.in according to the specified format.
        Method called if RECORD_LOCATIONS = True and developer clicks map. Merhod
        record the location of the click, and writes a template of the section
        to content.desktop.in to be filled in later by the developer.
        '''
        self.numLocations += 1
        self.data.add_section(str(self.numLocations))
        x = event.x
        y = event.y
        self.data.set(str(self.numLocations), 'x', int(x))
        self.data.set(str(self.numLocations), 'y', int(y))
        self.data.set(str(self.numLocations), '_title', _('Location Title Here'))
        self.data.set(str(self.numLocations), '_text', _('location text here'))
        self.data.set(str(self.numLocations), 'image', _('image filepath here, located in resources/name_of_activity/'))
        self.data.set(str(self.numLocations), 'music', _('music file name here'))
        self.data.set(str(self.numLocations), '_shortPrompt', _('enter text for child to match to the location'))

        # draw small elipse on screen to show developer where they clicked
        goocanvas.Ellipse(parent=self.rootitem,
        center_x=x,
        center_y=y,
        radius_x=5,
        radius_y=5,
        fill_color='white',
        stroke_color='white',
        line_width=1.0)

        # write a small number ontop of the ellipse so developer can record
        # which numbers (sections) correspond to which locations
        goocanvas.Text(
        parent=self.rootitem,
        x=x,
        y=y,
        text=str(self.numLocations),
        anchor=gtk.ANCHOR_CENTER,
        alignment=pango.ALIGN_CENTER,
        )

    def read_data(self):
        '''
        method to read in the data from content.desktop.in. Saves this data as
        self.data for reference later.
        '''
        #self.data = ConfigParser.RawConfigParser() # the data that is parsed from
        config = ConfigParser.RawConfigParser()
        filename = gcompris.DATA_DIR + '/' + self.gcomprisBoard.name + '/content.desktop.in'
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
        self.parseData()

    def parseData(self):
        '''
        extract the data from the content file
        '''
        self.sectionNames = []
        for section in self.data.sections():
            if section == 'common':
                try: self.credits = self.data.get('common', 'credits')
                except: self.credits = ''
                try: self.background = self.data.get('common', 'background')
                except:  gcompris.utils.dialog(_("Cannot find background in \
                content.desktop.in"), None, None)
                try: self.author = self.data.get('common', 'author')
                except: self.author = ''
                try: self.locationPic = self.activityDataFilePath + self.data.get('common', 'locationpic')
                except: self.locationPic = ExploreActivityResourcesFilepath + 'defaultLocationPic.png'
                try: self.gameWonPic = self.activityDataFilePath + self.data.get('common', 'gamewonpic')
                except: self.gameWonPic = ExploreActivityResourcesFilepath + 'happyFace.png'
                try: self.generalText = self.data.get('common', 'GeneralText')
                except:pass
                try: self.SoundMatchingGameText = self.data.get('common', 'SoundMatchingGameText')
                except:pass
                try: self.TextMatchingGameText = self.data.get('common', 'TextMatchingGameText')
                except:pass
                try: self.backgroundx = self.data.get('common', 'backgroundx')
                except:pass
                try: self.backgroundy = self.data.get('common', 'backgroundy')
                except:pass
                try: self.textBoxX = int(self.data.get('common', 'textBoxX'))
                except:pass
                try: self.textBoxY = int(self.data.get('common', 'textBoxY'))
                except:pass
            else:
                try:
                    self.soundClipsRemaining.append((self.data.get(section, 'music'), section))
                    self.allSoundClips.append((self.data.get(section, 'music'), section))
                except:
                    pass
                self.textPromptsRemaining.append((self.data.get(section, '_shortPrompt'), section))
                self.allTextPrompts.append((self.data.get(section, '_shortPrompt'), section))

                self.sectionNames.append(section)

    def end(self):
        '''
        write locations and common template to content.desktop.in
        '''
        if RECORD_LOCATIONS:
            try: self.data.set('common', 'credits', _('enter a list of credits and links to resources you used here'))
            except: pass
            try: self.data.set('common', 'creator', _('enter your name here!'))
            except: pass
            try: self.data.set('common', 'locationpic', _('enter the filename of the picture you would like to use to identify items to click on your background image'))
            except: pass
            try: self.data.set('common', 'gamewonpic', _('enter the filename of the picture to be shown when the player wins the entire game'))
            except: pass
            try: self.data.set('common', 'generalText', _('enter the text to appear on your image for textMatchingGame'))
            except: pass
            try: self.data.set('common', 'SoundMatchingGameText', _('enter the text to appear on your image for SoundMatchingGame'))
            except: pass
            try: self.data.set('common', 'TextMatchingGameText', _('enter the text to appear on your image for TextMatchingGame'))
            except: pass
            try: self.data.set('common', 'textBoxX', _('enter the x location for the text box to appear in the text matching game'))
            except: pass
            try: self.data.set('common', 'textBoxY', _('enter the x location for the text box to appear in the text matching game'))
            except: pass
            with open(gcompris.DATA_DIR + '/' + self.gcomprisBoard.name + '/content.desktop.in', 'wb') as configfile:
                self.data.write(configfile)

        self.rootitem.remove()
        gcompris.sound.policy_set(self.saved_policy)
        gcompris.sound.resume()

    def ok(self):
        pass

    def repeat(self):
        pass

    def config_stop(self):
        pass

    def config_start(self, profile):
        pass

    def key_press(self, keyval, commit_str, preedit_str):
        utf8char = gtk.gdk.keyval_to_unicode(keyval)
        strn = u'%c' % utf8char

    def pause(self, pause):
        pass

def ready(self, timeouttime=200):
    '''
    copied from gcomprismusic.py because I didn't want to try to import this module
    function to help prevent "double-clicks". If your function call is
    suffering from accidental system double-clicks, import this module
    and write these lines at the top of your method:

        if not ready(self):
            return False
    '''
    if not hasattr(self, 'clickTimers'):
        self.clickTimers = []
        self.readyForNextClick = True
        return True

    def clearClick():
        self.readyForNextClick = True
        return False

    if self.readyForNextClick == False:
        return
    else:
        self.clickTimers.append(gobject.timeout_add(timeouttime, clearClick))
        self.readyForNextClick = False
        return True

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

TEXT_BG_COLOR = 0xCCCCCC99L

class Gcompris_explore:

    def __init__(self, gcomprisBoard):

        self.gcomprisBoard = gcomprisBoard
        self.gcomprisBoard.level = 1
        self.gcomprisBoard.maxlevel = 2

        # Needed to get key_press
        gcomprisBoard.disable_im_context = True
        self.activityDataFilePath = '/' + self.gcomprisBoard.name + '/'

        self.numLocations = 0 # the total number of locations in this activity

        self.remainingItems = [] # list of all items still to be played during level 2 and 3
        self.allSoundClips = []  # list of sounds be played extracted from content.desktop.in
        self.allTextPrompts = [] # list of text be played extracted from content.desktop.in

        self.locationSeen = 0
        self.progressBar = None
        self.next_action = None

        self.first_run = True

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

        if self.gcomprisBoard.mode == "audio" \
                and not (gcompris.get_properties().fx):
            gcompris.utils.dialog(_("Error: This activity cannot be \
played with the\nsound effects disabled.\nGo to the configuration \
dialogue to\nenable the sound."), None)

    def display_level(self, x=None, y=None, z=None):

        # Create a rootitem.
        if hasattr(self, 'rootitem'):
            self.rootitem.remove()
        self.rootitem = goocanvas.Group(parent=
                                        self.gcomprisBoard.canvas.get_root_item())

        # silence any currently playing music
        if not self.first_run:
            gcompris.sound.play_ogg('boards/sounds/silence1s.ogg')
            self.first_run = False

        level = self.gcomprisBoard.level

        # set the game bar in the bottom left
        gcompris.bar_set(gcompris.BAR_LEVEL)
        gcompris.bar_set_level(self.gcomprisBoard)
        gcompris.bar_location(20, -1, 0.6)

        self.locationSeen = 0

        # -------------------------------------------------------------
        # Load Background Image
        # -------------------------------------------------------------
        if not hasattr(self, 'data'):
            self.read_data() # read in the data from content.desktop.in file

        # only allow second level if content file has the tag 'SoundMatchingGameText'
        if hasattr(self, 'SoundMatchingGameText'):
            self.gcomprisBoard.maxlevel = 3

        self.svghandle = gcompris.utils.load_svg(self.activityDataFilePath + self.background)
        goocanvas.Svg(
            parent = self.rootitem,
            svg_handle = self.svghandle,
            svg_id = self.backSvgId,
            pointer_events = goocanvas.EVENTS_NONE
            )


        self.drawLocations()

        if level == 1:
            self.writeText(self.generalText)
        else:
            # prepare game for play
            self.progressBar = ProgressBar( self.rootitem,
                                            200, 480, 400, 25,
                                            len(self.data.sections()) - 1 )

            if level == 2 and self.gcomprisBoard.maxlevel == 3:

                self.remainingItems = self.allSoundClips[:]
                self.writeText(self.SoundMatchingGameText)
                # PLAY BUTTON
                self.playButton = goocanvas.Image(
                        parent=self.rootitem,
                        pixbuf=gcompris.utils.load_pixmap('explore/playbutton.png'),
                        x=65,
                        y=110,
                        )
                self.playButton.connect("button_press_event", self.playCurrentMusicSelection)
                self.writeText(_('Click to play sound'), 100, 70)
                gcompris.utils.item_focus_init(self.playButton, None)
                self.playRandom()
            elif level == 3 or level == 2:

                self.remainingItems = self.allTextPrompts[:]
                self.writeText(self.TextMatchingGameText)
                self.playRandom()

    def next_level(self):
        if self.gcomprisBoard.level == self.gcomprisBoard.maxlevel:
            self.set_level( 1 )
        else:
            self.set_level( self.gcomprisBoard.level + 1 )

    def writeText(self, txt, x=100, y=250, width=150):
        '''
        write text box with background rectangle to game
        the text is returned and must be removed by the caller
        '''

        # A group that will hold the text description and the background
        textrootitem = goocanvas.Group(parent=
                                       self.rootitem)

        t = goocanvas.Text(
          parent = textrootitem,
          x=x,
          y=y,
          width=width,
          font = gcompris.skin.get_font("gcompris/board/medium"),
          text = txt,
          anchor=gtk.ANCHOR_CENTER,
          alignment=pango.ALIGN_CENTER,
          use_markup=True
          )

        TG = 10
        bounds = t.get_bounds()

        rect = goocanvas.Rect(parent = textrootitem,
                              x=bounds.x1 - TG,
                              y=bounds.y1 - TG,
                              width=bounds.x2 - bounds.x1 + TG * 2,
                              height=bounds.y2 - bounds.y1 + TG * 2,
                              line_width=2.0,
                              radius_x = 3.0,
                              radius_y = 3.0,
                              fill_color_rgba = TEXT_BG_COLOR,
                              stroke_color = "black")
        t.raise_(rect)
        return textrootitem

    def drawLocations(self):
        '''
        draw image on the map, one for each section in content.desktop.in at the
        location specified in the file by 'x' and 'y'.
        '''
        if self.gcomprisBoard.level == 1:
            method = self.goto_location
        else:
            method = self.checkAnswer

        for section in self.sectionNames:

            item = goocanvas.Svg(
                parent = self.rootitem,
                svg_handle = self.svghandle,
                svg_id = self.data.get(section, 'svgId'),
                )

            gcompris.utils.item_focus_init(item, None)
            item.set_data('sectionNum', section)
            item.set_data('seen', False)
            # Set the proper callback depending on the level
            item.connect("button_press_event", method)

    def location_quit(self, widget=None, target=None, event=None, location_rootitem=None):
            '''
            called when the user click on end in the location panel display
            the main display is just shown again and the location panel is removed.
            '''
            self.rootitem.props.visibility = goocanvas.ITEM_VISIBLE
            location_rootitem.remove()

            # silence any currently playing music
            gcompris.sound.play_ogg('boards/sounds/silence1s.ogg')

            # All the items have been seen, let's start the level 2
            if self.locationSeen == len(self.sectionNames):
                self.next_action = self.next_level
                gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.SMILEY)

    def goto_location(self, widget=None, target=None, event=None):
            '''
            method called when student clicks on one of the ellipses.
            method loads the location page, including the text, picture, music, and question.
            '''
            bounds = target.get_bounds()
            seen = target.get_data('seen')
            if not seen:
                target.set_data('seen', True)
                self.locationSeen += 1
            pixmap = gcompris.utils.load_pixmap('explore/star.png')
            goocanvas.Image(parent = self.rootitem,
                            x = bounds.x1 + \
                                (bounds.x2 - bounds.x1) / 2.0 - \
                                pixmap.get_width() / 2.0,
                            y = bounds.y2 - pixmap.get_height() / 2.0,
                            pixbuf = pixmap
                            )

            self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE

            if hasattr(self, 'location_rootitem'):
                self.location_rootitem.remove()
            self.location_rootitem = goocanvas.Group(parent=
                                                     self.gcomprisBoard.canvas.get_root_item())
            sectionNum = target.get_data('sectionNum')

            goocanvas.Image(parent=self.location_rootitem, x=10, y=10,
                pixbuf=gcompris.utils.load_pixmap('explore/border.png'))

            # draw back button
            txt = _('Back to Homepage')
            self.backButton = goocanvas.Text(
              parent=self.location_rootitem,
              x=400,
              y=495,
              text='<span font_family="century schoolbook L" size="medium" weight="bold">' + txt + '</span>',
              anchor=gtk.ANCHOR_CENTER,
              alignment=pango.ALIGN_CENTER,
              use_markup=True
              )

            self.backButton.connect("button_press_event", self.location_quit, self.location_rootitem)
            gcompris.utils.item_focus_init(self.backButton, None)

            # ---------------------------------------------------------------------
            # WRITE LOCATION-SPECIFIC CONTENT TO PAGE
            # ---------------------------------------------------------------------

            name = _(self.data.get(sectionNum, '_title'))
            goocanvas.Text(
              parent=self.location_rootitem,
              x=410,
              y=50,
              text='<span font_family="century schoolbook L" size="x-large" weight="bold">' + name + '</span>',
              fill_color="black",
              anchor=gtk.ANCHOR_CENTER,
              alignment=pango.ALIGN_CENTER,
              use_markup=True
              )

            text = _(self.data.get(sectionNum, '_text'))
            t = goocanvas.Text(
              parent=self.location_rootitem,
              x=170,
              y=120,
              width=240,
              text=_(text),
              font = gcompris.skin.get_font("gcompris/board/medium"),
              fill_color="black",
              anchor=gtk.ANCHOR_N,
              alignment=pango.ALIGN_CENTER
              )
            image = self.data.get(sectionNum, 'image')
            goocanvas.Image(
                parent=self.location_rootitem,
                x=300,
                y=120,
                pixbuf=gcompris.utils.load_pixmap(self.activityDataFilePath + image)
                )


            try:
                music = str(self.data.get(sectionNum, 'music'))
                gcompris.sound.play_ogg(self.activityDataFilePath + music)
            except: pass


    def checkAnswer(self, widget=None, target=None, event=None):
        '''
        check to see if the student pressed the correct answer.
        '''
        if target.get_data('sectionNum') == self.currentSelection[1] and \
            self.currentSelection in self.remainingItems:
            self.remainingItems.remove(self.currentSelection)
            self.progressBar.success()
            if len(self.remainingItems):
                self.next_action = self.playRandom
            else:
                self.next_action = self.next_level
            gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.SMILEY)
        else:
            gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.SMILEY)

    def playRandom(self):
        ''' call playRandomSong or playRandomText depending on the current
        level '''
        level = self.gcomprisBoard.level
        if level == 2 and self.gcomprisBoard.maxlevel == 3:
            self.playRandomSong()
        elif level == 3 or level == 2:
            self.playRandomText()

    def playRandomSong(self):
        '''
        play a random sound clip for use in the second level
        '''
        if self.remainingItems:
            self.currentSelection = \
                self.remainingItems[randint(0, len(self.remainingItems) - 1)]
            self.playCurrentMusicSelection()

    def playRandomText(self):

        if self.remainingItems:
            self.currentSelection = \
                self.remainingItems[randint(0, len(self.remainingItems) - 1)]

        if hasattr(self, 'randomtext'):
            self.randomtext.remove()
        self.randomtext = self.writeText(self.currentSelection[0],
                                         self.textBoxX, self.textBoxY, 200)


    def playCurrentMusicSelection(self, x=None, y=None, z=None):
        gcompris.sound.play_ogg(self.activityDataFilePath +
                                self.currentSelection[0])

    def set_level(self, level):
        '''
        updates the level for the game when child clicks on bottom
        left navigation bar to increment level
        '''
        self.gcomprisBoard.level = level
        gcompris.bar_set_level(self.gcomprisBoard)
        self.display_level(self.gcomprisBoard.level)

# --------------------------------------------------------------------------
# METHODS TO DEAL WITH INPUT & OUTPUT OF CONTENT FILE
# --------------------------------------------------------------------------

# FYI: at first I didn't see any need to make a class to handle the data, but in
# retrospect having a data object would have been much cleaner.

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
        errors = []
        for section in self.data.sections():
            if section == 'common':
                try: self.credits = self.data.get('common', 'credits')
                except: self.credits = ''
                try: self.background = self.data.get('common', 'background')
                except: errors.append("Missing 'background' key")
                try: self.backSvgId = self.data.get('common', 'backSvgId')
                except: errors.append("Missing 'background' key")
                try: self.author = self.data.get('common', 'author')
                except: self.author = ''
                try: self.generalText = _(self.data.get('common', '_GeneralText'))
                except: errors.append("Missing '_GeneralText' key")
                try: self.SoundMatchingGameText = _(self.data.get('common', '_SoundMatchingGameText'))
                except:pass
                try: self.TextMatchingGameText = _(self.data.get('common', '_TextMatchingGameText'))
                except:pass
                try: self.textBoxX = int(self.data.get('common', 'textBoxX'))
                except:pass
                try: self.textBoxY = int(self.data.get('common', 'textBoxY'))
                except:pass
            else:
                try:
                    self.allSoundClips.append( (self.data.get(section, 'music'), section))
                except:
                    pass
                self.allTextPrompts.append( ( _(self.data.get(section, '_shortPrompt')), section))

                self.sectionNames.append(section)

            if len(errors):
                gcompris.utils.dialog( "\n".join(errors), None)

    def end(self):
        # silence any currently playing music
        gcompris.sound.play_ogg('boards/sounds/silence1s.ogg')
        self.rootitem.remove()
        if hasattr(self, 'location_rootitem'):
            self.location_rootitem.remove()
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
        if not pause and self.next_action:
            self.next_action()
            self.next_action = None

class ProgressBar:

    def __init__(self, rootitem, x, y, width, height, number_of_sections):
        '''
        display an empty progress bar
        '''
        self.rootitem = rootitem
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.number_of_sections = number_of_sections

        txt2 = _('Explore Status:')
        item = goocanvas.Text(
          parent = self.rootitem,
          x = self.x,
          y = self.y,
          text = '<span font_family="URW Gothic L" size="medium" \
          weight="bold" style="italic">' + txt2 + '</span>',
          use_markup = True
          )

        bounds = item.get_bounds()
        # This is the start of the bar
        self.x += bounds.x2 - bounds.x1 + 20
        self.progressBar = goocanvas.Rect(
            parent = self.rootitem,
            x = self.x,
            y = self.y,
            width = self.width,
            height = self.height,
            stroke_color = "black",
            fill_color_rgba = 0x666666AAL,
            line_width = 2.0,
            radius_x = 3,
            radius_y = 3 )

    def success(self):
        ''' Add a success item in the progress bar '''
        success_width = self.width * 1.0 / self.number_of_sections
        goocanvas.Rect(
            parent = self.rootitem,
            x = self.x,
            y = self.y,
            width = success_width,
            height = self.height,
            stroke_color = "black",
            fill_color = "#32CD32",
            radius_x = 3,
            radius_y = 3,
            line_width = 2.0)

        # add a little decoration
        goocanvas.Image(
            parent = self.rootitem,
            x = self.x + (success_width / 2.0) - 15,
            y = self.y - 20,
            pixbuf = gcompris.utils.load_pixmap('explore/ribbon.png')
            )
        self.x += success_width


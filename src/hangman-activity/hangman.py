#  gcompris - hangman.py
# -*- coding: utf-8 -*-
#
# Copyright (C) 2003, 2008 Bruno Coudoin
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
# PythonTemplate Board module
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.score
import goocanvas
import pango
import gettext
from gcompris import gcompris_gettext as _
from gettext import dgettext as D_

fles = None

class Gcompris_hangman:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;

    # global parameter to access object structures from global fonctions
    global fles
    fles=self

    # The current word to search
    self.word = ""
    # The list of letter to find
    self.letters = []
    # The list of virtual keys displayed
    self.keys = []
    # the number of trials left to the user
    self.trial = 0
    # All the vowel of your language (keep empty if non applicable)
    # Separate with /. You can use digraphs, trigraphs etc.
    tempvowels = unicode(_("a/e/i/o/u/y"), encoding="utf8")
    #tempvowels = "a/e/i/o/u/y"
    self.vowels = tempvowels.split("/")
    # All the consonants of your language (keep empty if non applicable)
    # Separate with /. You can use digraphs, trigraphs etc. For example, both 's' and 'sch' could be in the list for German
    tempconsonants = unicode(_("b/c/d/f/g/h/j/k/l/m/n/p/q/r/s/t/v/w/x/z"), encoding="utf8")
    self.consonants = tempconsonants.split("/")
    
    # Keys to letters equivalence for the hangman activity. It has the
    # form of a space separated list like: "é=e/E è=e/E sch=S"
    # Letters on the left of the = can be multigraphs and represent the letters on the buttons.
    # Letters on the right are single letters pressed on the keyboard.
    # If you wish to allow different key presses for the same letter, separate the letters
    # on the right with /
    keyequivs = unicode(_("a=a"), encoding="utf8")
    if keyequivs == "a=a":
      keyequivs = None
    
    # Create equivs list
    self.keyequivList = {}
    if keyequivs:
      for keyequiv in keyequivs.split(' '):
        try:
          (k, v) = keyequiv.split('=')
          self.keyequivList[k]=v
        except:
          print ("ERROR: Bad key equivalence list '%s' for hangman: " %(keyequivs, ))

    # Letters equivalence for the hangman activity. It has the
    # form of a space separated list like: "e=éè a=àâ"
    # Keep the word NONE if not available in your language
    self.equivs = unicode(_("NONE"), encoding="utf8")
    if self.equivs == "NONE":
      self.equivs = ""

    # Create equivs list
    self.equivList = []
    for equiv in self.equivs.split(' '):
      try:
        (k, v) = equiv.split('=')
        self.equivList.append(k + v)
      except:
        print ("ERROR: Bad equivalence list '%s'" %(self.equivs, ))


  def start(self):
    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.backitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    svghandle = gcompris.utils.load_svg("hangman/back.svgz")
    goocanvas.Svg(
      parent = self.backitem,
      svg_handle = svghandle,
      pointer_events = goocanvas.EVENTS_NONE
      )

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.backitem)

    # Get the name of the language for the current locale
    self.wordlist = None
    try:
      self.language = D_(gcompris.GETTEXT_GUI,gcompris.get_locale_name(gcompris.get_locale()) )
      self.wordlist = gcompris.get_wordlist("hangman/default-$LOCALE.xml")
    except:
      pass
    # Fallback to wordsgame list
    if not self.wordlist:
        try:
            self.language = D_(gcompris.GETTEXT_GUI,gcompris.get_locale_name(gcompris.get_locale()) )
            self.wordlist = gcompris.get_wordlist("wordsgame/default-$LOCALE.xml")
        except:
            pass
    # Fallback to english
    if not self.wordlist:
        try:
            self.wordlist = gcompris.get_wordlist("hangman/default-en.xml")
            self.language = D_(gcompris.GETTEXT_GUI,"English")
        except:
            pass
    # Fallback to English wordsgame list
    if not self.wordlist:
        self.wordlist = gcompris.get_wordlist("wordsgame/default-en.xml")
        self.language = D_(gcompris.GETTEXT_GUI,"English")

    if not self.wordlist:
      gcompris.utils.dialog(D_(gcompris.GETTEXT_ERROR,"Could not find the list of words."),
                            stop_board)
      return;

    self.gcomprisBoard.level = 1
    self.gcomprisBoard.maxlevel = self.wordlist.number_of_level * 3
    self.gcomprisBoard.sublevel = 1
    self.gcomprisBoard.number_of_sublevel = 10

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(gcompris.BOARD_WIDTH - 160, -1, 0.6)

    # Set a background image
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    self.display_level()

  def end(self):
    # Remove the root item removes all the others inside it
    self.backitem.remove()
    gcompris.score.end()


  def ok(self):
    print("hangman ok.")


  def repeat(self):
    print("hangman repeat.")


  def config(self):
    print("hangman config.")


  def key_press(self, keyval, commit_str, preedit_str):
    if not commit_str:
      return

    for k in self.keys:
      if k.click(commit_str):
        break

    return True

  def pause(self, pause):

    self.board_paused = pause

    # When the bonus is displayed, it call us first
    # with pause(1) and then with pause(0)
    # the game is won if 1, lost if 2
    if(self.gamewon >= 1 and pause == 0):
      if self.gamewon == 1:
        self.increment_level()
      self.gamewon = 0
      self.next_level()
    return

  def set_level(self, level):
    self.gcomprisBoard.level = level;
    self.gcomprisBoard.sublevel = 1;
    gcompris.bar_set_level(self.gcomprisBoard)
    self.next_level()

#-------

  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel > self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
        self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

  def next_level(self):
    gcompris.score.end()
    self.rootitem.remove()
    self.rootitem = goocanvas.Group(parent =
                                    self.backitem)
    self.display_level()

  def display_letter_set(self, letter_set, y,
                         fill_color, stroke_color):
    group = goocanvas.Group(parent = self.rootitem)

    w = 60
    max_per_line =gcompris.BOARD_WIDTH / w - 2
    x = (gcompris.BOARD_WIDTH - (max_per_line * w)) / 2
    wc = 0
    line = 0
    #shifting the virtual keyboard down
    # - tested with 2 or 3 rows of consonants + 1 row of vowels
    yshift=60
    for line in range(0, len(letter_set) / max_per_line + 1):
      for i in range(0, max_per_line):
        if wc < len(letter_set):
          self.keys.append(
            Key(self, group, x + i*w, y +yshift + line*w/3*2, letter_set[wc],
                self.get_equiv(letter_set[wc]),
                fill_color, stroke_color) )
          wc += 1

    return (group, y + (line + 1 )* w)

  def display_level(self):
    w = 40
    # Less trial when the level increases
    self.trial = 12 - (self.gcomprisBoard.level - 1) / self.wordlist.number_of_level
    gcompris.bar_set_level(self.gcomprisBoard);
    gcompris.score.start(gcompris.score.STYLE_NOTE,
                         gcompris.BOARD_WIDTH / 2 - 100, 170, self.trial)
    gcompris.score.set(self.trial)

    # Display the word to find
    self.word = self.get_next_word()
    self.letters = []
    self.keys= []
    # get letters from letter list and parse according to multigraphs:
    # append consonants and vowels into a new list for easy reference
    multigraphlist = set(self.vowels+self.consonants)

    # find out the length of the longest multigraph and store in constant
    longestmultigraph=1;
    for i,v in enumerate(multigraphlist):
        if longestmultigraph < len(v):
            longestmultigraph =  len(v)
    # chop up the word according to multigraphs, so the length of the word can be calculated
    parsedword=[]
    graphlength=longestmultigraph
    wordlength=len(self.word)
    i=0
    while i < wordlength:
        graphlength=longestmultigraph
        # make sure the end of the multigraph does not get out of bounds
        while (i+graphlength) > wordlength:
           graphlength = graphlength-1
        #  compare substring with list of multigraphs and reduce length if not found
        found = False
        while (found == False):
            # check if multigrasph is valid
            if (self.word[i:i+graphlength] in multigraphlist):
                parsedword.append(self.word[i:i+graphlength])
                found = True
            else:
                # make sure we don't get an endless loop or empty letters
                if (graphlength<= 1):
                    found = True
                    parsedword.append(self.word[i])
                    graphlength=1
            # the next time, look for a shorter multigraph
            graphlength = graphlength - 1
        # skip to the next multigraph in the word
        i = i+1+graphlength
    # end parsing the word with multigraphs

    # now construct and display the word for the game    
    x = (gcompris.BOARD_WIDTH - (len(parsedword) * w)) / 2
  
    for i in range(0, len(parsedword)):
        # dynamic width of last multigraph in the word
        if(i>0):
            if(len(parsedword[i-1])>1):
                xshift=(len(parsedword[i-1])-1)*0.75/len(parsedword[i-1]) #todo factor
        else:
            xshift=0
    
        self.letters.append(Letter(self, x + (i+xshift)*w, 70,
                                 parsedword[i],
                                 self.get_equiv(parsedword[i])))

    # Display the language
    goocanvas.Text(
      parent = self.rootitem,
      x = gcompris.BOARD_WIDTH / 2,
      y = gcompris.BOARD_HEIGHT / 2 - 30,
      text = self.language,
      fill_color = "white",
      font = gcompris.skin.get_font("gcompris/board/medium"),
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    # Display the virtual keyboard
    (group_vowels, y_vowels) = self.display_letter_set(self.vowels, 0,
                                                       0xFF6633AAL, 0xFF33CCBBL)
    (group_cons, y_cons) = self.display_letter_set(self.consonants, y_vowels,
                                                   0x66FF33AAL, 0xFF33CCBBL)

    group_vowels.translate(0, gcompris.BOARD_HEIGHT - y_cons - 40)
    group_cons.translate(0, gcompris.BOARD_HEIGHT - y_cons - 40)


  def get_next_word(self):
    return unicode(gcompris.get_random_word(self.wordlist,
                                    self.gcomprisBoard.level),
                   encoding="utf8")

  def found_all_letters(self):
    retval = True
    for letter in self.letters:
      if not letter.found:
        retval = False
    return retval

  def check_letters(self, targetLetter):
    retval = False
    for letter in self.letters:
        if letter.check(targetLetter):
            retval = True
    return retval

  def hide_letters(self, status):
    for letter in self.letters:
      letter.hide(status)

  def get_equiv(self, letter):
    """ Get key equivalence for the given letter/multigraph """
    letters = [letter]
    if letter in self.keyequivList:
        try:
            tempequivs=self.keyequivList[letter].split('/')
            for v in tempequivs:
                letters.append(v)
        except:
            print("Error parsing multigraphs in equivalence list: "+letter)

    """ Get equivalence for the given letter """
    #print("----- getting equivalence for: "+letter)
    if(len(letter)==1):
        #print("grabbing from list")
        for v in self.equivList:
          if v.count(letter):
            letters.append(v)
    return letters

  def ok_event(self, widget, target, event=None):
    if self.gamewon == 2:
      gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.TUX)
    else:
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)

  def display_ok(self):
    # The OK Button
    item = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#OK"
                         )
    zoom = 0.8
    item.translate( (item.get_bounds().x1 * -1)
                     + ( gcompris.BOARD_WIDTH - 300 ) / zoom,
                    (item.get_bounds().y1 * -1) + 190.0 / zoom)
    item.scale(zoom, zoom)
    item.connect("button_press_event", self.ok_event)
    gcompris.utils.item_focus_init(item, None)



# A letter to find displayed on the screen
class Letter:
    def __init__(self, hangman, x, y,
                 letter, letters):
      self.found = False
      parent = hangman.rootitem

      fill_color = 0xFF3366AAL
      stroke_color = 0xFF33CCAAL
      # dynamic display width for multigraphs
      if(len(letter)==1):
          xshift=1
      else:
          if(len(letter)==2):
              xshift=len(letter)*0.75
          else:
              if(len(letter)==3):
                  xshift=len(letter)*0.5
              else:
                  xshift=len(letter)*0.25
      w = 30*xshift
      h = 30

      self.letters = letters

      self.rect = goocanvas.Rect(
        parent = parent,
        x = x,
        y = y,
        width = w,
        height = h,
        fill_color_rgba = fill_color,
        stroke_color_rgba = stroke_color,
        line_width = 1.0)

      self.letterItem = goocanvas.Text(
        parent = parent,
        x = x + w/2,
        y = y + h/2,
        text = letter,
        fill_color = "black",
        font = gcompris.skin.get_font("gcompris/board/medium"),
        anchor = gtk.ANCHOR_CENTER,
        alignment = pango.ALIGN_CENTER
        )

      self.hide(True)

    def hide(self, status):
      if status:
        self.letterItem.props.visibility = goocanvas.ITEM_INVISIBLE
      else:
        self.letterItem.props.visibility = goocanvas.ITEM_VISIBLE

    def check(self, targetLetter):
      if (self.letters.count(targetLetter) > 0):
        self.rect.set_properties(fill_color_rgba = 0x66CC33AAL)
        self.letterItem.props.visibility = goocanvas.ITEM_VISIBLE
        self.found = True
        return True

      return False


# A virtual key on screen
class Key:
    def __init__(self, hangman, parent, x, y,
                 letter, letters, fill_color, stroke_color):
      self.hangman = hangman
      self.letter = letter
      self.letters = letters
      self.disabled = False

      # wide enough to fit "mmm". If there should be a need for longer multigraphs in the future, some dynamic programming will be required
      w = 55
      h = 30

      self.rect = goocanvas.Rect(
        parent = parent,
        x = x,
        y = y,
        width = w,
        height = h,
        fill_color_rgba = fill_color,
        stroke_color_rgba = stroke_color,
        line_width = 1.0)

      self.letterItem = goocanvas.Text(
        parent = parent,
        x = x + w/2,
        y = y + h/2,
        text = letter,
        fill_color = "black",
        font = gcompris.skin.get_font("gcompris/board/medium"),
        anchor = gtk.ANCHOR_CENTER,
        alignment = pango.ALIGN_CENTER
        )

      self.letterItem.connect("button_press_event",
                              self.letter_event, letter)
      self.rect.connect("button_press_event",
                        self.letter_event, letter)

    def click(self, letter):
      if(self.hangman.gamewon):
        return False

      # Check we are the correct key for the given letter
      if not self.letters.count(letter):
        return False

      # Already done
      if self.disabled:
        return True

      # disable this key, mark it as used
      self.disabled = True
      self.rect.set_properties(fill_color_rgba = 0xCCCCCCCCL,
                               line_width = 3.0)
      self.rect.disconnect_by_func(self.letter_event)
      self.letterItem.disconnect_by_func(self.letter_event)

      # Decrease user lives and trig bonus if needed
      if not self.hangman.check_letters(letter):
        self.hangman.trial -= 1
        gcompris.score.set(self.hangman.trial)
        if self.hangman.trial == 0:
          self.hangman.gamewon = 2
          self.hangman.hide_letters(False)
          self.hangman.display_ok()
      elif self.hangman.found_all_letters():
        self.hangman.gamewon = 1
        self.hangman.display_ok()

      return True
    #
    # Event on a letter
    #
    def letter_event(self, item, target, event, letter):
      self.click(letter)


def stop_board():
  global fles
  fles.end()
  gcompris.end_board()

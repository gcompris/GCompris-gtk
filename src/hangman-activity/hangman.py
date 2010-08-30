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

from gcompris import gcompris_gettext as _

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
    self.vowels = unicode(_("aeiouy"), encoding="utf8")
    # All the consonants of your language (keep empty if non applicable)
    self.consonants = unicode(_("bcdfghjklmnpqrstvwxz"), encoding="utf8")
    # Letters equivallence for the hangman activity. It has the
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
        print ("ERROR: Bad equivalence liste '%s'" %(self.equivs, ))

  def start(self):
    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    self.wordlist = gcompris.get_wordlist("wordsgame/default-$LOCALE.xml")
    if not self.wordlist:
      # Fallback to english
      self.wordlist = gcompris.get_wordlist("wordsgame/default-en.xml")

    if not self.wordlist:
      gcompris.utils.dialog(_("Could not find the list of words."),
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
    self.rootitem.remove()
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
                                    self.gcomprisBoard.canvas.get_root_item())
    self.display_level()

  def display_letter(self, parent, letter, x, y,
                     fill_color, stroke_color, goal):
    w = 30
    h = 30

    rect = goocanvas.Rect(
      parent = parent,
      x = x,
      y = y,
      width = w,
      height = h,
      fill_color_rgba = fill_color,
      stroke_color_rgba = stroke_color,
      line_width = 1.0)

    text = goocanvas.Text(
      parent = parent,
      x = x + w/2,
      y = y + h/2,
      text= letter,
      fill_color="black",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

  def display_letter_set(self, letter_set, y,
                         fill_color, stroke_color):
    group = goocanvas.Group(parent = self.rootitem)

    w = 40
    max_per_line =gcompris.BOARD_WIDTH / w - 2
    x = (gcompris.BOARD_WIDTH - (max_per_line * w)) / 2
    wc = 0
    line = 0
    for line in range(0, len(letter_set) / max_per_line + 1):
      for i in range(0, max_per_line):
        if wc < len(letter_set):
          self.keys.append(
            Key(self, group, x + i*w, y + line*w, letter_set[wc],
                self.get_equiv(letter_set[wc]),
                fill_color, stroke_color) )
          wc += 1

    return (group, y + (line + 1 )* w)

  def display_level(self):
    w = 40
    # Less trial when the level increases
    self.trial = 9 - (self.gcomprisBoard.level - 1) / self.wordlist.number_of_level
    gcompris.bar_set_level(self.gcomprisBoard);
    gcompris.score.start(gcompris.score.STYLE_NOTE,
                         gcompris.BOARD_WIDTH / 2 - 100, 170, self.trial)
    gcompris.score.set(self.trial)

    # Display the word to find
    self.word = self.get_next_word()
    self.letters = []
    self.keys= []

    x = (gcompris.BOARD_WIDTH - (len(self.word) * w)) / 2
    for i in range(0, len(self.word)):
      self.letters.append(Letter(self, x + i*w, 40,
                                 self.word[i],
                                 self.get_equiv(self.word[i])))

    # Display the virtual keyboard
    (group_vowels, y_vowels) = self.display_letter_set(self.vowels, 0,
                                                       0xFF663333L, 0xFF33CCAAL)
    (group_cons, y_cons) = self.display_letter_set(self.consonants, y_vowels + 20,
                                                   0x66FF3333L, 0xFF33CCAAL)

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
    """ Return equivallence for the given letter """
    letters = letter
    for v in self.equivList:
      if v.count(letter):
        letters += v
    return letters

# A letter to find displayed on the screen
class Letter:
    def __init__(self, hangman, x, y,
                 letter, letters):
      self.found = False
      parent = hangman.rootitem

      fill_color = 0xFF336633L
      stroke_color = 0xFF33CCAAL
      w = 30
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
        self.rect.set_properties(fill_color_rgba = 0x66CC3333L)
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

      w = 30
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
      self.rect.set_properties(fill_color_rgba = 0xCCCCCC33L,
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
          gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.TUX)
      elif self.hangman.found_all_letters():
        self.hangman.gamewon = 1
        gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)

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

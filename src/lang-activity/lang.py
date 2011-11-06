#  gcompris - lang.py
#
# Copyright (C) 2010 Bruno Coudoin
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
# lang activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.sound
import goocanvas
import pango

from gcompris import gcompris_gettext as _
from langLib import *

class MissingImage:
  """This is used to display a missing image"""

  def __init__(self, rootitem, langActivity):
    self.missingroot = goocanvas.Group( parent = rootitem )
    width = 280
    height = 240
    item = goocanvas.Rect( parent = self.missingroot,
                           x = gcompris.BOARD_WIDTH  / 2 - width / 2,
                           y = gcompris.BOARD_HEIGHT / 2 - height / 2,
                           width = width,
                           height = height,
                           radius_x = 5,
                           radius_y = 5,
                           stroke_color_rgba = 0x666666FFL,
                           fill_color_rgba = 0x33333366L,
                           line_width = 2.0 )
    item.connect("button_press_event", langActivity.next_event, None)
    self.missingtext = goocanvas.Text(
      parent = self.missingroot,
      x = gcompris.BOARD_WIDTH / 2,
      y = gcompris.BOARD_HEIGHT / 2,
      fill_color = "black",
      font = gcompris.skin.get_font("gcompris/subtitle"),
      text = _("Missing Image"),
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

  def hide(self):
    self.missingroot.props.visibility = goocanvas.ITEM_INVISIBLE

  def show(self, triplet):
    self.missingroot.props.visibility = goocanvas.ITEM_VISIBLE


class Gcompris_lang:
  """Empty gcompris python class"""

  def __init__(self, gcomprisBoard):
    print "lang init"

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    print "lang start"

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT)
    gcompris.bar_location(gcompris.BOARD_WIDTH / 2 - 100, -1, 0.6)

    # Set a background image
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    self.langLib = LangLib(gcompris.DATA_DIR + "/lang/lang.xml.in")
    self.chapters = self.langLib.getChapters()
    # FIXME Do not manage Chapter yet
    self.currentChapterId = 0

    # Manage levels (a level is a lesson in the lang model)
    self.gcomprisBoard.level = 1
    self.gcomprisBoard.maxlevel = \
        len ( self.chapters.getChapters()[self.currentChapterId].getLessons() )
    gcompris.bar_set_level(self.gcomprisBoard)

    self.currentLesson = self.langLib.getLesson(self.currentChapterId,
                                                self.gcomprisBoard.level - 1)
    self.displayLesson( self.currentLesson )

  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("lang ok.")


  def repeat(self):
    self.playVoice( self.currentLesson.getTriplets()[self.currentTripletId] )

  def config(self):
    print("lang config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_lang key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    print("lang pause. %i" % pause)


  def set_level(self, level):
    self.gcomprisBoard.level = level;
    self.gcomprisBoard.sublevel = 1;
    gcompris.bar_set_level(self.gcomprisBoard)
    self.currentLesson = self.langLib.getLesson(self.currentChapterId,
                                                self.gcomprisBoard.level - 1)
    self.displayLesson( self.currentLesson )

# -------

  def clearLesson(self):
    self.lessonroot.remove()


  def displayLesson(self, lesson):

    try:
      self.lessonroot.remove()
    except:
      pass

    self.currentTripletId = 0
    self.lessonroot = goocanvas.Group( parent = self.rootitem )

    goocanvas.Rect(
      parent = self.lessonroot,
      x = 20,
      y = 10,
      width = gcompris.BOARD_WIDTH - 40,
      height = 65,
      fill_color_rgba = 0x6666FF33L,
      stroke_color_rgba = 0x1111FFAAL,
      line_width = 2.0,
      radius_x = 0.9,
      radius_y = 0.9)

    goocanvas.Text(
      parent = self.lessonroot,
      x = gcompris.BOARD_WIDTH / 2,
      y = 25.0,
      text = lesson.name,
      fill_color = "black",
      font = gcompris.skin.get_font("gcompris/title"),
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    goocanvas.Text(
      parent = self.lessonroot,
      x = gcompris.BOARD_WIDTH / 2,
      y = 55.0,
      text = lesson.description,
      fill_color = "black",
      font = gcompris.skin.get_font("gcompris/subtitle"),
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    # Previous Button
    item = goocanvas.Svg( parent = self.lessonroot,
                        svg_handle = gcompris.skin.svg_get(),
                        svg_id = "#PREVIOUS",
                        )
    gcompris.utils.item_absolute_move(item,
                                      100,
                                      gcompris.BOARD_HEIGHT / 2)
    item.connect("button_press_event", self.previous_event, None)
    gcompris.utils.item_focus_init(item, None)

    # Next Button
    item = goocanvas.Svg(parent = self.lessonroot,
                       svg_handle = gcompris.skin.svg_get(),
                       svg_id = "#NEXT",
                       )
    gcompris.utils.item_absolute_move(item,
                                      gcompris.BOARD_WIDTH - 130,
                                      gcompris.BOARD_HEIGHT / 2)
    item.connect("button_press_event", self.next_event, None)
    gcompris.utils.item_focus_init(item, None)

    self.counteritem = goocanvas.Text(
      parent = self.lessonroot,
      x = gcompris.BOARD_WIDTH - 40,
      y = gcompris.BOARD_HEIGHT - 40,
      fill_color = "black",
      font = gcompris.skin.get_font("gcompris/board/tiny"),
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    # The triplet area
    self.missingImage = MissingImage(self.lessonroot, self)
    self.imageitem = goocanvas.Image( parent = self.lessonroot )
    self.imageitem.connect("button_press_event", self.next_event, None)
    self.descriptionitem = goocanvas.Text(
      parent = self.lessonroot,
      x = gcompris.BOARD_WIDTH / 2,
      y = gcompris.BOARD_HEIGHT - 100,
      fill_color = "black",
      font = gcompris.skin.get_font("gcompris/subtitle"),
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )
    self.displayImage( lesson.getTriplets()[self.currentTripletId] )

  def playVoice(self, triplet):
    if triplet.voice:
      gcompris.sound.play_ogg("voices/$LOCALE/" + triplet.voice)

  def displayImage(self, triplet):
    self.playVoice( triplet )
    self.descriptionitem.set_properties (
      text = triplet.description,
      )
    self.counteritem.set_properties (
      text = str(self.currentTripletId + 1) + "/" \
        + str( len( self.currentLesson.getTriplets() ) ),
      )
    if triplet.image:
      self.missingImage.hide()
      self.imageitem.props.visibility = goocanvas.ITEM_VISIBLE
      pixbuf = gcompris.utils.load_pixmap(gcompris.DATA_DIR + "/lang/" +
                                          triplet.image)
      center_x =  pixbuf.get_width()/2
      center_y =  pixbuf.get_height()/2
      self.imageitem.set_properties(pixbuf = pixbuf,
                                    x = gcompris.BOARD_WIDTH  / 2 - center_x,
                                    y = gcompris.BOARD_HEIGHT / 2 - center_y )
    else:
      self.imageitem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.missingImage.show(triplet)

  def previous_event(self, event, target,item, dummy):
    self.currentTripletId -= 1
    if self.currentTripletId < 0:
      self.currentTripletId = len( self.currentLesson.getTriplets() ) - 1
    self.displayImage( self.currentLesson.getTriplets()[self.currentTripletId] )

  def next_event(self, event, target, item, dummy):
    self.currentTripletId += 1
    if self.currentTripletId >= len( self.currentLesson.getTriplets() ):
      self.currentTripletId = 0
    self.displayImage( self.currentLesson.getTriplets()[self.currentTripletId] )




#  gcompris - findit.py
#
# Copyright (C) 2011 Bruno Coudoin
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
# FindIt Activity
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.sound
import goocanvas
import pango

from gcompris import gcompris_gettext as _

import ConfigParser
import random

class Gcompris_findit:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    self.rootitem = None
    self.dataset = None
    self.datasetlevel = None
    self.selected = None
    self.object_target = None
    self.question_lost = []
    self.question_text_item = None
    self.current_background = None

  def start(self):
    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)

    if self.read_data():
      data = self.dataset.get_level(1, 1)
      if not data:
        return

      self.gcomprisBoard.level = 1
      self.gcomprisBoard.maxlevel = self.dataset.number_of_level()
      self.gcomprisBoard.sublevel = 1;
      gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT)
      gcompris.bar_set_level(self.gcomprisBoard)
      gcompris.sound.policy_set(gcompris.sound.PLAY_AND_INTERRUPT)
      self.play(data)


  def end(self):
    # Remove the root item removes all the others inside it
    if self.rootitem:
      self.rootitem.remove()
      self.rootitem = None

  def repeat(self):
    self.play_audio_question(self.datasetlevel.question_audio, self.object_target)


  def pause(self, pause):
    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(self.gamewon == 1 and pause == 0):

      # First check if there are more question to ask at this level
      self.object_target = self.get_next_question()
      if self.object_target:
        self.display_question(self.datasetlevel, self.object_target)
      else:
        [self.gcomprisBoard.level,
         self.gcomprisBoard.sublevel] = self.dataset.get_next_level(self.gcomprisBoard.level,
                                                                    self.gcomprisBoard.sublevel)
        gcompris.bar_set_level(self.gcomprisBoard)
        self.play(self.dataset.get_level(self.gcomprisBoard.level,
                                         self.gcomprisBoard.sublevel))
      self.gamewon = 0
      self.selected = None

    return


  def set_level(self, level):
    self.gcomprisBoard.level = level;
    self.gcomprisBoard.sublevel = 1;
    gcompris.bar_set_level(self.gcomprisBoard)
    self.play(self.dataset.get_level(self.gcomprisBoard.level,
                                     self.gcomprisBoard.sublevel))

  def play(self, datasetlevel):
    # Set a background image if it has changed
    if self.current_background != datasetlevel.background:
      gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                              gcompris.DATA_DIR + '/'
                              + self.gcomprisBoard.name + "/"
                              + datasetlevel.background)
      self.current_background = datasetlevel.background

    if self.rootitem:
      self.rootitem.remove()
    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())


    # Get a random question
    self.datasetlevel = datasetlevel
    datasetlevel.sort_objects()
    #datasetlevel.dump()
    self.current_question = -1
    self.question_lost = []

    self.question_text_item = goocanvas.Text(
      parent = self.rootitem,
      text = "",
      font = gcompris.skin.get_font(datasetlevel.question_font),
      x = datasetlevel.question_position[0],
      y = datasetlevel.question_position[1],
      fill_color_rgba = datasetlevel.question_color,
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    x = datasetlevel.object_area_xywh[0]
    y = datasetlevel.object_area_xywh[1]
    w = datasetlevel.object_area_xywh[2]
    h = datasetlevel.object_area_xywh[3]
    # Display the area into which we will display the objects
    if datasetlevel.object_area_fill or datasetlevel.object_area_stroke:
      goocanvas.Rect(
        parent = self.rootitem,
        x = x,
        y = y,
        width = w,
        height = h,
        fill_color_rgba = datasetlevel.object_area_fill,
        stroke_color_rgba = datasetlevel.object_area_stroke,
        radius_x = datasetlevel.object_area_radius_x,
        radius_y = datasetlevel.object_area_radius_x,
        line_width = datasetlevel.object_area_line_width
        )

    gap = datasetlevel.gap
    ix = x
    iy = y
    y_max = 0
    for object_source in datasetlevel.objects:
      item = object_source.create_item(self.rootitem,
                                       self.gcomprisBoard.name,
                                       0, 0)
      bounds = item.get_bounds()
      width = bounds.x2 - bounds.x1
      height = bounds.y2 - bounds.y1
      # Manage line wrapping
      if ix + width + gap > x + w:
        ix = x
        iy += y_max + gap
      item.props.x = ix
      item.props.y = iy
      item.connect("button_press_event", self.item_event, object_source)
      # This item is clickeable and it must be seen
      gcompris.utils.item_focus_init(item, None)
      ix += width + gap
      y_max = max(y_max, height)


    # The OK Button
    item = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#OK"
                         )
    zoom = 1
    item.translate( (item.get_bounds().x1 * -1)
                     + datasetlevel.ok_position[0] / zoom,
                    (item.get_bounds().y1 * -1)
                     + datasetlevel.ok_position[1] / zoom )
    item.scale(zoom, zoom)
    item.connect("button_press_event", self.ok_event, None)
    gcompris.utils.item_focus_init(item, None)

    self.datasetlevel.sort_objects()
    self.object_target = self.get_next_question()
    self.display_question(self.datasetlevel, self.object_target)

  def play_audio_question(self, question, object_):
    # Play the audio question if provided
    audio = question
    try:
      # The question audio can be formatted with {text} and if so this
      # is replaced by the target text
      audio = audio.format(text = object_.text)
    except:
      pass

    try:
      # The question audio can be formatted with {audio} and if so this
      # is replaced by the target text
      audio = audio.format(audio = object_.audio)
    except:
      pass

    if audio:
      gcompris.sound.play_ogg(
        gcompris.utils.find_file_absolute(audio) )


  # Return the next question or None if any
  def get_next_question(self):
    if (self.current_question < len(self.datasetlevel.objects) - 1 ):
      self.current_question += 1
      return self.datasetlevel.objects[self.current_question - 1]

    # All the question have been answered, now we return the question
    # where the children lost
    if self.question_lost:
      return self.question_lost.pop(0)

    # No more questions
    return None

  def display_question(self, datasetlevel, object_target):
    # The question
    text = datasetlevel.question_text
    try:
      # The question can be formatted with {text} and if so this
      # is replaced by the target name
      text = _(text.format(text = _(object_target.text)))
    except:
      pass

    try:
      # The question can be formatted with {audio} and if so this
      # is replaced by the target name
      text = _(text.format(audio = _(object_target.audio)))
    except:
      pass

    if self.selected:
      self.selected.select(False)

    self.question_text_item.props.text = text
    self.play_audio_question(datasetlevel.question_audio, object_target)

  def ok_event(self, widget, target, event, dummy):
    if self.selected:
      if (self.selected.text == self.object_target.text):
        print "WON " + self.selected.text
        self.gamewon = 1
        gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)
      else:
        print "LOST: " +  self.selected.text + " != " + self.object_target.text
        if not self.object_target in self.question_lost:
          self.question_lost.append( self.object_target )
        self.gamewon = 0
        gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.FLOWER)

  #
  def item_event(self, item, target, event, object_source):
    if self.selected:
      self.selected.select(False)
    self.selected = object_source
    self.selected.select(True)
    self.play_audio_question(object_source.audio, object_source)

    if (object_source.text == self.object_target.text):
      print "WON " + object_source.text
    else:
      print "LOST: " + object_source.text + " != " + self.object_target.text

  def read_data(self):
    '''Load the activity data, return True if OK'''
    config = ConfigParser.RawConfigParser()
    p = gcompris.get_properties()
    filename = gcompris.DATA_DIR + '/' + self.gcomprisBoard.name + \
        '/activity.desktop'
    try:
      gotit = config.read(filename)
      if not gotit:
        gcompris.utils.dialog(_("Cannot find the file '{filename}'").\
                                format(filename=filename),
                              None)
        return False
    except ConfigParser.Error as error:
      gcompris.utils.dialog(_("Failed to parse data set '{filename}'"
                              " with error:\n{error}").\
                              format(filename=filename, error=error),
                            None)
      return False

    self.dataset = finditDataSet(config)
    return True

class finditDataSet:
  '''The findit activity data structure'''

  def __init__(self, dataset):
    '''Creation from a ConfigParser object'''

    # Here we keep all the user level indexed by the key 'level'
    # in a list of level. The config file can contain multiple level 1
    # which will be seen as sublevels in GCompris.
    self.levels = {}
    for section in dataset.sections():
      if section == 'common': continue
      level = self._level(section)
      print "processing level " + str(level)
      if not level in self.levels:
        self.levels[level] = []
      self.levels[level].append( finditDataSetLevel(dataset, section) )
      self.levels[level] =  \
          sorted(self.levels[level],
                 key=lambda dataSetLevel: dataSetLevel.sublevel)

  def number_of_level(self):
    return len(self.levels)

  def number_of_sublevel(self, level):
    if level in self.levels:
      return len(self.levels[level])
    return 0

  def get_next_level(self, level, sublevel):
    '''Return the next [level, sublevel] or the current one if any'''
    sublevel += 1
    dataSetLevel = self.get_level(level, sublevel)
    if dataSetLevel: return [level, sublevel]

    level += 1
    sublevel = 1
    dataSetLevel = self.get_level(level, sublevel)
    if dataSetLevel: return [level, sublevel]

    # No more level, return the same one
    return [level - 1, sublevel - 1]

  def get_level(self, level, sublevel):
    '''Return a finditDataSetLevel'''
    try:
      if level in self.levels:
        return self.levels[level][sublevel - 1]
      else:
        return None
    except:
      return None

  def _level(self, section):
      return int(section.split(".")[0])

  def dump(self):
    for levels in self.levels.values():
      for level in levels:
        level.dump()

class finditDataSetLevel:
  '''The structure for a single level'''

  def __init__(self, dataset, section):
    '''Creation from a ConfigParser object'''

    self.level = int(section.split(".")[0])
    if len(section.split(".")) == 2:
      self.sublevel = int(section.split(".")[1])
    else:
      self.sublevel = 1
    self.background = load_common_prop(dataset, section, "background", "")

    self.object_area_xywh = \
        map(lambda x: int(x) ,
            load_common_prop(dataset, section, "objectAreaXYWH", "").split(','))
    self.object_area_fill = long(load_common_prop(dataset, section, "objectAreaFill", "0"), 16)
    self.object_area_stroke = long(load_common_prop(dataset, section, "objectAreaStroke", "0"), 16)
    self.object_area_radius_x = int(load_common_prop(dataset, section, "objectAreaRadiusX", "0"))
    self.object_area_radius_y = int(load_common_prop(dataset, section, "objectAreaRadiusY", "0"))
    self.object_area_line_width = int(load_common_prop(dataset, section, "objectAreaLineWidth", "0"))

    self.question_position = \
        map(lambda x: int(x) ,
            load_common_prop(dataset, section, "questionPosition", "").split(','))

    try:
      self.question_text = load_common_prop(dataset, section, "questionText", "")
    except:
      self.question_text = None

    print "     finditDataSetLevel " + self.question_text
    try:
      self.question_audio = load_common_prop(dataset, section, "questionAudio", "")
    except:
      self.question_audio = None

    self.question_color = long(load_common_prop(dataset, section, "questionColor", "0"), 16)
    self.question_font = load_common_prop(dataset, section, "questionFont", "")

    self.ok_position = \
        map(lambda x: int(x) ,
            load_common_prop(dataset, section, "okPosition", "").split(','))

    self.gap = long(load_common_prop(dataset, section, "objectGap", "20"), 10)

    i = 1
    self.objects = []
    while True:
      try:
        dataset.get(section, "object" + str(i) + "Image")
        self.objects.append(finditDataSetObject(dataset, section, i))
      except Exception as inst:
        print type(inst)     # the exception instance
        print inst.args
        print inst
        break
      i += 1
    # Shuffle the object
    self.sort_objects()

  def sort_objects(self):
    for object_ in self.objects:
      object_.init_randint()
    # Shuffle the object
    self.objects = sorted(self.objects,
                          key=lambda dataSetObject: dataSetObject.randint)

  def dump(self):
    print "Level      = " + str(self.level) + "." + str(self.sublevel)
    print "  Background = " + self.background
    print "  Area"
    print "    xywh    = " + str(self.object_area_xywh)
    print "    stroke  = " + hex(self.object_area_stroke)
    for objects in self.objects:
        objects.dump()

class finditDataSetObject:
  '''The structure for a single object'''

  def __init__(self, dataset, level, index):
    self.index = index
    self.image = self._load_prop(dataset, level, index, "Image")
    self.text  = self._load_prop(dataset, level, index, "Text")
    self.audio = self._load_prop(dataset, level, index, "Audio")
    self.selection_fill = long(load_common_prop(dataset, level, "objectSelectionFill", "0"), 16)
    self.selection_stroke = long(load_common_prop(dataset, level, "objectSelectionStroke", "0"), 16)
    self.text_color = load_common_prop(dataset, level, "textColor", "")
    if self.text_color:
      self.text_color =  long(self.text_color, 16)
    self.text_font = load_common_prop(dataset, level, "textFont", "")
    self.rootitem = None
    self.item = None
    self.selection = None
    # Some random key to allow the sorting
    self.randint = 0
    print "     finditDataSetObject " + self.text

  def init_randint(self):
    self.randint = random.randint(0, 100)

  def _load_prop(self, dataset, level, index, suffix):
    try:
      return dataset.get(level, "object" + str(index) + suffix)
    except:
      return None

  def create_item(self, rootitem, board_name, x, y):
    filename = gcompris.DATA_DIR + '/' \
          + board_name + "/" + self.image
    pixbuf = gcompris.utils.load_pixmap(filename)
    self.rootitem = goocanvas.Group(parent = rootitem)
    self.item = goocanvas.Image(parent = self.rootitem,
                           pixbuf = pixbuf,
                           x = x,
                           y = y
                           )
    bounds = self.item.get_bounds()
    width = bounds.x2 - bounds.x1
    height = bounds.y2 - bounds.y1

    if (self.text_color and self.text_font):
      goocanvas.Text(
        parent = self.rootitem,
        text = _(self.text),
        x = x + width / 2,
        y = y + height / 2,
        font = gcompris.skin.get_font(self.text_font),
        fill_color_rgba = self.text_color,
        anchor = gtk.ANCHOR_CENTER,
        alignment = pango.ALIGN_CENTER
        )

    self.selection = goocanvas.Rect(
      parent = self.rootitem,
      x = x,
      y = y,
      width = width,
      height = height,
      fill_color_rgba = self.selection_fill,
      stroke_color_rgba = self.selection_stroke,
      radius_x = 5,
      radius_y = 5,
      line_width = 1
      )
    self.selection.lower(self.item)
    self.selection.props.visibility = goocanvas.ITEM_INVISIBLE

    return self.rootitem

  def select(self, state):
    if self.selection:
      if state:
        self.selection.props.visibility = goocanvas.ITEM_VISIBLE
      else:
        self.selection.props.visibility = goocanvas.ITEM_INVISIBLE

  def dump(self):
    print "  Object " + str(self.index) + " " + str(self.randint)
    if self.image:
      print "    Image " + self.image
    if self.text:
      print "    Text  " + self.text
    if self.audio:
      print "    Audio " + self.audio


# Load the requested property_ first from the level section
# then from the common section if not found.
def load_common_prop(dataset, level, property_, default):
  try:
    return dataset.get(level, property_)
  except:
    try:
      return dataset.get("common", property_)
    except:
      return default


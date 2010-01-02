#  gcompris - redraw
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
from gcompris import gcompris_gettext as _
# PythonTest Board module
import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.sound
import gtk
import gtk.gdk
import copy
import math

class Gcompris_redraw:
  """The Re-drawing activity"""


  def __init__(self, gcomprisBoard):

    self.gcomprisBoard = gcomprisBoard

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;

    # TOOL SELECTION
    self.tools = [
      ["RECT",           "anim/tool-rectangle.png",       "anim/tool-rectangle_on.png",          gcompris.CURSOR_RECT],
      ["FILL_RECT",      "anim/tool-filledrectangle.png", "anim/tool-filledrectangle_on.png",    gcompris.CURSOR_FILLRECT],
      ["CIRCLE",         "anim/tool-circle.png",          "anim/tool-circle_on.png",             gcompris.CURSOR_CIRCLE],
      ["FILL_CIRCLE",    "anim/tool-filledcircle.png",    "anim/tool-filledcircle_on.png",       gcompris.CURSOR_FILLCIRCLE],
      ["LINE",           "anim/tool-line.png",            "anim/tool-line_on.png",               gcompris.CURSOR_LINE],
      ["DEL",            "anim/tool-del.png",             "anim/tool-del_on.png",                gcompris.CURSOR_DEL],
      ["FILL",           "anim/tool-fill.png",            "anim/tool-fill_on.png",               gcompris.CURSOR_FILL],
      ["SELECT",         "anim/tool-select.png",          "anim/tool-select_on.png",             gcompris.CURSOR_SELECT]
      ]

    self.current_tool=0

    # COLOR SELECTION
    self.color_fill = 0xA0
    self.color_stroke = 0xFF
    self.colors = [ 0x00000000L,
                    0xFF000000L,
                    0xFFFF0c00L,
                    0xB9BC0D00L,
                    0x33FF0000L,
                    0x83189100L,
                    0xC2C2C200L,
                    0x1010FF00L]

    self.current_color = 0
    self.current_step = 0

    self.current_drawing = []
    self.image_target    = []

    # Define the coord of each drawing area
    self.drawing_area = [140.0, 20.0, 420.0, 500.0]
    self.target_area  = [440.0, 20.0, 720.0, 500.0]

    # Global used for the select event
    self.in_select_ofx = -1
    self.in_select_ofy = -1

    # The error root item
    self.root_erroritem = []

    # Set it to 1 to let you create new forms
    # Once set, draw your shape in the right area. Then clic on OK to display
    # the data for the form (in the console). Then copy the data in the list at the end of
    # this file in init_item_list
    # Set self.editing_mode = None to forbid drawing in the right area
    self.editing_mode = None
    #self.editing_mode = 1


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.set_default_background(self.gcomprisBoard.canvas.get_root_item())
    gcompris.bar_set_level(self.gcomprisBoard)
    gcompris.bar_location(4, -1, 0.6)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

    item = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap(self.gcomprisBoard.name + "/" +
                                          self.gcomprisBoard.icon_name),
      )
    bounds = item.get_bounds()
    scale = 60.0 / (bounds.x2 - bounds.x1)
    item.scale(scale, scale)
    item.set_properties(x = (gcompris.BOARD_WIDTH - 65) / scale,
                        y = 5 / scale)

    self.draw_tools()
    self.draw_colors()
    self.draw_drawing_area(20)

    self.init_item_list()
    self.display_current_level()

    self.pause(0);

  def end(self):
    # Remove the root item removes all the others inside it
    gcompris.set_cursor(gcompris.CURSOR_DEFAULT);
    self.rootitem.remove()
    self.root_drawingitem.remove()
    self.root_targetitem.remove()
    # Delete error previous mark if any
    if(self.root_erroritem):
      self.root_erroritem.remove()



  def pause(self, pause):

    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(self.gamewon == 1 and pause == 0):
      self.root_targetitem.props.visibility = goocanvas.ITEM_VISIBLE
      self.gamewon = 0

    return


  def ok(self):
    # Save a copy of the target drawing future comparison
    target  = list(self.image_target)
    target2 = list(self.image_target)

    # Save a copy of the drawing cause we need to remove empty items
    source = list(self.current_drawing)

    # Remove all empty items
    while 1:
      try:
        source.remove ([])
      except:
        break

    if self.editing_mode != None :
      print("To add item in this activity, Copy the following data in init_item_list in redraw.py (near the end)")
      print("-------------------------------------------------------------------------------")
      print source
      print("-------------------------------------------------------------------------------")

    # Need to check if target image equals drawing image
    for i in source:
      for j in target:
        if i == j:
          target.remove(j)

    for i in target2:
      for j in source:
        if i == j:
          source.remove(j)

    if(len(target) == 0 and len(source) == 0):
      # This is a WIN
      self.erase_drawing_area()
      self.increment_level()
      self.gamewon = 1
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)
      self.display_current_level()
      self.root_targetitem.props.visibility = goocanvas.ITEM_INVISIBLE

    else:
      # Delete previous mark if any
      if(self.root_erroritem):
        self.root_erroritem.remove()

      # Create our rootitem for error items mark
      self.root_erroritem = goocanvas.Group(
        parent = self.gcomprisBoard.canvas.get_root_item())

      self.display_error(target, 1)
      self.display_error(source, 0)


  # display where there is errors
  # if in_target is set then error are displayed in the target area
  def display_error(self, target, in_target):

    # Bad Icon Width and Height / 2
    icw=8
    ich=8

    if self.gcomprisBoard.mode == 'symmetrical' and in_target:
      target = self.get_symmetry(target)

    # The images target are always drawn on the drawing area to ease the final comparison
    if in_target:
      xofset = self.target_area[0] - self.drawing_area[0]
    else:
      xofset = 0

    for t in target:
      if(t.has_key('points')):
        goocanvas.Image(
          parent = self.root_erroritem,
          pixbuf = gcompris.utils.load_pixmap("redraw/mini_bad.png"),
          x = t['points'][0] + (t['points'][2]-t['points'][0])/2 - icw + xofset,
          y = t['points'][1] + (t['points'][3]-t['points'][1])/2 -ich
          )
      elif(t.has_key('width')):
        goocanvas.Image(
          parent = self.root_erroritem,
          pixbuf = gcompris.utils.load_pixmap("redraw/mini_bad.png"),
          x = t['x'] + t['width']/2 - icw + xofset,
          y = t['y'] + t['height']/2 - ich
          )
      elif(t.has_key('radius_x')):
        goocanvas.Image(
          parent = self.root_erroritem,
          pixbuf = gcompris.utils.load_pixmap("redraw/mini_bad.png"),
          x = t['center_x'] -icw + xofset,
          y = t['center_y'] -ich
          )


  # Called by gcompris when the user click on the level icon
  def set_level(self, level):
    self.gcomprisBoard.level=level;
    self.gcomprisBoard.sublevel=1;

    self.erase_drawing_area()
    self.display_current_level()

  def repeat(self):
    pass


  def config(self):
    pass

  def key_press(self, keyval, commit_str, preedit_str):
    return False

  # Erase any displayed items (drawing and target)
  def erase_drawing_area(self):
    self.root_targetitem.remove()
    self.root_drawingitem.remove()
    if(self.root_erroritem):
      self.root_erroritem.remove()

  # Display the current level target
  def display_current_level(self):
    # Calc the index in drawlist
    i = (self.gcomprisBoard.level-1) * self.gcomprisBoard.number_of_sublevel+ \
        (self.gcomprisBoard.sublevel-1)

    if(i >= len(self.drawlist)):
      # Wrap to the first level when completed
      i = 0

    # Set the level in the control bar
    gcompris.bar_set_level(self.gcomprisBoard);

    self.draw_image_target(self.drawlist[i])

    self.display_sublevel()

    # Prepare an item for the coord display
    self.coorditem = goocanvas.Text(
      parent = self.root_targetitem,
      font = gcompris.skin.get_font("gcompris/content"),
      x = gcompris.BOARD_WIDTH / 2,
      y = gcompris.BOARD_HEIGHT - 20,
      fill_color_rgba = 0x000000FFL,
      anchor = gtk.ANCHOR_NE
      )

    # Create our rootitem for drawing items
    self.root_drawingitem = goocanvas.Group(
      parent = self.gcomprisBoard.canvas.get_root_item())

    # Reset the drawing
    self.current_drawing = []

  # Code that increments the sublevel and level
  # And stays at the last level
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      if(self.gcomprisBoard.level>self.gcomprisBoard.maxlevel) or self.gcomprisBoard.level*self.gcomprisBoard.sublevel>=len(self.drawlist):
        self.gcomprisBoard.level = self.gcomprisBoard.maxlevel


  # display current/sublevel number
  def display_sublevel(self):

    goocanvas.Text(
      parent = self.root_targetitem,
      text = _("Level") + " " + str(self.gcomprisBoard.sublevel) + "/" \
        + str(self.gcomprisBoard.number_of_sublevel),
      font = gcompris.skin.get_font("gcompris/content"),
      x = gcompris.BOARD_WIDTH - 10,
      y = gcompris.BOARD_HEIGHT - 20,
      fill_color_rgba = 0x000000FFL,
      anchor = gtk.ANCHOR_NE
      )

  # Display the tools
  def draw_tools(self):

    goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("anim/tool-selector.png"),
      x=5,
      y=5.0,
      width=30.0
      )

    x1=11.0
    x2=56.0
    y=30.0
    stepy=45

    # Display the tools
    for i in range(0,len(self.tools)):
      if(i%2 == 0):
        theX = x2
      else:
        theX = x1

      item = goocanvas.Image(
        parent = self.rootitem,
        pixbuf = gcompris.utils.load_pixmap(self.tools[i][1]),
        x = theX,
        y = y
        )
      item.connect("button_press_event", self.tool_item_event, i)
      if i%2:
        y += stepy

    # The last item is select, we select it by default
    self.current_tool = i
    self.old_tool_item = item
    self.old_tool_item.props.pixbuf = gcompris.utils.load_pixmap(self.tools[i][2])
    gcompris.set_cursor(self.tools[i][3]);

    # The OK Button
    item = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#OK"
                         )
    item.translate(item.get_bounds().x1 * -1
                   + 20,
                   item.get_bounds().y1 * -1
                   + gcompris.BOARD_HEIGHT - 100)

    item.connect("button_press_event", self.ok_event)
    gcompris.utils.item_focus_init(item, None)

    gcompris.bar_set_level(self.gcomprisBoard)


  # Event when a tool is selected
  def tool_item_event(self, item, target, event, tool):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/bleep.wav")
        # Deactivate old button
        self.old_tool_item.props.pixbuf = gcompris.utils.load_pixmap(self.tools[self.current_tool][1])

        # Activate new button
        self.current_tool = tool
        self.old_tool_item = item
        self.old_tool_item.props.pixbuf = gcompris.utils.load_pixmap(self.tools[self.current_tool][2])
        gcompris.set_cursor(self.tools[self.current_tool][3]);

  # Display the color selector
  def draw_colors(self):

    x1=13.0
    x2=59.0
    y=230.0
    stepy=45

    # Display the tools
    for i in range(0,len(self.colors)):
      if(i%2):
        theX = x2
      else:
        theX = x1

      item = goocanvas.Rect(
        parent = self.rootitem,
        fill_color_rgba = self.colors[i] | self.color_stroke,
        x=theX,
        y=y,
        width=30,
        height=30,
        line_width=0.0,
        stroke_color_rgba= 0x144B9DFFL
        )
      item.connect("button_press_event", self.color_item_event, i)
      if i%2:
        y += stepy

    # The last item is the one we select by default
    self.current_color = i
    self.old_color_item = item
    self.old_color_item.props.line_width = 4.0
    self.old_color_item.props.stroke_color_rgba = 0x16EC3DFFL

  # Color event
  def color_item_event(self, item, target, event, color):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/drip.wav")
        # Deactivate old button
        self.old_color_item.props.line_width = 0.0
        self.old_color_item.props.stroke_color_rgba = 0x144B9DFFL

        # Activate new button
        self.current_color = color
        self.old_color_item = item
        self.old_color_item.props.line_width = 4.0
        self.old_color_item.props.stroke_color_rgba= 0x16EC3DFFL


  # Display the drawing area
  def draw_drawing_area(self, step):

    self.current_step = step

    x1=self.drawing_area[0]
    y1=self.drawing_area[1]
    x2=self.drawing_area[2]
    y2=self.drawing_area[3]

    item = goocanvas.Rect(
      parent = self.rootitem,
      x = x1,
      y = y1,
      width = x2 - x1,
      height = y2 - y1,
      fill_color_rgba = 0xF0F0F0FFL,
      line_width = 2.0,
      stroke_color_rgba = 0x111199FFL
      )
    item.connect("button_press_event", self.create_item_event)
    item.connect("button_release_event", self.create_item_event)
    item.connect("motion_notify_event", self.create_item_event)
    item.connect("leave_notify_event", self.target_item_event)

    self.draw_grid(x1,x2,y1,y2,step)

    x1=self.target_area[0]
    y1=self.target_area[1]
    x2=self.target_area[2]
    y2=self.target_area[3]

    item =goocanvas.Rect(
      parent = self.rootitem,
      x=x1,
      y=y1,
      width=x2-x1,
      height=y2-y1,
      fill_color_rgba=0xF0F0F0FFL,
      line_width=2.0,
      stroke_color_rgba=0x111199FFL
      )
    item.connect("motion_notify_event", self.target_item_event)
    item.connect("leave_notify_event", self.target_item_event)

    self.draw_grid(x1,x2,y1,y2,step)

    #
    # Given coord are returned swapped
    # Work fine for rect and ellipse but not line
    # so that y2 > y1 and x2 > x1
    #
  def reorder_coord(self, x1, y1, x2, y2):
    p = [x1, y1, x2, y2]
    if(x1>x2):
      p[0] = x2
      p[2] = x1
    if(y1>y2):
      p[1] = y2
      p[3] = y1
    return p


  #
  # Take a drawing and return a symmetrical one
  #
  def get_symmetry(self, drawing_source):
    # Make a deepcopy of the list
    drawing  = copy.deepcopy(drawing_source)

    xofset = 0

    for item in drawing:

      for k, v in item.items():

        if k == 'points' :
          item[k] = (self.drawing_area[2] - (v[0] - self.drawing_area[0])+xofset, v[1],
                     self.drawing_area[2] - (v[2] - self.drawing_area[0])+xofset, v[3])
        elif k == 'x':
          item[k] = self.drawing_area[2] - (v - self.drawing_area[0])  + xofset - item['width']
        elif k == 'center_x':
          item[k] = self.drawing_area[2] - (v - self.drawing_area[0])  + xofset

    return drawing

  # Draw the image target
  # depending on self.gcomprisBoard.level drawing is 'normal' or 'symmetric'
  #
  def draw_image_target(self, drawing):

    # Save the drawing in image_target for future comparison
    self.image_target = drawing

    # Create our rootitem for target items
    self.root_targetitem = goocanvas.Group(
      parent = self.gcomprisBoard.canvas.get_root_item())

    if self.gcomprisBoard.mode == 'symmetrical':
      drawing = self.get_symmetry(drawing)

    # The images target are always drawn on the drawing area to ease the final comparison
    xofset = self.target_area[0] - self.drawing_area[0]
    for i in drawing:
      #
      # Can specify the item type to draw via a real GTK type or a TOOL string
      if(i.has_key('type')):
        item =  i['type']();
        self.root_targetitem.add_child (item, -1)
        item.connect("motion_notify_event", self.target_item_event)

      elif(i.has_key('tool')):
        if(i['tool'] == "RECT"):
          item =  goocanvas.Rect();
          self.root_targetitem.add_child (item, -1)
        elif(i['tool'] == "FILL_RECT"):
          item =  goocanvas.Rect();
          self.root_targetitem.add_child (item, -1)
        elif(i['tool'] == "CIRCLE"):
          item =  goocanvas.Ellipse();
          self.root_targetitem.add_child (item, -1)
        elif(i['tool'] == "FILL_CIRCLE"):
          item =  goocanvas.Ellipse();
          self.root_targetitem.add_child (item, -1)
        elif(i['tool'] == "LINE"):
          item =  goocanvas.Polyline();
          self.root_targetitem.add_child (item, -1)
        else:
          print ("ERROR: incorrect type in draw_image_target", i)
        item.connect("motion_notify_event", self.target_item_event)

      for k, v in i.items():
        if k == 'fill_color' :
          item.props.fill_color = v
        elif k == 'fill_color_rgba' :
          item.props.fill_color_rgba = v
        elif k == 'height' :
          item.props.height = v
        elif k == 'stroke_color' :
          item.props.stroke_color = v
        elif k == 'stroke_color_rgba' :
          item.props.stroke_color_rgba = v
        elif k == 'points' :
          item.props.points = goocanvas.Points([(v[0]+xofset, v[1]),
                                                (v[2]+xofset, v[3])])
        elif k == 'line_width' :
          item.props.line_width = v
        elif k == 'width_pixels' :
          item.props.width_pixels = v
        elif k == 'width' :
          item.props.width = v
        elif k == 'x' :
          item.props.x = v + xofset
        elif k == 'y' :
          item.props.y = v
        elif k == 'center_x' :
          item.props.center_x = v + xofset
        elif k == 'center_y' :
          item.props.center_y = v
        elif k == 'radius_x' :
          item.props.radius_x = v
        elif k == 'radius_y' :
          item.props.radius_y = v

  #
  # Draw the grid
  #
  def draw_grid(self, x1, x2, y1, y2, step):

    # Coord of the written numbers
    if(x1<self.target_area[0]):
      x_text = x1 - 25
    else:
      x_text = x2 + 5

    y_text = y1 - 15

    # We manage a 2 colors grid
    ci = 0
    ca = 0x1D0DFFFFL
    cb = 0xEEAAAAFFL

    for i in range(int(x1), int(x2), int(step)):
      if(ci%2):
        color = ca
      else:
        color = cb
      ci += 1

      item = goocanvas.Polyline(
        parent = self.rootitem,
        points = goocanvas.Points([(i , y1), (i , y2)]),
        stroke_color_rgba = color,
        line_width = 1.0,
        )
      # Text number
      goocanvas.Text(
        parent = self.rootitem,
        text = int((i-x1) / step),
        font = gcompris.skin.get_font("gcompris/board/tiny"),
        x = i,
        y = y_text,
        fill_color_rgba = 0x000000FFL
        )

      # Clicking on lines let you create object
      if(x1<self.target_area[0]):
        item.connect("button_press_event", self.create_item_event)
        item.connect("button_release_event", self.create_item_event)
        item.connect("motion_notify_event", self.create_item_event)
      else:
        item.connect("motion_notify_event", self.target_item_event)

    for i in range(int(y1), int(y2), int(step)):
      if(ci%2):
        color = ca
      else:
        color = cb
      ci += 1

      item = goocanvas.Polyline(
        parent = self.rootitem,
        points = goocanvas.Points([(x1, i), (x2 , i)]),
        stroke_color_rgba = color,
        line_width = 1.0,
        )

      # Text number
      goocanvas.Text(
        parent = self.rootitem,
        text = int((i-y1) / step),
        font = gcompris.skin.get_font("gcompris/board/tiny"),
        x = x_text,
        y = i,
        fill_color_rgba=0x000000FFL
        )

      # Clicking on lines let you create object
      if(x1<self.target_area[0]):
        item.connect("button_press_event", self.create_item_event)
        item.connect("button_release_event", self.create_item_event)
        item.connect("motion_notify_event", self.create_item_event)

  # Given x,y return a new x,y snapped to the grid
  def snap_to_grid(self, x, y):
    result = []
    tmp = round(((x+(self.current_step)) -
               self.drawing_area[0])/self.current_step) - 1
    result.append(float(self.drawing_area[0] + tmp*self.current_step))

    tmp = round(((y+(self.current_step)) -
               self.drawing_area[1])/self.current_step) - 1
    result.append(float(self.drawing_area[1] + tmp*self.current_step))
    return result


  # Event when a click on any item. Perform the move
  def move_item_event(self, item, target, event, item_index):

    if self.tools[self.current_tool][0] != "SELECT":
      return False

    if (event.type == gtk.gdk.BUTTON_PRESS
        and event.button == 1):
      gcompris.sound.play_ogg("sounds/smudge.wav")

      x = event.x
      y = event.y

      # Save the ofset between the mouse pointer and the
      # upper left corner of the object
      if(self.current_drawing[item_index].has_key('center_x')):
        self.in_select_ofx = x - self.current_drawing[item_index]['center_x']
        self.in_select_ofy = y - self.current_drawing[item_index]['center_y']
      else:
        bounds = item.get_bounds()
        self.in_select_ofx = x - bounds.x1
        self.in_select_ofy = y - bounds.y1

      return True

    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        gcompris.sound.play_ogg("sounds/smudge.wav")
        return True

    if event.state & gtk.gdk.BUTTON1_MASK:
      x = event.x
      y = event.y

      # Workaround for bad line positionning
      if(self.current_drawing[item_index].has_key('points')):
        item.props.line_width = 1.0

      bounds = item.get_bounds()

      if(self.current_drawing[item_index].has_key('center_x')):
        x = x - self.in_select_ofx - item.props.radius_x
        y = y - self.in_select_ofy - item.props.radius_y
      else:
        x -= self.in_select_ofx
        y -= self.in_select_ofy

      x,y = self.snap_to_grid(x,y)

      # Check drawing boundaries
      if(x < self.drawing_area[0]):
        x = self.drawing_area[0]
      if(x > (self.drawing_area[2]-(bounds.x2-bounds.x1))):
        x = self.drawing_area[2]-(bounds.x2-bounds.x1)
        # We need to realign x cause the bounds values are not precise enough
        x,n = self.snap_to_grid(x,y)
      if(y < self.drawing_area[1]):
        y = self.drawing_area[1]
      if(y > (self.drawing_area[3]-(bounds.y2-bounds.y1))):
        y = self.drawing_area[3]-(bounds.y2-bounds.y1)
        # We need to realign y cause the bounds values are not precise enough
        n,y = self.snap_to_grid(x,y)

      # Need to update current_drawing
      if(self.current_drawing[item_index].has_key('center_x')):
        new_x = x + item.props.radius_x
        new_y = y + item.props.radius_y
        self.current_drawing[item_index]['center_x'] = new_x
        self.current_drawing[item_index]['center_y'] = new_y
        item.props.center_x = new_x
        item.props.center_y = new_y
      elif(self.current_drawing[item_index].has_key('x')):
        # It's a rectangle
        ox = x - self.current_drawing[item_index]['x']
        oy = y - self.current_drawing[item_index]['y']
        self.current_drawing[item_index]['x'] += ox
        self.current_drawing[item_index]['y'] += oy
        item.props.x += ox
        item.props.y += oy
      else:
        # It can only be a line
        ox = x - min(self.current_drawing[item_index]['points'][0], self.current_drawing[item_index]['points'][2])
        oy = y - min(self.current_drawing[item_index]['points'][1], self.current_drawing[item_index]['points'][3])
        nx1 = self.current_drawing[item_index]['points'][0] + ox
        ny1 = self.current_drawing[item_index]['points'][1] + oy
        nx2 = self.current_drawing[item_index]['points'][2] + ox
        ny2 = self.current_drawing[item_index]['points'][3] + oy
        self.current_drawing[item_index]['points'] = (nx1, ny1, nx2, ny2)
        item.props.points = goocanvas.Points([(nx1, ny1),
                                              (nx2, ny2)])

      # Workaround for bad line positionning
      if(self.current_drawing[item_index].has_key('points')):
        item.props.line_width = 8.0

      return True

    return False

  # Event when a click on an item happen on fill in type object
  def fillin_item_event(self, item, target, event, drawing_item_index):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "FILL":
          item.props.fill_color_rgba = self.colors[self.current_color] | self.color_fill
          # Reset the item in our list
          self.current_drawing[drawing_item_index]['fill_color_rgba'] = \
              self.colors[self.current_color] | self.color_fill
          gcompris.sound.play_ogg("sounds/paint1.wav")
          return True
    return False

  # Event when a click on an item happen on border fill type object
  def fillout_item_event(self, item, target, event, drawing_item_index):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "FILL":
          item.props.stroke_color_rgba = self.colors[self.current_color] | self.color_stroke
          # Reset the item in our list
          self.current_drawing[drawing_item_index]['stroke_color_rgba'] = \
              self.colors[self.current_color] | self.color_stroke
          gcompris.sound.play_ogg("sounds/paint1.wav")
          return True
    return False

  # Del an item and internal struct cleanup
  def del_item(self, item, drawing_item_index):
    item.remove()
    # Warning, do not realy delete it or we bug the index of other items
    self.current_drawing[drawing_item_index] = []

  # Event when a click on an item happen
  def del_item_event(self, item, target, event, drawing_item_index):
    if (event.button == 1 and self.tools[self.current_tool][0] == "DEL") \
          or (event.button == 3):
      self.del_item(item, drawing_item_index);
      gcompris.sound.play_ogg("sounds/eraser1.wav",
      "sounds/eraser2.wav")
      return True
    return False

  #
  # Display the mouse coord in the drawing or target area
  # type:
  # 1 = in drawing area
  # 2 = in target area
  # 3 = out of both area
  #
  def display_coord(self, x, y, type):
    if(type == 1):
      xl = self.drawing_area[0]
    else:
      xl = self.target_area[0]

    yl = self.drawing_area[1]

    x = int(round(((x+(self.current_step/2)) - xl)/self.current_step) - 1)
    y = int(round(((y+(self.current_step/2)) - yl)/self.current_step) - 1)

    if(type == 3):
      self.coorditem.props.text = ""
    else:
      self.coorditem.props.text = _("Coordinate") + " = (" + str(x) + "/" + str(y) + ")"


  # Event when an event on the target area happen
  def target_item_event(self, item, target, event):

    if event.type == gtk.gdk.LEAVE_NOTIFY:
      self.display_coord(event.x,event.y, 3)
    else:
      self.display_coord(event.x,event.y, 2)


  # Event when an event on the drawing area happen
  def create_item_event(self, item, target, event):

    if event.type == gtk.gdk.LEAVE_NOTIFY:
      self.display_coord(event.x,event.y, 3)
    else:
      self.display_coord(event.x, event.y, 1)


    if event.type == gtk.gdk.BUTTON_PRESS:
      # Delete error previous mark if any
      if(self.root_erroritem):
        self.root_erroritem.remove()

      if event.button != 1:
        return False

      self.newitem = None

      if (self.tools[self.current_tool][0] == "DEL" or
          self.tools[self.current_tool][0] == "SELECT" or
          self.tools[self.current_tool][0] == "FILL"):
        # This event is treated in del_item_event to avoid
        # operating on background item and grid
        return False

      elif self.tools[self.current_tool][0] == "LINE":

        gcompris.sound.play_ogg("sounds/bleep.wav")

        x,y = self.snap_to_grid(event.x,event.y)
        self.pos_x = x
        self.pos_y = y

        self.newitem = goocanvas.Polyline(
          parent = self.root_drawingitem,
          points = goocanvas.Points([(self.pos_x, self.pos_y),
                                     (x, y)]),
          stroke_color_rgba = self.colors[self.current_color] | self.color_stroke,
          line_width = 8.0
          )
        self.newitem.connect("button_press_event",
                             self.fillout_item_event, len(self.current_drawing))
        self.newitem.connect("button_press_event",
                             self.move_item_event, len(self.current_drawing))
        self.newitem.connect("button_release_event",
                             self.move_item_event, len(self.current_drawing))
        self.newitem.connect("motion_notify_event",
                             self.move_item_event, len(self.current_drawing))

        # Add the new item in our list
        self.current_drawing.append({'tool': self.tools[self.current_tool][0],
                                     'points':(self.pos_x, self.pos_y, x, y),
                                     'stroke_color_rgba' : self.colors[self.current_color] | self.color_stroke,
                                     'line_width':8.0})

      elif self.tools[self.current_tool][0] == "RECT":

        gcompris.sound.play_ogg("sounds/bleep.wav")

        x,y = self.snap_to_grid(event.x,event.y)
        self.pos_x = x
        self.pos_y = y

        self.newitem =goocanvas.Rect(
          parent = self.root_drawingitem,
          x=self.pos_x,
          y=self.pos_y,
          width=0,
          height=0,
          stroke_color_rgba = self.colors[self.current_color] | self.color_stroke,
          line_width=4.0
          )
        self.newitem.connect("button_press_event",
                             self.fillout_item_event, len(self.current_drawing))
        self.newitem.connect("button_press_event",
                             self.move_item_event, len(self.current_drawing))
        self.newitem.connect("button_release_event",
                             self.move_item_event, len(self.current_drawing))
        self.newitem.connect("motion_notify_event",
                             self.move_item_event, len(self.current_drawing))

        # Add the new item in our list
        self.current_drawing.append({'tool': self.tools[self.current_tool][0],
                                     'x':self.pos_x,
                                     'y':self.pos_y,
                                     'width':x,
                                     'height':y,
                                     'stroke_color_rgba':self.colors[self.current_color] | self.color_stroke,
                                     'line_width':4.0})

      elif self.tools[self.current_tool][0] == "FILL_RECT":

        gcompris.sound.play_ogg("sounds/bleep.wav")

        x,y = self.snap_to_grid(event.x,event.y)
        self.pos_x = x
        self.pos_y = y

        self.newitem = goocanvas.Rect(
          parent = self.root_drawingitem,
          x = self.pos_x,
          y = self.pos_y,
          width = 0,
          height = 0,
          fill_color_rgba = self.colors[self.current_color] | self.color_fill,
          stroke_color_rgba = 0x000000FFL,
          line_width = 1.0
          )
        self.newitem.connect("button_press_event",
                             self.fillin_item_event, len(self.current_drawing))
        self.newitem.connect("button_press_event",
                             self.move_item_event, len(self.current_drawing))
        self.newitem.connect("button_release_event",
                             self.move_item_event, len(self.current_drawing))
        self.newitem.connect("motion_notify_event",
                             self.move_item_event, len(self.current_drawing))

        # Add the new item in our list
        self.current_drawing.append({'tool': self.tools[self.current_tool][0],
                                     'x':self.pos_x,
                                     'y':self.pos_y,
                                     'width':0,
                                     'height':0,
                                     'fill_color_rgba':self.colors[self.current_color] | self.color_fill,
                                     'stroke_color_rgba':0x000000FFL,
                                     'line_width':1.0})

      elif self.tools[self.current_tool][0] == "CIRCLE":

        gcompris.sound.play_ogg("sounds/bleep.wav")

        x,y = self.snap_to_grid(event.x,event.y)
        self.pos_x = x
        self.pos_y = y

        self.newitem = goocanvas.Ellipse(
          parent = self.root_drawingitem,
          center_x = self.pos_x,
          center_y = self.pos_y,
          radius_x = 0,
          radius_y = 0,
          stroke_color_rgba = self.colors[self.current_color] | self.color_stroke,
          line_width = 5.0
          )
        self.newitem.connect("button_press_event",
                             self.fillout_item_event, len(self.current_drawing))
        self.newitem.connect("button_press_event",
                             self.move_item_event, len(self.current_drawing))
        self.newitem.connect("button_release_event",
                             self.move_item_event, len(self.current_drawing))
        self.newitem.connect("motion_notify_event",
                             self.move_item_event, len(self.current_drawing))

        # Add the new item in our list
        self.current_drawing.append({'tool': self.tools[self.current_tool][0],
                                     'center_x':self.pos_x,
                                     'center_y':self.pos_y,
                                     'radius_x':0,
                                     'radius_y':0,
                                     'stroke_color_rgba':self.colors[self.current_color] | self.color_stroke,
                                     'line_width':5.0})

      elif self.tools[self.current_tool][0] == "FILL_CIRCLE":

        gcompris.sound.play_ogg("sounds/bleep.wav")

        x,y = self.snap_to_grid(event.x,event.y)
        self.pos_x = x
        self.pos_y = y

        self.newitem = goocanvas.Ellipse(
          parent = self.root_drawingitem,
          center_x = self.pos_x,
          center_y = self.pos_y,
          radius_x = 0,
          radius_y = 0,
          fill_color_rgba = self.colors[self.current_color] | self.color_fill,
          stroke_color_rgba = 0x000000FFL,
          line_width = 1.0
          )
        self.newitem.connect("button_press_event",
                             self.fillin_item_event, len(self.current_drawing))
        self.newitem.connect("button_press_event",
                             self.move_item_event, len(self.current_drawing))
        self.newitem.connect("button_release_event",
                             self.move_item_event, len(self.current_drawing))
        self.newitem.connect("motion_notify_event",
                             self.move_item_event, len(self.current_drawing))

        # Add the new item in our list
        self.current_drawing.append({'tool': self.tools[self.current_tool][0],
                                     'center_x':self.pos_x,
                                     'center_y':self.pos_y,
                                     'radius_x':0,
                                     'radius_y':0,
                                     'fill_color_rgba':self.colors[self.current_color] | self.color_fill,
                                     'stroke_color_rgba':0x000000FFL,
                                     'line_width':1.0})
      if self.newitem != 0:
        self.newitem.connect("button_press_event", self.create_item_event)
        self.newitem.connect("button_release_event", self.create_item_event)
        self.newitem.connect("motion_notify_event", self.create_item_event)
        self.newitem.connect("button_press_event", self.del_item_event, len(self.current_drawing)-1)

      return True

    #
    # MOTION EVENT
    # ------------
    if event.type == gtk.gdk.MOTION_NOTIFY:
      if event.state & gtk.gdk.BUTTON1_MASK:
        x = event.x
        y = event.y
        x,y = self.snap_to_grid(event.x,event.y)

        # Check drawing boundaries
        if(event.x<self.drawing_area[0]):
          x = self.drawing_area[0]
        if(event.x>self.drawing_area[2]):
          x = self.drawing_area[2]
        if(event.y<self.drawing_area[1]):
          y = self.drawing_area[1]
        if(event.y>self.drawing_area[3]):
          y = self.drawing_area[3]

        if self.tools[self.current_tool][0] == "LINE":
          self.newitem.props.points = goocanvas.Points([(self.pos_x, self.pos_y),
                                                        (x, y)])
          # Reset the item in our list
          self.current_drawing[len(self.current_drawing)-1]['points'] = ( self.pos_x, self.pos_y, x, y)

        elif (self.tools[self.current_tool][0] == "RECT" or
              self.tools[self.current_tool][0] == "FILL_RECT"):
          if(x - self.pos_x < 0):
            self.newitem.props.x = x
          else:
            self.newitem.props.x = self.pos_x

          if(y - self.pos_y < 0):
            self.newitem.props.y = y
          else:
            self.newitem.props.y = self.pos_y

          self.newitem.props.width = abs(x - self.pos_x)
          self.newitem.props.height = abs(y - self.pos_y)

          self.current_drawing[len(self.current_drawing)-1]['x'] = \
              self.newitem.props.x
          self.current_drawing[len(self.current_drawing)-1]['y'] = \
              self.newitem.props.y
          self.current_drawing[len(self.current_drawing)-1]['width'] = \
              self.newitem.props.width
          self.current_drawing[len(self.current_drawing)-1]['height'] = \
              self.newitem.props.height

        elif (self.tools[self.current_tool][0] == "CIRCLE" or
              self.tools[self.current_tool][0] == "FILL_CIRCLE"):
          self.newitem.props.radius_x = abs(x - self.pos_x)/2
          self.newitem.props.center_x = self.pos_x + (x - self.pos_x)/2
          self.newitem.props.radius_y = abs(y - self.pos_y)/2
          self.newitem.props.center_y = self.pos_y + (y - self.pos_y)/2

          self.current_drawing[len(self.current_drawing)-1]['radius_x'] = \
              self.newitem.props.radius_x
          self.current_drawing[len(self.current_drawing)-1]['center_x'] = \
              self.newitem.props.center_x
          self.current_drawing[len(self.current_drawing)-1]['radius_y'] = \
              self.newitem.props.radius_y
          self.current_drawing[len(self.current_drawing)-1]['center_y'] = \
              self.newitem.props.center_y


    #
    # MOUSE DRAG STOP
    # ---------------
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        # We have to remove empty created items (the kid did not drag enough)
        if self.tools[self.current_tool][0] == "LINE":
          bounds = self.current_drawing[len(self.current_drawing)-1]['points']
          if (bounds[0] == bounds[2]) and (bounds[1] == bounds[3]):
            # Oops, empty line
            self.del_item(self.newitem, len(self.current_drawing)-1)
          else:
            gcompris.sound.play_ogg("sounds/line_end.wav")
            # We need to reord the coord in increasing order to allow later comparison
            # I use a trick, I do x1*x1+y1 and x2*x2+y2, put the lower as the A point
            i = bounds[0]*bounds[0] + bounds[1]
            j = bounds[2]*bounds[2] + bounds[3]
            if(i<=j):
              self.current_drawing[len(self.current_drawing)-1]['points'] = (bounds[0], bounds[1],
                                                                             bounds[2], bounds[3])
            else:
              self.current_drawing[len(self.current_drawing)-1]['points'] = (bounds[2], bounds[3],
                                                                             bounds[0], bounds[1])

        elif (self.tools[self.current_tool][0] == "RECT" or
              self.tools[self.current_tool][0] == "FILL_RECT"):
          # It's a rect
          width = self.current_drawing[len(self.current_drawing)-1]['width']
          height = self.current_drawing[len(self.current_drawing)-1]['height']
          if (width == 0) or (height == 0):
            # Oups, empty rect
            self.del_item(self.newitem, len(self.current_drawing)-1)
        elif (self.tools[self.current_tool][0] == "CIRCLE" or
              self.tools[self.current_tool][0] == "FILL_CIRCLE"):
          # It's an ellipse
          radius_x = self.current_drawing[len(self.current_drawing)-1]['radius_x']
          radius_y = self.current_drawing[len(self.current_drawing)-1]['radius_y']
          if (radius_x == 0) or (radius_y == 0):
            # Oups, empty rect
            self.del_item(self.newitem, len(self.current_drawing)-1)
          else:
            gcompris.sound.play_ogg("sounds/line_end.wav")

        return True
    return False


  # The list of items (data) for this game
  def init_item_list(self):

    if self.gcomprisBoard.mode == 'normal':
      self.drawlist = \
                    [
        # Two stripes
      [{'width': 280.0, 'line_width': 1.0, 'height': 20.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 80.0, 'tool': 'FILL_RECT', 'x': 140.0},
       {'width': 280.0, 'line_width': 1.0, 'height': 20.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 420.0, 'tool': 'FILL_RECT', 'x': 140.0}]
      ,
      # Top centered box
      [{'width': 160.0, 'height': 40.0, 'line_width': 1.0, 'fill_color_rgba': 0x1010FFA0L, 'stroke_color_rgba': 255L, 'y': 40.0, 'x': 200.0, 'tool': 'FILL_RECT'}]
      ,
      # 4 small corners
      [{'width': 40.0, 'height': 40.0,  'line_width': 1.0,'fill_color_rgba': 0x1010FFA0L, 'stroke_color_rgba': 255L, 'y': 40.0, 'x': 160.0, 'tool': 'FILL_RECT'},
       {'width': 400.0-360.0, 'height': 80.0-40.0,  'line_width': 1.0,'fill_color_rgba': 0x1010FFA0L, 'stroke_color_rgba': 255L, 'y': 40.0, 'x': 360.0, 'tool': 'FILL_RECT'},
       {'width': 200.0-160.0, 'height': 480.0-440.0, 'line_width': 1.0, 'fill_color_rgba': 0x1010FFA0L, 'stroke_color_rgba': 255L, 'y': 440.0, 'x': 160.0, 'tool': 'FILL_RECT'},
       {'width': 400.0-360.0, 'height': 480.0-440.0,  'line_width': 1.0,'fill_color_rgba': 0x1010FFA0L, 'stroke_color_rgba': 255L, 'y': 440.0, 'x': 360.0, 'tool': 'FILL_RECT'}]
      ,
      # 4 non filled Rects organised in rect and shifted
      [{'width': 200.0-180.0, 'height': 360.0-200.0, 'stroke_color_rgba': 0x1010FFFFL, 'line_width': 4.0, 'y': 200.0, 'x': 180.0, 'tool': 'RECT'},
       {'width': 340.0-180.0, 'height': 400.0-380.0, 'stroke_color_rgba': 0x1010FFFFL, 'line_width': 4.0, 'y':380.0, 'x': 180.0, 'tool': 'RECT'},
       {'width': 380.0-360.0, 'height': 400.0-240.0, 'stroke_color_rgba': 0x1010FFFFL, 'line_width': 4.0, 'y': 240.0, 'x': 360.0, 'tool': 'RECT'},
       {'width': 380.0-220.0, 'height': 220.0-200.0, 'stroke_color_rgba': 0x1010FFFFL, 'line_width': 4.0, 'y': 200.0, 'x': 220.0, 'tool': 'RECT'}]
      ,
      # Letter A
      [{'tool': 'LINE', 'points': (200.0, 120.0, 280.0, 120.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (280.0, 120.0, 280.0,240.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (200.0, 120.0, 200.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (200.0, 180.0, 280.0, 180.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL}]
      ,
      # Letter B
      [{'tool': 'LINE', 'points': (240.0, 240.0, 320.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (300.0, 180.0, 320.0,200.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (300.0, 180.0, 320.0, 160.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (240.0, 180.0, 300.0, 180.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (320.0, 100.0, 320.0, 160.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool':'LINE', 'points': (240.0, 100.0, 320.0, 100.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (240.0, 100.0, 240.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (320.0, 200.0, 320.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL}]
      ,
      # A door
      [{'width': 360.0-200.0, 'height': 360.0-180.0, 'stroke_color_rgba': 0x1010FFFFL, 'line_width': 4.0, 'y': 180.0, 'x': 200.0, 'tool': 'RECT'},
       {'radius_x': 20, 'radius_y': 20, 'line_width': 1.0, 'fill_color_rgba': 0x1010FFA0L, 'stroke_color_rgba': 255L, 'center_y': 280.0+20, 'center_x': 300.0+20, 'tool': 'FILL_CIRCLE'},
       {'width': 320.0-240.0, 'height': 260.0-200.0, 'line_width': 1.0, 'fill_color_rgba': 0x1010FFA0L, 'stroke_color_rgba': 255L, 'y': 200.0, 'x': 240.0, 'tool': 'FILL_RECT'}],
      # A top left kind of target
      [{'radius_x': 50, 'radius_y': 50, 'stroke_color_rgba': 0x1010FFFFL, 'center_y': 40.0+50, 'center_x': 160.0+50, 'tool': 'CIRCLE', 'line_width': 5.0},
       {'radius_x': 30, 'radius_y': 30, 'line_width': 1.0, 'fill_color_rgba': 0x1010FFA0L, 'stroke_color_rgba': 255L, 'center_y': 60.0+30, 'center_x': 180.0+30, 'tool': 'FILL_CIRCLE'}]
      ,
      # 4 Huge Diagonal lines
      [{'tool': 'LINE', 'points': (140.0, 260.0, 260.0, 20.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (140.0, 260.0, 260.0, 500.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (260.0, 500.0, 420.0, 260.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'tool': 'LINE', 'points': (260.0, 20.0, 420.0, 260.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL}]
      ,
      # Balloon
      [{'tool': 'LINE', 'points': (220.0, 240.0, 220.0, 340.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
       {'radius_x': 40, 'line_width': 1.0, 'radius_y': 50, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'center_y': 140.0+50, 'tool': 'FILL_CIRCLE', 'center_x': 180.0+40},
       {'radius_x': 40, 'line_width': 1.0, 'radius_y': 50, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'center_y': 80.0+50, 'tool': 'FILL_CIRCLE', 'center_x': 300.0+40},
       {'tool': 'LINE', 'points': (340.0, 180.0, 340.0, 280.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL}]
      ,
      # Watch
      [{'radius_x': 100, 'radius_y': 100, 'stroke_color_rgba': 0x1010FFFFL, 'line_width': 5.0, 'center_y': 140.0+100, 'tool': 'CIRCLE', 'center_x': 180.0+100},
       {'tool': 'LINE', 'points': (280.0, 160.0, 280.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'tool': 'LINE', 'points': (280.0, 240.0, 320.0, 200.0), 'line_width': 8.0, 'stroke_color_rgba': 352271359L},
       {'tool': 'LINE', 'points': (220.0, 280.0, 280.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 4294905087L}]
      ,
      # Colored pyramid
      [{'width': 280.0-260.0, 'line_width': 1.0, 'height': 120.0-100.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 100.0, 'x': 260.0, 'tool': 'FILL_RECT'},
       {'width': 300.0-240.0, 'line_width': 1.0, 'height': 140.0-120.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xffff0cA0L, 'y': 120.0,  'x': 240.0, 'tool': 'FILL_RECT'},
       {'width': 320.0-220.0, 'line_width': 1.0, 'height': 160.0-140.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 140.0, 'tool': 'FILL_RECT', 'x': 220.0},
       {'width': 340.0-200.0, 'line_width': 1.0, 'height': 180.0-160.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 160.0, 'tool': 'FILL_RECT', 'x': 200.0},
       {'width': 360.0-180.0, 'line_width': 1.0, 'height': 200.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xC2C2C2A0L, 'y': 180.0, 'tool': 'FILL_RECT', 'x': 180.0}]
      ,
      # Colored Rectangle bigger and bigger
      [{'width': 180.0-140.0, 'line_width': 1.0, 'height': 60.0-20.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 20.0, 'tool': 'FILL_RECT', 'x': 140.0},
       {'width': 240.0-180.0, 'line_width': 1.0, 'height': 120.0-60.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 60.0, 'tool': 'FILL_RECT', 'x': 180.0},
       {'width': 320.0-240.0, 'line_width': 1.0, 'height': 200.0-120.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xffff0cA0L, 'y': 120.0, 'tool': 'FILL_RECT', 'x': 240.0},
       {'width': 420.0-320.0, 'line_width': 1.0, 'height': 300.0-200.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 200.0, 'tool': 'FILL_RECT', 'x': 320.0}]
      ,
      # Tree
      [{'width': 420.0-140.0, 'line_width': 1.0, 'height': 500.0-460.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x000000A0L, 'y': 460.0, 'tool': 'FILL_RECT', 'x': 140.0},
       {'width': 260.0-240.0, 'line_width': 1.0, 'height': 460.0-360.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 360.0, 'tool': 'FILL_RECT', 'x': 240.0},
       {'radius_x': 70, 'line_width': 1.0, 'radius_y': 40, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'center_y': 280.0+40, 'tool': 'FILL_CIRCLE', 'center_x': 180.0+70}]
      ,
      # bipbip (big non flying bird)
      [{'width': 280.0-260.0, 'line_width': 1.0, 'height': 320.0-120.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 120.0, 'tool': 'FILL_RECT', 'x': 260.0},
       {'width': 300.0-260.0, 'line_width': 1.0, 'height': 120.0-80.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 80.0, 'tool': 'FILL_RECT', 'x': 260.0},
       {'width': 320.0-300.0, 'line_width': 1.0, 'height': 120.0-100.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 100.0, 'tool': 'FILL_RECT', 'x': 300.0},
       {'width': 280.0-200.0, 'line_width': 1.0, 'height': 380.0-320.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 320.0, 'tool': 'FILL_RECT', 'x': 200.0},
       {'width': 220.0-200.0, 'line_width': 1.0, 'height': 320.0-300.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 300.0, 'tool': 'FILL_RECT', 'x': 200.0},
       {'width': 260.0-240.0, 'line_width': 1.0, 'height': 460.0-380.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 380.0, 'tool': 'FILL_RECT', 'x': 240.0},
       {'width': 280.0-260.0, 'line_width': 1.0, 'height': 460.0-440.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 440.0, 'tool': 'FILL_RECT', 'x': 260.0}]
      ,
      # Dog
      [{'width': 180.0-160.0, 'line_width': 1.0, 'height': 200.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 180.0, 'tool': 'FILL_RECT', 'x': 160.0},
       {'width': 340.0-180.0, 'line_width': 1.0, 'height': 240.0-200.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 200.0, 'tool': 'FILL_RECT', 'x': 180.0},
       {'width': 200.0-180.0, 'line_width': 1.0, 'height': 280.0-240.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 240.0, 'tool': 'FILL_RECT', 'x': 180.0},
       {'width': 340.0-320.0,'line_width': 1.0, 'height': 280.0-240.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 240.0, 'tool': 'FILL_RECT', 'x': 320.0},
       {'width': 380.0-320.0, 'line_width': 1.0, 'height': 200.0-160.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 160.0, 'tool': 'FILL_RECT', 'x': 320.0}]
      ,
      # Fish
      [{'radius_x': 90, 'line_width': 1.0, 'radius_y': 60, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xffff0cA0L, 'center_y': 160.0+60, 'tool': 'FILL_CIRCLE', 'center_x': 180.0+90},
       {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'center_y': 200.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 320.0+10},
       {'width': 180.0-160.0, 'line_width': 1.0, 'height': 260.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xffff0cA0L, 'y': 180.0, 'tool': 'FILL_RECT', 'x': 160.0}]
      ,
      # Balloon (human)
      [{'radius_x': 90, 'line_width': 1.0, 'radius_y': 100, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'center_y': 100.0+100, 'tool': 'FILL_CIRCLE', 'center_x': 200.0+90},
       {'radius_x': 50, 'line_width': 1.0, 'radius_y': 30, 'stroke_color_rgba': 255L, 'stroke_color_rgba': 4294905087L, 'center_y': 320.0, 'tool': 'FILL_RECT', 'center_x': 240.0},
       {'tool': 'LINE', 'points': (220.0, 260.0, 260.0, 320.0), 'line_width': 8.0, 'stroke_color_rgba': 4287383039L},
       {'tool': 'LINE', 'points': (320.0, 320.0, 360.0, 260.0), 'line_width': 8.0, 'stroke_color_rgba': 4287383039L}]
      ,
      # House
      [{'width': 360.0-200.0, 'line_width': 1.0, 'height': 340.0-240.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 240.0, 'tool': 'FILL_RECT', 'x': 200.0},
       {'width': 280.0-240.0, 'line_width': 1.0, 'height': 340.0-280.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 280.0, 'tool': 'FILL_RECT', 'x': 240.0},
       {'width': 340.0-300.0, 'line_width': 1.0, 'height': 300.0-260.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 260.0, 'tool': 'FILL_RECT', 'x': 300.0},
       {'tool': 'LINE', 'points': (200.0, 240.0, 280.0, 160.0), 'line_width': 8.0, 'stroke_color_rgba': 4287383039L},
       {'tool': 'LINE', 'points': (280.0, 160.0, 360.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 4287383039L}]
      ,
      # Truck
      [{'radius_x': 20, 'line_width': 1.0, 'radius_y': 20, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'center_y': 240.0+20, 'tool': 'FILL_CIRCLE', 'center_x': 180.0+20},
       {'radius_x': 20, 'line_width': 1.0, 'radius_y': 20, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'center_y': 240.0+20, 'tool': 'FILL_CIRCLE', 'center_x': 280.0+20},
       {'width': 220.0-160.0, 'line_width': 1.0, 'height': 260.0-220.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 220.0, 'tool': 'FILL_RECT', 'x': 160.0},
       {'width': 300.0-220.0, 'line_width': 1.0, 'height': 260.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 180.0, 'tool': 'FILL_RECT', 'x': 220.0},
       {'width': 320.0-300.0, 'line_width': 1.0, 'height': 260.0-220.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 220.0, 'tool': 'FILL_RECT', 'x': 300.0},
       {'width': 280.0-240.0, 'line_width': 1.0, 'height': 260.0-200.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 200.0, 'tool': 'FILL_RECT', 'x': 240.0}]
      ,
      # Fire truck
      [{'radius_x': 20, 'line_width': 1.0, 'radius_y': 20, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'center_y': 260.0+20, 'tool': 'FILL_CIRCLE', 'center_x': 160.0+20},
       {'radius_x': 20, 'line_width': 1.0, 'radius_y': 20, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'center_y': 260.0+20, 'tool': 'FILL_CIRCLE', 'center_x': 320.0+20},
       {'width': 380.0-160.0, 'line_width': 1.0, 'height': 280.0-220.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 220.0, 'tool': 'FILL_RECT', 'x': 160.0},
       {'tool': 'LINE', 'points': (160.0, 200.0, 340.0, 180.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'width': 200.0-180.0, 'line_width': 1.0, 'height': 220.0-200.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 200.0, 'tool': 'FILL_RECT', 'x': 180.0},
       {'width': 360.0-320.0, 'line_width': 1.0, 'height': 260.0-220.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 220.0, 'tool': 'FILL_RECT', 'x': 320.0},
       {'width': 300.0-280.0, 'line_width': 1.0, 'height': 260.0-220.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 220.0, 'tool': 'FILL_RECT', 'x': 280.0}]
      ,
      # Billard
      [
        {'line_width': 8.0, 'points': (180.0, 80.0, 180.0, 440.0), 'tool': 'LINE', 'stroke_color_rgba': 0x1010FFFFL},
        {'line_width': 8.0, 'points': (180.0, 460.0, 380.0, 460.0), 'tool': 'LINE', 'stroke_color_rgba': 0x1010FFFFL},
        {'line_width': 8.0, 'points': (380.0, 80.0, 380.0, 440.0), 'tool': 'LINE', 'stroke_color_rgba': 0x1010FFFFL},
        {'line_width': 8.0, 'points': (180.0, 60.0, 380.0, 60.0), 'tool': 'LINE', 'stroke_color_rgba': 0x1010FFFFL},
        {'line_width': 8.0, 'points': (280.0, 320.0, 280.0, 420.0), 'tool': 'LINE', 'stroke_color_rgba': 2199425535L},
        {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'center_y': 300.0+10, 'center_x': 260.0+10, 'tool': 'FILL_CIRCLE'},
        {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'center_y': 100.0+10, 'center_x': 240.0+10, 'tool': 'FILL_CIRCLE'},
        {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'center_y': 100.0+10, 'center_x': 260.0+10, 'tool': 'FILL_CIRCLE'},
        {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'center_y': 100.0+10, 'center_x': 280.0+10, 'tool': 'FILL_CIRCLE'},
        {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x000000A0L, 'center_y': 100.0+10, 'center_x': 300.0+10, 'tool': 'FILL_CIRCLE'},
        {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xb9bc0dffL, 'center_y': 120.0+10, 'center_x': 260.0+10, 'tool': 'FILL_CIRCLE'},
        {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'center_y': 120.0+10, 'center_x': 280.0+10, 'tool': 'FILL_CIRCLE'}
        ]
      ,
      # Clara (my daughter)
      [{'width': 240.0-220.0, 'line_width': 1.0, 'height': 480.0-400.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 400.0, 'x': 220.0, 'tool': 'FILL_RECT'},
       {'width': 320.0-300.0, 'line_width': 1.0, 'height': 480.0-400.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 400.0, 'x': 300.0, 'tool': 'FILL_RECT'},
       {'width': 220.0-160.0, 'line_width': 1.0, 'height': 200.0-180.0,'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 180.0, 'x': 160.0, 'tool': 'FILL_RECT'},
       {'width': 380.0-320.0, 'line_width': 1.0, 'height': 200.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 180.0, 'x': 320.0, 'tool': 'FILL_RECT'},
       {'width': 380.0-360.0, 'line_width': 1.0, 'height': 180.0-120.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 120.0, 'x': 360.0, 'tool': 'FILL_RECT'},
       {'width': 180.0-160.0, 'line_width': 1.0, 'height': 260.0-200.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 200.0, 'x': 160.0, 'tool': 'FILL_RECT'},
       {'radius_x': 50, 'line_width': 1.0, 'radius_y': 50, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'center_y': 60.0+50, 'center_x': 220.0+50, 'tool': 'FILL_CIRCLE'},
       {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00a0L, 'center_y': 100.0+10, 'center_x': 240.0+10, 'tool': 'FILL_CIRCLE'},
       {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00a0L, 'center_y': 100.0+10, 'center_x': 280.0+10, 'tool': 'FILL_CIRCLE'},
       {'line_width': 8.0, 'points': (260.0, 140.0, 280.0, 140.0), 'tool': 'LINE', 'stroke_color_rgba': 0xFF0000A0L},
       {'width': 300.0-240.0, 'line_width': 1.0, 'height': 180.0-160.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 160.0, 'x': 240.0, 'tool': 'FILL_RECT'},
       {'width': 320.0-220.0, 'line_width': 1.0, 'height': 320.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 180.0, 'x': 220.0, 'tool': 'FILL_RECT'},
       {'width': 340.0-200.0, 'line_width': 1.0, 'height': 400.0-320.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 320.0, 'x': 200.0, 'tool': 'FILL_RECT'}]
      ,
      # Bicycle
      [{'radius_x': 40, 'radius_y': 40, 'stroke_color_rgba': 4287383039L, 'line_width': 5.0, 'center_y': 260.0+40, 'tool': 'CIRCLE', 'center_x': 160.0+40},
       {'radius_x': 40, 'radius_y': 40, 'stroke_color_rgba': 4287383039L, 'line_width': 5.0, 'center_y': 260.0+40, 'tool': 'CIRCLE', 'center_x': 320.0+40},
       {'tool': 'LINE', 'points': (200.0, 300.0, 280.0, 300.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'tool': 'LINE', 'points': (280.0,300.0, 340.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'tool': 'LINE', 'points': (240.0, 240.0, 340.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'tool': 'LINE', 'points': (200.0, 300.0, 240.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'tool': 'LINE', 'points': (240.0, 220.0, 240.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 2199425535L},
       {'tool': 'LINE', 'points': (220.0, 220.0, 260.0, 220.0), 'line_width': 8.0, 'stroke_color_rgba': 2199425535L},
       {'tool': 'LINE', 'points': (340.0, 200.0, 340.0, 240.0), 'line_width': 8.0, 'stroke_color_rgba': 2199425535L},
       {'tool': 'LINE', 'points': (320.0, 200.0, 340.0, 200.0), 'line_width': 8.0, 'stroke_color_rgba': 2199425535L},
       {'tool': 'LINE', 'points': (340.0, 240.0, 360.0, 300.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L}]
      ,
      # Sea boat and sun
      [{'width': 420.0-140.0, 'line_width': 1.0, 'height': 500.0-420.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00a0L, 'y': 420.0, 'tool': 'FILL_RECT', 'x': 140.0},
       {'radius_x': 50, 'line_width': 1.0, 'radius_y': 50, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xffff0cA0L, 'center_y': 60.0+50, 'tool': 'FILL_CIRCLE', 'center_x': 160.0+50},
       {'radius_x': 50, 'line_width': 1.0, 'radius_y': 20, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'center_y': 160.0+20, 'tool': 'FILL_CIRCLE', 'center_x': 260.0+50},
       {'radius_x': 30, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'center_y': 200.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 300.0+30},
       {'radius_x': 20,'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba':0x1010FFA0L, 'center_y': 140.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 360.0+20},
       {'tool': 'LINE','points': (220.0, 400.0, 240.0, 420.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'tool': 'LINE', 'points': (240.0, 420.0, 280.0, 420.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'tool': 'LINE', 'points': (280.0, 420.0, 300.0, 400.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'tool': 'LINE', 'points': (220.0, 400.0, 300.0, 400.0), 'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'tool': 'LINE', 'points': (260.0, 280.0, 260.0, 400.0),'line_width': 8.0, 'stroke_color_rgba': 0xFF0000A0L},
       {'tool': 'LINE', 'points':(260.0, 280.0, 300.0, 380.0), 'line_width': 8.0, 'stroke_color_rgba': 2199425535L},
       {'tool': 'LINE', 'points': (260.0, 380.0, 300.0, 380.0), 'line_width': 8.0,'stroke_color_rgba': 2199425535L}]

      ]
    else:
      # Symmetrical items
      self.drawlist = \
      [
        # 3 white box in triangle
        [{'width': 420.0-380.0, 'line_width': 1.0, 'height': 260.0-220.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 220.0, 'tool': 'FILL_RECT', 'x': 380.0},
         {'width': 380.0-340.0, 'line_width': 1.0, 'height': 220.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 180.0, 'tool': 'FILL_RECT', 'x': 340.0},
         {'width': 380.0-340.0, 'line_width': 1.0, 'height': 300.0-260.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 260.0, 'tool': 'FILL_RECT', 'x': 340.0}]
        ,
        # Colored pyramid
        [{'width': 420.0-140.0, 'line_width': 1.0, 'height': 460.0-420.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xC2C2C2A0L, 'y': 420.0, 'tool': 'FILL_RECT', 'x': 140.0},
         {'width': 420.0-180.0, 'line_width': 1.0, 'height': 420.0-380.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 380.0, 'tool': 'FILL_RECT', 'x': 180.0},
         {'width': 420.0-220.0, 'line_width': 1.0, 'height': 380.0-340.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 340.0, 'tool': 'FILL_RECT', 'x': 220.0},
         {'width': 420.0-260.0, 'line_width': 1.0, 'height': 340.0-300.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xffff0cA0L, 'y': 300.0, 'tool': 'FILL_RECT', 'x': 260.0},
         {'width': 420.0-300.0, 'line_width': 1.0, 'height': 300.0-260.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 260.0, 'tool': 'FILL_RECT', 'x': 300.0},
         {'width': 420.0-340.0, 'line_width': 1.0, 'height': 260.0-220.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x000000A0L, 'y': 220.0, 'tool': 'FILL_RECT', 'x': 340.0},
         {'width': 420.0-380.0, 'line_width': 1.0, 'height': 220.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x000000A0L, 'y': 180.0, 'tool': 'FILL_RECT', 'x': 380.0}]
        ,
        # Butterfly
        [{'width': 420.0-380.0, 'line_width': 1.0, 'height': 380.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xffff0cA0L, 'y': 180.0, 'tool': 'FILL_RECT', 'x': 380.0},
         {'tool': 'LINE', 'points': (360.0, 80.0, 400.0, 180.0), 'line_width': 8.0, 'stroke_color_rgba': 707406591L},
         {'radius_x': 100, 'line_width': 1.0, 'radius_y': 170, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'center_y': 100.0+170, 'tool': 'FILL_CIRCLE', 'center_x': 180.0+100},
         {'radius_x': 50, 'line_width': 1.0, 'radius_y': 100, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'center_y': 180.0+100, 'tool': 'FILL_CIRCLE', 'center_x': 260.0+50}]
        ,
        # Robot
        [{'width': 420.0-340.0, 'line_width': 1.0, 'height': 360.0-160.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 160.0, 'tool': 'FILL_RECT', 'x': 340.0},
         {'width': 380.0-340.0, 'line_width': 1.0, 'height': 500.0-360.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 360.0, 'tool': 'FILL_RECT', 'x': 340.0},
         {'width': 340.0-260.0, 'line_width': 1.0, 'height': 200.0-160.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 160.0, 'tool': 'FILL_RECT', 'x': 260.0},
         {'width': 300.0-260.0, 'line_width': 1.0, 'height': 280.0-200.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 200.0, 'tool': 'FILL_RECT', 'x': 260.0},
         {'width': 420.0-380.0, 'line_width': 1.0, 'height': 160.0-140.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 140.0, 'tool': 'FILL_RECT', 'x': 380.0},
         {'width': 420.0-360.0, 'line_width': 1.0, 'height': 140.0-60.0,  'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00a0L, 'y': 60.0, 'tool': 'FILL_RECT', 'x': 360.0},
         {'width': 420.0-400.0, 'line_width': 1.0, 'height': 120.0-100.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xb9bc0dffL, 'y': 100.0, 'tool': 'FILL_RECT', 'x': 400.0},
         {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'center_y': 80.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 380.0+10}]
        ,
        # Arrow
        [{'width': 420.0-300.0, 'line_width': 1.0, 'height': 260.0-240.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 240.0, 'tool': 'FILL_RECT', 'x': 300.0},
         {'tool': 'LINE', 'points': (180.0, 240.0, 300.0, 140.0), 'line_width': 8.0, 'stroke_color_rgba': 4287383039L},
         {'tool': 'LINE', 'points': (180.0, 240.0, 180.0, 260.0), 'line_width': 8.0, 'stroke_color_rgba': 4287383039L},
         {'tool': 'LINE', 'points': (180.0, 260.0, 300.0, 360.0), 'line_width': 8.0, 'stroke_color_rgba': 4287383039L},
         {'tool': 'LINE', 'points': (300.0, 140.0, 300.0, 360.0), 'line_width': 8.0, 'stroke_color_rgba': 4287383039L}]
        ,
        # House
        [{'width': 420.0-200.0, 'line_width': 1.0, 'height': 460.0-440.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 440.0, 'tool': 'FILL_RECT', 'x': 200.0},
         {'width': 220.0-200.0, 'line_width': 1.0, 'height': 440.0-280.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 280.0, 'tool': 'FILL_RECT', 'x': 200.0},
         {'width': 420.0-200.0, 'line_width': 1.0, 'height': 280.0-260.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'y': 260.0, 'tool': 'FILL_RECT', 'x': 200.0},
         {'width': 420.0-380.0, 'line_width': 1.0, 'height': 440.0-360.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 360.0, 'tool': 'FILL_RECT', 'x': 380.0},
         {'width': 360.0-240.0, 'line_width': 1.0, 'height': 400.0-300.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xffff0cA0L, 'y': 300.0, 'tool': 'FILL_RECT', 'x': 240.0},
         {'tool': 'LINE', 'points': (200.0, 260.0, 420.0, 100.0), 'line_width': 8.0, 'stroke_color_rgba': 4294905087L}]
        ,
        # Plane
        [{'width': 420.0-360.0, 'line_width': 1.0, 'height': 380.0-140.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 140.0, 'tool': 'FILL_RECT', 'x': 360.0},
         {'width': 360.0-180.0, 'line_width': 1.0, 'height': 280.0-220.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 220.0, 'tool': 'FILL_RECT', 'x': 180.0},
         {'width': 260.0-240.0, 'line_width': 1.0, 'height': 220.0-200.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x000000A0L, 'y': 200.0, 'tool': 'FILL_RECT', 'x': 240.0},
         {'tool': 'LINE', 'points': (180.0, 200.0, 320.0, 200.0), 'line_width': 8.0, 'stroke_color_rgba': 4283674623L},
         {'width': 420.0-400.0, 'line_width': 1.0, 'height': 420.0-360.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 360.0, 'tool': 'FILL_RECT', 'x': 400.0},
         {'width': 420.0-380.0, 'line_width': 1.0, 'height': 180.0-160.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 160.0, 'tool': 'FILL_RECT', 'x': 380.0}]
        ,
        # bipbip (big non flying bird)
        [{'width': 280.0-260.0, 'line_width': 1.0, 'height': 320.0-120.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 120.0, 'tool': 'FILL_RECT', 'x': 260.0},
         {'width': 300.0-260.0, 'line_width': 1.0, 'height': 120.0-80.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 80.0, 'tool': 'FILL_RECT', 'x': 260.0},
         {'width': 320.0-300.0, 'line_width': 1.0, 'height': 120.0-100.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 100.0, 'tool': 'FILL_RECT', 'x': 300.0},
         {'width': 280.0-200.0, 'line_width': 1.0, 'height': 380.0-320.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 320.0, 'tool': 'FILL_RECT', 'x': 200.0},
         {'width': 220.0-200.0, 'line_width': 1.0, 'height': 320.0-300.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 300.0, 'tool': 'FILL_RECT', 'x': 200.0},
         {'width': 260.0-240.0, 'line_width': 1.0, 'height': 460.0-380.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 380.0, 'tool': 'FILL_RECT', 'x': 240.0},
         {'width': 280.0-260.0, 'line_width': 1.0, 'height': 460.0-440.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'y': 440.0, 'tool': 'FILL_RECT', 'x': 260.0}]
        ,
        # Dog
        [{'width': 180.0-160.0, 'line_width': 1.0, 'height': 200.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 180.0, 'tool': 'FILL_RECT', 'x': 160.0},
         {'width': 340.0-180.0, 'line_width': 1.0, 'height': 240.0-200.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 200.0, 'tool': 'FILL_RECT', 'x': 180.0},
         {'width': 200.0-180.0, 'line_width': 1.0, 'height': 280.0-240.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 240.0, 'tool': 'FILL_RECT', 'x': 180.0},
         {'width': 340.0-320.0, 'line_width': 1.0, 'height': 280.0-240.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 240.0, 'tool': 'FILL_RECT', 'x': 320.0},
         {'width': 380.0-320.0, 'line_width': 1.0, 'height': 200.0-160.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x831891A0L, 'y': 160.0, 'tool': 'FILL_RECT', 'x': 320.0}]
        ,
        # Fish
        [{'radius_x': 90, 'line_width': 1.0, 'radius_y': 60, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xffff0cA0L, 'center_y': 160.0+60, 'tool': 'FILL_CIRCLE', 'center_x': 180.0+90},
         {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x1010FFA0L, 'center_y': 200.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 320.0+10},
         {'width': 180.0-160.0, 'line_width': 1.0, 'height': 260.0-180.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xffff0cA0L, 'y': 180.0, 'tool': 'FILL_RECT', 'x': 160.0}]
        ,
        # Boat
        [{'tool': 'LINE', 'points': (260.0, 340.0, 420.0, 340.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 260.0, 260.0, 340.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 260.0, 420.0, 260.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (340.0, 260.0, 360.0, 220.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (360.0, 220.0, 420.0, 220.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'center_y': 280.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 300.0+10},
         {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'center_y': 280.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 340.0+10},
         {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'center_y': 280.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 380.0+10}]
        ,
        # Spaceship
        [{'tool': 'LINE', 'points': (220.0, 400.0, 340.0, 400.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (340.0, 400.0, 360.0, 420.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (360.0, 320.0, 360.0, 420.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (340.0, 300.0, 360.0, 320.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (340.0, 100.0, 340.0, 300.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (200.0, 420.0, 220.0, 400.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (200.0, 320.0, 200.0, 420.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (200.0, 320.0, 220.0, 300.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 100.0, 220.0, 300.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 100.0, 280.0, 20.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (280.0, 20.0, 340.0, 100.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 100.0, 340.0, 100.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 120.0, 340.0, 120.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 300.0, 340.0, 300.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (280.0, 300.0, 280.0, 420.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL}]
        ,
        # Question mark
        [{'tool': 'LINE', 'points': (280.0, 260.0, 280.0, 440.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (280.0, 260.0, 340.0, 220.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (340.0, 160.0, 340.0, 220.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (280.0, 120.0, 340.0, 160.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 160.0, 280.0, 120.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 160.0, 220.0, 200.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 200.0, 260.0, 220.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL}]
        ,
        # Flying toy (cerf volant in french)
        [{'tool': 'LINE', 'points': (160.0, 140.0, 260.0, 100.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (160.0, 140.0, 160.0, 220.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (160.0, 220.0, 260.0, 380.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (260.0, 380.0, 360.0, 220.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (360.0, 140.0, 360.0, 220.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (260.0, 100.0, 360.0, 140.0), 'line_width': 8.0, 'stroke_color_rgba': 0x1010FFFFL},
         {'tool': 'LINE', 'points': (220.0, 500.0, 260.0, 380.0), 'line_width': 8.0, 'stroke_color_rgba': 707406591L},
         {'width': 240.0-220.0, 'line_width': 1.0, 'height': 180.0-160.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 160.0, 'tool': 'FILL_RECT', 'x': 220.0},
         {'width': 300.0-280.0, 'line_width': 1.0, 'height': 220.0-200.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 200.0, 'tool': 'FILL_RECT', 'x': 280.0},
         {'width': 240.0-220.0, 'line_width': 1.0, 'height': 260.0-240.0, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0xFF0000A0L, 'y': 240.0, 'tool': 'FILL_RECT', 'x': 220.0},
         {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'center_y': 160.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 280.0+10},
         {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'center_y': 200.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 220.0+10},
         {'radius_x': 10, 'line_width': 1.0, 'radius_y': 10, 'stroke_color_rgba': 255L, 'fill_color_rgba': 0x33FF00A0L, 'center_y': 240.0+10, 'tool': 'FILL_CIRCLE', 'center_x': 280.0+10}]

      ]

    # No more than 9 level allowed in gcompris
    self.gcomprisBoard.number_of_sublevel=math.ceil(len(self.drawlist)/9.0)
    self.gcomprisBoard.maxlevel=min(9, math.ceil(float(len(self.drawlist))/self.gcomprisBoard.number_of_sublevel))

  def ok_event(self, widget, target, event=None):
    self.ok()

#  gcompris - redraw
# 
# Time-stamp: <2001/08/20 00:54:45 bruno>
# 
# Copyright (C) 2003 Bruno Coudoin
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 

# PythonTest Board module
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gtk
import gtk.gdk

class Gcompris_redraw:
  """The Re-drawing activity"""
  

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    # TOOL SELECTION
    self.tools = [
      ["RECT",           "draw/tool-rectangle.png",       "draw/tool-rectangle_on.png",          gcompris.CURSOR_RECT],
      ["FILL_RECT",      "draw/tool-filledrectangle.png", "draw/tool-filledrectangle_on.png",    gcompris.CURSOR_FILLRECT],
      ["CIRCLE",         "draw/tool-circle.png",          "draw/tool-circle_on.png",             gcompris.CURSOR_CIRCLE],
      ["FILL_CIRCLE",    "draw/tool-filledcircle.png",    "draw/tool-filledcircle_on.png",       gcompris.CURSOR_FILLCIRCLE],
      ["LINE",           "draw/tool-line.png",            "draw/tool-line_on.png",               gcompris.CURSOR_LINE],
      ["DEL",            "draw/tool-del.png",             "draw/tool-del_on.png",                gcompris.CURSOR_DEL],
      ["FILL",           "draw/tool-fill.png",            "draw/tool-fill_on.png",               gcompris.CURSOR_FILL],
      ["SELECT",         "draw/tool-select.png",          "draw/tool-select_on.png",             gcompris.CURSOR_SELECT]
      ]

    self.current_tool=0

    # COLOR SELECTION
    self.colors = [ 0x2A2A2AFFL,   
                    0x7C4B21FFL,   
                    0xEE0000FFL,   
                    0xFF8C45FFL,   
                    0xFFFF0cFFL,   
                    0xB9BC0DFFL,   
                    0x14FF3bFFL,   
                    0xA4FFB3FFL,   
                    0xFF53AFFFL,   
                    0x831891FFL,   
                    0x666666FFL,
                    0xF0F0F0FFL]

    self.current_color = 0
    self.current_step = 0

    self.current_drawing = []
    self.image_target    = []

    # Define the coord of each drawing area
    self.drawing_area = [140.0, 20.0, 420.0, 500.0]
    self.target_area  = [460.0, 20.0, 740.0, 500.0]

    # Global used for the select event
    self.in_select_ofx = -1
    self.in_select_ofy = -1
    
    # Set it to 1 to let you create new forms
    # Once set, draw your shape in the right area. Then clic on OK to display
    # the data for the form (in the console). Then copy the data in the list at the end of
    # this file in init_item_list
    # Set self.editing_mode = None to forbid drawing in the right area
    #    self.editing_mode = None
    self.editing_mode = 1
    

  def start(self):  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    
    gcompris.bar_set(gcompris.BAR_OK|gcompris.BAR_LEVEL)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.draw_tools()
    self.draw_colors()
    self.draw_drawing_area(20)

    self.init_item_list()
    self.display_current_level()

    
  def end(self):
    # Remove the root item removes all the others inside it
    gcompris.set_cursor(gcompris.CURSOR_DEFAULT);
    self.rootitem.destroy()
    self.root_drawingitem.destroy()
    self.root_targetitem.destroy()


  def pause(self, pause):  
    # Do nothing special
    return
                  

  def ok(self):
    # Save a copy of the target drawing future comparison
    target = list(self.image_target)

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

    # First, simply check number of items matches
    if(len(target) != len(source)):
      gcompris.bonus.display(0, gcompris.bonus.FLOWER)
      return
    
    # Need to check if target image equals drawing image
    for i in source:
      for j in target:
        if i == j:
          target.remove(j)
          
    if(len(target) == 0):
      self.erase_drawing_area()
      if (self.increment_level() == 1):
        gcompris.bonus.display(1, gcompris.bonus.FLOWER)
        self.display_current_level()
    else:
      gcompris.bonus.display(0, gcompris.bonus.FLOWER)
      
  # Called by gcompris when the user click on the level icon
  def set_level(self, level):
    self.gcomprisBoard.level=level;
    self.gcomprisBoard.sublevel=1;

    self.erase_drawing_area()
    self.display_current_level()
            
  def repeat(self):
    print("Gcompris_redraw repeat.")
            

  def config(self):
    print("Gcompris_redraw config.")
              
  def key_press(self, keyval):
    print("got key %i" % keyval)
    return

  # Erase any displayed items (drawing and target)
  def erase_drawing_area(self):
    self.root_targetitem.destroy()
    self.root_drawingitem.destroy()

  # Display the current level target
  def display_current_level(self):
    # Set the level in the control bar
    gcompris.bar_set_level(self.gcomprisBoard);

    self.draw_image_target(self.drawlist[(self.gcomprisBoard.level-1)*self.gcomprisBoard.number_of_sublevel+
                                         (self.gcomprisBoard.sublevel-1)])

    # Create our rootitem for drawing items
    self.root_drawingitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    # Reset the drawing
    self.current_drawing = []

  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return 1 if continue, 0 if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      if(self.gcomprisBoard.level>self.gcomprisBoard.maxlevel):
        # the current board is finished : bail out
        gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
        return 0
      
    return 1
        
  # Display the tools
  def draw_tools(self):

    self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("draw/tool-selector.jpg"),
      x=5,
      y=20.0,
      width=30.0
      )

    x1=11.0
    x2=56.0
    y=30.0
    stepy=45

    # Display the tools
    for i in range(0,len(self.tools)):
      if(i%2):
        theX = x2
      else:
        theX = x1
        
      item = self.rootitem.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = gcompris.utils.load_pixmap(self.tools[i][1]),
        x=theX,
        y=y
        )
      item.connect("event", self.tool_item_event, i)
      if i%2:
        y += stepy

    # The last item is select, we select it by default
    self.current_tool = i
    self.old_tool_item = item
    self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[i][2]))
    gcompris.set_cursor(self.tools[i][3]);


  # Event when a tool is selected
  def tool_item_event(self, item, event, tool):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        # Deactivate old button
        self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[self.current_tool][1]))

        # Activate new button                         
        self.current_tool = tool
        self.old_tool_item = item
        self.old_tool_item.set(pixbuf = gcompris.utils.load_pixmap(self.tools[self.current_tool][2]))
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
        
      item = self.rootitem.add(
        gnome.canvas.CanvasRect,
        fill_color_rgba = self.colors[i],
        x1=theX,
        y1=y,
        x2=theX+30,
        y2=y+30,
        width_units=0.0,
        outline_color_rgba= 0x144B9DFFL
        )
      item.connect("event", self.color_item_event, i)
      if i%2:
        y += stepy

    # The last item is the one we select by default
    self.current_color = i
    self.old_color_item = item
    self.old_color_item.set(width_units = 4.0,
                            outline_color_rgba= 0x16EC3DFFL)

  # Color event
  def color_item_event(self, item, event, color):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        # Deactivate old button
        self.old_color_item.set(width_units = 0.0,
                                outline_color_rgba= 0x144B9DFFL)

        # Activate new button
        self.current_color = color
        self.old_color_item = item
        self.old_color_item.set(width_units = 4.0,
                                outline_color_rgba= 0x16EC3DFFL)


  # Display the drawing area
  def draw_drawing_area(self, step):

    self.current_step = step

    x1=self.drawing_area[0]
    y1=self.drawing_area[1]
    x2=self.drawing_area[2]
    y2=self.drawing_area[3]

    item = self.rootitem.add (
      gnome.canvas.CanvasRect,
      x1=x1,
      y1=y1,
      x2=x2,
      y2=y2,
      fill_color_rgba=0x0099FFFFL,
      width_units=2.0,
      outline_color_rgba=0x111199FFL
      )
    item.connect("event", self.create_item_event)

    self.draw_grid(x1,x2,y1,y2,step)

    x1=self.target_area[0]
    y1=self.target_area[1]
    x2=self.target_area[2]
    y2=self.target_area[3]

    item = self.rootitem.add (
      gnome.canvas.CanvasRect,
      x1=x1,
      y1=y1,
      x2=x2,
      y2=y2,
      fill_color_rgba=0x0099FFFFL,
      width_units=2.0,
      outline_color_rgba=0x111199FFL
      )

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

  
  # Draw the image target
  def draw_image_target(self, drawing):

    # Save the drawing in image_target for future comparison
    self.image_target = drawing
    
    # Create our rootitem for target items
    self.root_targetitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    # The images target are always drawn on the drawing area to ease the final comparison
    xofset = self.target_area[0] - self.drawing_area[0]
    
    for i in drawing:
      #
      # Can specify the item type to draw via a real GTK type or a TOOL string 
      if(i.has_key('type')):
        item = self.root_targetitem.add ( i['type'] )
      elif(i.has_key('tool')):
        if(i['tool'] == "RECT"):
          item = self.root_targetitem.add ( gnome.canvas.CanvasRect )
        elif(i['tool'] == "FILL_RECT"):
          item = self.root_targetitem.add ( gnome.canvas.CanvasRect )
        elif(i['tool'] == "CIRCLE"):
          item = self.root_targetitem.add ( gnome.canvas.CanvasEllipse )
        elif(i['tool'] == "FILL_CIRCLE"):
          item = self.root_targetitem.add ( gnome.canvas.CanvasEllipse )
        elif(i['tool'] == "LINE"):
          item = self.root_targetitem.add ( gnome.canvas.CanvasLine )
        else:
          print ("ERROR: incorrect type in draw_image_target", i)
      
      for k, v in i.items():
        if k == 'fill_color' :
          item.set ( fill_color = v )
        elif k == 'fill_color_rgba' :
          item.set ( fill_color_rgba = v )
        elif k == 'height' :
          item.set ( height = v )
        elif k == 'height_set' :
          item.set ( height_set = v )
        elif k == 'outline_color' :
          item.set ( outline_color = v )
        elif k == 'outline_color_rgba' :
          item.set ( outline_color_rgba = v )
        elif k == 'points' :
          v2 = (v[0]+xofset, v[1], v[2]+xofset, v[3])
          item.set ( points = v2)
        elif k == 'width_units' :
          item.set ( width_units = v )
        elif k == 'width_pixels' :
          item.set ( width_pixels = v )
        elif k == 'width' :
          v[0] += xofset
          v[2] += xofset
          item.set ( width = v )
        elif k == 'width_set' :
          item.set ( width_set = v )
        elif k == 'x' :
          item.set ( x = v + xofset)
        elif k == 'y' :
          item.set ( y = v)
        elif k == 'x1' :
          item.set ( x1 = v + xofset)
        elif k == 'y1' :
          item.set ( y1 = v)
        elif k == 'x2' :
          item.set ( x2 = v + xofset)
        elif k == 'y2' :
          item.set ( y2 = v)

  def draw_grid(self, x1, x2, y1, y2, step):

    # Coord of the written numbers
    x_text = x1 - 14
    y_text = y1 - 10
    
    for i in range(x1,x2,step):
      item = self.rootitem.add (
        gnome.canvas.CanvasLine,
        points=(i , y1, i , y2),
        fill_color_rgba=0x3740E3FFL,
        width_units=1.0,
        )
      # Shadow for text number
      self.rootitem.add (
        gnome.canvas.CanvasText,
        text=int((i-x1) / step),
        font=gcompris.skin.get_font("gcompris/content"),
        x=i+1,
        y=y_text+1,
        fill_color_rgba=0x000000FFL
        )
      # Text number
      self.rootitem.add (
        gnome.canvas.CanvasText,
        text=int((i-x1) / step),
        font=gcompris.skin.get_font("gcompris/content"),
        x=i,
        y=y_text,
        fill_color_rgba=0xFFFFA0FFL
        )
      
      # Clicking on lines let you create object
      if(x1<self.target_area[0]):
        item.connect("event", self.create_item_event)

    for i in range(y1,y2,step):
      item = self.rootitem.add (
        gnome.canvas.CanvasLine,
        points=(x1, i, x2 , i),
        fill_color_rgba=0x3740E3FFL,
        width_units=1.0,
        )
      # Shadow for text number
      self.rootitem.add (
        gnome.canvas.CanvasText,
        text=int((i-y1) / step),
        font=gcompris.skin.get_font("gcompris/content"),
        x=x_text+1,
        y=i+1,
        fill_color_rgba=0x000000FFL
        )
      # Text number
      self.rootitem.add (
        gnome.canvas.CanvasText,
        text=int((i-y1) / step),
        font=gcompris.skin.get_font("gcompris/content"),
        x=x_text,
        y=i,
        fill_color_rgba=0xFFFFA0FFL
        )

      # Clicking on lines let you create object
      if(x1<self.target_area[0]):
        item.connect("event", self.create_item_event)

  # Given x,y return a new x,y snapped to the grid
  def snap_to_grid(self, x, y):
    result = []
    tmp = round(((x+(self.current_step)) -
               self.drawing_area[0])/self.current_step) - 1
    result.append(float(self.drawing_area[0] + tmp*self.current_step))

    tmp = round(((y+(self.current_step)) -
               self.drawing_area[2])/self.current_step) - 1
    result.append(float(self.drawing_area[2] + tmp*self.current_step))
    return result


  # Event when a click on any item. Perform the move
  def move_item_event(self, item, event, item_index):

    if self.tools[self.current_tool][0] != "SELECT":
      return gtk.FALSE

    #
    # MOUSE DRAG STOP
    # ---------------
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        
        # Reset thein_select_ofx ofset
        self.in_select_ofx = -1
        self.in_select_ofy = -1

        return gtk.TRUE
        
    if event.state & gtk.gdk.BUTTON1_MASK:
      x=event.x
      y=event.y

      # Workaround for bad line positionning
      if(self.current_drawing[item_index].has_key('points')):
        item.set(width_units=1.0)

      bounds = item.get_bounds()

      # Save the ofset between the mouse pointer and the upper left corner of the object
      if(self.in_select_ofx == -1):
        self.in_select_ofx = x-bounds[0]
        self.in_select_ofy = y-bounds[1]

      x -= self.in_select_ofx
      y -= self.in_select_ofy

      x,y = self.snap_to_grid(x,y)

      # Check drawing boundaries
      if(x<self.drawing_area[0]):
        x=self.drawing_area[0]
      if(x>(self.drawing_area[2]-(bounds[2]-bounds[0]))):
        x=self.drawing_area[2]-(bounds[2]-bounds[0])
      if(y<self.drawing_area[1]):
        y=self.drawing_area[1]
      if(y>(self.drawing_area[3]-(bounds[3]-bounds[1]))):
        y=self.drawing_area[3]-(bounds[3]-bounds[1])

      # Need to update current_drawing
      if(self.current_drawing[item_index].has_key('x1')):
        # It's not a line
        ox = x - self.current_drawing[item_index]['x1']
        oy = y - self.current_drawing[item_index]['y1']
        self.current_drawing[item_index]['x1'] += ox
        self.current_drawing[item_index]['y1'] += oy
        self.current_drawing[item_index]['x2'] += ox
        self.current_drawing[item_index]['y2'] += oy
      else:
        # It can only be a line
        ox = x - min(self.current_drawing[item_index]['points'][0], self.current_drawing[item_index]['points'][2])
        oy = y - min(self.current_drawing[item_index]['points'][1], self.current_drawing[item_index]['points'][3])
        nx1 = self.current_drawing[item_index]['points'][0] + ox
        ny1 = self.current_drawing[item_index]['points'][1] + oy
        nx2 = self.current_drawing[item_index]['points'][2] + ox
        ny2 = self.current_drawing[item_index]['points'][3] + oy
        self.current_drawing[item_index]['points'] = (nx1, ny1, nx2, ny2)

      # Now perform the object move
      gcompris.utils.item_absolute_move(item, x, y)

      # Workaround for bad line positionning
      if(self.current_drawing[item_index].has_key('points')):
        item.set(width_units=8.0)

      return gtk.TRUE
    
    return gtk.FALSE
          
  # Event when a click on an item happen on fill in type object
  def fillin_item_event(self, item, event, drawing_item_index):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "FILL":
          item.set(fill_color_rgba=self.colors[self.current_color])
          # Reset the item to our list
          self.current_drawing[drawing_item_index]['fill_color_rgba'] = self.colors[self.current_color]
          return gtk.TRUE
    return gtk.FALSE
          
  # Event when a click on an item happen on border fill type object
  def fillout_item_event(self, item, event, drawing_item_index):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "FILL":
          item.set(outline_color_rgba=self.colors[self.current_color])
          # Reset the item to our list
          self.current_drawing[drawing_item_index]['outline_color_rgba'] = self.colors[self.current_color]
          return gtk.TRUE
    return gtk.FALSE

  # Del an item and internal struct cleanup
  def del_item(self, item, drawing_item_index):
    item.destroy()
    # Warning, do not realy delete it or we bug the index of other items
    self.current_drawing[drawing_item_index] = []

  # Event when a click on an item happen
  def del_item_event(self, item, event, drawing_item_index):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "DEL":
          self.del_item(item, drawing_item_index);
          return gtk.TRUE
    return gtk.FALSE


  # Event when a click on the drawing area happen
  def create_item_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        self.newitem = None
        
        if (self.tools[self.current_tool][0] == "DEL" or
            self.tools[self.current_tool][0] == "SELECT" or
            self.tools[self.current_tool][0] == "FILL"):
          # This event is treated in del_item_event to avoid
          # operating on background item and grid
          return gtk.FALSE
        
        elif self.tools[self.current_tool][0] == "LINE":
          
          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y

          self.newitem = self.root_drawingitem.add(
            gnome.canvas.CanvasLine,
            points=(self.pos_x, self.pos_y, x, y),          
            fill_color_rgba=self.colors[self.current_color],
            width_units=8.0
            )
          self.newitem.connect("event", self.fillin_item_event, len(self.current_drawing))
          self.newitem.connect("event", self.move_item_event, len(self.current_drawing))

          # Add the new item to our list
          self.current_drawing.append({'tool': self.tools[self.current_tool][0],
                                        'points':(self.pos_x, self.pos_y, x, y),
                                        'fill_color_rgba':self.colors[self.current_color],
                                        'width_units':8.0})

        elif self.tools[self.current_tool][0] == "RECT":
          
          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y
          
          self.newitem = self.root_drawingitem.add(
            gnome.canvas.CanvasRect,
            x1=self.pos_x,
            y1=self.pos_y,
            x2=x,
            y2=y,
            outline_color_rgba=self.colors[self.current_color],
            width_units=4.0
            )
          self.newitem.connect("event", self.fillout_item_event, len(self.current_drawing))
          self.newitem.connect("event", self.move_item_event, len(self.current_drawing))

          # Add the new item to our list
          self.current_drawing.append({'tool': self.tools[self.current_tool][0],
                                       'x1':self.pos_x,
                                       'y1':self.pos_y,
                                       'x2':x,
                                       'y2':y,
                                       'outline_color_rgba':self.colors[self.current_color],
                                       'width_units':4.0})
                                      
        elif self.tools[self.current_tool][0] == "FILL_RECT":

          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y
          
          self.newitem = self.root_drawingitem.add(
            gnome.canvas.CanvasRect,
            x1=self.pos_x,
            y1=self.pos_y,
            x2=x,
            y2=y,
            fill_color_rgba=self.colors[self.current_color],
            outline_color_rgba=0x000000FFL,
            width_units=1.0
            )
          self.newitem.connect("event", self.fillin_item_event, len(self.current_drawing))
          self.newitem.connect("event", self.move_item_event, len(self.current_drawing))

          # Add the new item to our list
          self.current_drawing.append({'tool': self.tools[self.current_tool][0],
                                       'x1':self.pos_x,
                                       'y1':self.pos_y,
                                       'x2':x,
                                       'y2':y,
                                       'fill_color_rgba':self.colors[self.current_color],
                                       'outline_color_rgba':0x000000FFL,
                                       'width_units':1.0})

        elif self.tools[self.current_tool][0] == "CIRCLE":

          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y
          
          self.newitem = self.root_drawingitem.add(
            gnome.canvas.CanvasEllipse,
            x1=self.pos_x,
            y1=self.pos_y,
            x2=x,
            y2=y,
            outline_color_rgba=self.colors[self.current_color],
            width_units=5.0
            )
          self.newitem.connect("event", self.fillout_item_event, len(self.current_drawing))
          self.newitem.connect("event", self.move_item_event, len(self.current_drawing))

          # Add the new item to our list
          self.current_drawing.append({'tool': self.tools[self.current_tool][0],
                                       'x1':self.pos_x,
                                       'y1':self.pos_y,
                                       'x2':x,
                                       'y2':y,
                                       'outline_color_rgba':self.colors[self.current_color],
                                       'width_units':5.0})

        elif self.tools[self.current_tool][0] == "FILL_CIRCLE":

          x,y = self.snap_to_grid(event.x,event.y)
          self.pos_x = x
          self.pos_y = y
          
          self.newitem = self.root_drawingitem.add(
            gnome.canvas.CanvasEllipse,
            x1=self.pos_x,
            y1=self.pos_y,
            x2=x,
            y2=y,
            fill_color_rgba=self.colors[self.current_color],
            outline_color_rgba=0x000000FFL,
            width_units=1.0
            )
          self.newitem.connect("event", self.fillin_item_event, len(self.current_drawing))
          self.newitem.connect("event", self.move_item_event, len(self.current_drawing))
          
          # Add the new item to our list
          self.current_drawing.append({'tool': self.tools[self.current_tool][0],
                                       'x1':self.pos_x,
                                       'y1':self.pos_y,
                                       'x2':x,
                                       'y2':y,
                                       'fill_color_rgba':self.colors[self.current_color],
                                       'outline_color_rgba':0x000000FFL,
                                       'width_units':1.0})
        if self.newitem != 0:
          self.newitem.connect("event", self.create_item_event)
          self.newitem.connect("event", self.del_item_event, len(self.current_drawing)-1)
          
      return gtk.TRUE

    #
    # MOTION EVENT
    # ------------
    if event.type == gtk.gdk.MOTION_NOTIFY:
      if event.state & gtk.gdk.BUTTON1_MASK:
        x=event.x
        y=event.y
        x,y = self.snap_to_grid(event.x,event.y)
        
        # Check drawing boundaries
        if(event.x<self.drawing_area[0]):
          x=self.drawing_area[0]
        if(event.x>self.drawing_area[2]):
          x=self.drawing_area[2]
        if(event.y<self.drawing_area[1]):
          y=self.drawing_area[1]
        if(event.y>self.drawing_area[3]):
          y=self.drawing_area[3]
          
        if self.tools[self.current_tool][0] == "LINE":
          self.newitem.set( points=( self.pos_x, self.pos_y, x, y) )
          # Reset the item to our list
          self.current_drawing[len(self.current_drawing)-1]['points'] = ( self.pos_x, self.pos_y, x, y)
        elif (self.tools[self.current_tool][0] == "RECT" or
              self.tools[self.current_tool][0] == "FILL_RECT" or
              self.tools[self.current_tool][0] == "CIRCLE" or
              self.tools[self.current_tool][0] == "FILL_CIRCLE"):
          self.newitem.set(
            x2=x,
            y2=y)
          # Reset the item to our list
          self.current_drawing[len(self.current_drawing)-1]['x2'] = x
          self.current_drawing[len(self.current_drawing)-1]['y2'] = y
          

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
              self.tools[self.current_tool][0] == "FILL_RECT" or
              self.tools[self.current_tool][0] == "CIRCLE" or
              self.tools[self.current_tool][0] == "FILL_CIRCLE"):
          # It's a rect or ellipse
          x1 = self.current_drawing[len(self.current_drawing)-1]['x1']
          y1 = self.current_drawing[len(self.current_drawing)-1]['y1']
          x2 = self.current_drawing[len(self.current_drawing)-1]['x2']
          y2 = self.current_drawing[len(self.current_drawing)-1]['y2']
          if (x1 == x2) or (y1 == y2):
            # Oups, empty rect
            self.del_item(self.newitem, len(self.current_drawing)-1)
          else:
            # We need to reord the coord in increasing order to allow later comparison
            x1, y1, x2, y2 = self.reorder_coord(x1, y1, x2, y2)
            self.current_drawing[len(self.current_drawing)-1]['x1'] = x1
            self.current_drawing[len(self.current_drawing)-1]['y1'] = y1
            self.current_drawing[len(self.current_drawing)-1]['x2'] = x2
            self.current_drawing[len(self.current_drawing)-1]['y2'] = y2
          
        return gtk.TRUE
    return gtk.FALSE


  # The list of items (data) for this game
  def init_item_list(self):
    self.drawlist = \
    [
      # Two stripes
      [{'x2': 420.0, 'width_units': 1.0, 'y2': 100.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 80.0, 'tool': 'FILL_RECT', 'x1': 140.0}, {'x2': 420.0, 'width_units': 1.0, 'y2': 440.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 420.0, 'tool': 'FILL_RECT', 'x1': 140.0}]
      ,
      # Top centered box
      [{'x2': 360.0, 'y2': 80.0, 'width_units': 1.0, 'fill_color_rgba': 4042322175L, 'outline_color_rgba': 255L, 'y1': 40.0, 'x1': 200.0, 'tool': 'FILL_RECT'}]
      ,
      # 4 small corners
      [{'x2': 200.0, 'y2': 80.0,  'width_units': 1.0,'fill_color_rgba': 4042322175L, 'outline_color_rgba': 255L, 'y1': 40.0, 'x1': 160.0, 'tool': 'FILL_RECT'},
       {'x2': 400.0, 'y2': 80.0,  'width_units': 1.0,'fill_color_rgba': 4042322175L, 'outline_color_rgba': 255L, 'y1': 40.0, 'x1': 360.0, 'tool': 'FILL_RECT'},
       {'x2': 200.0, 'y2': 480.0, 'width_units': 1.0, 'fill_color_rgba': 4042322175L, 'outline_color_rgba': 255L, 'y1': 440.0, 'x1': 160.0, 'tool': 'FILL_RECT'},
       {'x2': 400.0, 'y2': 480.0,  'width_units': 1.0,'fill_color_rgba': 4042322175L, 'outline_color_rgba': 255L, 'y1': 440.0, 'x1': 360.0, 'tool': 'FILL_RECT'}]
      ,
      # 4 non filled Rects organised in rect and shifted
      [{'x2': 200.0, 'y2': 360.0, 'outline_color_rgba': 4042322175L, 'width_units': 4.0, 'y1': 200.0, 'x1': 180.0, 'tool': 'RECT'},
       {'x2': 340.0, 'y2': 400.0, 'outline_color_rgba': 4042322175L, 'width_units': 4.0, 'y1':380.0, 'x1': 180.0, 'tool': 'RECT'},
       {'x2': 380.0, 'y2': 400.0, 'outline_color_rgba': 4042322175L, 'width_units': 4.0, 'y1': 240.0, 'x1': 360.0, 'tool': 'RECT'},
       {'x2': 380.0, 'y2': 220.0, 'outline_color_rgba': 4042322175L, 'width_units': 4.0, 'y1': 200.0, 'x1': 220.0, 'tool': 'RECT'}]
      ,
      # Letter A
      [{'tool': 'LINE', 'points': (200.0, 120.0, 280.0, 120.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (280.0, 120.0, 280.0,240.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (200.0, 120.0, 200.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (200.0, 180.0, 280.0, 180.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}]
      ,
      # Letter B
      [{'tool': 'LINE', 'points': (240.0, 240.0, 320.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (300.0, 180.0, 320.0,200.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (300.0, 180.0, 320.0, 160.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (240.0, 180.0, 300.0, 180.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (320.0, 100.0, 320.0, 160.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool':'LINE', 'points': (240.0, 100.0, 320.0, 100.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (240.0, 100.0, 240.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (320.0, 200.0, 320.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}]
      ,
      # A door
      [{'x2': 360.0, 'y2': 360.0, 'outline_color_rgba': 4042322175L, 'width_units': 4.0, 'y1': 180.0, 'x1': 200.0, 'tool': 'RECT'},
       {'x2': 340.0, 'y2': 320.0, 'width_units': 1.0, 'fill_color_rgba': 4042322175L, 'outline_color_rgba': 255L, 'y1': 280.0, 'x1': 300.0, 'tool': 'FILL_CIRCLE'},
       {'x2': 320.0, 'y2': 260.0, 'width_units': 1.0, 'fill_color_rgba': 4042322175L, 'outline_color_rgba': 255L, 'y1': 200.0, 'x1': 240.0, 'tool': 'FILL_RECT'}]
      ,
      # A top left kind of target
      [{'x2': 260.0, 'y2': 140.0, 'outline_color_rgba': 4042322175L, 'y1': 40.0, 'x1': 160.0, 'tool': 'CIRCLE', 'width_units': 5.0},
       {'x2': 240.0, 'y2': 120.0, 'width_units': 1.0, 'fill_color_rgba': 4042322175L, 'outline_color_rgba': 255L, 'y1': 60.0, 'x1': 180.0, 'tool': 'FILL_CIRCLE'}]
      ,
      # 4 Huge Diagonal lines
      [{'tool': 'LINE', 'points': (140.0, 260.0, 260.0, 20.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (140.0, 260.0, 260.0, 500.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (260.0, 500.0, 420.0, 260.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'tool': 'LINE', 'points': (260.0, 20.0, 420.0, 260.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}]
      ,
      # Balloon
      [{'tool': 'LINE', 'points': (220.0, 240.0, 220.0, 340.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}, {'x2': 260.0, 'width_units': 1.0, 'y2': 240.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 140.0, 'tool': 'FILL_CIRCLE', 'x1': 180.0}, {'x2': 380.0, 'width_units': 1.0, 'y2': 180.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4283674623L, 'y1': 80.0, 'tool': 'FILL_CIRCLE', 'x1': 300.0}, {'tool': 'LINE', 'points': (340.0, 180.0, 340.0, 280.0), 'width_units': 8.0, 'fill_color_rgba': 4042322175L}]
      ,
      # Watch
      [{'x2': 380.0, 'y2': 340.0, 'outline_color_rgba': 4042322175L, 'width_units': 5.0, 'y1': 140.0, 'tool': 'CIRCLE', 'x1': 180.0}, {'tool': 'LINE', 'points': (280.0, 160.0, 280.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'tool': 'LINE', 'points': (280.0, 240.0, 320.0, 200.0), 'width_units': 8.0, 'fill_color_rgba': 352271359L}, {'tool': 'LINE', 'points': (220.0, 280.0, 280.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 4294905087L}]
      ,
      # Colored pyramid
      [{'x2': 280.0, 'width_units': 1.0, 'y2': 120.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 100.0, 'tool': 'FILL_RECT', 'x1': 260.0}, {'x2': 300.0, 'width_units': 1.0, 'y2': 140.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4294905087L, 'y1': 120.0, 'tool': 'FILL_RECT', 'x1': 240.0}, {'x2': 320.0, 'width_units': 1.0, 'y2': 160.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 140.0, 'tool': 'FILL_RECT', 'x1': 220.0}, {'x2': 340.0, 'width_units': 1.0, 'y2': 180.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4283674623L, 'y1': 160.0, 'tool': 'FILL_RECT', 'x1': 200.0}, {'x2': 360.0, 'width_units': 1.0, 'y2': 200.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 1717987071L, 'y1': 180.0, 'tool': 'FILL_RECT', 'x1': 180.0}]
      ,
      # Colored Rectangle bigger and bigger
      [{'x2': 180.0, 'width_units': 1.0, 'y2': 60.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4283674623L, 'y1': 20.0, 'tool': 'FILL_RECT', 'x1': 140.0}, {'x2': 240.0, 'width_units': 1.0, 'y2': 120.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 60.0, 'tool': 'FILL_RECT', 'x1': 180.0}, {'x2': 320.0, 'width_units': 1.0, 'y2': 200.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4294905087L, 'y1': 120.0, 'tool': 'FILL_RECT', 'x1': 240.0}, {'x2': 420.0, 'width_units': 1.0, 'y2': 300.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 200.0, 'tool': 'FILL_RECT', 'x1': 320.0}]
      ,
      # Tree
      [{'x2': 420.0, 'width_units': 1.0, 'y2': 500.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2085298687L, 'y1': 460.0, 'tool': 'FILL_RECT', 'x1': 140.0}, {'x2': 260.0, 'width_units': 1.0, 'y2': 460.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4287383039L, 'y1': 360.0, 'tool': 'FILL_RECT', 'x1': 240.0}, {'x2': 320.0, 'width_units': 1.0, 'y2': 360.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 280.0, 'tool': 'FILL_CIRCLE', 'x1': 180.0}]
      ,
      # bipbip (big non flying bird)
      [{'x2': 280.0, 'width_units': 1.0, 'y2': 320.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 120.0, 'tool': 'FILL_RECT', 'x1': 260.0}, {'x2': 300.0, 'width_units': 1.0, 'y2': 120.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 80.0, 'tool': 'FILL_RECT', 'x1': 260.0}, {'x2': 320.0, 'width_units': 1.0, 'y2': 120.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 100.0, 'tool': 'FILL_RECT', 'x1': 300.0}, {'x2': 280.0, 'width_units': 1.0, 'y2': 380.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 320.0, 'tool': 'FILL_RECT', 'x1': 200.0}, {'x2': 220.0, 'width_units': 1.0, 'y2': 320.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 300.0, 'tool': 'FILL_RECT', 'x1': 200.0}, {'x2': 260.0, 'width_units': 1.0, 'y2': 460.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 380.0, 'tool': 'FILL_RECT', 'x1': 240.0}, {'x2': 280.0, 'width_units': 1.0, 'y2': 460.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 440.0, 'tool': 'FILL_RECT', 'x1': 260.0}]
      ,
      # Dog
      [{'x2': 180.0, 'width_units': 1.0, 'y2': 200.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 180.0, 'tool': 'FILL_RECT', 'x1': 160.0}, {'x2': 340.0, 'width_units': 1.0, 'y2': 240.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 200.0, 'tool': 'FILL_RECT', 'x1': 180.0}, {'x2': 200.0, 'width_units': 1.0, 'y2': 280.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 240.0, 'tool': 'FILL_RECT', 'x1': 180.0}, {'x2': 340.0,'width_units': 1.0, 'y2': 280.0, 'outline_color_rgba': 255L, 'fill_color_rgba':2199425535L, 'y1': 240.0, 'tool': 'FILL_RECT', 'x1': 320.0}, {'x2': 380.0, 'width_units': 1.0, 'y2': 200.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 160.0, 'tool': 'FILL_RECT', 'x1': 320.0}]
      ,
      # Fish
      [{'x2': 360.0, 'width_units': 1.0, 'y2': 280.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4294905087L, 'y1': 160.0, 'tool': 'FILL_CIRCLE', 'x1': 180.0}, {'x2': 340.0, 'width_units': 1.0, 'y2': 220.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 200.0, 'tool': 'FILL_CIRCLE', 'x1': 320.0}, {'x2': 180.0, 'width_units': 1.0, 'y2': 260.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4294905087L, 'y1': 180.0, 'tool': 'FILL_RECT', 'x1': 160.0}]
      ,
      # Balloon (human)
      [{'x2': 380.0, 'width_units': 1.0, 'y2': 300.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 100.0, 'tool': 'FILL_CIRCLE', 'x1': 200.0}, {'x2': 340.0, 'width_units': 1.0, 'y2': 380.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4294905087L, 'y1': 320.0, 'tool': 'FILL_RECT', 'x1': 240.0}, {'tool': 'LINE', 'points': (220.0, 260.0, 260.0, 320.0), 'width_units': 8.0, 'fill_color_rgba': 4287383039L}, {'tool': 'LINE', 'points': (320.0, 320.0, 360.0, 260.0),
'width_units': 8.0, 'fill_color_rgba': 4287383039L}]
    ,
      # House
      [{'x2': 360.0, 'width_units': 1.0, 'y2': 340.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4287383039L, 'y1': 240.0, 'tool': 'FILL_RECT', 'x1': 200.0}, {'x2': 280.0, 'width_units': 1.0, 'y2': 340.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 280.0, 'tool': 'FILL_RECT', 'x1': 240.0}, {'x2': 340.0, 'width_units': 1.0, 'y2': 300.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 260.0, 'tool': 'FILL_RECT', 'x1': 300.0}, {'tool': 'LINE', 'points': (200.0, 240.0, 280.0, 160.0), 'width_units': 8.0, 'fill_color_rgba': 4287383039L}, {'tool': 'LINE', 'points': (280.0, 160.0, 360.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 4287383039L}]
      ,
      # Truck
      [{'x2': 220.0, 'width_units': 1.0, 'y2': 280.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 240.0, 'tool': 'FILL_CIRCLE', 'x1': 180.0}, {'x2': 320.0, 'width_units': 1.0, 'y2': 280.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 240.0, 'tool': 'FILL_CIRCLE', 'x1': 280.0}, {'x2': 220.0, 'width_units': 1.0, 'y2': 260.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 220.0, 'tool': 'FILL_RECT', 'x1': 160.0}, {'x2': 300.0, 'width_units': 1.0, 'y2': 260.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 180.0, 'tool': 'FILL_RECT', 'x1': 220.0}, {'x2': 320.0, 'width_units': 1.0, 'y2': 260.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 220.0, 'tool': 'FILL_RECT', 'x1': 300.0}, {'x2': 280.0, 'width_units': 1.0, 'y2': 260.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 200.0, 'tool': 'FILL_RECT', 'x1': 240.0}]
      ,
      # Fire truck
      [{'x2': 200.0, 'width_units': 1.0, 'y2': 300.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 260.0, 'tool': 'FILL_CIRCLE', 'x1': 160.0}, {'x2': 360.0, 'width_units': 1.0, 'y2': 300.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 260.0, 'tool': 'FILL_CIRCLE', 'x1': 320.0}, {'x2': 380.0, 'width_units': 1.0, 'y2': 280.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 220.0, 'tool': 'FILL_RECT', 'x1': 160.0}, {'tool': 'LINE', 'points': (160.0, 200.0, 340.0, 180.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'x2': 200.0, 'width_units': 1.0, 'y2': 220.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 200.0, 'tool': 'FILL_RECT', 'x1': 180.0}, {'x2': 360.0, 'width_units': 1.0, 'y2': 260.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 220.0, 'tool': 'FILL_RECT', 'x1': 320.0}, {'x2': 300.0, 'width_units': 1.0, 'y2': 260.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 220.0, 'tool': 'FILL_RECT', 'x1': 280.0}]
      ,
      # Billard
      [
      {'width_units': 8.0, 'points': (180.0, 80.0, 180.0, 440.0), 'tool': 'LINE', 'fill_color_rgba': 4042322175L},
      {'width_units': 8.0, 'points': (180.0, 460.0, 380.0, 460.0), 'tool': 'LINE', 'fill_color_rgba': 4042322175L},
      {'width_units': 8.0, 'points': (380.0, 80.0, 380.0, 440.0), 'tool': 'LINE', 'fill_color_rgba': 4042322175L},
      {'width_units': 8.0, 'points': (180.0, 60.0, 380.0, 60.0), 'tool': 'LINE', 'fill_color_rgba': 4042322175L},
      {'width_units': 8.0, 'points': (280.0, 320.0, 280.0, 420.0), 'tool': 'LINE', 'fill_color_rgba': 2199425535L},
      {'x2': 280.0, 'width_units': 1.0, 'y2': 320.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 300.0, 'x1': 260.0, 'tool': 'FILL_CIRCLE'},
      {'x2': 260.0, 'width_units': 1.0, 'y2': 120.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 100.0, 'x1': 240.0, 'tool': 'FILL_CIRCLE'},
      {'x2': 280.0, 'width_units': 1.0, 'y2': 120.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4287383039L, 'y1': 100.0, 'x1': 260.0, 'tool': 'FILL_CIRCLE'},
      {'x2': 300.0, 'width_units': 1.0, 'y2': 120.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3992977663L, 'y1': 100.0, 'x1': 280.0, 'tool': 'FILL_CIRCLE'},
      {'x2': 320.0, 'width_units': 1.0, 'y2': 120.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2085298687L, 'y1': 100.0, 'x1': 300.0, 'tool': 'FILL_CIRCLE'},
      {'x2': 280.0, 'width_units': 1.0, 'y2': 140.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 3116109311L, 'y1': 120.0, 'x1': 260.0, 'tool': 'FILL_CIRCLE'},
      {'x2': 300.0, 'width_units': 1.0, 'y2': 140.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 352271359L, 'y1': 120.0, 'x1': 280.0, 'tool': 'FILL_CIRCLE'}
      ]
      ,
      # Clara (my daughter)
      [{'x2': 240.0, 'width_units': 1.0, 'y2': 480.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 400.0, 'x1': 220.0, 'tool': 'FILL_RECT'}, {'x2': 320.0, 'width_units': 1.0, 'y2': 480.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 400.0, 'x1': 300.0, 'tool': 'FILL_RECT'}, {'x2': 220.0, 'width_units': 1.0, 'y2': 200.0,'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 180.0, 'x1': 160.0, 'tool': 'FILL_RECT'}, {'x2': 380.0, 'width_units': 1.0, 'y2': 200.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 180.0, 'x1': 320.0, 'tool': 'FILL_RECT'}, {'x2': 380.0, 'width_units': 1.0, 'y2': 180.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 120.0, 'x1': 360.0, 'tool': 'FILL_RECT'}, {'x2': 180.0, 'width_units': 1.0, 'y2': 260.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2199425535L, 'y1': 200.0, 'x1': 160.0, 'tool': 'FILL_RECT'}, {'x2': 320.0, 'width_units': 1.0, 'y2': 160.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4287383039L, 'y1': 60.0, 'x1': 220.0, 'tool': 'FILL_CIRCLE'}, {'x2': 260.0, 'width_units': 1.0, 'y2': 120.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2768221183L, 'y1': 100.0, 'x1': 240.0, 'tool': 'FILL_CIRCLE'}, {'x2': 300.0, 'width_units': 1.0, 'y2': 120.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2768221183L, 'y1': 100.0, 'x1': 280.0, 'tool': 'FILL_CIRCLE'}, {'width_units': 8.0, 'points': (260.0, 140.0, 280.0, 140.0), 'tool': 'LINE', 'fill_color_rgba': 3992977663L}, {'x2': 300.0, 'width_units': 1.0, 'y2': 180.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4287383039L, 'y1': 160.0, 'x1': 240.0, 'tool': 'FILL_RECT'}, {'x2': 320.0, 'width_units': 1.0, 'y2': 320.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4283674623L, 'y1': 180.0, 'x1': 220.0, 'tool': 'FILL_RECT'}, {'x2': 340.0, 'width_units': 1.0, 'y2': 400.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4283674623L, 'y1': 320.0, 'x1': 200.0, 'tool': 'FILL_RECT'}]
      ,
      # Bicycle
      [{'x2': 240.0, 'y2': 340.0, 'outline_color_rgba': 4287383039L, 'width_units': 5.0, 'y1': 260.0, 'tool': 'CIRCLE', 'x1': 160.0}, {'x2': 400.0, 'y2': 340.0, 'outline_color_rgba': 4287383039L, 'width_units': 5.0, 'y1': 260.0, 'tool': 'CIRCLE', 'x1': 320.0}, {'tool': 'LINE', 'points': (200.0, 300.0, 280.0, 300.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'tool': 'LINE', 'points': (280.0,300.0, 340.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'tool': 'LINE', 'points': (240.0, 240.0, 340.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'tool': 'LINE', 'points': (200.0, 300.0, 240.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'tool': 'LINE', 'points': (240.0, 220.0, 240.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 2199425535L}, {'tool': 'LINE', 'points': (220.0, 220.0, 260.0, 220.0), 'width_units': 8.0, 'fill_color_rgba': 2199425535L}, {'tool': 'LINE', 'points': (340.0, 200.0, 340.0, 240.0), 'width_units': 8.0, 'fill_color_rgba': 2199425535L}, {'tool': 'LINE', 'points': (320.0, 200.0, 340.0, 200.0), 'width_units': 8.0, 'fill_color_rgba': 2199425535L}, {'tool': 'LINE', 'points': (340.0, 240.0, 360.0, 300.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}]
      ,
      # Sea boat and sun
      [{'x2': 420.0, 'width_units': 1.0, 'y2': 500.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 2768221183L, 'y1': 420.0, 'tool': 'FILL_RECT', 'x1': 140.0}, {'x2': 260.0, 'width_units': 1.0, 'y2': 160.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4294905087L, 'y1': 60.0, 'tool': 'FILL_CIRCLE', 'x1': 160.0}, {'x2': 360.0, 'width_units': 1.0, 'y2': 200.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 160.0, 'tool': 'FILL_CIRCLE', 'x1': 260.0}, {'x2': 360.0, 'width_units': 1.0, 'y2': 220.0, 'outline_color_rgba': 255L, 'fill_color_rgba': 4042322175L, 'y1': 200.0, 'tool': 'FILL_CIRCLE', 'x1': 300.0}, {'x2': 400.0,'width_units': 1.0, 'y2': 160.0, 'outline_color_rgba': 255L, 'fill_color_rgba':4042322175L, 'y1': 140.0, 'tool': 'FILL_CIRCLE', 'x1': 360.0}, {'tool': 'LINE','points': (220.0, 400.0, 240.0, 420.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'tool': 'LINE', 'points': (240.0, 420.0, 280.0, 420.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'tool': 'LINE', 'points': (280.0, 420.0, 300.0, 400.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'tool': 'LINE', 'points': (220.0, 400.0, 300.0, 400.0), 'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'tool': 'LINE', 'points': (260.0, 280.0, 260.0, 400.0),'width_units': 8.0, 'fill_color_rgba': 3992977663L}, {'tool': 'LINE', 'points':(260.0, 280.0, 300.0, 380.0), 'width_units': 8.0, 'fill_color_rgba': 2199425535L}, {'tool': 'LINE', 'points': (260.0, 380.0, 300.0, 380.0), 'width_units': 8.0,'fill_color_rgba': 2199425535L}]

    ]

    # Take care here that the number of items in the above list must be a multiple of
    # number_of_sublevel*maxlevel
    self.gcomprisBoard.number_of_sublevel=4
    self.gcomprisBoard.maxlevel=len(self.drawlist)/self.gcomprisBoard.number_of_sublevel
    print "Number of target=", len(self.drawlist)
    print "Maxlevel=", self.gcomprisBoard.maxlevel, " sublevel=", self.gcomprisBoard.number_of_sublevel
    

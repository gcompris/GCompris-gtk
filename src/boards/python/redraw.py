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
    self.colors = [ 0x111111FFL,   
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

    # Define the coord of each drawing area
    self.drawing_area = [ 140.0, 20.0, 420.0, 500.0]
    self.target_area  = [460.0, 20.0, 740.0, 500.0]

    # Set it to 1 to let you create new forms
    # Once set, draw your shape in the right area. Then clic on OK to display
    # the data for the form (in the console). Then copy the data in the list at the end of
    # this file in init_item_list
    # Set self.editing_mode = None to forbid drawing in the right area
    #    self.editing_mode = None
    self.editing_mode = 1
    
    print("Gcompris_redraw __init__.")
  

  def start(self):  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1 
    self.gcomprisBoard.number_of_sublevel=1
    
    gcompris.bar_set(gcompris.BAR_OK)
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
    self.draw_image_target(self.drawlist[0])

    print("Gcompris_redraw start.")

    
  def end(self):
    # Remove the root item removes all the others inside it
    gcompris.set_cursor(gcompris.CURSOR_DEFAULT);
    self.rootitem.destroy()


  def ok(self):
    print("Gcompris_redraw ok.")
    if self.editing_mode != None :
      print("Copy the following data in init_item_list")
      print self.current_drawing
          

  def repeat(self):
    print("Gcompris_redraw repeat.")
            

  def config(self):
    print("Gcompris_redraw config.")
              
  def key_press(self, keyval):
    print("got key %i" % keyval)
    return


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
    # In editing mode, we allow the creation of items in the right area
    if(self.editing_mode != None):
      item.connect("event", self.create_item_event)

    self.draw_grid(x1,x2,y1,y2,step)

  # Draw the image target
  def draw_image_target(self, drawing):
      
    print "---"
    print drawing
    print "---"
    
    for i in drawing:
      print i
      item = self.rootitem.add ( i['type'] )
      for k, v in i.items():
        print k
        print v
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
            item.set ( points = v)
        elif k == 'width_units' :
            item.set ( width_units = v )
        elif k == 'width_pixels' :
            item.set ( width_pixels = v )
        elif k == 'width' :
            item.set ( width = v )
        elif k == 'width_set' :
            item.set ( width_set = v )
        elif k == 'x' :
            item.set ( x = v)
        elif k == 'y' :
            item.set ( y = v)
        elif k == 'x1' :
            item.set ( x1 = v)
        elif k == 'y1' :
            item.set ( y1 = v)
        elif k == 'x2' :
            item.set ( x2 = v)
        elif k == 'y2' :
              item.set ( y2 = v)

  def draw_grid(self, x1, x2, y1, y2, step):

    for i in range(x1,x2,step):
      item = self.rootitem.add (
        gnome.canvas.CanvasLine,
        points=(i , y1, i , y2),
        fill_color_rgba=0x3740E3FFL,
        width_units=1.0,
        )
      # In editing mode, we allow the creation of items in the right area
      if(x1<self.target_area[0] or self.editing_mode != None):
        item.connect("event", self.create_item_event)

    for i in range(y1,y2,step):
      item = self.rootitem.add (
        gnome.canvas.CanvasLine,
        points=(x1 , i, x2 , i),
        fill_color_rgba=0x3740E3FFL,
        width_units=1.0,
        )
      # In editing mode, we allow the creation of items in the right area
      if(x1<self.target_area[0] or self.editing_mode != None):
        item.connect("event", self.create_item_event)

  # Given x,y return a new x,y snapped to the grid
  def snap_to_grid(self, x, y):
    result = []
    tmp = int(((x+(self.current_step/2)) -
               self.drawing_area[0])/self.current_step)
    result.append(float(self.drawing_area[0] + tmp*self.current_step))

    tmp = int(((y+(self.current_step/2)) -
               self.drawing_area[2])/self.current_step)
    result.append(float(self.drawing_area[2] + tmp*self.current_step))
    return result

  # Event when a click on an any item. Perform the move
  def move_item_event(self, item, event, drawing_item_index):

    if self.tools[self.current_tool][0] != "SELECT":
      return gtk.FALSE

    if event.state & gtk.gdk.BUTTON1_MASK:
      x=event.x
      y=event.y
      x,y = self.snap_to_grid(event.x,event.y)

      bounds = item.get_bounds()
      x -= (bounds[2]-bounds[0])/2
      y -= (bounds[3]-bounds[1])/2

      # In editing mode, we allow the creation of items in the right area and
      # We disable drawing boundaries
      if(self.editing_mode == None):
        # Check drawing boundaries
        if(x<self.drawing_area[0]):
          x=self.drawing_area[0]
        if(x>(self.drawing_area[2]-(bounds[2]-bounds[0]))):
          x=self.drawing_area[2]-(bounds[2]-bounds[0])
        if(y<self.drawing_area[1]):
          y=self.drawing_area[1]
        if(y>(self.drawing_area[3]-(bounds[3]-bounds[1]))):
          y=self.drawing_area[3]-(bounds[3]-bounds[1])

      gcompris.utils.item_absolute_move(item, x, y)
      return gtk.TRUE
          
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
          
  # Event when a click on an item happen
  def del_item_event(self, item, event, drawing_item_index):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        if self.tools[self.current_tool][0] == "DEL":
          item.destroy()
          del self.current_drawing[drawing_item_index]
          return gtk.TRUE
    return gtk.FALSE


  # Event when a click on the drawing area happen
  def create_item_event(self, item, event):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        self.newitem = None
        x,y = self.snap_to_grid(event.x,event.y)
        self.pos_x = x
        self.pos_y = y
        
        if (self.tools[self.current_tool][0] == "DEL" or
            self.tools[self.current_tool][0] == "SELECT" or
            self.tools[self.current_tool][0] == "FILL"):
          # This event is treated in del_item_event to avoid
          # operating on background item and grid
          return gtk.FALSE
        
        elif self.tools[self.current_tool][0] == "LINE":
          self.newitem = self.rootitem.add(
            gnome.canvas.CanvasLine,
            points=(self.pos_x, self.pos_y, x, y),          
            fill_color_rgba=self.colors[self.current_color],
            width_units=5.0
            )
          self.newitem.connect("event", self.fillin_item_event, len(self.current_drawing))
          self.newitem.connect("event", self.move_item_event, len(self.current_drawing))

          # Add the new item to our list
          self.current_drawing.append({'type': gnome.canvas.CanvasLine,
                                        'point':(self.pos_x, self.pos_y, x, y),
                                        'fill_color_rgba':self.colors[self.current_color],
                                        'width_units':5.0})

        elif self.tools[self.current_tool][0] == "RECT":
          self.newitem = self.rootitem.add(
            gnome.canvas.CanvasRect,
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
          self.current_drawing.append({'type': gnome.canvas.CanvasRect,
                                        'x1':self.pos_x,
                                        'y1':self.pos_y,
                                        'x2':x,
                                        'y2':y,
                                        'outline_color_rgba':self.colors[self.current_color],
                                        'width_units':5.0})
                                      
        elif self.tools[self.current_tool][0] == "FILL_RECT":
          self.newitem = self.rootitem.add(
            gnome.canvas.CanvasRect,
            x1=self.pos_x,
            y1=self.pos_y,
            x2=x,
            y2=y,
            fill_color_rgba=self.colors[self.current_color],
            )
          self.newitem.connect("event", self.fillin_item_event, len(self.current_drawing))
          self.newitem.connect("event", self.move_item_event, len(self.current_drawing))

          # Add the new item to our list
          self.current_drawing.append({'type': gnome.canvas.CanvasRect,
                                        'x1':self.pos_x,
                                        'y1':self.pos_y,
                                        'x2':x,
                                        'y2':y,
                                        'fill_color_rgba':self.colors[self.current_color]})

        elif self.tools[self.current_tool][0] == "CIRCLE":
          self.newitem = self.rootitem.add(
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
          self.current_drawing.append({'type': gnome.canvas.CanvasEllipse,
                                        'x1':self.pos_x,
                                        'y1':self.pos_y,
                                        'x2':x,
                                        'y2':y,
                                        'outline_color_rgba':self.colors[self.current_color]})

        elif self.tools[self.current_tool][0] == "FILL_CIRCLE":
          self.newitem = self.rootitem.add(
            gnome.canvas.CanvasEllipse,
            x1=self.pos_x,
            y1=self.pos_y,
            x2=x,
            y2=y,
            fill_color_rgba=self.colors[self.current_color],
            )
          self.newitem.connect("event", self.fillin_item_event, len(self.current_drawing))
          self.newitem.connect("event", self.move_item_event, len(self.current_drawing))
          
          # Add the new item to our list
          self.current_drawing.append({'type': gnome.canvas.CanvasEllipse,
                                        'x1':self.pos_x,
                                        'y1':self.pos_y,
                                        'x2':x,
                                        'y2':y,
                                        'fill_color_rgba':self.colors[self.current_color]})
        if self.newitem != 0:
          self.newitem.connect("event", self.create_item_event)
          self.newitem.connect("event", self.del_item_event, len(self.current_drawing)-1)
          
      return gtk.TRUE
    
    if event.type == gtk.gdk.MOTION_NOTIFY:
      if event.state & gtk.gdk.BUTTON1_MASK:
        x=event.x
        y=event.y
        x,y = self.snap_to_grid(event.x,event.y)
        
        # In editing mode, we allow the creation of items in the right area and
        # We disable drawing boundaries
        if(self.editing_mode == None):
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
          self.newitem.set(
            points=( self.pos_x, self.pos_y, x, y) )
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
          
          
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        return gtk.TRUE
    return gtk.FALSE


  # The list of items (data) for this game
  def init_item_list(self):
    self.drawlist = [
      [
      {'x2': 520.0, 'y2': 500.0, 'fill_color_rgba': 4042322175L, 'y1': 440.0, 'x1': 460.0, 'type': gnome.canvas.CanvasRect}, {'x2': 740.0, 'y2': 500.0, 'fill_color_rgba': 3992977663L, 'y1': 440.0, 'x1': 680.0, 'type': gnome.canvas.CanvasRect}, {'x2': 460.0, 'y2': 20.0, 'fill_color_rgba': 4294905087L, 'y1': 100.0, 'x1': 520.0, 'type': gnome.canvas.CanvasRect}, {'x2': 740.0, 'y2': 120.0, 'fill_color_rgba': 352271359L, 'y1': 40.0, 'x1': 680.0, 'type': gnome.canvas.CanvasRect}
      ],
      [
      {'width_units': 5.0,
       'fill_color_rgba': 4042322175L,
       'type': gnome.canvas.CanvasLine,
       'point': (460.0, 80.0, 460.0, 80.0)},
      {'width_units': 5.0, 'fill_color_rgba': 4042322175L, 'type': gnome.canvas.CanvasLine, 'point': (460.0, 440.0, 460.0, 440.0)}
      ],
      [
      {'type': gnome.canvas.CanvasRect,
       'x2': 460.0,
       'y2': 460.0,
       'fill_color_rgba': 286331391L,
       'y1': 460.0,
       'x1': 460.0},
      {'x2': 700.0, 'y2': 460.0,
       'fill_color_rgba': 3992977663L,
       'y1': 460.0, 'x1': 700.0, 'type': gnome.canvas.CanvasRect},
      {'x2': 700.0, 'y2': 80.0, 'fill_color_rgba': 4294905087L,
       'y1': 80.0, 'x1': 700.0, 'type': gnome.canvas.CanvasRect},
      {'x2': 500.0, 'y2': 100.0, 'fill_color_rgba': 352271359L,
       'y1': 100.0, 'x1': 500.0, 'type': gnome.canvas.CanvasRect}
      ]
      ]



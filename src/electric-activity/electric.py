#  gcompris - electric
#
# Copyright (C) 2005 Bruno Coudoin
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

import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gcompris.bonus
import gtk
import gtk.gdk
import gobject
import cairo
import os
import tempfile

# Set to True to debug
debug = False

from gcompris import gcompris_gettext as _

class Gcompris_electric:
  """Tux hide a number, you must guess it"""

  def __init__(self, gcomprisBoard):

    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.disable_im_context = True

    # Part of UI : tools buttons
    # TOOL SELECTION
    self.tools = [
      ["DEL",    "draw/tool-del.png",     "draw/tool-del_on.png",     gcompris.CURSOR_DEL],
      ["SELECT", "draw/tool-select.png",  "draw/tool-select_on.png",  gcompris.CURSOR_SELECT]
      ]

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then
    # with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    # The list of placed components
    self.components = []
    self.gnucap_timer = 0
    self.gnucap_timer_interval = 500

    self.gnucap_binary = None

  def start(self):

    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=3
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(gcompris.BAR_LEVEL)

    gcompris.bar_set_level(self.gcomprisBoard)

    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))

    self.display_game()

    #
    # Check gnucap is installed and save it's path in self.gnucap_binary
    #
    # You can provide a gnucap binary in python_plugin_dir.
    #
    for binary in (os.path.join(gcompris.PYTHON_PLUGIN_DIR, 'gnucap'),
                   "/usr/bin/gnucap",
                   "/usr/local/bin/gnucap",
                   "gnucap.exe",
		   os.path.join(gcompris.DATA_DIR, '..' , '..', '..', 'bin', 'gnucap')):
      if(os.path.exists(binary)):
        self.gnucap_binary = binary
        break

    if not self.gnucap_binary:
      gcompris.utils.dialog(_("Cannot find the 'gnucap' electric simulator.\nYou can download and install it from:\n<http://geda.seul.org/tools/gnucap/>\nTo be detected, it must be installed in\n/usr/bin/gnucap or /usr/local/bin/gnucap.\nYou can still use this activity to draw schematics without computer simulation."),
                            None)


  def end(self):

    gcompris.reset_locale()

    gcompris.set_cursor(gcompris.CURSOR_DEFAULT);

    # Remove the root item removes all the others inside it
    self.cleanup_game()

  def ok(self):
    pass

  def repeat(self):
    if debug: print("Gcompris_electric repeat.")


  def key_press(self, keyval, commit_str, preedit_str):
    # Return  True  if you did process a key
    # Return  False if you did not processed a key
    #         (gtk need to send it to next widget)
    return False

  def pause(self, pause):
    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then
    # with pause(0)
    # the game is won
    if(pause == 0 and self.gamewon):
      self.increment_level()
      self.gamewon = 0

      self.cleanup_game()
      self.display_game()

    return


  def set_level(self, level):
    self.gcomprisBoard.level=level;
    self.gcomprisBoard.sublevel=1;

    # Set the level in the control bar
    gcompris.bar_set_level(self.gcomprisBoard);

    self.cleanup_game()
    self.display_game()

  #
  # End of Initialisation
  # ---------------------
  #

  # ------------------------------------------------------------
  # ------------------------------------------------------------
  # ------------------------------------------------------------
  # ------------------------------------------------------------

  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return 1 if continue, 0 if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      gcompris.bar_set_level(self.gcomprisBoard)

      if(self.gcomprisBoard.level>self.gcomprisBoard.maxlevel):
        # the current board is finished : bail out
        gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
        return 0

    return 1

  # Cleanup the board game
  def cleanup_game(self):
    self.gamewon = False

    if self.gnucap_timer :
      gobject.source_remove(self.gnucap_timer)
      self.gnucap_timer = 0

    # remove the appended items from our tools
    for i in range(0,len(self.tools)):
      self.tools[i].pop()

    # No more component in the simulation set
    self.components = []

    # Remove the root item removes all the others inside it
    self.rootitem.remove()

  # Display the board game
  def display_game(self):

      # Create our rootitem. We put each canvas item in it so at the end we
      # only have to kill it. The canvas deletes all the items it contains
      # automaticaly.
      self.rootitem = \
          goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

      self.create_components(self.gcomprisBoard.level)

      # Display the tools
      x = 12
      y = 10
      for i in range(0,len(self.tools)):

        item = \
            goocanvas.Image(
          parent = self.rootitem,
          pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[i][1])),
          x=x,
          y=y
          )
        x += 45
        item.connect("button_press_event", self.tool_item_event, i)

        if(self.tools[i][0]=="SELECT"):
          self.select_tool = item
          self.select_tool_number = i
          # Always select the SELECT item by default
          self.current_tool = i
          self.old_tool_item = item
          self.old_tool_item.props.pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[i][2]))
          gcompris.set_cursor(self.tools[i][3]);

        # Add the item in self.tools for later use
        self.tools[i].append(item)


  # Return the textual form of the current selected tool
  # Return on of self.tools[i][0]
  def get_current_tools(self):
      return(self.tools[self.current_tool][0])


  # Event when a tool is selected
  # Perform instant action or swich the tool selection
  def tool_item_event(self, item, target, event, tool):

    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        self.assign_tool(tool)
        return True

    return False


  def assign_tool(self, newtool):
    # Deactivate old button
    item = self.tools[self.current_tool][4]
    item.set_properties(pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[self.current_tool][1])))

    # Activate new button
    self.current_tool = newtool
    item = self.tools[newtool][4]
    item.set_properties(pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin(self.tools[self.current_tool][2])))
    gcompris.set_cursor(self.tools[self.current_tool][3]);


  #
  # Depending on the given level, initialize the component selector
  #
  def create_components(self, level):

    if(level == 1):
      # A list of couple (component class, it's value)
      component_set = ((Battery, 10),
                       (Bulb, 0.11),
                       (Switch, None),
                       )
    elif(level == 2):
      # A list of couple (component class, it's value)
      component_set = ((Battery, 10),
                       (Bulb, 0.11),
                       (Rheostat, 1000),
                       (Switch, None),
                       (Switch2, None),
                       (Connection, None),
                       )
    elif(level == 3):
      # A list of couple (component class, it's value)
      component_set = ((Battery, 10),
                       (Bulb, 0.11),
                       (Rheostat, 1000),
                       (Resistor, 1000),
                       (Switch, None),
                       (Connection, None),
                       (Diode, None),
                       )

    Selector(self, component_set)




# ----------------------------------------------------------------------
# ----------------------------------------------------------------------
# ----------------------------------------------------------------------
# ----------------------------------------------------------------------

  def run_simulation(self):
    if debug: print "run_simulation %s" %(self.gnucap_binary,)
    if not self.gnucap_binary:
      return

    if debug: print "self.gnucap_timer = %d" %(self.gnucap_timer,)
    if not self.gnucap_timer:
      if debug: print "run_simulation timeout_add"
      self.gnucap_timer = gobject.timeout_add(self.gnucap_timer_interval, self.call_gnucap)

  def call_gnucap(self):
    if not self.components:
      if debug: print "call_gnucap: No component"
      self.gnucap_timer = 0
      return

    connected = False
    for component in self.components:
      if component.is_connected():
        connected = True

    if debug: print "call_gnucap create the tempfile"

    fd, filename = tempfile.mkstemp(".gnucap", "gcompris_electric", None, True)
    f = os.fdopen(fd, "w+t")

    gnucap = "Title GCompris\n"

    # Ugly hack: connect a 0 ohm (1 fempto) resistor between net 0
    # and first net found
    gnucap += "R999999999 0 "
    found = False
    for component in self.components:
      if component.is_connected():
        for node in component.get_nodes():
          if(node.get_wires()):
            gnucap += str(node.get_wires()[0].get_wire_id())
            found = True
            break
      if found: break
    gnucap += " 1f\n"

    gnucap_print = ""
    for component in self.components:
      if component.is_connected():
        thisgnucap = component.to_gnucap("")
        gnucap += thisgnucap[0]
        gnucap_print += thisgnucap[1]

    gnucap += gnucap_print
    gnucap += ".dc\n"
    gnucap += ".end\n"
    if debug: print gnucap
    f.writelines(gnucap)
    f.close()

    #
    # Run gnucap with the temporary datafile we created.
    #
    if debug: print "calling gnucap: %s -b %s" % (self.gnucap_binary, filename)
    output = os.popen("%s -b %s" % (self.gnucap_binary, filename))

    #
    # Read and analyse gnucap result
    #
    if debug: print "---------------- GNUCAP OUTPUT PARSING ---------------------"
    line = ""
    for line in output.readlines():
      if debug: print "==="
      if debug: print line
      if(line.split() == " 0."):
        break
    if debug: print "------------------------------------------------------------"

    if debug: print "===>"
    if debug: print line
    if debug: print "===>"

    # Close it to check errors
    results = output.close()
    if results:
      print('Failed to run gnugap with error ', results)
      self.gnucap_timer = 0
      return

    values = []
    if(line.split()[0] == "0."):
      if debug: print "FOUND 0."
      values = line.split()
      del values[0]

    if debug: print values
    i = 0

    # Set all component values
    for component in self.components:
      if not component.is_connected():
        done = component.set_voltage_intensity(False, 0.0, 0.0)
      else:
        # Each Component class may have several gnucap component to retrieve
        # data from
        done = False
        while not done:
          if debug: print "Processing component %d" %(i,)
          try:
            if debug: print values[i]
            if debug: print values[i+1]
            dumy = values[i]
            dumy = values[i+1]
          except:
            if debug: print "Warning: gnucap parsing mismatch"
            done = True
            continue

          try:
            volt = self.convert_gnucap_value(values[i])
            amp = self.convert_gnucap_value(values[i+1])
            done = component.set_voltage_intensity(True, volt, amp)
            if debug: print "Converted U=%sV I=%sA" %(volt, amp)
          except:
            if debug: print "Failed to convert V=%sV I=%sA" %(values[i], values[i+1])
            done = component.set_voltage_intensity(False, 0.0, 0.0)

          i += 2


    if not debug: os.remove(filename)
    self.gnucap_timer = 0

  # Convert a gnucap value back in a regular number
  # Return a float value
  # Or a ValueError exception
  def convert_gnucap_value(self, value):
    unit = 1

    if value.endswith("T"):
      unit = e12
      value = value.replace("T", "")

    if value.endswith("G"):
      unit = 1e9
      value = value.replace("G", "")

    if value.endswith("Meg"):
      unit = 1e6
      value = value.replace("Meg", "")

    if value.endswith("K"):
      unit = 1e3
      value = value.replace("K", "")

    if value.endswith("u"):
      unit = 1e-6
      value = value.replace("u", "")

    if value.endswith("n"):
      unit = 1e-9
      value = value.replace("n", "")

    if value.endswith("p"):
      unit = 1e-12
      value = value.replace("p", "")

    if value.endswith("f"):
      unit = 1e-15
      value = value.replace("f", "")

    # return absolue value
    sign = 1
    if float(value) < 0:
      sign = -1

    return (float(sign)*float(value)*unit)




# ----------------------------------------------------------------------
# ----------------------------------------------------------------------
# ----------------------------------------------------------------------
# ----------------------------------------------------------------------
# ----------------------------------------------------------------------


# A wire between 2 Nodes
class Wire:
    # BUG: Wire ID 0 is a dummy wire, we start at 1
    # 2006/02/19 chgans@gna.org: From gnucap manual chapter 3.1:
    # Node 0 is used as a reference for all calculations and is assumed
    #to have a voltage of zero. (This is the ground, earth or common
    #node.) Nodes must be nonnegative integers, but need not be
    #numbered sequentially.
    counter = 1
    connection = {}
    colors = [ 0xdfc766FFL,
               0xdf9766FFL,
               0xdf667dFFL,
               0xdf66bcFFL,
               0xc466dfFFL,
               0x9c66dfFFL,
               0xA4FFB3FFL,
               0x6666dfFFL,
               0x669fdfFFL,
               0x66df6cFFL,
               0x66dfd8FFL ]

    def __init__(self, electric, source_node, x1, y1, x2, y2):
      self.electric = electric
      self.rootitem = electric.rootitem
      self.source_node = source_node
      self.target_node = None
      self.x1 = x1
      self.y1 = y1
      self.x2 = x2
      self.y2 = y2
      self.wire_item = \
          goocanvas.Polyline(
        parent = self.rootitem,
        points = goocanvas.Points([(self.x1, self.y1),
                                   (self.x2, self.y2)]),
        stroke_color_rgba = 0xFF0000FFL,
        line_cap = cairo.LINE_CAP_ROUND,
        line_width=5.0
        )
      self.wire_item.connect("button_press_event", self.delete_wire, self)
      self.wire_id = -1
      self._add_connection(source_node)

    def _add_connection(self, node):
      # Is this node already connected
      if(node.get_wires()):
        wire_id = node.get_wires()[0].get_wire_id()
        if debug: print "Node already connected to %d (self.wire_id=%d)" %(wire_id, self.wire_id)
        if self.wire_id >= 0:
          # This node was already connected elsewhere, reset it to use our
          # wire_id
          for wire in node.get_wires():
            if debug: print "   Was connected to %d" %(wire.get_wire_id(),)
            if wire.get_wire_id() != self.wire_id:
              wire.set_wire_id(self.wire_id)
        else:
          if wire_id == -1:
            self.wire_id = Wire.counter
            Wire.counter += 1
          else:
            self.wire_id = wire_id

      else:
        if self.wire_id == -1:
          self.wire_id = Wire.counter
          Wire.counter += 1

        Wire.connection[node] = self

      if debug: print "WIRE_ID = %d" %self.wire_id

    def set_wire_id(self, id):
      self.wire_id = id
      if(self.target_node):
        self.target_node.renumber_wire(self, id)
      if(self.source_node):
        self.source_node.renumber_wire(self, id)

      # Colorize the wire
      self.wire_item.set_properties(fill_color_rgba = Wire.colors[id % len(Wire.colors)])

    def get_wire_id(self):
      return self.wire_id

    def remove(self):
      self.wire_item.remove()
      self.wire_id = -1
      self.source_node.remove_wire(self, None)
      if self.target_node:
        self.target_node.remove_wire(self, Wire.counter)
      Wire.counter += 1
      self.source_node = None
      self.target_node = None
      self.electric.run_simulation()


    def set_target_node(self, node):
      self.target_node = node
      self._add_connection(node)

      # Colorize the wire
      self.wire_item.set_properties(fill_color_rgba = Wire.colors[self.wire_id % len(Wire.colors)])

    # Move wire. In fact, the attached component are moved and THEY
    # move the wire
    def move_all_wire(self, x, y):
      if(self.source_node and self.target_node):
        source_component = self.source_node.get_component()
        target_component = self.target_node.get_component()
        # TBD Need to move the components but not loosing their distance.
        #     Don't know yet how

    # Move one node of the wire at a time, depending on the given node
    def move(self, node, x, y):
      if(node == self.source_node):
        self.move_source_node(x, y)
      elif(node== self.target_node):
        self.move_target_node(x, y)

    def move_source_node(self, x1, y1):
      self.x1 = x1
      self.y1 = y1
      self.wire_item.set_properties(points = goocanvas.Points([(self.x1, self.y1),
                                                               (self.x2, self.y2)])
                                    )

    def move_target_node(self, x2, y2):
      self.x2 = x2
      self.y2 = y2
      self.wire_item.set_properties(points = goocanvas.Points([(self.x1, self.y1),
                                                               (self.x2, self.y2)])
                                    )


    # Callback event to delete a wire
    def delete_wire(self, widget, target, event, wire):

      if event.type == gtk.gdk.BUTTON_PRESS:
        if event.button == 1:
          wire.remove()
        elif event.button == 3:
          if debug: print "WIRE_ID = %d" %self.wire_id

      return False


#------------------------------------------------------------
# Node
#
# x, y are node coordinate relative to the component image
class Node:
    def __init__(self, image, name, x, y):
      self.image = image
      self.name  = name
      self.x     = x
      self.y     = y
      self.wires = []
      self.item  = None

    # Return the newly create Node item
    def create(self, component, x, y):
      self.component = component
      self.rootitem = component.get_rootitem()
      pixmap = gcompris.utils.load_pixmap(self.image)
      self.center_x =  pixmap.get_width()/2
      self.center_y =  pixmap.get_height()/2

      self.item = \
        goocanvas.Image(
          parent = self.rootitem,
        pixbuf = pixmap,
        x = x + self.center_x,
        y = y + self.center_y,
        )
      self.item.set_data('node', self)
      return self.item


    def get_component(self):
      return self.component

    # Add a wire to this node
    def add_wire(self, wire):
      self.wires.append(wire)

    # Remove a wire from this node and reasign all wires
    # already connected to this node a new given wire number
    def remove_wire(self, wire, wire_id):
      try:
        self.wires.remove(wire)
        if(wire_id):
          self.renumber_wire(wire, wire_id)
      except:
        pass


    # Renumber the wires
    def renumber_wire(self, wire, wire_id):
      for wire in self.wires:
        if(wire.get_wire_id() != wire_id):
          wire.set_wire_id(wire_id)

    # Return the list of all wires on this node
    def get_wires(self):
      return(self.wires)

    # Has wire, return True if the given wire is already connected
    # to this node
    def has_wire(self, wire):
      try:
        if(self.wires.index(wire)):
          return True
        else:
          return False
      except:
        pass


    def move(self, x, y):
      if(self.item):
        self.item.set_properties(x = x + self.x,
                      y = y + self.y)

      for wire in self.wires:
        wire.move(self,
                  x + self.x + self.center_x,
                  y + self.y + self.center_y)


#------------------------------------------------------------
# Component
#
# nodes is a list of class Node object
class Component(object):
    counter = 0
    def __init__(self, electric,
                 gnucap_name, gnucap_value,
                 image, nodes):
      self.electric = electric
      self.canvas = electric.gcomprisBoard.canvas
      self.rootitem = electric.rootitem

      if gnucap_name:
        self.gnucap_name = gnucap_name + str(Component.counter)
        self.electric.components.append(self)
      else:
        self.gnucap_name = ""

      Component.counter += 1
      self.gnucap_value = gnucap_value
      self.image = image
      self.nodes = nodes
      # Create a group for this component
      self.comp_rootitem = \
        goocanvas.Group(
          parent = self.rootitem,
        )
      self.comp_rootitem.props.visibility = goocanvas.ITEM_INVISIBLE

      # Add the component image
      pixmap = gcompris.utils.load_pixmap(self.image)
      self.x = 0
      self.y = 0
      self.center_x =  pixmap.get_width()/2
      self.center_y =  pixmap.get_height()/2

      self.component_item_offset_x = 0
      self.component_item_offset_y = 0
      self.component_item = \
        goocanvas.Image(
          parent = self.comp_rootitem,
        pixbuf = pixmap,
        x = self.x + self.component_item_offset_x,
        y = self.y + self.component_item_offset_y,
        )
      self.component_item.connect("button_press_event",
                                  self.component_move, self)
      self.component_item.connect("motion_notify_event",
                                  self.component_move, self)

      # Add each connector
      for node in self.nodes:
        item = node.create(self, node.x, node.y)
        item.connect("button_press_event", self.create_wire, node)
        item.connect("button_release_event", self.create_wire, node)
        item.connect("motion_notify_event", self.create_wire, node)

      # A text item to display textual values (volt and amp)
      self.item_values_x = 0
      self.item_values_y = 0
      self.item_values = \
        goocanvas.Text(
          parent = self.comp_rootitem,
        x = self.item_values_x,
        y = self.item_values_y,
        font = "Sans 8",
        text = "",
        fill_color = "white",
        anchor = gtk.ANCHOR_CENTER
        )
      self.item_values.connect("button_press_event", self.component_move, self)
      self.item_values.connect("motion_notify_event", self.component_move, self)

    # Return False if we need more value to complete our component
    # This is usefull in case where one Component is made of several gnucap component
    def set_voltage_intensity(self, valid_value, voltage, intensity):
      self.voltage = voltage
      self.intensity = intensity
      if(valid_value):
        self.item_values.set_properties(text="V=%.2fV\nI=%.3fA"%(voltage,intensity))
        self.item_values.props.visibility = goocanvas.ITEM_VISIBLE
      else:
        self.item_values.props.visibility = goocanvas.ITEM_INVISIBLE
      return True


    def get_rootitem(self):
      return self.comp_rootitem

    def move(self, x, y):
      self.x =  x - self.center_x
      self.y =  y - self.center_y

      self.item_values.set_properties(x =  self.item_values_x + self.x,
                           y =  self.item_values_y + self.y)

      self.component_item.set_properties(x =  self.x + self.component_item_offset_x,
                              y =  self.y + self.component_item_offset_y )

      for node in self.nodes:
        node.move( self.x,  self.y)

    def show(self):
      self.comp_rootitem.props.visibility = goocanvas.ITEM_VISIBLE

    def hide(self):
      self.comp_rootitem.props.visibility = goocanvas.ITEM_INVISIBLE

    def remove(self):
      try:
        # Remove ourself from the list of gnucap aware components
        self.electric.components.remove(self)
      except:
        pass

      for node in self.nodes:
        while node.get_wires():
          wire = node.get_wires()[0]
          node.remove_wire(wire, None)
          wire.remove()

      self.comp_rootitem.remove()

    # Return the nodes
    def get_nodes(self):
      return self.nodes

    # Return True if this component is connected and can provides a gnucap
    # description
    #
    # It assume that if a single node is not connected, the whole
    # component is not connected
    def is_connected(self):

      for node in self.nodes:
        if not node.get_wires():
          return False

      return True

    # Return the gnucap definition for this component
    # model is optional
    #
    def to_gnucap(self, model):
      gnucap = self.gnucap_name

      # No definition, it happens for connection spot
      # But in this case, it should not be called at all. it's not in the top level
      # components list.
      if gnucap == "":
        return "* Component ignored\n"

      # ignore component if there are some unconnected node.
      for node in self.nodes:
        if not node.get_wires():
          if debug: print "Component ignored"
          return "* Component ignored\n"

      for node in self.nodes:
        gnucap += " "
        if(node.get_wires()):
          gnucap += str(node.get_wires()[0].get_wire_id())

      gnucap += " "
      gnucap += str(self.gnucap_value)
      gnucap += "\n"
      gnucap += model

      return [gnucap, ".print dc + v(%s) i(%s)\n" %(self.gnucap_name, self.gnucap_name)]

    # Callback event to move the component
    def component_move(self, widget, target, event, component):

      if event.type == gtk.gdk.MOTION_NOTIFY:
        if event.state & gtk.gdk.BUTTON1_MASK:
          if(self.electric.get_current_tools()=="SELECT"):
            component.move(event.x, event.y)

      else:
        if(self.electric.get_current_tools()=="DEL"):
          self.remove()

      return True

    # Callback event to create a wire
    def create_wire(self, widget, target, event, node):

      if(self.electric.get_current_tools()=="DEL"):
        return True

      if event.type == gtk.gdk.BUTTON_PRESS:
        if event.button == 1:
          bounds = widget.get_bounds()
          self.pos_x = (bounds.x1+bounds.x2)/2
          self.pos_y = (bounds.y1+bounds.y2)/2
          self.wire = Wire(self.electric, node,
                           self.pos_x, self.pos_y, event.x, event.y)
          node.add_wire(self.wire)
          return True

      if event.type == gtk.gdk.MOTION_NOTIFY:
        if event.state & gtk.gdk.BUTTON1_MASK:
          self.wire.move_target_node(event.x, event.y)

      if event.type == gtk.gdk.BUTTON_RELEASE:
        if event.button == 1:
          node_target = None
          # Our wire line confuses the get_item_at. Look arround.
          for x in range(-10, 11, 5):
            target_item = self.canvas.get_item_at(event.x + x, event.y, True)
            if target_item:
              node_target = target_item.get_data('node')
              if(node_target):
                break

          # Take care not to wire the same component or 2 times the same node
          if(not node_target
             or node.get_component() == node_target.get_component()
             or node_target.has_wire(self.wire)):
            node.remove_wire(self.wire, None)
            self.wire.remove()
          else:
            self.wire.set_target_node(node_target)
            node_target.add_wire(self.wire)
            self.electric.run_simulation()

          return True

      return False

# -----------------------------------------------------------------------
# -----------------------------------------------------------------------
# -----------------------------------------------------------------------
#
# Pre defined components
# ----------------------
#

# ----------------------------------------
# RESISTOR
#
#
class Resistor(Component):
  image = "electric/resistor.png"
  icon  = "electric/resistor_icon.png"
  def __init__(self, electric,
               x, y, value):
    super(Resistor, self).__init__(electric,
                               "R",
                               value,
                               self.image,
                               [Node("electric/connect.png", "A", -30, -5),
                                Node("electric/connect.png", "B", 92, -5)])

    # Overide some values
    self.item_values_x = 50
    self.item_values_y = 12

    self.move(x, y)
    self.show()

# ----------------------------------------
# DIODE
#
#
class Diode(Component):
  image = "electric/diode.png"
  icon  = "electric/diode_icon.png"
  def __init__(self, electric,
               x, y, dummy):
    super(Diode, self).__init__(electric,
                                "D",
                                "ddd 1.",
                               self.image,
                               [Node("electric/connect.png", "A", -30, -9),
                                Node("electric/connect.png", "B", 95, -9)])

    # Overide some values
    self.item_values_x = 50
    self.item_values_y = 12

    self.move(x, y)
    self.show()


  # Return the gnucap definition for this component
  # Here we just add the diode model
  def to_gnucap(self, model):
    # Our 'ddd' Diode model
    # Idealized diode: ~0V treshold voltage. Characteristic graph
    # passes through the two points (10 mV, 10 mA) and (20 mV, 2000
    # mA) => N  = 0.072 IS = 5x10-5 A
    model = ".model  ddd  d  ( is= 50.u  rs= 0.  n= 0.072  tt= 0.  cjo= 1.p  vj= 1.  m= 0.5\n"
    model += "+ eg= 1.11  xti= 3.  kf= 0.  af= 1.  fc= 0.5  bv= 0.  ibv= 0.001 )\n"

    gnucap = []
    gnucap = super(Diode, self).to_gnucap(model)
    return gnucap

# ----------------------------------------
# SWITCH
#
#
class Switch(Component):
  image = "electric/switch_off.png"
  icon  = "electric/switch_icon.png"
  def __init__(self, electric,
               x, y, dummy):
    self.click_ofset_x = 32
    self.click_ofset_y = -28
    self.value_on  = "0"
    self.value_off = "10000k"

    super(Switch, self).__init__(electric,
                                 "R",
                                 self.value_off,
                                 self.image,
                                 [Node("electric/connect.png", "A", -30, -10),
                                  Node("electric/connect.png", "B", 100, -10)])

    # Overide some values
    self.item_values.props.visibility = goocanvas.ITEM_INVISIBLE

    self.move(x, y)

    pixmap = gcompris.utils.load_pixmap("electric/switch_click.png")
    self.click_item = goocanvas.Image(
      parent = self.comp_rootitem,
      pixbuf = pixmap,
      x = self.x + self.click_ofset_x,
      y = self.y + self.click_ofset_y,
      )
    self.click_item.connect("button_press_event", self.component_click)
    self.show()

  # Callback event on the switch
  def component_click(self, widget, target, event):
    if event.button == 1:
      if(self.gnucap_value == self.value_off):
        self.gnucap_value = self.value_on
        pixmap = gcompris.utils.load_pixmap("electric/switch_on.png")
      else:
        self.gnucap_value = self.value_off
        pixmap = gcompris.utils.load_pixmap("electric/switch_off.png")

      self.component_item.set_properties(pixbuf = pixmap)
      self.electric.run_simulation()

    return False

  # Callback event to move the component
  def component_move(self, widget, target, event, component):
     super(Switch, self).component_move(widget, target, event, component)

     if(self.electric.get_current_tools()=="DEL"):
       return True

     self.click_item.set_properties(
       x = self.x + self.click_ofset_x,
       y = self.y + self.click_ofset_y)

     return True

  # Return False if we need more value to complete our component
  # This is usefull in case where one Component is made of several gnucap component
  def set_voltage_intensity(self, valid_value, voltage, intensity):
    self.voltage = voltage
    self.intensity = intensity
    # Never show values
    self.item_values.props.visibility = goocanvas.ITEM_INVISIBLE
    return True

# ----------------------------------------
# SWITCH2
#
#
class Switch2(Component):
  image = "electric/switch2_on.png"
  icon  = "electric/switch2_icon.png"
  def __init__(self, electric,
               x, y, dummy):
    self.click_ofset_x = 22
    self.click_ofset_y = -28
    self.gnucap_current_resistor = 1
    self.gnucap_nb_resistor = 0
    self.value_on  = "0"
    self.value_off = "10000k"
    self.value_top = self.value_on
    self.value_bottom = self.value_off

    super(Switch2, self).__init__(electric,
                                   "R",
                                   self.value_off,
                                   self.image,
                                   [Node("electric/connect.png", "C", -15, 0),
                                    Node("electric/connect.png", "A", 80, -25),
                                    Node("electric/connect.png", "B", 80, 25)])

    # Overide some values
    self.item_values.props.visibility = goocanvas.ITEM_INVISIBLE

    self.move(x, y)

    pixmap = gcompris.utils.load_pixmap("electric/switch_click.png")
    self.click_item = goocanvas.Image(
      parent = self.comp_rootitem,
      pixbuf = pixmap,
      x = self.x + self.click_ofset_x,
      y = self.y + self.click_ofset_y,
      )
    self.click_item.connect("button_press_event", self.component_click)
    self.show()


  # Callback event on the switch
  def component_click(self, widget, target, event):
    if event.button == 1:
      if(self.value_top == self.value_off):
        self.value_top = self.value_on
        self.value_bottom = self.value_off
        self.component_item_offset_y = - 6
        pixmap = gcompris.utils.load_pixmap("electric/switch2_on.png")
      else:
        self.value_top = self.value_off
        self.value_bottom = self.value_on
        self.component_item_offset_y = 10
        pixmap = gcompris.utils.load_pixmap("electric/switch2_off.png")

      self.component_item.set_properties(y = self.y + self.component_item_offset_y)
      self.component_item.set_properties(pixbuf = pixmap)
      self.electric.run_simulation()

    return False

  # Callback event to move the component
  def component_move(self, widget, target, event, component):
    super(Switch2, self).component_move(widget, target, event, component)

    if(self.electric.get_current_tools()=="DEL"):
      return True

    self.click_item.set_properties(
      x = self.x + self.click_ofset_x,
      y = self.y + self.click_ofset_y)

    return True

  # Return True if this component is connected and can provides a gnucap
  # description
  #
  # The switch2 needs at least 2 connected nodes
  #
  def is_connected(self):
    count = 0
    for node in self.nodes:
      if node.get_wires():
        count += 1

    if count >= 2:
      return True

    return False

  # Return the gnucap definition for a single resitor of the switch2
  # node_id1 and node_id2 are the index in the list of nodes
  def to_gnucap_res(self, gnucap_name, node_id1, node_id2, gnucap_value):
    gnucap = gnucap_name
    gnucap += " "

    for i in (node_id1, node_id2):
      node = self.nodes[i]
      if node.get_wires():
        gnucap += str(node.get_wires()[0].get_wire_id())

      gnucap += " "

    gnucap += " "
    gnucap += str(gnucap_value)
    gnucap += "\n"

    return [gnucap, ".print dc + v(%s) i(%s)\n" %(gnucap_name, gnucap_name)]

  # Return the gnucap definition for this component
  # depending of the connected nodes, it create one or two resistor
  def to_gnucap(self, model):

    gnucap = ["", ""]

    # reset set_voltage_intensity counter
    self.gnucap_current_resistor = 0
    self.gnucap_nb_resistor = 0

    # top resistor
    if self.nodes[0].get_wires() and \
       self.nodes[1].get_wires():
      gnucap_resp = self.to_gnucap_res(self.gnucap_name + "_top", 0, 1,
                                        self.value_top)
      gnucap[0] += gnucap_resp[0]
      gnucap[1] += gnucap_resp[1]
      self.gnucap_nb_resistor += 1

    # bottom resistor
    if self.nodes[0].get_wires() and \
       self.nodes[2].get_wires():
      gnucap_resp = self.to_gnucap_res(self.gnucap_name + "_bot", 0, 2,
                                       self.value_bottom)
      gnucap[0] += gnucap_resp[0]
      gnucap[1] += gnucap_resp[1]
      self.gnucap_nb_resistor += 1

    return [gnucap[0], gnucap[1]]

  # Return False if we need more value to complete our component
  # This is usefull in case where one Component is made of several gnucap component
  def set_voltage_intensity(self, valid_value, voltage, intensity):
    # Never show values
    self.item_values.props.visibility = goocanvas.ITEM_INVISIBLE

    self.gnucap_current_resistor += 1

    if(self.gnucap_nb_resistor == self.gnucap_current_resistor):
      return True
    else:
      return False

# ----------------------------------------
# RHEOSTAT
#
#
class Rheostat(Component):
  image = "electric/resistor_track.png"
  icon  = "electric/resistor_track_icon.png"
  def __init__(self, electric,
               x, y, resitance):
    self.gnucap_current_resistor = 1
    self.gnucap_nb_resistor = 0
    self.wiper_ofset_x = -2
    self.wiper_ofset_min_y = 22
    self.wiper_ofset_max_y = 103
    self.wiper_ofset_y = self.wiper_ofset_min_y
    self.resitance = resitance
    super(Rheostat, self).__init__(electric,
                                   "R",
                                   self.resitance,
                                   self.image,
                                   [Node("electric/connect.png", "A", 0, -25),
                                    Node("electric/connect.png", "B", 50, 50),
                                    Node("electric/connect.png", "C", 0, 125)])

    # Overide some values
    self.item_values_x = 20
    self.item_values_y = 70
    self.item_values.set_properties(fill_color="blue")

    self.move(x, y)

    # The wiper wire
    self.wiper_wire_item = goocanvas.Polyline(
      parent = self.comp_rootitem,
      points = goocanvas.Points([(0,0), (0,0)]),
      fill_color_rgba = 0x5A5A5AFFL,
      line_width=5.0
      )
    self.update_wiper_wire()

    pixmap = gcompris.utils.load_pixmap("electric/resistor_wiper.png")
    self.wiper_item = goocanvas.Image(
      parent = self.comp_rootitem,
      pixbuf = pixmap,
      x = self.x + self.wiper_ofset_x,
      y = self.y + self.wiper_ofset_y,
      )
    self.wiper_item.connect("button_press_event", self.component_click)
    self.wiper_item.connect("motion_notify_event", self.component_click)

    self.show()

  def update_wiper_wire(self):
    self.wiper_wire_item.set_properties(
      points = goocanvas.Points([(self.x + self.wiper_ofset_x + 35,
                                  self.y + self.wiper_ofset_y + 10),
                                 (self.x + 55,
                                  self.y + 65)])
      )

  def move_wiper(self, new_y):
    if(new_y>self.y+self.wiper_ofset_max_y):
      self.wiper_ofset_y = self.wiper_ofset_max_y
    elif(new_y<self.y+self.wiper_ofset_min_y):
      self.wiper_ofset_y = self.wiper_ofset_min_y
    else:
      self.wiper_ofset_y = new_y - self.y

    self.wiper_item.set_properties(
      y = self.y + self.wiper_ofset_y,
      )
    self.update_wiper_wire()
    self.electric.run_simulation()

  # Fixme: can't connect "scroll-event" to this function
  def component_scroll(self, widget, target, event):
    if event.type == gtk.gdk.SCROLL:
      if event.direction == gtk.gdk.SCROLL_UP:
        self.move_wiper(self.y - 5)
      elif event.type == gtk.gdk.SCROLL_DOWN:
        self.move_wiper(self.y + 5)
    return True

  # Callback event on the wiper
  def component_click(self, widget, target, event):
    # drag and drop
    if event.type == gtk.gdk.MOTION_NOTIFY:
      if event.state & gtk.gdk.BUTTON1_MASK:
        self.move_wiper(event.y)

    return True

  # Callback event to move the component
  def component_move(self, widget, target, event, component):
     super(Rheostat, self).component_move(widget, target, event, component)

     if(self.electric.get_current_tools()=="DEL"):
       return True

     self.wiper_item.set_properties(
       x = self.x + self.wiper_ofset_x,
       y = self.y + self.wiper_ofset_y)
     self.update_wiper_wire()

     return True

  # Return True if this component is connected and can provides a gnucap
  # description
  #
  # The rheostat needs at least 2 connected nodes
  #
  def is_connected(self):
    count = 0
    for node in self.nodes:
      if node.get_wires():
        count += 1

    if count >= 2:
      return True

    return False

  # Return the gnucap definition for a single resitor of the rheostat
  # node_id1 and node_id2 are the index in the list of nodes
  def to_gnucap_res(self, gnucap_name, node_id1, node_id2, gnucap_value):
    # Ignore the component if there is some unconnected nodes.
    for i in (node_id1, node_id2):
      node = self.nodes[i]
      if not node.get_wires():
        gnucap = "* %s: component ignored: not connected\n" %(gnucap_name)
        return [gnucap, ""]

    gnucap = gnucap_name
    gnucap += " "

    for i in (node_id1, node_id2):
      node = self.nodes[i]
      if node.get_wires():
        gnucap += str(node.get_wires()[0].get_wire_id())

      gnucap += " "

    gnucap += " "
    gnucap += str(gnucap_value)
    gnucap += "\n"

    return [gnucap, ".print dc + v(%s) i(%s)\n" %(gnucap_name, gnucap_name)]

  # Return the gnucap definition for this component
  # depending of the connected nodes, it create one or two resistor
  def to_gnucap(self, model):

    gnucap = ["", ""]

    # reset set_voltage_intensity counter
    self.gnucap_current_resistor = 0

    gnucap_value = self.resitance * \
                   (self.wiper_ofset_y - self.wiper_ofset_min_y) / \
                   (self.wiper_ofset_max_y - self.wiper_ofset_min_y)

    # Main resistor
    if self.nodes[0].get_wires() and \
       not self.nodes[1].get_wires() and \
       self.nodes[2].get_wires():
      self.gnucap_nb_resistor = 1
      gnucap_resp = self.to_gnucap_res(self.gnucap_name + "_all", 0, 2,
                                       self.resitance)
      gnucap[0] += gnucap_resp[0]
      gnucap[1] += gnucap_resp[1]
      return [gnucap[0], gnucap[1]]


    self.gnucap_nb_resistor = 0

    # top resistor
    if self.nodes[0].get_wires() and \
       self.nodes[1].get_wires():
      self.gnucap_nb_resistor += 1
      gnucap_resp  = self.to_gnucap_res(self.gnucap_name + "_top", 0, 1,
                                        gnucap_value)
      gnucap[0] += gnucap_resp[0]
      gnucap[1] += gnucap_resp[1]

    # bottom resistor
    if self.nodes[1].get_wires() and \
       self.nodes[2].get_wires():
      self.gnucap_nb_resistor += 1
      gnucap_resp = self.to_gnucap_res(self.gnucap_name + "_bot", 1, 2,
                                       self.resitance - gnucap_value)
      gnucap[0] += gnucap_resp[0]
      gnucap[1] += gnucap_resp[1]

    return [gnucap[0], gnucap[1]]

  # Return False if we need more value to complete our component
  # This is usefull in case one Component is made of several gnucap component
  def set_voltage_intensity(self, valid_value, voltage, intensity):
    self.gnucap_current_resistor += 1
    if self.gnucap_current_resistor == 1:
      self.voltage = voltage
      self.intensity = intensity
      if(valid_value ):
        self.item_values.set_properties(text="U=%.1fV\nI=%.2fA"%(voltage,intensity))
        self.item_values.props.visibility = goocanvas.ITEM_VISIBLE
      else:
        self.item_values.props.visibility = goocanvas.ITEM_INVISIBLE

    if self.gnucap_nb_resistor != 1:
      self.gnucap_current_resistor += 1
      if self.gnucap_current_resistor > self.gnucap_nb_resistor:
        self.gnucap_current_resistor = 0
        return True
    else:
      self.gnucap_current_resistor = 0
      return True
    return False

# ----------------------------------------
# BULB
#
#
class Bulb(Component):
  image = "electric/bulb1.png"
  icon  = "electric/bulb_icon.png"
  def __init__(self, electric,
               x, y, power_max):
    self.internal_resistor = 1000
    super(Bulb, self).__init__(electric,
                               "R",
                               self.internal_resistor,
                               self.image,
                               [Node("electric/connect.png", "A", 7, 170),
                                Node("electric/connect.png", "B", 85, 170)])
    # Overide some values
    self.item_values_x = 55
    self.item_values_y = 80
    self.item_values.set_properties(fill_color="red")

    # Specific Bulb values
    self.is_blown = False
    self.move(x, y)
    self.show()
    self.power_max = power_max
    self.resistor_blown = 100000000

  # Change the pixmap depending on the real power in the Bulb
  # Return False if we need more value to complete our component
  # This is usefull in case where one Component is made of several gnucap component
  def set_voltage_intensity(self, valid_value, voltage, intensity):

    super(Bulb, self).set_voltage_intensity(valid_value, voltage, intensity)

    # If the Bulb is blown, do not update it anymore
    if self.is_blown:
      return True

    power = abs(voltage * intensity)
    image_index = min((power * 10) /  self.power_max + 1, 11)
    pixmap = gcompris.utils.load_pixmap("electric/bulb%d.png" %(image_index,))
    if debug: print "Power = %f (Max=%f) Image index = %d" %(power, self.power_max, image_index)
    self.component_item.set_properties(pixbuf = pixmap)

    # If the Bulb is blown, we have to change it's internal
    # Resistor value to infinite and ask for a circuit recalc
    if image_index == 11:
      self.gnucap_value = self.resistor_blown
      self.electric.run_simulation()
      self.is_blown = True

    return True

  # Callback event to move the component
  # We override it to repair the Bulb
  def component_move(self, widget, target, event, component):
    # If the Bulb is blown and we get a click repair it
    # If the bulb is not blown, you can blown it by right clicking on it
    if (event.state & gtk.gdk.BUTTON1_MASK) \
          and self.electric.get_current_tools() == "SELECT":
      if self.is_blown:
        self.is_blown = False
        self.gnucap_value = self.internal_resistor
        self.electric.run_simulation()

    elif (event.state & gtk.gdk.BUTTON3_MASK) and self.electric.get_current_tools()=="SELECT":
      if not self.is_blown:
        # Blown us with arbitrate high value
        self.set_voltage_intensity(True, 100, 10)

    return super(Bulb, self).component_move(widget, target, event, component)



# ----------------------------------------
# BATTERY
#
#
class Battery(Component):
  image = "electric/battery.png"
  icon  = "electric/battery_icon.png"
  def __init__(self, electric,
               x, y, value):
    super(Battery, self).__init__(electric,
                               "Vsupply",
                               value,
                               self.image,
                               [Node("electric/connect.png", "A", 6, -35),
                                Node("electric/connect.png", "B", 6, 120)])
    # Overide some values
    self.item_values_x = 23
    self.item_values_y = 70

    self.move(x, y)
    self.show()

  # Return False if we need more value to complete our component
  # This is usefull in case where one Component is made of several gnucap component
  def set_voltage_intensity(self, valid_value, voltage, intensity):
    super(Battery, self).set_voltage_intensity(valid_value, voltage, intensity)

    if(abs(self.intensity) > 1):
      # Short circuit case, set the dead battery icon
      self.component_item.set_properties(pixbuf = gcompris.utils.load_pixmap("electric/battery_dead.png"))
    else:
      self.component_item.set_properties(pixbuf = gcompris.utils.load_pixmap("electric/battery.png"))

    return True

# ----------------------------------------
# CONNECTION
# (a simple spot to connect wire and make a cute scematic)
#
class Connection(Component):
  image = "electric/connect_spot.png"
  icon  = "electric/connect_icon.png"
  def __init__(self, electric,
               x, y, dummy):
    super(Connection, self).__init__(electric,
                                     "",
                                     "",
                                     self.image,
                                     [Node("electric/connect.png", "A", 18, 10)])
    self.move(x, y)
    self.show()

# -----------------------------------------------------------------------
# -----------------------------------------------------------------------
# -----------------------------------------------------------------------
#
# Component selector
# ------------------
#

class Selector:
    def __init__(self, electric, components_class):
      self.electric = electric
      self.rootitem = electric.rootitem


      goocanvas.Image(
        parent = self.rootitem,
        pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("draw/tool-selector.png")),
        x=5,
        y=5.0,
        width=107.0,
        height=517.0,
        )

      self.x = 15
      self.y = 60

      index_y = 10
      gap     = 20
      self.init_coord = {}
      self.offset_x = self.offset_y = 0

      for component_class in components_class:
        pixmap = gcompris.utils.load_pixmap(component_class[0].icon)
        item = \
          goocanvas.Image(
            parent = self.rootitem,
          pixbuf = pixmap,
          x = self.x,
          y = self.y + index_y,
          )
        # Save the original coord to set it back once dropped
        self.init_coord[component_class[0]] = (self.x, self.y + index_y)
        index_y += pixmap.get_height() + gap

        item.connect("button_press_event", self.component_click,
                     component_class)
        item.connect("button_release_event", self.component_click,
                     component_class)
        item.connect("motion_notify_event", self.component_click,
                     component_class)


    # Callback event on the component
    def component_click(self, widget, target, event, component_class):

      if (event.state & gtk.gdk.BUTTON1_MASK
          and self.electric.get_current_tools()=="DEL"):
        # Switch to select mode
        self.electric.assign_tool(1)

      if event.type == gtk.gdk.MOTION_NOTIFY:
        if event.state & gtk.gdk.BUTTON1_MASK:
          # Save the click to image offset
          if self.offset_x == 0:
            bounds = widget.get_bounds()
            self.offset_x = (event.x - bounds.x1)
            self.offset_y = (event.y - bounds.y1)

          widget.set_properties(x = event.x - self.offset_x,
                                y = event.y - self.offset_y)

      if event.type == gtk.gdk.BUTTON_RELEASE:
        if event.button == 1:
          bounds = widget.get_bounds()
          component_class[0](self.electric,
                             event.x - self.offset_x + (bounds.x2-bounds.x1)/2,
                             event.y - self.offset_y + (bounds.y2-bounds.y1)/2,
                             component_class[1])

          widget.set_properties(x = self.init_coord[component_class[0]][0],
                                y = self.init_coord[component_class[0]][1])

          self.offset_x = self.offset_y = 0

        return True


    # ------------------------------------------------------------
    # ------------------------------------------------------------
    # ------------------------------------------------------------

def stop_board():
    gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)


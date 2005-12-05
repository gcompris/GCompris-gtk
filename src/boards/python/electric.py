#  gcompris - electric
# 
# Copyright (C) 2005 Bruno Coudoin
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

import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gcompris.bonus
import gtk
import gtk.gdk
import random
import pango

import os

try:
  import subprocess
except:
  print "This activity requires python 2.4."
  pass

from gcompris import gcompris_gettext as _

class Gcompris_electric:
  """Tux hide a number, you must guess it"""


  def __init__(self, gcomprisBoard):    

    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.disable_im_context = True

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    # The list of placed components
    self.components = []

  def start(self):
    
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=4
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(gcompris.BAR_OK|gcompris.BAR_LEVEL)
    
    gcompris.bar_set_level(self.gcomprisBoard)

    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))

    self.display_game()
    

  def end(self):

    gcompris.reset_locale()
    
    # Remove the root item removes all the others inside it
    self.cleanup_game()

  def ok(self):
    filename = "/tmp/gcompris_electric.gnucap.%d" %(os.getpid(),)
    f = file(filename, "w+")

    gnucap = "Title GCompris\n"
    connected_components = []
    for component in self.components:
      thisgnucap = component.to_gnucap()
      if thisgnucap != "":
        connected_components.append(component)
      gnucap += thisgnucap

    gnucap += ".dc\n"
    gnucap += ".end\n"
    print gnucap
    f.writelines(gnucap)
    f.close()
    output = subprocess.Popen(["/usr/bin/gnucap", "-b", filename ],
                              stdout=subprocess.PIPE).communicate()[0]
    print "---------------- GNUCAP OUTPUT -----------------------------"
    print output
    print "------------------------------------------------------------"
    values = output.splitlines()[11].split()
    del values[0]
    print values
    i = 0
    for component in connected_components:
      print values[i]
      print values[i+1]
      component.set_voltage_intensity(float(values[i]), float(values[i+1]))
      i += 2
      
      
    os.remove(filename)
    
    
  def repeat(self):
    print("Gcompris_electric repeat.")


  def config(self):
    print("Gcompris_electric config.")


  def key_press(self, keyval, commit_str, preedit_str):
    # Return  True  if you did process a key
    # Return  False if you did not processed a key
    #         (gtk need to send it to next widget)
    return False

  def pause(self, pause):
    self.board_paused = pause

    # Hack for widget that can't be covered by bonus and/or help
    if pause:
       self.entry.hide()
    else:
      self.entry.show()
      
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
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
    
  ###################################################
  # Configuration system
  ###################################################
  
  #mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    pass

  #
  # End of Initialisation
  # ---------------------
  #

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
  
  # Display the board game
  def cleanup_game(self):
    self.gamewon = False
      
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

  # Display the board game
  def display_game(self):

      # Create our rootitem. We put each canvas item in it so at the end we
      # only have to kill it. The canvas deletes all the items it contains automaticaly.
      self.rootitem = self.gcomprisBoard.canvas.root().add(
          gnome.canvas.CanvasGroup,
          x=0.0,
          y=0.0
          )
      self.create_components()

  def create_components(self):

    # Bulb
    a = Node("electric/connect.png", "A", -15, 150)
    b = Node("electric/connect.png", "B", 85, 150)
    bulb = Component(self.gcomprisBoard.canvas, self.rootitem,
                     "R1", "1k",
                     "electric/bulb.png", [a, b])

    bulb.move(400, 200)
    bulb.show()
    self.components.append(bulb)
    
    # Battery
    a = Node("electric/connect.png", "A", 11, -35)
    b = Node("electric/connect.png", "B", 11, 150)
    battery = Component(self.gcomprisBoard.canvas, self.rootitem,
                        "Vsupply1", "10",
                        "electric/battery.png", [a, b])

    battery.move(100, 200)
    battery.show()
    self.components.append(battery)

    # Battery
    a = Node("electric/connect.png", "A", 11, -35)
    b = Node("electric/connect.png", "B", 11, 150)
    battery2 = Component(self.gcomprisBoard.canvas, self.rootitem,
                        "Vsupply2", "10",
                        "electric/battery.png", [a, b])

    battery2.move(150, 200)
    battery2.show()
    self.components.append(battery2)

    # Resistor
    a = Node("electric/connect.png", "A", -30, -5)
    b = Node("electric/connect.png", "B", 130, -5)
    resistor = Component(self.gcomprisBoard.canvas, self.rootitem,
                         "R2", "1k",
                         "electric/resistor.png", [b, a])

    resistor.move(150, 400)
    resistor.show()
    self.components.append(resistor)




# ----------------------------------------------------------------------
# ----------------------------------------------------------------------
# ----------------------------------------------------------------------


# A wire between 2 Nodes
class Wire:
    counter = 0
    connection = {}
    
    def __init__(self, rootitem, source_node, x1, y1, x2, y2):
      self.rootitem = rootitem
      self.source_node = source_node
      self.target_node = None
      self.x1 = x1
      self.y1 = y1
      self.x2 = x2
      self.y2 = y2
      self.wire_item = self.rootitem.add(
        gnome.canvas.CanvasLine,
        points=( self.x1, self.y1, self.x2, self.y2),
        fill_color_rgba = 0xCC1111FFL,
        width_units=5.0
        )
      self.wire_item.connect("event", self.delete_wire, self)
      self.wire_id = -1
      self._add_connection(source_node)
      
    def _add_connection(self, node):
      # Is this node already connected
      if Wire.connection.has_key(node):
        wire_id = Wire.connection[node].get_wire_id()
        print "Node already connected to %d" %wire_id
        if self.wire_id >= 0:
          # This node was already connected elsewhere, reset it to use our wire_id
          for node in Wire.connection.keys():
            if Wire.connection[node].get_wire_id() == wire_id:
              Wire.connection[node].set_wire_id(self.wire_id)
        else:
          self.wire_id = wire_id
           
      else:
        if self.wire_id == -1:
          self.wire_id = Wire.counter
          Wire.counter += 1
          
        Wire.connection[node] = self
        
      print "WIRE_ID = %d" %self.wire_id

    def set_wire_id(self, id):
      self.wire_id = id
    
    def get_wire_id(self):
      return self.wire_id
    
    def destroy(self):
      self.wire_item.destroy()
      self.wire_id = -1
      self.source_node = None
      self.target_node = None
      
    def set_target_node(self, node):
      self.target_node = node
      self._add_connection(node)
      
    # Move wire. In fact, the attached component are moved and THEY move the wire
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
      self.wire_item.set( points = (self.x1, self.y1,
                                    self.x2, self.y2) )

    def move_target_node(self, x2, y2):
      self.x2 = x2
      self.y2 = y2
      self.wire_item.set( points = (self.x1, self.y1,
                                    self.x2, self.y2) )


    # Callback event to delete a wire
    def delete_wire(self, widget, event, wire):
      
      if event.type == gtk.gdk.BUTTON_PRESS:
        if event.button == 1:
          wire.destroy()

      return False



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
        
      self.item = self.rootitem.add(
        gnome.canvas.CanvasPixbuf,
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

    # Add a wire to this node
    def remove_wire(self, wire):
      self.wires.remove(wire)

    # Return the list of all wires on this node
    def get_wires(self):
      return(self.wires)

    # Has wire, return True if the given wire is already connected to this node
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
        self.item.set(x = x + self.x,
                      y = y + self.y)

      for wire in self.wires:
        wire.move(self,
                  x + self.x + self.center_x,
                  y + self.y + self.center_y)


      
# nodes is a list of class Node object
class Component:
    def __init__(self, canvas, rootitem,
                 gnucap_name, gnucap_value,
                 image, nodes):
      self.canvas = canvas
      self.rootitem = rootitem
      self.gnucap_name = gnucap_name
      self.gnucap_value = gnucap_value
      self.image = image
      self.nodes = nodes
      # Create a group for this component
      self.comp_rootitem = rootitem.add(
        gnome.canvas.CanvasGroup,
        x=0.0,
        y=0.0
        )
      self.comp_rootitem.hide()

      # Add the component image
      pixmap = gcompris.utils.load_pixmap(self.image)
      self.center_x =  pixmap.get_width()/2
      self.center_y =  pixmap.get_height()/2
      
      self.component_item = self.comp_rootitem.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = pixmap,
        x = 0,
        y = 0,
        )
      self.component_item.connect("event", self.component_move, self)

      # Add each connector
      for node in self.nodes:
        item = node.create(self, node.x, node.y)
        item.connect("event", self.create_wire, node)

      self.item_values = self.comp_rootitem.add(
        gnome.canvas.CanvasText,
        x = 0,
        y = 0,
        font=gcompris.skin.get_font("gcompris/tiny"),
        text="",
        fill_color="red",
        justification=gtk.JUSTIFY_CENTER
        )

    def set_voltage_intensity(self, voltage, intensity):
      self.voltage = voltage
      self.intensity = intensity
      self.item_values.set(text="V=%.1f\nI=%.2f"%(voltage,intensity))

      
    def get_rootitem(self):
      return self.comp_rootitem
    
    def move(self, x, y):
      x = x - self.center_x
      y = y - self.center_y
      
      self.item_values.set(x = x,
                           y = y + 50)

      self.component_item.set(x = x,
                              y = y)
      
      for node in self.nodes:
        node.move(x, y)
        
    def show(self):
      self.comp_rootitem.show()

    def hide(self):
      self.comp_rootitem.hide()

    # Return the nodes
    def get_nodes(self):
      return self.nodes

    # Return the gnucap definition for this component
    def to_gnucap(self):
      gnucap = self.gnucap_name
      node_count = 0
      
      for node in self.nodes:
        gnucap += " "
        if(node.get_wires()):
          gnucap += str(node.get_wires()[0].get_wire_id())
          node_count += 1
        
      gnucap += " "
      gnucap += str(self.gnucap_value)
      gnucap += "\n"
      gnucap += ".print dc + v(%s) i(%s)\n" %(self.gnucap_name, self.gnucap_name)

      if(node_count==2):
        return gnucap
      else:
        return ""
    
    # Callback event to move the component
    def component_move(self, widget, event, component):

      if event.type == gtk.gdk.MOTION_NOTIFY:
        if event.state & gtk.gdk.BUTTON1_MASK:
          component.move(event.x, event.y)
        
      return False

    # Callback event to create a wire
    def create_wire(self, widget, event, node):

      if event.type == gtk.gdk.BUTTON_PRESS:
        if event.button == 1:
          bounds = widget.get_bounds()
          self.pos_x = (bounds[0]+bounds[2])/2
          self.pos_y = (bounds[1]+bounds[3])/2
          self.wire = Wire(self.rootitem, node,
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
            target_item = self.canvas.get_item_at(event.x + x, event.y)
            node_target = target_item.get_data('node')
            if(node_target):
              break
            
          # Take care not to wire the same component or 2 times the same node
          if(not node_target
             or node.get_component() == node_target.get_component()
             or node_target.has_wire(self.wire)):
            node.remove_wire(self.wire)
            self.wire.destroy()
          else:
            self.wire.set_target_node(node_target)
            node_target.add_wire(self.wire)
            
          return True

      return False

    

# PythonTest Board module
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gtk
import gtk.gdk

class Gcompris_pythontest:
  """Testing gcompris python class"""
  

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.canvasitems = []
    print("Gcompris_pythontest __init__.")
  

  def start(self):  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1 
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.utils.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)
    
    self.canvasitems.append(self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasEllipse,
      x1=0.0,
      y1=0.0,
      x2=100.0,
      y2=100.0,
      fill_color='red',
      outline_color='white',
      width_units=1.0
      ))
    self.canvasitems[-1].connect("event", self.item_event)
    
    print("Gcompris_pythontest start.")

    
  def end(self):
    # Remove the canvas item we added during this plugin
    for item in self.canvasitems:
      item.destroy()
      print("Gcompris_pythontest end.")
        

  def ok(self):
    print("Gcompris_pythontest ok.")
          

  def repeat(self):
    print("Gcompris_pythontest repeat.")
            

  def config(self):
    print("Gcompris_pythontest config.")
              

  def key_press(self, keyval):  
    print("Gcompris_pythontest key press. %i" % keyval)
                

  def pause(self, pause):  
    print("Gcompris_pythontest pause. %i" % pause)
                  

  def set_level(self, level):  
    print("Gcompris_pythontest set level. %i" % level)

  def item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        widget.set(fill_color='blue')
        widget.raise_to_top()
        self.pos_x=event.x
        self.pos_y=event.y
        return gtk.TRUE
    if event.type == gtk.gdk.MOTION_NOTIFY:
      if event.state & gtk.gdk.BUTTON1_MASK:
        x = event.x
        y = event.y
        widget.move(x - self.pos_x, y - self.pos_y)
        self.pos_x = x
        self.pos_y = y
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        widget.set(fill_color='red')
        
        return gtk.TRUE
    return gtk.FALSE


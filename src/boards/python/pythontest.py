# PythonTest Board module
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
from gettext import gettext as _

class Gcompris_pythontest:
  """Testing gcompris python class"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.canvasitems = {}

    self.colors = {}
    self.colors['circle_in'] = gcompris.skin.get_color("pythontest/circle in")
    self.colors['circle_out'] = gcompris.skin.get_color("pythontest/circle out")
    self.colors['line'] = gcompris.skin.get_color("pythontest/line")

    self.movingline='none'

    print("Gcompris_pythontest __init__.")


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)

    self.canvasitems[1] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasEllipse,
      x1=300.0,
      y1=200.0,
      x2=280.0,
      y2=220.0,
      fill_color_rgba= self.colors['circle_in'],
      outline_color_rgba= self.colors['circle_out'],
      width_units=1.0
      )
    self.canvasitems[1].connect("event", self.circle_item_event)

    self.canvasitems[2] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasEllipse,
      x1=500.0,
      y1=200.0,
      x2=520.0,
      y2=220.0,
      fill_color_rgba= self.colors['circle_in'],
      outline_color_rgba= self.colors['circle_out'],
      width_units=1.0
      )
    self.canvasitems[2].connect("event", self.circle_item_event)

    self.canvasitems[3] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasText,
      x=400.0,
      y=100.0,
      text=_("This is the first plugin in gcompris coded in the Python\nProgramming language."),
      fill_color="white",
      justification=gtk.JUSTIFY_CENTER
      )

    self.canvasitems[4] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasText,
      x=400.0,
      y=140.0,
      text=_("It is now possible to develop gcompris activies in C or in Python.\n Thanks to Olivier Samys who makes this possible."),
      fill_color="white",
      justification=gtk.JUSTIFY_CENTER
      )

    self.canvasitems[5] = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasText,
      x=400.0,
      y=380.0,
      text=_("This activity is not playable yet, just a test"),
      fill_color="white",
      justification=gtk.JUSTIFY_CENTER
      )
    print("Gcompris_pythontest start.")


  def end(self):
    # Remove the canvas item we added during this plugin
    for item in self.canvasitems.values():
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

  def circle_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        bounds = widget.get_bounds()
        self.pos_x = (bounds[0]+bounds[2])/2
        self.pos_y = (bounds[1]+bounds[3])/2
        if 'line 1' in self.canvasitems:
          self.canvasitems['line 1'].destroy()
        self.canvasitems['line 1'] = self.gcomprisBoard.canvas.root().add(
          gnome.canvas.CanvasLine,
          points=( self.pos_x, self.pos_y, event.x, event.y),
          fill_color_rgba=self.colors['circle_in'],
          width_units=5.0
          )
        self.movingline='line 1'
        print "Button press"
        return gtk.TRUE
    if event.type == gtk.gdk.MOTION_NOTIFY:
      if event.state & gtk.gdk.BUTTON1_MASK:
        self.canvasitems[self.movingline].set(
          points=( self.pos_x, self.pos_y, event.x, event.y)
          )
    if event.type == gtk.gdk.BUTTON_RELEASE:
      if event.button == 1:
        self.movingline='line 1'
        print "Button release"
        return gtk.TRUE
    return gtk.FALSE


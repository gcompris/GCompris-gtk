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

    
        
    print("Gcompris_redraw __init__.")
  

  def start(self):  
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1 
    self.gcomprisBoard.number_of_sublevel=1
    
    gcompris.bar_set(0)
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

    self.drawlist = [
        {'type': gnome.canvas.CanvasLine,
         'points': (655 , 100, 655 , 300),
         'fill_color_rgba': 0x0033FFFFL,
         'width_units': 20.0},
        {'type': gnome.canvas.CanvasLine,
         'points': (95 , 0, 95 , 30),
         'fill_color_rgba': 0x0066FFFFL,
         'width_units': 10.0},
        {'type': gnome.canvas.CanvasRect,
         'x1': 10.0,
         'y1': 10.0,
         'x2': 100.0,
         'y2': 100.0,
         'fill_color_rgba': 0xFF001100L,
         'outline_color': 0xFF0011FFL,
         'width_pixels': 1.0}
        ]
    
    print "---"
    for i in self.drawlist:
        print i
        item = self.rootitem.add ( i['type'] )
        for k, v in i.items():
            print k
            print v
            if k == 'fill_color' :
                item.set ( fill_color = v )
            elif k == 'fill_color' :
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
            elif k == 'pixbuf' :
                item.set ( pixbuf = v)
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

    print("Gcompris_redraw start.")

    
  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()


  def ok(self):
    print("Gcompris_redraw ok.")
          

  def repeat(self):
    print("Gcompris_redraw repeat.")
            

  def config(self):
    print("Gcompris_redraw config.")
              
  def key_press(self, keyval):
    print("got key %i" % keyval)
    return


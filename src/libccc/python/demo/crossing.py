

try:
    import cccanvas
except:
    import sys
    sys.path.insert(0,"./../.libs/")
    import cccanvas

import page
import gtk
from math import pi
from math import sin
from math import cos 


class Crossing(page.Page):
    
    def __init__(self):
        self.title = 'Crossing'
        
        self.colors = [ cccanvas.ColorRgb(0.0,0.0,0.0,1.0),
                        cccanvas.ColorRgb(1.0,0.0,0.0,1.0) ]

        # main widget
        self.widget = gtk.VBox(False, 6)
        self.widget.set_border_width(6)

        # Scroll
        #swin = gtk.ScrolledWindow()
        #self.widget.pack_start(swin)

        # Create CCCanvas item and widget with this item as root
        self.root = cccanvas.Item() 
        self.view = cccanvas.view_widget_new_root(self.root)
        self.view.show()

        self.widget.pack_start(self.view)

        # Label
        self.label = gtk.Label('Move the mouse over a circle')
        self.label.show()
        self.widget.pack_start(self.label)

        n_circles = 12
        center = 130.0
        radius = 125.0

        r = [ 25.5,
              15.0,
              9.0,
              5.4,
              3.24 ]
        r.append( r[4]*0.6 )
        r.append( r[5]*0.6 )
        r.append( r[6]*0.6 )
 
        
        for i in range(n_circles):
            out = 0
            for r_el in r:
                circle = cccanvas.Circle()
                out += r_el
                circle.set_anchor( center + (radius - out) * sin(2.0*pi*i/n_circles),
                                   center - (radius - out) * cos(2.0*pi*i/n_circles))
                circle.set_radius( r_el );
                circle.set_brush_border (self.colors[0])
                
                circle.connect("enter-notify-event", self.enter_callback)
                circle.connect("leave-notify-event", self.leave_callback)

                self.root.append(circle)
                
        self.widget.show()

        
    def enter_callback(self, shape, view, event):
        shape.set_brush_border (self.colors[1])
        return False
        
    def leave_callback(self, shape, view, event):
        shape.set_brush_border (self.colors[0])
        return False
        

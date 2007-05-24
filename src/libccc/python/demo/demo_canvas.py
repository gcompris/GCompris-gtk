

try:
    import cccanvas as cc
except:
    import sys
    sys.path.insert(0,"./../.libs/")
    import cccanvas as cc

from math import pi as PI
from math import sin
from math import cos 
import gtk
class Canvas(cc.Item):
    def __init__(self):
        cc.Item.__init__(self)

        demo_canvas_center = 200.0

        center = demo_canvas_center
        radius = 125.0

        color_border = cc.color_new_rgb(0.0, 0.0, 0.0)
        brush_border = cc.BrushColor(color_border)
            
        for i in range(12):
            item = cc.Rectangle()
            item.set_grid_aligned(True)

            item.set_brush_border(brush_border)
            
            color = cc.color_new_hsva(1.0 * i / 12, 0.75, 1.0, 0.5)
            brush = cc.BrushColor(color)
            item.set_brush_content(brush)
            
            self.append(item)

            item.set_position(center - 50.0 + sin(i*PI/6) * radius,
                              center - 50.0 + cos(i*PI/6) * radius,
                              100.0, 100.0)

        item = cc.Text("libccc 0.0.2");
        item.set_anchor(center, center)
        item.set_anchor_type(gtk.ANCHOR_CENTER)
        self.append(item)


#!/usr/bin/env python

# libccc demo.py code get from pygtk helloworld.py

try:
    import cccanvas
except:
    import sys
    sys.path.insert(0,"./../.libs/")
    import cccanvas

import page

class Line(page.Page):

    def update_bounds(self, line, view, bounds, rect):
        if bounds==None :
            return

        rect.set_position(bounds.x1,
                          bounds.y1,
                          bounds.x2 - bounds.x1,
                          bounds.y2 - bounds.y1)

    def enter_callback(self, shape, view, event):
        shape.set_brush_border (self.brush_color1)
        return False
        
    def leave_callback(self, shape, view, event):
        shape.set_brush_border (self.brush_color2)
        return False

    def __init__(self, change_colors):
        page.Page.__init__(self)
        self.title = 'Line'

        # Simple line
        self.line = cccanvas.Line()
        self.line.move(5.0,5.0)
        self.line.line(95.0, 95.0)


        #circle
        self.circle = cccanvas.Circle()
        self.circle.set_anchor(50.0, 50.0)
        self.circle.set_radius(20)
        self.brush_green = cccanvas.BrushColor(cccanvas.color_new_rgb(0.0,1.0,0.0))
        self.circle.set_brush_border(cccanvas.BrushColor(cccanvas.color_new_rgb(0.0,1.0,0.0)))

        # colors for the line and the rect
        self.colors = [ cccanvas.color_new_rgb(1.0,0.0,0.0),
                        cccanvas.color_new_rgb(0.0,0.0,1.0) ]

        if change_colors:
            color_line = 0
            color_rect = 1
        else:
            color_rect = 0
            color_line = 1

        # brush for the line from that color
        self.brush_color1= cccanvas.BrushColor(self.colors[color_line])

        # apply the brush
        self.line.set_brush_border(self.brush_color1);

        # Simple Rectangle
        self.rect = cccanvas.Rectangle()

        # event for set the rect position when line is changed
        self.line.connect("all-bounds-changed", self.update_bounds, self.rect)
        
        # brush for the rect from that color
        self.brush_color2= cccanvas.BrushColor(self.colors[color_rect])

        # apply the brush
        self.rect.set_brush_border(self.brush_color2);

        self.circle.connect("enter-notify-event", self.enter_callback)
        self.circle.connect("leave-notify-event", self.leave_callback)

        # add line and rect to the item
        self.cc_item.append(self.line)
        self.cc_item.append(self.rect)
        self.cc_item.append(self.circle)


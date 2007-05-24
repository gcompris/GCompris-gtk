
try:
    import cccanvas
except:
    import sys
    sys.path.insert(0,"./../.libs/")
    import cccanvas

import gtk

class FifteenItem(cccanvas.Rectangle):
    def __init__(self, index):
        cccanvas.Rectangle.__init__(self)

        color = cccanvas.ColorHsv(1.0 * index / 15, 0.75, 1.0, 0.5)

        self.set_brush_content(color)

        self.index = index
        self.text = cccanvas.Text('%d' % index)
        self.text.set_anchor_type(gtk.ANCHOR_CENTER) 
        
        self.append(self.text)

    def move(self, position, edge):
        self.set_position( (position%4)*edge, (position/4)*edge, edge, edge)
        self.text.set_anchor((position%4 + 0.5)*edge, (position/4 + 0.5)*edge)
        
    
    def set_font(self, desc):
        self.text.set_font_description(desc)

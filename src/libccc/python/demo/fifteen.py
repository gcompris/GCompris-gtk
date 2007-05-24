

try:
    import cccanvas
except:
    import sys
    sys.path.insert(0,"./../.libs/")
    import cccanvas

import page
import gtk
import fifteen_grid

class Fifteen(page.Page):
    def scramble(self, button):
        self.canvas.scramble()
        
    def __init__(self):
        self.title = 'Fifteen'

        # main widget
        self.widget = gtk.VBox(False, 6)
        self.widget.set_border_width(6)
        self.widget.show()
        
        # Create CCCanvas item and widget with this item as root
        self.view = cccanvas.ViewWidget() 
        self.canvas = fifteen_grid.FifteenGrid(self.view.get_style().font_desc)
        self.view.set_root(self.canvas)
        self.view.show()

        self.widget.pack_start(self.view)

        button = gtk.Button("_Scramble")
        button.connect("clicked", self.scramble)
        button.show()
        
        self.widget.pack_start(button)


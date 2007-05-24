

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
import herzi_combo
import demo_canvas

class Zoom(page.Page):

    def spinbutton_update_zoom(self, spin):
        self.view.set_zoom(spin.get_value())
    
    def update_center_view(self, toggle):
        self.view.set_center_view(toggle.get_active())

    def update_zoom_mode(self, combo):
        self.view.set_property("zoom-mode", combo.get_value() )

    def update_scrolled_region(self,toggle):
        drect = cccanvas.DRect(100.0, 100.0, 300.0, 300.0)

        if toggle.get_active():
            self.view.set_scrolled_region(drect);
        else:
            self.view.set_scrolled_region(None);
    
    def __init__(self):
        self.title = 'Zoom and scrool'

        # main widget
        self.widget = gtk.Table(2,3,False)
        self.widget.set_border_width(6)

        # Create CCCanvas item and widget with this item as root
        self.root = demo_canvas.Canvas() 
        self.view = cccanvas.view_widget_new_root(self.root)
        self.view.show()

        # Zoom label and Spin
        widget_two = gtk.Label()
        
        widget_two.set_markup_with_mnemonic("<span weight='bold'>_Zoom</span>")
        widget_two.set_alignment(0.0, 0.5)
        widget_two.set_padding(6, 0);
        self.widget.attach(widget_two,
                      0,1,
                      0,1,
                      gtk.EXPAND | gtk.FILL,
                      gtk.FILL,
                      0,
                      0)

        widget_one = gtk.SpinButton(gtk.Adjustment(value=0,
                                                   lower=0.1,
                                                   upper=10000.0,
                                                   step_incr=0.1,
                                                   page_incr=0.0,
                                                   page_size=10.0),0.0,1)
        widget_one.connect("value-changed",self.spinbutton_update_zoom)
        widget_two.set_mnemonic_widget(widget_one)
        widget_one.set_value(1.0)
        self.widget.attach(widget_one,
                           1,2, 0,1,
                           gtk.FILL, gtk.FILL,
                           0,0);
        widget_one.show()
        widget_two.show()


        # ZoomMode label and combo
        widget_one = gtk.Label()
        widget_one.set_markup_with_mnemonic("<span weight='bold'>Zoom-_Mode</span>")
        widget_one.set_alignment(0.0, 0.5)
        widget_one.set_padding(6, 0);
        self.widget.attach(widget_one,
                      0,1, 1,2,
                      gtk.EXPAND | gtk.FILL,
                      gtk.FILL,
                      0,
                      0)
        widget_one.show()

        widget_one = herzi_combo.HerziCombo(cccanvas.ZOOM_AUTO)
        self.widget.attach(widget_one,
                           1,2, 1,2,
                           gtk.EXPAND | gtk.FILL,
                           gtk.FILL,
                           0,0)
        widget_one.connect("changed",self.update_zoom_mode)
        self.update_zoom_mode(widget_one)
        widget_one.show()

        # CenterView checkbutton
        widget_one = gtk.CheckButton("_Center View")
        self.update_center_view(widget_one)
        widget_one.connect("toggled",self.update_center_view)
        self.widget.attach(widget_one,
                           0,2, 2,3,
                           gtk.EXPAND | gtk.FILL,
                           gtk.FILL,
                           0,0)
        widget_one.show()

        # Scrolled region checkbutton
        widget_one = gtk.CheckButton("_Set Scrolled region to (100,100)=>(300,300)")
        self.update_scrolled_region(widget_one)
        widget_one.connect("toggled",self.update_scrolled_region)
        self.widget.attach(widget_one,
			 0,2, 3,4,
			 gtk.EXPAND | gtk.FILL,
                           gtk.FILL,
			 0,0)
        
        widget_one.show()

        widget_one = gtk.ScrolledWindow()
        self.widget.attach(widget_one,
                           0,2, 4,5,
                           gtk.EXPAND | gtk.FILL, gtk.EXPAND | gtk.FILL,
                           0,0)

        widget_one.add(self.view)
        
                           
        widget_one.show()
        
        self.widget.show()
        #self.widget.pack_start(self.view)

        

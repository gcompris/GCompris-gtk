
try:
    import cccanvas as cc
except:
    import sys
    sys.path.insert(0,"./../.libs/")
    import cccanvas as cc

import page
import gtk

import herzi_combo

import herzi_properties

class TextDemo(page.Page):
  
    def update_bounds(self, line, view, bounds, rect):
        if bounds==None :
            return

        rect.set_position(bounds.x1,
                          bounds.y1,
                          bounds.x2 - bounds.x1,
                          bounds.y2 - bounds.y1)

    def canvas_center_view_toggled(self, toggle, canvas):
        canvas.set_center_view(toggle.get_active())
        if (toggle.get_active()):
            toggle.set_label("Centered")
        else:
            toggle.set_label("Not centered")

    def canvas_zoom_changed(self, spin, canvas):
        canvas.set_zoom(spin.get_value())

    def text_anchor_type_changed(self, combo, text):
        text.set_anchor_type(combo.get_value())

    def text_size_pixels_toggled(self, toggle, text):
        text.set_size_pixels(toggle.get_active())
        if (toggle.get_active()):
            toggle.set_label("Size in Pixels")
        else:
            toggle.set_label("Size in Pixels")

    def text_text_changed( self, entry, pspec, text):
        t_layout = text.get_text()
        t_entry = entry.get_text()

        if (t_layout == t_entry):
            return;

        text.set_text(t_entry)

    def update_entry_from_layout( self, text, pspec, entry):
        t_layout = text.get_text()
        t_entry = entry.get_text()

        if (t_layout == t_entry):
            return;

        entry.freeze_notify()
        entry.set_text(t_entry)
        entry.freeze_notify()
            
    def __init__(self):
        self.title = 'Text Element'
        self.widget = gtk.VBox(False, 6)

        self.root = cc.Item()
        self.text = cc.Text('')

        self.text.SET_FLAGS(cc.CAN_FOCUS)

        self.text.set_editable(True)

        self.root.append(self.text)

        self.rect = cc.Rectangle()
        self.rect.set_grid_aligned(True)

        self.root.append(self.rect)

        self.arrow = cc.Line()
        self.arrow.set_grid_aligned(True)
        self.arrow.line(5.0, 0.0)
        self.arrow.line(0.0, 5.0)
        self.arrow.line(0.0, 0.0)
        self.arrow.line(5.0, 0.0)

        self.root.append(self.arrow)
        self.arrow.set_brush_border(
            cc.ColorRgb(0.0, 0.0, 1.0, 0.5))
        self.rect.set_brush_border(
            cc.ColorRgb(1.0, 0.0, 0.0, 0.5))
        
        self.text.connect("all-bounds-changed", self.update_bounds, self.rect)

        self.canvas = cc.view_widget_new_root(self.root)
        self.canvas.set_flags(gtk.CAN_FOCUS)
        
        self.widget.pack_start(self.canvas)

        self.notebook = gtk.Notebook()
        self.widget.pack_start(self.notebook, False, False, 0)

        props = herzi_properties.HerziProperties()

        prop_group = props.add("Canvas View")

        widget = gtk.ToggleButton()
        widget.set_use_underline(True)
        widget.connect("toggled", self.canvas_center_view_toggled, self.canvas)

        self.canvas_center_view_toggled(widget, self.canvas)

        prop_group.add("_Center View", widget)        

        widget = gtk.SpinButton(climb_rate=0.01, digits=2)
        widget.set_range(0, 30)
        widget.set_increments(0.01, 0.1)
        
        widget.set_value(self.canvas.get_zoom())

        widget.connect("value-changed", self.canvas_zoom_changed, self.canvas)
        
        prop_group.add("_Zoom", widget)        

        self.notebook.append_page(props, gtk.Label("Canvas View"))

        props = herzi_properties.HerziProperties()

        prop_group = props.add("Text Element")
         
        widget = herzi_combo.HerziCombo(gtk.ANCHOR_CENTER)
        widget.connect("changed", self.text_anchor_type_changed, self.text)
        self.text_anchor_type_changed(widget, self.text)
        prop_group.add("_Anchor Type", widget)

        widget = gtk.ToggleButton()
        widget.set_use_underline(True)
        widget.connect("toggled", self.text_size_pixels_toggled, self.text)
        self.text_size_pixels_toggled(widget, self.text)
 
        prop_group.add("_Size in Pixels", widget)        

        widget = gtk.Entry()
        widget.connect("notify::text", self.text_text_changed, self.text)
        widget.set_text('libccc 0.0.2')
        
        self.text.connect("notify::text", self.update_entry_from_layout, widget)
        prop_group.add("_Text", widget)
        
        self.notebook.append_page(props, gtk.Label("Text Element"))
        
        self.widget.show_all()


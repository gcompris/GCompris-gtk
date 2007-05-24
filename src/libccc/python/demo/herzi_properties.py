import gtk
import herzi_properties_group

class HerziProperties(gtk.HBox):
    def __init__(self):
        gtk.HBox.__init__(self)
        self.groups = []
        self.set_spacing(18)
        
    def add(self, label):
        self.groups.append(herzi_properties_group.HerziPropertiesGroup(label))
        self.pack_start(self.groups[-1], False, False, 0)
    
        return self.groups[-1]


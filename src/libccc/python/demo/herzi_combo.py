
import gtk

class HerziCombo(gtk.ComboBox):

    def __init__(self, element):
        
        self._type = type (element)

        self.liststore = gtk.ListStore(str, self._type)

        gtk.ComboBox.__init__(self, self.liststore)

        for i, val in self._type.__enum_values__.iteritems():
            self.liststore.append([val.value_nick, val])
            if val == element: self.set_active(i)

            
        cell = gtk.CellRendererText()
        self.pack_start(cell, True)
        self.add_attribute(cell, 'text', 0)

        self.show()

    def get_value(self):
        return self.liststore.get_value(self.get_active_iter(),1)
        
        

import gtk

class HerziPropertiesGroup(gtk.VBox):
    def __init__(self, label):
        gtk.VBox.__init__(self)

        self.set_spacing(12)

        self.label = label
        self.label_w = gtk.Label()
        self.label_w.set_alignment(0.0, 0.5)
        self.pack_start(self.label_w, False, False, 0)

        self.alignement = gtk.Alignment(0.0, 0.5, 1.0, 1.0)
        self.alignement.set_padding( 0, 0, 6, 0)
        self.pack_start(self.alignement, False, False, 0)

        self.intern = gtk.VBox(False, 6)
        self.alignement.add(self.intern)
        
        self.groups = []
        self.groups.append(gtk.SizeGroup(gtk.SIZE_GROUP_HORIZONTAL))
        self.groups.append(gtk.SizeGroup(gtk.SIZE_GROUP_HORIZONTAL))

    def set_label(self, label):
        self.label = label
        self.label_w.set_markup("<span weight='bold'>%s</span>", self.label)
        
    def add(self, label, widget):
        label_w = gtk.Label()
        box = gtk.HBox(False, 6)

        label_w.set_markup_with_mnemonic(label)
        label_w.set_mnemonic_widget(widget)
        label_w.set_alignment(0.0, 0.5)

        self.groups[0].add_widget(label_w)
        box.pack_start(label_w)

        self.groups[1].add_widget(label_w)
        box.pack_start(widget, False, False, 0)

        self.intern.pack_start(box, False, False, 0)
        
        

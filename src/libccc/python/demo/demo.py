#! /usr/bin/env python

import gtk
import line
import crossing
import zoom
import fifteen
import text_demo

try:
    import cccanvas
except:
    import sys
    sys.path.insert(0,"./../.libs/")
    import cccanvas

class Demo:
    
    def destroy(self, widget, data=None):
        print "destroy signal occurred"
        gtk.main_quit()

    def __init__(self):
        self .ListPages = []
        
        # create a new window
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)

        self.window.connect("destroy", self.destroy)

        # Sets the border width of the window.
        self.window.set_border_width(10)

        # HPaned
        self.paned = gtk.HPaned()

        # TreeStore
        self.liststore = gtk.ListStore(str, int)
        self.liststore.set_sort_column_id(0,gtk.SORT_ASCENDING)

        # create the TreeView using treestore
        self.treeview = gtk.TreeView(self.liststore)

        # create the TreeViewColumn to display the data
        self.tvcolumn = gtk.TreeViewColumn('Feature')

        # add tvcolumn to treeview
        self.treeview.insert_column_with_attributes(0, 'Feature',gtk.CellRendererText(), text=0 )

        self.treeview.connect("cursor-changed", self.tree_selection_changed)

        self.treeview.show()

        self.paned.pack1(self.treeview)
        

        # Create a new notebook, place the position of the tabs
        self.notebook = gtk.Notebook()

        # Mask the tabs
        self.notebook.set_show_tabs(False)

        self.notebook.show()
        
        self.paned.pack2(self.notebook)

        #
        self.paned.show()

        
        self.window.add(self.paned)
        self.window.show()
        

    def main(self):
        # All PyGTK applications must have a gtk.main(). Control ends here
        # and waits for an event to occur (like a key press or mouse event).
        gtk.main()

    def add_page(self, page):
        index = self.notebook.append_page(page.widget)
        iter = self.liststore.append([page.title, index])
        
        self.treeview.get_selection().select_iter(iter)
        self.notebook.set_current_page(index)


    def tree_selection_changed(self, treeview):
        model = treeview.get_selection().get_selected()[0]
        selection = treeview.get_selection().get_selected()[1]
        path = model.get_path(selection)
        
        self.notebook.set_current_page(model[path][1])

       

        

# If the program is run directly or passed as an argument to the python
# interpreter then create a HelloWorld instance and show it
if __name__ == "__main__":
    demo = Demo()
    demo.add_page(line.Line(True))
    demo.add_page(crossing.Crossing())
    demo.add_page(zoom.Zoom())
    demo.add_page(fifteen.Fifteen())
    demo.add_page(text_demo.TextDemo())
    demo.main()


        

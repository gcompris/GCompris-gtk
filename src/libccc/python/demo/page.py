#!/usr/bin/env python

# libccc demo.py code get from pygtk helloworld.py

try:
    import cccanvas
except:
    import sys
    sys.path.insert(0,"./../.libs/")
    import cccanvas

class Page:

    def __init__(self):
        self.title = 'Empty Page'
        
        # Create CCCanvas item and widget with this item as root
        self.cc_item = cccanvas.Item() 
        self.widget = cccanvas.view_widget_new_root(self.cc_item)

        # show the widget
        self.widget.show()





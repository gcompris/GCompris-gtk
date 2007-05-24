
try:
    import cccanvas
except:
    import sys
    sys.path.insert(0,"./../.libs/")
    import cccanvas

import fifteen_item

import random
import gtk
import pango

TILE_SIZE = 50.0

class FifteenGrid(cccanvas.Rectangle):
    def __init__(self, font_desc):
        global TITLE_SIZE 
        cccanvas.Rectangle.__init__(self)
        self.elements = []
        self.font_description = font_desc

        color = cccanvas.color_new_rgb(0,0,0)
        brush = cccanvas.BrushColor(color)

        self.set_brush_border (brush)

        self.set_position (0.0, 0.0, 4*TILE_SIZE, 4*TILE_SIZE)
        
        self.set_grid_aligned(True)

        for i in range(15):
            self.elements.append(fifteen_item.FifteenItem(i+1))

            self.elements[i].set_grid_aligned(True)
            self.elements[i].set_brush_border (brush)
            self.append(self.elements[i])
            
            self.elements[i].move(i, TILE_SIZE)

            self.elements[i].connect("button-press-event", self.fg_element_clicked)
            self.elements[i].set_font(self.font_description)
            

        self.elements.append(None)

    def fg_position (self, item):
        for i in range(16):
            if self.elements[i] == item:
                return i
            
        return False

    def fg_element_clicked(self, item, view, event):
        can_move = False
        direction = None

        move_value = { gtk.DIR_UP    : -4,
                       gtk.DIR_RIGHT :  1,
                       gtk.DIR_DOWN  :  4,
                       gtk.DIR_LEFT  : -1 }

        i = self.fg_position(item)

        if ( i%4 and not self.elements[i-1]):
            can_move = True
            direction = gtk.DIR_LEFT;
        elif ((i%4 < 3) and  not self.elements[i+1]):
            can_move = True
            direction = gtk.DIR_RIGHT
        elif ((i/4 > 0) and not self.elements[i-4]):
            can_move = True
            direction = gtk.DIR_UP
        elif ((i/4 < 3) and not self.elements[i+4]):
            can_move = True
            direction = gtk.DIR_DOWN

        if (can_move):
            new_i = i + move_value[direction]
            
            self.elements[new_i] = item
            self.elements[i] = None

            item.move(new_i, TILE_SIZE)
            

        return (not can_move)
    
    def scramble(self):
        global TILE_SIZE
        elements = {};
        
        for i in range(16, 0, -1):
            picked = random.randint(0,i-1)
            elements[i-1] = self.elements[picked]
            self.elements[picked] = self.elements[i-1]

        for i in range(16):
            self.elements[i] = elements[i]
            if (elements[i]):
                elements[i].move(i, TILE_SIZE)
            
            

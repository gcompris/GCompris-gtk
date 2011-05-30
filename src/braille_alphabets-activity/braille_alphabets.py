#  gcompris - braille_alphabets.py
#
# Copyright (C) 2003, 2008 Bruno Coudoin
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, see <http://www.gnu.org/licenses/>.
#
# braille_alphabets activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import goocanvas
import pango
import gcompris.sound
import string
from gcompris import gcompris_gettext as _

braille_letters = {
    "a": (1,), "b": (1, 2), "c": (1, 4), "d": (1, 4, 5), "e": (1, 5),
    "f": (1, 2, 4), "g": (1, 2, 4, 5), "h": (1, 2, 5), "i": (2, 4),
    "j": (2, 4, 5), "k": (1, 3), "l": (1, 2, 3), "m": (1, 3, 4),
    "n": (1, 3, 4, 5), "o": (1, 3, 5), "p": (1, 2, 3, 4), "q": (1, 2, 3, 4, 5),
    "r": (1, 2, 3, 5), "s": (2, 3, 4), "t": (2, 3, 4, 5), "u": (1, 3, 6),
    "v": (1, 2, 3, 6), "w": (2, 4, 5, 6), "x": (1, 3, 4, 6), "y": (1, 3, 4, 5, 6),
    "z": (1, 3, 5, 6),
}


class Gcompris_braille_alphabets:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    print "braille_alphabets init"

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    print "braille_alphabets start"

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "mosaic_bg.svgz")

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
   
    
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())
    self.display_game()
      
  def end(self):
    print "braille_alphabets end"
    # Remove the root item removes all the others inside it
    self.rootitem.remove()
    #self.cleanup_game()
  
  def ok(self):
    print("learnbraille ok.")


  def repeat(self):
    print("learnbraille repeat.")


  def config(self):
    print("learnbraille config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_learnbraille key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    print("learnbraille pause. %i" % pause)

  def set_level(self, level):
    gcompris.sound.play_ogg("sounds/receive.wav")
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)
    print("learnbraille set level. %i" % level)
    self.increment_level()
  
 
  def increment_level(self):
    gcompris.sound.play_ogg("sounds/bleep.wav")
    self.end()
    self.start()

        
  def display_game(self):
    goocanvas.Text(parent = self.rootitem,x=390.0,y=70.0,text=_("Alphabet"),width =500,height=500,fill_color="blue",anchor = gtk.ANCHOR_CENTER,font='BOLD')

    """call braille cell"""
    self.braille_cell()     
    
    """Call lower board 1 or 2"""
    if(self.gcomprisBoard.level == 1 & self.gcomprisBoard.sublevel == 1):
        print "Enter into lower board 2"
        self.board_lower2()
    else : 
        self.board_lower1()

        
  
  def board_lower1(self):
      self.cell_width = 40
      for i, letter in enumerate(string.ascii_uppercase[:13]):
          tile = self.braille_tile(letter,self.cell_width,i)
          
          
  def braille_tile(self, letter, cell_width,i):
        
        x1 = 190
        y1 = 150
        self.letter = letter
        self.cell_width = cell_width
        self.i =  i * (cell_width + 10) + 60
        
        padding = self.cell_width * 0.21
        cell_size = self.cell_width / 2 - padding

        inner_padding = self.cell_width * 0.08
        cell_radius = cell_size - inner_padding * 2
        self.item = goocanvas.Rect(parent = self.rootitem,x=self.i,y = 350, width=40, height=60,stroke_color="blue",line_width = 2.0)
        
        
        self.item.connect("button_press_event",self.braille_char,x1,y1,letter,True,True)
        gcompris.utils.item_focus_init(self.item,None)
        goocanvas.Text(parent=self.rootitem,x=self.i+10,y=425,text=str(letter),fill_color="blue",font='BOLD')
        
        #small eclipse in lower board area
        
        self.cells = []
        for u in range(2):
            for v in range(3):
                cell = goocanvas.Ellipse(parent=self.rootitem,center_x= i * 50 + u * 20 + 70,center_y=v*15+365,radius_x=cell_radius,radius_y=cell_radius,stroke_color="blue",line_width=2.0)
             
                self.cells.append(cell)   # keep a separate track so we don't mix up with other sprites

                
        #fill these eclipses with red color
        self.fillings = braille_letters.get(self.letter.lower())
        for i in range(6):
            if(i+1) in self.fillings:
                self.cells[i].set_property("fill_color","red") 
            else : 
                self.cells[i].set_property("fill_color","#DfDfDf")
          
  def braille_char(self,event,target,item,a,b,letter,clickable,displayable):
      """Checking the booleans to evaluate if text is to displayed & if its clickable"""
     
        
      if(displayable == True):
          self.increment_level()
          self.text1 = goocanvas.Text(parent=self.rootitem,
          x = a,
          y = b,
          fill_color = "blue",
          font = "Sans 78",
          anchor = gtk.ANCHOR_CENTER,
          text = str(letter))
          
      if(clickable == True):
          self.text1.connect("button_press_event",self.display_letter,letter)
          gcompris.utils.item_focus_init(self.text1,None) 
  
  def display_letter(self,item,event,target,letter):
      print "clickable"
      self.letter = letter
      print self.letter
      self.fillings = braille_letters.get(self.letter.lower())
      for i in range(6):
            if(i+1) in self.fillings:
                self.c[i].set_property("fill_color","red") 
            else : 
                self.c[i].set_property("fill_color","#DfDfDf")
       
  def board_lower2(self):
      self.cell_width = 40
      for i,letter in enumerate(string.ascii_uppercase[13:]):
          tile = self.braille_tile(letter,self.cell_width,i)
  
  
  

  """Braille Cell function to represent the dots & braille code"""
  def braille_cell(self):
      
      self.c = [] #Braille Cell Array
      goocanvas.Text(parent = self.rootitem,x=540.0,y=70.0,text=_("Braille Cell"),fill_color="blue",font='BOLD') 
      for i in range(2):
          for j in range(3):
                  goocanvas.Text(parent = self.rootitem,text=(str(j + 1 + i * 3)),font ='Sans 30',fill_color = "blue",x = i * 140 + 510, y = j * 50 + 100)
                  cell1 = goocanvas.Ellipse(parent=self.rootitem,center_x=i * 50 +570,center_y=j*50 + 120,radius_x=20,radius_y=20, stroke_color="blue",line_width=5.0)
                  self.c.append(cell1)
                  

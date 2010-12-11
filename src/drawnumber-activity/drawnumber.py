#  gcompris - drawnumber
#
# Copyright (C) 2007, 2008 Olivier Ponchaut
# Copyright (C) 2010 Bruno Coudoin
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

import goocanvas
import gcompris
import gcompris.utils
import gcompris.bonus
import gcompris.score
import gcompris.skin
import gtk
import gtk.gdk
import gobject
import cairo

from gcompris import gcompris_gettext as _

class Gcompris_drawnumber :

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;
    self.timeout       = 0;
    self.data_activity = {};

  def start(self):
    #definition of attributs
    self.MAX=0
    self.gcomprisBoard.level=1
    self.load_activity()
    self.gcomprisBoard.maxlevel = len(self.data_activity)

    #Display and configuration of menu bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_set_level(self.gcomprisBoard)
    gcompris.bar_location(gcompris.BOARD_WIDTH - 160, -1, 0.7)

    #Start of level 1
    self.set_level()

  def end(self):
    gcompris.score.end()
    self.ROOT.remove()
    if self.timeout:
      gobject.source_remove(self.timeout)
    self.timeout = 0

  #Callable GCompris method definition, but unused for this board
  def repeat(self):
    pass

  def config(self):
    pass

  def key_press(self, keyval, commit_str, preedit_str):
    pass

  def pause(self,pause):

    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(pause == 0 and self.gamewon):
      self.next_level()
      self.gamewon = 0

    return

  def set_level(self,level=1):
    """Start of the game at level number level"""
    self.gcomprisBoard.level=level
    self.data = self.load_data( level )
    gcompris.bar_set_level(self.gcomprisBoard)

    self.set_sublevel(1)

  def set_sublevel(self,sublevel=1) :
    """Start of the game at sublevel number sublevel of level n"""
    if self.MAX!=0 :
      self.end()

    # Setting of the first background image of the sublevel
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            self.data[sublevel-1].img1)

    # Creation of canvas group use by the activity
    self.ROOT = \
      goocanvas.Group(
        parent = self.gcomprisBoard.canvas.get_root_item(),
      )

    #Initialisation of sub-elements in list
    self.POINT=[]
    self.TEXT=[]
    self.actu=0

    #Display actual sublevel and number of sublevel of this level
    self.gcomprisBoard.sublevel = sublevel
    self.gcomprisBoard.number_of_sublevel = len(self.data)
    #Display of score
    gcompris.score.start(gcompris.score.STYLE_NOTE, 10, 485,
                         self.gcomprisBoard.number_of_sublevel)
    gcompris.score.set(self.gcomprisBoard.sublevel)

    self.data[sublevel-1].dump()
    # Set point number 0 from which the draw start. This point is equal to first one.
    self.POINT.append( self.point(self.data[sublevel-1].points[0][0],
                                  self.data[sublevel-1].points[0][1]) )
    self.POINT[0].props.visibility = goocanvas.ITEM_INVISIBLE
    self.TEXT.append(None)
    self.MAX = len( self.data[sublevel-1].points )

    # Data loading from global data and display of points and numbers
    prev_text = None
    prev_point = None
    for i in range(0, self.MAX):
      point = self.point(self.data[sublevel-1].points[i][0],
                         self.data[sublevel-1].points[i][1])
      self.POINT.append( point )
      self.POINT[i+1].connect('button_press_event', self.action, i+1)

      text = self.text(i + 1,
                       self.data[sublevel-1].points[i][0],
                       self.data[sublevel-1].points[i][1])
      self.TEXT.append( text )
      self.TEXT[i+1].connect('button_press_event', self.action, i+1)

      # Setting of display level to prevent covering a point with another point which
      # cause an impossibility to select it.
      self.TEXT[i+1].lower(prev_point)
      prev_text = self.TEXT[i+1]
      self.POINT[i+1].lower(prev_text)
      prev_point = self.POINT[i+1]

    #Setting color of the first point to blue instead of green
    self.POINT[1].set_properties(fill_color_rgba=0x003DF5D0)

  def point(self, x, y, d=30):
    """Setting point from his x and y location"""
    rond = goocanvas.Ellipse(
      parent = self.ROOT,
      center_x = x,
      center_y = y,
      radius_x = d/2,
      radius_y = d/2,
      fill_color_rgba = 0x3DF500D0, # default color is green and outline in black
      stroke_color = "black",
      line_width = 1.5
      )
    rond.x,rond.y = x,y
    return rond

  def text(self, idpt, xt, yt):
    """Setting text beside the point number idpt locate as xt, yt"""
    labell = goocanvas.Text(
      parent = self.ROOT,
      x = xt,
      y = yt,
      fill_color = "black",
      font = gcompris.skin.get_font("gcompris/board/big bold"),
      anchor = gtk.ANCHOR_CENTER,
      text = str(idpt))
    return labell

  def lauch_bonus(self):
    gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.RANDOM)

  def action(self, objet, target, truc, idpt):
    """Action to do at each step during normal execution of the game"""
    if truc.type == gtk.gdk.BUTTON_PRESS :
      if idpt == (self.actu+1): #Action to execute if the selected point is the following of previous one
        xd,yd,xa,ya = self.POINT[(idpt-1)].x,self.POINT[(idpt-1)].y,self.POINT[idpt].x,self.POINT[idpt].y
        item = goocanvas.Polyline(
          parent = self.ROOT,
          points = goocanvas.Points([(xd,yd),(xa,ya)]),
          stroke_color = 'black',
          line_cap = cairo.LINE_CAP_ROUND,
          line_width = 1.5)


        if idpt == 2: # Always raise the first point
          self.POINT[self.MAX].raise_(None)
          self.TEXT[self.MAX].raise_(None)

        self.POINT[idpt].props.visibility = goocanvas.ITEM_INVISIBLE
        self.TEXT[idpt].props.visibility = goocanvas.ITEM_INVISIBLE
        if idpt == self.MAX : #Action to execute if all points have been selected in good way
          gcompris.set_background(self.ROOT,
                                  self.data[self.gcomprisBoard.sublevel-1].img2)
          self.gamewon = 1
          gcompris.bar_hide(True)
          self.timeout = gobject.timeout_add(1500, self.lauch_bonus) # The level is complete -> Bonus display

        else : #Action to execute if the selected point isn''t the last one of this level
          #Set color in blue to next point. Too easy ???
          #self.POINT[(idpt+1)].set_properties(fill_color_rgba=0x003DF5F0)
          self.actu=self.actu+1 #self.actu update to set it at actual value of selected point

  def ok(self):
    pass

  def next_level(self) :
    if self.gcomprisBoard.sublevel == self.gcomprisBoard.number_of_sublevel :
      if (self.gcomprisBoard.level+1) > self.gcomprisBoard.maxlevel :
        self.set_level((self.gcomprisBoard.level))
      else :
        self.set_level((self.gcomprisBoard.level+1))
    else :
      self.set_sublevel((self.gcomprisBoard.sublevel+1))

  # Grab the data for this activity from the file activity.txt
  def load_activity(self):
    filename = gcompris.DATA_DIR + '/drawnumber/activity.txt'
    try:
      f = open( filename, 'r')
    except:
      gcompris.utils.dialog(_("Cannot find the file '{filename}'").format(filename=filename),
                            None)
      return

    level = 1
    for line in f:
      line = line.strip()
      if line.startswith('#'): continue
      if line == 'NEXT_LEVEL':
        level += 1
        continue

      linesplit = line.split(';')
      linesplit = [line.strip() for line in linesplit]
      if len(linesplit) < 4: continue
      img1 = linesplit[0]
      img2 = linesplit[1]
      points = []
      for coord in linesplit[2:]:
        # coord is a python list [x, y]
        points.append( eval(coord) )

      if not self.data_activity.has_key(level):
         self.data_activity[level] = []
      self.data_activity[level].append( Dataset(img1, img2, points) )

    f.close()


  # Data for this level
  def load_data(self, level):
    return self.data_activity[level]


# The data for a given sublevel
class Dataset:

  def __init__(self, img1, img2, points):
    self.img1 = img1
    self.img2 = img2
    self.points = points

  def dump(self):
    print self.img1
    print self.points

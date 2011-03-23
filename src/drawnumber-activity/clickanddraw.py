#  gcompris - clickanddraw
#
#
# Copyright (C) 2007, 2008 Olivier Ponchaut
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
import gtk
import gtk.gdk
import gobject
import cairo
from drawnumber import Gcompris_drawnumber

class Gcompris_clickanddraw(Gcompris_drawnumber):

  def set_sublevel(self, sublevel=1):
    """Start of the game at sublevel number sublevel of level n"""

    if self.MAX!=0 :
      self.end()

    #Creation of canvas group use by the activity
    self.ROOT = \
    goocanvas.Group(
      parent = self.gcomprisBoard.canvas.get_root_item(),
      )

    #Setting of the first background image of the level
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            self.data[sublevel-1].img1)

    #Initialisation of sub-elements in list
    self.POINT = []
    self.actu = 0

    #Display actual sublevel and number of sublevel of this level
    self.gcomprisBoard.sublevel=sublevel
    self.gcomprisBoard.number_of_sublevel=len(self.data)

    #Display of score
    gcompris.score.start(gcompris.score.STYLE_NOTE, 10, 485,
                         self.gcomprisBoard.number_of_sublevel)
    gcompris.score.set(self.gcomprisBoard.sublevel)

    #Set point number 0 from which the draw start. This point is equal to first one.
    self.MAX = len( self.data[sublevel-1].points )
    self.POINT.append(self.point(self.data[sublevel-1].points[0][0],
                                 self.data[sublevel-1].points[0][1]))
    self.POINT[0].props.visibility = goocanvas.ITEM_INVISIBLE

    #Data loading from global data and display of points and numbers
    i=self.MAX
    prev_point = None
    for i in range(0, self.MAX):
      diameter = 0
      if self.gcomprisBoard.level == 1:
        diameter = 45
      elif self.gcomprisBoard.level == 2:
        diameter = 30
      else :
        diameter = 20

      point = self.point(self.data[sublevel-1].points[i][0],
                                   self.data[sublevel-1].points[i][1],
                                   diameter)
      self.POINT.append(point)
      self.POINT[i+1].connect('button_press_event', self.action, i+1)

      # Setting of display level to prevent covering a point with another point which
      # cause an impossibility to select it.
      self.POINT[i+1].lower(prev_point)
      prev_point = self.POINT[i+1]


    #Setting color of the first point to blue instead of green
    self.POINT[1].set_properties(fill_color_rgba=0x003DF5D0)

  def action(self, objet, target, truc, idpt):
    """Action to do at each step during normal execution of the game"""
    if truc.type == gtk.gdk.BUTTON_PRESS :
      if idpt == (self.actu+1): #Action to execute if the selected point is the following of previous one
        xd, yd, xa, ya = \
            self.POINT[(idpt-1)].x, \
            self.POINT[(idpt-1)].y, \
            self.POINT[idpt].x, \
            self.POINT[idpt].y
        goocanvas.Polyline(
          parent = self.ROOT,
          points = goocanvas.Points([(xd,yd), (xa,ya)]),
          fill_color = 'black',
          line_cap = cairo.LINE_CAP_ROUND,
          line_width = 2)

        if idpt == 2: # Always raise the first point
          self.POINT[self.MAX].raise_(None)

        objet.props.visibility = goocanvas.ITEM_INVISIBLE
        if idpt==self.MAX : #Action to exectute if all points have been selected in good way
          gcompris.set_background(self.ROOT,
                                  self.data[self.gcomprisBoard.sublevel-1].img2)
          self.gamewon = 1
          gcompris.bar_hide(True)
          self.timeout = gobject.timeout_add(1500, self.lauch_bonus) # The level is complete -> Bonus display

        else : # Action to execute if the selected point isn''t the last one of this level
          # Set color in blue to next point. Too easy ???
          self.POINT[(idpt+1)].set_properties(fill_color_rgba=0x003DF5D0)
          self.actu = self.actu+1 #self.actu update to set it at actual value of selected point

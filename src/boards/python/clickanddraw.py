#  gcompris - clickanddraw
#
# Time-stamp: <2007/03/30 20:04:45 olivier>
#
# Copyright (C) 2007 Olivier Ponchaut
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

import gnomecanvas
import gcompris
import gcompris.utils
import gcompris.bonus
import gcompris.score
import gtk
import gtk.gdk
import gobject
from drawnumber import Gcompris_drawnumber

class Gcompris_clickanddraw(Gcompris_drawnumber) :

  def set_sublevel(self,sublevel=1):
    """Start of the game at sublevel number sublevel of level n"""

    if self.MAX!=0 :
      self.end()

    #Creation of canvas group use by the activity
    self.ROOT=self.gcomprisBoard.canvas.root().add(
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    #Setting of the first background image of the level
    gcompris.set_background(self.ROOT,self.data[sublevel][0][1])

    #Initialisation of sub-elements in list
    self.POINT=[0]
    self.actu=0

    #Display actual sublevel and number of sublevel of this level
    self.gcomprisBoard.sublevel=sublevel
    self.gcomprisBoard.number_of_sublevel=self.data[0]
    #Display of score
    gcompris.score.start(gcompris.score.STYLE_NOTE, 10, 485,self.gcomprisBoard.number_of_sublevel)
    gcompris.score.set(self.gcomprisBoard.sublevel)

    #Set point number 0 from which the draw start. This point is equal to first one.
    self.MAX=self.data[sublevel][0][0]
    self.POINT[0]=self.point(0,self.data[sublevel][self.MAX][0],self.data[sublevel][self.MAX][1],30)
    self.POINT[0].hide()

    #Data loading from global data and display of points and numbers
    i=self.MAX
    while(i>0):
      if self.gcomprisBoard.level==1 :
        self.POINT.append(self.point(idpt=(self.MAX-i+1),
                                     x=self.data[sublevel][i][0],
                                     y=self.data[sublevel][i][1],d=45))
      elif self.gcomprisBoard.level==2 :
        self.POINT.append(self.point(idpt=(self.MAX-i+1),
                                     x=self.data[sublevel][i][0],
                                     y=self.data[sublevel][i][1],d=30))
      else :
        self.POINT.append(self.point(idpt=(self.MAX-i+1),
                                     x=self.data[sublevel][i][0],
                                     y=self.data[sublevel][i][1],d=20))

      self.POINT[(self.MAX-i+1)].connect('event',self.action,(self.MAX-i+1))

      #Setting of display level to prevent from covert a point with another
      #point which cause an impossibility to select it.
      self.POINT[(self.MAX-i+1)].lower(300-(self.MAX-i+1))
      i=i-1

    #Setting color of the first point to blue instead of green
    self.POINT[1].set(fill_color='blue')

  def action(self,objet,truc,idpt):
    """Action to do at each step during normal execution of the game"""
    if truc.type == gtk.gdk.BUTTON_PRESS :
      if idpt == (self.actu+1): #Action to execute if the selected point is the following of previous one
        xd,yd,xa,ya = self.POINT[(idpt-1)].x, self.POINT[(idpt-1)].y, self.POINT[idpt].x, self.POINT[idpt].y
        self.ROOT.add(gnomecanvas.CanvasLine,points=(xd,yd,xa,ya),
                      fill_color='black',
                      width_units=2)

        if idpt == 2: # Always raise the first point
          self.POINT[self.MAX].raise_to_top()

        objet.hide()
        if idpt==self.MAX : #Action to exectute if all points have been selected in good way
          gcompris.set_background(self.ROOT,
                                  self.data[self.gcomprisBoard.sublevel][0][2])
          self.gamewon = 1
          self.timeout = gobject.timeout_add(1500, self.lauch_bonus) # The level is complete -> Bonus display

        else : #Action to execute if the selected point isn't the last one of this level
          self.POINT[(idpt+1)].set(fill_color='blue') #Set color in blue to next point. Too easy ???
          self.actu = self.actu+1 #self.actu update to set it at actual value of selected point

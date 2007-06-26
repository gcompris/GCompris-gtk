#  gcompris - drawnumber
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

class Gcompris_drawnumber :

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0;
    self.gamewon       = 0;
    self.timeout       = 0;

  def start(self):
    #definition of attributs
    self.MAX=0
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=3    #self.data[0]

    #Display and configuration of menu bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_set_level(self.gcomprisBoard)

    #Start of level 1
    self.set_level()

  def end(self):
    gcompris.score.end()
    self.ROOT.destroy()
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
    self.data=self.load_data(level)
    gcompris.bar_set_level(self.gcomprisBoard)

    #Start sublevel 1 of selected level
    self.set_sublevel(1)

  def set_sublevel(self,sublevel=1) :
    """Start of the game at sublevel number sublevel of level n"""
    if self.MAX!=0 :
      self.end()

    #Creation of canvas group use by the activity
    self.ROOT=self.gcomprisBoard.canvas.root().add(
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    #Setting of the first background image of the sublevel
    gcompris.set_background(self.ROOT, self.data[sublevel][0][1])

    #Initialisation of sub-elements in list
    self.POINT=[0]
    self.TEXT=[0]
    self.actu=0

    #Display actual sublevel and number of sublevel of this level
    self.gcomprisBoard.sublevel=sublevel
    self.gcomprisBoard.number_of_sublevel=len(self.data)
    #Display of score
    gcompris.score.start(gcompris.score.STYLE_NOTE, 10, 485,self.gcomprisBoard.number_of_sublevel)
    gcompris.score.set(self.gcomprisBoard.sublevel)

    #Set point number 0 from which the draw start. This point is equal to first one.
    self.POINT[0]=self.point(0,self.data[sublevel][1][0],self.data[sublevel][1][1],30)
    self.POINT[0].hide()
    self.MAX=self.data[sublevel][0][0]

    #Data loading from global data and display of points and numbers
    i=1
    while(i<=(self.MAX)):
      self.TEXT.append(self.texte(i,self.data[sublevel][i][0],self.data[sublevel][i][1]))
      self.TEXT[i].connect('event',self.action,i)
      self.TEXT[i].lower(300-i)
      self.POINT.append(self.point(i,self.data[sublevel][i][0],self.data[sublevel][i][1],30))
      self.POINT[i].connect('event',self.action,i)

      #Setting of display level to prevent from covert a point with another point which
      #cause an impossibility to select it.
      self.POINT[i].lower(300-i)
      i=i+1

    #Setting color of the first point to blue instead of green
    self.POINT[1].set(fill_color='blue')

  def point(self,idpt,x,y,d=30):
    """Setting point from his x and y location"""
    rond=self.ROOT.add(gnomecanvas.CanvasEllipse,
      x1=(x-(d/2)),y1=(y-(d/2)),
      x2=(x+(d/2)),y2=(y+(d/2)),
      fill_color = "green", # default color is green and outline in black
      outline_color = "black",
      width_units = 1.5
      )
    rond.x,rond.y=x,y
    return rond

  def texte(self,idpt,xt,yt):
    """Setting text beside the point number idpt locate as xt, yt"""
    labell = self.ROOT.add(gnomecanvas.CanvasText,
                           x=xt,
                           y=yt,
                           fill_color="black",
                           size_points=14,
                           text=str(idpt)) # ,font=gcompris.FONT_TITLE)
    return labell

  def lauch_bonus(self):
    gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.RANDOM)

  def action(self,objet,truc,idpt):
    """Action to do at each step during normal execution of the game"""
    if truc.type == gtk.gdk.BUTTON_PRESS :
      if idpt == (self.actu+1): #Action to execute if the selected point is the following of previous one
        xd,yd,xa,ya=self.POINT[(idpt-1)].x,self.POINT[(idpt-1)].y,self.POINT[idpt].x,self.POINT[idpt].y
        item = self.ROOT.add(gnomecanvas.CanvasLine,
                             points=(xd,yd,xa,ya),
                             fill_color='black',
                             width_units=1.5)


        if idpt == 2: # Always raise the first point
          self.POINT[self.MAX].raise_to_top()
          self.TEXT[self.MAX].raise_to_top()

        self.POINT[idpt].hide()
        self.TEXT[idpt].hide()
        if idpt == self.MAX : #Action to execute if all points have been selected in good way
          gcompris.set_background(self.ROOT, self.data[self.gcomprisBoard.sublevel][0][2])
          self.gamewon = 1
          self.timeout = gobject.timeout_add(1500, self.lauch_bonus) # The level is complete -> Bonus display

        else : #Action to execute if the selected point isn't the last one of this level
          #self.POINT[(idpt+1)].set(fill_color='blue') #Set color in blue to next point. Too easy ???
          self.actu=self.actu+1 #self.actu update to set it at actual value of selected point

  def ok(self):
    pass

  def next_level(self) :
    if self.gcomprisBoard.sublevel==self.gcomprisBoard.number_of_sublevel :
      if (self.gcomprisBoard.level+1)>self.gcomprisBoard.maxlevel :
        self.end()
        gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
      else :
        self.set_level((self.gcomprisBoard.level+1))
    else :
      self.set_sublevel((self.gcomprisBoard.sublevel+1))

  def load_data(self,level): #Data of this game

    #Game data of drawnumber board
    #How to add level to this board ? ->
    #data[x] = Data of level x : [ [ nbr of points in level x,
    #                                "First background image of level x",
    #                                "Last background image of level x"
    #                              ],
    #                              [x1,y1],...,[xn,yn]
    #                            ]
    # (with xn,yn the locations x and y of point number n of the image)

    if level==1 : #Picture with 1 to 20 points
      data={}
      data[1]=[[6,"drawnumber/dn_fond1.png", "drawnumber/dn_fond1.png"],
               [407,121],[489,369],[279,216],[537,214],[330,369],[407,121]]

      data[2]=[[13,"drawnumber/dn_de1.png", "drawnumber/dn_de2.png"],
               [404,375],[406,271],[503,233],[588,278],[588,337],[611,311],[727,320],[728,424],[682,486],[560,474],[560,397],[495,423],[404,375]]

      data[3]=[[14,"drawnumber/dn_house1.png", "drawnumber/dn_house2.png"],
               [406,360],[512,360],[513,175],[456,120],[455,70],[430,70],[430,96],[372,40],[219,177],[220,361],[353,361],[352,276],[406,276],[406,360]]

      data[4]=[[20,"drawnumber/dn_sapin1.png", "drawnumber/dn_sapin2.png"],
               [244,463],[341,361],[267,373],[361,238],[300,251],[377,127],[329,146],[399,52],[464,144],[416,128],[492,251],[435,239],[527,371],[458,362],[557,466],[422,453],[422,504],[374,504],[374,453],[244,463]]

    elif level==2 : #Picture with 21 to 50 points
      data={}
      data[1]=[[28,"drawnumber/dn_epouvantail1.png", "drawnumber/dn_epouvantail2.png"],
               [255,449],[340,340],[340,224],[212,233],[208,168],[395,160],[368,135],[367,77],[340,79],[339,68],[363,65],[359,18],[460,9],[464,56],[488,54],[489,66],[463,68],[462,135],[434,161],[612,163],[607,223],[477,221],[477,337],[561,457],[507,487],[413,377],[309,491],[255,449]]

      data[2]=[[37,"drawnumber/dn_plane1.png", "drawnumber/dn_plane2.png"],
               [342,312],[163,317],[141,309],[128,285],[141,256],[165,236],[246,212],[170,127],[149,86],[165,70],[190,69],[234,92],[369,200],[500,198],[577,188],[534,161],[533,147],[545,141],[567,144],[604,163],[631,116],[649,105],[664,117],[671,197],[708,212],[718,227],[712,238],[688,238],[652,224],[576,263],[459,288],[533,334],[575,380],[573,398],[551,407],[404,343],[342,312]]

      data[3]=[[42,"drawnumber/dn_poisson1.png", "drawnumber/dn_poisson2.png"],
               [33,338],[78,238],[152,172],[320,158],[378,84],[423,70],[450,83],[463,117],[448,154],[453,164],[478,174],[526,144],[552,158],[555,168],[545,188],[557,215],[623,241],[685,222],[712,188],[739,176],[761,194],[766,274],[740,380],[721,422],[678,408],[654,362],[558,349],[488,367],[498,394],[495,427],[461,448],[414,443],[350,389],[312,387],[320,404],[315,431],[291,433],[267,400],[78,400],[55,384],[47,340],[33,338]]

    elif level==3 : #Picture with 51 to ... points
      data={}
      data[1]=[[36,"drawnumber/dn_fou1.png", "drawnumber/dn_fou2.png"],
               [285,204],[342,143],[374,180],[396,118],[415,188],[419,120],[427,179],[462,154],[514,203],[465,188],[436,224],[436,236],[434,296],[429,315],[470,306],[514,262],[534,184],[554,186],[536,279],[463,366],[454,473],[462,518],[358,518],[370,473],[352,365],[327,402],[350,468],[330,483],[298,407],[330,333],[377,317],[369,295],[364,235],[363,223],[333,189],[285,204]]

      data[2]=[[71,"drawnumber/dn_bear1.png", "drawnumber/dn_bear2.png"],
               [304,256],[262,240],[216,206],[192,174],[195,159],[216,154],[257,167],[300,188],[334,220],[352,206],[370,200],[342,185],[321,154],[319,117],[337,90],[320,76],[315,55],[335,38],[360,40],[376,52],[377,66],[400,59],[431,60],[450,66],[451,48],[468,37],[490,36],[507,48],[510,70],[501,82],[486,86],[502,110],[506,136],[499,164],[479,189],[458,200],[480,212],[522,180],[578,158],[611,154],[622,164],[618,184],[592,213],[550,236],[515,251],[529,286],[532,329],[526,364],[548,354],[566,360],[576,384],[571,417],[548,455],[520,470],[493,456],[490,419],[461,440],[414,453],[360,441],[330,422],[331,450],[317,471],[284,467],[255,434],[242,402],[248,366],[271,354],[296,364],[288,326],[292,282],[304,256]]

      data[3]=[[85,"drawnumber/dn_hibou1.png", "drawnumber/dn_hibou2.png"],
               [443,133],[423,11],[434,10],[472,56],[458,20],[467,19],[495,71],[491,30],[501,33],[522,116],[519,72],[529,75],[537,153],[539,119],[546,125],[547,159],[555,154],[556,208],[583,169],[724,98],[739,99],[703,137],[743,109],[742,121],[691,165],[742,143],[741,155],[654,200],[716,182],[713,195],[650,215],[689,212],[680,219],[615,235],[661,229],[653,238],[604,250],[622,250],[615,255],[582,260],[600,263],[590,268],[577,268],[606,300],[644,294],[643,302],[599,314],[625,311],[623,318],[593,323],[615,323],[611,327],[586,329],[599,332],[591,336],[553,335],[535,357],[534,377],[527,368],[511,367],[493,375],[501,366],[485,371],[493,363],[482,365],[492,355],[514,354],[532,332],[520,331],[505,317],[500,291],[501,303],[490,289],[486,297],[479,282],[473,287],[466,269],[448,252],[446,224],[460,201],[456,191],[459,182],[467,197],[479,196],[443,133]]

    return data

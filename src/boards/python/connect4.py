#  gcompris - connect4 
# 
# Time-stamp: 
# 
# Copyright (C) 2005 Laurent Lacheny 
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
# The IA code comes from the project 4stattack by Jeroen Vloothuis
# http://forcedattack.sourceforge.net/
#


#TODO 
#Correct bug when click out of board: done
#Reset board on level change: done
#After level 4 go back to level 1: done
#Anim coin when playing: done
#Hide tux after first round: done
#Enable click on coins: done


import gobject
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.anim
import gtk
import gtk.gdk
from gettext import gettext as _

import types
import thread
from  connect4p import rules
from  connect4p import human
from  connect4p import minmax
from  connect4p import board
# ----------------------------------------
# 

class Gcompris_connect4:
  """catch the ball"""


  def __init__(self, gcomprisBoard):
    print("Gcompris_connect4 __init__.")
    self.gcomprisBoard = gcomprisBoard

  def start(self):
    print "Gcompris_connect4_start."
    self.board_size = 490.0
    self.boarditem = None
    self.nb_columns = 7
    self.nb_lines = 6
    self.stone_size = 70.0
    self.firstPlayer = False
    self.timer_anim = 0 
    self.timer_machine = 0 
    self.machineHasPlayed = True
    self.endAnimCallback = None
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=9
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(gcompris.BAR_LEVEL)
    #gcompris.set_background(self.gcomprisBoard.canvas.root(),
    #                        "connect4/back.png")
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_reload.png"))
    if(pixmap):
      gcompris.bar_set_repeat_icon(pixmap)
      gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT_ICON)
    else:
      gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT)

    selector = self.rootitem.add(
     gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("connect4/back.png"),
      x=0.0,
      y=0.0
      )
    selector.connect("event", self.columnItemEvent, 0)

    selector = self.rootitem.add(
     gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("connect4/grid.png"),
      x=1+(gcompris.BOARD_WIDTH-self.board_size)/2,
      y=1+50.0
      )

    self.prof = self.rootitem.add(
     gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("connect4/prof.png"),
      x=10,
      y=350.0
      )
    self.prof.connect("event", self.profItemEvent, 0)
    self.prof.connect("event", gcompris.utils.item_event_focus)

    self.tuxboatanim = gcompris.anim.Animation("connect4/sablier.txt")
    self.tuxboatitem = gcompris.anim.CanvasItem( self.tuxboatanim, self.rootitem )
    self.tuxboatitem.gnome_canvas.hide()

    self.newGame()

  def end(self):
    print "Gcompris_connect4 end."
    # Remove the root item removes all the others inside it
    del self.tuxboatitem
    del self.tuxboatanim
    self.rootitem.destroy()
    self.boarditem.destroy()
    if self.timer_anim:
      gtk.timeout_remove(self.timer_anim)
    if self.timer_machine:
      gtk.timeout_remove(self.timer_machine)
    

  def ok(self):
    print("Gcompris_connect4 ok.")

  def repeat(self):
    print("Gcompris_connect4 repeat.")
    self.newGame()

  def config(self):
    print("Gcompris_connect4 config.")


  def key_press(self, keyval):
    print("Gcompris_connect4 key press. %i" % keyval)
    self.tuxboatitem.hide()
    return gtk.FALSE

  def pause(self, pause):
    print("Gcompris_connect4 pause.")

  # Called by gcompris when the user click on the level icon
  def set_level(self, level):
    if level > 4: level = 1
    self.gcomprisBoard.level=level
    self.gcomprisBoard.sublevel=1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.player1.setDifficulty(level)
    self.player2.setDifficulty(level)
    self.newGame()

  # End of Initialisation
  # ---------------------

  def newGame(self):
    if self.boarditem != None:
      self.boarditem.destroy()
    self.boarditem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=(gcompris.BOARD_WIDTH-self.board_size)/2.0,
      y=50.0 
      )
    self.player1 = human.Human(self.gcomprisBoard.level)
    self.player2 = minmax.MinMax(self.gcomprisBoard.level, self.refreshScreen)
    self.board = board.Board()
    self.gamewon = False 
    self.firstPlayer = False 
    self.prof.show()

  def columnItemEvent(self, widget, event, column):
    if event.type == gtk.gdk.BUTTON_PRESS:
      print event.x, event.y
      if event.button == 1 and self.gamewon == False and self.machineHasPlayed:
        column = int((event.x - (gcompris.BOARD_WIDTH-self.board_size)/2.0) // self.stone_size)
        print "columnItemEvent", column
        if not (column < 0 or column > (self.nb_columns-1)):
          self.play(self.player1, 1, column)
          if self.gamewon == False:
            self.tuxboatitem.gnome_canvas.show()
            gcompris.bar_hide(True)
            #To improve the two timers
            #self.timer_machine = gtk.timeout_add(2000, self.machinePlay)
            self.endAnimCallback = self.machinePlay
            self.machineHasPlayed = False
            #thread.start_new_thread(self.machinePlay, ())
   
    return gtk.FALSE
  
  def profItemEvent(self, widget, event, column):
    if event.type == gtk.gdk.BUTTON_PRESS and self.firstPlayer == False:
      self.firstPlayer = True
      self.machinePlay()

  def machinePlay(self):
    print "ai starts"
    self.play(self.player2, 2, 0)
    print "ai ends"
    self.tuxboatitem.gnome_canvas.hide()
    gcompris.bar_hide(False)
    self.prof.hide()
    self.endAnimCallback = self.machinePlayed

  def machinePlayed(self):
    self.machineHasPlayed = True

  def refreshScreen(self):
    gtk.main_iteration(block=False)
    pass

  def play(self, player, numPlayer, column):
    move = player.doMove(self.board, numPlayer, column)
    if type(move) is types.IntType and rules.isMoveLegal(self.board, move):
      self.firstPlayer = True
      self.board.move(move, numPlayer)
      self.drawBoard(self.board)
      if rules.isWinner(self.board, numPlayer):
        return self.winner(numPlayer)
      elif rules.isBoardFull(self.board):
        return self.winner(0)
        
  def drawBoard(self, board):
    stone = self.board.state[self.board.last_move][-1]
    x = self.board.last_move
    y = len(self.board.state[self.board.last_move])-1
    file = "connect4/stone_%d.png" % stone

    self.stone = self.boarditem.add(
      gnome.canvas.CanvasPixbuf,
       pixbuf = gcompris.utils.load_pixmap(file),
       x=x*(self.board_size/self.nb_columns),
       #y=(self.board_size/self.nb_columns)*(self.nb_lines-1-y)
       y=0
       )
    self.stone.connect("event", self.columnItemEvent, 0)
    self.countAnim = self.nb_lines-y
    self.timer_anim = gtk.timeout_add(200, self.timerAnim)

  def timerAnim(self):
    print "timerAnim", self.countAnim
    self.countAnim -= 1
    if self.countAnim > 0:
      y= self.stone.get_property('y')
      self.stone.set_property('y', y + (self.board_size/self.nb_columns))
      self.timer_anim = gtk.timeout_add(200, self.timerAnim)
    else:
      if self.endAnimCallback:
        self.endAnimCallback()

  def winner(self, player):
    self.gamewon = True
    print 'The winner is:', player
    if player == 1:
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)
    elif player == 2:
      gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.FLOWER)
    else:
      gcompris.bonus.display(gcompris.bonus.DRAW, gcompris.bonus.FLOWER)


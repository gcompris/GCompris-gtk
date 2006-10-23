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

import gobject
import gnomecanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.anim
import gtk
import gtk.gdk
from gettext import gettext as _

import types
from  connect4p import rules
from  connect4p import human
from  connect4p import minmax
from  connect4p import board
# ----------------------------------------
#

class Gcompris_connect4:
  """Connect 4 stones"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

  def start(self):
    self.boardSize = 490.0
    self.nbColumns = 7
    self.nbLines = 6
    self.stoneSize = 70.0
    self.maxVictory = 3
    self.maxLevel = 4
    self.firstPlayer = False
    self.boardItem = None
    self.timerAnim = 0
    self.timerMachine = 0
    self.humanVictory = 0
    self.machineHasPlayed = True
    self.endAnimCallback = None
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=9
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    self.winnercall = None
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnomecanvas.CanvasGroup,
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
     gnomecanvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("connect4/back.png"),
      x=0.0,
      y=0.0
      )
    selector.connect("event", self.columnItemEvent, 0)

    self.prof = self.rootitem.add(
     gnomecanvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap("connect4/tux-teacher.png"),
      x=10,
      y=350.0
      )
    self.prof.connect("event", self.profItemEvent, 0)
    self.prof.connect("event", gcompris.utils.item_event_focus)

    self.timericon = gcompris.anim.CanvasItem( gcompris.anim.Animation("connect4/sablier.txt"),
                                               self.rootitem )
    self.timericon.gnomecanvas.hide()

    self.newGame()

  def end(self):
    if self.timerAnim:
      gtk.timeout_remove(self.timerAnim)
    if self.timerMachine:
      gtk.timeout_remove(self.timerMachine)

    del self.timericon

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()
    self.boardItem.destroy()


  def ok(self):
    pass

  def repeat(self):
    if self.humanVictory >= self.maxVictory:
      if self.gcomprisBoard.level < self.maxLevel:
        self.set_level(self.gcomprisBoard.level+1)
      else:
        self.end()
        gcompris.end_board()
    else:
      self.newGame()

  def config(self):
    pass


  def key_press(self, keyval, commit_str, preedit_str):
    self.timericon.gnomecanvas.hide()
    return False

  def pause(self, pause):
    pass

  # Called by gcompris when the user click on the level icon
  def set_level(self, level):
    if level > self.maxLevel: level = 1
    self.gcomprisBoard.level=level
    self.gcomprisBoard.sublevel=1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.player1.setDifficulty(level)
    self.player2.setDifficulty(level)
    self.humanVictory = 0
    self.newGame()

  # End of Initialisation
  # ---------------------

  def newGame(self):
    if self.boardItem != None:
      self.boardItem.destroy()
    self.boardItem = self.gcomprisBoard.canvas.root().add(
      gnomecanvas.CanvasGroup,
      x=(gcompris.BOARD_WIDTH-self.boardSize)/2.0,
      y=50.0
      )
    self.player1 = human.Human(self.gcomprisBoard.level)
    self.player2 = minmax.MinMax(self.gcomprisBoard.level, self.refreshScreen)
    self.board = board.Board()
    self.gamewon = False
    self.machineHasPlayed = True
    self.winLine = None
    try:
      del self.redLine
    except:
      pass
    self.firstPlayer = False
    self.prof.show()

  def columnItemEvent(self, widget, event, column):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1 and self.gamewon == False and self.machineHasPlayed:
        column = int((event.x - (gcompris.BOARD_WIDTH-self.boardSize)/2.0) // self.stoneSize)
        if not (column < 0 or column > (self.nbColumns-1)):
          gcompris.bar_hide(True)
          if self.play(self.player1, 1, column):
            if self.winnercall == None:
              self.timericon.gnomecanvas.show()
              self.endAnimCallback = self.machinePlay
              self.machineHasPlayed = False

    return False

  def profItemEvent(self, widget, event, column):
    #if event.type == gtk.gdk.BUTTON_PRESS and self.firstPlayer == False:
    if event.type == gtk.gdk.BUTTON_PRESS:
      self.firstPlayer = True
      self.machinePlay()

  def machinePlay(self):
    # ai starts
    self.play(self.player2, 2, 0)
    # ai ends
    self.timericon.gnomecanvas.hide()
    self.prof.hide()
    self.endAnimCallback = self.machinePlayed

  def machinePlayed(self):
    self.machineHasPlayed = True
    gcompris.bar_hide(False)

  def refreshScreen(self):
    gtk.main_iteration(block=False)
    pass

  def play(self, player, numPlayer, column):
    move = player.doMove(self.board, numPlayer, column)

    if type(move) is types.IntType and rules.isMoveLegal(self.board, move):
#      self.firstPlayer = True
      self.board.move(move, numPlayer)
      self.drawBoard(self.board)
      self.winLine = rules.isWinner(self.board, numPlayer)

      if self.winLine:
        self.winnercall = lambda : self.winner(numPlayer)
      elif rules.isBoardFull(self.board):
        self.winnercall = lambda : self.winner(0)
      return True

    gcompris.bar_hide(False)
    return False

  def drawBoard(self, board):
    stone = self.board.state[self.board.last_move][-1]
    x = self.board.last_move
    y = len(self.board.state[self.board.last_move])-1
    file = "connect4/stone_%d.png" % stone

    self.stone = self.boardItem.add(
      gnomecanvas.CanvasPixbuf,
       pixbuf = gcompris.utils.load_pixmap(file),
       x=x*(self.boardSize/self.nbColumns),
       #y=(self.boardSize/self.nbColumns)*(self.nbLines-1-y)
       y=0
       )
    self.stone.connect("event", self.columnItemEvent, 0)
    self.countAnim = self.nbLines-y
    self.timerAnim = gtk.timeout_add(200, self.animTimer)

  def animTimer(self):
    self.countAnim -= 1
    if self.countAnim > 0:
      y= self.stone.get_property('y')
      self.stone.set_property('y', y + (self.boardSize/self.nbColumns))
      self.timerAnim = gtk.timeout_add(200, self.animTimer)
    else:
      if self.winnercall:
        self.winnercall()
        self.winnercall=None
        self.endAnimCallback=None
      if self.endAnimCallback:
        self.endAnimCallback()

  def winner(self, player):
    self.gamewon = True

    # It's a draw, no line to draw
    if player == 0:
      gcompris.bonus.display(gcompris.bonus.DRAW, gcompris.bonus.FLOWER)
      return

    # Display the winner line
    if ((self.firstPlayer and (player==2)) or
        ((not self.firstPlayer) and (player==1))):
      self.humanVictory += 1
    else:
      self.humanVictory = 0
    points = ( self.winLine[0][0]*(self.boardSize/self.nbColumns)+self.stoneSize/2,
               (self.boardSize/self.nbColumns)*(self.nbLines-1-self.winLine[0][1])+self.stoneSize/2,
               self.winLine[1][0]*(self.boardSize/self.nbColumns)+self.stoneSize/2,
               (self.boardSize/self.nbColumns)*(self.nbLines-1-self.winLine[1][1])+self.stoneSize/2
               )

    self.redLine = self.boardItem.add(
      gnomecanvas.CanvasLine,
       fill_color_rgba=0xFF0000FFL,
       points=points,
       width_pixels = 8
       )
    self.redLine.set_property("cap-style", gtk.gdk.CAP_ROUND)
    if player == 1:
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)
    elif player == 2:
      gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.FLOWER)


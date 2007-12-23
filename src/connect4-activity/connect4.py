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
import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.anim
import gtk
import gtk.gdk
from gettext import gettext as _

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
        self.boardItem = None
        self.timerAnim = 0
        self.humanVictory = 0
        self.endAnimCallback = None
        self.gcomprisBoard.level=1
        self.gcomprisBoard.maxlevel=9
        self.gcomprisBoard.sublevel=1
        self.gcomprisBoard.number_of_sublevel=1
        self.mode = (self.gcomprisBoard.mode == "2players") and 2 or 1
        if self.mode == 1:
            board_bar = gcompris.BAR_LEVEL
            gcompris.bar_set_level(self.gcomprisBoard)
        else:
            board_bar = 0
        # Create our rootitem. We put each canvas item in it so at the end we
        # only have to kill it. The canvas deletes all the items it contains automaticaly.
        self.rootitem = goocanvas.Group(parent =  self.gcomprisBoard.canvas.get_root_item())

        pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_reload.png"))
        if(pixmap):
            gcompris.bar_set_repeat_icon(pixmap)
            board_bar = board_bar | gcompris.BAR_REPEAT_ICON
        else:
            board_bar = board_bar | gcompris.BAR_REPEAT
        gcompris.bar_set(board_bar)

        selector = \
            goocanvas.Image(
                parent = self.rootitem,
            pixbuf = gcompris.utils.load_pixmap("connect4/back.png"),
            x=0.0,
            y=0.0
            )
        selector.connect("event", self.columnItemEvent, 0)

        if self.mode == 1:
            self.prof = \
                goocanvas.Image(
                    parent = self.rootitem,
                pixbuf = gcompris.utils.load_pixmap("connect4/tux-teacher.png"),
                x=10,
                y=350.0
                )
            self.prof.connect("event", self.profItemEvent, 0)
            self.prof.connect("event", gcompris.utils.item_event_focus)

        self.timericon = gcompris.anim.CanvasItem( gcompris.anim.Animation("connect4/sablier.txt"),
            self.rootitem )
        self.timericon.goocanvas.props.visibility = goocanvas.ITEM_INVISIBLE

        self.player_stone = None

        for i in range(2):
            goocanvas.Image(
                parent = self.rootitem,
                pixbuf = gcompris.utils.load_pixmap("connect4/stone_%d.png" % (i+1)),
                x= 25,
                y=175 + i * 73 )
        self.score_item = \
            goocanvas.Text(
                parent = self.rootitem,
            font=gcompris.skin.get_font("gcompris/board/huge bold"),
            x=60,
            y=250,
            justification=gtk.JUSTIFY_CENTER,
            fill_color_rgba=0xFF0000FFL)

        self.update_scores((0, 0))

        self.newGame()

    def end(self):
        if self.timerAnim:
            gobject.source_remove(self.timerAnim)

        del self.timericon

        # Remove the root item removes all the others inside it
        self.rootitem.remove()
        self.boardItem.remove()

    def ok(self):
        pass

    def repeat(self):
        if max(self.scores) >= self.maxVictory:
            if self.scores[0] >= self.maxVictory and self.mode == 1:
                if self.gcomprisBoard.level < self.maxLevel:
                    self.set_level(self.gcomprisBoard.level+1)
                else:
                    self.end()
                    gcompris.end_board()
            else:
                self.update_scores([0, 0])
                self.newGame()
        else:
            self.newGame()

    def config(self):
        pass
    def update_scores(self, scores=None):
        if scores: self.scores = list(scores)
        txt = str(self.scores[0]) + "\n\n" + str(self.scores[1])
        self.score_item.set(text=txt)

    def stone_init(self):
        if self.player_stone:
            self.player_stone.remove()
        self.player_stone = \
            goocanvas.Image(
                parent = self.rootitem,
            pixbuf = gcompris.utils.load_pixmap("connect4/stone_%d.png"
                                                % self.cur_player),
            x=0,
            y=-20 )
        self.update_stone2()
    def update_stone2(self):
        self.player_stone.set_property("x", (gcompris.BOARD_WIDTH - self.boardSize) /2 +
            self.keyb_column * self.boardSize / self.nbColumns)

    def key_press(self, keyval, commit_str, preedit_str):
        if self.mode == 1 and self.cur_player == 2: # AI playing
            return False
        if self.cur_player == 0 or self.timerAnim:  # Game over or Timer animate
            return False
        key = gtk.gdk.keyval_name(keyval)
        if key == "Left":
            self.keyb_column = max(self.keyb_column-1, 0)
            self.update_stone2()
        elif key == "Right":
            self.keyb_column = min(self.keyb_column+1, self.nbColumns-1)
            self.update_stone2()
        elif key =="Down" or commit_str == " ":
            if self.play(self.player1, self.cur_player, self.keyb_column):
                self.player_stone.props.visibility = goocanvas.ITEM_INVISIBLE
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
        self.update_scores([0, 0])
        self.newGame()

  # End of Initialisation
  # ---------------------

    def newGame(self):
        if self.timerAnim:
            gobject.source_remove(self.timerAnim)
            self.timerAnim = None
        if self.boardItem:
            self.boardItem.remove()
        self.boardItem = \
            goocanvas.Group(
                parent = self.gcomprisBoard.canvas.get_root_item(),
            x=(gcompris.BOARD_WIDTH-self.boardSize)/2.0,
            y=50.0
            )
        self.player1 = human.Human(self.gcomprisBoard.level)
        if self.mode == 1:
            self.player2 = minmax.MinMax(self.gcomprisBoard.level, self.refreshScreen)
        self.board = board.Board()
        self.gamewon = False
        self.winLine = None
        try:
            del self.redLine
        except:
            pass
        self.keyb_column = 0
        self.cur_player = 1
        self.stone_init()
        self.update_stone2()

        self.timericon.goocanvas.props.visibility = goocanvas.ITEM_INVISIBLE
        if self.mode == 1:
            self.prof.props.visibility = goocanvas.ITEM_VISIBLE

    def columnItemEvent(self, widget, event, columns):
        if self.mode == 1 and self.cur_player == 2: # AI playing
            return False
        if self.cur_player == 0 or self.timerAnim:  # Game over or Timer animate
            return False
        if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
            column = int((event.x - (gcompris.BOARD_WIDTH-self.boardSize)/2.0) // self.stoneSize)
            if 0 <= column < self.nbColumns:
                if self.play(self.player1, self.cur_player, column):
                    self.timericon.goocanvas.props.visibility = goocanvas.ITEM_VISIBLE
                    self.player_stone.props.visibility = goocanvas.ITEM_INVISIBLE
        elif event.type == gtk.gdk.MOTION_NOTIFY:
            column = int((event.x - (gcompris.BOARD_WIDTH-self.boardSize)/2.0) // self.stoneSize)
            if 0 <= column < self.nbColumns:
                self.keyb_column = column
                self.update_stone2()
        return False

    def profItemEvent(self, widget, event, column):
        if event.type == gtk.gdk.BUTTON_PRESS:
            self.cur_player=1
            self.prof.props.visibility = goocanvas.ITEM_INVISIBLE
            self.player_stone.props.visibility = goocanvas.ITEM_INVISIBLE
            self.play_next()

    def play_next(self):
        if self.cur_player == 1:
            self.cur_player = 2
            if self.mode == 1:
                # IA play
                self.timericon.goocanvas.props.visibility = goocanvas.ITEM_VISIBLE
                self.play(self.player2, 2, 0)
            else:
                # player 2
                self.stone_init()
        else:
            # player 1
            self.cur_player = 1
            self.stone_init()

    def refreshScreen(self):
        gtk.main_iteration(block=False)
        pass

    def play(self, player, numPlayer, column):
        if self.mode == 1:
            self.prof.props.visibility = goocanvas.ITEM_INVISIBLE
        move = player.doMove(self.board, numPlayer, column)

        if isinstance(move, int) and rules.isMoveLegal(self.board, move):
            self.board.move(move, numPlayer)
            self.drawBoard(self.board)
            return True
        return False

    def drawBoard(self, board):
        stone = self.board.state[self.board.last_move][-1]
        x = self.board.last_move
        y = len(self.board.state[self.board.last_move])-1
        file = "connect4/stone_%d.png" % stone

        self.stone = \
            goocanvas.Image(
                parent = self.boardItem,
            pixbuf = gcompris.utils.load_pixmap(file),
            x=x*(self.boardSize/self.nbColumns),
            y=0
            )
        self.stone.connect("event", self.columnItemEvent, 0)
        self.countAnim = self.nbLines-y
        self.timerAnim = gobject.timeout_add(200, self.animTimer)
        self.timericon.goocanvas.props.visibility = goocanvas.ITEM_VISIBLE

    def animTimer(self):
        self.countAnim -= 1
        if self.countAnim > 0:
            y= self.stone.get_property('y')
            self.stone.set_property('y', y + (self.boardSize/self.nbColumns))
            self.timerAnim = gobject.timeout_add(200, self.animTimer)
        else:
            self.timerAnim = None
            self.timericon.goocanvas.props.visibility = goocanvas.ITEM_INVISIBLE
            self.winLine = rules.isWinner(self.board, self.cur_player)
            if self.winLine:
                self.winner(self.cur_player)
            elif rules.isBoardFull(self.board):
                self.winner(0)
            else:
                self.play_next()

    def winner(self, player):
        self.gamewon = True
        self.cur_player = 0

        # It's a draw, no line to draw
        if player == 0:
            gcompris.bonus.display(gcompris.bonus.DRAW, gcompris.bonus.FLOWER)
            return

        # Display the winner line
        self.scores[player-1] += 1
        self.update_scores()

        points = ( self.winLine[0][0]*(self.boardSize/self.nbColumns)+self.stoneSize/2,
                (self.boardSize/self.nbColumns)*(self.nbLines-1-self.winLine[0][1])+self.stoneSize/2,
                self.winLine[1][0]*(self.boardSize/self.nbColumns)+self.stoneSize/2,
                (self.boardSize/self.nbColumns)*(self.nbLines-1-self.winLine[1][1])+self.stoneSize/2
                )

        self.redLine = \
            goocanvas.Polyline(
                parent = self.boardItem,
            fill_color_rgba=0xFF0000FFL,
            points=points,
            width_pixels = 8
            )
        self.redLine.set_property("cap-style", gtk.gdk.CAP_ROUND)
        if player == 1:
            gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)
        elif player == 2:
            gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.FLOWER)


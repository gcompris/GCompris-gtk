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
# This code comes from the project 4stattack
# http://forcedattack.sourceforge.net/
#
#########################################################################
#                            4st Attack 2                               #
#########################################################################
# Created by:                                                           #
# Developer            - "slm" - Jeroen Vloothuis                       #
# Graphics             - "Korruptor" - Gareth Noyce                     #
# Music                - "theGREENzebra"                                #
#########################################################################
# Specail thanks:                                                       #
# chakie(Jan Elkholm)    - letting me "embrace and extend" his gui lib  #
# Mighty(Xander Soldaat) - for the Makefile and the Debian packages     #
# Han                    - for the rpms                                 #
# jk                     - for the FreeBSD port                         #
# Tjerk Nan              - for the Windows version                      #
# Micon                  - for the webdesign                            #
# Everyone in #pygame and the opensource community in general           #
#########################################################################
# This software is licensed under the GPL - General Public License      #
#########################################################################

try:
  import psyco
except ImportError:
  print "Not using psyco (psyco.sf.net), AI not a maximum speed!"


import rules
from player import *
from random import *
import copy
from board import *

class Node:
  def __init__(self, board, move, player):
    self.board = board
    #self.parent = parent
    self.value = 0
    self.childs = []
    self.move = move
    self.player = player

  def __repr__(self):
    if self.childs > 1:
      number=0
      for child in self.childs:
        number=number+1
      return "Has %s child(s)" % number
    return "STATE"

class MinMax(Player):
  type = 'AI'

  def __init__(self, difficulty, f):
    self.search_depth = difficulty
    self.f = f

  def setDifficulty(self, difficulty):
    self.search_depth = difficulty

  def evaluate(self, node, player, opponent, depth):
    if len(node.childs) > 0:
      list = []
      for child in node.childs:
        list.append(child.value)
      if node.player == player:
        node.value = min(list)
      else:
        node.value = max(list)

    else:
      node.value = self.score(node, player, opponent)  / (depth + 1)

  def score(self, node, player, opponent):
    return int(random() * 100) - 50


  def makeBoard(self, move, board, player):
    temp_board = copy.deepcopy(board)
    temp_board.move(move, player)
    return temp_board

  def listMoves(self, board, player):
    checkmove = rules.isMoveLegal

    options = []
    for move in range(7):
      if checkmove(board, move):
        options.append(move)
    return options

  def statespace(self, node, depth, current_player, player, opponent):
    self.f() 
    if rules.isWinner(node.board, opponent):
      #print "a lose up ahead"
      node.value = -10000 + depth
      return node
    elif rules.isWinner(node.board, player):
      node.value = 10000 - depth
      return node

    elif self.listMoves(node.board, 0) < 1:
      self.evaluate(node, player, opponent, depth)
      return node
    elif depth < self.search_depth:
      if current_player==2:
        next_player = 1
      else:
        next_player = 2
      for move in self.listMoves(node.board, current_player):
        node.board.move(move, current_player)

        node.childs.append(self.statespace(Node( node.board, move, player), depth+1, next_player, player, opponent))
        node.board.undomove(move)

    self.evaluate(node, player, opponent, depth)
    return node


  def doMove(self, current_board, player, event):
    board = copy.deepcopy(current_board)

    if player == 1: opponent = 2
    else: opponent = 1

    node = Node(board, 0, player)

    node =  self.statespace( node, 0, player, player, opponent);

    bestscore = -100000
    best_moves = []
    #print "New Round"
    for child in node.childs:
      #print "Child value=", child.value, "move=", child.board.last_move
      if child.value >= bestscore:
        #if rules.isMoveLegal(board, child.board.last_move):
        if bestscore == child.value:
          #print "Move added"
          best_moves.append(child.move)
        else:
          #print "New best move"
          bestscore = child.value
          best_moves = [child.move]
    return best_moves[int(random()*len(best_moves))]

  def gameOver(self, move):
    return None
#try:
#  psyco.bind(MinMax)
#  psyco.bind(Node)
#except:
#  pass

#  gcompris - connect4
#
# Time-stamp:
#
# Copyright (C) 2005 Laurent Lacheny
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

from board import *

# Checks if the move is legal
def isMoveLegal(board, selector_pos):
	return len(board.state[selector_pos]) < 6 and selector_pos >= 0

def isBoardFull(board):
	for pos in range(7):
		if len(board.state[pos]) < 6:
			return 0
	return 1

def isWinner(board, player):
	result = _isVerticalWin(board, player)
        if not result:
           result =  _isHorizontalWin(board, player)
	if not result:
	   result = _isDiagonalWin(board, player)
        return result

def _isVerticalWin(board, player):
   x = board.last_move
   four_in_a_row = [player, player, player, player]
   if board.state[x][-4:] == four_in_a_row:
      return ((x,len(board.state[x])-4),(x,len(board.state[x])-1))
   else:
      return None

def _isHorizontalWin(board, player):
        x = board.last_move
        y = len(board.state[x]) - 1
        four_in_a_row = [player, player, player, player]
        row = []
        for i in range(7):
                try:
                        row.append(board.state[i][y])
                except IndexError:
                        row.append('s')  # 's' stands for sentinel
	for i in range(4):
	  try:
             if row[x - 3 + i : x + 1 +i] == four_in_a_row:
	        return ((x -3 + i,y),( x +i,y))
	  except IndexError:
	     pass

        return None

def _isDiagonalWin(board, player):
        x = board.last_move
        y = len(board.state[x]) - 1
        four_in_a_row = [player, player, player, player]
        row = []
        for i in range(-3, 4):
                try:
			if  (x+i < 0) or (y+i < 0):
				row.append('s')  # 's' stands for sentinel
			else:
				row.append(board.state[x+i][y+i])
                except IndexError:
                        row.append('s')  # 's' stands for sentinel
	for i in range(4):
	  try:
             if row[i : i + 4] == four_in_a_row:
	        return ((x -3 + i,y-3+i),( x +i,y+i))
	  except IndexError:
	     pass

        row = []
        for i in range(-3, 4):
                try:
			if  (x+i < 0) or (y-i < 0):
				row.append('s')  # 's' stands for sentinel
			else:
				row.append(board.state[x+i][y-i])
                except IndexError:
                        row.append('s')  # 's' stands for sentinel
	for i in range(4):
	  try:
             if row[i : i + 4] == four_in_a_row:
	        return ((x -3 + i , y + 3 - i),( x +i , y - i))
	  except IndexError:
	     pass

        return None

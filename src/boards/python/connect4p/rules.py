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
        if _isVerticalWin(board, player):
                return 4
        if _isHorizontalWin(board, player):
                return 4
        if _isDiagonalWin(board, player):
                return 4
        return 0


def _isVerticalWin(board, player):
        x = board.last_move
        four_in_a_row = [player, player, player, player]
        return board.state[x][-4:] == four_in_a_row


def _isHorizontalWin(board, player):
        x = board.last_move
        y = len(board.state[x]) - 1
        four_in_a_row = str(player) * 4
        row = []
        for i in range(-3, 4):
                try:
                        if x+i < 0: continue
                        row.append(str(board.state[x+i][y]))
                except IndexError:
                        row.append('s')  # 's' stands for sentinel
        return ''.join(row).find(four_in_a_row) >= 0


def _isDiagonalWin(board, player):
        x = board.last_move
        y = len(board.state[x]) - 1
        four_in_a_row = str(player) * 4
        row = []
        for i in range(-3, 4):
                try:
                        if x+i < 0: continue
                        if y+i < 0: continue
                        row.append(str(board.state[x+i][y+i]))
                except IndexError:
                        row.append('s')  # 's' stands for sentinel
        if ''.join(row).find(four_in_a_row) >= 0:
                return 1
        row = []
        for i in range(-3, 4):
                try:
                        if x+i < 0: continue
                        if y-i < 0: continue
                        row.append(str(board.state[x+i][y-i]))
                except IndexError:
                        row.append('s')  # 's' stands for sentinel
        if ''.join(row).find(four_in_a_row) >= 0:
                return 1
        return 0

"""

# Checks if theres a winner
def isWinner(board, player):

	# Check vertical wins
	for x in range(7):
		sequence = 0
		for y in range(len(board.state[x])):
			if board.state[x][y] == player:
				sequence += 1
				if sequence == 4:
					# Whe've got a winner!!
					return sequence
			else: sequence = 0
	# Check horizontal wins
	for y in range(6):
		sequence = 0
		for x in range(7):
			# Make sure we wont go out of bounds
			if len(board.state[x]) > y:
				if board.state[x][y] == player:
					sequence += 1
                	                if sequence == 4:
                        	               # Whe've got a winner!!
                                	       return sequence
				else: sequence = 0
			else: sequence = 0

	# Check diagonally - bottom to top
	for height in range(6+1-4):
		for start in range(7+1-4):
			sequence = 0
			for pos in range(7 - start):
				# Make sure we wont go out of bounds
	                        if len(board.state[pos+start]) > (pos + height):
				 	if board.state[pos+start][pos+ height] == player:
						sequence += 1
						if sequence == 4:
							# Whe've got a winner!!
							return sequence
					else: sequence = 0
				else: sequence = 0

	# Check diagonally - top to bottom
	for height in range(6+2-4):
		for start in range(7+1-4):
			sequence = 0
			for pos in range(7-start):
				# Make sure we wont go out of bounds
	                        if (len(board.state[pos+start]) > (6 - pos - height) )  and (6 - pos - height) >=0 :
				 	if board.state[pos+start][6 - pos - height] == player:
						sequence += 1
						if sequence == 4:
							# Whe've got a winner!!
							return sequence
					else: sequence = 0
				else: sequence = 0
	# No win for player
	return 0
"""

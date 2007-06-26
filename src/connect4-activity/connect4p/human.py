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

from player import *
import rules

class Human:
  type = 'human'

  def __init__(self, difficulty):
    pass

  # This method returns the move to be made
  def doMove(self, board, player, event):
    self.board = board
    if event >-1 and event < 7:
      return event
    return -1

  def setmove(self, trigger, event, number):
    if rules.isMoveLegal(self.board, number):
      self.move = number
      return widget.DONE
  def gameOver(self, move):
    return None

  def setDifficulty(self, difficulty):
    pass

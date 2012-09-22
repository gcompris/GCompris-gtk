#  gcompris - mining_tools.py
#
# Copyright (C) 2012 Peter Albrecht
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


class Area:
  """ A class for comfortable working with goocanvas.Bounds """

  def __init__(self, bounds):
    self.x1 = bounds.x1
    self.x2 = bounds.x2
    self.y1 = bounds.y1
    self.y2 = bounds.y2

    self.width = self.x2 - self.x1
    self.height = self.y2 - self.y1

    self.center_x = self.x1 + self.width / 2.0
    self.center_y = self.y1 + self.height / 2.0


class BlockingArea:
  """ This class defines a blocking area, where no nugget should be put """

  def __init__(self, x1, y1, x2, y2):
    """
    Constructor:
      x1, y1, x2, y2 : bounding box of the blocker in canvas coordinates (0 - 800 / 0 - 520)
    """
    self.bounds = goocanvas.Bounds(x1, y1, x2, y2)


  def get_bounds(self):
    """ Return the bounds, defined in the constructor """
    return self.bounds

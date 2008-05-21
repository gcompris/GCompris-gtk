#  gcompris - anim : Item management base class
#
# Copyright (C) 2008 Bruno Coudoin
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

import gobject
import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.sound
import gtk
import gtk.gdk
import math
import cairo

class AnimItem:

    def __init__(self, anim):
        self.anim = anim
        self.rootitem = goocanvas.Group(parent = anim.rootitem)
        self.drawing_area = anim.drawing_area
        self.step = anim.current_step
        gcompris.sound.play_ogg("sounds/bleep.wav")

        self.step = 1

        self.item = None
        self.events = None
        self.anchor = None

        self.old_x = 0
        self.old_y = 0

    # Given x,y return a new x,y snapped to the grid
    def snap_to_grid(self, x, y):

        if self.item:
            (x, y) = self.anim.gcomprisBoard.canvas.\
                convert_from_item_space(self.item, x, y)

        # Check drawing boundaries
        if(x < self.drawing_area[0]):
          x = self.drawing_area[0]
        if(x > self.drawing_area[2]):
          x = self.drawing_area[2]
        if(y < self.drawing_area[1]):
          y = self.drawing_area[1]
        if(y > self.drawing_area[3]):
          y = self.drawing_area[3]

        result = []
        tmp = round(((x+(self.step)) -
                     self.drawing_area[0])/self.step) - 1
        result.append(float(self.drawing_area[0] + tmp*self.step))

        tmp = round(((y+(self.step)) -
                     self.drawing_area[1])/self.step) - 1
        result.append(float(self.drawing_area[1] + tmp*self.step))

        if self.item:
            return self.anim.gcomprisBoard.canvas.\
                convert_to_item_space(self.item, x, y)

        return result

    # Given two points p1 and p2, return the
    # boundings coordinates (x1, y2, x2, y2)
    # all snaped to the grid
    def snap_obj_to_grid(self, p1, p2):
        x = min(p1[0], p2[0])
        y = min(p1[1], p2[1])
        (x1, y1) = self.snap_to_grid(x, y)
        w = abs(p1[0] - p2[0])
        h = abs(p1[1] - p2[1])
        (x2, y2) = self.snap_to_grid(x+w, y+h)
        return (x1, y1, x2, y2)

    # Selecting the item creates and display its anchors
    def select(self):
        if not self.anchor:
            self.anchor = Anchor(self)
        self.anchor.show()

    # Deselecting the item deletes the anchor
    def deselect(self):
        if self.anchor:
            self.anchor.hide()

    #
    # Default actions, maybe overloaded by specific items
    #
    def flip(self):
        bounds = self.item.get_bounds()
        (cx, cy) = ( bounds.x1 + (bounds.x2-bounds.x1)/2,
                     bounds.y1 + (bounds.y2-bounds.y1)/2 )
        m1 = self.item.get_transform()
        m2 = cairo.Matrix( -1, 0, 0, 1, 2*cx, 0)
        self.item.set_transform(m1*m2)

        if self.anchor:
            self.anchor.update()


    def fill(self, fill, stroke):
        gcompris.sound.play_ogg("sounds/paint1.wav")
        self.item.set_properties(fill_color_rgba = fill,
                                 stroke_color_rgba = stroke)

    def delete(self):
        gcompris.sound.play_ogg("sounds/eraser1.wav",
                                "sounds/eraser2.wav")
        fill = self.item.get_property("fill_color_rgba")
        stroke = self.item.get_property("stroke_color_rgba")
        fill_alpha = (fill & 0x000000FFL) / 2
        stroke_alpha = (stroke & 0x000000FFL) /2
        fill &= 0xFFFFFF00L | fill_alpha
        stroke &= 0xFFFFFF00L | stroke_alpha
        self.item.set_properties(fill_color_rgba = fill,
                                 stroke_color_rgba = stroke)

    def raise_(self):
        print "raise"
        self.item.raise_(None)

    def lower(self):
        print "lower"
        self.item.lower(None)


    def rotate(self, angle):
        bounds = self.item.get_bounds()
        (cx, cy) = ( bounds.x1 + (bounds.x2-bounds.x1)/2,
                     bounds.y1 + (bounds.y2-bounds.y1)/2 )
        (cx, cy) = self.anim.gcomprisBoard.canvas.\
            convert_to_item_space(self.item, cx, cy)
        self.item.rotate(angle, cx, cy)

        if self.anchor:
            self.anchor.update()


    def create_item_event(self, item, target, event):

        if (event.type == gtk.gdk.BUTTON_RELEASE
            or event.type == gtk.gdk.BUTTON_PRESS):
            self.refpoint = None

        elif (event.type == gtk.gdk.MOTION_NOTIFY
            and event.state & gtk.gdk.BUTTON1_MASK):

            if not self.refpoint:
                self.refpoint = self.anim.gcomprisBoard.canvas.\
                    convert_from_item_space(target,
                                          self.get_x1y1()[0],
                                          self.get_x1y1()[1])

            (x, y) = self.anim.gcomprisBoard.canvas.\
                convert_from_item_space(item, event.x, event.y)

            self.set_bounds(
                self.refpoint,
                (x, y) )

    def move_item_event(self, item, target, event):
        if event.type == gtk.gdk.BUTTON_RELEASE:
            self.old_x = 0
            self.old_y = 0
        elif event.type == gtk.gdk.BUTTON_PRESS:
            self.old_x = event.x
            self.old_y = event.y
        elif event.type == gtk.gdk.MOTION_NOTIFY:
            dx = event.x - self.old_x
            dy = event.y - self.old_y

            bounds = self.item.get_bounds()
            # Check drawing boundaries
            if(bounds.x1 + dx < self.drawing_area[0]):
                dx = self.drawing_area[0] - bounds.x1
            if(bounds.x2 + dx > self.drawing_area[2]):
                dx = self.drawing_area[2] - bounds.x2
            if(bounds.y1 + dy < self.drawing_area[1]):
                dy = self.drawing_area[1]- bounds.y1
            if(bounds.y2 + dy > self.drawing_area[3]):
                dy = self.drawing_area[3]- bounds.y2

            self.item.translate(dx, dy)

            if self.anchor:
                self.anchor.update()

  #
  # Add the anchors and callbacks on an item
  #
class Anchor:
    # group contains normal items.

    A_WIDTH = 14
    A_HEIGHT = 14

    # anchortype
    ANCHOR_NW = 1
    ANCHOR_N  = 2
    ANCHOR_NE = 3
    ANCHOR_E  = 4
    ANCHOR_W  = 5
    ANCHOR_SW = 6
    ANCHOR_S  = 7
    ANCHOR_SE = 8

    anchortypes = [ ANCHOR_N,
                ANCHOR_NE,
                ANCHOR_E,
                ANCHOR_SE,
                ANCHOR_S,
                ANCHOR_SW,
                ANCHOR_W,
                ANCHOR_NW ]

    # kind of beautiful blue
    ANCHOR_COLOR = 0x36ede480

    def __init__(self, animitem):
        self.animitem = animitem

        self.anchorgroup = None
        self.anchors = []

        self.anchorgroup = \
          goocanvas.Group(
            parent = animitem.anim.rootitem,
          )

        self.anchorbound = \
            goocanvas.Rect(
                parent = self.anchorgroup,
                stroke_color_rgba = 0x000000FFL,
                line_width = 3)

        for anchor_type in self.anchortypes:
            anchor = \
                goocanvas.Rect(
                    parent = self.anchorgroup,
                    fill_color_rgba = self.ANCHOR_COLOR,
                    stroke_color_rgba = 0x000000FFL,
                    width = self.A_WIDTH,
                    height = self.A_HEIGHT,
                    line_width = 1,
                    )
            self.anchors.append(anchor)
            anchor.connect("button_press_event",
                           self.resize_item_event, anchor_type)
            anchor.connect("button_release_event",
                           self.resize_item_event, anchor_type)
            anchor.connect("motion_notify_event",
                           self.resize_item_event, anchor_type)

        self.refpoint = None
        self.fixed_x = 0
        self.fixed_y = 0

    def show(self):
        self.anchorgroup.props.visibility = goocanvas.ITEM_VISIBLE
        self.anchorgroup.raise_(None)
        self.update()

    def update(self):
        A_WIDTH = self.A_WIDTH
        A_HEIGHT = self.A_HEIGHT
        b = self.animitem.item.get_bounds()

        self.anchorbound.set_properties(x = b.x1,
                                        y = b.y1,
                                        width = b.x2 - b.x1,
                                        height = b.y2 - b.y1)

        # Update the anchors
        self.update_anchor(self.anchors[0],
                           b.x1 + (b.x2 - b.x1)/2 - A_WIDTH/2,
                           b.y1- A_HEIGHT/2)
        self.update_anchor(self.anchors[1],
                           b.x2 - A_WIDTH/2,
                           b.y1- A_HEIGHT/2)
        self.update_anchor(self.anchors[2],
                           b.x2 - A_WIDTH/2,
                           b.y1 + (b.y2 - b.y1)/2 - A_HEIGHT/2)
        self.update_anchor(self.anchors[3],
                           b.x2 - A_WIDTH/2,
                           b.y2 - A_HEIGHT/2)
        self.update_anchor(self.anchors[4],
                           b.x1 + (b.x2 - b.x1)/2 - A_WIDTH/2,
                           b.y2 - A_HEIGHT/2)
        self.update_anchor(self.anchors[5],
                           b.x1 - A_WIDTH/2,
                           b.y2 - A_HEIGHT/2)
        self.update_anchor(self.anchors[6],
                           b.x1 - A_WIDTH/2,
                           b.y1 + (b.y2 - b.y1)/2 - A_HEIGHT/2)
        self.update_anchor(self.anchors[7],
                           b.x1 - A_WIDTH/2,
                           b.y1 - A_HEIGHT/2)


    def hide(self):
        self.anchorgroup.props.visibility = goocanvas.ITEM_INVISIBLE

    def update_anchor(self, anchor, x, y):
        anchor.set_properties(x = x,
                              y = y)


    def resize_item_event(self, item, target, event, anchor):

        if (event.type == gtk.gdk.BUTTON_RELEASE
            or event.type == gtk.gdk.BUTTON_PRESS):
            self.refpoint = None
            self.fixed_x = 0
            self.fixed_y = 0

        elif (event.type == gtk.gdk.MOTION_NOTIFY
            and event.state & gtk.gdk.BUTTON1_MASK):

            if not self.refpoint:
                if anchor == self.ANCHOR_N:
                    self.refpoint = self.animitem.get_x2y2()
                    self.fixed_x = self.animitem.get_x1y1()[0]
                elif anchor == self.ANCHOR_NE:
                    self.refpoint = self.animitem.get_x1y2()
                elif anchor == self.ANCHOR_E:
                    self.refpoint = self.animitem.get_x1y1()
                    self.fixed_y = self.animitem.get_x2y2()[1]
                elif anchor == self.ANCHOR_SE:
                    self.refpoint = self.animitem.get_x1y1()
                elif anchor == self.ANCHOR_S:
                    self.refpoint = self.animitem.get_x1y1()
                    self.fixed_x = self.animitem.get_x2y2()[0]
                elif anchor == self.ANCHOR_SW:
                    self.refpoint = self.animitem.get_x2y1()
                elif anchor == self.ANCHOR_W:
                    self.refpoint = self.animitem.get_x2y2()
                    self.fixed_y = self.animitem.get_x1y1()[1]
                elif anchor == self.ANCHOR_NW:
                    self.refpoint = self.animitem.get_x2y2()

            (x, y) = self.animitem.anim.gcomprisBoard.canvas.\
                convert_from_item_space(item, event.x, event.y)
            (x, y) = self.animitem.anim.gcomprisBoard.canvas.\
                convert_to_item_space(self.animitem.item, x, y)

            if self.fixed_x:
                self.animitem.set_bounds(
                    self.refpoint,
                    (self.fixed_x,
                     y) )
            elif self.fixed_y:
                self.animitem.set_bounds(
                    self.refpoint,
                    (x,
                     self.fixed_y) )
            else:
                self.animitem.set_bounds(
                    self.refpoint,
                    (x,
                     y) )

            self.update()

#
# The Filled rectangle
#
class AnimItemFillRect(AnimItem):

    x = 0
    y = 0
    width = 0
    height = 0
    line_width = 1

    def __init__(self, anim, x, y):
        AnimItem.__init__(self, anim)
        x,y = self.snap_to_grid(x, y)
        self.x = x
        self.y = y

        self.item = \
            goocanvas.Rect(
                parent = self.rootitem,
                x = self.x,
                y = self.y,
                width = self.width,
                height = self.height,
                fill_color_rgba = self.anim.color.fill,
                stroke_color_rgba = self.anim.color.stroke,
                line_width = self.line_width)

        self.item.set_data("AnimItem", self)
        self.item.connect("button_press_event", anim.item_event)
        self.item.connect("button_release_event", anim.item_event)
        self.item.connect("motion_notify_event", anim.item_event)

    # Fixme, should replace set_bounds in resize cases
    def scale_bounds(self, p1, p2):
        (x1, y1, x2, y2) = self.snap_obj_to_grid(p1, p2)
        bounds = self.item.get_bounds()
        sx = (x2 - x1) / (bounds.x2 - bounds.x1)
        sy = (y2 - y1) / (bounds.y2 - bounds.y1)
        print "sx=%f sy=%f" %(sx, sy)
        self.item.scale(sx, sy)


    def set_bounds(self, p1, p2):
        (x1, y1, x2, y2) = self.snap_obj_to_grid(p1, p2)
        self.item.set_properties(x = x1,
                                 y = y1,
                                 width = abs(x2-x1),
                                 height = abs(y2-y1) )

    def get_x1y1(self):
        x = self.item.get_property("x")
        y = self.item.get_property("y")
        return(x, y)

    def get_x2y1(self):
        x = self.item.get_property("x") + self.item.get_property("width")
        y = self.item.get_property("y")
        return(x, y)

    def get_x2y2(self):
        x = self.item.get_property("x") + self.item.get_property("width")
        y = self.item.get_property("y") + self.item.get_property("height")
        return(x, y)

    def get_x1y2(self):
        x = self.item.get_property("x")
        y = self.item.get_property("y") + self.item.get_property("height")
        return(x, y)


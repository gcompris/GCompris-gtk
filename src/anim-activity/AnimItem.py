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
import sys

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

        # We keep the timeline index to which we are visible
        # This is a list of tuple (from, to)
        self.visible = []

        # The timeline store the state of this item in time.
        # The key is the time (number) and the value is a tuple
        # (properties, transformation).
        self.timeline = {}

    def test(self):
        self.set_visible(0, 10)
        self.set_visible(11, 20)
        self.set_visible(4, 8)
        self.set_visible(0, 10)
        self.set_visible_to_end(1000)
        print self.is_visible(10)
        print self.is_visible(800)
        print self.is_visible(1001)
        self.delete_at_time(10)
        self.delete_at_time(1000)
        self.delete_at_time(20)
        print self.visible

    # Mark this object to be visible from fromtime to totime.
    def set_visible(self, fromtime, totime):
        self.visible.append( (fromtime, totime) )
        self.visible.sort()

        # Reduce the items in the list that overlaps
        deleteit = []
        for index in range(0, len(self.visible)-1):
            # Overlap set
            if self.visible[index][1] >= self.visible[index+1][0] - 1:
                self.visible[index] = \
                    (min (self.visible[index][0], self.visible[index+1][0]),
                     max (self.visible[index][1], self.visible[index+1][1]) )
                deleteit.append(index+1)

        for i in deleteit:
            del self.visible[i]

    # Mark this object to be visible from fromtime to the end
    # of the timeline.
    def set_visible_to_end(self, fromtime):
        self.set_visible(fromtime, sys.maxint)

    # Mark this object to be deleted at the given time
    def delete_at_time(self, time):
        for index in range(0, len(self.visible)):
            # It's the set start
            if self.visible[index][0] == time:
                self.visible[index] = \
                    (self.visible[index][0] + 1,
                     self.visible[index][1])
                break
            # It's the set end
            elif self.visible[index][1] == time:
                self.visible[index] = \
                    (self.visible[index][0],
                     self.visible[index][1] - 1)
                break
            # It's inside the set
            elif (self.visible[index][0] <= time and
                self.visible[index][1] >= time):
                oldend = self.visible[index][1]
                self.visible[index] = \
                    (self.visible[index][0], time - 1)
                self.visible.append((time + 1, oldend))
                self.visible.sort()
                break

    # Given a timeline index, return True if it is visible
    def is_visible(self, index):
        for visset in self.visible:
            if ( visset[0] <= index and
                 visset[1] >= index ):
                 return True

        return False

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


    def delete(self):
        gcompris.sound.play_ogg("sounds/eraser1.wav",
                                "sounds/eraser2.wav")
        self.delete_at_time(self.anim.timeline.get_time())
        self.show(False)
        print self.visible

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


    # Request to hide or show the object
    def show(self, state):
        if state:
            self.item.props.visibility = goocanvas.ITEM_VISIBLE
        else:
            self.item.props.visibility = goocanvas.ITEM_INVISIBLE
            if self.anchor:
                self.anchor.hide()


    def create_item_event(self, item, target, event):

        if (event.type == gtk.gdk.BUTTON_RELEASE
            or event.type == gtk.gdk.BUTTON_PRESS):
            self.refpoint = None
            self.save_at_time(self.anim.timeline.get_time())
            # By default, an object is displayed till the timeline end
            self.set_visible_to_end(self.anim.timeline.get_time())

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

            # We need to have a translation matrix set
            self.item.translate(0, 0)

    def move_item_event(self, item, target, event):
        if event.type == gtk.gdk.BUTTON_RELEASE:
            self.old_x = 0
            self.old_y = 0
            self.save_at_time(self.anim.timeline.get_time())
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


    # Save the current place of the object for the given time
    def save_at_time(self, time):
        self.timeline[time] = self.get()

    def display_at_time(self, time):

        if self.is_visible(time):
            self.show(True)
        else:
            self.show(False)
            # No need to waste more time there
            return

        # If we have a value at that time, use it.
        if self.timeline.has_key(time):
            self.set(self.timeline[time][0], self.timeline[time][1])
            return

        # We have to find the latest closest time for this object
        lastval = []
        for k, v in self.timeline.iteritems():
            lastval = v
            break
        for k, v in self.timeline.iteritems():
            if(k > time):
                self.set(lastval[0],
                         lastval[1])
                break
            lastkey = k
            lastval = v


    # Return the (properties, transformation) of this
    # object.
    def get(self):
        result = {}
        for prop in self.get_properties():
            result[prop] = self.item.get_property(prop)
        return(result, self.item.get_transform())

    # Apply the given properties and transformation to this
    # object.
    def set(self, prop, transform):
        self.item.set_properties(**prop)
        self.item.set_transform(transform)
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
            self.offset_x = 0
            self.offset_y = 0
            self.animitem.save_at_time(self.animitem.anim.timeline.get_time())

        elif (event.type == gtk.gdk.MOTION_NOTIFY
            and event.state & gtk.gdk.BUTTON1_MASK):

            (x, y) = self.animitem.anim.gcomprisBoard.canvas.\
                convert_from_item_space(item, event.x, event.y)
            (x, y) = self.animitem.anim.gcomprisBoard.canvas.\
                convert_to_item_space(self.animitem.item, x, y)

            if not self.refpoint:
                mypoint = [] # Will reference the selected anchors coord
                if anchor == self.ANCHOR_N:
                    self.refpoint = self.animitem.get_x2y2()
                    self.fixed_x = self.animitem.get_x1y1()[0]
                    mypoint = self.animitem.get_x1y1()
                elif anchor == self.ANCHOR_NE:
                    self.refpoint = self.animitem.get_x1y2()
                    mypoint = self.animitem.get_x2y1()
                elif anchor == self.ANCHOR_E:
                    self.refpoint = self.animitem.get_x1y1()
                    self.fixed_y = self.animitem.get_x2y2()[1]
                    mypoint = self.animitem.get_x2y2()
                elif anchor == self.ANCHOR_SE:
                    self.refpoint = self.animitem.get_x1y1()
                    mypoint = self.animitem.get_x2y2()
                elif anchor == self.ANCHOR_S:
                    self.refpoint = self.animitem.get_x1y1()
                    self.fixed_x = self.animitem.get_x2y2()[0]
                    mypoint = self.animitem.get_x2y2()
                elif anchor == self.ANCHOR_SW:
                    self.refpoint = self.animitem.get_x2y1()
                    mypoint = self.animitem.get_x1y2()
                elif anchor == self.ANCHOR_W:
                    self.refpoint = self.animitem.get_x2y2()
                    self.fixed_y = self.animitem.get_x1y1()[1]
                    mypoint = self.animitem.get_x1y1()
                elif anchor == self.ANCHOR_NW:
                    self.refpoint = self.animitem.get_x2y2()
                    mypoint = self.animitem.get_x1y1()

                self.offset_x = mypoint[0] - x
                self.offset_y = mypoint[1] - y

            if self.fixed_x:
                self.animitem.set_bounds(
                    self.refpoint,
                    (self.fixed_x,
                     y + self.offset_y) )
            elif self.fixed_y:
                self.animitem.set_bounds(
                    self.refpoint,
                    (x + self.offset_x,
                     self.fixed_y) )
            else:
                self.animitem.set_bounds(
                    self.refpoint,
                    (x + self.offset_x,
                     y + self.offset_y) )

            self.update()

#
# The Filled rectangle
#
class AnimItemRect(AnimItem):

    x = 0
    y = 0
    width = 0
    height = 0
    filled = False

    def __init__(self, anim, x, y, color_fill, color_stroke, line_width):
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
                stroke_color_rgba = color_stroke,
                line_width = line_width)

        if color_fill:
            self.filled = True
            self.item.set_properties(fill_color_rgba = color_fill)

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

    # Return the list of properties that have to be saved for
    # this object
    def get_properties(self):
        return('x', 'y',
               'width', 'height',
               'fill_color_rgba',
               'stroke_color_rgba')

    def fill(self, fill, stroke):
        gcompris.sound.play_ogg("sounds/paint1.wav")
        if self.filled:
            self.item.set_properties(fill_color_rgba = fill,
                                     stroke_color_rgba = stroke)
        else:
            self.item.set_properties(stroke_color_rgba = stroke)


# -*- coding: utf-8 -*-
#  gcompris - gnumch
#
# Copyright (C) 2005, 2008 Joe Neeman
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

import gobject
import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import gcompris.score
import gcompris.anim
import gcompris.sound
import gobject
import gtk
import gtk.gdk
import random
import math
import os.path

from gcompris import gcompris_gettext as _

class Number:
    def __init__(self, text, good):
        self.text = text
        self.good = good

class Square:
    def __init__(self, x, y):
        self.num = None
        self.pic = goocanvas.Text(
            parent = game.rootitem,
            text = "",
            font = gcompris.skin.get_font("gcompris/content"),
            x = x,
            y = y )

    def setNum(self, num):
        self.num = num
        if num != None:
            self.pic.set_properties( text = num.text )
        else:
            self.pic.set_properties( text = "" )

class Level:
    def __init__(self, title, numlist):
        self.title = title
        self.numbers = numlist

# the following class goes unused: is is only here to document the Levelset interface.
class Levelset:
    def __init__(self):
        pass

    def getError(self, num):
        pass

    def getTitle(self):
        pass

    def setLevel(self, level, sublevel):
        pass

    def getNumber(self):
        pass

def isPrime(n):
    if n == 1:
        return 0
    for i in range(2, int(math.sqrt(n) + 1) ):
        if (n % i) == 0:
            return 0
    return 1

def makeNumList(nums):
    if len(nums) == 0:
        return ""
    fmt = '%d'
    if len(nums) >= 2:
        for i in range(1, len(nums)-1):
            fmt += _(', %d')
        fmt += _(' and %d')
    return fmt

def getFactors(n):
    f = []
    for i in range(1, n/2 + 1):
        if n%i == 0:
            f.append(i)
    f.append(n)
    return f

class PrimeLevelset:
    def __init__(self):
        self.numlevels = 1
        self.num_sublevels = 9
        self.level_max = [ 3, 5, 7, 11, 13, 17, 19, 23, 29 ]
        self.curlevel = 1
        self.cur_sublevel = 1

    def getError(self, num):
        fmt = _('{0} is divisible by {1}.')
        n = int(num.text)

        if n == 1:
            return _("1 is not a prime number.")

        factors = []
        for i in range(2, n/2 + 1):
            if n % i == 0:
                factors.append(i)
        s = makeNumList(factors) % tuple(factors)
        return fmt.format(str(n), s)

    def getTitle(self):
        return _('Primes less than %d') % ( self.level_max[self.cur_sublevel-1] + 1 )

    def setLevel(self, level, sublevel):
        self.cur_sublevel = level
        self.cur_sublevel = sublevel

    def getNumber(self):
        n = random.randint( 1, self.level_max[self.cur_sublevel-1] )
        return Number( str(n), isPrime(n) )

class FactorLevelset:
    def __init__(self):
        self.num_sublevels = 9
        self.numlevels = 1
        self.level_multiple = [ 4, 6, 8, 10, 12, 15, 18, 20, 24 ]
        self.curlevel = 1
        self.cur_sublevel = 1
        self.factors = []
        self.nonfactors = []

    def getError(self, num):
        # Translators: You can swap %(x)y elements in the string.
        fmt = _('Multiples of %(d1)d include %(s)s,\nbut %(d2)d is not a multiple of %(d3)d.')
        n = int(num.text)
        mults = []
        for i in range(2, 5):
            mults.append(n*i)
        return fmt % { 'd1': n,
                       's': makeNumList(mults) % tuple(mults),
                       'd2': self.level_multiple[self.cur_sublevel-1],
                       'd3': n }

    def getTitle(self):
        return _('Factors of %d') % ( self.level_multiple[self.cur_sublevel-1] )

    def setLevel(self, level, sublevel):
        self.curlevel = level
        self.cur_sublevel = sublevel
        self.factors = []
        self.nonfactors = []
        for i in range(1, self.level_multiple[sublevel-1]+1):
            if self.level_multiple[sublevel-1] % i == 0:
                self.factors.append(i)
            else:
                self.nonfactors.append(i)

    def getNumber(self):
        if random.randint(0,1):
            # choose a good number
            n = self.factors[ random.randint(0, len(self.factors)-1) ]
            num = Number( str(n), 1 )
        else:
            # choose a wrong number
            n = self.nonfactors[ random.randint(0, len(self.nonfactors)-1) ]
            num = Number( str(n), 0 )
        return num

class MultipleLevelset:
    def __init__(self):
        self.numlevels = 4
        self.num_sublevels = 9
        self.min_mult = 4
        self.curlevel = 1
        self.cur_sublevel = 1

    def getError(self, num):
        fmt = _('{0} are the factors of {1}.')
        n = int(num.text)

        factors = []
        for i in range(1, n/2+1):
            if n % i == 0:
                factors.append(i)
        factors.append(n)
        s = makeNumList(factors) % tuple(factors)
        return fmt.format(s, str(n))

    def getTitle(self):
        return _('Multiples of %d') % ( self.cur_sublevel+1 )

    def setLevel(self, level, sublevel):
        self.curlevel = level
        self.cur_sublevel = sublevel

    def getNumber(self):
        if random.randint(0,1):
            # choose a good number
            n = (self.cur_sublevel+1) * random.randint(1, self.min_mult + self.curlevel*2)
            num = Number( str(n), 1 )
        else:
            # choose a wrong number
            n = (self.cur_sublevel+1) * random.randint(1, self.min_mult + self.curlevel*2) - random.randint(1, self.cur_sublevel)
            num = Number( str(n), 0 )
        return num

# for all expression-based levels, we add a value field to the Number
class ExpressionLevelset(object):
    def __init__(self):
        self.numlevels = 7
        self.num_sublevels = 7
        self.levelops = [ [self.getPlus],
                          [self.getMinus],
                          [self.getPlus, self.getMinus],
                          [self.getTimes],
                          [self.getPlus, self.getMinus, self.getTimes],
                          [self.getDivide],
                          [self.getPlus, self.getMinus, self.getTimes, self.getDivide]
                        ]
        self.curlevel = 1
        self.cur_sublevel = 1

    def getError(self, num):
        fmt = _('{0} = {1}')
        return fmt.format(num.text, str(num.value))

    def getNumberWithAnswer(self, answer):
        fn = random.choice( self.levelops[self.curlevel-1] )
        num = fn(answer)
        num.value = answer
        return num

    def getPlus(self, answer):
        n = random.randint(0, answer)
        num = Number( _(u'{0} + {1}').format(str(n), str(answer-n)), 1 )
        return num

    def getMinus(self, answer):
        n = random.randint(answer, answer*2)
        num = Number( _(u'{0} \u2212 {1}').format(str(n), str(n-answer)), 1 )
        return num

    def getTimes(self, answer):
        n = random.choice( getFactors(answer) )
        return Number( _(u'{0} \u00d7 {1}').format(str(n), str(answer/n)), 1 )

    def getDivide(self, answer):
        n = random.randint(1, 5)
        return Number( _(u'{0} \u00f7 {1}').format(str(answer*n), str(n)), 1 )

class EqualityLevelset(ExpressionLevelset):
    def __init__(self):
        super(EqualityLevelset, self).__init__()
        self.answermin = 5

    def getTitle(self):
        return _('Equal to %d') % (self.answer,)

    def setLevel(self, level, sublevel):
        self.curlevel = level
        self.cur_sublevel = sublevel
        self.answer = self.answermin + self.cur_sublevel

    def getNumber(self):
        if random.randint(0, 1):
            # correct number
            num = self.getNumberWithAnswer(self.answer)
            num.good = 1
        else:
            # wrong number
            ans = random.choice( range(self.answermin, self.answer) + range(self.answer+1, self.answer*2) )
            num = self.getNumberWithAnswer(ans)
            num.good = 0
        return num

class InequalityLevelset(EqualityLevelset):
    def getTitle(self):
        return _('Not equal to %d') % (self.answer,)

    def getNumber(self):
        num = super(InequalityLevelset, self).getNumber()
        if num.good:
            num.good = 0
        else:
            num.good = 1
        return num

class Player(object):

    def __init__(self):
        self.move_stepnum = 0
        self.x = self.y = self.x_old = self.y_old = -1
        self.moving = self.exists = False
        self.action_start = 0
        self.anim = None
        self.velocity = []

        self.movestep_timer = 0
        self.munch_timer = 0

        # food chain status
        self.foodchain = 0

    # These are defined in Muncher and Troggle
    def spawn(self):
        pass

    def die(self):
        if self.movestep_timer != 0:
            gobject.source_remove(self.movestep_timer)
            self.movestep_timer = 0
        if self.munch_timer != 0:
            gobject.source_remove(self.munch_timer)
            self.munch_timer = 0

    def getEaten(self):
        pass

    def isAt(self, x, y):
        return self.exists and not self.moving and (self.x == x and self.y == y)

    def isNear(self, x, y):
        return self.exists and (    (self.x == x and self.y == y)
                                or  (self.moving and self.x_old == x
                                                 and self.y_old == y)
                               )

    def move_step(self):
        if self.move_stepnum < game.num_moveticks-1:
            self.move_stepnum += 1
            x_old = self.anim.goocanvas.get_property("x")
            y_old = self.anim.goocanvas.get_property("y")
            x = self.anim.goocanvas.get_property("x") + self.velocity[0]*game.sw/game.num_moveticks
            y = self.anim.goocanvas.get_property("y") + self.velocity[1]*game.sh/game.num_moveticks
            ret = True
        else:
            self.move_stepnum = 0
            x = game.sw * self.x + game.left
            y = game.sh * self.y + game.top
            self.stop()
            self.movestep_timer = 0
            ret = False

        self.anim.goocanvas.set_properties(x=x, y=y)
        return ret

    def move(self, x_old, y_old, x, y):
        gcompris.sound.play_ogg("sounds/smudge.wav")
        self.x_old = x_old
        self.y_old = y_old
        self.x = x
        self.y = y
        self.velocity = [x-x_old, y-y_old]
        self.anim.goocanvas.set_properties(x=(self.x_old * game.sw + game.left),
                                           y=(self.y_old * game.sh + game.top))
        self.moving = True

        # it takes game.num_moveticks iterations of duration game.move_tick to move squares
        if x != x_old or y != y_old:
            self.anim.setState(1)
            self.movestep_timer = game.timeout_add(game.move_tick, self.move_step)
        else:
            self.stop()

    def startMunching(self):
        eatfile = gcompris.utils.find_file_absolute("voices/$LOCALE/misc/eat.wav")
        if(os.path.isfile(eatfile)) :
            gcompris.sound.play_ogg("voices/$LOCALE/misc/eat.wav")
        else :
            gcompris.sound.play_ogg("sounds/eat.wav")
        
        self.anim.setState(2)
        self.munch_timer = game.timeout_add(game.munch_time, self.stopMunching)
        return False

    def stopMunching(self):
        self.munch_timer = 0
        self.anim.setState(0)
        return False

    def stop(self):
        self.anim.setState(0)
        self.moving = False

        # work out eating stuff
        for p in game.players:
            if p.isAt(self.x, self.y) and p != self:
                if self.foodchain >= p.foodchain:
                    p.getEaten()
                    self.startMunching()
                else:
                    self.getEaten()
                    p.startMunching()


class Muncher(Player):
    def __init__(self):
        super(Muncher, self).__init__()
        self.lives = 1
        self.anim = gcompris.anim.CanvasItem(game.munchanimation, game.rootitem)
        self.spare = gcompris.anim.CanvasItem(game.munchanimation, game.rootitem)
        self.anim.goocanvas.props.visibility = goocanvas.ITEM_INVISIBLE
        self.key_queue = []
        self.spare.goocanvas.set_properties(x=0, y=0)

    def spawn(self):
        if self.lives >= 1:
            self.spare.goocanvas.props.visibility = goocanvas.ITEM_VISIBLE
        elif self.lives == 0:
            self.spare.goocanvas.props.visibility = goocanvas.ITEM_INVISIBLE
        else:
            game.loseGame()
        self.key_queue = []
        game.hide_message()
        self.exists = True
        self.move(0,0,0,0)
        self.anim.goocanvas.props.visibility = goocanvas.ITEM_VISIBLE

    def die(self):
        super(Muncher, self).die()
        self.lives -= 1
        self.exists = False
        self.anim.goocanvas.props.visibility = goocanvas.ITEM_INVISIBLE
        self.key_queue = []

    def getEaten(self):
        game.show_message( _("You were eaten by a Troggle.\nPress <Return> to continue.") )
        self.die()

    def push_key(self, key):
        if self.exists:
            if self.moving:
                self.key_queue.append(key)
            elif len(self.key_queue) == 0:
                self.handle_key(key)
            else:
                self.key_queue.append(key)
                self.handle_key(self.key_queue.pop(0))
        else:
            if key == gtk.keysyms.Return:
                self.spawn()

    def handle_key(self, key):
        if key == gtk.keysyms.Left:
            if self.x > 0:
                self.move(self.x, self.y, (self.x-1), self.y)
            else:
                self.stop()
        elif key == gtk.keysyms.Right:
            if self.x < game.width - 1:
                self.move(self.x, self.y, (self.x+1), self.y)
            else:
                self.stop()
        elif key == gtk.keysyms.Up:
            if self.y > 0:
                self.move(self.x, self.y, self.x, (self.y-1))
            else:
                self.stop()
        elif key == gtk.keysyms.Down:
            if self.y < game.height - 1:
                self.move(self.x, self.y, self.x, (self.y+1))
            else:
                self.stop()
        elif key == gtk.keysyms.space:
            self.munch()
            if len( self.key_queue ) > 0: # we don't need to wait for munching to finish to start the next action
                self.handle_key( self.key_queue.pop(0) )

    def munch(self):
        num = game.squares[self.x][self.y].num
        if num == None:
            return
        if num.good:
            self.startMunching()
        else:
            game.show_message( _("You ate a wrong number.\n") +game.levelset.getError(num) +
                               _("\nPress <Return> to continue.") )
            self.die()
        game.setNum(self.x, self.y, None)

    def stop(self):
        super(Muncher, self).stop()
        if len(self.key_queue) > 0:
            key = self.key_queue.pop(0)
            self.handle_key(key)


class Troggle(Player):
    def __init__(self):
        super(Troggle, self).__init__()
        self.anim = gcompris.anim.CanvasItem(game.munchanimation, game.rootitem)

        self.nextspawn_timer = 0
        self.nextmove_timer = 0
        self.warn_timer = 0

        self.foodchain = 1

    def spawn(self):
        self.nextspawn_timer = 0
        self.warn_timer = 0
        self.exists = True
        index = random.randint(0, (len( game.troganimation )-1) * game.board.sublevel / game.board.number_of_sublevel)
        self.anim.swapAnimation(game.troganimation[index])
        self.getMove = game.trogmoves[index]
        self.onMove = game.onmove[index]
        self.onStop = game.onstop[index]
        if random.randint(0,1) == 0:
            if random.randint(0,1) == 0:
                self.x_old = -1
                self.x = 0
            else:
                self.x_old = game.width
                self.x = game.width - 1
            self.y = self.y_old = random.randint(0, game.height-1)
        else:
            if random.randint(0,1) == 0:
                self.y_old = -1
                self.y = 0
            else:
                self.y_old = game.height
                self.y = game.height - 1
            self.x = self.x_old = random.randint(0, game.width-1)
        self.move(self.x_old, self.y_old, self.x, self.y)
        self.anim.goocanvas.props.visibility = goocanvas.ITEM_VISIBLE
        game.hide_trogwarning()

    def die(self):
        super(Troggle, self).die()
        self.exists = 0
        self.anim.goocanvas.props.visibility = goocanvas.ITEM_INVISIBLE

        time = game.trog_spawn_time()
        self.nextspawn_timer = game.timeout_add( time + game.trogwarn_time, self.spawn )
        self.warn_timer = game.timeout_add( time, game.show_trogwarning )
        if self.nextmove_timer != 0:
            gobject.source_remove(self.nextmove_timer)
            self.nextmove_timer = 0

    def getEaten(self):
        self.die()

    def move(self, a, b, c, d):
        if self.onMove != None and (a != c or b != d) and game.onBoard(a, b):
            self.x = a
            self.y = b
            self.onMove(self)
        super(Troggle, self).move(a, b, c, d)

    def stop(self):
        self.moving = False
        if self.x < 0 or self.x >= game.width or self.y < 0 or self.y >= game.height:
            self.die()
        else:
            super(Troggle, self).stop()
            self.nextmove_timer = game.timeout_add(game.trog_wait, self.getTrogMove)
            if self.onStop != None:
                self.onStop(self)

    def getTrogMove(self):
        self.nextmove_timer = 0
        x_old = self.x
        y_old = self.y
        x, y = self.getMove(self)
        self.move(x_old, y_old, x, y)

    # the troggle move types
    def trogMove_straight(self):
        x = self.x + (self.x - self.x_old)
        y = self.y + (self.y - self.y_old)
        return x, y

    def trogMove_random(self):
        x = self.x
        y = self.y
        r = random.randint(0,3)
        if r >= 1: # move straight
            x += self.x - self.x_old
            y += self.y - self.y_old
        elif r == 2: # turn left
            x += self.y - self.y_old
            y -= self.x - self.x_old
        else: # turn right
            x -= self.y - self.y_old
            y += self.x - self.x_old
        return x, y

    def trogMove_chase(self):
        x = self.x
        y = self.y
        dx = game.muncher.x - x
        dy = game.muncher.y - y
        if dx == 0 and dy == 0:
            return self.trogMove_straight()
        if not game.muncher.exists or abs(dx) > game.width/2 or abs(dy) > game.height/2:
            return self.trogMove_straight()
        if abs(dx) > abs(dy) or (abs(dx) == abs(dy) and random.randint(0,1) == 0):
            x += dx / abs(dx)
        else:
            y += dy / abs(dy)
        return x, y

    def trogMove_run(self):
        x = self.x
        y = self.y
        dx = game.muncher.x - x
        dy = game.muncher.y - y
        if not game.muncher.exists or abs(dx) > game.width/2 or abs(dy) > game.height/2:
            return self.trogMove_random()
        if abs(dx) > abs(dy) or (abs(dx) == abs(dy) and random.randint(0,1) == 0):
            x -= dx / abs(dx)
        else:
            y -= dy / abs(dy)
        return x, y

    def onMove_create(self):
        game.setNum( self.x, self.y, game.levelset.getNumber() )

    def onStop_munch(self):
        if game.squares[self.x][self.y].num != None:
            self.startMunching()
            game.setNum( self.x, self.y, None )

class Gcompris_gnumch:
    def __init__(self, board):
        global game
        game = self

        self.board = board
        self.board.disable_im_context = True
        self.scrw = gcompris.BOARD_WIDTH
        self.scrh = gcompris.BOARD_HEIGHT
        self.width = 6
        self.height = 6

        self.sw = self.scrw / (self.width + 1)
        self.sh = self.scrh / (self.width + 1)
        self.left = self.scrw - (self.sw * self.width)
        self.top = self.scrh - (self.sh * self.height)

        self.munchanimation = gcompris.anim.Animation("gnumch/muncher.txt")
        self.troganimation = []
        self.trogmoves = [Troggle.trogMove_straight, Troggle.trogMove_random,
                          Troggle.trogMove_run, Troggle.trogMove_random,
                          Troggle.trogMove_chase]
        self.onmove = [ None, Troggle.onMove_create, None, None, None ]
        self.onstop = [ None, None, None, Troggle.onStop_munch, None ]
        for file in [ "gnumch/reggie.txt",
                      "gnumch/diaper.txt",
                      "gnumch/fraidy.txt",
                      "gnumch/eater.txt",
                      "gnumch/smarty.txt" ]:
            self.troganimation.append( gcompris.anim.Animation(file) )

        self.goodies = 0
        self.paused = 0
        self.stopped = 0
        if board.mode == "primes":
            self.levelset = PrimeLevelset()
        elif board.mode == "factors":
            self.levelset = FactorLevelset()
        elif board.mode == "multiples":
            self.levelset = MultipleLevelset()
        elif board.mode == "equality":
            self.levelset = EqualityLevelset()
        elif board.mode == "inequality":
            self.levelset = InequalityLevelset()
        else:
            print "Warning: no levelset type specified, defaulting to primes"
            self.levelset = PrimeLevelset()

        print "Gcompris_gnumch __init__."

        # config options
        self.move_tick = 30
        self.num_moveticks = 10
        self.munch_time = 400
        self.trog_wait = 1000
        self.trogwarn_time = 1000
        self.trogspawn_min = 3000
        self.trogspawn_max = 10000

    def start(self):
        self.board.level = 1
        self.board.maxlevel = self.levelset.numlevels
        self.board.sublevel = 1
        self.board.number_of_sublevel = self.levelset.num_sublevels
        self.trog_wait = 1900

        gcompris.set_default_background(self.board.canvas.get_root_item())
        gcompris.bar_set_level(self.board)

        gcompris.bar_set(gcompris.BAR_LEVEL | gcompris.BAR_REPEAT)

        # create our rootitem. We put each canvas item here so at the end we only
        # need to remove the rootitem
        self.rootitem = goocanvas.Group(
            parent = self.board.canvas.get_root_item())

        # draw the board on top of the background
        for i in range(0,self.width+1):
            goocanvas.Polyline(
                parent = self.rootitem,
                points = goocanvas.Points([(i*self.sw + self.left, self.top),
                                            (i*self.sw + self.left, self.scrh)]),
                fill_color_rgba = 0x000000FFL,
                line_width = 3.0)
        for i in range(0,self.height+1):
            goocanvas.Polyline(
                parent = self.rootitem,
                points = goocanvas.Points([(self.left, self.top + i*self.sh),
                                            (self.scrw, self.top + i*self.sh)]),
                fill_color_rgba = 0x000000FFL,
                line_width = 3.0)

        # munchers and troggles
        self.players = []
        self.muncher = Muncher()
        self.troggles = [Troggle(), Troggle(), Troggle()]

        self.players[:] = self.troggles
        self.players.append(self.muncher)

        # create the squares
        self.squares = []
        for i in range(0, self.width):
            tmp = []
            for j in range(0, self.height):
                s = Square(self.left + self.sw*i + self.sw/2, self.top + self.sh*j + self.sh/2)
                s.pic.raise_(None)
                tmp.append( s )
            self.squares.append(tmp)

        # so that the troggles get clipped to the board area
        goocanvas.Rect(
            parent = self.rootitem,
            x=0, y=0,
            width=self.scrw, height=self.top,
            fill_color_rgba = 0xFFFFFFFFL)
        goocanvas.Rect(
            parent = self.rootitem,
            x=0, y=0,
            width=self.left, height=self.scrh,
            fill_color_rgba = 0xFFFFFFFFL)

        # the board title
        self.title = goocanvas.Text(
            parent = self.rootitem,
            text = "",
            font = gcompris.skin.get_font("gcompris/board/title bold"),
            x = self.sw + 10,
            y = self.top/2)

        # the message
        self.message_back = goocanvas.Rect(
            parent = self.rootitem,
            x=0, y=0, width=1, height=1,
            fill_color_rgba = 0x60F060F0L)
        self.message = goocanvas.Text(
            parent = self.rootitem,
            text = "",
            anchor = gtk.ANCHOR_CENTER,
            font = gcompris.skin.get_font("gcompris/board/big bold"),
            x = self.scrw/2,
            y = self.scrh/2)
        self.hide_message()

        # the trogwarning
        self.trogwarning = goocanvas.Text(
            parent = self.rootitem,
            text = _("T\nR\nO\nG\nG\nL\nE"),
            anchor = gtk.ANCHOR_CENTER,
            font = gcompris.skin.get_font("gcompris/board/title bold"),
            x = self.left/2,
            y = self.scrh/2)
        self.trogwarning.props.visibility = goocanvas.ITEM_INVISIBLE
        self.trogwarning_num = 0

        # the spare life
        self.muncher.spare.goocanvas.raise_(None)

        gcompris.bar_set(0)
        gcompris.bar_location(10, -1, 0.6)

        self.startGame()

    def show_trogwarning(self):
        self.trogwarning_num += 1
        if self.trogwarning_num == 1:
            self.trogwarning.props.visibility = goocanvas.ITEM_VISIBLE

    def hide_trogwarning(self):
        self.trogwarning_num -= 1
        if self.trogwarning_num == 0:
            self.trogwarning.props.visibility = goocanvas.ITEM_INVISIBLE

    def show_message(self, text):
        self.message.set_properties( text = text )
        bounds = self.message.get_bounds()
        gap = 10
        w = bounds.x2 - bounds.x1 + gap * 2
        h = bounds.y2 - bounds.y1 + gap * 2
        self.message_back.set_properties( x = bounds.x1 - gap,
                                          y = bounds.y1 - gap,
                                          width = w + gap,
                                          height = h + gap)
        self.message_back.props.visibility = goocanvas.ITEM_VISIBLE
        self.message.props.visibility = goocanvas.ITEM_VISIBLE

    def hide_message(self):
        self.message.props.visibility = goocanvas.ITEM_INVISIBLE
        self.message_back.props.visibility = goocanvas.ITEM_INVISIBLE

    def set_level(self, level):
        self.board.level = level;
        self.board.sublevel = 1;
        gcompris.bar_set_level(self.board);
        self.trog_wait = 2000 - self.board.level*100
        self.stopGame()
        self.startGame()

    def trog_spawn_time(self):
        return random.randint(self.trogspawn_min, self.trogspawn_max)

    def setNum(self, x, y, new):
        change = 0
        if new == None or not new.good:
            change -= 1
        else:
            change += 1
        if self.squares[x][y].num == None or not self.squares[x][y].num.good:
            change += 1
        else:
            change -= 1

        change /= 2
        self.goodies += change
        if self.goodies == 0:
            self.winGame()
        self.squares[x][y].setNum(new)

    def key_press(self, keyval, commit_str, preedit_str):
        self.muncher.push_key(keyval)
        return True

    def stopGame(self):
        self.stopped = 1
        if self.muncher.munch_timer != 0:
            gobject.source_remove(self.muncher.munch_timer)
        if self.muncher.movestep_timer != 0:
            gobject.source_remove(self.muncher.movestep_timer)

        for t in self.troggles:
            for timer in [t.munch_timer, t.movestep_timer, t.nextmove_timer, t.nextspawn_timer, t.warn_timer]:
                if timer != 0:
                    gobject.source_remove(timer)

    def startGame(self):
        self.stopped = 0
        self.levelset.setLevel(self.board.level, self.board.sublevel)
        self.title.set_properties(text = self.levelset.getTitle())
        self.trogwarning_num = 1
        self.hide_trogwarning()
        self.goodies = 0
        self.won_level = 0
        for col in self.squares:
            for s in col:
                s.setNum( self.levelset.getNumber() )
                if s.num.good:
                    self.goodies += 1

        self.muncher.lives = 1
        for i in range(0, len(self.troggles)):
            if i < self.board.sublevel-1:
                self.troggles[i].die()
            else: # don't move them into the spawning queue
                self.troggles[i].exists = 0
                self.troggles[i].anim.goocanvas.props.visibility = goocanvas.ITEM_INVISIBLE
        self.muncher.spawn()

    def winGame(self):
        self.stopGame()
        self.won_level = 1
        gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)

    def loseGame(self):
        self.stopGame()
        gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.TUX)

    def onBoard(self, x, y):
        return x >= 0 and x < self.width and y >= 0 and y < self.height

    def pause(self, p):
        self.paused = p

        if p == 0:
            if self.won_level:
                # if we are paused, then unpaused it means that they beat the sublevel
                self.increment_level()
            else:
                self.set_level(self.board.level)

    def increment_level(self):
        self.board.sublevel += 1
        if self.board.sublevel > self.board.number_of_sublevel:
            self.set_level( self.board.level % self.board.maxlevel + 1)
        else:
            self.startGame();


    def repeat(self):
        self.stopGame()
        self.startGame()

    def timeout_add(self, t, fn):
        if not self.paused and not self.stopped:
            return gobject.timeout_add(t, fn)
        else:
            return 0

    def end(self):
        for i in range(0, len(self.troggles)):
            self.troggles[i].anim.destroy()
        if self.muncher.anim:
            self.muncher.anim.destroy()
        self.stopGame()
        self.rootitem.remove()

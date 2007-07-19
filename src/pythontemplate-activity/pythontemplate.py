# PythonTemplate Board module
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gnomecanvas

class Gcompris_pythontemplate:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    print "template init"

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = gcomprisBoard.canvas.root().add(
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

  def start(self):
    print "template start"
    gcompris.bar_set(gcompris.BAR_OK|gcompris.BAR_LEVEL)


    pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("gcompris-shapelabel.png"))
    item = self.rootitem.add(
      gnomecanvas.CanvasPixbuf,
      pixbuf = pixmap,
      x=0,
      y=200,
      )
    item.connect("event", self.rmll)

  def rmll(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        print event.x
        print event.y


  def end(self):
    print "template end"


  def ok(self):
    print("template ok.")


  def repeat(self):
    print("template repeat.")


  def config(self):
    print("template config.")


  def key_press(self, keyval, commit_str, preedit_str):
    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    print("Gcompris_pythontemplate key press keyval=%i %s" % (keyval, strn))

  def pause(self, pause):
    print("template pause. %i" % pause)


  def set_level(self, level):
    print("template set level. %i" % level)


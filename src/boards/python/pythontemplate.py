# PythonTemplate Board module
import gtk
import gtk.gdk

class Gcompris_pythontemplate:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    print "template init"

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    print "template start"


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


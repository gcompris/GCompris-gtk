# PythonTemplate Board module

class Gcompris_pythontemplate:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    print "template init"

    #initialisation to default values. Some of them will be replaced by
    #the configured values.

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
    print "template  key_press %i" % keyval)
    pass

  def pause(self, pause):
    print("template pause. %i" % pause)


  def set_level(self, level):
    print("template set level. %i" % level)


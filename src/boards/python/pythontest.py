# PythonTest Board module
import gnome
import gnome.canvas
import gcompris

def img_selected(image):
  print("img selected")
  print(image)

def pythontest_start():  
  """Gcompris board starting function.

  The python equivalent of the C function:
    void start_board(GcomprisBoard* gcomprisBoard);
  There's no parameter given here since the gcomprisBoard structure
  automatically set into a module variable named: 'gcomprisBoard'
  """
  
  gcomprisBoard.level=1
  gcomprisBoard.maxlevel=1
  gcomprisBoard.sublevel=1 
  gcomprisBoard.number_of_sublevel=1
  gcompris.bar_set(0)
  gcompris.set_background(gcomprisBoard.canvas.root(),
			  "gcompris/nur00523.jpg")
  gcompris.bar_set_level(gcomprisBoard)

  gcompris.images_selector_start(gcomprisBoard,
                                 "%s%s" % (gcompris.DATA_DIR, "/dataset/mrpatate.xml"),
                                 img_selected)

  print("pythontest start.")

def pythontest_end():
  print("pythontest end.")

def pythontest_ok():
  print("pythontest ok.")

def pythontest_repeat():
  print("pythontest repeat.")

def pythontest_config():
  print("pythontest config.")

def pythontest_key_press(keyval):  
  print("pythontest key press. %i" % keyval)

def pythontest_pause(pause):  
  print("pythontest pause. %i" % pause)
  
def pythontest_set_level(level):  
  print("pythontest set level. %i" % level)

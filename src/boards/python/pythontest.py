# PythonTest Board module
import gnome
import gnome.canvas
import gcompris
import gcompris.utils

cercle = 0

print dir(gcompris)
print dir(gcompris.utils)

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
			  gcompris.utils.image_to_skin("gcompris-bg.jpg"))
  gcompris.bar_set_level(gcomprisBoard)

  global cercle
  cercle = gcomprisBoard.canvas.root().add(
    gnome.canvas.CanvasEllipse,
    x1=0.0,
    y1=0.0,
    x2=100.0,
    y2=100.0,
    fill_color='red',
    outline_color='white',
    width_units=1.0
    )

  print("pythontest start.")

def pythontest_end():
  """Gcompris board end function.

  The python equivalent of the C function:
    void end_board();
  """
  cercle.destroy()
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

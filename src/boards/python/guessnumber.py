# PythonTest Board module
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gtk
import gtk.gdk
import random
import pango

from gcompris import gcompris_gettext as _

class Gcompris_guessnumber:
  """Tux hide a number, you must guess it"""


  def __init__(self, gcomprisBoard):    
    self.gcomprisBoard = gcomprisBoard

    self.gcomprisBoard.disable_im_context = True
    
    self.indicator = None
    
    print("Gcompris_guessnumber __init__.")

    #initialisation to default values. Some of them will be replaced by
    #the configured values.

  def start(self):
    
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=4
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_reload.png"))
    if(pixmap):
      gcompris.bar_set_repeat_icon(pixmap)
      gcompris.bar_set(gcompris.BAR_OK|gcompris.BAR_LEVEL|gcompris.BAR_REPEAT_ICON)
    else:
      gcompris.bar_set(gcompris.BAR_OK|gcompris.BAR_LEVEL|gcompris.BAR_REPEAT);
    
    gcompris.bar_set_level(self.gcomprisBoard)

    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))


    self.display_game()
    
    print("Gcompris_guessnumber start.")


  def end(self):

    gcompris.reset_locale()
    
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

  def ok(self):
    print("Gcompris_guessnumber ok.")


  def repeat(self):
    print("Gcompris_guessnumber repeat.")


  def config(self):
    print("Gcompris_guessnumber config.")


  def key_press(self, keyval, commit_str, preedit_str):
    print("Gcompris_guessnumber key press. %i %s" % (keyval, str))

    if (keyval == gtk.keysyms.KP_0):
      keyval= gtk.keysyms._0
    if (keyval == gtk.keysyms.KP_1):
      keyval= gtk.keysyms._1
    if (keyval == gtk.keysyms.KP_2):
      keyval= gtk.keysyms._2
    if (keyval == gtk.keysyms.KP_2):
      keyval= gtk.keysyms._2
    if (keyval == gtk.keysyms.KP_3):
      keyval= gtk.keysyms._3
    if (keyval == gtk.keysyms.KP_4):
      keyval= gtk.keysyms._4
    if (keyval == gtk.keysyms.KP_5):
      keyval= gtk.keysyms._5
    if (keyval == gtk.keysyms.KP_6):
      keyval= gtk.keysyms._6
    if (keyval == gtk.keysyms.KP_7):
      keyval= gtk.keysyms._7
    if (keyval == gtk.keysyms.KP_8):
      keyval= gtk.keysyms._8
    if (keyval == gtk.keysyms.KP_9):
      keyval= gtk.keysyms._9

    utf8char = gtk.gdk.keyval_to_unicode(keyval)
    strn = u'%c' % utf8char

    # Return  True  if you did process a key
    # Return  False if you did not processed a key
    #         (gtk need to send it to next widget)
    return True

  def pause(self, pause):
    print("Gcompris_guessnumber pause. %i" % pause)


  def set_level(self, level):
    print("Gcompris_guessnumber set level. %i" % level)

    self.gcomprisBoard.level=level;
    self.gcomprisBoard.sublevel=1;

    # Set the level in the control bar
    gcompris.bar_set_level(self.gcomprisBoard);

    self.cleanup_game()
    self.display_game()
    
# ---- End of Initialisation

  # Display the board game
  def cleanup_game(self):
      # Remove the root item removes all the others inside it
      self.rootitem.destroy()

  # Display the board game
  def display_game(self):

      # Create our rootitem. We put each canvas item in it so at the end we
      # only have to kill it. The canvas deletes all the items it contains automaticaly.
      self.rootitem = self.gcomprisBoard.canvas.root().add(
          gnome.canvas.CanvasGroup,
          x=0.0,
          y=0.0
          )

      min = 1
      max = 10
      if(self.gcomprisBoard.level == 2):
          max = 100
      elif(self.gcomprisBoard.level == 3):
          max = 500
      elif(self.gcomprisBoard.level == 4):
          max = 1000

      # Find a number game
      self.solution = random.randint(min, max)
      print self.solution
    
      text = "Guess a number between %d and %d" %(min, max)
      self.rootitem.add(
          gnome.canvas.CanvasText,
          x=400.0,
          y=100.0,
          font=gcompris.skin.get_font("gcompris/title"),
          text=(text),
          fill_color="black",
          justification=gtk.JUSTIFY_CENTER
          )

      self.indicator = self.rootitem.add(
          gnome.canvas.CanvasText,
          x=400.0,
          y=300.0,
          font=gcompris.skin.get_font("gcompris/subtitle"),
          text=(""),
          fill_color="black",
          justification=gtk.JUSTIFY_CENTER
          )

      self.entry_text()

  def entry_text(self):
    print "__entry__"
    entry = gtk.Entry()

    entry.modify_font(pango.FontDescription("sans bold 36"))
    text_color = gtk.gdk.color_parse("white")
    text_color_selected = gtk.gdk.color_parse("green")
    bg_color = gtk.gdk.color_parse("blue")

    entry.modify_text(gtk.STATE_NORMAL, text_color)
    entry.modify_text(gtk.STATE_SELECTED, text_color_selected)
    entry.modify_base(gtk.STATE_NORMAL, bg_color)

    entry.set_max_length(4)
    entry.connect("activate", self.enter_callback)
    entry.connect("changed", self.enter_char_callback)

    entry.show()

    self.widget = self.rootitem.add(
      gnome.canvas.CanvasWidget,
      widget=entry,
      x=400,
      y=200,
      width=400,
      height=72,
      anchor=gtk.ANCHOR_CENTER,
      size_pixels=False
      )

    self.widget.raise_to_top()

    # does not work. Why ?
    #self.gcomprisBoard.canvas.grab_focus()
    self.widget.grab_focus()
    entry.grab_focus()
   
  def enter_char_callback(self, widget):
      text = widget.get_text()
      widget.set_text(text.decode('utf8').upper().encode('utf8'))
    
  def enter_callback(self, widget):
    text = widget.get_text()

    # Find a number game
    if str(self.solution) == text:
        self.indicator.set(text=_("You won"))
    else:
        if(int(text) > self.solution):
            self.indicator.set(text=_("Too high"))
        else:
            self.indicator.set(text=_("Too low"))

    widget.set_text('')



  ###################################################
  # Configuration system
  ###################################################
  
  #mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    pass



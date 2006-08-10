# Oscar_and_friends Board module
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gtk
import gtk.gdk
import random

from gcompris import gcompris_gettext as _

class Gcompris_oscar_and_friends:
  """Testing gcompris python class"""


  def __init__(self, gcomprisBoard):    
    self.gcomprisBoard = gcomprisBoard
    self.next_item = None
    self.back_item = None
    self.text_item = None

    self.current_texts = None
    self.current_texts_index = 0
    
    #initialisation to default values. Some of them will be replaced by
    #the configured values.

  def start(self):
    print("Gcompris_oscar_and_friends start.")

    gcompris.bar_set(0)
    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.display_book_control()
    self.display_scene_1_0()


  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

  def ok(self):
    print("Gcompris_oscar_and_friends ok.")


  def repeat(self):
    print("Gcompris_oscar_and_friends repeat.")


  def config(self):
    print("Gcompris_oscar_and_friends config.")


  def key_press(self, keyval, commit_str, preedit_str):
    print("Gcompris_oscar_and_friends key press. %i %s" % (keyval, str))

    # Return  True  if you did process a key
    # Return  False if you did not processed a key
    #         (gtk need to send it to next widget)
    return True

  def pause(self, pause):
    print("Gcompris_oscar_and_friends pause. %i" % pause)


  def set_level(self, level):
    print("Gcompris_oscar_and_friends set level. %i" % level)

  # ---- End of Initialisation

  def display_book_control(self):
    self.back_item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_backward.png")),
      x = gcompris.BOARD_WIDTH - 100,
      y = gcompris.BOARD_HEIGHT - 40
      )
    self.back_item.hide()
    self.back_item.connect("event", gcompris.utils.item_event_focus)
    self.back_item.connect("event", self.back_item_event)

    self.next_item = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_forward.png")),
      x = gcompris.BOARD_WIDTH - 50,
      y = gcompris.BOARD_HEIGHT - 40
      )
    self.next_item.hide()
    self.next_item.connect("event", gcompris.utils.item_event_focus)
    self.next_item.connect("event", self.next_item_event)

    self.text_item = self.rootitem.add(
      gnome.canvas.CanvasText,
      x=400.0,
      y=gcompris.BOARD_HEIGHT - 40.0,
      text="",
      font=gcompris.skin.get_font("gcompris/content"),
      fill_color="black",
      justification=gtk.JUSTIFY_CENTER
      )
    self.text_item.hide()
    
  def display_scene_1_0(self):
      gcompris.set_background(self.gcomprisBoard.canvas.root(),
                              "oscar_and_friends/scene1_0.png")

      self.display_texts_init(
        [
        _("Below the surface, Oscar the octopus and his friends sat on a rock,\nwatching the bad weather above them."),
        _("They were all happy that the storms never reached all the way down to them."),
        _("No matter how bad it was up there, the bottom was always calm and quiet."),
        _("Oscar's best friends were Charlie the crab and Sandy the sea-star.")
        ]
        )


  def display_texts_init(self, texts):
    self.current_texts = texts
    self.current_texts_index = 0
    self.text_item.set(text=self.current_texts[self.current_texts_index])
    self.display_back_previous_buttons()
    self.text_item.show()

    
  def display_next_text(self):
    self.current_texts_index += 1
    if(self.current_texts_index < len(self.current_texts)):
      self.text_item.set(text=self.current_texts[self.current_texts_index])
      self.display_back_previous_buttons()
    
  def display_previous_text(self):
    self.current_texts_index -= 1
    if(self.current_texts_index >= 0):
      self.text_item.set(text=self.current_texts[self.current_texts_index])
      self.display_back_previous_buttons()

  def display_back_previous_buttons(self):
    if(self.current_texts_index>0):
      self.back_item.show()
    else:
      self.back_item.hide()

    if(self.current_texts_index<len(self.current_texts)-1):
      self.next_item.show()
    else:
      self.next_item.hide()

  def back_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        self.display_previous_text()
        return True
    return False

  def next_item_event(self, widget, event=None):
    if event.type == gtk.gdk.BUTTON_PRESS:
      if event.button == 1:
        print self.current_texts_index
        self.display_next_text()
        print self.current_texts_index
        print "---"
        return True
    return False


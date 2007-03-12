# MyPaint Board module
import gnomecanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gtk
import gtk.gdk
import random
import os

from gcompris import gcompris_gettext as _

class Gcompris_mypaint:
  """MyPaint draw board"""

  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    gcompris.bar_set(gcompris.BAR_CONFIG)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.skin.image_to_skin("gcompris-bg.jpg"))
    gcompris.bar_set_level(self.gcomprisBoard)

    import sys, os
    sys.path.insert(0, os.path.join(gcompris.get_properties().package_plugin_dir , 'pythonlib'))
    sys.path.append(os.path.join(gcompris.get_properties().package_python_plugin_dir , 'mypaint'))

    self.init_brushes(simple=True)
    self.init_mydrawwidget(simple=True)
    self.init_brushes(simple=True)
    #self.init_colorselection(simple=True)

  def end(self):
    self.mdw.destroy()
    self.color_selector.destroy()
    self.brush_selector.destroy()

  def ok(self):
    pass
    
  def config(self):
    print("Gcompris_pythontest config.")


  def key_press(self, keyval, commit_str, preedit_str):
    print("Gcompris_pythontest key press. %i %s %s" % (keyval, commit_str, preedit_str))

  def pause(self, pause):
    pass

  ###################################################
  # Configuration system
  ###################################################

  #mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    # keep profile in mind
    # profile can be Py_None
    self.configuring_profile = profile


  ##############################################
  # init_mydrawwidget
  ##############################################
  def init_mydrawwidget(self, simple=True):
    
    import infinitemydrawwidget

    self.mdw = infinitemydrawwidget.InfiniteMyDrawWidget()
    self.mdw.allow_dragging(not simple)
    self.mdw.clear()
    #self.mdw.set_brush(self.app.brush)

    self.mdw.resize_if_needed(size=(600 ,400))

    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnomecanvas.CanvasGroup,
      x=0.0,
      y=0.0
      )

    self.rootitem.add(
      gnomecanvas.CanvasWidget,
      widget=self.mdw,
      x=150.0,
      y=0.0,
      width=650,
      height=500,
      size_pixels=True
      )


  def init_brushes(self, simple=True):
    import brush
    
    self.stock_brushpath = gcompris.DATA_DIR + '/mypaint/brushes/'
    self.user_brushpath = None
    
    self.brush = brush.Brush(self)
    self.brushes = []
    self.selected_brush = None
    #self.brush_selected_callbacks = [self.brush_selected_cb]
    self.contexts = []
    for i in range(10):
      c = brush.Brush(self)
      c.name = 'context%02d' % i
      self.contexts.append(c)
      self.selected_context = None

      # find all brush names to load
      deleted = []
      if self.user_brushpath:
        filename = self.user_brushpath + 'deleted.conf'
      else:
        filename=None
        
      if filename and os.path.exists(filename): 
        for name in open(filename).readlines():
          deleted.append(name.strip())

      def listbrushes(path):
        if path:
          return [filename[:-4] for filename in os.listdir(path) if filename.endswith('.myb')]
        else:
          return []
        
      stock_names = listbrushes(self.stock_brushpath)
      user_names =  listbrushes(self.user_brushpath)
      stock_names = [name for name in stock_names if name not in deleted and name not in user_names]
      loadnames_unsorted = user_names + stock_names
      loadnames_sorted = []

      # sort them
      for path in [self.user_brushpath, self.stock_brushpath]:
        if not path:
          continue
        filename = path + 'order.conf'
        if not os.path.exists(filename): continue
        for name in open(filename).readlines():
          name = name.strip()
          if name in loadnames_sorted: continue
          if name not in loadnames_unsorted: continue
          loadnames_unsorted.remove(name)
          loadnames_sorted.append(name)

      if len(loadnames_unsorted) > 3: 
        # many new brushes, do not disturb user's order
        loadnames = loadnames_sorted + loadnames_unsorted
      else:
        loadnames = loadnames_unsorted + loadnames_sorted

      for name in loadnames:
        # load brushes from disk
        b = brush.Brush(self)
        b.load(name)
        if name.startswith('context'):
          i = int(name[-2:])
          assert i >= 0 and i < 10 # 10 for now...
          self.contexts[i] = b
        else:
          self.brushes.append(b)

      if self.brushes:
        self.select_brush(self.brushes[0])

      self.brush.set_color((0, 0, 0))

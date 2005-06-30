
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
import gobject
from gettext import gettext as _

# Database
from pysqlite2 import dbapi2 as sqlite

import module
import user_list
import class_list

class Users(module.Module):
  """Administrating GCompris Users"""


  def __init__(self, canvas):
      print("Gcompris_administration __init__ users panel.")
      module.Module.__init__(self, canvas, "users", _("Users list"))


  def start(self, area):
      print "starting users panel"

      # Connect to our database
      self.con = sqlite.connect(gcompris.get_database())
      self.cur = self.con.cursor()
        
      # Create our rootitem. We put each canvas item in it so at the end we
      # only have to kill it. The canvas deletes all the items it contains automaticaly.
      self.rootitem = self.canvas.add(
          gnome.canvas.CanvasGroup,
          x=0.0,
          y=0.0
          )

      # Call our parent start
      module.Module.start(self)

      hgap = 20
      vgap = 15

      # Define the area percent for each list
      class_percent = 0.45
      user_percent = 1.0-class_percent

      origin_y = area[1]+vgap
      
      class_height = (area[3]-area[1])*class_percent - vgap*2
      list_area = ( area[0], origin_y, area[2], class_height)
      class_list.Class_list(self.rootitem,
                            self.con, self.cur,
                            list_area, hgap, vgap)

      origin_y =  origin_y + class_height + vgap
      user_height =  origin_y + (area[3]-area[1])*user_percent - vgap*2
      list_area = ( area[0], origin_y, area[2], user_height)
      user_list.User_list(self.rootitem,
                          self.con, self.cur,
                          list_area, hgap, vgap)



  def stop(self):
    print "stopping profiles panel"
    module.Module.stop(self)
    
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

    # Close the database
    self.cur.close()
    self.con.close()



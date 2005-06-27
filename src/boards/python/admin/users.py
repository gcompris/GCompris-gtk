
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

# User Management
(
  COLUMN_USERID,
  COLUMN_LOGIN,
  COLUMN_FIRSTNAME,
  COLUMN_LASTNAME,
  COLUMN_BIRTHDATE,
  COLUMN_CLASS,
  COLUMN_USER_EDITABLE
) = range(7)

# Class Management
(
  COLUMN_CLASSID,
  COLUMN_NAME,
  COLUMN_TEACHER,
  COLUMN_CLASS_EDITABLE
) = range(4)

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

      # ---------------
      # User Management
      # ---------------

      # Grab the user data
      self.cur.execute('select * from users')
      self.user_data = self.cur.fetchall()

      # Create the table
      sw = gtk.ScrolledWindow()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

      # create tree model
      model = self.__create_model()

      # create tree view
      treeview = gtk.TreeView(model)
      treeview.set_rules_hint(True)
      treeview.set_search_column(COLUMN_FIRSTNAME)

      sw.add(treeview)

      # Some constants for the layout
      hgap = 20
      vgap = 15
      origin_y = area[1]+vgap
      but_height = 40
      but_width  = 120
            
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=sw,
        x=area[0] + hgap,
        y=area[1] + origin_y,
        width=area[2]-area[0]-hgap*2-but_width,
        height=(area[3]-area[1])/2 - vgap,
        anchor=gtk.ANCHOR_NW,
        size_pixels=gtk.FALSE)
      sw.show()
      treeview.show()

      # add columns to the tree view
      self.__add_columns(treeview)

      # Add buttons

      button_add = gtk.Button(_("Add user"))
      button_add.connect("clicked", self.on_add_item_clicked, model)
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=button_add,
        x=area[2]-but_width,
        y=area[1]+ origin_y,
        width=100,
        height=but_height,
        anchor=gtk.ANCHOR_NW,
        size_pixels=gtk.FALSE)
      button_add.show()
      
      button_rem = gtk.Button(_("Remove user"))
      button_rem.connect("clicked", self.on_remove_item_clicked, treeview)
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=button_rem,
        x=area[2]-but_width,
        y=area[1]+ origin_y + but_height + vgap,
        width=100,
        height=but_height,
        anchor=gtk.ANCHOR_NW,
        size_pixels=gtk.FALSE)
      button_rem.show()

      button_imp = gtk.Button(_("Import file"))
      button_imp.connect("clicked", self.on_import_cvs_clicked, treeview)
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=button_imp,
        x=area[2]-but_width,
        y=area[1]+ origin_y + (but_height + vgap) * 2,
        width=100,
        height=but_height,
        anchor=gtk.ANCHOR_NW,
        size_pixels=gtk.FALSE)
      button_imp.show()

      # ---------------
      # Class Management
      # ---------------

      # Grab the class data
      self.cur.execute('select * from class')
      self.class_data = self.cur.fetchall()

      # Create the table
      sw = gtk.ScrolledWindow()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

      # create tree model
      model = self.__create_model_class()

      # create tree view
      treeview_class = gtk.TreeView(model)
      treeview_class.set_rules_hint(True)
      treeview_class.set_search_column(COLUMN_NAME)

      sw.add(treeview_class)

      # Some constants for the layout
      origin_y = (area[3]-area[1])/2+vgap*2
            
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=sw,
        x=area[0] + hgap,
        y=area[1] + origin_y,
        width=area[2]-area[0]-hgap*2-but_width,
        height=(area[3]-area[1])/2 - vgap*3,
        anchor=gtk.ANCHOR_NW,
        size_pixels=gtk.FALSE)
      sw.show()
      treeview_class.show()

      # add columns to the tree view
      self.__add_columns_class(treeview_class)

      # Add buttons

      button_add = gtk.Button(_("Add class"))
      button_add.connect("clicked", self.on_add_class_clicked, model)
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=button_add,
        x=area[2]-but_width,
        y=area[1]+ origin_y,
        width=100,
        height=but_height,
        anchor=gtk.ANCHOR_NW,
        size_pixels=gtk.FALSE)
      button_add.show()
      
      button_rem = gtk.Button(_("Remove class"))
      button_rem.connect("clicked", self.on_remove_class_clicked, treeview_class)
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=button_rem,
        x=area[2]-but_width,
        y=area[1]+ origin_y + but_height + vgap,
        width=100,
        height=but_height,
        anchor=gtk.ANCHOR_NW,
        size_pixels=gtk.FALSE)
      button_rem.show()


  def stop(self):
    print "stopping profiles panel"
    module.Module.stop(self)
    
    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

    # Close the database
    self.cur.close()
    self.con.close()


  # -------------------
  # User Management
  # -------------------

  def __create_model(self):
    model = gtk.ListStore(
      gobject.TYPE_INT,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_INT,
      gobject.TYPE_BOOLEAN)

    for item in self.user_data:
      iter = model.append()
      model.set(iter,
                 COLUMN_USERID,    item[COLUMN_USERID],
                 COLUMN_LOGIN,     item[COLUMN_LOGIN],
                 COLUMN_FIRSTNAME, item[COLUMN_FIRSTNAME],
                 COLUMN_LASTNAME,  item[COLUMN_LASTNAME],
                 COLUMN_BIRTHDATE, item[COLUMN_BIRTHDATE],
                 COLUMN_CLASS,     item[COLUMN_CLASS],
                 COLUMN_USER_EDITABLE,  True)
    return model


  def __add_columns(self, treeview):
    
    model = treeview.get_model()

    # columns for login
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_edited, model)
    renderer.set_data("column", COLUMN_LOGIN)
    column = gtk.TreeViewColumn(_('Login'), renderer,
                                text=COLUMN_LOGIN,
                                editable=COLUMN_USER_EDITABLE)
    column.set_sort_column_id(COLUMN_LOGIN)
    treeview.append_column(column)

    # columns for first name
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_edited, model)
    renderer.set_data("column", COLUMN_FIRSTNAME)
    column = gtk.TreeViewColumn(_('First Name'), renderer,
                                text=COLUMN_FIRSTNAME,
                                editable=COLUMN_USER_EDITABLE)
    column.set_sort_column_id(COLUMN_FIRSTNAME)
    treeview.append_column(column)

    # column for last name
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_edited, model)
    renderer.set_data("column", COLUMN_LASTNAME)
    column = gtk.TreeViewColumn(_('Last Name'), renderer,
                                text=COLUMN_LASTNAME,
                                editable=COLUMN_USER_EDITABLE)
    column.set_sort_column_id(COLUMN_LASTNAME)
    treeview.append_column(column)

    # column for birth date
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_edited, model)
    renderer.set_data("column", COLUMN_BIRTHDATE)
    column = gtk.TreeViewColumn(_('Birth Date'), renderer,
                                text=COLUMN_BIRTHDATE,
                                editable=COLUMN_USER_EDITABLE)
    column.set_sort_column_id(COLUMN_BIRTHDATE)
    treeview.append_column(column)

    # column for class
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_edited, model)
    renderer.set_data("column", COLUMN_CLASS)
    column = gtk.TreeViewColumn(_('Class'), renderer,
                                text=COLUMN_CLASS,
                                editable=COLUMN_USER_EDITABLE)
    column.set_sort_column_id(COLUMN_CLASS)
    treeview.append_column(column)


  # Return the next user id
  def get_next_user_id(self):
    self.cur.execute('select max(user_id) from users')
    user_id = self.cur.fetchone()[0]
    if(user_id == None):
      user_id=0
    else:
      user_id += 1
      
    return user_id


  # Add user in the model
  def add_user_in_model(self, model, user):
    iter = model.append()
    model.set (iter,
               COLUMN_USERID,    user[COLUMN_USERID],
               COLUMN_LOGIN,     user[COLUMN_LOGIN],
               COLUMN_FIRSTNAME, user[COLUMN_FIRSTNAME],
               COLUMN_LASTNAME,  user[COLUMN_LASTNAME],
               COLUMN_BIRTHDATE, user[COLUMN_BIRTHDATE],
               COLUMN_CLASS,     user[COLUMN_CLASS],
               COLUMN_USER_EDITABLE,  True
               )

    
  #
  def on_add_item_clicked(self, button, model):
    user_id = self.get_next_user_id()

    new_user = [user_id, "?", "?", "?", "?", 0]
    self.add_user_in_model(model, new_user)


  def on_remove_item_clicked(self, button, treeview):

    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      user_id = model.get_value(iter, COLUMN_USERID)
      model.remove(iter)
      # Remove it from the base
      print "Deleting user_id=" + str(user_id)
      self.cur.execute('delete from users where user_id=?', (user_id,))
      self.con.commit()


  def on_import_cvs_clicked(self, button, treeview):

    model = treeview.get_model()
    
    dialog = gtk.FileChooserDialog("Open..",
                                   None,
                                   gtk.FILE_CHOOSER_ACTION_OPEN,
                                   (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                    gtk.STOCK_OPEN, gtk.RESPONSE_OK))
    dialog.set_default_response(gtk.RESPONSE_OK)
    
    filter = gtk.FileFilter()
    filter.set_name("All files")
    filter.add_pattern("*")
    dialog.add_filter(filter)

    response = dialog.run()
    if response == gtk.RESPONSE_OK:
      filename = dialog.get_filename()
    elif response == gtk.RESPONSE_CANCEL:
      dialog.destroy()
      return
    
    dialog.destroy()

    # Parse the file and include each line in the user table
    file = open(filename, 'r')

    # Determine the separator (the most used in the set ,;:)
    # Warning, the input file must use the same separator on each line and
    # between each fields
    line = file.readline()
    file.seek(0)
    sep=''
    count=0
    for asep in [',', ';', ':']:
      c = line.count(asep)
      if(c>count):
        count=c
        sep=asep

    for line in file.readlines():
      print line
      line = line.rstrip("\n\r")
      user_id = self.get_next_user_id()
      login, firstname, lastname, birthdate = line.split(sep)
      # Save the changes in the base
      new_user = [user_id, login, firstname, lastname, birthdate, 0]
      self.add_user_in_model(model, new_user)
      self.cur.execute('insert or replace into users (user_id, login, name, firstname, birthdate, class_id) values (?, ?, ?, ?, ?, ?)', new_user)
      self.con.commit()

    file.close()

  def on_cell_edited(self, cell, path_string, new_text, model):

    iter = model.get_iter_from_string(path_string)
    path = model.get_path(iter)[0]
    column = cell.get_data("column")

    user_id = model.get_value(iter, COLUMN_USERID)
    
    if column == COLUMN_LOGIN:
      model.set(iter, column, new_text)

    elif column == COLUMN_FIRSTNAME:
      model.set(iter, column, new_text)

    elif column == COLUMN_LASTNAME:
      model.set(iter, column, new_text)

    elif column == COLUMN_BIRTHDATE:
      model.set(iter, column, new_text)

    user_data = (user_id,
                 model.get_value(iter, COLUMN_LOGIN),
                 model.get_value(iter, COLUMN_FIRSTNAME),
                 model.get_value(iter, COLUMN_LASTNAME),
                 model.get_value(iter, COLUMN_BIRTHDATE),
                 model.get_value(iter, COLUMN_CLASS))

    # Save the changes in the base
    self.cur.execute('insert or replace into users (user_id, login, name, firstname, birthdate, class_id) values (?, ?, ?, ?, ?, ?)',
                     user_data)
    self.con.commit()


  # -------------------
  # Class Management
  # -------------------

  def __create_model_class(self):
    model = gtk.ListStore(
      gobject.TYPE_INT,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_BOOLEAN)

    for item in self.class_data:
      iter = model.append()
      model.set(iter,
                 COLUMN_CLASSID,   item[COLUMN_CLASSID],
                 COLUMN_NAME,      item[COLUMN_NAME],
                 COLUMN_TEACHER,   item[COLUMN_TEACHER],
                 COLUMN_CLASS_EDITABLE,  True)
    return model


  def __add_columns_class(self, treeview):
    
    model = treeview.get_model()

    # columns for name
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_class_edited, model)
    renderer.set_data("column", COLUMN_NAME)
    column = gtk.TreeViewColumn(_('Name'), renderer,
                                text=COLUMN_NAME,
                                editable=COLUMN_CLASS_EDITABLE)
    column.set_sort_column_id(COLUMN_NAME)
    treeview.append_column(column)

    # columns for first name
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_class_edited, model)
    renderer.set_data("column", COLUMN_TEACHER)
    column = gtk.TreeViewColumn(_('Teacher'), renderer,
                                text=COLUMN_TEACHER,
                                editable=COLUMN_CLASS_EDITABLE)
    column.set_sort_column_id(COLUMN_TEACHER)
    treeview.append_column(column)


  # Return the next class id
  def get_next_class_id(self):
    self.cur.execute('select max(class_id) from class')
    class_id = self.cur.fetchone()[0]
    if(class_id == None):
      class_id=0
    else:
      class_id += 1
      
    return class_id


  # Add class in the model
  def add_class_in_model(self, model, aclass):
    iter = model.append()
    model.set (iter,
               COLUMN_CLASSID,    aclass[COLUMN_CLASSID],
               COLUMN_NAME,       aclass[COLUMN_NAME],
               COLUMN_TEACHER,    aclass[COLUMN_TEACHER],
               COLUMN_CLASS_EDITABLE,  True
               )

    
  #
  def on_add_class_clicked(self, button, model):
    class_id = self.get_next_class_id()

    new_class = [class_id, "?", "?", "?", "?", 0]
    self.add_class_in_model(model, new_class)


  def on_remove_class_clicked(self, button, treeview):

    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      class_id = model.get_value(iter, COLUMN_CLASSID)
      model.remove(iter)
      # Remove it from the base
      print "Deleting class_id=" + str(class_id)
      self.cur.execute('delete from class where class_id=?', (class_id,))
      self.con.commit()


  def on_cell_class_edited(self, cell, path_string, new_text, model):

    iter = model.get_iter_from_string(path_string)
    path = model.get_path(iter)[0]
    column = cell.get_data("column")

    class_id = model.get_value(iter, COLUMN_CLASSID)
    
    if column == COLUMN_NAME:
      model.set(iter, column, new_text)

    elif column == COLUMN_TEACHER:
      model.set(iter, column, new_text)

    class_data = (class_id,
                  model.get_value(iter, COLUMN_NAME),
                  model.get_value(iter, COLUMN_TEACHER))
    # Save the changes in the base
    self.cur.execute('insert or replace into class (class_id, name, teacher) values (?, ?, ?)', class_data)
    self.con.commit()

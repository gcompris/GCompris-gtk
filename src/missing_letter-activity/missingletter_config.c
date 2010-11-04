#include "gcompris/gcompris.h"
#include <string.h>

#include "missingletter.h"


typedef struct
  {
    GtkComboBox *combo_level;
    GtkTreeView *view;

    GtkFileChooserButton *pixmap;
    GtkEntry *question, *answer, *choice;
    gboolean changed;
  } _config_missing;

enum
  {
    QUESTION_COLUMN,
    ANSWER_COLUMN,
    CHOICE_COLUMN,
    PIXMAP_COLUMN,
    PIXBUF_COLUMN,
    N_COLUMNS
  };

#define ICON_SIZE 32

static void new_clicked(GtkButton *b, gpointer data)
{
  _config_missing *u = (_config_missing*)data;
  GtkListStore *ls;
  GtkTreeIter iter;

  ls = GTK_LIST_STORE(gtk_tree_view_get_model(u->view));
  gtk_list_store_append(ls, &iter);
  gtk_list_store_set(ls, &iter,
                     QUESTION_COLUMN, "",
                     ANSWER_COLUMN, "",
                     CHOICE_COLUMN, "",
                     PIXMAP_COLUMN, "",
                     PIXBUF_COLUMN, NULL,
                     -1);
  GtkTreeSelection* sel = gtk_tree_view_get_selection(u->view);
  gtk_tree_selection_select_iter(sel , &iter);
}

static void delete_clicked(GtkButton *b, gpointer data)
{
  _config_missing *u = (_config_missing*)data;
  GtkTreeSelection *selection = gtk_tree_view_get_selection(u->view);
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
      u->changed = TRUE;
    }
}

static gboolean valid_entry(gchar *question, gchar *answer,
			    gchar *choice, gchar *pixmap)
{
  gboolean result=FALSE;
  gchar **split;

  if(choice && question && answer && pixmap &&
     strlen(choice)==3 && strlen(question) && strlen(answer)
     && strchr(question, '_'))
    {
      split = g_strsplit(question, "_", 2);
      if(g_str_has_prefix(answer, split[0]) &&
         g_str_has_suffix(answer, split[1]) &&
         answer[strlen(split[0])] == choice[0])
        result = TRUE;
      g_strfreev(split);
    }
  return result;
}

static void apply_clicked(GtkButton *b, gpointer data)
{
  _config_missing *u = (_config_missing*)data;
  const gchar *question, *answer, *choice;
  gchar *pixmap, *pixfile;
  GtkTreeSelection *selection = gtk_tree_view_get_selection(u->view);
  GtkTreeModel *model;
  GtkTreeIter iter;

  question = gtk_entry_get_text(u->question);
  answer = gtk_entry_get_text(u->answer);
  choice = gtk_entry_get_text(u->choice);
  pixmap = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(u->pixmap));

  if(valid_entry((gchar*)question, (gchar*) answer, (gchar*)choice, pixmap))
    {
      if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {
          pixfile = gc_cache_import_pixmap(pixmap, "missingletter", 300, 300);
          GdkPixbuf *pixbuf =
	    gdk_pixbuf_new_from_file_at_size(pixmap, ICON_SIZE,
					     ICON_SIZE, NULL);

          gtk_list_store_set(GTK_LIST_STORE(model),&iter,
                             QUESTION_COLUMN, question,
                             ANSWER_COLUMN, answer,
                             CHOICE_COLUMN, choice,
                             PIXMAP_COLUMN, pixfile,
                             PIXBUF_COLUMN, pixbuf,
                             -1);
          u->changed = TRUE;
          g_free(pixfile);
          g_object_unref(pixbuf);
        }
    }
  g_free(pixmap);
}

static void up_clicked(GtkButton *b, gpointer data)
{
  _config_missing *u = (_config_missing*)data;
  GtkTreeSelection *selection = gtk_tree_view_get_selection(u->view);
  GtkTreeIter itera, iterb;
  GtkTreeModel *model;
  GtkTreePath *tpa, *tpb;
  gchar *pa, *pb;

  if (gtk_tree_selection_get_selected (selection, &model, &itera))
    {
      pa = gtk_tree_model_get_string_from_iter(model, &itera);
      tpa = gtk_tree_path_new_from_string(pa);
      tpb = gtk_tree_path_copy(tpa);
      gtk_tree_path_prev(tpb);
      pb = gtk_tree_path_to_string(tpb);
      gtk_tree_model_get_iter_from_string(model, &iterb, pb);
      gtk_list_store_swap(GTK_LIST_STORE(model), &itera, &iterb);
      gtk_tree_path_free(tpa);
      gtk_tree_path_free(tpb);
      g_free(pa);
      g_free(pb);
      u->changed = TRUE;
    }
}

static void down_clicked(GtkButton *b, gpointer data)
{
  _config_missing *u = (_config_missing*)data;
  GtkTreeSelection *selection = gtk_tree_view_get_selection(u->view);
  GtkTreeIter itera, iterb;
  GtkTreeModel *model;
  GtkTreePath *tpa, *tpb;
  gchar *pa, *pb;

  if (gtk_tree_selection_get_selected (selection, &model, &itera))
    {
      pa = gtk_tree_model_get_string_from_iter(model, &itera);
      tpa = gtk_tree_path_new_from_string(pa);
      tpb = gtk_tree_path_copy(tpa);
      gtk_tree_path_next(tpb);
      pb = gtk_tree_path_to_string(tpb);
      if(gtk_tree_model_get_iter_from_string(model, &iterb, pb))
        gtk_list_store_swap(GTK_LIST_STORE(model), &itera, &iterb);
      gtk_tree_path_free(tpa);
      gtk_tree_path_free(tpb);
      g_free(pa);
      g_free(pb);
      u->changed = TRUE;
    }
}

static gboolean _save(GtkTreeModel *model, GtkTreePath *path,
                      GtkTreeIter *iter, gpointer data)
{
  gchar *question, *answer, *choice, *pixmap;
  gchar *tmp = NULL;
  xmlNodePtr root, node;

  gtk_tree_model_get (model, iter,
                      QUESTION_COLUMN, &question,
                      ANSWER_COLUMN, &answer,
                      CHOICE_COLUMN, &choice,
                      PIXMAP_COLUMN, &pixmap,
                      -1);
  if(valid_entry(question, answer, choice, pixmap))
    {
      tmp = g_strdup_printf("%s/%s/%c/%c/%c",
			    answer, question,
			    choice[0], choice[1], choice[2]);

      root =(xmlNodePtr)data;
      node = xmlNewChild(root, NULL, BAD_CAST "Board", NULL);
      xmlNewChild(node, NULL,BAD_CAST "pixmapfile", BAD_CAST pixmap);
      xmlNewChild(node, NULL, BAD_CAST "data", BAD_CAST tmp);
    }
  g_free(tmp);
  g_free(question);
  g_free(answer);
  g_free(choice);
  return FALSE;
}

static void save_clicked(GtkButton *b, gpointer data)
{
  _config_missing *u = (_config_missing*)data;
  GtkTreeModel *model;
  gchar *filename;
  xmlNodePtr root;
  xmlDocPtr doc;
  int level;

  level = gtk_combo_box_get_active(u->combo_level)+1;
  if(level==0)
    return;
  if(! u->changed)
    return;
  model = gtk_tree_view_get_model(u->view);
  doc = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);
  root = xmlNewNode(NULL, BAD_CAST "missing_letter");
  xmlDocSetRootElement(doc,root);

  gtk_tree_model_foreach(model, _save, root);

  filename =
    gc_file_find_absolute_writeable("%s/board%d.xml",
				    gcomprisBoard_missing->boarddir, level);
  if(xmlSaveFormatFileEnc(filename, doc, NULL, 1)<0)
    g_warning("Fail to write %s", filename);
  g_free(filename);
  xmlFreeDoc(doc);
  u->changed = FALSE;
}

static void level_changed(GtkComboBox *combo, gpointer data)
{
  _config_missing *u = (_config_missing*)data;
  GtkListStore *ls;
  GtkTreeIter iter;
  gchar *filename;
  GList *list=NULL, *l;
  int level, result;

  level = gtk_combo_box_get_active(u->combo_level)+1;
  if(level==0)
    return;
  if(u->changed)
    {
      GtkWidget *dialog;

      dialog = gtk_dialog_new_with_buttons("Save changes ?",
                                           NULL,
                                           GTK_DIALOG_MODAL,
                                           GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                           NULL);
      result = gtk_dialog_run(GTK_DIALOG(dialog));
      switch(result)
        {
        case GTK_RESPONSE_ACCEPT:
          save_clicked(NULL, data);
          break;
        default:
          u->changed=FALSE;
          break;
        }
      gtk_widget_destroy (dialog);
    }
  ls = GTK_LIST_STORE(gtk_tree_view_get_model(u->view));
  filename = gc_file_find_absolute("%s/board%d.xml",
                                   gcomprisBoard_missing->boarddir, level);
  missing_read_xml_file(filename,&list);
  g_free(filename);
  gtk_list_store_clear(ls);

  for(l=list; l; l=l->next)
    {
      Board *b = l->data;
      gchar *pixfile = gc_file_find_absolute(b->pixmapfile);
      GdkPixbuf *pixbuf;
      gchar tmp[MAX_PROPOSAL+1];
      int i = 0;

      pixbuf =
	gdk_pixbuf_new_from_file_at_size(pixfile, ICON_SIZE, ICON_SIZE,
					 NULL);

      tmp[0] = '\0';
      while(b->text[i])
	{
	  g_strlcat(tmp, b->text[i], MAX_PROPOSAL);
	  i++;
	}
      gtk_list_store_append(ls, &iter);
      gtk_list_store_set(ls, &iter,
                         QUESTION_COLUMN, b->question,
                         ANSWER_COLUMN, b->answer,
                         CHOICE_COLUMN, tmp,
                         PIXMAP_COLUMN, b->pixmapfile,
                         PIXBUF_COLUMN, pixbuf,
                         -1);
      g_free(pixfile);
      g_object_unref(pixbuf);
    }
  missing_destroy_board_list(list);
}

static void text_changed(GtkWidget *widget, gpointer data)
{
  _config_missing *u = (_config_missing*)data;

  const gchar *question, *answer, *choice;

  question = gtk_entry_get_text(u->question);
  answer = gtk_entry_get_text(u->answer);
  choice = gtk_entry_get_text(u->choice);

  if(widget == (GtkWidget*)u->answer)
    {
      if(g_str_has_prefix(answer,question))
        {
          gtk_entry_set_text(u->question,answer);
        }
    }
  else if(widget ==(GtkWidget*) u->question)
    {
      if(strchr(question, '_'))
        {
          gchar ** split = g_strsplit(question,"_",2);
          if(answer[strlen(split[0])]!= choice[0])
            {
              gchar *tmp;
              tmp = g_new0(gchar, 4);
              tmp[0]= answer[strlen(split[0])];
              gtk_entry_set_text(u->choice, tmp);
              g_free(tmp);
            }
          g_strfreev(split);
        }
    }
}

void selection_changed (GtkTreeSelection *selection,gpointer data)
{
  _config_missing *u = (_config_missing*)data;
  gchar *question, *answer, *choice, *pixmap, *pixfile;
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter,
                          QUESTION_COLUMN, &question,
                          ANSWER_COLUMN, &answer,
                          CHOICE_COLUMN, &choice,
                          PIXMAP_COLUMN, &pixmap,
                          -1);
      gtk_entry_set_text(u->question, question);
      gtk_entry_set_text(u->answer, answer);
      gtk_entry_set_text(u->choice, choice);
      pixfile = gc_file_find_absolute(pixmap);
      gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(u->pixmap), pixfile);

      g_free(question);
      g_free(answer);
      g_free(choice);
      g_free(pixmap);
      g_free(pixfile);
    }
}

void destroy_conf_data(void *not_used, gpointer *data)
{
  g_free(data);
}

static void configure_colummns(GtkTreeView *treeview)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  /* pixbuf column */
  renderer = gtk_cell_renderer_pixbuf_new();
  column = gtk_tree_view_column_new_with_attributes(_("Picture"),
                                                    renderer, "pixbuf", PIXBUF_COLUMN, NULL);
  gtk_tree_view_append_column(treeview, column);

  /* Answer column */
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Answer"),
                                                    renderer, "text", ANSWER_COLUMN, NULL);
  gtk_tree_view_append_column(treeview, column);

  /* Question column */
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Question"),
                                                    renderer, "text", QUESTION_COLUMN, NULL);
  gtk_tree_view_append_column(treeview, column);

  /* Choice column */
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Choice"),
                                                    renderer, "text", CHOICE_COLUMN, NULL);
  gtk_tree_view_append_column(treeview, column);
#if 0
  /* pixmap column (debug only)*/
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("File"),
                                                    renderer, "text", PIXMAP_COLUMN, NULL);
  gtk_tree_view_append_column(treeview, column);
#endif
}

  void config_missing_letter(GcomprisBoardConf *config)
  {
    GtkWidget *frame, *view, *pixmap, *question, *answer, *choice;
    GtkWidget *level, *vbox, *hbox, *label;
    GtkWidget *bbox, *button, *table;
    GtkFileFilter *file_filter;
    _config_missing *conf_data;
    int i;

    conf_data = g_new0(_config_missing,1);

    /* frame */
    frame = gtk_frame_new("");
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(config->main_conf_box), frame, TRUE, TRUE, 8);

    vbox = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(frame), vbox);

    /* hbox */
    hbox = gtk_hbox_new(FALSE, 8);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 8);

    /* combo level */
    label = gtk_label_new(_("Level:"));
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 8);

    level = gtk_combo_box_new_text();
    for(i=1; i< gcomprisBoard_missing->maxlevel; i++)
      {
        gchar *tmp;
        tmp = g_strdup_printf(_("Level %d"), i);
        gtk_combo_box_append_text(GTK_COMBO_BOX(level), tmp);
        g_free(tmp);
      }
    gtk_widget_show(level);
    gtk_box_pack_start(GTK_BOX(hbox), level, FALSE, FALSE, 8);

    /* list view */
    GtkListStore *list = gtk_list_store_new(N_COLUMNS,
                                            G_TYPE_STRING,   /*Question */
                                            G_TYPE_STRING,   /* Answer */
                                            G_TYPE_STRING,   /* Choice */
                                            G_TYPE_STRING,   /* pixmap */
                                            GDK_TYPE_PIXBUF  /* pixbuf */
                                            );

    view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list));
    configure_colummns(GTK_TREE_VIEW(view));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
    gtk_tree_view_set_search_column (GTK_TREE_VIEW (view), ANSWER_COLUMN);
    gtk_widget_set_size_request(view, -1, 200);
    gtk_widget_show(view);

    GtkScrolledWindow *scroll = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL,NULL));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_show(GTK_WIDGET(scroll));
    gtk_container_add(GTK_CONTAINER(scroll), view);

    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(scroll), TRUE, TRUE, 10);

    /* button box */
    bbox = gtk_hbutton_box_new();
    gtk_widget_show(bbox);
    gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 8);
    button = gtk_button_new_from_stock(GTK_STOCK_NEW);
    gtk_widget_show(button);
    gtk_container_add(GTK_CONTAINER(bbox), button);
    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(new_clicked), (gpointer) conf_data);

    button = gtk_button_new_from_stock(GTK_STOCK_DELETE);
    gtk_widget_show(button);
    gtk_container_add(GTK_CONTAINER(bbox), button);
    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(delete_clicked), (gpointer) conf_data);

    button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    gtk_widget_show(button);
    gtk_container_add(GTK_CONTAINER(bbox), button);
    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(apply_clicked), (gpointer) conf_data);

    button = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
    gtk_widget_show(button);
    gtk_container_add(GTK_CONTAINER(bbox), button);
    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(up_clicked), (gpointer) conf_data);

    button = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
    gtk_widget_show(button);
    gtk_container_add(GTK_CONTAINER(bbox), button);
    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(down_clicked), (gpointer) conf_data);

    button = gtk_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_widget_show(button);
    gtk_container_add(GTK_CONTAINER(bbox), button);
    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(save_clicked), (gpointer) conf_data);

    /* table */
    table = gtk_table_new(2, 4, FALSE);
    gtk_widget_show(table);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 8);

    /* answer */
    label = gtk_label_new(_("Answer"));
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);

    answer = gtk_entry_new();
    gtk_widget_show(answer);
    gtk_table_attach_defaults(GTK_TABLE(table), answer, 1, 2, 0, 1);

    /* pixmap */
    label = gtk_label_new(_("Picture"));
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 2, 3, 0, 1);

    pixmap = gtk_file_chooser_button_new(_("Filename:"),
                                         GTK_FILE_CHOOSER_ACTION_OPEN);

    file_filter = gtk_file_filter_new();
    gtk_file_filter_add_pixbuf_formats(file_filter);
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(pixmap), file_filter);
    gtk_widget_show(pixmap);
    gtk_table_attach_defaults(GTK_TABLE(table), pixmap, 3, 4, 0, 1);

    /* question */
    label = gtk_label_new(_("Question"));
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);

    question = gtk_entry_new();
    gtk_widget_show(question);
    gtk_table_attach_defaults(GTK_TABLE(table), question, 1, 2, 1, 2);

    /* choice */
    label = gtk_label_new(_("Choice"));
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 2, 3, 1, 2);

    choice = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(choice), 3);
    gtk_widget_show(choice);
    gtk_table_attach_defaults(GTK_TABLE(table), choice, 3, 4, 1, 2);

    conf_data -> combo_level = GTK_COMBO_BOX(level);
    conf_data -> view = GTK_TREE_VIEW(view);
    conf_data -> pixmap = GTK_FILE_CHOOSER_BUTTON(pixmap);
    conf_data -> question = GTK_ENTRY(question);
    conf_data -> answer = GTK_ENTRY(answer);
    conf_data -> choice = GTK_ENTRY(choice);

    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

    g_signal_connect(G_OBJECT(selection),
                     "changed",
                     G_CALLBACK(selection_changed),
                     (gpointer) conf_data);
    g_signal_connect(G_OBJECT(frame), "destroy",
                     G_CALLBACK(destroy_conf_data), (gpointer) conf_data);
    g_signal_connect(G_OBJECT(level), "changed",
                     G_CALLBACK(level_changed), (gpointer) conf_data);
    g_signal_connect(G_OBJECT(question), "changed",
                     G_CALLBACK(text_changed), (gpointer) conf_data);
    g_signal_connect(G_OBJECT(answer), "changed",
                     G_CALLBACK(text_changed), (gpointer) conf_data);

    gtk_combo_box_set_active(GTK_COMBO_BOX(level), 0);
  }


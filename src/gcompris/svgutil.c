/* gcompris - gameutil.c
 *
 * Time-stamp: <2004/09/04 16:44:32 bcoudoin>
 *
 * Copyright (C) 2004 Yves Combe
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <dirent.h>
#include <math.h>

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

#include "gcompris.h"

#include "assetml.h"

#define IMAGEEXTENSION ".png"

#define MAX_DESCRIPTION_LENGTH 1000

#define MY_ENCODING "utf-8"

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

typedef void (*sighandler_t)(int);

/** 
 * recursive c func to clone GnomeCanvasItem
 * parent is parent for new item
 * item is item to clone 
 */
/*
 * Clone an item.
 * Items that contains the property "anchors" will not be saved.
 * To mark an item use:
 *    gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", TRUE);
 *
 */

void gcompris_clone_item(GnomeCanvasItem *item, GnomeCanvasGroup *parent)
{
  GnomeCanvasItem *cloned;

  GParamSpec **properties;
  guint n_properties;

  int i;
  int *empty = NULL ;
  int *anchors = NULL ;
  char *filename = NULL;

  /* anchors are not copied  */
  anchors = g_object_get_data(G_OBJECT(item), "anchors");
  if (anchors != NULL)
    return;
  
  /* cloned = gcompris_anim_clone_item(parent, item); */
  cloned = gnome_canvas_item_new(parent,
				 G_OBJECT_TYPE(item),
				 NULL);

  /* copy flags. Need to check what must be done EXACTLY */
  GTK_OBJECT_FLAGS(cloned) = GTK_OBJECT_FLAGS(item); 

  /* copy affine matrix */
  if (item->xform) {
    if ( GTK_OBJECT_FLAGS(item) & GNOME_CANVAS_ITEM_AFFINE_FULL ){
      cloned->xform = malloc(sizeof(double)*6);
      for (i=0; i<6; i++)
	cloned->xform[i]=item->xform[i];
    }
    else {
      cloned->xform = malloc(sizeof(double)*2);
      for (i=0; i<2; i++)
	cloned->xform[i]=item->xform[i];
    }
  }
  /* check if this is alresady the case ? */
  else cloned->xform = NULL;

  /* get all the properties and copy them */
  properties = g_object_class_list_properties (G_OBJECT_GET_CLASS(item),
					       &n_properties);

  /* anchors are hidden to play without them */
  anchors = g_object_get_data(G_OBJECT(item), "anchors");
  if (anchors != NULL)
    gnome_canvas_item_hide(cloned);

  empty = g_object_get_data(G_OBJECT(item), "empty");
  if (empty != NULL)
    g_object_set_data(G_OBJECT(cloned), "empty", empty);

  filename = g_object_get_data(G_OBJECT(item), "filename");
  if (filename != NULL)
    g_object_set_data(G_OBJECT(cloned), "filename", filename);

  for (i=0; i< n_properties; i++) {
    GValue property;

    /* fill-* properties are not passed if object is empty. */
    /* Note: strncmp returns 0  on success */
    if (strncmp("parent",properties[i]->name,4))
      if ((strncmp("fill",properties[i]->name,4) || ! empty))
	if ( (properties[i]->flags & G_PARAM_READABLE) && (properties[i]->flags & G_PARAM_WRITABLE ) ){

	  memset(&property, 0, sizeof(property));
	  g_value_init(&property,G_PARAM_SPEC_VALUE_TYPE(properties[i]));
	  
	  g_object_get_property(G_OBJECT(item), properties[i]->name, &property);

	  if (&property){
	    g_object_set_property(G_OBJECT(cloned), properties[i]->name, &property);
	  }
	}
  }

  /* recursively clone items from GnomeCanvasGroup */
  if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_GROUP )
    g_list_foreach(GNOME_CANVAS_GROUP(item)->item_list, (GFunc) gcompris_clone_item, GNOME_CANVAS_GROUP(cloned));

}


/**************
 * SVG saving *
 **************/

/* number of second before begin animation */
#define WAIT_FOR_BEGIN 1


void *gcompris_item_to_svg_file( GnomeCanvasItem *item, xmlNodePtr svgNode);

/* This fonction set transform attributes in svg element as transformation
   matrix from gnome canvas */
void *gcompris_transform_canvas_to_svg_file(GnomeCanvasItem *item, xmlNodePtr svgNode ){
  char tmp[128];
  int i,rc;

  if (item->xform == NULL)
    /* no affine transformation */
    return;
  else if ( GTK_OBJECT_FLAGS(item) & GNOME_CANVAS_ITEM_AFFINE_FULL )
    /* 6 element matrix */
    snprintf(tmp,127,"matrix( %lf, %lf, %lf, %lf, %lf, %lf)",
	     item->xform[0],
	     item->xform[1],
	     item->xform[2],
	     item->xform[3],
	     item->xform[4],
	     item->xform[5]);
  else
    /* 2 elements translation (see gnome canvas doc) */
    snprintf(tmp,127,"translate( %lf, %lf )",
	     item->xform[0],
	     item->xform[1]);

  xmlNewProp( svgNode, 
	      BAD_CAST "transform",
	      BAD_CAST tmp);
}


void *gcompris_re_colors_to_svg_file(GnomeCanvasItem *item,  xmlNodePtr svgNode ){
  int *empty;
  int rc;
  guint intval;
  char tmp[128];
  gchararray fill=NULL;

  //empty = g_object_get_data(G_OBJECT(item), "empty");
  empty = g_object_get_data(G_OBJECT(item), "empty");
  if (empty==NULL) {
    g_object_get(G_OBJECT(item), "fill-color-rgba", &intval, NULL);
    rc = snprintf( tmp, 
		   127,
		   "rgb( %d, %d, %d )", 
		   (intval >> 24)  & 255, (intval >> 16 ) & 255, 
		   ( intval >> 8 ) & 255);
  }
  
    
  xmlNewProp ( svgNode, BAD_CAST "fill",
	       BAD_CAST (empty==NULL ? tmp : "none"));
  
  g_object_get(G_OBJECT(item), "outline-color-rgba", &intval, NULL);
  
  rc = snprintf(tmp, 
		127,"rgb( %d, %d, %d )", 
		(intval >> 24)  & 255, 
		(intval >> 16 ) & 255, 
		( intval >> 8 ) & 255);

  xmlNewProp (svgNode, 
	      BAD_CAST "stroke",
	      BAD_CAST tmp);

  g_object_get ( G_OBJECT(item), "width-pixels", &intval, NULL);
 
  rc = snprintf( tmp,
		 127,
		 "%dpx", intval);

  xmlNewProp( svgNode, 
	      BAD_CAST "stroke-width",
	      BAD_CAST tmp);

}



void *gcompris_group_to_svg_file( GnomeCanvasItem *item, xmlNodePtr svgNode ){
  int rc;
  xmlNodePtr cur;

  cur = xmlNewChild( svgNode,
		     NULL,
		     BAD_CAST "g",
		     NULL );

  gcompris_transform_canvas_to_svg_file( item, cur);

  g_list_foreach(GNOME_CANVAS_GROUP(item)->item_list, (GFunc) gcompris_item_to_svg_file, cur);

}

void *gcompris_rect_to_svg_file( GnomeCanvasItem *item, xmlNodePtr svgNode){
  int rc;
  char tmp[128];
  gdouble x1,y1,x2,y2;
  double x,y,height,width;
  xmlNodePtr cur;

  cur = xmlNewChild( svgNode,
		     NULL,
		     BAD_CAST "rect",
		     NULL);

  gcompris_transform_canvas_to_svg_file( item, cur);

  gcompris_re_colors_to_svg_file(item, cur);

  g_object_get(G_OBJECT(item), "x1", &x1, "y1", &y1, "x2", &x2, "y2", &y2, NULL);
  x = min(x1,x2);
  y = min(y1,y2);
  
  width = max(x1,x2) - x;
  height = max(y1,y2)- y;

  rc = snprintf(tmp, 127,"%lf", x);
  xmlNewProp( cur, 
	      BAD_CAST "x",
	      BAD_CAST tmp);

  rc = snprintf(tmp, 127,"%lf", y);
  xmlNewProp( cur,
	      BAD_CAST "y",
	      BAD_CAST tmp);

  rc = snprintf(tmp, 127,"%lf", height);
  xmlNewProp( cur, 
	      BAD_CAST "height",
	      BAD_CAST tmp);

  rc = snprintf(tmp, 127,"%lf", width);
  xmlNewProp( cur,
	      BAD_CAST "width",
	      BAD_CAST tmp);
  
}

void *gcompris_ellipse_to_svg_file( GnomeCanvasItem *item, xmlNodePtr svgNode ){
  int rc;
  gdouble x1,y1,x2,y2;
  double x,y,xrad,yrad;
  char tmp[128];
  xmlNodePtr cur;

  cur = xmlNewChild( svgNode,
		     NULL,
		     BAD_CAST "ellipse",
		     NULL);

  gcompris_transform_canvas_to_svg_file( item, cur);

  gcompris_re_colors_to_svg_file(item, cur);

  g_object_get(G_OBJECT(item), "x1", &x1, "y1", &y1, "x2", &x2, "y2", &y2, NULL);
  x = (x1 + x2)*.5;
  y = (y1+y2)*.5;
  
  xrad = max(x1,x2) - x;
  yrad = max(y1,y2)- y;

  rc = snprintf(tmp, 127,"%lf", x);
  xmlNewProp( cur, 
	      BAD_CAST "cx",
	      BAD_CAST tmp);

  rc = snprintf(tmp, 127,"%lf", y);
  xmlNewProp( cur,
	      BAD_CAST "cy",
	      BAD_CAST tmp);

  rc = snprintf(tmp, 127,"%lf", xrad);
  xmlNewProp( cur, 
	      BAD_CAST "rx",
	      BAD_CAST tmp);

  rc = snprintf(tmp, 127,"%lf", yrad);
  xmlNewProp( cur,
	      BAD_CAST "ry",
	      BAD_CAST tmp);

}


void *gcompris_line_to_svg_file( GnomeCanvasItem *item, xmlNodePtr svgNode ){
  int rc;
  guint intval;
  char tmp[128];
  gdouble width;
  GnomeCanvasPoints *points;
  xmlNodePtr cur;

  cur = xmlNewChild( svgNode,
		     NULL,
		     BAD_CAST "line",
		     NULL);

  gcompris_transform_canvas_to_svg_file( item, cur);


  g_object_get(G_OBJECT(item), "fill-color-rgba", &intval, NULL);

  rc = snprintf(tmp, 127,"rgb( %d, %d, %d )", (intval >> 24)  & 255, (intval >> 16 ) & 255, ( intval >> 8 ) & 255);
  
  xmlNewProp( cur,
	      BAD_CAST "stroke",
	      BAD_CAST tmp );
  

  g_object_get(G_OBJECT(item), "width-units", &width, NULL);
  
  rc = snprintf(tmp, 127,"%lfpx",width);

  xmlNewProp( cur,
	      BAD_CAST "stroke-width",
	      BAD_CAST tmp);


  g_object_get(G_OBJECT(item), "points", &points, NULL);
  if (points == NULL){
    g_warning("ERROR: LINE points NULL \n");
    return;
  }

  rc = snprintf(tmp, 127,"%lf", points->coords[0]);
  xmlNewProp ( cur,
	       BAD_CAST "x1",
	       BAD_CAST tmp);

  rc = snprintf(tmp, 127,"%lf", points->coords[1]);
  xmlNewProp ( cur,
	       BAD_CAST "y1",
	       BAD_CAST tmp);

  rc = snprintf(tmp, 127,"%lf", points->coords[2]);
  xmlNewProp ( cur,
	       BAD_CAST "x2",
	       BAD_CAST tmp);

  rc = snprintf(tmp, 127,"%lf", points->coords[3]);
  xmlNewProp ( cur,
	       BAD_CAST "y2",
	       BAD_CAST tmp);

}


void *gcompris_text_to_svg_file( GnomeCanvasItem *item, xmlNodePtr svgNode ){
  int rc;
  gchararray text;
  gchararray font;
  double x, y;
  GtkAnchorType anchor;
  guint intval;
  char tmp[128];
  xmlNodePtr cur;

  g_object_get(G_OBJECT(item), "text", &text, NULL);

  cur = xmlNewChild( svgNode,
		     NULL,
		     BAD_CAST "text",
		     BAD_CAST text);

  gcompris_transform_canvas_to_svg_file( item, cur);

  g_object_get(G_OBJECT(item), "font", &font, NULL);
  
  xmlNewProp ( cur,
	       BAD_CAST "font-family",
	       BAD_CAST font);

  g_object_get(G_OBJECT(item), "fill_color_rgba", &intval, NULL);
  
  rc = snprintf( tmp,
		 127,
		 "rgb( %d, %d, %d )", 
		 (intval >> 24)  & 255, (intval >> 16 ) & 255, 
		 ( intval >> 8 ) & 255);

  xmlNewProp ( cur,
	       BAD_CAST "fill",
	       BAD_CAST tmp);

  g_object_get(G_OBJECT(item), "x", &x, "y", &y, NULL);

  rc = snprintf(tmp, 127,"%lf", x);
  xmlNewProp ( cur,
	       BAD_CAST "x",
	       BAD_CAST tmp);

  rc = snprintf(tmp, 127,"%lf", y);
  xmlNewProp ( cur,
	       BAD_CAST "y",
	       BAD_CAST tmp);

  //g_object_get(G_OBJECT(item), "anchor", &anchor, NULL);
  xmlNewProp ( cur,
	       BAD_CAST "text-anchor",
	       BAD_CAST "middle");
  

}

/* This function:
   - check if image is already in <defs/>
   - eventually save image in <defs>, with width and height, x="0", y="0"
   - put use section with x and y (translation).
*/
void *gcompris_pixbuf_to_svg_file( GnomeCanvasItem *item, xmlNodePtr svgNode){
  int rc;
  GdkPixbuf *pixbuf;
  gdouble x, y, width, height;
  gchar *buffer;
  gsize buffer_size;
  xmlNodePtr cur, defs, img, gcomprisPrivateData, gc_img;
  GError **error = NULL;
  char tmp[128], *id, *id_;
  xmlChar *imageName;
  int i;
  char *itemName; 
  xmlTextWriterPtr imageWriter;
  xmlDocPtr imageDoc;
  int pixmap_height, pixmap_width;

  defs = xmlDocGetRootElement( svgNode->doc );

  defs = defs->xmlChildrenNode;

  while ( defs != NULL ){
    if (!xmlStrcmp(defs->name, (const xmlChar *)"defs")){
      break;
    }
    defs = defs->next;
  }

  gcomprisPrivateData = defs->xmlChildrenNode;
  while ( gcomprisPrivateData != NULL ){
    if (!xmlStrcmp(gcomprisPrivateData->name, (const xmlChar *)"gcompris:anim")){
      break;
    }
    gcomprisPrivateData = gcomprisPrivateData->next;
  }

  itemName = g_object_get_data(G_OBJECT(item), "filename");

  i = 0;
  img = gcomprisPrivateData->xmlChildrenNode;
  while ( img != NULL ){
    if (!xmlStrcmp(img->name, (const xmlChar *)"gcompris:image")){
      imageName = xmlGetProp(img, "filename");
      printf("Filename %s : %s \n", imageName, itemName);
      if (!xmlStrcmp((xmlChar*)  itemName, imageName)){
	break;}
      i++;
    }
    img = img->next;
  }


  g_object_get(G_OBJECT(item), "pixbuf", &pixbuf, NULL);
  if (pixbuf == NULL)
    printf("Erreur %s pixbuf null !!!\n", itemName);
  
  pixmap_width = gdk_pixbuf_get_width(pixbuf);
  pixmap_height = gdk_pixbuf_get_height(pixbuf);
  
  
  
  if (img == NULL){
    /* image not found, we save it */
    printf("Pixbuf saving %s \n", itemName);

    /* we create a writer to use Base64 facility */
    imageWriter = xmlNewTextWriterDoc (&imageDoc,0);

    xmlTextWriterStartDocument(imageWriter, NULL, MY_ENCODING, NULL);

    rc = xmlTextWriterStartElement(imageWriter, BAD_CAST "image");
    if (rc < 0) {
      printf
	("gcompris_svg_save: Error at xmlTextWriterStartElement\n");
      return;
    }

    rc = gdk_pixbuf_save_to_buffer ( pixbuf,
				     &buffer,
				     &buffer_size,
				     "png",
				     error, NULL);

    rc = xmlTextWriterStartAttribute(imageWriter, BAD_CAST "base64");

    rc = xmlTextWriterWriteString(imageWriter, BAD_CAST "data:image/png;base64,");

    rc = xmlTextWriterWriteBase64(imageWriter, buffer, 0, buffer_size);

    rc = xmlTextWriterEndAttribute(imageWriter);

    xmlTextWriterEndElement(imageWriter);
    
    xmlTextWriterEndDocument(imageWriter);

    xmlFreeTextWriter(imageWriter);
    /* writer is no more interesting */

    cur = xmlNewChild ( defs,
			NULL,
			BAD_CAST "image",
			NULL);

    gc_img = xmlNewChild ( gcomprisPrivateData,
			   NULL,
			   BAD_CAST "gcompris:image",
			   NULL);


    id = malloc(12*sizeof(char));
    rc = snprintf(id, 10,"image%04d", i);

    xmlNewProp ( cur, 
		 BAD_CAST "id",
		 BAD_CAST id);

    rc = snprintf(id, 11,"#image%04d", i);

    xmlNewProp ( gc_img, 
		 BAD_CAST "xlink:href",
		 BAD_CAST id);

    xmlNewProp( gc_img,
		BAD_CAST "filename",
		BAD_CAST itemName);


    /* x="0", y="0", width and height are passed to image in <defs/> */
    xmlNewProp ( cur,
		 BAD_CAST "x",
		 BAD_CAST "0");

    xmlNewProp ( cur,
		 BAD_CAST "y",
		 BAD_CAST "0");

    rc = snprintf(tmp, 127,"%d", pixmap_width);
    xmlNewProp ( cur,
		 BAD_CAST "width",
		 BAD_CAST tmp);
    
    rc = snprintf(tmp, 127,"%d", pixmap_height);
    xmlNewProp ( cur,
		 BAD_CAST "height",
		 BAD_CAST tmp);


    /* This is the base64 transformed image */
    img = xmlDocGetRootElement( imageDoc );

    buffer = xmlGetProp ( img, 
			  BAD_CAST "base64");

    xmlFreeNode(img);
    img = NULL;

    xmlNewProp ( cur,
		 "xlink:href",
		 BAD_CAST buffer );


  }
  else {
    id = (char *) xmlGetProp(img, "xlink:href");
  
  }

  cur = xmlNewChild ( svgNode,
		      NULL,
		      BAD_CAST "svg",
		      NULL);

    /* transform go to svg */
  gcompris_transform_canvas_to_svg_file( item, cur);

  g_object_get(G_OBJECT(item), "x", &x, "y", &y, NULL);
  g_object_get(G_OBJECT(item), "width", &width, "height", &height, NULL);

    
  /* x and y width and height are passed to <svg/> */
  /* viewBox is 0 0 pixmap_width pixmap_height */
  rc = snprintf(tmp, 127,"%lf", x);
  xmlNewProp ( cur,
	       BAD_CAST "x",
	       BAD_CAST tmp);
    
  rc = snprintf(tmp, 127,"%lf", y);
  xmlNewProp ( cur,
	       BAD_CAST "y",
	       BAD_CAST tmp);
  rc = snprintf(tmp, 127,"%lf", width);
  xmlNewProp ( cur,
	       BAD_CAST "width",
	       BAD_CAST tmp);
    
  rc = snprintf(tmp, 127,"%lf", height);
  xmlNewProp ( cur,
	       BAD_CAST "height",
	       BAD_CAST tmp);

  rc = snprintf(tmp, 127,"0 0 %d %d", pixmap_width, pixmap_height);
  xmlNewProp ( cur,
	       BAD_CAST "viewBox",
	       BAD_CAST tmp);

  xmlNewProp ( cur,
	       BAD_CAST "preserveAspectRatio",
	       BAD_CAST "none");

  cur = xmlNewChild ( cur,
		      NULL,
		      BAD_CAST "use",
		      NULL);

  xmlNewProp ( cur, 
	       BAD_CAST "xlink:href",
	       BAD_CAST id);
    
  if (img == NULL)
    free(id);


}

void *gcompris_item_to_svg_file( GnomeCanvasItem *item, xmlNodePtr svgNode ){

  /* anchors are not saved  */
  if (g_object_get_data(G_OBJECT(item), "anchors") != NULL)
    return;

  if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_GROUP )
    gcompris_group_to_svg_file( item, svgNode );
  else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_RECT )
    gcompris_rect_to_svg_file( item, svgNode );
  else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_ELLIPSE )
    gcompris_ellipse_to_svg_file( item, svgNode );
  else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_LINE )
    gcompris_line_to_svg_file( item, svgNode );
  else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_TEXT )
    gcompris_text_to_svg_file( item, svgNode );
  else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_PIXBUF )
    gcompris_pixbuf_to_svg_file( item, svgNode );

  // printf("");

}

void *gcompris_anim_set( xmlNodePtr svgNode, int begin, int delay, int prev, int total, const char *visible){
  xmlNodePtr cur;
  char tmp[128];
  int rc;
  
  cur = xmlNewChild( svgNode,
		     NULL,
		     "set",
		     NULL);
  
  //xmlNewProp ( cur, "dur", "0s");

  rc = snprintf( tmp, 127, "%d%s",(prev + 1)%total  , visible);
  xmlNewProp ( cur, "id", tmp);

  rc = snprintf( tmp, 127, "%dms;%d%s.begin+%dms", begin, prev, visible, delay);

  xmlNewProp ( cur, "begin", tmp);

  xmlNewProp ( cur, BAD_CAST "attributeName",BAD_CAST "visibility");
  xmlNewProp ( cur, BAD_CAST "attributeType",BAD_CAST "CSS");
  xmlNewProp ( cur, BAD_CAST "to",BAD_CAST visible);
  //xmlNewProp ( cur, BAD_CAST "repeatCount",BAD_CAST "indefinite");
  
}

void *gcompris_anim_to_svg_file( GnomeCanvasItem *item, xmlNodePtr svgNode ){
    xmlNodePtr cur, animation;
    xmlChar *sdelay;
    int delay, nbr, total;
    int time;

    /* svgNode is <svg/>, the root node */
    cur = svgNode->xmlChildrenNode;
    /* find defs */
    while ( cur != NULL ){
      if (!xmlStrcmp(cur->name, (const xmlChar *)"defs")){
	break;
      }
      cur = cur->next;
    }

    /* find "gcompris:anim" */
    cur = cur->xmlChildrenNode;
    while ( cur != NULL ){
      if (!xmlStrcmp(cur->name, (const xmlChar *)"gcompris:anim")){
	break;
      }
      cur = cur->next;
    }

    /* find "gcompris:animation" */
    cur = cur->xmlChildrenNode;
    while ( cur != NULL ){
      if (!xmlStrcmp(cur->name, (const xmlChar *)"gcompris:animation")){
	break;
      }
      cur = cur->next;
    }

    animation = cur;

    /* get the rank of the image in animation */
    nbr = 0;

    cur = svgNode->xmlChildrenNode;
    while ( cur != NULL ){
      if (!xmlStrcmp(cur->name, (const xmlChar *)"g")){
	nbr++;
      }
      cur = cur->next;
    }

    sdelay = xmlGetProp(animation, "delay");
    
    printf("delay lu %s \n",sdelay);
    
    sscanf(sdelay,"%d", &delay);

    sdelay = xmlGetProp(animation, "length");
    
    printf("total lu %s \n",sdelay);
    
    sscanf(sdelay,"%d", &total);

    
    cur = xmlNewChild( svgNode,
		       NULL,
		       BAD_CAST "g",
		       NULL );

    gcompris_transform_canvas_to_svg_file( item, cur);

    xmlNewProp(cur, BAD_CAST "visibility", BAD_CAST "hidden");

    /* warning:  ms for all time value */
    printf("nbr %d total %d prev %d ",nbr , total , nbr == 0 ? nbr + total -1 : nbr-1 );
    gcompris_anim_set ( cur, 
			WAIT_FOR_BEGIN * 1000 + delay*10*nbr, 
			delay*10 ,
			nbr == 0 ? nbr + total -1 : nbr-1,
			total, 
			"visible" );
    gcompris_anim_set ( cur, 
			WAIT_FOR_BEGIN * 1000 + delay*10*(nbr+1), 
			delay*10 ,
			nbr == 0 ? nbr + total -1 : nbr-1,
			total,
			"hidden" );

    g_list_foreach(GNOME_CANVAS_GROUP(item)->item_list, (GFunc) gcompris_item_to_svg_file, cur);

}



/* Parameters: */
/* module : name of calling module. Will be put in desc and comment */
/* file : file where save the Group; */
/* hsize, vsize: size of the canvas. Passed in svg attributs height & width */
/* anim: numbers of ms between animation. 0 means no animations. */

void gcompris_svg_save(char *module, char *file, GnomeCanvasItem *item, int hsize, int vsize, int anim){
    xmlDocPtr svgDoc;
    xmlNodePtr svgNode, cur, com; 
    xmlDtdPtr dtd = NULL;

    int rc;
    //  xmlChar *tmp;
    char tmp[128];
    FILE *fp;

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION


    /* Create a new doc */
    svgDoc = xmlNewDoc("1.0");

    svgNode = xmlNewNode(NULL, BAD_CAST "svg");
    xmlDocSetRootElement(svgDoc, svgNode);

    //dtd = xmlCreateIntSubset(svgDoc, BAD_CAST "svg", NULL, BAD_CAST "svg.dtd");

    rc = snprintf(tmp,127,"%dpx",hsize);
    xmlNewProp(svgNode, BAD_CAST "width", BAD_CAST tmp);

    rc = snprintf(tmp,127,"%dpx",vsize);
    xmlNewProp(svgNode, BAD_CAST "height", BAD_CAST tmp);

    xmlNewProp(svgNode, BAD_CAST "version", BAD_CAST "1.1");
    xmlNewProp(svgNode, BAD_CAST "xmlns", BAD_CAST "http://www.w3.org/2000/svg");
    xmlNewProp(svgNode, BAD_CAST "xmlns:xlink", BAD_CAST "http://www.w3.org/1999/xlink");

    rc = snprintf(tmp, 127, "SVG file created by Gcompris:%s",module);
    com = xmlNewComment(BAD_CAST tmp);
    xmlAddChild(svgNode, com);

    /* defs Node */
    cur = xmlNewChild(svgNode, NULL, BAD_CAST "defs", NULL);

    rc = snprintf(tmp, 127 , "gcompris:%s",module);
    cur = xmlNewChild(cur, NULL, BAD_CAST tmp, NULL);
    xmlNewProp(cur, BAD_CAST "xmlns:gcompris", BAD_CAST "http://www.ofset.org/gcompris");

    rc = snprintf(tmp, 127, "Gcompris:%s private data ",module);
    com = xmlNewComment(BAD_CAST tmp);
    xmlAddChild(cur, com);

    /* animation is set here : */
    /*   every top level group is an image for the animation */

    printf ("Anim %d and list %d \n", anim, g_list_length(GNOME_CANVAS_GROUP(item)->item_list));

    if ( (anim > 0) && (g_list_length(GNOME_CANVAS_GROUP(item)->item_list)>1)){
      rc = snprintf(tmp, 127, "%d", anim);
      cur = xmlNewChild(cur, NULL, BAD_CAST "gcompris:animation", NULL);
      xmlNewProp( cur, BAD_CAST "delay", BAD_CAST tmp);

      rc = snprintf(tmp, 127, "%d", g_list_length(GNOME_CANVAS_GROUP(item)->item_list) );
      xmlNewProp( cur, BAD_CAST "length", BAD_CAST tmp);

      g_list_foreach(GNOME_CANVAS_GROUP(item)->item_list, (GFunc) gcompris_anim_to_svg_file, svgNode);

    }
    else
      g_list_foreach(GNOME_CANVAS_GROUP(item)->item_list, (GFunc) gcompris_item_to_svg_file, svgNode);

    xmlSaveFormatFileEnc(file, svgDoc, MY_ENCODING, 0);

    /*free the document */

    xmlFreeDoc(svgDoc);

    xmlCleanupParser();
}
 

/****************/
/* SVG restore  */
/****************/

void *gcompris_restore_svg_group(GnomeCanvasGroup *parent, xmlNodePtr svgNode);

void *svg_transform_to_canvas_matrix(xmlNodePtr svgNode, GnomeCanvasItem *item){
  xmlChar *value = NULL;
  char tmp[128];
  int rc;
  double *xform;

  value = xmlGetProp(svgNode, BAD_CAST "transform");

  if (value == NULL)
    return;

  if (strncmp((char *) value, "translate(", 10 ) == 0){
    xform = malloc(2*sizeof(double));
    rc = sscanf( value, "translate( %lf, %lf )", &(xform[0]), &(xform[1]));
  }
  else {
    /* Normally matrix */
    xform = malloc(6*sizeof(double));
    rc = sscanf( value, "matrix( %lf, %lf, %lf, %lf, %lf, %lf)", &(xform[0]), &(xform[1]),  &(xform[2]), &(xform[3]),  &(xform[4]), &(xform[5]));
    GTK_OBJECT_FLAGS(item) |= GNOME_CANVAS_ITEM_AFFINE_FULL;
  }

  item->xform = xform;
  xmlFree(value);
}

void *gcompris_svg_restore_color_re(GnomeCanvasItem *re,  xmlNodePtr cur){
  xmlChar *value;
  guint intval;
  guint r, g, b;
  gdouble doubleval;
  int i=0;

  value = xmlGetProp(cur, BAD_CAST "fill" );

  if(!xmlStrcmp( value , (const xmlChar *)"none")){
    g_object_set(G_OBJECT(re), "fill-color", NULL, NULL);
    g_object_set_data(G_OBJECT(re), "empty", &i);
  }
  else {
    sscanf(value, "rgb( %d, %d, %d )", &r, &g, &b);
    intval = (r << 24) + (g << 16) + (b << 8) + 0xFF;
    g_object_set(G_OBJECT(re), "fill-color-rgba", intval, NULL);
  }

  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "stroke" );
  sscanf(value, "rgb( %d, %d, %d )", &r, &g, &b);
  xmlFree(value);

  intval = (r << 24) + (g << 16) + (b << 8) + 0xFF;
  g_object_set(G_OBJECT(re), "outline-color-rgba", intval, NULL);

  value = xmlGetProp(cur, BAD_CAST "stroke-width" );
  sscanf(value, "%lfpx", &doubleval);

  g_object_set(G_OBJECT(re), "width-pixels", (int) doubleval, NULL);
}

void *gcompris_restore_svg_rect(GnomeCanvasGroup *parent, xmlNodePtr cur){
  GnomeCanvasItem *rect;
  xmlChar *value;
  gdouble x1, x2, y1, y2, width, height;

  rect = gnome_canvas_item_new      (parent,
				     GNOME_TYPE_CANVAS_RECT,
				     NULL);

  svg_transform_to_canvas_matrix( cur, rect );

  value = xmlGetProp(cur, BAD_CAST "x");
  sscanf( value, "%lf", &x1);
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "y");
  sscanf( value, "%lf", &y1);
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "width");
  sscanf( value, "%lf", &width);
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "height");
  sscanf( value, "%lf", &height);
  xmlFree(value);

  x2 = x1 + width;
  y2 = y1 + height;

  g_object_set(G_OBJECT(rect), "x1", x1, "x2", x2, "y1", y1, "y2", y2, NULL);

  gcompris_svg_restore_color_re(rect, cur);

}
void *gcompris_restore_svg_ellipse(GnomeCanvasGroup *parent, xmlNodePtr cur){
  GnomeCanvasItem *ellipse;
  xmlChar *value;
  double cx, cy, rx, ry;
  gdouble x1, x2, y1, y2;
  int i=0;

  ellipse = gnome_canvas_item_new   (parent,
				     GNOME_TYPE_CANVAS_ELLIPSE,
				     NULL);

  svg_transform_to_canvas_matrix( cur, ellipse );

  value = xmlGetProp(cur, BAD_CAST "cx");
  sscanf( value, "%lf", &cx);
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "cy");
  sscanf( value, "%lf", &cy);
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "rx");
  sscanf( value, "%lf", &rx);
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "ry");
  sscanf( value, "%lf", &ry);
  xmlFree(value);

  x1 = cx - rx;
  x2 = cx + rx;
  y1 = cy - ry;
  y2 = cy + ry;

  g_object_set(G_OBJECT(ellipse), "x1", x1, "x2", x2, "y1", y1, "y2", y2, NULL);
  gcompris_svg_restore_color_re(ellipse, cur);

}
void *gcompris_restore_svg_line(GnomeCanvasGroup *parent, xmlNodePtr cur){
  GnomeCanvasItem *line;
  xmlChar *value;
  guint intval, r, g, b;
  gdouble doubleval;
  GnomeCanvasPoints *points;
  int i=0;
  
  line = gnome_canvas_item_new      (parent,
				     GNOME_TYPE_CANVAS_LINE,
				     NULL);

  svg_transform_to_canvas_matrix( cur, line );

  value = xmlGetProp(cur, BAD_CAST "stroke" );
  sscanf(value, "rgb( %d, %d, %d )", &r, &g, &b);
  xmlFree(value);

  intval = (r << 24) + (g << 16) + (b << 8) + 0xFF;
  g_object_set(G_OBJECT(line), "fill-color-rgba", intval, NULL);

  value = xmlGetProp(cur, BAD_CAST "stroke-width" );
  sscanf(value, "%lfpx", &doubleval);
  g_object_set(G_OBJECT(line), "width-units", doubleval, NULL);

  points = gnome_canvas_points_new (2);

  value = xmlGetProp(cur, BAD_CAST "x1");
  sscanf( value, "%lf", &(points->coords[0]));
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "y1");
  sscanf( value, "%lf", &(points->coords[1]));
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "x2");
  sscanf( value, "%lf", &(points->coords[2]));
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "y2");
  sscanf( value, "%lf", &(points->coords[3]));
  xmlFree(value);

  g_object_set(G_OBJECT(line), "points", points, NULL);

}

void *gcompris_restore_svg_text(GnomeCanvasGroup *parent, xmlNodePtr cur){
  GnomeCanvasItem *text;
  xmlChar *value;
  guint intval, r, g, b;
  gdouble x, y;


  text = gnome_canvas_item_new (parent,
				GNOME_TYPE_CANVAS_TEXT,
				NULL);

  svg_transform_to_canvas_matrix( cur, text );

  value = xmlGetProp( cur, BAD_CAST "font-family");
  g_object_set(G_OBJECT(text), "font", value, NULL);
  /* do not xmlFree(value); */

  value = xmlGetProp(cur, BAD_CAST "fill" );
  sscanf(value, "rgb( %d, %d, %d )", &r, &g, &b);
  xmlFree(value);
  intval = (r << 24) + (g << 16) + (b << 8) + 0xFF;
  g_object_set(G_OBJECT(text), "fill-color-rgba", intval, NULL);

  value = xmlGetProp(cur, BAD_CAST "x");
  sscanf( value, "%lf", &x);
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "y");
  sscanf( value, "%lf", &y);
  xmlFree(value);

  g_object_set(G_OBJECT(text), "x", x, "y", y, NULL);

  g_object_set(G_OBJECT(text), "anchor", GTK_ANCHOR_CENTER, NULL);

  value = xmlNodeGetContent(cur);
  g_object_set(G_OBJECT(text), "text", value, NULL);
}

void *gcompris_restore_svg_image(GnomeCanvasGroup *parent, xmlNodePtr cur){
  GnomeCanvasItem *pixbuf;
  xmlChar *value, *ref;
  xmlNodePtr gcomprisImage;
  char tmp[128];
  int rc;
  GdkPixbuf *pixmap = NULL;
  double x, y, width, height;

  /* reference is in child */
  /* <svg ... ><use xlink:href="ref"/></svg> */
  ref = xmlGetProp(cur->xmlChildrenNode, "href");
  if (ref==NULL){
    printf("svg image bad parse !\n");
    return;
  }
  printf("Image ref : %s \n",ref);
  
  /* Here we look for /svg/defs/gcompris:module/image */
  /* TODO: xmlpath could maybe a good solution ? */
  /* find root svg */
  /* /svg */
  gcomprisImage = cur->parent;
  while ( gcomprisImage->parent->name != NULL ){
    printf(" Rech /svg %s \n", gcomprisImage->name);
    gcomprisImage = gcomprisImage->parent;
  }
  printf("Trouve /svg %s \n", gcomprisImage->name);

  /* /svg/defs */
  gcomprisImage = gcomprisImage->xmlChildrenNode;
  while ( gcomprisImage != NULL ){
    printf(" Rech /svg/defs %s \n", gcomprisImage->name);
    if (!xmlStrcmp(gcomprisImage->name, (const xmlChar *)"defs"))
      break;
    gcomprisImage = gcomprisImage->next;
  }

  if (gcomprisImage == NULL){
    printf("Image /svg/defs echec \n");
    return;
  }


  /* /svg/defs/gcompris:module */
  gcomprisImage = gcomprisImage->xmlChildrenNode;
  while ( gcomprisImage != NULL ){
    printf(" Rech /svg/defs/gcompris %s \n", gcomprisImage->name);
    if ((gcomprisImage->nsDef != NULL) && (!xmlStrcmp(gcomprisImage->nsDef->prefix, (const xmlChar *) "gcompris"))){
	break;
    }
    gcomprisImage = gcomprisImage->next;
  }

  if (gcomprisImage == NULL){
    printf("Image /svg/defs/gcompris echec \n");
    return;
  }

  /* /svg/defs/gcompris:module/image */
  gcomprisImage = gcomprisImage->xmlChildrenNode;
  while ( gcomprisImage != NULL ){
    printf("Rech image : %s \n", gcomprisImage->name);
    if (!xmlStrcmp(gcomprisImage->name, (const xmlChar *)"image")){
      value = xmlGetProp(gcomprisImage, "href");
      if (!xmlStrcmp(value , (const xmlChar *) ref))
	break;
    }
    gcomprisImage = gcomprisImage->next;
  }
  xmlFree(ref);
  
  if (gcomprisImage == NULL){
    printf("Can't get  <gcompris:image/> \n");
    return;
  }
  
  xmlFree(value);

  /* Here we got the right image reference */
  /* just get the filename */
  value = xmlGetProp(gcomprisImage, "filename");
  if (value == NULL){
    return;
  }


  pixmap = gcompris_load_pixmap(value);
  if (pixmap == NULL){
    printf("Can't get image from %s \n", value);
    xmlFree(value);
    return;
  }
  /* TODO */
  /* Get the image from the base64 definition if filename is missing */


  /* We got the pixmap from the original file */

  pixbuf = gnome_canvas_item_new      (parent,
				       GNOME_TYPE_CANVAS_PIXBUF,
				       NULL);

  svg_transform_to_canvas_matrix( cur, pixbuf );

  g_object_set_data (pixbuf, "filename", value);
  /* do not xmlFree(value) */

  /* <svg> element for image
     - has attributs: x y width height viewBox
     - has element <use> with image ref */

  value = xmlGetProp(cur, BAD_CAST "x");
  sscanf( value, "%lf", &x);
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "y");
  sscanf( value, "%lf", &y);
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "width");
  sscanf( value, "%lf", &width);
  xmlFree(value);

  value = xmlGetProp(cur, BAD_CAST "height");
  sscanf( value, "%lf", &height);
  xmlFree(value);

  g_object_set(G_OBJECT(pixbuf), 
	       "pixbuf", pixmap,
	       "x", x,
	       "y", y,
	       "width", width,
	       "height", height,
	       "width_set", TRUE,
	       "height_set", TRUE,
	       NULL);

  //gdk_pixbuf_unref(pixmap);

}


void *gcompris_restore_svg_group(GnomeCanvasGroup *parent, xmlNodePtr svgNode){
  /* the node itself has to be restored */
  GnomeCanvasItem *group;
  xmlNodePtr cur;
  xmlChar *value;

  group = gnome_canvas_item_new (parent,
				 GNOME_TYPE_CANVAS_GROUP,
				 NULL);

  svg_transform_to_canvas_matrix( svgNode, group);

  /* if hidden we are in anim root of a shot. hide the canvas group too */
  value = xmlGetProp( svgNode, "visibility");
  if (!xmlStrcmp( value , (const xmlChar *)"hidden"))    
    gnome_canvas_item_hide(group);
  xmlFree(value);
  
  cur = svgNode->xmlChildrenNode;
  while ( cur != NULL){
    if (!xmlStrcmp( cur->name , (const xmlChar *)"g"))
      gcompris_restore_svg_group(GNOME_CANVAS_GROUP(group), cur);
    else if (!xmlStrcmp( cur->name , (const xmlChar *)"rect"))
      gcompris_restore_svg_rect(GNOME_CANVAS_GROUP(group), cur);
    else if (!xmlStrcmp( cur->name , (const xmlChar *)"ellipse"))
      gcompris_restore_svg_ellipse(GNOME_CANVAS_GROUP(group), cur);
    else if (!xmlStrcmp( cur->name , (const xmlChar *)"line"))
      gcompris_restore_svg_line(GNOME_CANVAS_GROUP(group), cur);
    else if (!xmlStrcmp( cur->name , (const xmlChar *)"text"))
      gcompris_restore_svg_text(GNOME_CANVAS_GROUP(group), cur);

    /* SVG when is not the root element of file  is for an image */
    else if (!xmlStrcmp( cur->name , (const xmlChar *)"svg")){
      printf("Image %s détectée\n", cur->name);
      gcompris_restore_svg_image(GNOME_CANVAS_GROUP(group), cur);
    }
    cur = cur->next;
  }
  
}


void gcompris_svg_restore(char *module, char *filename, GnomeCanvasGroup *parent)
{
  xmlDocPtr svgDoc;
  xmlNodePtr svgNode, cur;
  char tmp[128];
  int rc;

  svgDoc = xmlRecoverFile(filename);
  if (svgDoc == NULL){
    printf ("Can't parse %s\n", filename);
    return;
  }

  /* Get The Root Node : normally /svg */
  /* stop on error */
  svgNode= xmlDocGetRootElement(svgDoc);
  if (svgNode == NULL){
    printf ("Document %s parse error : /svg is NULL \n", filename);
    return;
  }

  /* find "defs" /svg/defs */
  /* continue on error */
  cur = svgNode->xmlChildrenNode;
  while ( cur != NULL ){
    if (!xmlStrcmp(cur->name, (const xmlChar *)"defs")){
      break;
    }
    cur = cur->next;
  }

  if (cur == NULL){
    printf ("Document %s parse error : can't find /svg/defs\n", filename);
    //return;
  }

  /* find "gcompris:module" */
  cur = cur->xmlChildrenNode;
  while ( cur != NULL ){
      if ((cur->nsDef != NULL) && (!xmlStrcmp(cur->name, (const xmlChar *) module)) &&
	  (!xmlStrcmp(cur->nsDef->prefix, (const xmlChar *) "gcompris"))){
	break;
      }
    cur = cur->next;
  }

  /* erreur */
  if (cur == NULL){
    printf("Document %s parse error : can't find  gcompris:%s in defs\n", filename, module);
    return;
  }

  /* here we are (maybe) on a gcompris:module file */
  /* we got every <g/> in <svg/> and add it into item */
  cur = svgNode->xmlChildrenNode;
  while (cur != NULL){
    if (!xmlStrcmp(cur->name, (const xmlChar *) "g")){
      GnomeCanvasItem *group;

      /* We put each item in a group. This will help drawing application to add their anchors */
      group = gnome_canvas_item_new (parent,
				     GNOME_TYPE_CANVAS_GROUP,
				     "x", (double)0,
				     "y", (double)0,
				     NULL);

      gcompris_restore_svg_group(group, cur);
    }
    cur = cur->next;
  }
}

/* void *gcompris_filename_pass( GnomeCanvasItem *item, char *string) */
/* { */
/*   g_object_set_data( G_OBJECT(item), "filename", string); */
/* } */

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */

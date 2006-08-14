
/*
 * Copyright (C) 1999  Philippe Banwarth
 * email: bwt@altern.org
 * smail: Philippe Banwarth, 8 sente du milieu des Gaudins, 95150 Taverny, France.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gcompris/gcompris.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gtans_callbacks.h"
#include "gtans_interface.h"
#include "gtans.h"
#include "gtans_support.h"

void colselinit (int col);
void filselinit (int col);

#define FIGGC 2848

gboolean
on_wdrawareagrande_expose_event        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  if (pixmapgrande1!=NULL)
    gdk_draw_pixmap(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		    pixmapgrande1,
		    event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);

  return FALSE;
}


gboolean
on_wdrawareagrande_configure_event     (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{
  double izoom;
  tanpiecepos *piecepos;
  int i;

  widgetgrande=widget;
  if (!initcbgr)
    taninitcbgr();

  if (pixmapgrande1!=NULL){
    gdk_pixmap_unref(pixmapgrande1);
    gdk_pixmap_unref(pixmapgrande2);
  }

  pixmapgrande1 = gdk_pixmap_new(widget->window,
				 widget->allocation.width,
				 widget->allocation.height,
				 -1);
  pixmapgrande2 = gdk_pixmap_new(widget->window,
				 widget->allocation.width,
				 widget->allocation.height,
				 -1);
   
  if ( !editmode ){
    izoom=(widgetgrande->allocation.width*figgrande.zoom);
    piecepos=figgrande.piecepos;
    for (i=0; i<PIECENBR; i++){
      piecepos->posx=floor((piecepos->posx*izoom)+ARON)/izoom;
      piecepos->posy=floor((piecepos->posy*izoom)+ARON)/izoom;
      piecepos++;
    }
  }

  gdk_gc_set_line_attributes(tabgc[GCPIECEHLP],
			     widget->allocation.width>340 ? 2 : 1,
			     GDK_LINE_SOLID,
			     GDK_CAP_ROUND,
			     GDK_JOIN_ROUND);

  tanredrawgrande();

  return TRUE;
}


gboolean
on_wdrawareagrande_button_press_event  (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  int x,y;
  int piece;

  if (!selpossible || event->type!=GDK_BUTTON_PRESS)     /* double-click ou deja trouve */
    return TRUE;
  
  if (actiongrande != AN_none)      /* le button release event a ete avale */
    tanreleaseifrot();

  x=event->x;
  y=event->y;

  if ( event->button!=3){
    if ( (piece=tanwichisselect(x,y))>=0 ){
      taninitselect(piece, FALSE);
      selectedgrande=TRUE;
      actiongrande=AN_move;
      xold=x;
      yold=y;
      tandrawselect(0,0,0);
    }
    else{
      if (selectedgrande){
	actiongrande=AN_rot;
	xact=(gint16)(ARON+figgrande.piecepos[PIECENBR-1].posx*
		      widgetgrande->allocation.width*figgrande.zoom);
	yact=(gint16)(ARON+figgrande.piecepos[PIECENBR-1].posy*
		      widgetgrande->allocation.width*figgrande.zoom);
	xoth=xold=x;
	yoth=yold=y;
	rotact=tanangle((double)(xact-x),(double)(y-yact));
	rotold=0;
	
	invx2=x;
	invy2=y;
	gdk_draw_line (widgetgrande->window,
		       invertgc,
		       xact,yact,invx2,invy2);
	
      }
    }
  }
  
  if (event->button==3 && selectedgrande==TRUE){
    if (figgrande.piecepos[PIECENBR-1].type==3)
      figgrande.piecepos[PIECENBR-1].flipped^=1;
    else
      figgrande.piecepos[PIECENBR-1].rot=(figgrande.piecepos[PIECENBR-1].rot+TOUR/2)%TOUR;
    tandrawselect(0,0,0);
  }

  return TRUE;
}


gboolean
on_wdrawareagrande_button_release_event (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  tanreleaseifrot();

  /* actiongrande=AN_none; dans releaseifrot */

  if ( selpossible && figtabsize ){
    tanmaketinytabnotr(&figgrande,tinytabgr);
    tantranstinytab(tinytabgr);
    if (tantinytabcompare(tinytabgr, tinytabpe, accuracy)){
      tanunselect();
      selpossible=FALSE;
      tansetreussiactual();
      tanredrawpetite();
      /* hide widget because of gnomecanvaswidget on top of bonus image */
      gtk_widget_hide(widgetgrande);
      gtk_widget_hide(widgetpetite);
      gcompris_display_bonus( TRUE, BONUS_RANDOM);
    }
  }

  return TRUE;
}


gboolean
on_wdrawareagrande_motion_notify_event (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data)
{
  int x,y;
  GdkModifierType state;
  int rot;

  if (event->is_hint)
    gdk_window_get_pointer (event->window, &x, &y, &state);
  else{
    x=event->x;
    y=event->y;
    state=event->state;
  }

  if (actiongrande==AN_move){
    tandrawselect(x-xold,y-yold,0);
    xold=x;
    yold=y;
  }

  if (actiongrande==AN_rot){
    gdk_draw_line (widgetgrande->window,
		   invertgc,
		   xact,yact,invx2,invy2);

    rot=(rotact-tanangle((double)(xact-x),(double)(y-yact))+rotstepnbr/2+TOUR*3)%TOUR;
    rot=(int)(rot/rotstepnbr)*rotstepnbr;
    if (rot!=rotold){
      rotold=rot;
      tandrawselect(0,0,rot);
    }

    invx2=x;
    invy2=y;
    gdk_draw_line (widgetgrande->window,
		   invertgc,
		   xact,yact,invx2,invy2);

  }

  return TRUE;
}


gboolean
on_wdrawareapetite_configure_event     (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{
  widgetpetite=widget;
  if (!initcbpe)
    taninitcbpe();

  if (pixmappetite!=NULL)
    gdk_pixmap_unref(pixmappetite);
  
  pixmappetite = gdk_pixmap_new(widget->window,
				widget->allocation.width,
				widget->allocation.height,
				-1);

  tanredrawpetite();

  return TRUE;
}


gboolean
on_wdrawareapetite_expose_event        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  if (pixmappetite!=NULL)
    gdk_draw_pixmap(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		    pixmappetite,
		    event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);

  return FALSE;
}


gboolean
on_arrow_clicked (GnomeCanvasItem *item, GdkEvent *event, gpointer user_data)
{
  if ((event->type == GDK_BUTTON_PRESS)  
      && (event->button.button == 1)) {
    change_figure((gboolean) GPOINTER_TO_INT(user_data));
    return TRUE;
  }
  return FALSE;
  
}


gboolean
on_show_clicked (GnomeCanvasItem *canvasitem,
		  GdkEvent *event,
		  gpointer user_data)
{
  if ((event->type == GDK_BUTTON_PRESS)  
      && (event->button.button == 1)) {
  helptanset = (helptanset+1)%PIECENBR;
  tanredrawpetite();
  return TRUE;
  }
  return FALSE;
}

gboolean
on_outline_clicked (GnomeCanvasItem *canvasitem,
		  GdkEvent *event,
		  gpointer user_data)
{
  if ((event->type == GDK_BUTTON_PRESS)  
      && (event->button.button == 1)) {
    if(!helpoutset){
      helpoutset = TRUE;
      tanredrawgrande();
    }
    return TRUE;
  }
  return FALSE;
}

gboolean
on_symetry_clicked (GnomeCanvasItem *canvasitem,
		  GdkEvent *event,
		  gpointer user_data)
{
  if ((event->type == GDK_BUTTON_PRESS)  
      && (event->button.button == 1)) {
    if (selectedgrande==TRUE){
      if (figgrande.piecepos[PIECENBR-1].type==3)
	figgrande.piecepos[PIECENBR-1].flipped^=1;
      else
	figgrande.piecepos[PIECENBR-1].rot=(figgrande.piecepos[PIECENBR-1].rot+TOUR/2)%TOUR;
      tandrawselect(0,0,0);
      return TRUE;
    }
  }
  return FALSE;
}

gboolean
on_rotation_clicked (GnomeCanvasItem *canvasitem,
		  GdkEvent *event,
		  gpointer user_data)
{
  if ((event->type == GDK_BUTTON_PRESS)  
      && (event->button.button == 1)) {
    gint angle = 0;

    if (selectedgrande==TRUE){
      switch ((gint) user_data){
      case 0:
	angle = -rotstepnbr;
	break;
      case 1:
	angle = rotstepnbr;
	break;
      case 2:
	angle = -4*rotstepnbr;
	break;
      case 3:
	angle = 4*rotstepnbr;
	break;
      }
      figgrande.piecepos[PIECENBR-1].rot += angle;

      tandrawselect(0,0,0);
      return TRUE;
    }
  }
  return FALSE;
}


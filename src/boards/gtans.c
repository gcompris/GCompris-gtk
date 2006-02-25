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

#include "gcompris/gcompris.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

/* Added by Florian Ernst <florian_ernst@gmx.net> for lines 193 and 194 */
//#include <string.h>
/* End of added section */

/* uniquement pour mkdir() */
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h> 

#include "gtans.h"
#include "gtans_interface.h"
#include "gtans_support.h"

void taninitstart(void);

/********************************/
/* int main (int argc, char *argv[]) */
/* { */
/*    textdomain ("gtans"); */

/*   gtk_set_locale (); */
/*   gtk_init (&argc, &argv); */

/*   /\* */
/*     add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps"); */
/*     add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps"); */
/*   *\/ */

/*   taninitstart(); */

/*   gtk_main (); */
/*   return 0; */
/* } */


/* GCompris */

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);

static gint actual_figure = -2;

/* Description of this plugin */


static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "GTans",
    "Tangram",
    "Bruno Coudoin <bruno.coudoin@free.fr>",
    NULL,
    NULL,
    NULL,
    NULL,
    start_board,
    pause_board,
    end_board,
    is_our_board,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
  };

GET_BPLUGIN_INFO(gtans)

static GcomprisBoard *gcomprisBoard = NULL;

static GnomeCanvasGroup *boardRootItem = NULL;

static gboolean board_paused = FALSE;

static void start_board (GcomprisBoard *agcomprisBoard)
{

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 1;
      gcompris_bar_set(0);

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "images/gtans_bg.png");

      selectedgrande = FALSE;
      taninitstart();
    }
}

static void end_board (void)
{
  gtk_object_destroy (GTK_OBJECT(boardRootItem));
  boardRootItem = NULL;

  widgetpetite = NULL;
  widgetgrande = NULL;


  pixmapgrande1=NULL;
  pixmapgrande2=NULL;
  pixmappetite=NULL;
  pixmappiece1=NULL;
  pixmappiece2=NULL;
  pixmapfond=NULL;
  colselwin=NULL;
  filselwin=NULL;

  selectedgrande = FALSE;
}

static gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "gtans")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

static void pause_board (gboolean pause)
{

  if(gcomprisBoard==NULL)
    return;

  board_paused = pause;

  if ((!pause) && figpetite.reussi) {
    gtk_widget_show(widgetgrande);
    gtk_widget_show(widgetpetite);

    actual_figure++;
    tansetnewfigurepart1(actual_figure);
    tansetnewfigurepart2();
  } else {
    if (pause) {
      gtk_widget_hide(widgetgrande);
      gtk_widget_hide(widgetpetite);
    
    } else {
      gtk_widget_show(widgetgrande);
      gtk_widget_show(widgetpetite);
    }
  }
    
}

void change_figure(gboolean next){
  if (next)
    tansetnewfigurepart1((actual_figure+1)%figtabsize);
  else
    tansetnewfigurepart1((actual_figure + figtabsize -1)%figtabsize);
  tansetnewfigurepart2();
}



/********************************/
GtkWidget *mainwindow;
GtkWidget *widgetgrande;        /* widget de la grande drawarea */
GtkWidget *widgetpetite;        /* widget de la petite drawarea */
GtkStatusbar *widgetstat;       /* widget de la statusbar */
GtkSpinButton *spinner=NULL;

gint statconid;                 /* context id de la statusbar */
GdkRectangle selbackrect;       /* rectangle a redessiner pour effacer la piece selected */

GdkPixmap *pixmapgrande1=NULL,*pixmapgrande2;
GdkPixmap *pixmappetite=NULL;
GdkPixmap *pixmappiece1=NULL,*pixmappiece2=NULL,*pixmapfond=NULL;

GtkWidget *colselwin=NULL,*filselwin=NULL;


/* les polygones doivent etre clockwise */
tanpiecedef piecesdef[]={
 {2.0/3,2.0/3,   4,{{1,0,6*HT},{1,0,0},{0,1,2*HT},{0,1,0}},     3,{{0,0},{2,0},{0,2},{0,0}}},
 {CL/3,CL/3,     2,{{CC,CC,5*HT},{CC,CC,3*HT},{0,0,0},{0,0,0}}, 3,{{0,0},{CL,0},{0,CL},{0,0}}},
 {0.5,0.5,       2,{{0,0,0},{1,1,4*HT},{0,0,0},{0,0,0}},        4,{{0,0},{1,0},{1,1},{0,1}}},
 {CC/2,(CC+CL)/2,2,{{0,CC,1*HT},{CC,CL,5*HT},{0,0,0},{0,0,0}},4,{{0,CC},{CC,0},{CC,CL},{0,CC+CL}}},
 {1.0/3,1.0/3,   1,{{0,0,0},{0,0,0},{0,0,0},{0,0,0}},           3,{{0,0},{1,0},{0,1},{0,0}}}
};

tanfigure figuredebut={
  0.125,1,TOUR/64,FALSE,
  {{0,0,0.8,0.8,0},
   {0,0,3.3,0.8,0},
   {1,0,1.5,1.5,HT*4},
   {2,0,6,0.6,0},
   {3,0,6,1.6,HT*2},
   {4,0,3.6,1.8,HT*7},
   {4,0,4.1,1.3,HT*1}}
};

tanfigure *figtab=NULL;
int figtabsize;                    /* ==0 : pas de figure */
char *figfilename=NULL;
int figactualnr;                   /* nr de la figure dans le figtab (-1=figuredebut, -2=figpetite) */

tantinytri tinytabpe[TINYNBR],tinytabgr[TINYNBR];
tanfigure figgrande,figpetite;
int selectedgrande=FALSE;
int xact,yact,xoth,yoth,xold,yold;
int actiongrande=AN_none;
int selpossible=TRUE;
int rotact,rotnew,rotold;

int rotstepnbr=TOUR/32;            /* nb. de pas de rotation affiches */
int initcbgr=FALSE,initcbpe=FALSE; /* init cb deja appellee ? */

GdkGC *invertgc;
int invx2,invy2;

GdkGC *tabgc[GCNBR];
GdkColor colortab[GCNBR];
GdkPixmap *dumtabpxpx[3],**tabpxpx=dumtabpxpx-PXSTART;
char *dumtabpxnam[3],**tabpxnam=dumtabpxnam-PXSTART;

gboolean helpoutset=FALSE;
gboolean helptanset=PIECENBR;
int accuracy;                          /* precision de reconaissance */

gboolean editmode;

#define FLPNTMAX PIECENBR*(PNTNBRMAX+1)*2
static gboolean dumtabpxpixmode[3],*tabpxpixmode=dumtabpxpixmode-PXSTART; /* mode VOULU */
static gboolean tabcolalloc[GCNBR];   /* couleur allouee ? */
static gchar *userconf=NULL;          /* nom complet du fichier de config local */
static gchar *usergtdir=NULL;         /* nom du home directory local */
static guint provmsg=0;              /* nr. du dernier msg. provisoire */
static tanflfig flfigpetite;
static tanfpnt fpntspetite[FLPNTMAX];

static double dxout,dyout,dxpetite,dypetite;

static double selposxnc,selposync;      /* position de la piece actuelle non limitee */


/********************************/
/* change la valeur max du spinbutton (si il existe) */
void tanspinsetvalmax (int val){
  GtkAdjustment *adj;

  if (spinner!=NULL){
    adj = gtk_spin_button_get_adjustment(spinner);
    adj->upper = (gfloat)val;
    gtk_adjustment_changed(adj);
    if (gtk_spin_button_get_value_as_int(spinner)){
      gtk_spin_button_set_value(spinner, 0);
    }
    else{
      tansetnewfigurepart1(0);
      tansetnewfigurepart2();
    }      
  } else
    {
      tansetnewfigurepart1(0);
      tansetnewfigurepart2();
    }

}


/********************************/
void tanallocname (char **pnt, char *name){
  if (*pnt!=name){
    if (*pnt!=NULL)
      g_free(*pnt);
    *pnt=(char *)g_malloc(strlen(name)+1);
    strcpy(*pnt,name);
  }
}



/********************************/
/* maintien les pieces dans les limites */
void tanclampgrandefig (void){
  
  tanpiecepos *piecepos;
  int i;
  double dumzoom;
  
  dumzoom = 1.0/figgrande.zoom;
  piecepos = figgrande.piecepos;
  for (i = 0; i<PIECENBR; i++){
    piecepos->posx = CLAMP(piecepos->posx, 0.0, dumzoom);
    piecepos->posy = CLAMP(piecepos->posy, 0.0, dumzoom);
    piecepos++;
  }
  
  return;
}


/********************************/
/* renvoi la direction d'un segment */
int tanangle (double dx, double dy){
  int ret;

  ret = (int)(atan2(dy,dx)/PASTOUR);
  ret = (ret+TOUR)%TOUR;

  return (ret);

}


/********************************/
gboolean tantinytabcompare (tantinytri *tinys1, tantinytri *tinys2, int accuracy){

  gboolean libre[TINYNBR];
  int i,j,jmin;
  double dist,mindist,mindistmax,xi,yi;
  int drot,drotmax,roti;
  double flaccur;

  switch (accuracy){
  case 0 :
    flaccur = 1.0;
    drotmax = (int)(TOUR/64)+1;
    break;
  case 2 :
    flaccur = 4.0;
    drotmax = (int)(TOUR/32)+1;
    break;
  default :
    flaccur = 2.0;
    drotmax = (int)(TOUR/64)+1;
  }
  
  /* drotmax=figpetite.drotmax; */
  mindistmax=pow(figpetite.distmax*0.10*flaccur,2);

  for (i=0; i<TINYNBR; i++)
    libre[i]=TRUE;

  for (i=0; i<TINYNBR; i++){
    jmin=0;
    mindist=100000;
    xi=tinys1[i].posx;
    yi=tinys1[i].posy;
    roti=tinys1[i].rot;

    for (j=0; j<TINYNBR; j++){
      if (libre[j]){
	dist=pow(xi-tinys2[j].posx,2)+pow(yi-tinys2[j].posy,2);
	drot=ABS(roti-tinys2[j].rot);
	if ( drot>TOUR/2 )
	  drot = TOUR-drot;
	if ( dist<mindist && drot<drotmax ){
	  mindist=dist;
	  jmin=j;
	}
      }
    }
    libre[jmin]=FALSE;
    if ( mindist>mindistmax )
      return FALSE;
  }

  return TRUE;

}


/********************************/
void tansmall2tiny (tansmalltri *small, tantinytri *tiny1, tantinytri *tiny2){

  double cosrot,sinrot;
  int rot;

  rot=small->rot;
  cosrot=cos(rot*PASTOUR);
  sinrot=sin(rot*PASTOUR);

  tiny1->rot=(rot+HT*3)%TOUR;
  tiny1->posx=small->posx+0.5*cosrot       +0.16666666*sinrot;
  tiny1->posy=small->posy+0.16666666*cosrot-0.5*sinrot;

  tiny2->rot=(rot+HT*5)%TOUR;
  tiny2->posx=small->posx+0.16666666*cosrot+0.5*sinrot;
  tiny2->posy=small->posy+0.5*cosrot       -0.16666666*sinrot;

}


/********************************/
void tanmaketinytabnotr (tanfigure *figure, tantinytri *tinys){

  int i,j;
  tansmalltri dusmall,*small=&dusmall;
  tanpiecepos *piecepos;
  tanpiecedef *piecedat;
  double ly,lx2,cosrot,sinrot;
  int rot,rottri;

  piecepos=figure->piecepos;

  for (j=0; j<PIECENBR; j++){

    piecedat=&piecesdef[piecepos->type];
    rot=piecepos->rot;
    cosrot=cos(rot*PASTOUR);
    sinrot=sin(rot*PASTOUR);

    for (i=0; i<piecedat->trinbr; i++){
      lx2=piecedat->tri[i].posx-piecedat->handlex;
      ly=piecedat->tri[i].posy-piecedat->handley;
      rottri=piecedat->tri[i].rot;

      if (piecepos->flipped){
	lx2=-lx2;
	rottri=TOUR+6*HT-rottri;
      }
      
      small->posx=piecepos->posx+lx2*cosrot+ly*sinrot;
      small->posy=piecepos->posy+ly*cosrot-lx2*sinrot;
      small->rot=(rottri+rot)%TOUR;
      tansmall2tiny(small,tinys,tinys+1);
      tinys+=2;
    }
    piecepos++;
  }

}


/********************************/
void tantranstinytab (tantinytri *tinys){

  int i;
  double moyx=0,moyy=0;

  for (i=0; i<TINYNBR; i++){
    moyx+=tinys->posx;
    moyy+=tinys->posy;
    tinys++;
  }

  moyx/=TINYNBR;
  moyy/=TINYNBR;

  for (i=0; i<TINYNBR; i++){
    (--tinys)->posx-=moyx;
    tinys->posy-=moyy;
  }

}


/********************************/
/* termine la rotation (lorsque le mouse button est relache) */
/* est appele par redrawgrande et on_buttonpress au cas ou le signal a ete masque par*/
/* l'appui sur un bouton */
void tanreleaseifrot (void){
  if (actiongrande==AN_rot){
    gdk_draw_line (widgetgrande->window,
		   invertgc,
		   xact,yact,invx2,invy2);
    
    figgrande.piecepos[PIECENBR-1].rot=(rotnew+TOUR*5)%TOUR;
  }
  
  actiongrande = AN_none;
  
  return;
}


/********************************/
/* calcule une serie de GdkPoint correspondant au polygone de la piece */
/* + un point correspondand au centre de la piece */
int tanplacepiece (tanpiecepos *piecepos, GdkPoint *pnts, double zoom){

  int i;
  tanpiecedef *piecedat;
  double lx,ly,lx2,cosrot,sinrot;
  int rot;

  piecedat=&piecesdef[piecepos->type];
  rot=piecepos->rot;
  cosrot=cos(rot*PASTOUR);
  sinrot=sin(rot*PASTOUR);

  for(i=0; i<piecedat->pntnbr; i++){
    lx2=piecedat->pnt[i].posx-piecedat->handlex;
    ly=piecedat->pnt[i].posy-piecedat->handley;
    if (piecepos->flipped)
      lx2=-lx2;
    lx=(piecepos->posx+lx2*cosrot+ly*sinrot)*zoom;
    ly=(piecepos->posy+ly*cosrot-lx2*sinrot)*zoom;
    pnts->x=(gint16)(lx+ARON);
    pnts->y=(gint16)(ly+ARON);
    pnts++;
  }

  pnts->x=(gint16)(piecepos->posx*zoom+ARON);
  pnts->y=(gint16)(piecepos->posy*zoom+ARON);

  return(piecedat->pntnbr);

}


/********************************/
/* calcule une serie de point en flottant correspondant au polygone de la piece */
/* pas de point central */
/* copie le premier points derriere le dernier */
/* le polygone retourne est clockwise (si la def l'est) */
int tanplacepiecefloat (tanpiecepos *piecepos, tanfpnt *fpnts, double zoom){

  int i;
  tanpiecedef *piecedat;
  double lx,ly,lx2,cosrot,sinrot;
  int rot;
  int nbr;

  piecedat=&piecesdef[piecepos->type];
  nbr=piecedat->pntnbr;
  rot=piecepos->rot;
  cosrot=cos(rot*PASTOUR);
  sinrot=sin(rot*PASTOUR);

  for(i=0; i<nbr; i++){
    lx2=piecedat->pnt[i].posx-piecedat->handlex;
    ly=piecedat->pnt[i].posy-piecedat->handley;
    if (piecepos->flipped)
      lx2=-lx2;
    lx=(piecepos->posx+lx2*cosrot+ly*sinrot)*zoom;
    ly=(piecepos->posy+ly*cosrot-lx2*sinrot)*zoom;
    fpnts->posx=lx;
    fpnts++->posy=ly;
  }
  fpnts -=nbr;

  if (piecepos->flipped){
    tanfpnt dumfpnt;
    for (i = 0; i<nbr/2 ;i++){
      dumfpnt = fpnts[i];
      fpnts[i] = fpnts[nbr-i-1];
      fpnts[nbr-i-1] = dumfpnt;
    }
  }

  *(fpnts+nbr)=*fpnts;

  return(nbr);

}


/********************************/
/* calcule le carre de la distance et le deplacement entre un point et un segment */
/* renvoie 100000.0 si la projection sur la droite du point n'est pas sur le segment */
/* resultat : vecteur du segment au point */
double tandistcarsegpnt (tanfpnt *segment, tanfpnt *point, double *pdx, double *pdy){

  double segdx,segdy;
  double seglencar,scal,dum;

  segdx=segment[1].posx-segment[0].posx;
  segdy=segment[1].posy-segment[0].posy;
  *pdx=point->posx-segment->posx;
  *pdy=point->posy-segment->posy;

  seglencar=segdx*segdx+segdy*segdy;
  if ( (scal=(*pdx*segdx)+(*pdy*segdy))<0 || (dum=scal/seglencar)>1 )
    return (1000000.0);

  *pdx-=segdx*dum;
  *pdy-=segdy*dum;

  return (*pdx*(*pdx)+*pdy*(*pdy));

}


/********************************/
double tandistcar (tanfpnt *pnt1, tanfpnt *pnt2){

  double dx,dy;

  dx=(pnt1->posx-pnt2->posx);
  dy=(pnt1->posy-pnt2->posy);
  return (dx*dx+dy*dy);

}


/********************************/
void tancolle (tanfigure *figure, double seuil){

  tanpiecepos *piecepos;
  tanfpnt pnts1[PNTNBRMAX+1],pnts2[PNTNBRMAX+1];
  int pntnbr1,pntnbr2;
  int i,j,k,l;
  int nbrcommun;
  double dx,dy,dx2,dy2,dxtot,dytot;

  seuil*=seuil;
  piecepos=figure->piecepos;

  for (i=0; i<PIECENBR-1; i++){
    for (j=i+1; j<PIECENBR; j++) {
      pntnbr1=tanplacepiecefloat(&piecepos[i],pnts1,1);
      pntnbr2=tanplacepiecefloat(&piecepos[j],pnts2,1);
      nbrcommun=0;
      dxtot=dytot=0;
      for (k=0; k<pntnbr1; k++){
	for (l=0; l<pntnbr2; l++){
	  dx=pnts1[k+1].posx-pnts2[l].posx;
	  dy=pnts1[k+1].posy-pnts2[l].posy;
	  dx2=pnts1[k].posx-pnts2[l+1].posx;
	  dy2=pnts1[k].posy-pnts2[l+1].posy;
	  if ( (dx*dx+dy*dy)>seuil && (dx2*dx2+dy2*dy2)>seuil ){
	    if ( tandistcarsegpnt(&pnts1[k],&pnts2[l],&dx,&dy)<seuil/4 ){
	      nbrcommun++;
	      dxtot-=dx;
	      dytot-=dy;
	    }
	    if ( tandistcarsegpnt(&pnts2[l],&pnts1[k],&dx,&dy)<seuil/4 ){
	      nbrcommun++;
	      dxtot+=dx;
	      dytot+=dy;
	    }
	  }
	}
      }
      if (nbrcommun){
	piecepos[j].posx+=dxtot/nbrcommun;
	piecepos[j].posy+=dytot/nbrcommun;
      }
      
      pntnbr2=tanplacepiecefloat(&piecepos[j],pnts2,1);
      nbrcommun=0;
      dxtot=dytot=0;
      for (k=0; k<pntnbr1; k++){
	for (l=0; l<pntnbr2; l++){
	  dx=(pnts1[k].posx-pnts2[l].posx);
	  dy=(pnts1[k].posy-pnts2[l].posy);
	  if ( (dx*dx+dy*dy)<seuil ){
	    nbrcommun++;
	    dxtot+=dx;
	    dytot+=dy;
	  }
/*	  else{
	    dx=(pnts1[(k+1)*2]-pnts2[l*2]);
	    dy=(pnts1[(k+1)*2+1]-pnts2[l*2+1]);
	    dx2=(pnts1[k*2]-pnts2[(l+1)*2]);
	    dy2=(pnts1[k*2+1]-pnts2[(l+1)*2+1]);
	    if ( (dx*dx+dy*dy)>seuil && (dx2*dx2+dy2*dy2)>seuil ){
	      if ( tandistcarsegpnt(pnts1+k*2,pnts2+l*2,&dx,&dy)<seuil/100 ){
		nbrcommun++;
		dxtot-=dx;
		dytot-=dy;
	      }
	      if ( tandistcarsegpnt(pnts2+l*2,pnts1+k*2,&dx,&dy)<seuil/100 ){
		nbrcommun++;
		dxtot+=dx;
		dytot+=dy;
	      }
	    }
	  }         */
	}
      }
      
      if (nbrcommun){
	piecepos[j].posx+=dxtot/nbrcommun;
	piecepos[j].posy+=dytot/nbrcommun;
      }
    }
  }
}


/********************************/
GdkRectangle tandrawpiece (GtkWidget *widget,GdkPixmap *pixmap,
			   tanpiecepos *piecepos,
			   double zoom, tanremplis remplis){

  GdkPoint pnt[PNTNBRMAX+1];
  int i,pntnbr,ix,iy,ixmax=-20000,ixmin=20000,iymax=-20000,iymin=20000;
  GdkRectangle update_rect;
  GdkGC *gc;
  double gris,rx,ry;

  pntnbr=tanplacepiece(piecepos,pnt,zoom);

  for(i=0; i<pntnbr; i++){
    ix=pnt[i].x;
    iy=pnt[i].y;
    if (ix<ixmin)
      ixmin=ix;
    if (ix>ixmax)
      ixmax=ix;
    if (iy<iymin)
      iymin=iy;
    if (iy>iymax)
      iymax=iy;
  }

  update_rect.x=ixmin;
  update_rect.y=iymin;
  update_rect.width=ixmax-ixmin+1;
  update_rect.height=iymax-iymin+1;

  switch (remplis){
  case TAN_PETITEHLP:
    gc=tabgc[GCPETITEHLP];
    break;
  case TAN_PIECENOR:
    gc=tabgc[GCPIECENOR];
    gdk_gc_set_ts_origin (gc,pnt[pntnbr].x,pnt[pntnbr].y);
    break;
  case TAN_PIECEHI:
    gc=tabgc[GCPIECEHI];
    gdk_gc_set_ts_origin (gc,pnt[pntnbr].x,pnt[pntnbr].y);
    break;
  default:
    gc=widget->style->white_gc;
    break;
  }

  gdk_draw_polygon (pixmap,
		    gc,
		    TRUE,
		    pnt,
		    pntnbr);

  if ( remplis==TAN_PIECENOR || remplis==TAN_PIECEHI ){
    pnt[pntnbr]=pnt[0];                 /* ecrase le point du centre */
    for (i=0; i<pntnbr; i++){
      rx=pnt[i+1].x-pnt[i].x;
      ry=pnt[i].y-pnt[i+1].y;
      gris=(ry+rx)*0.35355339/sqrt(rx*rx+ry*ry);
      if (piecepos->flipped)
	gris=-gris;
      gris=gris+0.5;
      gdk_draw_line (pixmap,
		     tabgc[(int)(gris*(GRISNBR))],
		     pnt[i].x,pnt[i].y,pnt[i+1].x,pnt[i+1].y);
    }
  }
  return(update_rect);

}


/********************************/
void tandrawfigure (GtkWidget *widget,GdkPixmap *pixmap,
		    tanfigure *figure,int exclue, tanremplis remplis){

  double zoom;
  int i;
  tanpiecepos *piecepos;

  zoom=widget->allocation.width*figure->zoom;
  piecepos=figure->piecepos;

  for (i=0; i<PIECENBR; i++){
    if (i!=exclue)
      tandrawpiece(widget,pixmap,piecepos,zoom,remplis);
    piecepos++;
  }
}


/********************************/
/* affiche flfigpetite dans le pixmap */
void tandrawfloat (GdkPixmap *pixmap, gboolean isoutline){

  tanflfig *flfig=&flfigpetite;
  double dx,dy;
  GdkPoint pnts[PIECENBR*(PNTNBRMAX+1)];
  int flpntnbr;
  int flpiecenbr;
  int i,j;
  tanfpnt *figfpnts;
  double zoom;
  tanpolytype polytype;
  
  
  if (isoutline){
    zoom = widgetgrande->allocation.width*figgrande.zoom;
    dx=dxout;
    dy=dyout;
  }
  else{
    zoom = widgetpetite->allocation.width*figpetite.zoom;
    dx=dxpetite;
    dy=dypetite;
  }  
  
  flpiecenbr = flfig->flpiecenbr;
  for (i = 0; i<flpiecenbr; i++){
    figfpnts = flfig->flpieces[i].flpnts;
    flpntnbr = flfig->flpieces[i].flpntnbr;
    polytype = flfig->flpieces[i].polytype;
    for (j = 0; j<flpntnbr; j++){
      pnts[j].x = (gint16)(zoom*(figfpnts[j].posx-dx)+ARON);
      pnts[j].y = (gint16)(zoom*(figfpnts[j].posy-dy)+ARON);
    }
    if (isoutline){
      pnts[flpntnbr] = pnts[0];
      gdk_draw_lines(pixmap, tabgc[GCPIECEHLP], pnts, flpntnbr+1);
    }
    else {
      gdk_draw_polygon(pixmap,
		       (polytype==TAN_POLYON) ? ( figpetite.reussi ? tabgc[GCPETITECHK] : tabgc[GCPETITEBG] ) : tabgc[GCPETITEFG],
		       TRUE, pnts, flpntnbr);
    }
  }
}


/********************************/
/* affiche le fond de la widgetgrande */
void tandrawbgndgr (GdkPixmap *pixmap){
  

  gdk_draw_rectangle (pixmap,
		      tabgc[GCPIECEBG],
		      TRUE,
		      0, 0,
		      widgetgrande->allocation.width,
		      widgetgrande->allocation.height);

  if ( helpoutset && figtabsize )
    tandrawfloat(pixmap, TRUE);

}


/********************************/
void taninitselect(int selected, gboolean force){

  int i;
  tanpiecepos dum;

  if ( force ||
       selected != PIECENBR-1 ||
       !selectedgrande ){

    tandrawbgndgr(pixmapgrande2);

    tandrawfigure(widgetgrande,pixmapgrande2,&figgrande,
		  selected,TAN_PIECENOR);

    selbackrect.x=0;
    selbackrect.y=0;
    selbackrect.width=widgetgrande->allocation.width;
    selbackrect.height=widgetgrande->allocation.height;

    dum=figgrande.piecepos[selected];
    for (i=selected; i<PIECENBR-1; i++)
      figgrande.piecepos[i]=figgrande.piecepos[i+1];
    figgrande.piecepos[PIECENBR-1]=dum;
  }

  selposxnc = figgrande.piecepos[PIECENBR-1].posx;
  selposync = figgrande.piecepos[PIECENBR-1].posy;

}

/********************************/
void tandrawselect(int dx, int dy, int drot){
  tanpiecepos *selpiece;
  double zoom;
  int dumrot;
  GdkRectangle selbk2;

  selpiece=&(figgrande.piecepos[PIECENBR-1]);
  zoom=widgetgrande->allocation.width*figgrande.zoom;
  
  selposxnc += dx/zoom;
  selposync += dy/zoom;

  selpiece->posx = CLAMP(selposxnc, 0, 1.0/figgrande.zoom);
  selpiece->posy = CLAMP(selposync, 0, 1.0/figgrande.zoom);
  dumrot=selpiece->rot;
  rotnew=selpiece->rot-=drot;

  gdk_draw_pixmap(pixmapgrande1,
		  widgetgrande->style->fg_gc[GTK_WIDGET_STATE (widgetgrande)],
		  pixmapgrande2,
		  selbackrect.x,selbackrect.y,
		  selbackrect.x,selbackrect.y,
		  selbackrect.width,selbackrect.height);

  selbk2=tandrawpiece(widgetgrande,pixmapgrande1,
		      selpiece,
		      zoom,
		      TAN_PIECEHI);

  gtk_widget_draw (widgetgrande, &selbackrect);
  gtk_widget_draw (widgetgrande, &selbk2);

  selbackrect=selbk2;

  selpiece->rot=dumrot;

}


/********************************/
void tanredrawgrande (void){

  GdkRectangle rect={0,0,0,0};
  GtkWidget *widget=NULL;


  tanreleaseifrot();

  if (selectedgrande){
    taninitselect(PIECENBR-1, TRUE);
    tandrawselect(0,0,0);
  }
  else{
    widget=widgetgrande;
    tandrawbgndgr(pixmapgrande1);
    tandrawfigure(widget, pixmapgrande1, &figgrande, PIECENBR+1, TAN_PIECENOR);
    rect.width=widget->allocation.width;
    rect.height=widget->allocation.height;
    gtk_widget_draw (widget, &rect);
  }
  
}


/********************************/
void tanclearreussinr (int fignr){

  if ( fignr>= 0 && fignr<figtabsize )
    (figtab+fignr)->reussi = FALSE;
  
}


/********************************/
void tansetreussiactual (void){

  figpetite.reussi = TRUE;
  if ( figactualnr>= 0 && figactualnr<figtabsize )
    (figtab+figactualnr)->reussi = TRUE;
  
}


/********************************/
void tanredrawpetite (void){

  GdkRectangle rect={0,0,0,0};
  int wid,hei;

  /* in case we are called before widget configured */
  if (!widgetpetite)
    return;

  wid = widgetpetite->allocation.width;
  hei = widgetpetite->allocation.height;

  gdk_draw_rectangle (pixmappetite,
		      figpetite.reussi ? tabgc[GCPETITECHK] : tabgc[GCPETITEBG],
		      TRUE,
		      0, 0, wid, hei);

  if (!figtabsize)
    return;

  tandrawfloat (pixmappetite, FALSE);

  if (helptanset<PIECENBR)
    tandrawpiece(widgetpetite,
		 pixmappetite,
		 &figpetite.piecepos[helptanset],
		 widgetpetite->allocation.width*figpetite.zoom,
		 TAN_PETITEHLP);

  /* tandrawfigure(widget, pixmappetite, &figpetite, PIECENBR+1, TAN_PETITEFG); */

  rect.width=wid;
  rect.height=hei;
  gtk_widget_draw (widgetpetite, &rect);

}


/********************************/
void tanunselect (void){
  
  if (selectedgrande){
    selectedgrande=FALSE;
    tanredrawgrande();
  }
}


/********************************/
void tanloadfigstatus (char *name, tanfigure *nfigtab, int nfigsize){

  int i;
  FILE *hand=NULL;
  gchar *statusfilename;
  
  statusfilename = g_strconcat(usergtdir, G_DIR_SEPARATOR_S, g_basename(name), ".status", NULL);
  
  if ( (hand = fopen(statusfilename, "r"))!=NULL ){
    for (i=0; i<nfigsize; i++)
      if ( fgetc(hand)=='y' )
	(nfigtab+i)->reussi = TRUE;
    fclose(hand);
  }
  
  g_free(statusfilename);
  
}


/********************************/
void tansavefigstatus (char *name, tanfigure *nfigtab, int nfigsize){

  int i;
  FILE *hand=NULL;
  gchar *statusfilename;
  
  if(figtabsize){
    statusfilename = g_strconcat(usergtdir, G_DIR_SEPARATOR_S, g_basename(name), ".status", NULL);
    
    if ( (hand = fopen(statusfilename, "w"))!=NULL ){
      for (i=0; i<nfigsize; i++)
	if ( (nfigtab+i)->reussi )
	  fputc ('y', hand);
	else
	  fputc ('n', hand);
      fclose(hand);
    }
    
    g_free(statusfilename);
  }
}


/********************************/
gdouble tanreadfloat(FILE *fhd, int *lres)
{
  gdouble pouet;
  char buf[100];

  pouet = 1;
  if (*lres==1){
    *lres = fscanf(fhd, "%99s",buf);
    pouet=g_strtod(buf,NULL);
  }

  return pouet;

}


/********************************/
#define SPESC if (lres==1) lres = fscanf

gboolean tanloadfigtab (char *name){

 FILE *hand=NULL;
 int i,j;
 gboolean succes;
 int newfigtabsize;
 tanfigure *newfigtab=NULL,*figs;
 int lres;

 lres=0;

 if ( (hand = fopen(name, "r"))!=NULL &&
      fscanf(hand, "gTans v1.0 %d \n", &newfigtabsize)==1 &&
      (newfigtab = (tanfigure *)g_malloc(sizeof(tanfigure)*newfigtabsize))!=NULL ){

   lres=1;
   figs = newfigtab;
   for (i = 0; i<newfigtabsize; i++){
     *figs = figuredebut;
     figs->zoom = tanreadfloat(hand, &lres);
     figs->distmax = tanreadfloat(hand, &lres);
     SPESC(hand,"%d \n", &figs->drotmax);
     /*fscanf(hand,"%le %le %d \n",&figs->zoom,&figs->distmax,&figs->drotmax);*/
     for (j=0; j<PIECENBR; j++){
       SPESC(hand,"p %d", &figs->piecepos[j].type);
       SPESC(hand,"%d", &figs->piecepos[j].flipped);
       figs->piecepos[j].posx = tanreadfloat(hand, &lres);
       figs->piecepos[j].posy = tanreadfloat(hand, &lres);
       SPESC(hand,"%d \n", &figs->piecepos[j].rot);
       /*fscanf(hand,"%d %d %le %le %d \n",&figs->piecepos[j].type,&figs->piecepos[j].flipped,
	 &figs->piecepos[j].posx,&figs->piecepos[j].posy,&figs->piecepos[j].rot);*/
     }
     figs++;
   }
 } else
      g_warning("Opening file %s fails",name);
 
 if (hand!=NULL)
   fclose(hand);

 succes=FALSE;
 if (lres==1){
   succes=TRUE;

   tansavefigstatus(figfilename, figtab, figtabsize);
   
   if(figtab!=NULL)
     g_free(figtab);
   
   tanloadfigstatus(name, newfigtab, newfigtabsize);

   figtab=newfigtab;
   figtabsize=newfigtabsize;

   actual_figure = 0;

   tansetnewfigurepart1(actual_figure);
   tansetnewfigurepart2();

   //tanspinsetvalmax(figtabsize-1);
 }

 if (succes || figfilename==NULL)
   tanallocname(&figfilename, name);
 
 return(succes);

}


/********************************/
/* charge un pixmap, si necessaire desalloue et/ou (re)alloue la couleur */
gboolean tansetpixmapmode(GtkWidget *widget, char *aname, int gcnbr){

  GdkPixmap *pixmap;
  GdkGC *gc;
  char *pname;
  gboolean ret;
  

  pixmap=tabpxpx[gcnbr];
  pname=tabpxnam[gcnbr];
  gc=tabgc[gcnbr];

  if (tabcolalloc[gcnbr]){
    gdk_colormap_free_colors (gdk_colormap_get_system(), &colortab[gcnbr], 1);
    tabcolalloc[gcnbr] = FALSE;
  }
  
  if (pixmap!=NULL)
    gdk_pixmap_unref(pixmap);
  
  ret=FALSE;
  if ( (pixmap=gdk_pixmap_create_from_xpm (widget->window, NULL, NULL, aname))!=NULL ){
    tanallocname(&pname,aname);
    gdk_gc_set_fill (gc, GDK_TILED);
    gdk_gc_set_tile (gc, pixmap);
    ret=TRUE;
  }
  
  if (pname==NULL)
    tanallocname(&pname,"LoadPixmapFailed");
  
  tabpxpx[gcnbr] = pixmap;
  tabpxnam[gcnbr] = pname;
  tabpxpixmode[gcnbr] = ret;

  if (!ret)
    tansetcolormode(&colortab[gcnbr],gcnbr);
  
  return (ret);
  
}


/********************************/
/* passe en mode color, decharge le pixmap (mais pas le nom) */
void tansetcolormode(GdkColor *acolor, int gcnbr){

  GdkPixmap *pixmap;
  GdkGC *gc;
  GdkColor *pcolor;
  GdkColormap *syscmap;
  
  gc = tabgc[gcnbr];
  pcolor = &colortab[gcnbr];
  syscmap = gdk_colormap_get_system();

  if (tabcolalloc[gcnbr])
    gdk_colormap_free_colors (syscmap, pcolor, 1);
  
  if ( gcnbr>=PXSTART && gcnbr<PXSTART+PXNBR ){
    tabpxpixmode[gcnbr] = FALSE;
    if ( (pixmap = tabpxpx[gcnbr])!=NULL ){
      tabpxpx[gcnbr] = NULL;
      gdk_pixmap_unref(pixmap);
    }  
  }
  
  pcolor->red = acolor->red;
  pcolor->green = acolor->green;
  pcolor->blue = acolor->blue;
  tabcolalloc[gcnbr] = gdk_colormap_alloc_color (syscmap, pcolor, FALSE, TRUE);
  gdk_gc_set_fill (gc, GDK_SOLID);
  gdk_gc_set_foreground (gc, pcolor);
  
}

/********************************/
/* config par defaut */
void tansetdefconfig (void){
  int i;

  guint backgroung_color_red   = 0xe8<<8;
  guint backgroung_color_green = 0xe7<<8;
  guint backgroung_color_blue  = 0xe2<<8;

  for (i = PXSTART; i<PXNBR+PXSTART; i++){
    tabpxpixmode[i] = FALSE;
    tanallocname(&tabpxnam[i], "NoConfigFile");
  }

  colortab[GCPETITEFG].red   = colortab[GCPETITEFG].green = colortab[GCPETITEFG].blue = 0;
  colortab[GCPETITEBG].red   = backgroung_color_red;
  colortab[GCPETITEBG].green = backgroung_color_green;
  colortab[GCPETITEBG].blue  = backgroung_color_blue;

  colortab[GCPIECENOR].red = colortab[GCPIECENOR].green = 32768;
  colortab[GCPIECENOR].blue = 50000;

  colortab[GCPIECEHI].red = colortab[GCPIECEHI].green = 40000;
  colortab[GCPIECEHI].blue = 65535;

  colortab[GCPIECEBG].red   = backgroung_color_red;
  colortab[GCPIECEBG].green = backgroung_color_green;
  colortab[GCPIECEBG].blue  = backgroung_color_blue;

  colortab[GCPIECEHLP].blue = colortab[GCPIECEHLP].green = 0;
  colortab[GCPIECEHLP].red = 65535;

  colortab[GCPETITECHK].blue = colortab[GCPETITECHK].red = 40000;
  colortab[GCPETITECHK].green = 60000;

  colortab[GCPETITEHLP].red = (colortab[GCPETITEFG].red+colortab[GCPETITEBG].red)/2;
  colortab[GCPETITEHLP].green = (colortab[GCPETITEFG].green+colortab[GCPETITEBG].green)/2;
  colortab[GCPETITEHLP].blue = (colortab[GCPETITEFG].blue+colortab[GCPETITEBG].blue)/2;
  
  /* Gcompris */
  GcomprisProperties	*properties = gcompris_get_properties();
  gchar *deffigfile = g_strconcat(properties->package_data_dir,"/gtans/figures/default.figures", NULL); 

  tanallocname(&figfilename, deffigfile);

  g_free(deffigfile);

  accuracy = 1;
  rotstepnbr = TOUR/32;

  figgrande.zoom = 0.125;

  return;
}


/********************************/
/* supprime 2 points successifs identiques (en principe inutile) */
gboolean tanremsame(tanflfig *flfig, tanpoly *polys, int *pntsuiv, tanfpnt *fpnts, double seuil){
  int i,k;
  gboolean trouve, ret;
  int polynbr;
  int act;
  int suiv;
  
  polynbr = flfig->flpiecenbr;

  ret = FALSE;
  trouve = TRUE;
  while (trouve){
    trouve = FALSE;
    for (i = 0; i<polynbr && !trouve; i++){
      act = polys[i].firstpnt;
      for (k = 0; k<polys[i].pntnbr && !trouve; k++){
	suiv = pntsuiv[act];
	if ( tandistcar(&fpnts[act],&fpnts[suiv])<seuil ){
	  pntsuiv[act] = pntsuiv[suiv];
	  polys[i].pntnbr--;
	  polys[i].firstpnt = act;
	  trouve = ret = TRUE;
	  printf ("j'en ai trouve un.\n");
	}
	act = suiv;
      }
    }
  }
  
  return (ret);
}


/* ajoute des point intermediaire */
gboolean tanajoute(tanflfig *flfig,
		   tanpoly *polys,
		   int *pntsuiv,
		   tanfpnt *fpnts,
		   double seuil,
		   int flptnew){
  int i, j, k, l;
  gboolean trouve, ret;
  tanfpnt segment[2];
  double dx, dy;
  int polynbr;
  int act1, act2;
  int suiv1, suiv2;
  
  polynbr = flfig->flpiecenbr;

  ret = FALSE;
  trouve = TRUE;
  while ( trouve && flptnew<FLPNTMAX ){
    trouve = FALSE;
    for (i = 0; i<polynbr && !trouve; i++){
      for (j = 0; j<polynbr && !trouve; j++){
	if (i!=j){
	  act1 = polys[i].firstpnt;
	  segment[0] = fpnts[act1];
	  for (k = 0; k<polys[i].pntnbr && !trouve; k++){
	    suiv1 = pntsuiv[act1];
	    segment[1] = fpnts[suiv1];
	    act2 = polys[j].firstpnt;
	    for (l = 0; l<polys[j].pntnbr && !trouve; l++){
	      suiv2 = pntsuiv[act2];
	      if ( tandistcar(&segment[0], &fpnts[act2])>seuil &&
		   tandistcar(&segment[1], &fpnts[act2])>seuil &&
		   tandistcarsegpnt(segment, &fpnts[act2], &dx, &dy)<seuil/4 ){
		fpnts[flptnew].posx = fpnts[act2].posx-dx;
		fpnts[flptnew].posy = fpnts[act2].posy-dy;
		pntsuiv[flptnew] = pntsuiv[act1];
		pntsuiv[act1] = flptnew;
		flptnew++;
		polys[i].pntnbr++;
		polys[i].firstpnt = act1;
		trouve = ret = TRUE;
	      }
	      act2 = suiv2;
	    }
	    act1 = suiv1;
	    segment[0] = segment[1];
	  }
	}
      }
    }
  }
  
  flfig->flpiecenbr = polynbr;

  return (ret);
}  


/* "tasse" les fpnt et recree la floatfig */
/* copie les point de ref dans cop */
/* en sortie les 2 sont identiques mais la floatfig pointe sur cop */
int tantasse(tanflfig *flfig,
	     tanpoly *polys,
	     int *pntsuiv,
	     tanfpnt *fpntsref,
	     tanfpnt *fpntscop){
  int i, j;
  int act, pntnbr;
  tanfpnt *fpnts;
  
  fpnts=fpntscop;
  for (i = 0; i<flfig->flpiecenbr; i++){
    pntnbr = polys[i].pntnbr;
    flfig->flpieces[i].flpntnbr = pntnbr;
    flfig->flpieces[i].flpnts = fpnts;
    flfig->flpieces[i].polytype = polys[i].polytype;
    act = polys[i].firstpnt;
    for (j = 0; j<pntnbr+1; j++){
      *fpnts++ = fpntsref[act];
      act = pntsuiv[act];
    }
  }

  act=0;
  for (i = 0; i<flfig->flpiecenbr; i++){
    pntnbr = polys[i].pntnbr;
    polys[i].firstpnt = act;
    for (j = 0; j<pntnbr-1; j++)
      pntsuiv[act+j] = act+j+1;
    pntsuiv[act+j] = act;
    act += pntnbr+1;
  }

  pntnbr = fpnts-fpntscop;
  for (i = 0; i<pntnbr; i++)
    *fpntsref++ = *fpntscop++;

  return (pntnbr);
}


/* supprime les points intermediaires de segments alignes */
gboolean tanalign(tanflfig *flfig, tanpoly *polys, int *pntsuiv, tanfpnt *fpnts){
  int i,k;
  gboolean trouve, ret;
  int polynbr;
  int act;
  int suiva,suivb;
  int diract,dirsuiv;
  int dumi;
  
  polynbr = flfig->flpiecenbr;

  ret = FALSE;
  trouve = TRUE;
  while (trouve){
    trouve = FALSE;
    for (i = 0; i<polynbr && !trouve; i++){
      act = polys[i].firstpnt;
      suiva = pntsuiv[act];
      dumi = tanangle(fpnts[suiva].posx-fpnts[act].posx, fpnts[suiva].posy-fpnts[act].posy);
      diract = (int)((dumi+rotstepnbr/2)/rotstepnbr);
      for (k = 0; k<polys[i].pntnbr && !trouve; k++){
	suiva = pntsuiv[act];
	suivb = pntsuiv[suiva];
	dumi = tanangle(fpnts[suivb].posx-fpnts[suiva].posx, fpnts[suivb].posy-fpnts[suiva].posy);
	dirsuiv = (int)((dumi+rotstepnbr/2)/rotstepnbr);
	if ( diract==dirsuiv ){
	  pntsuiv[act] = suivb;
	  polys[i].pntnbr--;
	  polys[i].firstpnt = act;
	  trouve = ret = TRUE;
	}
	act = suiva;
	diract = dirsuiv;
      }
    }
  }
  
  return (ret);
}


/* supprime les segments consecutifs superposes */
gboolean tanconseq(tanflfig *flfig, tanpoly *polys, int *pntsuiv, tanfpnt *fpnts, double seuil){
  int i,k;
  gboolean trouve, ret;
  int polynbr;
  int act;
  int suiva,suivb;
  
  polynbr = flfig->flpiecenbr;

  ret = FALSE;
  trouve = TRUE;
  while (trouve){
    trouve = FALSE;
    for (i = 0; i<polynbr && !trouve; i++){
      act = polys[i].firstpnt;
      for (k = 0; k<polys[i].pntnbr && !trouve; k++){
	suiva = pntsuiv[act];
	suivb = pntsuiv[suiva];
	if ( tandistcar(&fpnts[act],&fpnts[suivb])<seuil ){
	  pntsuiv[act] = pntsuiv[suivb];
	  polys[i].pntnbr -= 2;
	  polys[i].firstpnt = act;
	  trouve = ret = TRUE;
	}
	act = suiva;
      }
    }
  }
  
  return (ret);
}


/* concatene les poly ayant 1 segment commun */
gboolean tanconcat(tanflfig *flfig, tanpoly *polys, int *pntsuiv, tanfpnt *fpnts, double seuil){
  int i,j,k,l,m;
  gboolean trouve, ret;
  int polynbr;
  int act1,act2;
  int suiv1,suiv2;
  
  polynbr = flfig->flpiecenbr;

  ret = FALSE;
  trouve = TRUE;
  while (trouve){
    trouve = FALSE;
    for (i = 0; i<polynbr-1 && !trouve; i++){
      for (j = i+1; j<polynbr && !trouve; j++){
	act1 = polys[i].firstpnt;
	for (k = 0; k<polys[i].pntnbr && !trouve; k++){
	  suiv1 = pntsuiv[act1];
	  act2 = polys[j].firstpnt;
	  for (l = 0; l<polys[j].pntnbr && !trouve; l++){
	    suiv2 = pntsuiv[act2];
	    if ( tandistcar(&fpnts[act1],&fpnts[suiv2])<seuil &&
		 tandistcar(&fpnts[suiv1],&fpnts[act2])<seuil ){
	      pntsuiv[act1] = pntsuiv[suiv2];
	      pntsuiv[act2] = pntsuiv[suiv1];
	      polys[i].pntnbr += polys[j].pntnbr-2;
	      polys[i].firstpnt = act1;
	      for (m = j; m<polynbr-1; m++)
		polys[m] = polys[m+1];
	      polynbr--;
	      trouve = ret = TRUE;
	    }
	    act2 = suiv2;
	  }
	  act1 = suiv1;
	}
      }
    }
  }

  flfig->flpiecenbr = polynbr;

  return (ret);
}  


/* detecte les poly "inclus" */
/* probleme potentiel : pourrait ne pas detecter une inclusion
car on n'ajoute pas de points pour les 'auto-corespondance' */
gboolean taninclus(tanflfig *flfig, tanpoly *polys, int *pntsuiv, tanfpnt *fpnts, double seuil){
  int i,k,l,m,n;
  gboolean trouve, ret;
  int polynbr;
  int act1,act2;
  int suiv1,suiv2;
  int pntnbr;
  tanpoly dumpoly;
  double dumposxmin;
  int dumpntposxmin = 0;
  
  polynbr = flfig->flpiecenbr;

  trouve = ret = FALSE;
  for (i = 0; i<polynbr && !trouve; i++){
    pntnbr = polys[i].pntnbr;

    /*pour etre sur de partir de l'exterieur*/
    act1 = polys[i].firstpnt;
    dumposxmin = 99999999.0;
    for (m=0; m<pntnbr; m++){
      if ( fpnts[act1].posx<dumposxmin ){
	dumposxmin = fpnts[act1].posx;
	dumpntposxmin = act1;
      }
      act1 = pntsuiv[act1];
    }
    act1 = dumpntposxmin;

    for (k = 0; k<pntnbr-2 && !trouve; k++){
      suiv1 = pntsuiv[act1];
      act2 = pntsuiv[suiv1];
      for (l = k+2; l<pntnbr && !trouve; l++){
	suiv2 = pntsuiv[act2];
	if ( tandistcar(&fpnts[act1],&fpnts[suiv2])<seuil &&
	     tandistcar(&fpnts[suiv1],&fpnts[act2])<seuil ){
	  
	  pntsuiv[act1] = pntsuiv[suiv2];
	  pntsuiv[act2] = pntsuiv[suiv1];
	  
	  dumpoly = polys[i];
	  for (n = i; n<polynbr-1; n++)
	    polys[n] = polys[n+1];
	  
	  polynbr--;
	  
	  for (m = 0; polys[m].polytype==TAN_POLYBACK && m<polynbr; m++);
	  
	  /*	  printf("inclusion trouvee\n");*/

	  for (n = polynbr+1; n>m+1; n--)
	    polys[n] = polys[n-2];
	  
	  dumpoly.pntnbr -= l-k+1;
	  dumpoly.firstpnt = act1;
	  if (dumpoly.polytype!=TAN_POLYON)
	    dumpoly.polytype = TAN_POLYBACK;
	  else
	    dumpoly.polytype = TAN_POLYON;
	  
	  polys[m] = dumpoly;
	  
	  polys[m+1].pntnbr = l-k-1; 
	  polys[m+1].firstpnt = act2;
	  polys[m+1].polytype = TAN_POLYON;
	  
	  polynbr += 2;

	  trouve = ret = TRUE;


	}
	act2 = suiv2;
      }
      act1 = suiv1;
    }
  }
  
  
  flfig->flpiecenbr = polynbr;

  return (ret);
}  


/* change de petite figure */
/* pas d'acces gtk */
void tansetnewfigurepart1(int nrfig){

  tanfigure *figure;
  int i,j;
  double xmin=10000,xmax=-10000,ymin=10000,ymax=-10000;
  tanpiecepos *piecepos;
  tanflfig *flfig=&flfigpetite;
  int flpiecenbr,flpntnbr;
  tanfpnt *fpnts;
  tanfpnt dumfpnts[FLPNTMAX];
  tanpoly polys[PIECENBR];
  int polypntact,polypntnbr;
  int pntsuivants[FLPNTMAX];
  double seuil=0.00000000001;
  int pntnew;

  if ( nrfig>=0 && figtabsize ){
    nrfig %= figtabsize;
    actual_figure = nrfig;
    figure = figtab+nrfig;
  } else {
    if (nrfig==-1)
      figure = &figuredebut;
    else
      figure = &figpetite;
    nrfig = -1;
  }

  figactualnr = nrfig;

  helptanset=PIECENBR;
  figpetite=*figure;
  tancolle(&figpetite,0.02);
  tanmaketinytabnotr(&figpetite,tinytabpe);
  tantranstinytab(tinytabpe);

  /* la floatfig et preparation de la concatenation */
  flfig->flpiecenbr = PIECENBR;
  flfig->figure = figure;
  fpnts = dumfpnts;
  polypntact = 0;
  for (i = 0; i<PIECENBR; i++){
    polypntnbr = piecesdef[figure->piecepos[i].type].pntnbr;
    polys[i].pntnbr = polypntnbr;
    polys[i].firstpnt = polypntact;
    polys[i].polytype = TAN_POLYNORMAL;

    for (j = 0; j<polypntnbr-1; j++)
      pntsuivants[polypntact+j] = polypntact+j+1;
    pntsuivants[polypntact+j] = polypntact;
    polypntact += polypntnbr+1;

    tanplacepiecefloat(&figure->piecepos[i], fpnts,1);
    fpnts += polypntnbr+1;
  }  
  
  tanconcat(flfig, polys, pntsuivants, dumfpnts, seuil);
  tanconseq(flfig, polys, pntsuivants, dumfpnts, seuil);

  pntnew = tantasse(flfig, polys, pntsuivants, dumfpnts, fpntspetite);
  tanajoute(flfig, polys, pntsuivants, dumfpnts, seuil, pntnew);
  tanconcat(flfig, polys, pntsuivants, dumfpnts, seuil);
  tanconseq(flfig, polys, pntsuivants, dumfpnts, seuil);
  if (taninclus(flfig, polys, pntsuivants, dumfpnts, seuil))
    taninclus(flfig, polys, pntsuivants, dumfpnts, seuil);
  tanalign(flfig, polys, pntsuivants, dumfpnts);
  tanremsame(flfig, polys, pntsuivants, dumfpnts, seuil);
  
  pntnew = tantasse(flfig, polys, pntsuivants, dumfpnts, fpntspetite);
  tanajoute(flfig, polys, pntsuivants, dumfpnts, seuil, pntnew);
  tanconcat(flfig, polys, pntsuivants, dumfpnts, seuil);
  tanconseq(flfig, polys, pntsuivants, dumfpnts, seuil);
  if (taninclus(flfig, polys, pntsuivants, dumfpnts, seuil))
    taninclus(flfig, polys, pntsuivants, dumfpnts, seuil);
  tanalign(flfig, polys, pntsuivants, dumfpnts);
  tanremsame(flfig, polys, pntsuivants, dumfpnts, seuil);

  tantasse(flfig, polys, pntsuivants, dumfpnts, fpntspetite);

  /* calcul du centrage */
  flpiecenbr = flfig->flpiecenbr;
  for (i = 0; i<flpiecenbr; i++){
    fpnts = flfig->flpieces[i].flpnts;
    flpntnbr = flfig->flpieces[i].flpntnbr;
    for (j = 0; j<flpntnbr; j++){
      if (fpnts[j].posx>xmax)
	xmax=fpnts[j].posx;
      if (fpnts[j].posy>ymax)
	ymax=fpnts[j].posy;
      if (fpnts[j].posx<xmin)
	xmin=fpnts[j].posx;
      if (fpnts[j].posy<ymin)
	ymin=fpnts[j].posy;
    }
  }
  
  figpetite.zoom = 1/(( (xmax-xmin)>(ymax-ymin) ? (xmax-xmin) : (ymax-ymin) )+0.25);
  dxpetite = 0.5*(xmax+xmin)-(0.5/figpetite.zoom);
  dypetite = 0.5*(ymax+ymin)-(0.5/figpetite.zoom);
  
  dxout = 0.5*(xmax+xmin)-(0.5/figgrande.zoom);    /* cf tanrecentreout pour correction */
  dyout = 0.5*(ymax+ymin)-(0.5/figgrande.zoom);
  
  /* centrage des pieces petite */
  piecepos=figpetite.piecepos;
  for (i=0; i<PIECENBR; i++){
    piecepos->posx-=dxpetite;
    piecepos->posy-=dypetite;
    piecepos++;
  }
}


/********************************/
/* corrige dxout et dyout pour les changement de zoom de figgrande */
void tanrecentreout(double oldzoom, double newzoom){

  tanpiecepos *piecepos;
  int i;
  double correction;
  
  correction = 0.5*(1/oldzoom-1/newzoom);

  dxout += correction;
  dyout += correction;

  piecepos = figgrande.piecepos;
  for (i = 0; i<PIECENBR; i++){
    piecepos->posx -= correction;
    piecepos->posy -= correction;
    piecepos++;
  }
  
  return;
}


/********************************/
/* change de petite figure */
void tansetnewfigurepart2(void){
  
  if (selectedgrande){
    helpoutset=FALSE;
    tanunselect();
  }
  else if (helpoutset){       /* pour eviter 2 appels successif a tanredrawgrande */
    helpoutset=FALSE;
    tanredrawgrande();
  }

  tanredrawpetite();

  selpossible=TRUE;

}


/********************************/
void spesavefig (void){

 FILE *hand;
 int j;
 tanfigure *fig=NULL; /*juste pour eviter un warning*/

 if ( (hand=fopen("pouet.fig", "w"))!=NULL){

   fprintf(hand, "gTans v1.0 %d \n",figtabsize);

   fig=&figgrande;
   fprintf(hand,"%e %e %d \n",1.0,fig->distmax,fig->drotmax);
   for (j=0; j<PIECENBR; j++)
     fprintf(hand,"p %d %d %e %e %d \n",fig->piecepos[j].type,fig->piecepos[j].flipped,
	     fig->piecepos[j].posx,fig->piecepos[j].posy,fig->piecepos[j].rot);
 }

 if (hand!=NULL){
   fclose(hand);
   figpetite=*fig;
   figpetite.zoom=1;
   tansetnewfigurepart1(-2);
   tansetnewfigurepart2();
 }

}

/********************************/
void taninitstart(void){

  int i;
  char* accurstr;

  usergtdir = gcompris_get_current_board_dirname();
  mkdir(usergtdir, -1);         /* mmouais bof */

  for (i = PXSTART; i<PXNBR+PXSTART; i++){
    tabpxnam[i] = NULL;
    tabpxpx[i] = NULL;
  }

  for (i = 0; i<GCNBR; i++)
    tabcolalloc[i] = FALSE;
  
  editmode = FALSE;
  figgrande = figuredebut;
  figtabsize = 0;

  tansetnewfigurepart1(-1);

  tansetdefconfig();

  tanclampgrandefig();


  boardRootItem = GNOME_CANVAS_GROUP(gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type(),
							    "x", 0.0,
							    "y", 0.0,
							    NULL
							    ));

  create_mainwindow(boardRootItem);

  switch (accuracy){
  case 0:
    accurstr = "maccuracy1";
    break;
  case 2:
    accurstr = "maccuracy3";
    break;
  default :
    accurstr = "maccuracy2";
  }

  if (rotstepnbr==TOUR/256)
    accurstr = "mrotcont";
  else
    accurstr = "mrotstp";

  tanloadfigtab(figfilename);

}


/********************************/
void tanend(void){
  int i;
  GdkColormap *syscmap;

  syscmap = gdk_colormap_get_system();

  tansavefigstatus(figfilename, figtab, figtabsize);

  if (usergtdir!=NULL)
    g_free(usergtdir);

  if (userconf!=NULL)
    g_free(userconf);

  if (figfilename!=NULL)
    g_free(figfilename);

  if (figtab!=NULL)
    g_free(figtab);
  
  if (pixmappetite!=NULL)
    gdk_pixmap_unref(pixmappetite);
  if (pixmapgrande1!=NULL)
    gdk_pixmap_unref(pixmapgrande1);
  if (pixmapgrande2!=NULL)
    gdk_pixmap_unref(pixmapgrande2);
  if (pixmappiece1!=NULL)
    gdk_pixmap_unref(pixmappiece1);
  if (pixmappiece2!=NULL)
    gdk_pixmap_unref(pixmappiece2);
  if (pixmapfond!=NULL)
    gdk_pixmap_unref(pixmapfond);
  
  for (i=PXSTART; i<PXSTART+PXNBR; i++){
    if (tabpxpx[i]!=NULL)
      gdk_pixmap_unref(tabpxpx[i]);
    if (tabpxnam[i]!=NULL)
      g_free(tabpxnam[i]);
  }

  for (i = 0; i<GCNBR; i++){
    if (tabgc[i]!=NULL)
      gdk_gc_unref(tabgc[i]);
    if (tabcolalloc[i])
      gdk_colormap_free_colors (syscmap, &colortab[i], 1);
  }  

  gdk_gc_unref(invertgc);

  gtk_main_quit ();
}


/********************************/
void taninitcbcommun(void){

}


/********************************/
void taninitcbgr(void){
  int i;
  GdkColor *color;

  initcbgr = TRUE; /* pour ne pas initialiser 2 fois */

  for (i=PXSTART; i<PXSTART+PXNBR; i++){
    tabgc[i] = gdk_gc_new(widgetgrande->window);
    if (tabpxpixmode[i])
      tansetpixmapmode(widgetgrande,tabpxnam[i],i);
    else
      tansetcolormode(&colortab[i],i);
  }

  for (i=0; i<GRISNBR; i++){
    color = &colortab[i];
    color->red = color->green = color->blue = (gushort)(65535.0/(GRISNBR-1)*i);
    tabgc[i] = gdk_gc_new(widgetgrande->window);
    tansetcolormode(color,i);
  }

  invertgc=gdk_gc_new(widgetgrande->window);
  gdk_gc_set_function(invertgc,GDK_INVERT);
  tabgc[GCPIECEHLP]=gdk_gc_new(widgetgrande->window);
  tansetcolormode(&colortab[GCPIECEHLP],GCPIECEHLP);
  /* les line attribute sont dans le callback */

  if (initcbpe)
    taninitcbcommun();

}


/********************************/
void taninitcbpe(void){

  initcbpe=TRUE; /* pour ne pas initialiser 2 fois (c'est pas propre, mais bon) */

  tabgc[GCPETITEFG]=gdk_gc_new(widgetpetite->window);
  tansetcolormode(&colortab[GCPETITEFG],GCPETITEFG);

  tabgc[GCPETITEBG]=gdk_gc_new(widgetpetite->window);
  tansetcolormode(&colortab[GCPETITEBG],GCPETITEBG);

  tabgc[GCPETITEHLP]=gdk_gc_new(widgetpetite->window);
  tansetcolormode(&colortab[GCPETITEHLP],GCPETITEHLP);

  tabgc[GCPETITECHK]=gdk_gc_new(widgetpetite->window);
  tansetcolormode(&colortab[GCPETITECHK],GCPETITECHK);

  if (initcbgr)
    taninitcbcommun();

}


/********************************/
/* determine si le point x,y est dans la piece */
gboolean tanpntisinpiece(int x, int y, tanpiecepos *piecepos){

  int i;
  gboolean in;
  GdkPoint pnt[PNTNBRMAX+2];
  int nbrpnt;
  
  nbrpnt=tanplacepiece(piecepos,pnt,widgetgrande->allocation.width*figgrande.zoom);
  pnt[nbrpnt]=pnt[0];

  in=TRUE;
  if (piecepos->flipped){
    for (i=0; (i<nbrpnt && in); i++)
      if ( (x-pnt[i].x)*(pnt[i+1].y-pnt[i].y)-(y-pnt[i].y)*(pnt[i+1].x-pnt[i].x)<0 )
	in=FALSE;
  }
  else{
    for (i=0; (i<nbrpnt && in); i++)
      if ( (x-pnt[i].x)*(pnt[i+1].y-pnt[i].y)-(y-pnt[i].y)*(pnt[i+1].x-pnt[i].x)>0 )
	in=FALSE;
  }


  return (in);

}


/********************************/
/* determine dans quelle piece se trouve le point (-1=aucune) */
int tanwichisselect(int x, int y){
  
  int i;
  gboolean trouve;

  trouve=FALSE;
  for (i=PIECENBR-1; i>=0 && !trouve; i--)
    trouve=tanpntisinpiece(x,y,figgrande.piecepos+i);

  if (trouve)
    i++;

  return (i);

}
























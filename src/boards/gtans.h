

#define AN_none 0
#define AN_move 1
#define AN_rot 2

#define HELPFILE_PATH     HOMEDIR "gtanshelp%s.txt"
#define DEFAULTFIGFILE    FIGDIR "default.figures"
#define DEFAULTCONFIGFILE "/etc/gtansrc"
#define USERHOMEDIR       ".gtans"
#define USERCONFIG        ".gtansrc"

#define ARON 0.39999

#define mPI 3.14159265356
#define TOUR 65536
#define PASTOUR (mPI/TOUR*2)
#define CL 1.4142135624
#define CC CL/2
#define HT TOUR/8
#define TYPENBR 5    /* nombre de type de pieces */
#define PIECENBR 7   /* nombre de pieces par figure */
#define TRINBRMAX 4  /* nombre de triangle max par piece */
#define PNTNBRMAX 4  /* nombre de points max par piece */
#define TINYNBR 32   /* nombre de tiny triangles */

/* gc (couleur ou (couleur ou texture)) */
#define GRISNBR 8
#define GCPIECENOR  GRISNBR
#define GCPIECEHI   GRISNBR+1
#define GCPIECEBG   GRISNBR+2
#define GCPIECEHLP  GRISNBR+3
#define GCPETITEFG  GRISNBR+4
#define GCPETITEBG  GRISNBR+5
#define GCPETITECHK GRISNBR+6
#define GCPETITEHLP GRISNBR+7

#define GCPIECELAST   GCPIECEHLP
#define GCPETITEFIRST GCPETITEFG
#define GCNBR GCPETITEHLP+1

/* gc (couleur ou texture) */
#define PXNBR      3
#define PXSTART    GCPIECENOR


typedef enum
{
  TAN_PIECENOR=5,
  TAN_PIECEHI,
  TAN_PETITEFG,
  TAN_PETITEHLP
} tanremplis;

typedef enum
{
  TAN_POLYNORMAL=5,
  TAN_POLYBACK,
  TAN_POLYON,
} tanpolytype;

/* polygone (uniquement pour le calcul de la floatfig) */
typedef struct _tanpoly {
  int pntnbr;
  tanpolytype polytype;
  int firstpnt;
} tanpoly;

/* petit (et tiny) triangle */
typedef struct _tansmalltri tantinytri;
typedef struct _tansmalltri {
  double posx;
  double posy;
  int rot;
} tansmalltri;

/* point */
typedef struct _tanfpnt {
  double posx;
  double posy;
} tanfpnt;

/* definition d'une piece */
typedef struct _tanpiecedef {
  double handlex;     /* centre de gravite x */
  double handley;
  int trinbr;         /* nombre de petits triangles */
  tansmalltri tri[TRINBRMAX];
  int pntnbr;         /* nombre de points du polygone */
  tanfpnt pnt[PNTNBRMAX];
} tanpiecedef;

 /* position d'une piece */
typedef struct _tanpiecepos {
  int type;
  int flipped;
  double posx;
  double posy;
  int rot;
} tanpiecepos;

/* figure (jeu de pieces) */
typedef struct _tanfigure {
  double zoom;           /* non utilise */
  double distmax;        /* en principe ==1.0 */
  int drotmax;           /* non utilise */
  gboolean reussi;
  tanpiecepos piecepos[PIECENBR];
} tanfigure;

/* flpiece */
typedef struct _tanflpiece {
  int flpntnbr;
  tanpolytype polytype;
  tanfpnt *flpnts;
} tanflpiece;

/* flfig */
typedef struct _tanflfig {
  tanfigure *figure;
  int flpiecenbr;
  tanflpiece flpieces[PIECENBR];
} tanflfig;



extern GtkWidget *mainwindow;
extern GtkWidget *widgetgrande;        /* widget de la grande drawarea */
extern GtkWidget *widgetpetite;        /* widget de la petite drawarea */
extern GtkStatusbar *widgetstat;       /* widget de la statusbar */
extern GtkSpinButton *spinner;

extern gint statconid;                 /* context id de la statusbar */
extern GdkRectangle selbackrect;       /* rectangle a redessiner pour effacer la piece selected */

extern GdkPixmap *pixmapgrande1,*pixmapgrande2;
extern GdkPixmap *pixmappetite;
extern GdkPixmap *pixmappiece1,*pixmappiece2,*pixmapfond;

extern GtkWidget *colselwin,*filselwin;


extern tanpiecedef piecesdef[];

extern tanfigure figuredebut;

extern tanfigure *figtab;
extern int figtabsize;
extern int figactualnr;
extern char *figfilename;

extern tantinytri tinytabpe[TINYNBR],tinytabgr[TINYNBR];
extern tanfigure figgrande,figpetite;
extern int selectedgrande;
extern int xact,yact,xoth,yoth,xold,yold;
extern int actiongrande;
extern int selpossible;
extern int rotact,rotnew,rotold;

extern int rotstepnbr;            /* nb. de pas de rotation */
extern int initcbgr,initcbpe;     /* init cb deja appellee ? */

extern GdkGC *invertgc;
extern int invx2,invy2;

extern GdkGC *tabgc[GCNBR];
extern GdkColor colortab[GCNBR];
extern GdkPixmap **tabpxpx;
extern char **tabpxnam;

extern gboolean helpoutset;
extern gboolean helptanset;
extern int accuracy;

extern gboolean editmode;


void     tanclampgrandefig (void);
void     tanrecentreout(double oldzoom, double newzoom);
void     tanclearreussinr (int fignr);
void     tansetreussiactual (void);
void     tanreleaseifrot (void);
int      tanangle (double dx, double dy);
void     spesavefig (void);
void     tanend(void);
gboolean tanloadfigtab (char *name);
gboolean tansetpixmapmode(GtkWidget *widget, char *aname, int gcnbr);
void     tansetcolormode(GdkColor *acolor, int gcnbr);
void     tanredrawgrande (void);
void     tanredrawpetite (void);
gboolean tansaveconfig (void);
void     taninitcbgr(void);
void     taninitcbpe(void);
int      tanwichisselect(int x, int y);
void     tansetnewfigurepart2(void);
void     tansetnewfigurepart1(int nrfig);
void     tanunselect (void);
void     tandrawselect(int dx, int dy, int drot);
void     taninitselect(int selected, gboolean necessaire);
void     tantranstinytab (tantinytri *tinys);
void     tanmaketinytabnotr (tanfigure *figure, tantinytri *tinys);
gboolean tantinytabcompare (tantinytri *tinys1, tantinytri *tinys2, int accuracy);
void     tancolle (tanfigure *figure, double seuil);
void     tanstatprov (gchar *msg);
void     tanstatpop (void);
void     tanstatpush (gchar *msg);















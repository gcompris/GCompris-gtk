
#define BOUTON "awele/bouton1.png"	//Chemin relatif vers fichiers boutons
#define BOUTON_NOTIFY "awele/bouton1_notify.png"	//Chemin relatif vers fichiers boutons cliqués
#define BOUTON_CLIC "awele/bouton1_clic.png"	//Chemin relatif vers fichiers boutons cliqués
#define NEWGAME "awele/nv_partie.png"
#define NEWGAME_NOTIFY "awele/nv_partie_notify.png"
#define NEWGAME_CLIC "awele/nv_partie_clic.png"
#define BEAN "awele/graine1.png"	//Chemin relatif vers fichiers graines

#define Y_BOUTONS 412		//Abcisse des boutons

#define WIDTH 800 	// Largeur Fenetre
#define HEIGHT 600	// Hauteur Fenetre
/**
*	Tableau de graines
*/
typedef struct {
	GnomeCanvasItem *beanPixbuf;
	char hole;
}BEANHOLE_LINK;


typedef struct {
    GnomeCanvasItem *msg;	//Item message pour dialogue avec utilisateur   
    GnomeCanvasItem *nbBeansHole[12];	//Tableau d'item affichant le nbre de graine par trou.
    BEANHOLE_LINK *ptBeansHoleLink;	//pointeur sur structures stockant les item graines et la case dans laquelle elles se trouvent.
    GnomeCanvasItem *button[6];	//Item des boutons (affichés avec pixbufButton) 
    GdkPixbuf *pixbufButtonNotify[6];	//pixbux des boutons notifiés
    GdkPixbuf *pixbufButton[6];	//pixbuf des boutons(selection de la case a jouer)
    GdkPixbuf *pixbufButtonClicked[6];	//pixbuf des boutons cliqués
    GnomeCanvasItem *Captures[2];	//Tableau d'item affichage nbre graine capturées.
    GdkPixbuf *pixbufBeans[4];	//pixbufs des graines
    GdkPixbuf *pixbufButtonNewGame;
    GdkPixbuf *pixbufButtonNewGameNotify;
    GdkPixbuf *pixbufButtonNewGameClicked;
    GnomeCanvasItem *ButtonNewGame;
} GRAPHICS_ELT;

typedef struct {
    short int numeroCase;
    GRAPHICS_ELT *graphsElt;
} CALLBACK_ARGS;

/* 
*	Fonctions de traitement des évènements, signaux et rappels
*/
gint eventDelete (GtkWidget * widget, GdkEvent *event, gpointer data);
gint eventDestroy (GtkWidget * widget, GdkEvent *event, gpointer data);
gint buttonClick (GtkWidget *item, GdkEvent *event, gpointer data);
gint buttonNewGameClick (GtkWidget *item, GdkEvent *event, gpointer data);

/**
*	Fonctions Mise a jour de l'affichage
*/
BEANHOLE_LINK * updateNbBeans (GnomeCanvasItem *nbBeansHole[NBHOLE], GnomeCanvasGroup *rootGroup, BEANHOLE_LINK *ptLink, int alpha);
void updateCapturedBeans (GnomeCanvasItem *Captures[2]);
void initBoardGraphics (GRAPHICS_ELT *graphsElt);

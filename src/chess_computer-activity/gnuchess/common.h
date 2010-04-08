/* GNU Chess 5.0 - common.h - common symbolic definitions
   Copyright (c) 1999-2002 Free Software Foundation, Inc.

   GNU Chess is based on the two research programs 
   Cobalt by Chua Kong-Sian and Gazebo by Stuart Cracraft.

   GNU Chess is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GNU Chess is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Chess; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Contact Info: 
     bug-gnu-chess@gnu.org
     cracraft@ai.mit.edu, cracraft@stanfordalumni.org, cracraft@earthlink.net
*/

#ifndef COMMON_H
#define COMMON_H

#include <config.h>

#ifndef __GNUC__
# define __attribute__(x)
#endif

/*
 * Include "uint64_t" and similar types using the ac_need_stdint_h ac macro
 */

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

 /* 
  * Define time structures to get timeval for Timer 
  */

#include <sys/time.h>

 /*
  * Define macro for declaring 64bit constants for compilers not using ULL
  */

#ifdef _MSC_VER
   #define ULL(x) ((uint64_t)(x))
#else
   #define ULL(x) x ## ULL
#endif

/*
 * BitBoard is a key data type.  It is a 64-bit value, in which each
 * bit represents a square on the board as defined by "enum Square".
 * Thus, bit position 0 represents a fact about square a1, and bit
 * position 63 represents a fact about square h8.  For example, the
 * bitboard representing "white rook" positions will have a bit set
 * for every position occupied on the board by a white rook.
 */

typedef uint64_t BitBoard;
typedef uint64_t HashType;
typedef uint32_t KeyType;

/*
 * Board represents an entire board's state, and is structured to
 * simplify analysis:
 */

typedef struct 
{
   BitBoard b[2][7];      /* piece/pawn positions by side (0=white, 1=black)
                             and then by piece (1=pawn..6=king). For example,
                             b[white][knight] has bits set for every board
                             position occupied by a White Knight. */
   BitBoard friends[2];   /* Friendly (this side's) pieces */
   BitBoard blocker;
   BitBoard blockerr90;   /* rotated 90 degrees */
   BitBoard blockerr45;   /* rotated 45 degrees */
   BitBoard blockerr315;  /* rotated 315 degrees */
   short ep;              /* Location of en passant square */
   short flag;            /* Flags related to castle privileges */
   short side;            /* Color of side on move: 0=white, 1=black */
   short material[2];     /* Total material by side not inc. king */
   short pmaterial[2];    /* Total pawn material by side not inc. king */
   short castled[2];      /* True (1) if side is castled */
   short king[2];         /* Location of king 0 - a1 .. 63 - h8 */
} Board; 

/* leaf describes a leaf-level analysis result */

typedef struct
{
   int move;   /* the move that produced this particular board */
   int score;  /* the scored value of this leaf */
} leaf;

/*
 * GameRec records an individual move made in the game; an entire
 * Game is a set of GameRec's:
 */

#define SANSZ 8 /* longest move is "exf8=Q+" so 7+1 */

typedef struct
{
   int move;    /* The actual move made; this is NOT the move count! */
   short epsq;  /* en passant square */
   short bflag; /* Flags for castle privs, see Board.flag */
   short Game50; /* The last value of GameCnt (a count of half-moves)
                    that castled, captured, or moved a pawn */
   short mvboard;
   float et;     /* elapsed time */
   HashType hashkey;
   HashType phashkey;
   char SANmv[SANSZ];  /* The move in SAN notation */
   char *comments;
} GameRec;

typedef struct
{
   HashType key;    /* Full 64 bit hash */
   int move;        /* Best move */
   int score;       /* Estimated score, may be lower or upper bound */
   uint8_t flag;    /* Is this alpha, beta, quiescent or exact? */
   uint8_t depth;   /* Search depth */
} HashSlot;   

typedef struct
{
   KeyType pkey;  
   BitBoard passed;
   BitBoard weaked;
   int score;
   int phase;
} PawnSlot;


/*  MACRO definitions */

#ifndef MAX
#define MAX(a,b)     ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)     ((a) < (b) ? (a) : (b))
#endif
#define SET(a,b)                     \
  do {                               \
    (a) |= (b);                      \
    dbg_printf("Set   0x%x\n", (b)); \
  } while (0)
#define CLEAR(a,b)                   \
  do {                               \
    (a) &= ~(b);                     \
    dbg_printf("Clear 0x%x\n", (b)); \
  } while (0)

/* Draw score can be used to penalise draws if desired */
#define	DRAWSCORE   0 
#define MATERIAL     (board.material[side] - board.material[1^side])
#define PHASE	     (8 - (board.material[white]+board.material[black]) / 1150)
#define KEY(a)	     (a >> 32) 

/*  Attack MACROS */

#define BishopAttack(sq) \
	(Bishop45Atak[sq][(board.blockerr45 >> Shift45[sq]) & Mask45[sq]] | \
	 Bishop315Atak[sq][(board.blockerr315 >> Shift315[sq]) & Mask315[sq]])
#define RookAttack(sq)	\
	(Rook00Atak[sq][(board.blocker >> Shift00[sq]) & 0xFF] | \
         Rook90Atak[sq][(board.blockerr90 >> Shift90[sq]) & 0xFF])
#define QueenAttack(sq)	\
	(BishopAttack(sq) | RookAttack(sq))


/*  Some bit macros  */

/*
 * gcc 2.95.4 completely screws up the macros with lookup tables 
 * with -O2 on PPC, maybe this check has to be refined. (I don't know
 * whether other architectures also suffer from this gcc bug.) However,
 * with gcc 3.0, the lookup tables are _much_ faster than this direct
 * calculation.
 */
#if defined(__GNUC__) && defined(__PPC__) && __GNUC__ < 3
#  define SETBIT(b,i)   ((b) |=  ((ULL(1)<<63)>>(i)))
#  define CLEARBIT(b,i) ((b) &= ~((ULL(1)<<63)>>(i)))
#else
#  define SETBIT(b,i)   ((b) |= BitPosArray[i])
#  define CLEARBIT(b,i) ((b) &= NotBitPosArray[i])
#endif

#define RANK(i) ((i) >> 3)
#define ROW(i) ((i) & 7)
#define trailz(b) (leadz ((b) & ((~b) + 1)))

/* Move Descriptions (moves) are represented internally as integers.
 * The lowest 6 bits define the destination ("TO") square, and
 * the next lowest 6 bits define the source ("FROM") square,
 * using the values defined by "enum Square" (0=a1, 63=h8).
 * Upper bits are used to identify other move information such as
 * a promotion (and to what), a capture (and of what),
 * CASTLING moves, and ENPASSANT moves; see the "constants for
 * move description" below for more information on these upper bits.
 */
#define PROMOTEPIECE(a) ((a >> 12) & 0x0007)
#define CAPTUREPIECE(a) ((a >> 15) & 0x0007)
#define TOSQ(a)         ((a) & 0x003F)
#define FROMSQ(a)       ((a >> 6) & 0x003F)
#define MOVE(a,b)       (((a) << 6) | (b))

/* constants for move description */
#define KNIGHTPRM     0x00002000
#define BISHOPPRM     0x00003000 
#define ROOKPRM       0x00004000
#define QUEENPRM      0x00005000
#define PROMOTION     0x00007000
#define PAWNCAP       0x00008000
#define KNIGHTCAP     0x00010000 
#define BISHOPCAP     0x00018000
#define ROOKCAP       0x00020000 
#define QUEENCAP      0x00028000 
#define CAPTURE       0x00038000 
#define NULLMOVE      0x00100000 
#define CASTLING      0x00200000 
#define ENPASSANT     0x00400000
#define MOVEMASK      (CASTLING | ENPASSANT | PROMOTION | 0x0FFF)

#define white  0
#define black  1
#define false  0
#define true   1
#define ks 0
#define qs 1
#define INFINITY  32767
#define MATE	  32767
#define MATESCORE(a)	((a) > MATE-255  || (a) < -MATE+255)

/* constants for Board */
#define WKINGCASTLE   0x0001
#define WQUEENCASTLE  0x0002
#define BKINGCASTLE   0x0004
#define BQUEENCASTLE  0x0008
#define WCASTLE	      (WKINGCASTLE | WQUEENCASTLE)
#define BCASTLE	      (BKINGCASTLE | BQUEENCASTLE)

/* Material values */
#define ValueP   100	
#define ValueN   350
#define ValueB   350
#define ValueR   550
#define ValueQ   1100
#define ValueK   2000


/*  Some special BitBoards  */
#define NULLBITBOARD  ( ULL(0x0000000000000000))
#define WHITESQUARES  ( ULL(0x55AA55AA55AA55AA))
#define BLACKSQUARES  ( ULL(0xAA55AA55AA55AA55))
#define CENTRESQUARES ( ULL(0x0000001818000000))
#define COMPUTERHALF  ( ULL(0xFFFFFFFF00000000))
#define OPPONENTHALF  ( ULL(0x00000000FFFFFFFF))

/*  Game flags */
#define QUIT    0x0001
#define TESTT   0x0002
#define THINK   0x0004
#define MANUAL  0x0008
#define TIMEOUT 0x0010
#define SPARE1  0x0020
#define ENDED   0x0040
#define USEHASH 0x0080
#define SOLVE   0x0100
#define USENULL 0x0200
#define XBOARD  0x0400
#define TIMECTL 0x0800
#define POST    0x1000
#define PONDER  0x2000 /* We are in pondering (during search) */
#define HARD    0x4000 /* Pondering is turned on */
#define ANALYZE 0x8000 /* In ANALYZE mode */

/*  Node types  */ 
#define PV  0
#define ALL 1
#define CUT 2

/*  Transposition table flags  */
#define EXACTSCORE  1
#define LOWERBOUND  2
#define UPPERBOUND  3
#define POORDRAFT   4
#define QUIESCENT   5
#define STICKY      8

/*  Book modes */
#define BOOKOFF 0
#define BOOKRAND 1
#define BOOKBEST 2
#define BOOKWORST 3
#define BOOKPREFER 4

/*  The various phases during move selection  */
#define PICKHASH    1
#define PICKGEN1    2
#define PICKCAPT    3
#define PICKKILL1   4
#define PICKKILL2   5
#define PICKGEN2    6
#define PICKHIST    7
#define PICKREST    8
#define PICKCOUNTER 9

#define MAXTREEDEPTH  2000
#define MAXPLYDEPTH   65
#define MAXGAMEDEPTH  600

/* 
   Smaller HASHSLOT defaults 20011017 to improve blitz play
   and make it easier to run on old machines
*/
#define HASHSLOTS 1024 
#define PAWNSLOTS 512

extern short distance[64][64];
extern short taxicab[64][64];
extern unsigned char lzArray[65536];
extern short Shift00[64];
extern short Shift90[64];
extern short Shift45[64];
extern short Shift315[64];
extern BitBoard DistMap[64][8];
extern BitBoard BitPosArray[64];
extern BitBoard NotBitPosArray[64];
extern BitBoard MoveArray[8][64];
extern BitBoard Ray[64][8];
extern BitBoard FromToRay[64][64];
extern BitBoard RankBit[8];
extern BitBoard FileBit[8];
extern BitBoard Ataks[2][7];
extern BitBoard PassedPawnMask[2][64];
extern BitBoard IsolaniMask[8];
extern BitBoard SquarePawnMask[2][64];
extern BitBoard Rook00Atak[64][256]; 
extern BitBoard Rook90Atak[64][256]; 
extern BitBoard Bishop45Atak[64][256];
extern BitBoard Bishop315Atak[64][256];
extern BitBoard pinned;
extern BitBoard rings[4];
extern BitBoard boxes[2];
extern BitBoard stonewall[2];
extern BitBoard pieces[2];
extern BitBoard mask_kr_trapped_w[3];
extern BitBoard mask_kr_trapped_b[3];
extern BitBoard mask_qr_trapped_w[3];
extern BitBoard mask_qr_trapped_b[3];
extern BitBoard boardhalf[2];
extern BitBoard boardside[2];
extern short directions[64][64];
extern unsigned char BitCount[65536];
extern leaf Tree[MAXTREEDEPTH];
extern leaf *TreePtr[MAXPLYDEPTH];
extern int RootPV;
extern GameRec Game[MAXGAMEDEPTH];
extern int RealGameCnt;
extern short RealSide;
extern int GameCnt;
extern int computer;
extern unsigned int flags;
extern unsigned int preanalyze_flags;
extern Board board;
extern int cboard[64];
extern int Mvboard[64];
extern HashType hashcode[2][7][64];
extern HashType ephash[64];
extern HashType WKCastlehash;
extern HashType WQCastlehash;
extern HashType BKCastlehash;
extern HashType BQCastlehash;
extern HashType Sidehash;
extern HashType HashKey;
extern HashType PawnHashKey;
extern HashSlot *HashTab[2];
extern PawnSlot *PawnTab[2];
extern int Idepth;
extern int SxDec;
extern int Game50;
extern int lazyscore[2];
extern int maxposnscore[2];
extern int rootscore;
extern int lastrootscore;
extern unsigned long GenCnt;
extern unsigned long NodeCnt;
extern unsigned long QuiesCnt;
extern unsigned long EvalCnt;
extern unsigned long EvalCall;
extern unsigned long ChkExtCnt;
extern unsigned long OneRepCnt;
extern unsigned long RcpExtCnt;
extern unsigned long PawnExtCnt;
extern unsigned long HorzExtCnt;
extern unsigned long ThrtExtCnt;
extern unsigned long KingExtCnt;
extern unsigned long NullCutCnt;
extern unsigned long FutlCutCnt;
extern unsigned long RazrCutCnt;
extern unsigned long TotalGetHashCnt;
extern unsigned long GoodGetHashCnt;
extern unsigned long TotalPutHashCnt;
extern unsigned long CollHashCnt;
extern unsigned long TotalPawnHashCnt;
extern unsigned long GoodPawnHashCnt;
extern unsigned long RepeatCnt;
extern unsigned HashSize;
extern unsigned long TTHashMask;
extern unsigned long PHashMask;
extern int slider[8];
extern int Value[7];
extern char SANmv[SANSZ];
extern unsigned long history[2][4096];
extern int killer1[MAXPLYDEPTH];
extern int killer2[MAXPLYDEPTH];
extern int ChkCnt[MAXPLYDEPTH];
extern int ThrtCnt[MAXPLYDEPTH];
extern char id[32];
extern char solution[64];
extern float SearchTime;
extern int SearchDepth;
extern int MoveLimit[2];
extern float TimeLimit[2];
extern int TCMove;
extern int TCinc;
extern float TCTime;
extern int hunged[2];
extern int phase;
extern int Hashmv[MAXPLYDEPTH];
extern short RootPieces;
extern short RootPawns;
extern short RootMaterial;
extern short RootAlpha;
extern short RootBeta;
extern short pickphase[MAXPLYDEPTH];
extern short InChk[MAXPLYDEPTH];
extern short KingThrt[2][MAXPLYDEPTH];
extern short KingSafety[2];
extern short pscore[64];

extern short bookmode;
extern short bookfirstlast;

extern int range[8];
extern int ptype[2];
extern char algbr[64][3];
extern char algbrfile[9];
extern char algbrrank[9];
extern char notation[8];
extern char lnotation[8];
extern int r90[64];
extern int r45[64];
extern int r315[64];
extern int Mask45[64];
extern int Mask315[64];

extern int rank6[2];
extern int rank7[2];
extern int rank8[2];

extern char *progname;
extern FILE *ofp;
extern int myrating, opprating, suddendeath;

#define MAXNAMESZ 50
extern char name[MAXNAMESZ];
extern int computerplays;
extern int wasbookmove;
extern int nmovesfrombook;
extern float maxtime;
extern int n; 		/* Last mobility returned by CTL */
extern int ExchCnt[2];
extern int newpos, existpos;		/* For book statistics */
extern int bookloaded;

enum Piece { empty, pawn, knight, bishop, rook, queen, king, bpawn };

enum Square { A1, B1, C1, D1, E1, F1, G1, H1,
	      A2, B2, C2, D2, E2, F2, G2, H2,
	      A3, B3, C3, D3, E3, F3, G3, H3,
	      A4, B4, C4, D4, E4, F4, G4, H4,
	      A5, B5, C5, D5, E5, F5, G5, H5,
	      A6, B6, C6, D6, E6, F6, G6, H6,
	      A7, B7, C7, D7, E7, F7, G7, H7,
	      A8, B8, C8, D8, E8, F8, G8, H8 };

enum File { A_FILE, B_FILE, C_FILE, D_FILE, E_FILE, F_FILE, G_FILE, H_FILE };

/****************************************************************************
 *
 *  The various function prototypes.  They are group into the *.c files
 *  in which they are defined.
 *
 ****************************************************************************/

/*
 * Explanation of the #ifdef NO_INLINE conditionals:
 *
 * Define NO_INLINE only if you really must, implementations will be
 * provided by the corresponding *.c files. The better solution is to
 * not define it, in which case inlines.h will be included which
 * provides static inline version of these functions.
 */

/*  The initialization routines  */
void Initialize (void);
void InitLzArray (void);
void InitBitPosArray (void);
void InitMoveArray (void);
void InitRay (void);
void InitFromToRay (void);
void InitRankFileBit (void);
void InitBitCount (void);
void InitPassedPawnMask (void);
void InitIsolaniMask (void);
void InitSquarePawnMask (void);
void InitRandomMasks (void);
void InitRotAtak (void);
void InitDistance (void);
void InitVars (void);
void InitHashCode (void);
void InitHashTable (void);
void CalcHashSize (int);
void NewPosition (void);
void InitFICS (void);
void InitInput (void);

/* Cleanup routines */
void CleanupInput(void);

/*  The book routines */
int BookQuery (int);
int BookBuilderOpen(void);
int BookBuilder (short result, uint8_t side);
int BookBuilderClose(void);

/*
 * Return values (errorcodes) for the book routines,
 * maybe one should have a global enum of errorcodes
 */
enum {
  BOOK_SUCCESS,
  BOOK_EFORMAT,  /* Wrong format (e.g. produced by older version) */
  BOOK_EMIDGAME, /* Move is past the opening book's move limit */ 
  BOOK_EIO,      /* I/O error, e.g. caused by wrong permissions */
  BOOK_EFULL,    /* Book hash is full, new position was not added. */
  BOOK_ENOBOOK,  /* No book present */
  BOOK_ENOMOVES, /* No moves found (in BookQuery() only) */
  BOOK_ENOMEM    /* Memory allocation failed */
};
 
/*  The move generation routines  */
void GenMoves (short);
void GenCaptures (short);
void GenNonCaptures (short);
void GenCheckEscapes (short);
void FilterIllegalMoves (short);

/*  The move routines  */
void MakeMove (int, int *);
void UnmakeMove (int, int *);
void MakeNullMove (int);
void UnmakeNullMove (int);
void SANMove (int, int);
leaf *ValidateMove (char *);
leaf *IsInMoveList (int, int, int, char);
int IsLegalMove (int);
char *AlgbrMove (int);

/*  The attack routines  */
short SqAtakd (short sq, short side);
void GenAtaks (void);
BitBoard AttackTo (int, int);
BitBoard AttackXTo (int, int);
BitBoard AttackFrom (int, int, int);
BitBoard AttackXFrom (int, int);
int PinnedOnKing (int, int);
void FindPins (BitBoard *);
int MateScan (int);

/*  The swap routines  */
int SwapOff (int);
void AddXrayPiece (int, int, int, BitBoard *, BitBoard *);

/*  The EPD routines  */
short ReadEPDFile (const char *, short);
int ParseEPD (char *);
void LoadEPD (char *);
void SaveEPD (char *);

/* Error codes for ParseEPD */
enum {
   EPD_SUCCESS,
   EPD_ERROR
};

void UpdateFriends (void);
void UpdateCBoard (void);
void UpdateMvboard (void);
void EndSearch (int);
short ValidateBoard (void);

/*  PGN routines  */
void PGNSaveToFile (const char *, const char *);
void PGNReadFromFile (const char *);
void BookPGNReadFromFile (const char *);
int IsTrustedPlayer(const char *name);


/*  The hash routines  */
void CalcHashKey (void);
void ShowHashKey (HashType);

/*  The evaluation routines  */
int ScoreP (short);
int ScoreN (short);
int ScoreB (short);
int ScoreR (short);
int ScoreQ (short);
int ScoreK (short);
int ScoreDev (short);
int Evaluate (int, int);
short EvaluateDraw (void);

/*  Hung routines  */
int EvalHung (int);

/*  The search routines  */
void Iterate (void);
int Search (uint8_t ply, short depth, int alpha, int beta, short nodetype);
int SearchRoot (short depth, int alpha, int beta);
int Quiesce (uint8_t ply, int alpha, int beta);
void pick (leaf *, short);
short Repeat (void);
void ShowLine (int, int, char);

/*
 * Set up a timer by first calling StartTiming(), saving
 * the return value and feeding it to GetElapsed() for
 * timings in seconds.
 */
typedef struct timeval Timer;
extern double ElapsedTime;
extern Timer StartTime;
Timer StartTiming (void);
double GetElapsed (Timer start);

/*  The transposition table routies */
void TTPut (uint8_t side, uint8_t depth, uint8_t ply, 
	    int alpha, int beta, int score, int move);
/* Returns flag if it finds an entry, 0 otherwise */
uint8_t TTGet (uint8_t side, uint8_t depth, uint8_t ply,
	     int *score, int *move);
short TTGetPV (uint8_t side, uint8_t ply, int score, int *move);
void TTClear (void);
void PTClear (void);

/*  Sorting routines  */
void SortCaptures (int);
void SortMoves (int);
void SortRoot (void);
int PhasePick (leaf **, int);
int PhasePick1 (leaf **, int);

/*  Some output routines */
void ShowMoveList (int);
void ShowSmallBoard (void);
void ShowBoard (void);
void ShowBitBoard (BitBoard *);
void ShowCBoard (void);
void ShowMvboard (void);

void ShowGame (void);
void ShowTime (void);

/*  Random numbers routines */
uint32_t Rand32 (void);
HashType Rand64 (void);

/*  Solver routines  */
void Solve (char *);

/*  Test routines  */
void TestMoveGenSpeed (void);
void TestNonCaptureGenSpeed (void);
void TestCaptureGenSpeed (void);
void TestMoveList (void);
void TestNonCaptureList (void);
void TestCaptureList (void);
void TestEvalSpeed (void);
void TestEval (void);

/* Player database */
void DBSortPlayer (const char *style);
void DBListPlayer (const char *style); 	
void DBReadPlayer (void);	
void DBWritePlayer (void);
int DBSearchPlayer (const char *player);
void DBUpdatePlayer (const char *player, const char *resultstr);
void DBTest (void);

/* Input thread and thread function */
#include <pthread.h>
extern pthread_t input_thread;
void *input_func(void *);

/*
 * Status variable used by the input thread to signal
 * pending input. Thought about using flags for this
 * but this can be refined and is conceptually different
 * from flags.
 */
enum {
  INPUT_NONE,
  INPUT_AVAILABLE
};
extern volatile int input_status;

/*
 * Function to wake up the input thread, should be called after
 * input has been parsed.
 */
void input_wakeup(void);

/* Wait for input. */

void wait_for_input(void);

/*
 * Input routine, initialized to one of the specific
 * input routines. The given argument is the prompt.
 */
void (*getline_intrl) (char *);

#define MAXSTR 128
extern char inputstr[MAXSTR];

/* Input parser */
void parse_input(void);

/* Pondering */
void ponder(void);

/*  The command subroutines */
void ShowCmd (char *);
void BookCmd (char *);
void TestCmd (char *);

/* Commands from the input engine */
void cmd_accepted(void);
void cmd_activate(void); 
void cmd_analyze(void);
void cmd_bk(void);
void cmd_black(void);
void cmd_book(void);
void cmd_computer(void);
void cmd_depth(void);
void cmd_draw(void);
void cmd_easy(void);
void cmd_edit(void); 
void cmd_epd(void);
void cmd_exit(void);
void cmd_force(void);
void cmd_go(void);
void cmd_hard(void);
void cmd_hash(void);
void cmd_hashsize(void);
void cmd_help (void);
void cmd_hint(void);
void cmd_level(void);
void cmd_list(void);
void cmd_load(void);
void cmd_manual(void);
void cmd_movenow(void);
void cmd_name(void);
void cmd_new(void);
void cmd_nopost(void);
void cmd_null(void);
void cmd_otim(void);
void cmd_pgnload(void);
void cmd_pgnsave(void);
void cmd_ping(void);
void cmd_post(void);
void cmd_protover(void);
void cmd_quit(void);
void cmd_random(void);
void cmd_rating(void);
void cmd_rejected(void);
void cmd_remove(void);
void cmd_result(void);
void cmd_save(void);
void cmd_setboard(void);
void cmd_show (void);
void cmd_solve(void);
void cmd_st(void); 
void cmd_switch(void);
void cmd_test (void);
void cmd_time(void);
void cmd_undo(void);
void cmd_usage(void);
void cmd_variant(void);
void cmd_version(void);
void cmd_white(void);
void cmd_xboard(void);

/*
 * Define NO_INLINE only if you really must, implementations will be
 * provided by the corresponding *.c files. The better solution is to
 * not define it, in which case inlines.h will be included which
 * provides static inline version of these functions.
 */

/*  Some utility routines  */
#ifdef NO_INLINE
unsigned char leadz (BitBoard);
unsigned char nbits (BitBoard);
#else
# include "inlines.h"
#endif

/* More elaborate debugging output to logfile */

/* All the following functions are no-ops if DEBUG is not defined */

/*
 * dbg_open() can be called with NULL as argument, using a default
 * filename, defined in debug.c, for the debug log. Otherwise the
 * argument is the filename. If dbg_open() fails or is not called at
 * all, debugging output goes to stderr by default.
 */
int dbg_open(const char *name);

/* Same format rules as printf() */
int dbg_printf(const char *fmt, ...);

/* Closes the debugging log, if it is not stderr */
int dbg_close(void);

# ifdef DEBUG
#  include <assert.h>
#  define ASSERT(x) assert(x)
# else
#  define ASSERT(x)
# endif

#endif /* !COMMON_H */

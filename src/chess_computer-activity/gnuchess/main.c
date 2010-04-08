/* GNU Chess 5.0 - main.c - entry point
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

#include <config.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>                                                              

#include "common.h"
#include "book.h"

short distance[64][64];
short taxicab[64][64];
unsigned char lzArray[65536];
BitBoard DistMap[64][8];
BitBoard BitPosArray[64];
BitBoard NotBitPosArray[64];
BitBoard MoveArray[8][64];
BitBoard Ray[64][8];
BitBoard FromToRay[64][64];
BitBoard RankBit[8];
BitBoard FileBit[8];
BitBoard Ataks[2][7];
BitBoard PassedPawnMask[2][64];
BitBoard IsolaniMask[8];
BitBoard SquarePawnMask[2][64];
BitBoard Rook00Atak[64][256];
BitBoard Rook90Atak[64][256];
BitBoard Bishop45Atak[64][256];
BitBoard Bishop315Atak[64][256];
BitBoard pinned;
BitBoard rings[4];
BitBoard boxes[2];
BitBoard stonewall[2];
BitBoard pieces[2];
BitBoard mask_kr_trapped_w[3];
BitBoard mask_kr_trapped_b[3];
BitBoard mask_qr_trapped_w[3];
BitBoard mask_qr_trapped_b[3];
BitBoard boardhalf[2];
BitBoard boardside[2];
short directions[64][64];
unsigned char BitCount[65536];
leaf Tree[MAXTREEDEPTH];
leaf *TreePtr[MAXPLYDEPTH];
int RootPV;
GameRec Game[MAXGAMEDEPTH];
int GameCnt;
int RealGameCnt;
short RealSide;
int computer;
unsigned int flags;
unsigned int preanalyze_flags;
int cboard[64];
int Mvboard[64];
Board board;
HashType hashcode[2][7][64];
HashType ephash[64];
HashType WKCastlehash;
HashType WQCastlehash;
HashType BKCastlehash;
HashType BQCastlehash;
HashType Sidehash;
HashType HashKey;
HashType PawnHashKey;
HashSlot *HashTab[2];
PawnSlot *PawnTab[2];
int Idepth;
int SxDec;
int Game50;
int lazyscore[2];
int maxposnscore[2];
int rootscore;
int lastrootscore;
unsigned long GenCnt;
unsigned long NodeCnt;
unsigned long QuiesCnt;
unsigned long EvalCnt;
unsigned long EvalCall;
unsigned long ChkExtCnt;
unsigned long OneRepCnt;
unsigned long RcpExtCnt;
unsigned long PawnExtCnt;
unsigned long HorzExtCnt;
unsigned long ThrtExtCnt;
unsigned long KingExtCnt;
unsigned long NullCutCnt;
unsigned long FutlCutCnt;
unsigned long RazrCutCnt;
unsigned long TotalGetHashCnt;
unsigned long GoodGetHashCnt;
unsigned long TotalPutHashCnt;
unsigned long CollHashCnt;
unsigned long TotalPawnHashCnt;
unsigned long GoodPawnHashCnt;
unsigned long RepeatCnt;
unsigned HashSize;
unsigned long TTHashMask;
unsigned long PHashMask;
char SANmv[SANSZ];
unsigned long history[2][4096];
int killer1[MAXPLYDEPTH];
int killer2[MAXPLYDEPTH];
int ChkCnt[MAXPLYDEPTH];
int ThrtCnt[MAXPLYDEPTH];
char id[32];
char solution[64];
double ElapsedTime;
Timer StartTime;
float SearchTime;
int SearchDepth;
int MoveLimit[2];
float TimeLimit[2];
int TCMove;
int TCinc;
float TCTime;
int castled[2];
int hunged[2];
int phase;
int Hashmv[MAXPLYDEPTH];
short RootPieces;
short RootPawns;
short RootMaterial;
short RootAlpha;
short RootBeta;
short pickphase[MAXPLYDEPTH];
short InChk[MAXPLYDEPTH];
short KingThrt[2][MAXPLYDEPTH];
short threatmv;
uint8_t threatply;
short KingSafety[2];
short pscore[64];
short bookmode;
short bookfirstlast;

char *progname;
FILE *ofp;
int myrating, opprating, suddendeath, TCionc;
char name[50];
int computerplays;		/* Side computer is playing */
int wasbookmove;		/* True if last move was book move */
int nmovesfrombook;		/* Number of moves since last book move */
int newpos, existpos;		/* For book statistics */
float maxtime;		/* Max time for the next searched move */
int n;		/* Last mobility returned by CTL */
int ExchCnt[2];	/* How many exchanges? */
int bookloaded = 0;  	/* Is the book loaded already into memory? */

int slider[8] = { 0, 0, 0, 1, 1, 1, 0, 0 };
int Value[7] = { 0, ValueP, ValueN, ValueB, ValueR, ValueQ, ValueK};
int range[8] = { 0, 0, 0, 1, 1, 1, 0, 0 };
int ptype[2] = { pawn, bpawn };
char algbr[64][3] =
{ "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8" 
};
char algbrfile[9] = "abcdefgh";
char algbrrank[9] = "12345678";
  
char notation[8] = { " PNBRQK" };
char lnotation[8] = { " pnbrqk" };

short Shift00[64] =
{ 56, 56, 56, 56, 56, 56, 56, 56,
  48, 48, 48, 48, 48, 48, 48, 48,
  40, 40, 40, 40, 40, 40, 40, 40,
  32, 32, 32, 32, 32, 32, 32, 32,
  24, 24, 24, 24, 24, 24, 24, 24,
  16, 16, 16, 16, 16, 16, 16, 16,
   8,  8,  8,  8,  8,  8,  8,  8,
   0,  0,  0,  0,  0,  0,  0,  0
};

int r90[64] =
{ A8, A7, A6, A5, A4, A3, A2, A1,
  B8, B7, B6, B5, B4, B3, B2, B1,
  C8, C7, C6, C5, C4, C3, C2, C1,
  D8, D7, D6, D5, D4, D3, D2, D1,
  E8, E7, E6, E5, E4, E3, E2, E1,
  F8, F7, F6, F5, F4, F3, F2, F1,
  G8, G7, G6, G5, G4, G3, G2, G1,
  H8, H7, H6, H5, H4, H3, H2, H1 };

short Shift90[64] =
{ 0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56
};

int r45[64] =
{ E4, F3, H2, C2, G1, D1, B1, A1,
  E5, F4, G3, A3, D2, H1, E1, C1,
  D6, F5, G4, H3, B3, E2, A2, F1, 
  B7, E6, G5, H4, A4, C3, F2, B2,
  G7, C7, F6, H5, A5, B4, D3, G2, 
  C8, H7, D7, G6, A6, B5, C4, E3, 
  F8, D8, A8, E7, H6, B6, C5, D4, 
  H8, G8, E8, B8, F7, A7, C6, D5 };

short Shift45[64] =
{ 28, 36, 43, 49, 54, 58, 61, 63,
  21, 28, 36, 43, 49, 54, 58, 61,
  15, 21, 28, 36, 43, 49, 54, 58,
  10, 15, 21, 28, 36, 43, 49, 54,
   6, 10, 15, 21, 28, 36, 43, 49,
   3,  6, 10, 15, 21, 28, 36, 43,
   1,  3,  6, 10, 15, 21, 28, 36,
   0,  1,  3,  6, 10, 15, 21, 28 };

int Mask45[64] =
{ 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01,
  0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 
  0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 
  0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 
  0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 
  0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 
  0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 
  0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

int r315[64] =
{ A1, C1, F1, B2, G2, E3, D4, D5,
  B1, E1, A2, F2, D3, C4, C5, C6,
  D1, H1, E2, C3, B4, B5, B6, A7,
  G1, D2, B3, A4, A5, A6, H6, F7,
  C2, A3, H3, H4, H5, G6, E7, B8,
  H2, G3, G4, G5, F6, D7, A8, E8,
  F3, F4, F5, E6, C7, H7, D8, G8,
  E4, E5, D6, B7, G7, C8, F8, H8 };

short Shift315[64] =
{ 63, 61, 58, 54, 49, 43, 36, 28,
  61, 58, 54, 49, 43, 36, 28, 21,
  58, 54, 49, 43, 36, 28, 21, 15,
  54, 49, 43, 36, 28, 21, 15, 10,
  49, 43, 36, 28, 21, 15, 10,  6,
  43, 36, 28, 21, 15, 10,  6,  3,
  36, 28, 21, 15, 10,  6,  3,  1,
  28, 21, 15, 10,  6,  3,  1,  0 };

int Mask315[64] =
{ 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF,
  0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F,
  0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F,
  0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F,
  0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F,
  0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07,
  0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03,
  0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

int rank6[2] = { 5, 2 };
int rank7[2] = { 6, 1 };
int rank8[2] = { 7, 0 };

int main (int argc, char *argv[])
{
  int i;

  /*
   * Parse command line arguments conforming with getopt_long syntax
   * Note: we have to support "xboard" and "post" as bare strings
   * for backward compatibility.
   */
 
  int c;
  int opt_help = 0, opt_version = 0, opt_post = 0, opt_xboard = 0, opt_hash = 0, opt_easy = 0, opt_manual = 0;
  char *endptr;

  progname = argv[0]; /* Save in global for cmd_usage */

  while (1)
  {
    static struct option long_options[] =
    {
        {"hashsize", 1, 0, 's'},
        {"version", 0, 0, 'v'},
        {"help", 0, 0, 'h'},
        {"xboard", 0, 0, 'x'},
        {"post", 0, 0, 'p'},
        {"easy", 0, 0, 'e'},
        {"manual", 0, 0, 'm'},
        {0, 0, 0, 0}
    };
 
    /* getopt_long stores the option index here. */ 

    int option_index = 0;
 
    c = getopt_long (argc, argv, "ehmpvxs:",
             long_options, &option_index);
 
    /* Detect the end of the options. */
   if (c == -1)
     break;

   /* 
    * Options with a straight flag, could use getoopt_long
    * flag setting but this is more "obvious" and easier to
    * modify.
    */
   switch (c)
     {
     case 'v':
       opt_version = 1;
       break;
     case 'h':
       opt_help = 1;
       break;
     case 'x':
       opt_xboard = 1;
       break;
     case 'p':
       opt_post = 1;
       break;
     case 'e':
       opt_easy = 1;
       break;
     case 'm':
       opt_manual = 1;
       break;
     case 's':    
       if  ( optarg == NULL ){ /* we have error such as two -s */
         opt_help = 1;
         break;
       }
       errno = 0; /* zero error indicator */
       opt_hash = strtol (optarg, &endptr, 10);
       if ( errno != 0 || *endptr != '\0' ){
         printf("Hashsize out of Range or Invalid\n");
         return(1);
       }
       break;
     case '?': /* On error give help - getopt does a basic message. */
       opt_help = 1;
       break;
     default:
       puts ("Option Processing Failed\n");
       abort();
     }
  } /* end of getopt_long style parsing */

  /* Initialize random number generator */
  srand((unsigned int) time(NULL));
  
  /* initialize control flags */
  flags = ULL(0);

  /* output for thinking */
  ofp = stdout;

  /* Handle old style command line options */
  if (argc > 1) {
    for (i = 0; i < argc; i++) {
      if (strcmp(argv[i],"xboard") == 0) {
	SET (flags, XBOARD);
      } else if (strcmp(argv[i],"post") == 0) {
	SET (flags, POST);
      }
    }
  }
  if (opt_xboard == 1)
	SET (flags, XBOARD);
  if (opt_post == 1)
	SET (flags, POST);	
  if (opt_manual ==1)
	SET (flags, MANUAL);
  cmd_version();
  
  /* If the version option was specified we can exit here */
  if (opt_version == 1)
	return(0);
  
  /* If a usage statement is required output it here */
  if (opt_help == 1){
    cmd_usage();
    return (1); /* Maybe an error if due to bad arguments. */
  }

  dbg_open(NULL);

  HashSize = 0 ; /* Set HashSize zero */
  if ( opt_hash != 0)
    CalcHashSize(opt_hash);

  Initialize ();

  if ( opt_easy == 0)
   SET (flags, HARD);

  if (argc > 1) {
    for (i = 0; i < argc; i++) {
      if (strcmp(argv[i],"xboard") == 0) {
	SET (flags, XBOARD);
      } else if (strcmp(argv[i],"post") == 0) {
	SET (flags, POST);
      } 
    }
  }

  bookmode = BOOKPREFER;
  bookfirstlast = 3;

  while (!(flags & QUIT)) {
    dbg_printf("Waiting for input...\n");
    wait_for_input();
    dbg_printf("Parsing input...\n");
    parse_input();
    dbg_printf("input_status = %d\n", input_status);
    if ((flags & THINK) && !(flags & MANUAL) && !(flags & ENDED)) {
      if (!(flags & XBOARD)) printf("Thinking...\n");
      Iterate ();
      CLEAR (flags, THINK);
    }
    RealGameCnt = GameCnt;
    RealSide = board.side;
    dbg_printf("Waking up input...\n");
    dbg_printf("input_status = %d\n", input_status);
    input_wakeup();
    dbg_printf("input_status = %d\n", input_status);
    /* Ponder only after first move */
    /* Ponder or (if pondering disabled) just wait for input */
    if ((flags & HARD) && !(flags & QUIT) ) {
      ponder();
    }
  }
  
  CleanupInput();

  /*  Some cleaning up  */
  free (HashTab[0]);
  free (HashTab[1]);

  dbg_close();
  return (0);
}

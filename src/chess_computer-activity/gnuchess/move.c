/* GNU Chess 5.0 - move.c - make and unmake moves code
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
/*
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

void MakeMove (int side, int *move)
/**************************************************************************
 *
 *  To make a move on the board and update the various game information.
 *
 **************************************************************************/
{
   BitBoard *a;
   int f, t, fpiece, tpiece; 
   int rookf, rookt, epsq, sq;
   int xside;
   GameRec *g;

   xside = 1^side;
   f = FROMSQ(*move);
   t = TOSQ(*move);
   fpiece = cboard[f];
   tpiece = cboard[t];
   a = &board.b[side][fpiece];
   CLEARBIT (*a, f);
   SETBIT (*a, t);
   CLEARBIT (board.blockerr90, r90[f]);
   SETBIT (board.blockerr90, r90[t]);
   CLEARBIT (board.blockerr45, r45[f]);
   SETBIT (board.blockerr45, r45[t]);
   CLEARBIT (board.blockerr315, r315[f]);
   SETBIT (board.blockerr315, r315[t]);
   cboard[f] = empty;
   cboard[t] = fpiece;
   GameCnt++;
   g = &Game[GameCnt];
   g->epsq = board.ep; 
   g->bflag = board.flag;
   g->Game50 = Game50;
   g->hashkey = HashKey;
   g->phashkey = PawnHashKey;
   g->mvboard = Mvboard[t];
   g->comments = NULL;
   Mvboard[t] = Mvboard[f]+1; 
   Mvboard[f] = 0;
   if (board.ep > -1)
      HashKey ^= ephash[board.ep];
   HashKey ^= hashcode[side][fpiece][f];
   HashKey ^= hashcode[side][fpiece][t];
   if (fpiece == king)
      board.king[side] = t;
   if (fpiece == pawn)
   {
      PawnHashKey ^= hashcode[side][pawn][f];
      PawnHashKey ^= hashcode[side][pawn][t];
   }

   if (tpiece != 0)		/* Capture */
   {
      ExchCnt[side]++;
      CLEARBIT (board.b[xside][tpiece], t);
      *move |= (tpiece << 15);
      HashKey ^= hashcode[xside][tpiece][t];
      if (tpiece == pawn)
         PawnHashKey ^= hashcode[xside][pawn][t];
      board.material[xside] -= Value[tpiece];
      if (tpiece != pawn)
         board.pmaterial[xside] -= Value[tpiece];
   }

   if (*move & PROMOTION) 	/* Promotion */ 
   {
      SETBIT (board.b[side][PROMOTEPIECE (*move)], t);
      CLEARBIT (*a, t);
      cboard[t] = PROMOTEPIECE (*move);
      HashKey ^= hashcode[side][pawn][t];
      HashKey ^= hashcode[side][cboard[t]][t];
      PawnHashKey ^= hashcode[side][pawn][t];
      board.material[side] += (Value[cboard[t]] - ValueP);
      board.pmaterial[side] += Value[cboard[t]];
   }

   if (*move & ENPASSANT)	/* En passant */
   {
      ExchCnt[side]++;
      epsq = board.ep + (side == white ? - 8 : 8);
      CLEARBIT (board.b[xside][pawn], epsq);
      CLEARBIT (board.blockerr90, r90[epsq]);
      CLEARBIT (board.blockerr45, r45[epsq]);
      CLEARBIT (board.blockerr315, r315[epsq]);
      cboard[epsq] = empty;
      HashKey ^= hashcode[xside][pawn][epsq];
      PawnHashKey ^= hashcode[xside][pawn][epsq];
      board.material[xside] -= ValueP;
   }
   if (*move & (CAPTURE | CASTLING) || fpiece == pawn)
      Game50 = GameCnt;

   if (*move & CASTLING) 	/* Castling */
   {
      if (t & 0x04)		/* King side */
      {
         rookf = t + 1;
         rookt = t - 1;
      }
      else			/* Queen side */
      {
         rookf = t - 2;
         rookt = t + 1;
      }
      a = &board.b[side][rook];
      CLEARBIT (*a, rookf);
      SETBIT (*a, rookt);
      CLEARBIT (board.blockerr90, r90[rookf]);
      SETBIT (board.blockerr90, r90[rookt]);
      CLEARBIT (board.blockerr45, r45[rookf]);
      SETBIT (board.blockerr45, r45[rookt]);
      CLEARBIT (board.blockerr315, r315[rookf]);
      SETBIT (board.blockerr315, r315[rookt]);
      cboard[rookf] = empty;
      cboard[rookt] = rook;
      Mvboard[rookf] = 0;
      Mvboard[rookt] = 1;
      HashKey ^= hashcode[side][rook][rookf];
      HashKey ^= hashcode[side][rook][rookt];
      board.castled[side] = true;
   }

   /* If king or rook move, clear castle flag. */
   if (side == white)
   {
      if (fpiece == king && board.flag & WCASTLE)
      {
	 if (board.flag & WKINGCASTLE)
            HashKey ^= WKCastlehash;
	 if (board.flag & WQUEENCASTLE)
            HashKey ^= WQCastlehash;
         board.flag &= ~WCASTLE;
      }
      else if (fpiece == rook)
      {
         if (f == H1)
	 {
	    if (board.flag & WKINGCASTLE)
               HashKey ^= WKCastlehash;
	    board.flag &= ~WKINGCASTLE;
	 }
         else if (f == A1) 
	 {
	    if (board.flag & WQUEENCASTLE)
               HashKey ^= WQCastlehash;
	    board.flag &= ~WQUEENCASTLE;
	 }
      }
      if (tpiece == rook)
      {
         if (t == H8) 
	 {
	    if (board.flag & BKINGCASTLE)
	       HashKey ^= BKCastlehash;
	    board.flag &= ~BKINGCASTLE;
         }
         else if (t == A8) 
	 {
	    if (board.flag & BQUEENCASTLE)
	       HashKey ^= BQCastlehash;
	    board.flag &= ~BQUEENCASTLE;
	 }
      }
   }
   else
   {
      if (fpiece == king && board.flag & BCASTLE)
      {
	 if (board.flag & BKINGCASTLE)
	    HashKey ^= BKCastlehash;
	 if (board.flag & BQUEENCASTLE)
	    HashKey ^= BQCastlehash;
         board.flag &= ~BCASTLE;
      }
      else if (fpiece == rook)
      {
         if (f == H8) 
	 {
	    if (board.flag & BKINGCASTLE)
	       HashKey ^= BKCastlehash;
	    board.flag &= ~BKINGCASTLE;
         }
         else if (f == A8) 
	 {
	    if (board.flag & BQUEENCASTLE)
	       HashKey ^= BQCastlehash;
	    board.flag &= ~BQUEENCASTLE;
	 }
      }
      if (tpiece == rook)
      {
         if (t == H1) 
	 {
	    if (board.flag & WKINGCASTLE)
               HashKey ^= WKCastlehash;
	    board.flag &= ~WKINGCASTLE;
	 }
         else if (t == A1) 
	 {
	    if (board.flag & WQUEENCASTLE)
               HashKey ^= WQCastlehash;
	    board.flag &= ~WQUEENCASTLE;
	 }
      } 
   }


   /* If pawn move 2 squares, set ep passant square. */
   if (fpiece == pawn && abs(f-t) == 16)
   {
      sq = (f + t) / 2;
      board.ep = sq;
      HashKey ^= ephash[sq];
   }
   else
      board.ep = -1;

   board.side = xside;
   HashKey ^= Sidehash;
   UpdateFriends ();

   /* Update game record */
   g->move = *move;
   return;
}



void UnmakeMove (int side, int *move)
/****************************************************************************
 *
 *  To unmake a move on the board and update the various game information.
 *  Note that if side is black, then black is about to move, but we will be
 *  undoing a move by white, not black.
 *
 ****************************************************************************/
{
   BitBoard *a;
   int f, t, fpiece, cpiece;   
   int rookf, rookt, epsq;
   int xside;
   GameRec *g;

   side = 1^side;
   xside = 1^side;
   f = FROMSQ(*move);
   t = TOSQ(*move);
   fpiece = cboard[t];
   cpiece = CAPTUREPIECE (*move);
   a = &board.b[side][fpiece];
   CLEARBIT (*a, t);
   SETBIT (*a, f);   
   CLEARBIT (board.blockerr90, r90[t]); 
   SETBIT (board.blockerr90, r90[f]); 
   CLEARBIT (board.blockerr45, r45[t]); 
   SETBIT (board.blockerr45, r45[f]); 
   CLEARBIT (board.blockerr315, r315[t]); 
   SETBIT (board.blockerr315, r315[f]); 
   cboard[f] = cboard[t];
   cboard[t] = empty;
   g = &Game[GameCnt];
   Mvboard[f] = Mvboard[t]-1;
   Mvboard[t] = g->mvboard;
   if (fpiece == king)
      board.king[side] = f;
   
   /* if capture, put back the captured piece */
   if (*move & CAPTURE)
   {
      ExchCnt[side]--;
      SETBIT (board.b[xside][cpiece], t);  
      SETBIT (board.blockerr90, r90[t]);  
      SETBIT (board.blockerr45, r45[t]);  
      SETBIT (board.blockerr315, r315[t]);  
      cboard[t] = cpiece; 
      board.material[xside] += Value[cpiece];  
      if (cpiece != pawn)
         board.pmaterial[xside] += Value[cpiece];  
   }

   /* Undo promotion */
   if (*move & PROMOTION)
   {
      CLEARBIT (*a, f);
      SETBIT (board.b[side][pawn], f);  
      cboard[f] = pawn;
      board.material[side] += (ValueP - Value[PROMOTEPIECE (*move)]); 
      board.pmaterial[side] -= Value[PROMOTEPIECE (*move)];
   }

   /* Undo enpassant */
   if (*move & ENPASSANT)
   {
      ExchCnt[side]--;
      epsq = (side == white ? g->epsq - 8 : g->epsq + 8);
      SETBIT (board.b[xside][pawn], epsq); 
      SETBIT (board.blockerr90, r90[epsq]); 
      SETBIT (board.blockerr45, r45[epsq]); 
      SETBIT (board.blockerr315, r315[epsq]); 
      cboard[epsq] = pawn;
      board.material[xside] += ValueP;  
   }   

   /* if castling, undo rook move */
   if (*move & CASTLING)
   {
      if (t & 0x04)		/* King side */
      {
         rookf = t + 1;
         rookt = t - 1;
      }
      else			/* Queen side */
      {
         rookf = t - 2;
         rookt = t + 1;
      }
      a = &board.b[side][rook];
      CLEARBIT (*a, rookt);
      SETBIT (*a, rookf); 
      CLEARBIT (board.blockerr90, r90[rookt]);
      SETBIT (board.blockerr90, r90[rookf]);
      CLEARBIT (board.blockerr45, r45[rookt]);
      SETBIT (board.blockerr45, r45[rookf]);
      CLEARBIT (board.blockerr315, r315[rookt]);
      SETBIT (board.blockerr315, r315[rookf]);
      cboard[rookf] = rook;
      cboard[rookt] = empty;
      Mvboard[rookf] = 0;
      Mvboard[rookt] = 0;
      board.castled[side] = false;
   }

   UpdateFriends ();
   board.side = side;
   board.ep = g->epsq;
   board.flag = g->bflag;  
   HashKey = g->hashkey;
   PawnHashKey = g->phashkey;
   Game50 = g->Game50;
   GameCnt--;
   return;
}


void SANMove (int move, int ply)
/****************************************************************************
 *
 *  Convert the move to a SAN format.  GenMoves (ply) needs to be called
 *  by the calling routine for this to work.
 *
 ****************************************************************************/
{
   int side;
   int piece, ambiguous;
   int f, t;
   BitBoard b;
   leaf *node1;
   char *s;

   side = board.side;
   s = SANmv;
   f = FROMSQ(move);
   t = TOSQ(move);
   /* Check some special moves like castling */
   if (move & CASTLING)
   {
      if (t == 6 || t == 62)
         strcpy (s, "O-O");
      else
	 strcpy (s, "O-O-O");
      return;
   }
 
/****************************************************************************
 *
 *  Here split the code into 2 parts for clarity sake.  First part deals 
 *  with pawn moves only, 2nd part only piece moves.  However before doing 
 *  that, see if the move is ambiguous.
 *
 ****************************************************************************/

   /*  AMBIGUITY CHECK  */
   piece = cboard[f];
   side = board.side;
   b = board.b[side][piece];
   ambiguous = false;
   node1 = TreePtr[ply];
   if (nbits (b) > 1)
   {
      /* 
       *  Scan the movelist to see if another same-type piece is
       *  also moving to that particular to-square.
       */
      for (node1 = TreePtr[ply]; node1 < TreePtr[ply + 1]; node1++)
      {
         if (FROMSQ(node1->move) == f)
            continue;                   /* original piece, skip */
         if (TOSQ(node1->move) != t)
            continue;                   /* diff to-square, skip */
         if (cboard[FROMSQ(node1->move)] != piece)
            continue;                   /* diff piece   */
         ambiguous = true;
	 break;
      }
   }

   if (piece == pawn)
   {
      /* Capture or enpassant */
      if (cboard[t] != 0 || board.ep == t)
      {
         *s++ = algbrfile [ROW (f)];
	 *s++ = 'x';
      }
      strcpy (s, algbr[t]);
      s += 2;

      /* Promotion */
      if (move & PROMOTION)
      {
         *s++ = '=';
         *s++ = notation[PROMOTEPIECE (move)];
      }
   }
   else	/* its not pawn */
   {
      *s++ = notation[piece];
      if (ambiguous)
      {
         if (ROW (f) == ROW (FROMSQ(node1->move)))
            *s++ = algbrrank[RANK (f)];
	 else
	    *s++ = algbrfile[ROW (f)];
      }
      if (cboard[t] != 0)		/* capture */
         *s++ = 'x'; 
      strcpy (s, algbr[t]);
      s += 2;
   }

   /* See if it is a checking or mating move */
   MakeMove (side, &move);
   if (SqAtakd (board.king[1^side], side))
   {
      TreePtr[ply+2] = TreePtr[ply+1];
      GenCheckEscapes (ply+1);
      if (TreePtr[ply+1] == TreePtr[ply+2])
         *s++ = '#';
      else
         *s++ = '+';
      GenCnt -= TreePtr[ply+2] - TreePtr[ply+1];
   }
   UnmakeMove (1^side, &move); 

   *s = '\0';
   return;
}


#define ASCIITOFILE(a) ((a) - 'a')
#define ASCIITORANK(a) ((a) - '1')
#define ASCIITOSQ(a,b) (ASCIITOFILE(a)) + (ASCIITORANK(b)) * 8
#define ATOH(a) ((a) >= 'a' && (a) <= 'h')
#define ITO8(a) ((a) >= '1' && (a) <= '8')

static inline int piece_id(const char c)
{
/* Given c, what is the piece id.  This only takes one char, which
 * isn't enough to handle two-character names (common in Russian text
 * and old English notation that used Kt), but we're not supposed to
 * see such text here anyway.  This will
 * accept "P" for pawn, and many lowercase chars (but not "b" for Bishop). */
   switch (c)
   {
      case 'n':
      case 'N':
         return knight;
      case 'B':
         return bishop;
      case 'r':
      case 'R':
         return rook;
      case 'q':
      case 'Q':
         return queen;
      case 'k':
      case 'K':
         return king;
      case 'p':
      case 'P':
         return pawn;
   }
   return empty;
}



leaf * ValidateMove (char *s)
/*************************************************************************
 *
 *  This routine takes a string and check to see if it is a legal move.
 *  Note.  At the moment, we accept 2 types of moves notation.
 *  1.  e2e4 format.   2. SAN format. (e4)
 *
 **************************************************************************/
{
   short f, t, side, rank, file, fileto;
   short piece, piece2, kount;
   char promote;
   char mvstr[MAXSTR], *p;
   BitBoard b, b2;
   leaf *n1, *n2;

   TreePtr[2] = TreePtr[1];
   GenMoves (1);   
   FilterIllegalMoves (1);	
   side = board.side;

   /************************************************************************
    * The thing to do now is to clean up the move string.  This
    * gets rid of things like 'x', '+', '=' and so forth.
    ************************************************************************/
   p = mvstr;
   do
   {
      if (*s != 'x' && *s != '+' && *s != '=' && !isspace(*s))
         *p++ = *s; 
   } while (*s++ != '\0' );

   /* Flush castles that check */
   if (mvstr[strlen(mvstr)-1] == '+' || mvstr[strlen(mvstr)-1] == '#' ||
       mvstr[strlen(mvstr)-1] == '=') mvstr[strlen(mvstr)-1] = '\000'; 

   /* Check for castling */
   if (strcmp (mvstr, "O-O") == 0 || strcmp (mvstr, "o-o") == 0 || 
       strcmp (mvstr, "0-0") == 0)
   {
      if (side == white)
      {
         f = 4; t = 6;
      }
      else 
      {
	 f = 60; t = 62;
      }
      return (IsInMoveList (1, f, t, ' '));
   }

   if (strcmp (mvstr, "O-O-O") == 0 || strcmp (mvstr, "o-o-o") == 0 ||
       strcmp (mvstr, "0-0-0") == 0)
   {
      if (side == white)
      {
         f = 4; t = 2;
      }
      else
      {
         f = 60; t = 58;
      }
      return (IsInMoveList (1, f, t, ' '));
   }

   /*  Test to see if it is e2e4 type notation */
   if (ATOH (mvstr[0]) && ITO8 (mvstr[1]) && ATOH (mvstr[2]) &&
	ITO8 (mvstr[3]))
   {
      f = ASCIITOSQ (mvstr[0], mvstr[1]);
      t = ASCIITOSQ (mvstr[2], mvstr[3]);
      piece = (strlen (mvstr) == 5 ? mvstr[4] : ' ');
      return (IsInMoveList (1, f, t, piece));
   }


   /***********************************************************************
    *  Its a SAN notation move.  More headache!  
    *  We generate all the legal moves and start comparing them with
    *  the input move.
    ***********************************************************************/
   if (ATOH (mvstr[0]))	/* pawn move */
   {
      if (ITO8 (mvstr[1]))					/* e4 type */
      {
	 t = ASCIITOSQ (mvstr[0], mvstr[1]);
         f = t + (side == white ? -8 : 8);
         /* Add Sanity Check */
         if ( f > 0 && f < 64 ) {
	   if (BitPosArray[f] & board.b[side][pawn])
           {
              if (mvstr[2] != '\0') 
                 return (IsInMoveList (1, f, t, mvstr[2]));
              else
                 return (IsInMoveList (1, f, t, ' '));
           }
           f = t + (side == white ? -16 : 16);
           if ( f > 0 && f < 64 ) {
	     if (BitPosArray[f] & board.b[side][pawn])
                return (IsInMoveList (1, f, t, ' '));
	   } /* End bound check +/- 16 */
	 } /* End bound check +/- 8 */
        }
      else if (ATOH (mvstr[1]) && ITO8 (mvstr[2]))		/* ed4 type */
      {
	 t = ASCIITOSQ (mvstr[1], mvstr[2]);
	 rank = ASCIITORANK (mvstr[2]) + (side == white ? -1 : 1);
         f = rank * 8 + ASCIITOFILE (mvstr[0]);
         piece = (strlen (mvstr) == 4 ? mvstr[3] : ' ');
         return (IsInMoveList (1, f, t, piece));
      }
      else if (ATOH (mvstr[1]))					/* ed type */
      {
         file = ASCIITOFILE (mvstr[0]);
         fileto = ASCIITOFILE (mvstr[1]);
	 b = board.b[side][pawn] & FileBit[file];
	 if (side == white)
	    b = b >> (fileto < file ? 7 : 9);
         else
	    b = b << (fileto < file ? 9 : 7);
         if (board.ep > -1)
	    b = b & (board.friends[1^side] | BitPosArray[board.ep]);
         else
	    b = b & (board.friends[1^side]);
         switch (nbits (b))
	 {
	    case 0  : return ((leaf *) NULL);
	    case 1  : t = leadz (b);
	    	      f = t - (side == white ? 8 : -8) + (file - fileto);
         	      piece = (strlen (mvstr) == 3 ? mvstr[2] : ' ');
	    	      return (IsInMoveList (1, f, t, piece));
	    default : 
		      printf ("Ambiguous move: %s %s\n",s,mvstr);
		      ShowBoard();
/*
		      getchar();
*/
	    	      return ((leaf *) NULL);
	 }
      } 

   } 
   else	if ((piece = piece_id(mvstr[0])) != empty &&
            (piece_id(mvstr[1]) == empty))	/* Is a piece move */
   {
      /* Since piece_id accepts P as pawns, this will correctly
       * handle malformed commands like Pe4 */

      b = board.b[side][piece];
      t = -1;
      if (ITO8 (mvstr[1]))				/* N1d2 type move */
      {
         rank = ASCIITORANK (mvstr[1]);
	 b &= RankBit[rank];
	 t = ASCIITOSQ (mvstr[2], mvstr[3]);
      }
      else if (ATOH (mvstr[1]) && ATOH (mvstr[2]))	/* Nbd2 type move */
      {
         file = ASCIITOFILE (mvstr[1]);
	 b &= FileBit[file];
	 t = ASCIITOSQ (mvstr[2], mvstr[3]);
      }
      else if (ATOH (mvstr[1]) && ITO8 (mvstr[2]))	/* Nd2 type move */
      {
	 t = ASCIITOSQ (mvstr[1], mvstr[2]);
      }

      kount = 0;
      n1 = n2 = (leaf *) NULL;
      while (b)
      {
         f = leadz (b);
 	 CLEARBIT (b, f);
	 if ((n1 = IsInMoveList (1, f, t, ' ')) != (leaf *) NULL )
	 {
	    n2 = n1;
	    kount++;
	 }
      }
      if (kount > 1)
      {
	 printf ("Ambiguous move: %s %s\n",s,mvstr);
	 ShowBoard();
/*
	 getchar();
*/
   	 return ((leaf *) NULL);
      } 
      else if (kount == 0)
   	 return ((leaf *) NULL);
      else
         return (n2);
   }
   else	if (((piece = piece_id(mvstr[0])) != empty) &&
            ((piece2 = piece_id(mvstr[1])) != empty) &&
	    ( (mvstr[2] == '\0') ||
	      ((piece_id(mvstr[2]) != empty) && mvstr[3] == '\0')))
   { /* KxP format */
      promote = ' ';
      if (piece_id(mvstr[2] != empty)) {
          promote = mvstr[2];
      }
      kount = 0;
      n1 = n2 = (leaf *) NULL;
      b = board.b[side][piece];
      while (b)
      {
         f = leadz (b);
 	 CLEARBIT (b, f);
         b2 = board.b[1^side][piece2];
	 while (b2)
	 {
           t = leadz (b2);
 	   CLEARBIT (b2, t);
	   printf("Trying %s: ", AlgbrMove(MOVE(f,t)));
	   if ((n1 = IsInMoveList (1, f, t, promote)) != (leaf *) NULL)
	   {
	     n2 = n1;
	     kount++;
	     printf("Y  ");
	   }
	   else printf("N  ");
	 }
      }
      if (kount > 1)
      {
	 printf ("Ambiguous move: %s %s\n",s,mvstr);
	 ShowBoard();
/*
	 getchar();
*/
   	 return ((leaf *) NULL);
      } 
      else if (kount == 0)
   	 return ((leaf *) NULL);
      else
         return (n2);
      
   }

   /* Fall through.  Nothing worked, return that no move was performed. */
   return ((leaf *) NULL);
}


leaf * IsInMoveList (int ply, int f, int t, char piece)
/**************************************************************************
 *
 *  Checks to see if from and to square can be found in the movelist
 *  and is legal.
 *
 **************************************************************************/
{
   leaf *node;

   for (node = TreePtr[ply]; node < TreePtr[ply + 1]; node++)
   {
      if ((int) (node->move & 0x0FFF) == MOVE(f,t)  && 
	toupper(piece) == notation[PROMOTEPIECE (node->move)])
         return (node);
   }
   return ((leaf *) NULL);
}


int IsLegalMove (int move)
/*****************************************************************************
 *
 *  Check that a move is legal on the current board.  
 *  Perform some preliminary sanity checks.
 *  1.  If from square is emtpy, illegal.
 *  2.  Piece not of right color.
 *  3.  To square is friendly, illegal.
 *  4.  Promotion move or enpassant, so piece must be pawn.
 *  5.  Castling move, piece must be king.
 *  Note that IsLegalMove() no longer care about if a move will place the
 *  king in check.  This will be caught by the Search().
 *
 *****************************************************************************/
{
   int side;
   int f, t, piece;
   BitBoard blocker, enemy;

   f = FROMSQ(move); 
   t = TOSQ(move);

   /*  Empty from square  */
   if (cboard[f] == empty)
      return (false);

   /*  Piece is not right color  */
   side = board.side;
   if (!(BitPosArray[f] & board.friends[side]))
      return (false);

   /*  TO square is a friendly piece, so illegal move  */
   if (BitPosArray[t] & board.friends[side])
      return (false);

   piece = cboard[f];
   /*  If promotion move, piece must be pawn */
   if ((move & (PROMOTION | ENPASSANT)) && piece != pawn)
      return (false);

   /*  If enpassant, then the enpassant square must be correct */
   if ((move & ENPASSANT) && t != board.ep)
      return (false);

   /*  If castling, then make sure its the king */
   if ((move & CASTLING) && piece != king)
      return (false); 

   blocker = board.blocker;
   /*  Pawn moves need to be handle specially  */
   if (piece == pawn)
   {
      if ((move & ENPASSANT) && board.ep > -1)
         enemy = board.friends[1^side] | BitPosArray[board.ep];
      else
         enemy = board.friends[1^side];
      if (side == white)
      {
         if (!(MoveArray[pawn][f] & BitPosArray[t] & enemy) &&
             !(t - f == 8 && cboard[t] == empty) &&
             !(t - f == 16 && RANK(f) == 1 && !(FromToRay[f][t] & blocker)))
	    return (false);
      }
      else if (side == black)
      {
         if (!(MoveArray[bpawn][f] & BitPosArray[t] & enemy) &&
             !(t - f == -8 && cboard[t] == empty) &&
             !(t - f == -16 && RANK(f) == 6 && !(FromToRay[f][t] & blocker)))
	    return (false);
      }
   }
   /*  King moves are also special, especially castling  */
   else if (piece == king)
   {
      if (side == white)
      {
         if (!(MoveArray[piece][f] & BitPosArray[t]) &&
	     !(f == E1 && t == G1 && board.flag & WKINGCASTLE &&
	       !(FromToRay[E1][G1] & blocker) && !SqAtakd(E1,black) &&
		!SqAtakd(F1,black)) &&
	     !(f == E1 && t == C1 && board.flag & WQUEENCASTLE &&
	       !(FromToRay[E1][B1] & blocker) && !SqAtakd(E1,black) &&
		!SqAtakd(D1,black)))
            return (false);
      }
      if (side == black)
      {
         if (!(MoveArray[piece][f] & BitPosArray[t]) &&
	     !(f == E8 && t == G8 && board.flag & BKINGCASTLE &&
	       !(FromToRay[E8][G8] & blocker) && !SqAtakd(E8,white) &&
		!SqAtakd(F8,white)) &&
	     !(f == E8 && t == C8 && board.flag & BQUEENCASTLE &&
	       !(FromToRay[E8][B8] & blocker) && !SqAtakd(E8,white) &&
		!SqAtakd(D8,white)))
            return (false);
      }
   }
   else 
   {
      if (!(MoveArray[piece][f] & BitPosArray[t]))
         return (false);
   }

   /*  If there is a blocker on the path from f to t, illegal move  */
   if (slider[piece])
   {
      if (FromToRay[f][t] & NotBitPosArray[t] & blocker)
         return (false);
   }

   return (true);
}

char *AlgbrMove (int move)
/*****************************************************************************
 *
 *  Convert an int move format to algebraic format of g1f3.
 *
 *****************************************************************************/
{
   int f, t;
   static char s[6];

   f = FROMSQ(move);
   t = TOSQ(move);
   strcpy (s, algbr[f]);
   strcpy (s+2, algbr[t]);
   if (move & PROMOTION)
   {
      if (flags & XBOARD)
        s[4] = lnotation[PROMOTEPIECE (move)];
      else
        s[4] = notation[PROMOTEPIECE (move)];
      s[5] = '\0';
   }
   else
      s[4] = '\0';
   return (s);
}


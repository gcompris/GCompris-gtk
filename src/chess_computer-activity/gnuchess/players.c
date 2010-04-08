
/* GNU Chess 5.0 - player.c - database on players
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "book.h"

#define PLAYERFILE "players.dat"

int totalplayers = 0;

#define MAXPLAYERS 500

typedef struct {
  char player[MAXNAMESZ];
  int wins;
  int losses;
  int draws;
} playerentry;

playerentry playerdb[MAXPLAYERS];
static char lname[MAXNAMESZ];

static int rscorecompare(const void *aa, const void *bb)
{
    const playerentry *a = aa;
    const playerentry *b = bb;   
    float ascore, bscore;
    ascore = (a->wins+(a->draws/2))/(a->wins+a->draws+a->losses);
    bscore = (b->wins+(b->draws/2))/(b->wins+b->draws+b->losses);
    if (ascore > bscore) return(-1);
    else if (bscore > ascore) return(1);
    else return(0);
}

static int scorecompare(const void *aa, const void *bb)
{
    const playerentry *a = aa;
    const playerentry *b = bb;   
    int ascore, bscore;
    ascore = 100*(a->wins+(a->draws/2))/(a->wins+a->draws+a->losses);
    bscore = 100*(b->wins+(b->draws/2))/(b->wins+b->draws+b->losses);
    if (bscore > ascore) return(1);
    else if (bscore < ascore) return(-1);
    else return(0);
}

static int namecompare(const void *aa, const void *bb)
{
    const playerentry *a = aa;
    const playerentry *b = bb;   
    if (strcmp(a->player,b->player) > 0) return(1);
    else if (strcmp(a->player,b->player) < 0) return(-1);
    else return(0);
}

void DBSortPlayer (const char *style)
{
  if (strncmp(style,"score",5) == 0) {
    qsort(&playerdb,totalplayers,sizeof(playerentry),scorecompare);
  } else if (strncmp(style,"name",4) == 0) {
    qsort(&playerdb,totalplayers,sizeof(playerentry),namecompare);
  } else if (strncmp(style,"reverse",7) == 0) {
    qsort(&playerdb,totalplayers,sizeof(playerentry),rscorecompare);
  }
}

void DBListPlayer (const char *style)
{
  int i;
	
  DBReadPlayer ();
  DBSortPlayer (style);
  for (i = 0; i < totalplayers; i++) {
    printf("%s %2.0f%% %d %d %d\n",
	playerdb[i].player,
	100.0*(playerdb[i].wins+((float)playerdb[i].draws/2))/
	 (playerdb[i].wins+playerdb[i].draws+playerdb[i].losses),
	playerdb[i].wins,
	playerdb[i].losses,
	playerdb[i].draws);
    if ((i+1) % 10 == 0) {printf("[Type a character to continue.]\n"); getchar();}
  }
}

void DBWritePlayer (void)
{
   int i;
   float result1;
   int result2;
   FILE *wfp;
   DBSortPlayer ("reverse");
   if ((wfp = fopen(PLAYERFILE,"w")) != NULL) {
     for (i = 0; i < totalplayers; i++) {
        result1 =
  	  100.0*(playerdb[i].wins+((float)playerdb[i].draws/2))/
	   (playerdb[i].wins+playerdb[i].draws+playerdb[i].losses),
	result2 = (int) result1;
        fprintf(wfp,"%s %d %d %d\n",
	  playerdb[i].player,
          playerdb[i].wins,
          playerdb[i].losses,
	  playerdb[i].draws);
     }
   }
   fclose(wfp);
}

void DBReadPlayer (void)
{
   FILE *rfp;
   int n;
   totalplayers = 0;
   if ((rfp = fopen(PLAYERFILE,"r")) != NULL) {
    while (!feof(rfp)) {
     n = fscanf(rfp,"%49s %d %d %d\n",  /* 49 MAXNAMESZ-1 */
	playerdb[totalplayers].player,
        &playerdb[totalplayers].wins,
        &playerdb[totalplayers].losses,
        &playerdb[totalplayers].draws);
     if (n == 4) totalplayers++;
    }
    fclose(rfp);
   }
}

int DBSearchPlayer (const char *player)
{
  int index = -1;
  int i;

  for (i = 0; i < totalplayers; i++)
    if (strncmp(playerdb[i].player,player,strlen(playerdb[i].player)) == 0)
    {
      index = i;
      break;
    }
  return (index);
}

void DBUpdatePlayer (const char *player, const char *resultstr)
{
  const char *p;
  char *x;
  int index;
  int result = R_NORESULT;

  p = player;
  x = lname;
  strcpy(lname,player);
  do {
    if (*p != ' ') 
      *x++ = *p++;
    else
	p++;
  } while (*p != '\0');
  *x = '\000';
  memset(playerdb,0,sizeof(playerdb));
  DBReadPlayer ();
  index = DBSearchPlayer (lname);
  if (index == -1) {
    strcpy(playerdb[totalplayers].player,lname);
    playerdb[totalplayers].wins = 0;
    playerdb[totalplayers].losses = 0;
    playerdb[totalplayers].draws = 0;
    index = totalplayers;
    totalplayers++;
  }
  if (strncmp(resultstr,"1-0",3) == 0)
     result = R_WHITE_WINS;
  else if (strncmp(resultstr,"0-1",3) == 0)
     result = R_BLACK_WINS;
  else if (strncmp(resultstr,"1/2-1/2",7) == 0)
     result = R_DRAW;

  if ((computerplays == white && result == R_WHITE_WINS)||
      (computerplays == black && result == R_BLACK_WINS))
    playerdb[index].wins++;
  else if ((computerplays == white && result == R_BLACK_WINS)||
      (computerplays == black && result == R_WHITE_WINS))
    playerdb[index].losses++;
  else
    /* Shouln't one check for draw here? Broken PGN files surely exist */
    playerdb[index].draws++;
  DBWritePlayer ();
}

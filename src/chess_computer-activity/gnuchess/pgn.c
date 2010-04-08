/* GNU Chess 5.0 - pgn.c - pgn game format code
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
#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#include "common.h"
#include "version.h"
#include "book.h"
#include "lexpgn.h"

#define NULL2EMPTY(x) ( (x) ? (x) : "")

extern FILE *yyin;

int yylex(void);

void PGNSaveToFile (const char *file, const char *resultstr)
/****************************************************************************
 *
 *  To save a game into PGN format to a file.  If the file does not exist,
 *  it will create it.  If the file exists, the game is appended to the file.
 *
 ****************************************************************************/
{
   FILE *fp;
   char s[100];
   int len;
   char *p;
   int i;
   time_t secs;
   struct tm *timestruct;

   fp = fopen (file, "a");		/*  Can we append to it?  */
   if (fp == NULL)
   {
      printf ("Cannot write to file %s\n", file);
      return;
   }

   /* Write the seven tags */
   fprintf (fp, "[Event \"%s\"]\n", NULL2EMPTY(pgn_event));
   fprintf (fp, "[Site \"%s\"]\n", NULL2EMPTY(pgn_site));
   secs=time(0);
   if (pgn_date)
     fprintf(fp,"[Date \"%s\"]\n", pgn_date);
   else {
     timestruct=localtime((time_t*) &secs);
     fprintf(fp,"[Date \"%4d.%02d.%02d\"]\n",timestruct->tm_year+1900,
             timestruct->tm_mon+1,timestruct->tm_mday);
   }
   fprintf (fp, "[Round \"%s\"]\n", NULL2EMPTY(pgn_round));

   if (pgn_white)
     fprintf (fp, "[White \"%s\"]\n", pgn_white);
   else if (computer == white)
     fprintf (fp, "[White \"%s %s\"]\n",PROGRAM,VERSION);
   else
     fprintf (fp, "[White \"%s\"]\n",name);

   if (pgn_black)
     fprintf (fp, "[Black \"%s\"]\n", pgn_black);
   else if (computer == black)
     fprintf (fp, "[Black \"%s %s\"]\n",PROGRAM,VERSION);
   else
     fprintf (fp, "[Black \"%s\"]\n",name);

   if (pgn_whiteELO)
     fprintf (fp, "[WhiteELO \"%s\"]\n", NULL2EMPTY(pgn_white));
   else
     fprintf(fp, "[WhiteELO \"%d\"]\n",computer==white?myrating:opprating);
   if (pgn_blackELO)
     fprintf (fp, "[BlackELO \"%s\"]\n", NULL2EMPTY(pgn_black));
   else
     fprintf (fp, "[BlackELO \"%d\"]\n",computer==white?opprating:myrating);

   if (pgn_result)
     fprintf (fp, "[Result \"%s\"]\n", pgn_result);
   else {
     /* Revive the little-known standard functions! */
     len = strcspn(resultstr," {");
     fprintf (fp, "[Result \"%.*s\"]\n", len, resultstr);
   }

   if (pgn_othertags) {
     fprintf (fp, "%s", pgn_othertags);
   }

   fprintf (fp, "\n");

   if (initial_comments)
   {
     fprintf(fp, "\n%s\n", initial_comments);
     /* If it doesn't end in \n, add it */
     if (initial_comments[0] &&
         initial_comments[strlen(initial_comments)-1] != '\n')
        fprintf(fp, "\n");
   }

   s[0] = '\0';
   for (i = 0; i <= GameCnt; i++)
   {
      if (! (i % 2)) {
        sprintf (s, "%s%d. ", s, i/2 + 1);
      }
      sprintf (s, "%s%s ", s, Game[i].SANmv);
      if (strlen (s) > 80)
      {
         p = s + 79;
         while (*p-- != ' ');
         *++p = '\0';
         fprintf (fp, "%s\n", s);
         strcpy (s, p+1);
      }
      if (Game[i].comments) {
         fprintf (fp, "%s\n", s);
         fprintf (fp, "%s", Game[i].comments);
         if (Game[i].comments[0] &&
             Game[i].comments[strlen(Game[i].comments)-1] != '\n')
              fprintf(fp, "\n");
         s[0] = '\0';
      }
   }
   fprintf (fp, "%s", s);
   fprintf (fp, "%s", resultstr);
   fprintf (fp, "\n\n");
   fclose (fp);

}


void PGNReadFromFile (const char *file)
/****************************************************************************
 *
 *  To read a game from a PGN file.
 *
 ****************************************************************************/
{
   FILE *fp;

   fp = fopen (file, "r");
   if (fp == NULL)
   {
      printf ("Cannot open file %s\n", file);
      return;
   }
   yyin = fp;

   InitVars ();

   data_dest = DEST_GAME;
   (void) yylex();

   fclose (fp);
   ShowBoard ();
   TTClear ();
}

/* Only players in the table below are permitted into the opening book
   from the PGN files. Please expand the table as needed. Generally,
   I would recommend only acknowledged GM's and IM's and oneself, but
   because of the self-changing nature of the book, anything inferior
   will eventually be eliminated through automatic play as long as
   you feed the games the program plays back to itself with "book add pgnfile"
*/
/* TODO: Fix this so the list isn't hardcoded. */

static const char *const trusted_players[] = {
  "Alekhine",
  "Adams",
  "Anand",
  "Anderssen",
  "Andersson",
  "Aronin",
  "Averbakh",
  "Balashov",
  "Beliavsky",
  "Benko",
  "Bernstein",
  "Bird",
  "Bogoljubow",
  "Bolbochan",
  "Boleslavsky",
  "Byrne",
  "Botvinnik",
  "Bronstein",
  "Browne",
  "Capablanca",
  "Chigorin",
  "Christiansen",
  "De Firmian",
  "Deep Blue",
  "Deep Thought",
  "Donner",
  "Dreev",
  "Duras",
  "Euwe",
  "Evans",
  "Filip",
  "Fine",
  "Fischer",
  "Flohr",
  "Furman",
  "Gelfand",
  "Geller",
  "Gereben",
  "Glek",
  "Gligoric",
  "GNU",
  "Golombek",
  "Gruenfeld",
  "Guimard",
  "Hodgson",
  "Ivanchuk",
  "Ivkov",
  "Janowsky",
  "Kamsky",
  "Kan",
  "Karpov",
  "Kasparov",
  "Keres",
  "Korchnoi",
  "Kortschnoj",
  "Kotov",
  "Kramnik",
  "Kupreich",
  "Lasker",
  "Lautier",
  "Letelier",
  "Lilienthal",
  "Ljubojevic",
  "Marshall",
  "Maroczy",
  "Mieses",
  "Miles",
  "Morphy",
  "Mueller",     /* Every other German has this name... */
  "Nimzowitsch",
  "Nunn",
  "Opocensky",
  "Pachman",
  "Petrosian",
  "Piket",
  "Pilnik",
  "Pirc",
  "Polgar",
  "Portisch",
  "Psakhis",
  "Ragozin",
  "Reshevsky",
  "Reti",
  "Romanish",
  "Rubinstein",
  "Saemisch",
  "Seirawan",
  "Shirov",
  "Short",
  "Silman",
  "Smyslov",
  "Sokolsky",
  "Spassky",
  "Sveshnikov",
  "Stahlberg",
  "Steinitz",
  "Tal",
  "Tarjan",
  "Tartakower",
  "Timman",
  "Topalov",
  "Torre",
  "Vidmar"

  };

int IsTrustedPlayer(const char *name)
/* Return 1 if name in trusted_players list, else 0 */
{
	int i;
	for (i = 0; i < (sizeof(trusted_players) / sizeof(*trusted_players));
	    	i++) {
		if (strstr(name, trusted_players[i]) != NULL)
			return 1;
       }
	return 0;
}

void BookPGNReadFromFile (const char *file)
/****************************************************************************
 *
 *  To read a game from a PGN file and store out the hash entries to book.
 *
 ****************************************************************************/
{
   FILE *fp;
   int ngames = 0;
   time_t t1, t2;
   double et;
   int error;

   /* TODO: Fix reading from file */

   et = 0.0;
   t1 = time(NULL);

   fp = fopen (file, "r");
   if (fp == NULL)
   {
     fprintf(stderr, "Cannot open file %s: %s\n",
	     file, strerror(errno));
     return;
   }
   yyin = fp;

   /* Maybe add some more clever error handling later */
   if (BookBuilderOpen() != BOOK_SUCCESS)
     return;
   newpos = existpos = 0;
   data_dest = DEST_BOOK;

   while(1) {
     InitVars ();
     NewPosition ();
     CLEAR (flags, MANUAL);
     CLEAR (flags, THINK);
     myrating = opprating = 0;

     error = yylex();
     if (error) break;

     ngames++;
     if (ngames % 10 == 0) printf("Games processed: %d\r",ngames);
     fflush(stdout);
   }

   fclose (fp);
   if (BookBuilderClose() != BOOK_SUCCESS) {
     perror("Error writing opening book during BookBuilderClose");
   }

   /* Reset the board otherwise we leave the last position in the book
      on the board. */
   InitVars ();
   NewPosition ();
   CLEAR (flags, MANUAL);
   CLEAR (flags, THINK);
   myrating = opprating = 0;

   t2 = time(NULL);
   et += difftime(t2, t1);
   putchar('\n');

   /* Handle divide-by-zero problem */
   if (et < 0.5) { et = 1.0; };

   printf("Time = %.0f seconds\n", et);
   printf("Games compiled: %d\n",ngames);
   printf("Games per second: %f\n",ngames/et);
   printf("Positions scanned: %d\n",newpos+existpos);
   printf("Positions per second: %f\n",(newpos+existpos)/et);
   printf("New & unique added: %d positions\n",newpos);
   printf("Duplicates not added: %d positions\n",existpos);
}

/* GNU Chess 5.0 - cmd.c - command parser
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "version.h"
#include "common.h"
#include "eval.h"

static char logfile[MAXSTR];
static char gamefile[MAXSTR];

/*
 * Splitting input is actually not neccessary, but we find
 * tokens separated by whitespace and put pointers on them.
 * How many do we need? We just take 3 for now. Check if the
 * fact that tokens are not 0-terminated but whitespace-terminated
 * generates bugs. (Already killed one bug in move.c)
 * We also kill trailing whitespace. (The trailing '\n' might
 * be really annoying otherwise.)
 */

#define TOKENS 3

static char *token[TOKENS];
char *endptr;

static void split_input(void)
{
  /* r points to the last non-space character */
  char *s, *r;
  int k;

  for (k = 0, s = r = inputstr; k < TOKENS; ++k) {
    /* Skip leading whitespace */
    while (isspace(*s)) s++;
    token[k] = s;
    /* Skip token */
    while (*s && !isspace(*s)) r = s++;
  }
  while (*s) {
    while (isspace(*s)) s++;
    while (*s && !isspace(*s)) r = s++;
  }
  r[1] = '\0';
}

/*
 * Compares two tokens, returns 1 on equality. Tokens
 * are separated by whitespace.
 */
static int tokeneq(const char *s, const char *t)
{
  while (*s && *t && !isspace(*s) && !isspace(*t)) {
    if (*s++ != *t++) return 0;
  }
  return (!*s || isspace(*s)) && (!*t || isspace(*t));
}
  
void cmd_accepted(void) {}

void cmd_activate(void) 
{
  CLEAR (flags, TIMEOUT);
  CLEAR (flags, ENDED);
}
 
void cmd_analyze(void)
{
  /*
   * "analyze" mode is similar to force, hard and post together
   * in that it produces a text output like post, but must
   * think indefinitely like ponder.
   *
   * Some additional output is expected in command ".\n" but if ignored
   * this should not be sent any more
   */

/* TODO correct output, add fail high low */

   if (!(flags & ANALYZE)){
     preanalyze_flags=flags; /* save these flags for exit */
     SET (flags, ANALYZE);
     cmd_post();
     cmd_force();
     cmd_hard();
   }
}

void cmd_bk(void)
{
  /* Print moves from Open Book for Xboard/WinBoard */
  /* Lines must start with " " and end with blank line */
  /* No need to test for xboard as it is generally useful */
  BookQuery(1);
  printf("\n"); /* Blank line */
  fflush(stdout);
}

void cmd_black(void) 
{
 /* 
  * No longer used by Xboard but requested as a feature
  */

  NewPosition(); /* Reset some flags and settings */
  CLEAR ( flags, THINK);
  CLEAR ( flags, MANUAL);
  CLEAR (flags, TIMEOUT);   
  computer = white;
  if ( board.side == white ) {
    board.side = black;
    board.ep = -1; 
  }
}

void cmd_book(void)
{
  if (tokeneq(token[1], "add")) {
    if (access(token[2], F_OK) < 0) {
      printf("The syntax to add a new book is:\n\n\tbook add file.pgn\n");
    } else {
      BookPGNReadFromFile (token[2]);
    }
  } else if (tokeneq (token[1], "on") || tokeneq(token[1], "prefer")) {
    bookmode = BOOKPREFER;
    printf("book now on.\n");
  } else if (tokeneq (token[1], "off")) {
    bookmode = BOOKOFF;
    printf("book now off.\n");
  } else if (tokeneq (token[1], "best")) {
    bookmode = BOOKBEST;
    printf("book now best.\n");
  } else if (tokeneq (token[1], "worst")) {
    bookmode = BOOKWORST;
    printf("book now worst.\n");
  } else if (tokeneq (token[1], "random")) {
    bookmode = BOOKRAND;
    printf("book now random.\n");
  }
}

/* Our opponent is a computer */
void cmd_computer(void) {}

void cmd_depth(void)
{
  SearchDepth = atoi (token[1]);
  printf("Search to a depth of %d\n",SearchDepth);
}

/* Ignore draw offers */
void cmd_draw(void) {}

void cmd_easy(void) { CLEAR (flags, HARD); }

/* Predecessor to setboard */
void cmd_edit(void) 
{
  if ( flags & XBOARD ) {
    printf("tellusererror command 'edit' not implemented\n");
    fflush(stdout);
  }
}

void cmd_epd(void)
{
  ParseEPD (token[1]);
  NewPosition();
  ShowBoard();
  printf ("\n%s : Best move = %s\n", id, solution); 
}

void cmd_exit(void) 
{ 
  /*
   * "exit" is a synonym for quit except in engine mode
   * when it means leave analyze mode
   */

  if ( flags & ANALYZE ){
	flags = preanalyze_flags ; /* this implicitly clears ANALYZE flag */
  } else {
    cmd_quit(); 
  }
   

}

void cmd_force(void) { SET (flags, MANUAL); }

void cmd_go(void)
{
  SET (flags, THINK);
  CLEAR (flags, MANUAL);
  CLEAR (flags, TIMEOUT);
  CLEAR (flags, ENDED);
  computer = board.side;
}

void cmd_hard(void) { SET (flags, HARD); }

void cmd_hash(void)
{
  if (tokeneq (token[1], "off"))
    CLEAR (flags, USEHASH);
  else if (tokeneq (token[1], "on"))
    SET (flags, USEHASH);
  printf ("Hashing %s\n", flags & USEHASH ? "on" : "off");
}

void cmd_hashsize(void)
{
  if (token[1][0] == 0) {
    printf("Current HashSize is %d slots\n", HashSize);
  } else {
    int i;
    /* i = atoi (token[1]); */
    errno = 0;
    i = strtol (token[1], &endptr, 10);
    if ( errno != 0 || *endptr != '\0' ){
      printf("Hashsize out of Range or Invalid\n");
    }
    else {
     CalcHashSize(i);
     InitHashTable (); 
    }
  }
}

/* Give a possible move for the player to play */
void cmd_hint(void)
{
  if ( flags & ENDED ){
    printf("The game is over.\n");
  } else {
    int HintMove;
    HintMove = TreePtr[1]->move; /* Pick first move in tree */
    if (IsLegalMove(HintMove)){
      GenMoves(1);
      SANMove(TreePtr[1]->move,1);
      printf("Hint: %s\n", SANmv);
    } else {
      printf("No hint available at this time\n");
    }
  }
  fflush(stdout);
}

void cmd_level(void)
{
  SearchDepth = 0;
  sscanf (token[1], "%d %f %d", &TCMove, &TCTime, &TCinc);
  if (TCMove == 0) {
    TCMove =  35 /* MIN((5*(GameCnt+1)/2)+1,60) */;
    printf("TCMove = %d\n",TCMove);
    suddendeath = 1;
  } else
    suddendeath = 0;
  if (TCTime == 0) {
    SET (flags, TIMECTL);
    SearchTime = TCinc / 2.0f ;
    printf("Fischer increment of %d seconds\n",TCinc);
  } else {
    SET (flags, TIMECTL);
    MoveLimit[white] = MoveLimit[black] = TCMove - (GameCnt+1)/2;
    TimeLimit[white] = TimeLimit[black] = TCTime * 60;
    if (!(flags & XBOARD)) {
      printf ("Time Control: %d moves in %.2f secs\n", 
	      MoveLimit[white], TimeLimit[white]);
      printf("Fischer increment of %d seconds\n",TCinc);
    }
  }
}

void cmd_list(void)
{
  if (token[1][0] == '?') {
    printf("name    - list known players alphabetically\n");
    printf("score   - list by GNU best result first \n");
    printf("reverse - list by GNU worst result first\n");
  } else {
    if (token[1][0] == '\0') DBListPlayer("rscore");
    else DBListPlayer(token[1]);
  }
}

void cmd_load(void)
{
  LoadEPD (token[1]);
  if (!ValidateBoard()) {
    SET (flags, ENDED);
    printf ("Board is wrong!\n");
  }
}

void cmd_manual(void) { SET (flags, MANUAL); }

/* Move now, not applicable */
void cmd_movenow(void) {}

/*
 * TODO: Add a logpath variable or macro, not always dump into current
 * dir. Again, how does one handle paths portably across Unix/Windows?
 *   -- Lukas 
 */
void cmd_name(void)
{
  int suffix = 0;

  /* name[sizeof name - 1] should always be 0 */
  strncpy(name, token[1], sizeof name - 1);
  for (suffix = 0; suffix < 1000; suffix++) {
    sprintf(logfile,"log.%03d",suffix);
    sprintf(gamefile,"game.%03d",suffix);
    /*
     * There is an obvious race condition here but who cares, we just
     * bail out in case of failure... --Lukas 
     */
    if (access(logfile,F_OK) < 0) {
      ofp = fopen(logfile,"w");
      if (ofp == NULL) {
	ofp = stdout;
	fprintf(stderr, "Failed to open %s for writing: %s\n", 
		logfile, strerror(errno));
      }
      return;
    }
  }
  fprintf(stderr, "Could not create logfile, all slots occupied.\n");
  fprintf(stderr, "You may consider deleting or renaming your existing logfiles.\n");
}

void cmd_new(void)
{
  InitVars ();
  NewPosition ();
  /* Protocol specification for ANALYZE says "new" does not end analysis */
  if (!(flags & ANALYZE))
    CLEAR (flags, MANUAL);
  CLEAR (flags, THINK);
  myrating = opprating = 0;
}

void cmd_nopost(void) {	CLEAR (flags, POST); }

void cmd_null(void)
{
  if (tokeneq (token[1], "off"))
    CLEAR (flags, USENULL);
  else if (tokeneq (token[1], "on"))
    SET (flags, USENULL);
  printf ("Null moves %s\n", flags & USENULL ? "on" : "off");
}

void cmd_otim(void) {}

void cmd_pgnload(void) { PGNReadFromFile (token[1]); }

/*
 * XXX - Filenames with spaces will break here,
 * do we want to support them? I vote for "no" 
 *   - Lukas
 */
void cmd_pgnsave(void)
{
  if ( strlen(token[1]) > 0 )
    PGNSaveToFile (token[1], "");
  else
    printf("Invalid filename.\n");
}
 
void cmd_ping(void)
{
  /* If ping is received when we are on move, we are supposed to 
     reply only after moving.  In this version of GNU Chess, we
     never read commands while we are on move, so we don't have to
     worry about that here. */
  printf("pong %s\n", token[1]);
  fflush(stdout);
}
 
void cmd_post(void) { SET (flags, POST); }

void cmd_protover(void)
{
  if (flags & XBOARD) {
    /* Note: change this if "draw" command is added, etc. */
    printf("feature setboard=1 analyze=1 ping=1 draw=0 sigint=0"
	   " variants=\"normal\" myname=\"%s %s\" done=1\n",
	   PROGRAM, VERSION);
    fflush(stdout);
  }
}

void cmd_quit(void) { SET (flags, QUIT); }

void cmd_random(void) {}

void cmd_rating(void)
{
  myrating = atoi(token[1]);
  opprating = atoi(token[2]);
  fprintf(ofp,"my rating = %d, opponent rating = %d\n",myrating,opprating); 
  /* Change randomness of book based on opponent rating. */
  /* Basically we play narrower book the higher the opponent */
  if (opprating >= 1700) bookfirstlast = 2;
  else if (opprating >= 1700) bookfirstlast = 2;
  else bookfirstlast = 2;
}

void cmd_rejected(void) {}

void cmd_remove(void)
{
  if (GameCnt >= 0) {
    CLEAR (flags, ENDED);
    CLEAR (flags, TIMEOUT);
    UnmakeMove (board.side, &Game[GameCnt].move);
    if (GameCnt >= 0) {
      UnmakeMove (board.side, &Game[GameCnt].move);
      if (!(flags & XBOARD))
           ShowBoard ();
    }
    PGNSaveToFile ("game.log","");
  } else
    printf ("No moves to undo! \n");
}

void cmd_result(void)
{
  if (ofp != stdout) {  
    fprintf(ofp, "result: %s\n",token[1]);
    fclose(ofp); 
    ofp = stdout;
    printf("Save to %s\n",gamefile);
    PGNSaveToFile (gamefile, token[1]);
    DBUpdatePlayer (name, token[1]);
  }
}
	
void cmd_save(void)
{  
  if ( strlen(token[1]) > 0 )
    SaveEPD (token[1]);
  else
    printf("Invalid filename.\n");
}

void cmd_setboard(void)
{
  /* setboard uses FEN, not EPD, but ParseEPD will accept FEN too */
  ParseEPD (token[1]);
  NewPosition();
}

void cmd_solve(void) { Solve (token[1]); }

/* Set total time for move to be N seconds is "st N" */
void cmd_st(void) 
{
  /* Approximately level 1 0 N */
  sscanf(token[1],"%d",&TCinc);
  suddendeath = 0 ;
  /* Allow a little fussiness for failing low etc */
  SearchTime = TCinc * 0.90f ;
  CLEAR (flags, TIMECTL);
}

void cmd_switch(void)
{
  board.side = 1^board.side;
  board.ep = -1 ; /* Enpassant doesn't apply after switch */
  printf ("%s to move\n", board.side == white ? "White" : "Black");
}

void cmd_time(void)
{
  TimeLimit[1^board.side] = atoi(token[1]) / 100.0f ;
}

void cmd_undo(void)
{
  if (GameCnt >= 0)
    UnmakeMove (board.side, &Game[GameCnt].move);
  else
    printf ("No moves to undo! \n");
  MoveLimit[board.side]++;
  TimeLimit[board.side] += Game[GameCnt+1].et;
  if (!(flags & XBOARD)) ShowBoard ();
}

void cmd_usage(void) 
{
      printf (
     "\n"
     " Usage: %s [OPTION]\n"
     "\n"
     " -h, --help         display this help and exit\n"
     " -v, --version      display version information and exit\n" 
     "\n"
     " -x, --xboard       start in engine mode\n"
     " -p, --post   	   start up showing thinking\n"
     " -e, --easy   	   disable thinking in opponents time\n"
     " -m, --manual  	   enable manual mode\n"
     " -s size, --hashsize=size   specify hashtable size in slots\n"
     "\n"
     " Options xboard and post are accepted without leading dashes\n"
     " for backward compatibility\n"
     "\n"
     "Report bugs to <bug-gnu-chess@gnu.org>.\n"
     "\n", progname);
     }


/* Play variant, we instruct interface in protover we play normal */
void cmd_variant(void) {}

void cmd_version(void)
{
   if (!(flags & XBOARD))
     printf ("%s %s\n", PROGRAM, VERSION);
   else
     printf ("Chess\n");
}

void cmd_white(void) 
{
 /* 
  * No longer used by Xboard but requested as a feature
  */

  NewPosition(); /* Reset some flags and settings */
  CLEAR ( flags, THINK);
  CLEAR ( flags, MANUAL);
  CLEAR (flags, TIMEOUT);   
  computer = black;
  if ( board.side == black ){
    board.side = white;
    board.ep = -1; /* Hack to fixed appearing pawn bug */
  }
}

void cmd_xboard(void)
{
  if (tokeneq (token[1], "off"))
    CLEAR (flags, XBOARD);
  else if (tokeneq (token[1], "on"))
    SET (flags, XBOARD);
  else if (!(flags & XBOARD)) { /* set if unset and only xboard called */
    SET (flags, XBOARD);	    /* like in xboard/winboard usage */
  }
}

/*
 * Command with subcommands, could write secondary method
 * tables here
 */

void cmd_show (void)
/************************************************************************
 *
 *  The show command driver section.
 *
 ************************************************************************/
{
   if (tokeneq (token[1], "board"))
      ShowBoard ();
   else if (tokeneq (token[1], "rating"))
   {
      printf("My rating = %d\n",myrating);
      printf("Opponent rating = %d\n",opprating);
   } 
   else if (tokeneq (token[1], "time"))
      ShowTime ();
   else if (tokeneq (token[1], "moves")) {
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenMoves (1);      
      ShowMoveList (1);
      printf ("No. of moves generated = %ld\n", GenCnt);
   }
   else if (tokeneq (token[1], "escape")) {
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenCheckEscapes (1);      
      ShowMoveList (1);
      printf ("No. of moves generated = %ld\n", GenCnt);
   }
   else if (tokeneq (token[1], "noncapture"))
   {
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenNonCaptures (1);      
      FilterIllegalMoves (1);
      ShowMoveList (1);
      printf ("No. of moves generated = %ld\n", GenCnt);
   }
   else if (tokeneq (token[1], "capture"))
   {
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenCaptures (1);      
      FilterIllegalMoves (1);
      ShowMoveList (1);
      printf ("No. of moves generated = %ld\n", GenCnt);
   }
   else if (tokeneq (token[1], "eval") || tokeneq (token[1], "score"))
   {
      int s, wp, bp, wk, bk;
      BitBoard *b;

      phase = PHASE;
      GenAtaks ();
      FindPins (&pinned);
      hunged[white] = EvalHung(white);
      hunged[black] = EvalHung(black);
      b = board.b[white];
      pieces[white] = b[knight] | b[bishop] | b[rook] | b[queen]; 
      b = board.b[black];
      pieces[black] = b[knight] | b[bishop] | b[rook] | b[queen]; 
      wp = ScoreP (white);
      bp = ScoreP (black);
      wk = ScoreK (white);
      bk = ScoreK (black);
      printf ("White:  Mat:%4d/%4d  P:%d  N:%d  B:%d  R:%d  Q:%d  K:%d  Dev:%d  h:%d x:%d\n",
	board.pmaterial[white], board.material[white], wp, ScoreN(white), 
        ScoreB(white), ScoreR(white), ScoreQ(white), wk, 
        ScoreDev(white), hunged[white], ExchCnt[white]);
      printf ("Black:  Mat:%4d/%4d  P:%d  N:%d  B:%d  R:%d  Q:%d  K:%d  Dev:%d  h:%d x:%d\n",
	board.pmaterial[black], board.material[black], bp, ScoreN(black), 
        ScoreB(black), ScoreR(black), ScoreQ(black), bk,
        ScoreDev(black), hunged[black], ExchCnt[black]);
      printf ("Phase: %d\t", PHASE);
      s = ( EvaluateDraw () ? DRAWSCORE : Evaluate (-INFINITY, INFINITY));
      printf ("score = %d\n", s);
      printf ("\n");
   }
   else if (tokeneq (token[1], "game"))
     ShowGame ();
   else if (tokeneq (token[1], "pin"))
   {
      BitBoard b;
      GenAtaks ();
      FindPins (&b);
      ShowBitBoard (&b);
   }
}

void cmd_test (void)
/*************************************************************************
 *
 *  The test command driver section.
 *
 *************************************************************************/
{
  if (tokeneq (token[1], "movelist"))
    TestMoveList ();
  else if (tokeneq (token[1], "capture"))
    TestCaptureList ();
  else if (tokeneq (token[1], "movegenspeed"))
    TestMoveGenSpeed ();
  else if (tokeneq (token[1], "capturespeed"))
    TestCaptureGenSpeed ();
  else if (tokeneq (token[1], "eval"))
    TestEval ();
  else if (tokeneq (token[1], "evalspeed"))
    TestEvalSpeed ();
}

/*
 * This is more or less copied from the readme, and the
 * parser is not very clever, so the lines containing
 * command names should not be indented, the lines with
 * explanations following them should be indented. Do not
 * use tabs for indentation, only spaces. CAPITALS are
 * reserved for parameters in the command names. The
 * array must be terminated by two NULLs.
 * 
 * This one should be integrated in the method table.
 * (Very much like docstrings in Lisp.)
 */

static const char * const helpstr[] = {
   "^C",
   " Typically the interrupt key stops a search in progress,",
   " makes the move last considered best and returns to the",
   " command prompt",
   "quit",
   " quit the program.",
   "exit",
   " In analysis mode this stops analysis, otherwise it quits the program.",
   "help",
   " Produces a help blurb corresponding to this list of commands.",
   "book",
   " add - compiles book.dat from book.pgn",
   " on - enables use of book",
   " off - disables use of book",
   " worst - play worst move from book",
   " best - play best move from book",
   " prefer - default, same as 'book on'",
   " random - play any move from book",
   "version",
   " prints out the version of this program",
   "pgnsave FILENAME",
   " saves the game so far to the file from memory",
   "pgnload FILENAME",
   " loads the game in the file into memory",
   "force",
   "manual",
   " Makes the program stop moving. You may now enter moves",
   " to reach some position in the future.",
   " ",
   "white",
   " Program plays white",
   "black",
   " Program plays black",
   "go",
   " Computer takes whichever side is on move and begins its",
   " thinking immediately",
   "post",
   " Arranges for verbose thinking output showing variation, score,",
   " time, depth, etc.",
   "nopost",
   " Turns off verbose thinking output",
   "name NAME",
   " Lets you input your name. Also writes the log.nnn and a",
   " corresponding game.nnn file. For details please see",
   " auxillary file format sections.",
   "result",
   " Mostly used by Internet Chess server.",
   "activate",
   " This command reactivates a game that has been terminated automatically",
   " due to checkmate or no more time on the clock. However, it does not",
   " alter those conditions. You would have to undo a move or two or",
   " add time to the clock with level or time in that case.",
   "rating COMPUTERRATING OPPONENTRATING",
   " Inputs the estimated rating for computer and for its opponent",
   "new",
   " Sets up new game (i.e. positions in original positions)",
   "time",
   " Inputs time left in game for computer in hundredths of a second.",
   " Mostly used by Internet Chess server.",
   "hash",
   " on - enables using the memory hash table to speed search",
   " off - disables the memory hash table",
   "hashsize N",
   " Sets the hash table to permit storage of N positions",
   "null",
   " on - enables using the null move heuristic to speed search",
   " off - disables using the null move heuristic",
   "xboard",
   " on - enables use of xboard/winboard",
   " off - disables use of xboard/winboard",
   "depth N",
   " Sets the program to look N ply (half-moves) deep for every",
   " search it performs. If there is a checkmate or other condition",
   " that does not allow that depth, then it will not be ",
   "level MOVES MINUTES INCREMENT",
   " Sets time control to be MOVES in MINUTES with each move giving",
   " an INCREMENT (in seconds, i.e. Fischer-style clock).",
   "load",
   "epdload",
   " Loads a position in EPD format from disk into memory.",
   "save",
   "epdsave",
   " Saves game position into EPD format from memory to disk.",
   "switch",
   " Switches side to move",
   "solve FILENAME",
   "solveepd FILENAME",
   " Solves the positions in FILENAME",
   "remove",
   " Backs up two moves in game history",
   "undo",
   " Backs up one move in game history",
   "usage",
   " Display command line syntax",
   "show",
   " board - displays the current board",
   " time - displays the time settings",
   " moves - shows all moves using one call to routine",
   " escape - shows moves that escape from check using one call to routine",
   " noncapture - shows non-capture moves",
   " capture - shows capture moves",
   " eval [or score] - shows the evaluation per piece and overall",
   " game - shows moves in game history",
   " pin - shows pinned pieces",
   "test",
   " movelist - reads in an epd file and shows legal moves for its entries",
   " capture - reads in an epd file and shows legal captures for its entries",
   " movegenspeed - tests speed of move generator",
   " capturespeed - tests speed of capture move generator",
   " eval - reads in an epd file and shows evaluation for its entries",
   " evalspeed tests speed of the evaluator",
   "bk",
   " show moves from opening book.",
   NULL,
   NULL
};

void cmd_help (void)
/**************************************************************************
 *
 *  Display all the help commands.
 *
 **************************************************************************/
{
   const char * const *p;
   int count, len;

   if (strlen(token[1])>0) {
      for (p=helpstr, count=0; *p; p++) {
	 if  (strncmp(*p, token[1], strlen(token[1])) == 0) {
	    puts(*p);
	    while (*++p && **p != ' ') /* Skip aliases */ ;
	    for (; *p && **p == ' '; p++) {
	       puts(*p);
	    }
	    return;
	 }
      }
      printf("Help for command %s not found\n\n", token[1]);
   }
   printf("List of commands: (help COMMAND to get more help)\n");
   for (p=helpstr, count=0; *p; p++) {
      len = strcspn(*p, " ");
      if (len > 0) {
	 count += printf("%.*s  ", len, *p);
	 if (count > 60) {
	    count = 0;
	    puts("");
	 }
      }
   }
   puts("");
}

/*
 * Try a method table, one could also include the documentation
 * strings here
 */

struct methodtable {
  const char *name;
  void (*method) (void);
};

/* Last entry contains to NULL pointers */

/* List commands we don't implement to avoid illegal moving them */

const struct methodtable commands[] = {
  { "?", cmd_movenow },
  { "accepted", cmd_accepted },
  { "activate", cmd_activate },
  { "analyze", cmd_analyze },
  { "bk", cmd_bk },
  { "black", cmd_black },
  { "book", cmd_book },
  { "computer", cmd_computer },
  { "depth", cmd_depth },
  { "draw", cmd_draw },
  { "easy", cmd_easy },
  { "edit", cmd_edit },
  { "epd", cmd_epd },
  { "epdload", cmd_load },
  { "epdsave", cmd_save },
  { "exit", cmd_exit },
  { "force", cmd_force },
  { "go", cmd_go },
  { "hard", cmd_hard },
  { "hash", cmd_hash },
  { "hashsize", cmd_hashsize },
  { "help", cmd_help },
  { "hint", cmd_hint },
  { "level", cmd_level },
  { "list", cmd_list },
  { "load", cmd_load },
  { "manual", cmd_manual },
  { "name", cmd_name },
  { "new", cmd_new },
  { "nopost", cmd_nopost },
  { "null", cmd_null },
  { "otim", cmd_otim },
  { "pgnload", cmd_pgnload },
  { "pgnsave", cmd_pgnsave },
  { "ping", cmd_ping },
  { "post", cmd_post },
  { "protover", cmd_protover },
  { "quit", cmd_quit },
  { "random", cmd_random },
  { "rating", cmd_rating },
  { "rejected", cmd_rejected },
  { "remove", cmd_remove },
  { "result", cmd_result },
  { "save", cmd_save },
  { "setboard", cmd_setboard },
  { "show", cmd_show },
  { "solve", cmd_solve },
  { "solveepd", cmd_solve },
  { "st", cmd_st },
  { "switch", cmd_switch },
  { "test", cmd_test },
  { "time", cmd_time },
  { "undo", cmd_undo },
  { "usage", cmd_usage },
  { "variant", cmd_variant },
  { "version", cmd_version },
  { "white", cmd_white },
  { "xboard", cmd_xboard },
  { NULL, NULL }
};

void parse_input(void)
/*************************************************************************
 *
 *  This is the main user command interface driver.
 *
 *************************************************************************/
{
   leaf *ptr; 
   const struct methodtable * meth;
 
   dbg_printf("parse_input() called, inputstr = *%s*\n", inputstr);
   
   split_input();

   for (meth = commands; meth->name != NULL; meth++) {
     if (tokeneq(token[0], meth->name)) {
       meth->method();
       return;
     }
   }

   /* OK, no known command, this should be a move */
   ptr = ValidateMove (token[0]);
   if (ptr != NULL) {
     SANMove (ptr->move, 1);
     MakeMove (board.side, &ptr->move);
     strcpy (Game[GameCnt].SANmv, SANmv);
     printf("%d. ",GameCnt/2+1);
     printf("%s",token[0]);
     if (ofp != stdout) {
       fprintf(ofp,"%d. ",GameCnt/2+1);
       fprintf(ofp,"%s",token[0]);
     }
     putchar('\n');
     fflush(stdout);
     if (ofp != stdout) {
       fputc('\n',ofp);
       fflush(ofp);
     }
     if (!(flags & XBOARD)) ShowBoard (); 
     SET (flags, THINK);
   }
   else {
     /*
      * Must Output Illegal move to prevent Xboard accepting illegal
      * en passant captures and other subtle mistakes
      */
     printf("Illegal move: %s\n",token[0]);
     fflush(stdout);
   }  
}


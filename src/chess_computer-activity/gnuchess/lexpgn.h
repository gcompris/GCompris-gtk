#ifndef LEXPGNX_H

#define LEXPGNX_H

/* lexpgnx.h - This header file defines the types and global variables
 * exported from the generated lexpgn file to others.
 * It's named "lexpgnx" and not "lexpgn", because some lex-like program
 * might automatically generate a file named lexpgn.h and we don't
 * want this file to be overwritten.
 */

enum data_destination_t { DEST_TRASH, DEST_GAME, DEST_BOOK };
extern enum data_destination_t data_dest;

extern char *initial_comments;
extern char *pgn_event;
extern char *pgn_site;
extern char *pgn_date;
extern char *pgn_round;
extern char *pgn_white;
extern char *pgn_black;
extern char *pgn_whiteELO;
extern char *pgn_blackELO;
extern char *pgn_result;
extern char *pgn_othertags;

#endif

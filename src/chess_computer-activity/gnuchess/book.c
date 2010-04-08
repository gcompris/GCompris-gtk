/* GNU Chess 5.0 - book.c - book code
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
     lukas@debian.org
*/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "common.h"
#include "book.h"

#define MAXMOVES 200
#define MAXMATCH 100

/*
 * BookQuery() tries the following filenames in order to find
 * a valid opening book. The array must be NULL-terminated.
 */
static char const * bookbin[] = {
   "book.dat",
   "/usr/share/games/gnuchess/book.dat",
   "/usr/lib/games/gnuchess/book.dat",
   NULL
};

static int bookcnt;
static HashType posshash[MAXMOVES];

/*
 * This is the only authoritative variable telling us
 * whether a book has been allocated or not. (Other
 * parts may mess around with bookloaded.)
 */
static int book_allocated = 0;

/*
 * The last byte of magic_str should be the version
 * number of the format, in case we have to change it.
 *
 * Format 0x01 had an index field which is now gone.
 *
 * Format 0x03 uses an additional size header, which
 * comes directly after the magic string, and has the
 * number of entries as a big-endian encoded uint32_t
 * number.
 */

#define MAGIC_LENGTH 5

static const char magic_str[] = "\x42\x23\x08\x15\x03";

static int check_magic(FILE *f)
{
  char buf[MAGIC_LENGTH];
  int r;

  r = fread(&buf, 1, MAGIC_LENGTH, f);
  return (r == MAGIC_LENGTH &&
	  memcmp(buf, magic_str, MAGIC_LENGTH) == 0);
}

static int write_magic(FILE *f)
{
  if (MAGIC_LENGTH != fwrite(&magic_str, 1, MAGIC_LENGTH, f)) {
    return BOOK_EIO;
  } else {
    return BOOK_SUCCESS;
  }
}

/* Write and read size information for the binary book */

static int write_size(FILE *f, uint32_t size)
{
  unsigned char sizebuf[4];
  int k;

  for (k = 0; k < 4; k++) {
    sizebuf[k] = (size >> ((3-k)*8)) & 0xff;
  }
  if (1 == fwrite(&sizebuf, sizeof(sizebuf), 1, f)) {
    return BOOK_SUCCESS;
  } else {
    return BOOK_EIO;
  }
}

/* Returns 0 if some read error occurs */

static uint32_t read_size(FILE *f)
{
  unsigned char sizebuf[4];
  uint32_t size = 0;
  int k;

  if (1 != fread(&sizebuf, sizeof(sizebuf), 1, f)) {
    return 0;
  }
  for (k = 0; k < 4; k++) {
    size = (size << 8) | sizebuf[k];
  }
  return size;
}

/*
 * We now allocate memory for the book dynamically,
 * according to the size field in the header of the binary
 * book. However, during book building the following value
 * is used.
 */

#define MAX_DIGEST_BITS 20

static int digest_bits;

#define DIGEST_SIZE (1UL << digest_bits)
#define DIGEST_MASK (DIGEST_SIZE - 1)

static struct hashtype {
  uint16_t wins;
  uint16_t losses;
  uint16_t draws;
  HashType key;
} *bookpos;

static inline int is_empty(uint32_t index)
{
  return
    bookpos[index].key    == 0 &&
    bookpos[index].wins   == 0 &&
    bookpos[index].draws  == 0 &&
    bookpos[index].losses == 0;
}

/*
 * Initial hash function, relies on the quality of the lower
 * bits of the 64 bit hash function
 */
#define DIGEST_START(i,key) \
  ((i) = (key) & DIGEST_MASK)

/*
 * See if there is already a matching entry
 */
#define DIGEST_MATCH(i,the_key) \
  ((the_key) == bookpos[i].key)

/*
 * See if the entry is empty
 */
#define DIGEST_EMPTY(i) is_empty(i)

/*
 * Check for collision
 */
#define DIGEST_COLLISION(i,key) \
  (!DIGEST_MATCH(i,key) && !DIGEST_EMPTY(i))

/*
 * The next macro is used in the case of hash collisions.
 * We use double hashing with the higher bits of the key.
 * In order to have the shift relatively prime to the hash
 * size, we OR by 1.
 */
#define DIGEST_NEXT(i,key) \
  ((i) = ((i) + (((key) >> digest_bits) | 1)) & DIGEST_MASK)


/* Mainly for debugging purposes */
static int bookhashcollisions = 0;

/*
 * This is considered to be the limit for the hash, I chose
 * 95% because it is Monday... No, I am open for suggestions
 * for the right value, I just don't know.
 */
#define DIGEST_LIMIT (0.95 * DIGEST_SIZE)

/*
 * This is the record as it will be written in the binary
 * file in network byte order. HashType is uint64_t. To
 * avoid endianness and padding issues, we do not read or
 * write structs but put the values in an unsigned char array.
 */

static unsigned char buf[2+2+2+8];

/* Offsets */
static const int wins_off   = 0;
static const int losses_off = 2;
static const int draws_off  = 4;
static const int key_off    = 6;

static void buf_to_book(void)
{
  HashType key;
  uint32_t i;

  key = ((uint64_t)buf[key_off] << 56)
    | ((uint64_t)buf[key_off+1] << 48)
    | ((uint64_t)buf[key_off+2] << 40)
    | ((uint64_t)buf[key_off+3] << 32)
    | ((uint64_t)buf[key_off+4] << 24)
    | ((uint64_t)buf[key_off+5] << 16)
    | ((uint64_t)buf[key_off+6] << 8)
    | ((uint64_t)buf[key_off+7]);
  /*
   * This is an infinite loop if the hash is 100% full,
   * but other parts should check that this does not happen.
   */
  for (DIGEST_START(i, key);
       DIGEST_COLLISION(i, key);
       DIGEST_NEXT(i, key))
    /* Skip */
    bookhashcollisions++;

  bookpos[i].wins   += (buf[wins_off]   << 8) | buf[wins_off  +1];
  bookpos[i].draws  += (buf[draws_off]  << 8) | buf[draws_off +1];
  bookpos[i].losses += (buf[losses_off] << 8) | buf[losses_off+1];
  bookpos[i].key = key;
}

static void book_to_buf(uint32_t index)
{
  int k;

  for (k=0; k<2; k++) {
    buf[wins_off + k]   = ((bookpos[index].wins)   >> ((1-k)*8)) & 0xff;
    buf[draws_off + k]  = ((bookpos[index].draws)  >> ((1-k)*8)) & 0xff;
    buf[losses_off + k] = ((bookpos[index].losses) >> ((1-k)*8)) & 0xff;
  }
  for (k=0; k<8; k++) {
    buf[key_off + k] = ((bookpos[index].key) >> ((7-k)*8)) & 0xff;
  }
}

static int compare(const void *aa, const void *bb)
{
  const leaf *a = aa;
  const leaf *b = bb;

  if (b->score > a->score) return(1);
  else if (b->score < a->score) return(-1);
  else return(0);
}

/*
 * Reads an existing binary book from f. The header must
 * already be skipped, when you call this function. The
 * variable digest_bits must be set to the correct value
 * before calling this function. If any book was allocated
 * before, it will be lost.
 */
static int read_book(FILE *f)
{
  if (book_allocated) {
    free(bookpos);
    book_allocated = 0;
  }
  bookpos = calloc(DIGEST_SIZE, sizeof(struct hashtype));
  if (bookpos == NULL) {
    return BOOK_ENOMEM;
  }
  book_allocated = 1;
  bookcnt = 0;
  bookhashcollisions = 0;
  while ( 1 == fread(&buf, sizeof(buf), 1, f) ) {
    buf_to_book();
    bookcnt++;
  }
  return BOOK_SUCCESS;
}

/*
 * Return values are defined in common.h
 */

int BookBuilderOpen(void)
{
  FILE *rfp, *wfp;
  int res;

  if ((rfp = fopen(BOOKRUN,"rb")) != NULL) {
    printf("Opened existing book!\n");
    if (!check_magic(rfp)) {
      fprintf(stderr,
	      "File %s does not conform to the current format.\n"
	      "Consider rebuilding your book.\n",
	      BOOKRUN);
      return BOOK_EFORMAT;
    }
    /*
     * We have to read the size header, but in book building we
     * use the maximum-sized hash table, so we discard the value.
     */
    digest_bits = MAX_DIGEST_BITS;
    read_size(rfp);
    res = read_book(rfp);
    fclose(rfp);
    if (res != BOOK_SUCCESS) {
      fclose(rfp);
      return res;
    }
    printf("Read %d book positions\n", bookcnt);
    printf("Got %d hash collisions\n", bookhashcollisions);
  } else {
    wfp = fopen(BOOKRUN,"w+b");
    if (wfp == NULL) {
      fprintf(stderr, "Could not create %s file: %s\n",
		 BOOKRUN, strerror(errno));
      return BOOK_EIO;
    }
    if (write_magic(wfp) != BOOK_SUCCESS) {
      fprintf(stderr, "Could not write to %s: %s\n",
	      BOOKRUN, strerror(errno));
      return BOOK_EIO;
    }
    if (fclose(wfp) != 0) {
      fprintf(stderr, "Could not write to %s: %s\n",
	      BOOKRUN, strerror(errno));
      return BOOK_EIO;
    }
    printf("Created new book %s!\n", BOOKRUN);
    rfp = fopen(BOOKRUN, "rb");
    if (rfp == NULL) {
      fprintf(stderr, "Could not open %s for reading: %s\n",
	      BOOKRUN, strerror(errno));
      return BOOK_EIO;
    }
    digest_bits = MAX_DIGEST_BITS;
    /* We use read_book() here only to allocate memory */
    if (read_book(wfp) == BOOK_ENOMEM) {
      return BOOK_ENOMEM;
    }
  }
  return BOOK_SUCCESS;
}

/*
 * Store the position and results of last search
 * if and only if in the first 10 moves of the game.
 * This routine is called after the computer makes its
 * most recent move. Lastly, after the 10th move, on
 * the 11th move, store the data out to the running file.
 */
/*
 * NOTE: Before you build a book, you have to call
 * BookBuilderOpen() now, after building it BookBuilderClose()
 * in order to actually write the book to disk.
 */

int BookBuilder(short result, uint8_t side)
{
  uint32_t i;

  /* Only first BOOKDEPTH moves */
  if (GameCnt > BOOKDEPTH)
    return BOOK_EMIDGAME;
  CalcHashKey();
  for (DIGEST_START(i, HashKey);
       ;
       DIGEST_NEXT(i, HashKey)) {
    if (DIGEST_MATCH(i, HashKey)) {
      existpos++;
      break;
    } else if (DIGEST_EMPTY(i)) {
      if (bookcnt > DIGEST_LIMIT)
	return BOOK_EFULL;
      bookpos[i].key = HashKey;
      newpos++;
      bookcnt++;
      break;
    } else {
      bookhashcollisions++;
    }
  }
  if (side == white) {
    if (result == R_WHITE_WINS)
      bookpos[i].wins++;
    else if (result == R_BLACK_WINS)
      bookpos[i].losses++;
    else if (result == R_DRAW)
      bookpos[i].draws++;
  } else {
    if (result == R_WHITE_WINS)
      bookpos[i].losses++;
    else if (result == R_BLACK_WINS)
      bookpos[i].wins++;
    else if (result == R_DRAW)
      bookpos[i].draws++;
  }
  return BOOK_SUCCESS;
}

int BookBuilderClose(void)
{
  /*
   * IMHO the following part needs no temporary file.
   * If two gnuchess invocations try to write the same
   * file at the same time, this goes wrong anyway.
   * Please correct me if I am wrong. If you generate
   * a temporary file, try to generate it in some writable
   * directory.
   */
  FILE *wfp;
  unsigned int i;
  int errcode = BOOK_SUCCESS;

  wfp = fopen(BOOKRUN, "wb");
  if (wfp == NULL) {
    errcode = BOOK_EIO;
    goto bailout_noclose;
  }
  if (write_magic(wfp) != BOOK_SUCCESS) {
    errcode = BOOK_EIO;
    goto bailout;
  }
  if (write_size(wfp, bookcnt) != BOOK_SUCCESS) {
    errcode = BOOK_EIO;
    goto bailout;
  }
  for (i = 0; i < DIGEST_SIZE; i++) {
    if (!is_empty(i)) {
      book_to_buf(i);
      if (1 != fwrite(&buf, sizeof(buf), 1, wfp)) {
	errcode = BOOK_EIO;
	goto bailout;
      }
    }
  }
  printf("Got %d hash collisions\n", bookhashcollisions);

 bailout:
  if (fclose(wfp) != 0) {
    errcode = BOOK_EIO;
  }

 bailout_noclose:
  free(bookpos);
  book_allocated = 0;

  /* Let BookQuery allocate again */
  bookloaded = 0;

  return errcode;
}

/*
 * Return values are defined in common.h
 * NOTE: This function now returns BOOK_SUCCESS on
 * success, which is 0. So all the possible callers
 * should adjust to this. (At present it is only called
 * in iterate.c.) It used to return 1 on success before.
 */

int BookQuery(int BKquery)
{
  /*
   * BKquery denotes format; 0 text, 1 engine
   * In general out put is engine compliant, lines start with a blank
   * and end with emtpy line
   */
  int i,j,k,icnt = 0, mcnt, found, tot, maxdistribution;
  int matches[MAXMATCH] ;
  leaf m[MAXMOVES];
  leaf pref[MAXMOVES];
  struct {
    uint16_t wins;
    uint16_t losses;
    uint16_t draws;
  } r[MAXMOVES];
  FILE *rfp = NULL;
  leaf *p;
  short side,xside,temp;
  uint32_t booksize;
  int res;

  if (bookloaded && !book_allocated) {
    /* Something failed during loading the book */
    return BOOK_ENOBOOK;
  }
  if (!bookloaded) {
    char const * const *booktry;

    bookloaded = 1;
    for (booktry = bookbin; *booktry ; booktry++) {
      if (!(flags & XBOARD)) {
	fprintf(ofp, "Looking for opening book in %s...\n", *booktry);
      }
      rfp = fopen(*booktry, "rb");
      /* XXX: Any further error analysis here?  -- Lukas */
      if (rfp == NULL) continue;
      if (!(flags & XBOARD))
	fprintf(ofp,"Read opening book (%s)...\n", *booktry);
      if (!check_magic(rfp)) {
	fprintf(stderr,
		" File %s does not conform to the current format.\n"
		" Consider rebuilding it.\n\n",
		*booktry);
	fclose(rfp);
	rfp = NULL;
      } else break; /* Success, stop search */
    }
    if (rfp == NULL) {
      /* If appropriate print error */
      if (!(flags & XBOARD) || BKquery == 1)
        fprintf(ofp," No book found.\n\n");
      return BOOK_ENOBOOK;
    }
    if (!(flags & XBOARD)) {
      fprintf(ofp, "Loading book from %s.\n", *booktry);
    }
    /*
     * The factor 1.06 is the minimum additional amount of
     * free space in the hash
     */
    booksize = read_size(rfp) * 1.06;
    for (digest_bits = 1; booksize; booksize >>= 1) {
      digest_bits++;
    }
    res = read_book(rfp);
    if (res != BOOK_SUCCESS) {
      return res;
    }
    if (!(flags & XBOARD))
      fprintf(ofp,"%d hash collisions... ", bookhashcollisions);
  }

  mcnt = -1;
  side = board.side;
  xside = 1^side;
  TreePtr[2] = TreePtr[1];
  GenMoves(1);
  FilterIllegalMoves (1);
  for (p = TreePtr[1]; p < TreePtr[2]; p++) {
    MakeMove(side, &p->move);
    m[icnt].move = p->move;
    posshash[icnt] = HashKey;
    icnt++;
    UnmakeMove(xside,&p->move);
  }
  for (i = 0; i < icnt; i++) {
    for (DIGEST_START(j,posshash[i]);
	 !DIGEST_EMPTY(j);
	 DIGEST_NEXT(j, posshash[i])) {
      if (DIGEST_MATCH(j, posshash[i])) {
	found = 0;
	for (k = 0; k < mcnt; k++)
	  if (matches[k] == i) {
	    found = 1;
	    break;
	  }
	/* Position must have at least some wins to be played by book */
	if (!found) {
	  matches[++mcnt] = i;
	  pref[mcnt].move = m[matches[mcnt]].move;
	  r[i].losses = bookpos[j].losses;
	  r[i].wins = bookpos[j].wins;
	  r[i].draws = bookpos[j].draws;

	  /* by percent score starting from this book position */

	  pref[mcnt].score = m[i].score =
	    100*(r[i].wins+(r[i].draws/2))/
	    (MAX(r[i].wins+r[i].losses+r[i].draws,1)) + r[i].wins/2;

	}
	if (mcnt >= MAXMATCH) {
	  fprintf(ofp," Too many matches in book.\n\n");
	  goto fini;
	}
	break; /* If we found a match, the following positions can not match */
      }
    }
  }

fini:
  if (!(flags & XBOARD) || BKquery == 1)
  {
    fprintf(ofp," Opening database: %d book positions.\n",bookcnt);
    if (mcnt+1 == 0) {
      fprintf(ofp," In this position, there is no book move.\n\n");
    } else if (mcnt+1 == 1) {
      fprintf(ofp," In this position, there is one book move:\n");
    } else {
      fprintf(ofp," In this position, there are %d book moves:\n", mcnt+1);
    }
  }
  /* No book moves */
  if (mcnt == -1) {
    return BOOK_ENOMOVES;
  }
  k = 0;
  if (mcnt+1) {
    if ( !(flags & XBOARD) || BKquery == 1 ) {
      for (i = 0; i <= mcnt; i++) {
	if (!(flags & XBOARD) || BKquery == 1 ) {
	  SANMove(m[matches[i]].move,1);
          tot = r[matches[i]].wins+r[matches[i]].draws+r[matches[i]].losses;
	  fprintf(ofp," %s(%2.0f/%d/%d/%d) ",SANmv,
		100.0*(r[matches[i]].wins+(r[matches[i]].draws/2.0))/tot,
		r[matches[i]].wins,
		r[matches[i]].losses,
		r[matches[i]].draws);
          if ((i+1) % 4 == 0) fputc('\n',ofp);
	}
      }
      if (!(flags & XBOARD) || BKquery == 1 )
        if (i % 4 != 0) fprintf(ofp," \n \n");
    }
    if (bookmode == BOOKRAND) {
      k = rand();
      k = k % (mcnt+1);
      RootPV = m[matches[k]].move;
      if (!(flags & XBOARD)) {
        printf("\n(Random picked move #%d %s%s from above list)\n",k,
	  algbr[FROMSQ(RootPV)],algbr[TOSQ(RootPV)]);
        tot = r[matches[k]].wins+r[matches[k]].draws+r[matches[k]].losses;
        if (tot != 0)
          printf("B p=%2.0f\n",
	   100.0*(r[matches[k]].wins+r[matches[k]].draws)/tot);
        else
          printf("p=NO EXPERIENCES\n");
      }
    } else if (bookmode == BOOKBEST) {
      qsort(&pref,mcnt+1,sizeof(leaf),compare);
      RootPV = pref[0].move;
    } else if (bookmode == BOOKWORST) {
      qsort(&pref,mcnt+1,sizeof(leaf),compare);
      RootPV = pref[mcnt].move;
    } else if (bookmode == BOOKPREFER) {
      qsort(&pref,mcnt+1,sizeof(leaf),compare);
      for (i = 0; i <= mcnt; i++) {
	if (!(flags & XBOARD) || BKquery == 1) {
	  SANMove(pref[i].move,1);
          printf(" %s(%d) ",SANmv,pref[i].score);
	}
	m[i].move = pref[i].move;
	if (!(flags & XBOARD) || BKquery == 1)
          if ((i+1) % 8 == 0) fputc('\n',ofp);
      }
      if (!(flags & XBOARD) || BKquery == 1)
	if (i % 8 != 0) fprintf(ofp," \n \n");
        temp = (bookfirstlast > mcnt+1 ? mcnt+1 : bookfirstlast);
      /* Choose from the top preferred moves based on distribution */
      maxdistribution = 0;
      for (i = 0; i < temp; i++)
        maxdistribution += pref[i].score;
      /* Do not play moves that have only losses! */
      if (maxdistribution == 0)
	return BOOK_ENOMOVES;
      k = rand() % maxdistribution;
      maxdistribution = 0;
      for (i = 0; i < temp; i++) {
	maxdistribution += pref[i].score;
	if (k >= maxdistribution - pref[i].score &&
	    k < maxdistribution)
	{
	  k = i;
	  RootPV = m[k].move;
	  break;
	}
      }
    }
  }
  return BOOK_SUCCESS;
}

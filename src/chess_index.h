#ifndef CHESS_INDEX_H
#define CHESS_INDEX_H

#include <stddef.h>
#include <stdio.h>

/* pg */
#include "postgres.h"
#include "fmgr.h"

#include "catalog/namespace.h"
#include "catalog/pg_type.h"
#include "parser/parse_type.h" 	//typenameTypeId
#include "utils/builtins.h"
#include "utils/array.h" 		//PG_RETURN_ARRAYTYPE_P(x)
#include "utils/lsyscache.h"

//#define EXTRA_DEBUG 1


/********************************************************
* 		defines
********************************************************/
///*{{{*/

#define CH_NOTICE(...) ereport(NOTICE, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__)))
#define CH_ERROR(...) ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__)))
#define CH_DEBUG5(...) ereport(DEBUG5, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__))) // most detail
#define CH_DEBUG4(...) ereport(DEBUG4, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__)))
#define CH_DEBUG3(...) ereport(DEBUG3, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__)))
#define CH_DEBUG2(...) ereport(DEBUG2, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__)))
#define CH_DEBUG1(...) ereport(DEBUG1, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__))) // least detail

#define CH_DEBUG_SQUARE(s)  CH_NOTICE("square:%c%c", CHAR_CFILE(s), CHAR_RANK(s));

#define BAD_TYPE_IN(type, input) ereport( \
        ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), \
            errmsg("invalid input syntax for %s: \"%s\"", type, input)))

#define BAD_TYPE_OUT(type, input) ereport( \
        ERROR, (errcode(ERRCODE_DATA_CORRUPTED), \
            errmsg("corrupt internal data for %s: \"%d\"", type, input)))

#define FEN_MAX 100
#define SQUARE_MAX 64

#define TO_RANK(s) ((s)/8)
#define TO_FILE(s) ((s)%8)
#define CHAR_CFILE(s) ('a' + TO_FILE(s))
#define CHAR_RANK(s) ('1' + TO_RANK(s))

#define SET_BIT8(i8, k)   ((i8)  |= ((bits8)1 << (k)))
#define SET_BIT16(i16, k) ((i16) |= ((uint16)1 << (k)))
#define SET_BIT32(i32, k) ((i32) |= ((uint32)1 << (k)))
#define SET_BIT64(i64, k) ((i64) |= ((uint64)1 << (k)))

#define CLEAR_BIT8(i8, k)   ((i8)  &= ~((bits8)1 << (k)))
#define CLEAR_BIT16(i16, k) ((i16) &= ~((uint16)1 << (k)))
#define CLEAR_BIT32(i32, k) ((i32) &= ~((uint32)1 << (k)))
#define CLEAR_BIT64(i64, k) ((i64) &= ~((uint64)1 << (k)))

#define GET_BIT8(i8, k)   (((i8)  >> (k)) & (bits8)1)
#define GET_BIT16(i16, k) (((i16) >> (k)) & (uint16)1)
#define GET_BIT32(i32, k) (((i32) >> (k)) & (uint32)1)
#define GET_BIT64(i64, k) (((i64) >> (k)) & (uint64)1)

// piecesquare type
/* +------+------+------+------+------+------+------+------+
 * | BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 |
 * +------+------+------+------+------+------+------+------+
 * |                     square              |   kind      |
 * |                                         |             |
 * +------+------+------+------+------+------+------+------+
 * +------+------+------+------+------+------+------+------+
 * | BIT8 | BIT9 | BIT10| BIT11| BIT12| BIT13| BIT14| BIT15|
 * +------+------+------+------+------+------+------+------+
 * |      piece/target         |           subject         |
 * |                           |                           |
 * +------+------+------+------+------+------+------+------+
 */
#define PS_PIECE    0
#define PS_ATTACKS  1
#define PS_DEFENDS  2
#define PS_XRAY     3

#define PS_SQUARE_MASK  63
#define PS_KIND_MASK    192
#define PS_TARGET_MASK  3840
#define PS_SUBJECT_MASK 61440

#define GET_PS_SQUARE(i16)      ((i16) & PS_SQUARE_MASK)
#define GET_PS_KIND(i16)        (((i16) & PS_KIND_MASK)>>6 )
#define GET_PS_PIECE(i16)       (((i16) & PS_TARGET_MASK)>>8 )
#define GET_PS_SUBJECT(i16)     (((i16) & PS_SUBJECT_MASK)>>12 )

#define INIT_PS(i16, p, s)      do {i16=0; ((i16) = ((p)<<8) | (s)); } while(0)
#define SET_PS_KIND(i16, k)     ((i16) = ((k)<<6) | (i16))
#define SET_PS_SUBJECT(i16, p)  ((i16) = ((p)<<12) | (i16) )

// board type
//
//XXX CHECK_BIT should start at SQUARE_MAX and decrement to be meaningful TO_SQUARE_IDX & TO_BB_IDX
#define CHECK_BIT(board, k) ((1ull << (k)) & board)
#define GET_PIECE(pieces, k) ((k)%2 ? pieces[(k)/2] & 0x0f : (pieces[(k)/2] & 0xf0) >> 4)
#define SET_PIECE(pieces, k, v) (pieces[(k)/2] = (k)%2 ? ( (pieces[(k)/2] & 0xF0) | ((v) & 0xF)) : ((pieces[(k)/2] & 0x0F) | ((v) & 0xF) << 4))

#define TO_SQUARE_IDX(i)  (((i)/8)*8 + (8 - (i)%8) - 1) // from a fen string
#define FROM_BB_IDX(i) ( 63 - ((i)/8)*8 - (7 - (i)%8))  // from cpiece board to square idx
#define TO_BB_IDX(i) (56 - (i/8)*8 + (i%8))             // from square idx to bb idx

#define PIECE_SIZE(k) ((k)/2 + ((k)%2))
#define BOARD_SIZE(k) (PIECE_SIZE(k) + sizeof(Board))
#define INIT_BOARD(b, k) do { \
        b = (Board*)palloc(BOARD_SIZE(k)); memset(b, 0, BOARD_SIZE(k)); memset(b->pieces,0,PIECE_SIZE(k)); SET_VARSIZE(b, BOARD_SIZE(k)); \
    } while(0)

#define DIR_N   8
#define DIR_S  -8
#define DIR_W  -1
#define DIR_E   1
#define DIR_NW  7
#define DIR_SE -7
#define DIR_NE  9
#define DIR_SW -9

/*}}}*/
/********************************************************
* 		types
********************************************************/
/*{{{*/

typedef                 uint64          bitboard_t; // for Board type storage
typedef                 unsigned char   pieces_t;   // nibbles of bytes for piece storage board
typedef                 unsigned char   board_t;    // for use with cpiece

typedef enum            {BLACK, WHITE} side_t;

typedef enum            {NO_CPIECE, WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
                                    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
                        CPIECE_MAX, CPIECE_MOBILITY} cpiece_t;
const cpiece_t          WHITE_PIECES[6];
const cpiece_t          BLACK_PIECES[6];

typedef enum            {NO_PIECE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_MAX} piece_t;
const piece_t           PIECE_INDEX_PIECES[5];
const int               PIECE_INDEX_COUNTS[5];

#define PIECE_INDEX_SUM 15
#define PIECE_INDEX_MAX 5


/*
 * base-size -> 16:   4 bytes length (required by pg) 
 *                  + 4 bytes required for 64 bit alignment (holds state) 
                    + 8 bytes bitboard
                    + 4 bit nibble * n pieces
 * for valid chess games:
 * min size ->  17: 2 pieces of 4 bit nibbles = 1 bytes
 * max size ->  32: 32 pieces of 4 bit nibbles = 16 bytes
 *
 */

typedef struct {
    int32                 vl_len;
    unsigned int          blacksgo : 1;     // zeroing memory inits as whites go
    unsigned int          pcount : 6;       // 63
    unsigned int          ep_present : 1;   // en passant
    unsigned int          ep_is_white : 1;
    unsigned int          ep_file: 3;       // we only need the file
    unsigned int          wk : 1;           // 1 if white king can castle
    unsigned int          wq : 1;           // "" white queen ""
    unsigned int          bk : 1;           // "" black king  ""
    unsigned int          bq : 1;           // "" black queen ""
    unsigned int          move : 9;         // max 511  
    unsigned int          last_capt: 7;     // max 127
    bitboard_t            board;            // occupancy bitmap
#ifdef EXTRA_DEBUG
	char			orig_fen[FEN_MAX];
#endif
    pieces_t        pieces[FLEXIBLE_ARRAY_MEMBER];
} Board;

#define PIECES_MAX 63
#define CH_MOVE_MAX 511
#define CH_LCAPT_MAX 127


/*}}}*/

// declarations
  
uint32          _sdbm_hash(char * str);
void            ch_itoa(int value, char* buf, int base);

uint16          _pindex_in(char * str);
char            _square_in(char file, char rank);
char            _adiagonal_in(char square);
char            _diagonal_in(char square);

int             _piece_value(const piece_t p);
char            _cpiece_char(const cpiece_t p);
int             _cpiece_value(const cpiece_t p) ;
cpiece_t        _cpiece_type(const piece_t p, bool iswhite);
cpiece_t        _cpiece_in(char c);
side_t          _cpiece_side(const cpiece_t p);

piece_t         _piece_type(const cpiece_t p);
piece_t         _piece_in(char c);
char            _piece_char(const piece_t p);

board_t *       _bitboard_to_board(const Board *);
bitboard_t      _board_to_bitboard(pieces_t * pieces, const board_t * board);
Board *         _init_board(Board * b, int psize);
void            _board_footer_in(Board * b, char * str);

void            debug_bitboard(const bitboard_t a);
void            debug_board(const board_t * b);
void            debug_bits(uint64 a, unsigned char bits);
char            _cfile_in(char f);

#endif


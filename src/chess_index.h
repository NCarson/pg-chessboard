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

#define CH_NOTICE(...) ereport(NOTICE, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__)))
#define CH_ERROR(...) ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__)))
#define CH_DEBUG5(...) ereport(DEBUG5, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__))) // most detail
#define CH_DEBUG4(...) ereport(DEBUG4, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__)))
#define CH_DEBUG3(...) ereport(DEBUG3, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__)))
#define CH_DEBUG2(...) ereport(DEBUG2, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__)))
#define CH_DEBUG1(...) ereport(DEBUG1, (errcode(ERRCODE_INTERNAL_ERROR), errmsg(__VA_ARGS__))) // least detail

#define BAD_TYPE_IN(type, input) ereport( \
        ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), \
            errmsg("invalid input syntax for %s: \"%s\"", type, input)))

#define BAD_TYPE_OUT(type, input) ereport( \
        ERROR, (errcode(ERRCODE_DATA_CORRUPTED), \
            errmsg("corrupt internal data for %s: \"%d\"", type, input)))/*}}}*/

#define FEN_MAX 100
#define PIECES_MAX 32
#define SQUARE_MAX 64

#define TO_RANK(s) ((s)/8)
#define TO_FILE(s) ((s)%8)
#define CHAR_CFILE(s) ('a' + TO_FILE(s))
#define CHAR_RANK(s) ('1' + TO_RANK(s))

#define SET_BIT16(i16, k) ((i16) |= ((uint16)1 << (k)))
#define SET_BIT32(i32, k) ((i32) |= ((uint32)1 << (k)))
#define SET_BIT64(i64, k) ((i64) |= ((uint64)1 << (k)))
#define CLEAR_BIT16(i16, k) ((i16) &= ~((uint16)1 << (k)))
#define CLEAR_BIT32(i32, k) ((i32) &= ~((uint32)1 << (k)))
#define CLEAR_BIT64(i64, k) ((i64) &= ~((uint64)1 << (k)))
#define GET_BIT16(i16, k) (((i16) >> (k)) & (uint16)1)
#define GET_BIT32(i32, k) (((i32) >> (k)) & (uint32)1)
#define GET_BIT64(i64, k) (((i64) >> (k)) & (uint64)1)

#define SET_PS(i16, p, s) ((i16) = ((s) | ((p) & 0xFF) <<8))
#define GET_PS_PIECE(i16) (((i16) & 0xff00)>>8)
#define GET_PS_SQUARE(i16) ((i16) & 0x00ff)

#define CHECK_BIT(board, k) ((1ull << (k)) & board)
#define GET_PIECE(pieces, k) ((k)%2 ? pieces[(k)/2] & 0x0f : (pieces[(k)/2] & 0xf0) >> 4)
#define SET_PIECE(pieces, k, v) (pieces[(k)/2] = (k)%2 ? ( (pieces[(k)/2] & 0xF0) | ((v) & 0xF)) : ((pieces[(k)/2] & 0x0F) | ((v) & 0xF) << 4))

#define TO_SQUARE_IDX(i)  (((i)/8)*8 + (8 - (i)%8) - 1) //from a fen string
#define FROM_BB_IDX(i) ( 63 - ((i)/8)*8 - (7 - (i)%8))  // from bb to square idx
#define TO_BB_IDX(i) (56 - (i/8)*8 + (i%8))             // from square idx to bb idx

#define PIECE_SIZE(k) ((k)/2 + ((k)%2))
#define BOARD_SIZE(k) (PIECE_SIZE(k) + sizeof(Board))
#define INIT_BOARD(b, k) do { \
        b = (Board*)palloc(BOARD_SIZE(k)); memset(b, 0, BOARD_SIZE(k)); memset(b->pieces,0,PIECE_SIZE(k)); SET_VARSIZE(b, BOARD_SIZE(k)); \
    } while(0)

typedef enum            {BLACK, WHITE} side_t;

typedef enum            {WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
                         BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING, CPIECE_MAX, NO_CPIECE} cpiece_t;
const cpiece_t          WHITE_PIECES[6];
const cpiece_t          BLACK_PIECES[6];

typedef enum            {NO_PIECE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_MAX} piece_t;
const piece_t           PIECE_INDEX_PIECES[5];
const int               PIECE_INDEX_COUNTS[5];

typedef                 uint64          bitboard_t; // for Board type storage
typedef                 unsigned char   pieces_t;   // nibbles of bytes for piece storage board
typedef                 unsigned char   board_t;    // for use with cpiece

#define PIECE_INDEX_SUM 15
#define PIECE_INDEX_MAX 5

/*
 * base-size -> 14: 8 byte bitboard + 4 byte struct length (required by pg) + 2 for state
 * min size ->  16: 3 pieces of 4 bit nibbles = 2 bytes
 * max size ->  30: 32 pieces of 4 bit nibbles = 16 bytes
 *
 * reality of alignment 16-32
 */
typedef struct {
    int32                 vl_len;
    unsigned int          whitesgo : 1;
    unsigned int          pcount : 6; //0-32
    int                   enpassant : 7; // this could be reduced to side and file - 4 bits
    unsigned int          wk : 1;
    unsigned int          wq : 1;
    unsigned int          bk : 1;
    unsigned int          bq : 1;
    unsigned int          _unused: 14;
    bitboard_t            board;
#ifdef EXTRA_DEBUG
	char			orig_fen[FEN_MAX];
#endif
    pieces_t        pieces[FLEXIBLE_ARRAY_MEMBER];
} Board;


uint32          _sdbm_hash(char * str);
uint16          _pindex_in(char * str);
char            _square_in(char file, char rank);
char            _piece_char(const piece_t p);
char            _cpiece_char(const cpiece_t p);
cpiece_t        _cpiece_in(char c);
piece_t         _piece_type(const cpiece_t p);

board_t *       _bitboard_to_board(board_t * board, const uint64 bboard, const pieces_t * pieces);
bitboard_t      _board_to_bitboard(pieces_t * pieces, const board_t * board);
Board *         _init_board(Board * b, int psize);
void            _board_footer_in(Board * b, const char * str);

#endif


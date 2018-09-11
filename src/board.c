
#include <stdlib.h>

#include "chess_index.h"

#include "utils/varbit.h"


/********************************************************
 **     board
 ********************************************************/

/* 
 * This is a really nice compact way to store fen without Huffman compresion:
 * https://codegolf.stackexchange.com/questions/19397/smallest-chess-board-compression
 * second answer : 192 bits
 *
 * This ended up not using the king and en passant tricks for code simplicity
 */

board_t * _bitboard_to_board(const Board * b);

/*-------------------------------------------------------
 -      function info
 -------------------------------------------------------*/
/*{{{*/
PG_FUNCTION_INFO_V1(board_in);
PG_FUNCTION_INFO_V1(board_out);

// ops
PG_FUNCTION_INFO_V1(board_cmp);
PG_FUNCTION_INFO_V1(board_eq);
PG_FUNCTION_INFO_V1(board_ne);
PG_FUNCTION_INFO_V1(board_lt);
PG_FUNCTION_INFO_V1(board_gt);
PG_FUNCTION_INFO_V1(board_le);
PG_FUNCTION_INFO_V1(board_ge);
PG_FUNCTION_INFO_V1(board_hash);

// functions
PG_FUNCTION_INFO_V1(footer);
PG_FUNCTION_INFO_V1(pcount);
PG_FUNCTION_INFO_V1(pcount_piece);
PG_FUNCTION_INFO_V1(pcount_cpiece);
PG_FUNCTION_INFO_V1(board_side);
PG_FUNCTION_INFO_V1(pieceindex);
PG_FUNCTION_INFO_V1(_pieces);
PG_FUNCTION_INFO_V1(_pieces_cpiece);
PG_FUNCTION_INFO_V1(_pieces_piece);
PG_FUNCTION_INFO_V1(_pieces_square);
PG_FUNCTION_INFO_V1(_pieces_squares);
PG_FUNCTION_INFO_V1(board_to_fen);
PG_FUNCTION_INFO_V1(board_remove_pieces);
PG_FUNCTION_INFO_V1(heatmap);
PG_FUNCTION_INFO_V1(_attacks);
PG_FUNCTION_INFO_V1(_mobility);
PG_FUNCTION_INFO_V1(score);
PG_FUNCTION_INFO_V1(bitboard);
PG_FUNCTION_INFO_V1(int_array);
PG_FUNCTION_INFO_V1(board_move);
PG_FUNCTION_INFO_V1(board_halfmove);
PG_FUNCTION_INFO_V1(board_fiftyclock);
PG_FUNCTION_INFO_V1(board_cpiece_max_rank);
PG_FUNCTION_INFO_V1(board_cpiece_min_rank);
PG_FUNCTION_INFO_V1(board_cfile_type);
PG_FUNCTION_INFO_V1(board_to_int);
PG_FUNCTION_INFO_V1(board_hamming);
PG_FUNCTION_INFO_V1(board_moveless);
PG_FUNCTION_INFO_V1(board_clr_enpassant);
PG_FUNCTION_INFO_V1(board_invert);
PG_FUNCTION_INFO_V1(piecesquares_board);
PG_FUNCTION_INFO_V1(board_ucimove);
PG_FUNCTION_INFO_V1(board_ucimoves);

static void _board_footer_in(Board * b, char * str);

/*}}}*/
/*-------------------------------------------------------
 -      defines
 -------------------------------------------------------*/
/*{{{*/
#define PIECE_SIZE(k) ((k)/2 + ((k)%2))
#define BOARD_SIZE(k) (PIECE_SIZE(k) + sizeof(Board))
#define CHANGED_RANK(s, ss) ((s)/8 != (ss)/8)
#define CHANGED_FILE(s, ss) ((s)%8 != (ss)%8)

#define INIT_BOARD(b, k) do { \
        b = (Board*)palloc(BOARD_SIZE(k)); memset(b, 0, BOARD_SIZE(k)); memset(b->pieces,0,PIECE_SIZE(k)); SET_VARSIZE(b, BOARD_SIZE(k)); \
    } while(0)

static const int        KNIGHT_DIRS[] = {6, 15, 17, 10, -6, -15, -17, -10};
static const int        ROOK_DIRS[] =   {DIR_N, DIR_S, DIR_E, DIR_W};
static const int        BISHOP_DIRS[] = {DIR_NW, DIR_SW, DIR_SE, DIR_NE};
static const int        QUEEN_DIRS[] =  {DIR_N, DIR_S, DIR_E, DIR_W, DIR_NW, DIR_SW, DIR_SE, DIR_NE};

static int _board_out(const Board * b, char * str);
static char _board_cpiece_min_rank(const board_t * , const char, const cpiece_t );

 /*}}}*/
/*-------------------------------------------------------
 -      debug funcs
 -------------------------------------------------------*/
/*{{{*/
void debug_bitboard(const bitboard_t b)
{
    char            str[100];
    int             j = 0;

    for (int i=0; i<SQUARE_MAX; i++) {
        if (i && !(i%8)) {
            str[j++] = '\0';
            CH_NOTICE("bboard: %s", str);
            j = 0;
        }
        if (CHECK_BIT(b, SQUARE_MAX-i-1))
            str[j++] = 'x';
        else
            str[j++] = '.';
    }
    str[j++] = '\0';
    CH_NOTICE("bboard: %s", str);
}

void debug_board(const board_t * b)
{

    char            str[100];
    int             j = 0;

    for (int i=0; i<SQUARE_MAX; i++) {
        if (i && !(i%8)) {
            CH_NOTICE("board:  %s", str);
            j = 0;
        }
        if (b[i] == NO_CPIECE)
            str[j++] = '.';
        else
            str[j++] = _cpiece_char(b[i]);
    }
    str[j++] = '\0';
    CH_NOTICE("board:  %s", str);
}

void debug_bits(uint64 a, unsigned char bits) {

    char            *str = (char *) palloc(bits+1);
    int             cnt=bits-1;
    uint64           b = a;

    str += (bits- 1);
	while (cnt >=0) {
          str[cnt] = (b & 1) + '0';
          b >>= 1;
	     cnt--;
	}
	str[bits] = '\0';
	CH_NOTICE("bits: int:%ld: bits:|%s|", a, str);
}/*}}}*/
/*-------------------------------------------------------
 -      static
 -------------------------------------------------------*/
/*{{{*/

static bitboard_t 
_set_pieces(const board_t * board, pieces_t * pieces)
{

    bitboard_t          bboard=0;

    for (int i=0, k=0; i<SQUARE_MAX; i++) {
        if (board[i] != NO_CPIECE) {
            SET_BIT64(bboard, i);
            SET_PIECE(pieces, k, board[i]);
            k++;
        }
        if (k > PIECES_MAX)
            CH_ERROR("_board_to_bitboard: internal error: too many pieces :%i", k);
    }
    return bboard;
}

static inline void
_copy_board(const Board * a, Board * b)
{
    memcpy(b, a, BOARD_SIZE(a->pcount));
    for (int i=0; i<a->pcount; i++) {
        SET_PIECE(b->pieces, i, GET_PIECE(a->pieces, i));
    }
}


//XXX needs to be pfreed
board_t *
_bitboard_to_board(const Board * b)
{
	unsigned char		k=0;
    board_t             *board=(board_t *) palloc(SQUARE_MAX);

    for (int i=SQUARE_MAX; i>0; i--) {
        if (CHECK_BIT(b->board, i-1)) {
            board[SQUARE_MAX-i] = GET_PIECE(b->pieces, k);
            k++;
        } else {
            board[SQUARE_MAX-i] = NO_CPIECE;
        }
        if (k > PIECES_MAX)
            CH_ERROR("_bitboard_to_board: internal error: too many pieces :%i", k);
    }
    return board;
}

static bitboard_t
_board_to_bits_piece(const Board * b, const cpiece_t piece)
{
    board_t             *board = _bitboard_to_board(b);
    bitboard_t          bboard=0;

    for (int i=0; i<SQUARE_MAX; i++)
    {
        if (board[i]==piece)
            SET_BIT64(bboard, SQUARE_MAX-1-i);
    }
    pfree(board);
    return bboard;
}

//XXX merge with below
static bitboard_t
_board_to_bits(const Board * b)
{
    board_t             *board = _bitboard_to_board(b);
    bitboard_t          bboard=0;

    for (int i=0; i<SQUARE_MAX; i++)
    {
        if (board[i])
            SET_BIT64(bboard, SQUARE_MAX-1-i);
    }
    pfree(board);
    return bboard;
}

/*
static bitboard_t
_boardt_to_bits(const board_t * b)
{
    bitboard_t          bboard=0;

    for (int i=0; i<SQUARE_MAX; i++)
    {
        if (b[i])
            SET_BIT64(bboard, SQUARE_MAX-1-i);
    }
    return bboard;
}
*/

Datum
board_to_int(PG_FUNCTION_ARGS)
{
    const Board         *b = (Board *)PG_GETARG_POINTER(0);
    const cpiece_t      piece = PG_GETARG_CHAR(1);
    
    PG_RETURN_INT64(_board_to_bits_piece(b, piece));
}

Datum
board_hamming(PG_FUNCTION_ARGS)
{
    const Board         *a = (Board *)PG_GETARG_POINTER(0);
    const Board         *b = (Board *)PG_GETARG_POINTER(1);
    
    PG_RETURN_INT32(hamming_uint64(_board_to_bits(a), _board_to_bits(b)));

}

static char *_board_pieceindex(const Board * b, side_t go)/*{{{*/
{

    char                *result=palloc(PIECE_INDEX_SUM+1); // 15 pieces without K
    unsigned int        i;
    unsigned char       j=0, k, l, n;
    cpiece_t            subject, target;
    const cpiece_t       *pieces = go==WHITE ? WHITE_PIECES : BLACK_PIECES;

    for (i=0; i<PIECE_INDEX_MAX; i++) {
        n = 0;
        target = pieces[i];

        for (k=0; k<b->pcount; k++) {
            subject = GET_PIECE(b->pieces, k);
            if (subject == target) {
                n++;
                if (n >= PIECE_INDEX_COUNTS[i])
                    break;
            }
        }
        for (l=0; l<PIECE_INDEX_COUNTS[i]; l++) {
            if (l<n) {
                result[j++] = _piece_char(_piece_type(target));
            } else {
                result[j++] = _piece_char(NO_PIECE);
            }
        }
    }
    result[PIECE_INDEX_SUM] = '\0';
    return result;
}/*}}}*/

//XXX this a pretty slow way to do this
static uint16 *_board_pieces(const Board * b)/*{{{*/
{

    uint16 				*result = (uint16 *)palloc(b->pcount * sizeof(uint16));
	unsigned char		k=0, p;
    board_t             *board = _bitboard_to_board(b);
    uint16              j;

    memset(result, 0, b->pcount * sizeof(uint16));

    //debug_bitboard(b->board);
    //debug_board(board);
    if (b->pcount <=0)
        return result;

    for (int i=0; i<SQUARE_MAX; i++) {
        if (board[i] != NO_CPIECE) {
            p = board[i];
            INIT_PS(j, p, FROM_BB_IDX(i));
            result[k] = j;
            if (k > b->pcount) CH_ERROR("_board_pieces: internal error: too many pieces");
            k++;
        }
    }
    pfree(board);
    return result;
}/*}}}*/

//TODO en passant
static int _board_attacks(const Board * b, int * heatmap, uint16 * piecesquares, bool mobility)
{
    unsigned char       s, p, k=0, count=0, dcount=0, subject, target;
    uint16              ps;
    int                 sign;
    size_t              n=0;
    const int           *dirs=0;
    board_t             *board = _bitboard_to_board(b);

    //debug_bitboard(b->board);
    //debug_board(board);

    for (int i=0; i<SQUARE_MAX; i++)
    {
        if (!board[i]) 
            continue;

        s = FROM_BB_IDX(i);
        subject = board[i];
        p = _piece_type(subject);
        sign = _cpiece_side(subject)==WHITE ? 1 : -1;
        switch (p)
        {
            case PAWN:
                dirs = BISHOP_DIRS;
                count = 1;
                dcount = 4;
                break;
            case KNIGHT:
                dirs = KNIGHT_DIRS;
                count = 1;
                dcount = 8;
                break;
            case KING:
                dirs = QUEEN_DIRS;
                count = 1;
                dcount = 8;
                break;

            //sliders
            case QUEEN:
                dirs = QUEEN_DIRS;
                count = 255;
                dcount = 8;
                break;
            case ROOK:
                dirs = ROOK_DIRS;
                count = 255;
                dcount = 4;
                break;
            case BISHOP:
                dirs = BISHOP_DIRS;
                count = 255;
                dcount = 4;
                break;

            default:
                CH_ERROR("internal error: unknown cpiece type %d", p);
                break;
        }
        k++;
        for (int i=0, blockers, d, ss, cc; i<dcount; i++) {
            blockers=0;
            ss = s;
            cc = count;
            d = dirs[i];
            while(cc > 0) {
                ss += d;
                if (ss < 0 || ss >= SQUARE_MAX) // off board
                    break;
                if ((d==DIR_W || d==DIR_E) && CHANGED_RANK(s, ss))
                    break;
                if ((d==DIR_N || d==DIR_S) && CHANGED_FILE(s, ss))
                    break;
                // we could find diagonals with rotated bitboards instead of functions
                // https://chessprogramming.wikispaces.com/Flipping%20Mirroring%20and%20Rotating
                if ((d==DIR_NE || d==DIR_SW) && _adiagonal_in(s) !=_adiagonal_in(ss))
                    break;
                if ((d==DIR_NW || d==DIR_SE) && _diagonal_in(s) !=_diagonal_in(ss))
                    break;
                if (p==PAWN && sign > 0 && d < 0)
                    break;
                if (p==PAWN && sign < 0 && d > 0)
                    break;

                cc--;
                heatmap[ss] += sign;
                target = board[TO_BB_IDX(ss)];

                // if mobility (only)
                if (mobility) {
                    if (p==PAWN)
                        continue;
                    INIT_PS(ps, subject, ss);
                    SET_PS_SUBJECT(ps, CPIECE_MOBILITY);
                    piecesquares[n++] = ps;

                // else attacks (only)
                } else if (target){

                    //CH_NOTICE("target: %d", target);
                    //CH_NOTICE("%c%c%c > %c%c%c", 
                    //      _cpiece_char(subject), CHAR_CFILE(s), CHAR_RANK(s), 
                    //      _cpiece_char(target), CHAR_CFILE(ss), CHAR_RANK(ss));
                    INIT_PS(ps, target, ss);
                    SET_PS_SUBJECT(ps, subject);

                    //xray does not diff attack/defend
                    if (blockers==1)
                        SET_PS_KIND(ps, PS_XRAY);
                    else if (_cpiece_side(target) == _cpiece_side(subject))
                        SET_PS_KIND(ps, PS_DEFENDS);
                    else
                        SET_PS_KIND(ps, PS_ATTACKS);
                    piecesquares[n++] = ps;
                    if (blockers>1)
                        break;
                }

                if (target)
                    blockers++;
            }
        }
    }
    return n;
}


/*}}}*/
/*-------------------------------------------------------
 -      operators
 -------------------------------------------------------*/
/*{{{*/

static int _board_compare(const Board *a, const Board *b) 
{
    if (a->pcount > b->pcount)
        return 1;
    else if (a->pcount < b->pcount)
        return -1;
    else
        return (memcmp(a, b, BOARD_SIZE(a->pcount)));
}

Datum
board_cmp(PG_FUNCTION_ARGS)
{
    const Board     *a = (Board *) PG_GETARG_POINTER(0);
    const Board     *b = (Board *) PG_GETARG_POINTER(1);
    PG_RETURN_INT32(_board_compare(a, b));
}

Datum
board_eq(PG_FUNCTION_ARGS)
{
    const Board     *a = (Board *) PG_GETARG_POINTER(0);
    const Board     *b = (Board *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(_board_compare(a, b) == 0);
}

Datum
board_ne(PG_FUNCTION_ARGS)
{
    const Board     *a = (Board *) PG_GETARG_POINTER(0);
    const Board     *b = (Board *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(_board_compare(a, b) != 0);
}

Datum
board_lt(PG_FUNCTION_ARGS)
{
    const Board     *a = (Board *) PG_GETARG_POINTER(0);
    const Board     *b = (Board *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(_board_compare(a, b) < 0);
}

Datum
board_gt(PG_FUNCTION_ARGS)
{
    const Board     *a = (Board *) PG_GETARG_POINTER(0);
    const Board     *b = (Board *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(_board_compare(a, b) > 0);
}

Datum
board_le(PG_FUNCTION_ARGS)
{
    const Board     *a = (Board *) PG_GETARG_POINTER(0);
    const Board     *b = (Board *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(_board_compare(a, b) <= 0);
}

Datum
board_ge(PG_FUNCTION_ARGS)
{
    const Board     *a = (Board *) PG_GETARG_POINTER(0);
    const Board     *b = (Board *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(_board_compare(a, b) >= 0);
}

Datum
board_hash(PG_FUNCTION_ARGS)
{

	const Board     *b = (Board *) PG_GETARG_POINTER(0);
    char            *str = palloc(FEN_MAX);
    
    _board_out(b, str);

	PG_RETURN_INT64(_sdbm_hash(str));
}

/*}}}*/
/*-------------------------------------------------------
 -      constructors
 -------------------------------------------------------*/
/*{{{*/
Datum
board_out(PG_FUNCTION_ARGS)
{

    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    char            *str = palloc(FEN_MAX);
    
    _board_out(b, str);
    PG_RETURN_CSTRING(str);
}

static int 

_board_out(const Board * b, char * str)
{
    int             i;
    unsigned char   j=0, k=0, empties=0, s, piece;
    char            move[5];

#ifdef EXTRA_DEBUG
    debug_bitboard(b->board);
    CH_DEBUG5("orig fen: %s", b->orig_fen);
    CH_DEBUG5("piece count: %i", b->pcount);
#endif
    for (i=SQUARE_MAX-1; i>=0; i--) {
        if (j >= FEN_MAX) {
#ifdef EXTRA_DEBUG
            CH_ERROR("_board_out: internal error: fen is too long; original fen:'%s'", b->orig_fen);
#else 
            CH_ERROR("_board_out: internal error: fen is too long");
#endif
        }
        if (k > b->pcount) {
#ifdef EXTRA_DEBUG
            CH_ERROR("_board_out: internal error: too many pieces; original fen:'%s'", b->orig_fen);
#else 
            CH_ERROR("_board_out: internal error: too many pieces");
#endif
        }
        // need for square type
        s = TO_SQUARE_IDX(i);

        // if its an empty square
        if (!CHECK_BIT(b->board, i)) {
            empties += 1;
            // else there is apiece
        } else {
            // if we have some empties then empty them
            if (empties) 
                str[j++] = empties + '0';

            empties = 0;
            piece = GET_PIECE(b->pieces, k);
            k++;

            switch(piece) {
                case BLACK_PAWN:    str[j++] = 'p'; break;
                case BLACK_KNIGHT:  str[j++] = 'n'; break;
                case BLACK_BISHOP:  str[j++] = 'b'; break;
                case BLACK_ROOK:    str[j++] = 'r'; break;
                case BLACK_QUEEN:   str[j++] = 'q'; break;
                case BLACK_KING:    str[j++] = 'k'; break;
                case WHITE_PAWN:    str[j++] = 'P'; break;
                case WHITE_KNIGHT:  str[j++] = 'N'; break;
                case WHITE_BISHOP:  str[j++] = 'B'; break;
                case WHITE_ROOK:    str[j++] = 'R'; break;
                case WHITE_QUEEN:   str[j++] = 'Q'; break;
                case WHITE_KING:    str[j++] = 'K'; break;
                default:
                                    // should not get here :)
                                    CH_ERROR("internal error: unknown piece type at piece %i in board: %i", k, piece);
                                    break;
            }
            CH_DEBUG5("_board_out: i:%i :square: %c%c piece:%c",i, CHAR_CFILE(s), CHAR_RANK(s), str[j-1]);
        }
        // if were at the end or a row add '/' and perhaps and empty number
        if (i%8==0) { 
            if (empties) 
                str[j++] = empties + '0';
            if (i) 
                str[j++] = '/';
            empties = 0;
        }
    }
    str[j++] = ' ';
    str[j++] = b->blacksgo ? 'b' : 'w';

    str[j++] = ' ';
    if (b->wk + b->wq + b->bk + b->bq > 0) {
        if (b->wk) str[j++] = 'K';
        if (b->wq) str[j++] = 'Q';
        if (b->bk) str[j++] = 'k';
        if (b->bq) str[j++] = 'q';
    } else
        str[j++] = '-';

    str[j++] = ' ';
    if (b->ep_present) {
        str[j++] = CHAR_CFILE(b->ep_file);
        if (b->ep_is_white) {
            str[j++] = '3';
        } else {
            str[j++] = '6';
        }
    } else
        str[j++] = '-';


    if (!b->move) { // if move is 0 it was never set
        str[j++] = '\0';
        return j;
    }

    str[j++] = ' ';
    i=0;
    ch_itoa(b->last_capt, move, 10);
    while(move[i]) {
        str[j++] = move[i++];
    }
    i=0;
    str[j++] = ' ';
    ch_itoa(b->move, move, 10);
    while(move[i]) {
        str[j++] = move[i++];
    }
    str[j++] = '\0';
    return j;
}


Datum
board_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);
    Board           *result=0;
    pieces_t        pieces[PIECES_MAX];
    unsigned char   c, p='\0';
    int64           bitboard=0;
    int             i=0, j=SQUARE_MAX-1, k=0, s;
    bool            done=false;

    if (strlen(str) > FEN_MAX)
        CH_ERROR("fen string too long");
    memset(pieces, 0, PIECES_MAX);

    i = 0;
    while (str[i] != '\0') {

        // i indexes differently than square type (for enpassant)
        s = TO_SQUARE_IDX(j);

        switch (c=str[i]) {
                case 'p': case 'n': case 'b': case 'r': case 'q': case 'k':
                case 'P': case 'N': case 'B': case 'R': case 'Q': case 'K':

                    CH_DEBUG5("_fen_in: :square: %c%c: str[i]:%c", CHAR_CFILE(s), CHAR_RANK(s), str[i]);
                    SET_BIT64(bitboard, j);
                    j--;
                    //if (s==enpassant && str[i] != 'p' && str[i] != 'P') {
                    //    CH_ERROR("no pawn found for enpassant at %c%c", CHAR_CFILE(s), CHAR_RANK(s));
                    //}
                    switch(str[i]) {
                        case 'p': p = BLACK_PAWN; break;
                        case 'n': p = BLACK_KNIGHT; break;
                        case 'b': p = BLACK_BISHOP; break;
                        case 'r': p = BLACK_ROOK; break;
                        case 'q': p = BLACK_QUEEN; break;
                        case 'k': p = BLACK_KING; break;

                        case 'P': p = WHITE_PAWN; break;
                        case 'N': p = WHITE_KNIGHT; break;
                        case 'B': p = WHITE_BISHOP; break;
                        case 'R': p = WHITE_ROOK; break;
                        case 'Q': p = WHITE_QUEEN; break;
                        case 'K': p = WHITE_KING; break;
                    }
                    SET_PIECE(pieces, k, p);
                    k++;
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                          j -= (c - '0');
                case '/':
                          break;
                case ' ':
                          done=true;
                          break;
                default:
                          CH_ERROR("unkown character in fen '%c'", c);
                          break;
        }
        if (k>PIECES_MAX)
            CH_ERROR("too many pieces in fen");
        i++;
        if (done) 
            break;
        if (j<-1)
            CH_ERROR("FEN board is too long");
    }

    if (j>-1)
        CH_ERROR("FEN board is too short");


#ifdef EXTRA_DEBUG
	debug_bitboard(bitboard);
	strcpy(result->orig_fen, str);
#endif

    INIT_BOARD(result, k);
    if (str[i])
        _board_footer_in(result, &str[i]);
    result->board = bitboard;
    result->pcount = k;
    memcpy(result->pieces, pieces, PIECE_SIZE(k));

    PG_RETURN_POINTER(result);
}

static
void _board_footer_in(Board * b, char * str)
{
    char        c, rank=0, ep_file=0;
    int         i=0;
    bool        ep_is_white=false, ep_present=false;
    long        move=0;
    char        *ptr;

    switch (str[i++]) {
        case 'w': b->blacksgo=0; break;
        case 'b': b->blacksgo=1; break;
        default: CH_ERROR("bad move side in fen: '%c'", str[i-1]); break;
    }
    i++;
    while (str[i] != '\0' && str[i] != ' ') {
        switch (str[i++]) {
            case 'K': b->wk=1; break;
            case 'Q': b->wq=1; break;
            case 'k': b->bk=1; break;
            case 'q': b->bq=1; break;
            case '-': break;
            default: CH_ERROR("bad castle availability in fen: '%c'", str[i-1]); break;
        }
    }
    c = str[++i];
    if (c >= 'a' && c <= 'h') {
        rank = str[++i];
        ep_present=true;
        if (rank== '3') 
            ep_is_white=true;
        else if (rank== '6') 
            ep_is_white=false;
        else 
            CH_ERROR("bad en passant rank in fen: '%c'", c);
        ep_file = _cfile_in(c);
    } else if (c=='-') {
    } else {
        CH_ERROR("bad en passant square in fen: '%c%c':", c, rank);
    }
    b->ep_present = ep_present;
    b->ep_file = ep_file;
    b->ep_is_white = ep_is_white;

    if (!str[++i])
        return;

    if (str[i] != ' ')
        CH_ERROR("bad en passant in fen footer '%s' at char '%c'", str, str[i]);

    ptr = &str[++i];

    if (!ptr)
        return;
    if (!isdigit(*ptr)) 
        CH_ERROR("bad move number in fen footer '%s' at char '%c'", str, *ptr);

    move = strtol(ptr, &ptr, 10);
    if (move > CH_LCAPT_MAX)
        CH_ERROR("halfmove clock number '%ld' is greater than the maximum of %d:", move, CH_LCAPT_MAX);
    if (move < 0)
        CH_ERROR("halfmove clock number '%ld' is negative:", move);
    b->last_capt = move;
	ptr++;

    if (!ptr)
        return;
    if (!isdigit(*ptr))
        CH_ERROR("bad halfmove clock number in fen footer '%s' at char '%c'", str, *ptr);

    move = strtol(ptr, &ptr, 10);
    if (move > CH_MOVE_MAX)
        CH_ERROR("move number '%ld' is greater than maximum of %d:", move, CH_MOVE_MAX);
    if (move < 0)
        CH_ERROR("move number '%ld' is negative:", move);
    b->move = move;

    if (*(ptr))
        CH_ERROR("extra characters after last field in fen footer: '%s'", str);
}/*}}}*/
/*-------------------------------------------------------
 -      functions
 -------------------------------------------------------*/
/*{{{*/

Datum
board_move(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    PG_RETURN_INT32(b->move);
}

Datum
board_halfmove(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    if (!b->move)
        PG_RETURN_INT32(0);
    else
        PG_RETURN_INT32((b->move*2-1) + b->blacksgo);
}

Datum
board_fiftyclock(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    PG_RETURN_INT32(b->last_capt);
}

Datum
pcount(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    PG_RETURN_INT16(b->pcount);
}

Datum
pcount_piece(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    const piece_t   piece = PG_GETARG_CHAR(1);
    int             result=0;
    uint16	        *pieces= _board_pieces(b);
    uint16          ps;

	for (int i = 0; i < b->pcount; i++) { 
        ps = pieces[i];
        if (_piece_type(GET_PS_PIECE(ps)) == piece) {
            result++;
        }
	}
    pfree(pieces);
    PG_RETURN_INT16(result);
}

Datum
pcount_cpiece(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    cpiece_t        piece = PG_GETARG_CHAR(1);
    int             result=0;
    uint16	        *pieces= _board_pieces(b);
    uint16          ps;

	for (int i = 0; i < b->pcount; i++) { 
        ps = pieces[i];
        if (GET_PS_PIECE(ps) == piece) {
            result++;
        }
	}
    pfree(pieces);
    PG_RETURN_INT16(result);
}

Datum
board_side(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    PG_RETURN_CHAR(b->blacksgo ? BLACK: WHITE);
}

Datum
pieceindex(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    const side_t    go = PG_GETARG_CHAR(1);

    PG_RETURN_INT16(_pindex_in(_board_pieceindex(b, go)));
}

Datum
_pieces(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    uint16			*pieces= _board_pieces(b);
    ArrayType       *a;
	Datum           *d = (Datum *) palloc(sizeof(Datum) * b->pcount);
    side_t          side = -1;
    size_t          n=0;
    uint16          ps;

    if (PG_NARGS() > 1)
        side = PG_GETARG_CHAR(1);

	for (int i = 0; i<b->pcount; i++) { 
        ps = pieces[i];
        if (side == -1) {
            d[n++] = UInt16GetDatum(ps);
        } else if (side == WHITE || side == BLACK) {
            if (_cpiece_side(GET_PS_PIECE(ps)) == side) {
                d[n++] = UInt16GetDatum(ps);
            }
        } else {
            CH_ERROR("internal error with side_t");
        }
	}
	a = construct_array(d, n, INT2OID, sizeof(uint16), true, 'c');

    PG_RETURN_ARRAYTYPE_P(a);
}

Datum
_pieces_cpiece(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    uint16			*pieces= _board_pieces(b);
    ArrayType       *a;
	Datum           *d = (Datum *) palloc(sizeof(Datum) * b->pcount);
    size_t          n=0;
    uint16          ps;
    cpiece_t        piece = PG_GETARG_CHAR(1);

	for (int i = 0; i < b->pcount; i++) { 
        ps = pieces[i];
        if (GET_PS_PIECE(ps) == piece) {
            d[n++] = UInt16GetDatum(ps);
        }
	}
	a = construct_array(d, n, INT2OID, sizeof(uint16), true, 'c');

    PG_RETURN_ARRAYTYPE_P(a);
}

Datum
_pieces_piece(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    uint16			*pieces= _board_pieces(b);
    ArrayType       *a;
	Datum           *d = (Datum *) palloc(sizeof(Datum) * b->pcount);
    size_t          n=0;
    uint16          ps;
    piece_t        piece = PG_GETARG_CHAR(1);

	for (int i = 0; i < b->pcount; i++) { 
        ps = pieces[i];
        if (_piece_type(GET_PS_PIECE(ps)) == piece) {
            d[n++] = UInt16GetDatum(ps);
        }
	}
	a = construct_array(d, n, INT2OID, sizeof(uint16), true, 'c');

    PG_RETURN_ARRAYTYPE_P(a);
}

Datum
_pieces_square(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    uint16			*pieces= _board_pieces(b);
    uint16          ps;
    char            square = PG_GETARG_CHAR(1);
    uint16          result = NO_CPIECE;

	for (int i = 0; i < b->pcount; i++) { 
        ps = pieces[i];
        if (GET_PS_SQUARE(ps) == square) {
            result = UInt16GetDatum(ps);
            break;
        }
	}
    if (result == NO_CPIECE)
        PG_RETURN_NULL();
    else
        PG_RETURN_UINT16(result);

}


Datum 
_pieces_squares(PG_FUNCTION_ARGS)
{
    //https://github.com/pjungwir/aggs_for_arrays/blob/master/array_to_max.c
	ArrayType 			*vals;              // Our arguments:
	Oid 				valsType;           // The array element type:
	int16 				valsTypeWidth;      // The array element type widths for our input array:
	bool 				valsTypeByValue;    // The array element type "is passed by value" flags (not really used):
	char 				valsTypeAlignmentCode; // The array element type alignment codes (not really used):
	Datum 				*valsContent;       // The array contents, as PostgreSQL "Datum" objects:
	bool 				*valsNullFlags;     // List of "is null" flags for the array contents:
	int 				valsLength;         // The size of the input array:

    ArrayType           *a;
    const Board         *b = (Board *) PG_GETARG_POINTER(0);
	Datum               *d = (Datum *) palloc(sizeof(Datum) * b->pcount);
    uint16			    *pieces= _board_pieces(b);
    size_t               n=0;
    uint16               ps;
    char                 square;

	if (PG_ARGISNULL(0)) { ereport(ERROR, (errmsg("Null arrays not accepted"))); } 
	vals = PG_GETARG_ARRAYTYPE_P(1);
	if (ARR_NDIM(vals) == 0) { PG_RETURN_NULL(); }
	if (ARR_NDIM(vals) > 1) { ereport(ERROR, (errmsg("One-dimesional arrays are required"))); }

	// Determine the array element types.
	valsType = ARR_ELEMTYPE(vals);
	valsLength = (ARR_DIMS(vals))[0];

	get_typlenbyvalalign(valsType, &valsTypeWidth, &valsTypeByValue, &valsTypeAlignmentCode);
	// Extract the array contents (as Datum objects).
	deconstruct_array(vals, valsType, valsTypeWidth, valsTypeByValue, valsTypeAlignmentCode, &valsContent, &valsNullFlags, &valsLength);

	for (int i = 0; i < valsLength; i++) {
		if (valsNullFlags[i]) continue;
        square = DatumGetChar(valsContent[i]);

        for (int j = 0; j < b->pcount; j++) { 
            ps = pieces[j];
            if (GET_PS_SQUARE(ps) == square) {
                d[n++] = UInt16GetDatum(ps);
            }
        }
    }
	a = construct_array(d, n, INT2OID, sizeof(uint16), true, 'c');

    PG_RETURN_ARRAYTYPE_P(a);

}

Datum
footer(PG_FUNCTION_ARGS)
{

    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    char            *str = palloc(FEN_MAX);
    int             i;
    
    _board_out(b, str);
    for (i=0; i<FEN_MAX; i++)
        if (str[i] == ' ')
            break;

    PG_RETURN_CSTRING(&str[i+1]);
}

Datum
board_moveless(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    Board           *result;

    INIT_BOARD(result, b->pcount);
    _copy_board(b, result);
    result->move = 0;
    result->last_capt = 0;
    PG_RETURN_POINTER(result);
}

Datum
board_clr_enpassant(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    Board           *result;

    INIT_BOARD(result, b->pcount);
    _copy_board(b, result);
    result->ep_present= 0;
    result->ep_is_white= 0;
    result->ep_file= 0;
    PG_RETURN_POINTER(result);
}

Datum
board_remove_pieces(PG_FUNCTION_ARGS)
{

    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    Board           *result;
    bool            *pfilter = (bool *) PG_GETARG_POINTER(1);
    board_t         *board = _bitboard_to_board(b);
    unsigned char   wp, bp, n=0;

    for (int i=PAWN; i<PIECE_MAX; i++) {
        if (pfilter[i-1]) {
            wp = _cpiece_type(i, true);
            bp = _cpiece_type(i, false);

            for (int j=0; j<SQUARE_MAX; j++) {
                if (board[j] == wp || board[j] == bp) {
                    board[j] = NO_CPIECE;
                    n++;
                }
            }
        }
    }
    pfree(board);

    INIT_BOARD(result, b->pcount - n);
    result->board = _set_pieces(board, result->pieces);
    result->blacksgo = b->blacksgo;
    result->pcount = b->pcount - n;
    result->ep_present = b->ep_present;
    result->ep_file = b->ep_file;
    result->ep_is_white = b->ep_is_white;
    result->wk = b->wk;
    result->wq = b->wq;
    result->bk = b->bk;
    result->bq = b->bq;
    result->move = b->move;
    result->last_capt = b->last_capt;
    PG_RETURN_POINTER(result);
}

Datum
board_invert(PG_FUNCTION_ARGS)
{

    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    Board           *result;
    board_t         *old = _bitboard_to_board(b);
    board_t         *new = palloc0(SQUARE_MAX);
    cpiece_t         p;

    INIT_BOARD(result, b->pcount);
    _copy_board(b, result);

    for (int i=0; i<SQUARE_MAX; i++) {
        if (!old[i]) continue;
        if (_cpiece_side(old[i])==WHITE)
            p = old[i] + 6;
        else
            p = old[i] - 6;
        //XXX board is in square idx ???(!no) so we dont
        // need to anything to invert
        // as _set_pieces will do the work for us
        new[i] = p;
    }

    result->board = _set_pieces(new, result->pieces);
    result->blacksgo = b->blacksgo ? 0 : 1;
    result->ep_is_white = b->ep_is_white ? 0 : 1;
    pfree(old);
    pfree(new);
    PG_RETURN_POINTER(result);
}

//XXX this does not set enpassant
//FIXME handle promotion
Datum
board_ucimove(PG_FUNCTION_ARGS)
{
    const uci_t          move  = PG_GETARG_UINT16(0);
    const Board         *b = (Board *)PG_GETARG_POINTER(1);
    size_t               pcount = b->pcount;
    int                  to = FROM_BB_IDX(GET_UCI_TO(move));
    int                  from = FROM_BB_IDX(GET_UCI_FROM(move));
    board_t             *old = _bitboard_to_board(b);
    Board               *result;

    if (old[to] && old[from]) {
        pcount--;
    }
    //XXX should moving empty squares raise an error?
    if (!old[to] && !old[from]) {
        pcount--;
    }
    INIT_BOARD(result, pcount);

    result->pcount = pcount;
    old[to] = old[from];
    old[from] = NO_CPIECE;

    //result->board = _set_pieces(old, result->pieces);
    //FIXME make this the main _set_pieces
    result->board=0;
    for (int i=0, k=0; i<SQUARE_MAX; i++) {
        if (old[i] != NO_CPIECE) {
            SET_BIT64(result->board, TO_SQUARE_IDX(FROM_BB_IDX(i)));
            SET_PIECE(result->pieces, k, old[i]);
            k++;
        }
    }

    result->blacksgo = b->blacksgo ? 0 : 1;
    result->move = b->move + (b->blacksgo ? 1 : 0);

    pfree(old);
    PG_RETURN_POINTER(result);
    
}

static Board *
_arr_board_ucimoves(PG_FUNCTION_ARGS, const Board *b, const Datum * d, bool * nulls, const size_t len)
{

    uci_t                from, to;
    board_t             *old = _bitboard_to_board(b);
    Board               *result;

    INIT_BOARD(result, b->pcount);
    result->pcount = b->pcount;

    for (int i=0; i<len; i++){
       if (nulls[i])
           continue;

        from = FROM_BB_IDX(GET_UCI_FROM(d[i]));
        to = FROM_BB_IDX(GET_UCI_TO(d[i]));

        if (old[to] && old[from]) {
            result->pcount--;
        }
        if (!old[to] && !old[from]) {
            result->pcount--;
        }

        old[to] = old[from];
        old[from] = NO_CPIECE;

        //result->blacksgo = b->blacksgo ? 0 : 1;
        //result->move = b->move + (b->blacksgo ? 1 : 0);
   }

    //FIXME make this the main _set_pieces
    result->board=0;
    for (int i=0, k=0; i<SQUARE_MAX; i++) {
        if (old[i] != NO_CPIECE) {
            SET_BIT64(result->board, TO_SQUARE_IDX(FROM_BB_IDX(i)));
            SET_PIECE(result->pieces, k, old[i]);
            k++;
        }
    }

    pfree(old);
    return result;
}

Datum
board_ucimoves(PG_FUNCTION_ARGS)
{
	Datum 				*d=0;
	bool 				*nulls=0;
    int                  len;
    Board               *result;
    const Board         *b = (Board *) PG_GETARG_POINTER(1);

    len = _get_array_arg(PG_FUNCTION_ARGS_CALL, 0, &d, &nulls);
    result = _arr_board_ucimoves(PG_FUNCTION_ARGS_CALL, b, d, nulls, len);

    PG_RETURN_POINTER(result);

}

Datum
heatmap(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    int32           heatmap[SQUARE_MAX];
    uint16          piecesquares[PIECE_MAX*PIECE_MAX];
    char            *str = palloc(SQUARE_MAX + 8 + 1);
    int             j=0;

    memset(heatmap, 0, sizeof(int32) * SQUARE_MAX);
    memset(piecesquares, 0, sizeof(uint16) * PIECE_MAX * PIECE_MAX);
    _board_attacks(b, heatmap, piecesquares, true);

    for (int i=0, v; i<SQUARE_MAX; i++)
    {
       if (i && !(i%8))
           str[j++] = '\n';
        v = heatmap[TO_BB_IDX(i)];
        str[j++] = v >= 0 ? (v > 0 ? '+' : '.') : '-';
    }
    str[j] = '\0';

    PG_RETURN_CSTRING(str);
}

Datum
_attacks(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    int32           heatmap[SQUARE_MAX];
    uint16          pieces[PIECE_MAX*PIECE_MAX];
    int             n = _board_attacks(b, heatmap, pieces, false);

    ArrayType       *a;
	Datum *d 		= (Datum *) palloc(sizeof(Datum) * n);

	for (int i = 0; i<n; i++) { 
		d[i] = UInt16GetDatum(pieces[i]);
	}
	a = construct_array(d, n, INT2OID, sizeof(uint16), true, 's');

    PG_RETURN_ARRAYTYPE_P(a);
}

Datum
_mobility(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    int32           heatmap[SQUARE_MAX];
    uint16          pieces[PIECE_MAX*PIECE_MAX];
    int             n = _board_attacks(b, heatmap, pieces, true);

    ArrayType       *a;
	Datum *d 		= (Datum *) palloc(sizeof(Datum) * n);

	for (int i = 0; i<n; i++) { 
		d[i] = UInt16GetDatum(pieces[i]);
	}
	a = construct_array(d, n, INT2OID, sizeof(uint16), true, 's');

    PG_RETURN_ARRAYTYPE_P(a);
}

Datum
score(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    int             result=0;

	for (int i = 0; i < b->pcount; i++) { 
        result += _cpiece_value(GET_PIECE(b->pieces, i));
	}

    PG_RETURN_INT32(result);
}

Datum
bitboard(PG_FUNCTION_ARGS)
{
    const Board         *b = (Board *) PG_GETARG_POINTER(0);
    const cpiece_t      piece = PG_GETARG_INT16(1);

    /*
    typedef struct
    {
        int32       vl_len_;        // varlena header (do not touch directly!) /
        int32       bit_len;        // number of valid bits /
        bits8       bit_dat[FLEXIBLE_ARRAY_MEMBER];  bit string, most sig. byte * first /
    } VarBit;
    */
    // bits8 is always going to be 1 byte?
    size_t          bsize = sizeof(bits8);
    VarBit          *result = (VarBit *) palloc(VARBITTOTALLEN(SQUARE_MAX)); //VARHDRSZ + sizeof(int32) + SQUARE_MAX/bsize);
    
    bits8           *r = result->bit_dat;
    bits8           x = HIGHBIT;

    memset(result, 0, VARBITTOTALLEN(SQUARE_MAX));
	for (int i=SQUARE_MAX-1, k=0; i>=0; i--)
	{
		if (CHECK_BIT(b->board, i)) 
        {
            if (GET_PIECE(b->pieces, k)==piece)
			*r |= x;
            k++;
        }

		x >>= 1;
		if (x == 0)
		{
			x = HIGHBIT;
			r++;
		}
	}

    SET_VARSIZE(result, VARHDRSZ + sizeof(int32) + SQUARE_MAX/bsize);
    result->bit_len = SQUARE_MAX;

    PG_RETURN_VARBIT_P(result);
}

Datum
int_array(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    ArrayType       *a;
	Datum           *d = (Datum *) palloc(sizeof(Datum) * SQUARE_MAX);
    cpiece_t        piece;

	for (int i=0, k=0; i<SQUARE_MAX; i++)
	{
		if (CHECK_BIT(b->board, i)) 
        {
            piece = GET_PIECE(b->pieces, k);
            k++;
            d[i] = Int32GetDatum(piece);
        }
        else
            d[i] = 0;
	}

	a = construct_array(d, SQUARE_MAX, INT4OID, sizeof(int32), true, 'i');
    pfree(d);
    PG_RETURN_ARRAYTYPE_P(a);
}


Datum
board_cpiece_max_rank(PG_FUNCTION_ARGS)
{
    const Board         *b = (Board *)PG_GETARG_POINTER(0);
    const char          file = PG_GETARG_CHAR(1);
    const cpiece_t      piece = PG_GETARG_CHAR(2);
    
    board_t             *board = _bitboard_to_board(b);
    char                result=-1;
    

    if (_cpiece_side(piece)== WHITE) {
        for (int i=7, j=0+file; i>=0; i--)  {
            if ((board[j]) == piece) {
                result = i;
                break;
            }
            j+=8;
        }
    } else {
        for (int i=0, j=56+file; i<8; i++)  {
            if ((board[j]) == piece) {
                result = i;
                break;
            }
            j-=8;
        }
    }
    pfree(board);
    if (result != -1)
        PG_RETURN_CHAR(result);
    else
        PG_RETURN_NULL();
}

char
_board_cpiece_min_rank(const board_t * board, const char file, const cpiece_t piece)
{
    
    char                result=-1;

    if (_cpiece_side(piece)== WHITE) {
        for (int i=0, j=56+file; i<8; i++)  {
            if ((board[j]) == piece) {
                result = i;
                break;
            }
            j-=8;
        }
    } else {
        for (int i=7, j=0+file; i>=0; i--)  {
            if ((board[j]) == piece) {
                result = i;
                break;
            }
            j+=8;
        }
    }
    return result;
}

Datum
board_cpiece_min_rank(PG_FUNCTION_ARGS)
{
    const Board         *b = (Board *)PG_GETARG_POINTER(0);
    const char          file = PG_GETARG_CHAR(1);
    const cpiece_t      piece = PG_GETARG_CHAR(2);
    
    board_t             *board = _bitboard_to_board(b);
    char                result;
    
    result = _board_cpiece_min_rank(board, file, piece);
    pfree(board);
    if (result==-1)
        PG_RETURN_NULL();
    else
        PG_RETURN_CHAR(result);

}

Datum
board_cfile_type(PG_FUNCTION_ARGS)
{
    const Board         *b = (Board *)PG_GETARG_POINTER(0);
    const char          file = PG_GETARG_CHAR(1);
    
    board_t             *board = _bitboard_to_board(b);
    char                wr, br;
    
    wr = _board_cpiece_min_rank(board, file, WHITE_PAWN);
    br = _board_cpiece_min_rank(board, file, BLACK_PAWN);
    pfree(board);
    if (wr == -1 && br == -1)     //open
        PG_RETURN_TEXT_P(cstring_to_text("o"));
    else if(wr == -1 && br != -1) // half-open white
        PG_RETURN_TEXT_P(cstring_to_text("w"));
    else if(wr != -1 && br == -1) // half-open black
        PG_RETURN_TEXT_P(cstring_to_text("b"));
    else if(wr != -1 && br != -1) // closed
        PG_RETURN_TEXT_P(cstring_to_text("c"));
    else
        CH_ERROR("internal error in board_cfile_type");
}

static Board *_piecesquares_board(char *footer, const Datum * pieces, const int size, const bool * nulls)
{

    uint16              ps;
    cpiece_t            p;
    char                s;
    Board               *b; 
    int                 count=0;

    INIT_BOARD(b, size);
    b->pcount = size;
    if (footer)
        _board_footer_in(b, footer);

    if (size > PIECES_MAX)
        CH_ERROR("_piecesquares_to_board: internal error: too many pieces :%i", size);

    // we have to sort the pieces in bitboard order
	for (int i=0, kk=0; i < SQUARE_MAX; i++) {
        count = 0;
        // then check each piece to see if its on the square
        for (int k=0; k < size; k++) {
            if (nulls[k]) {
                continue;
            } 
            ps = DatumGetUInt16(pieces[k]);
            s = GET_PS_SQUARE(ps);
            if (s < 0 || s >= SQUARE_MAX) CH_ERROR("_piecesquares_to_board: internal error: invalid square: %i", s);
            if (TO_BB_IDX(s) == i) {
                count++;
                if (count > 1)
                    CH_ERROR("duplicate piece on square %i", s);
                p = GET_PS_PIECE(ps);
                if (p < 0 || p >= CPIECE_MAX) CH_ERROR("_piecesquares_to_board: internal error: invalid piece:%i", p);
                SET_BIT64(b->board, TO_SQUARE_IDX(s));
                SET_PIECE(b->pieces, kk, p);
                kk++;
            }
        }
    }
    return b;
}

Datum 
piecesquares_board(PG_FUNCTION_ARGS)
{

	Datum 				*pieces=0;
	bool 				*pieces_nulls=0;
    int                 len_pieces;
    Board               *board;
    char                *str=0;

    if (PG_NARGS() == 2)
        str = text_to_cstring(PG_GETARG_TEXT_P(1));

    len_pieces = _get_array_arg(PG_FUNCTION_ARGS_CALL, 0, &pieces, &pieces_nulls);
    board = _piecesquares_board(str, pieces, len_pieces, pieces_nulls);

    if (str) pfree(str);

    PG_RETURN_POINTER(board);
}


/*}}}*/


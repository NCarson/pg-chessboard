
#include "chess_index.h"

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
PG_FUNCTION_INFO_V1(side);
PG_FUNCTION_INFO_V1(pieceindex);
PG_FUNCTION_INFO_V1(_pieces);
PG_FUNCTION_INFO_V1(board_to_fen);
PG_FUNCTION_INFO_V1(remove_pieces);
PG_FUNCTION_INFO_V1(heatmap);
PG_FUNCTION_INFO_V1(_attacks);
PG_FUNCTION_INFO_V1(_mobility);

#define SIZEOF_PIECES(k) ((k)/2 + (k)%2)
#define SIZEOF_BOARD(k) (sizeof(Board) + SIZEOF_PIECES(k))
#define CHANGED_RANK(s, ss) ((s)/8 != (ss)/8)
#define CHANGED_FILE(s, ss) ((s)%8 != (ss)%8)

static const int        KNIGHT_DIRS[] = {6, 15, 17, 10, -6, -15, -17, -10};
static const int        ROOK_DIRS[] =   {DIR_N, DIR_S, DIR_E, DIR_W};
static const int        BISHOP_DIRS[] = {DIR_NW, DIR_SW, DIR_SE, DIR_NE};
static const int        QUEEN_DIRS[] =  {DIR_N, DIR_S, DIR_E, DIR_W, DIR_NW, DIR_SW, DIR_SE, DIR_NE};
 
/*-------------------------------------------------------
 -      static
 -------------------------------------------------------*/
/*{{{*/
static int _board_out(const Board * b, char * str)/*{{{*/
{
    int             i;
    unsigned char   j=0, k=0, empties=0, s, piece;

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
    str[j++] = b->whitesgo ? 'w' : 'b';

    str[j++] = ' ';
    if (b->wk + b->wq + b->bk + b->bq > 0) {
        if (b->wk) str[j++] = 'K';
        if (b->wq) str[j++] = 'Q';
        if (b->bk) str[j++] = 'k';
        if (b->bq) str[j++] = 'q';
    } else
        str[j++] = '-';

    str[j++] = ' ';
    CH_DEBUG5("enp: %i", b->enpassant);
    if (b->enpassant > -1) {
        str[j++] = CHAR_CFILE(b->enpassant);
        str[j++] = CHAR_RANK(b->enpassant);
    } else
        str[j++] = '-';

    str[j++] = '\0';
    return j;
}
/*}}}*/

static char *_board_pieceindex(const Board * b, side_t go)/*{{{*/
{

    char                *result=palloc(PIECE_INDEX_SUM+1); // 15 pieces without K
    unsigned int        i;
    unsigned char       j=0, k, l, n;
    cpiece_t            subject, target;
    const cpiece_t       *pieces = go==WHITE ? WHITE_PIECES : BLACK_PIECES;

    if (b->pcount <=0)
        CH_ERROR("board has no pieces");

    for (i=0; i<PIECE_INDEX_MAX; i++) {
        n = 0;
        target = pieces[i];
        //CH_NOTICE("target: %c", _piece_out(target));

        for (k=0; k<b->pcount; k++) {
            subject = GET_PIECE(b->pieces, k);
            if (subject == target) {
                n++;
                if (n >= PIECE_INDEX_COUNTS[i])
                    break;
            }
        }
        //CH_NOTICE("n: %i", n);
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
        CH_ERROR("board has no pieces");

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
        //CH_NOTICE("subject: %d", subject);
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
            //CH_NOTICE("dir:%d", d);
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


Datum
board_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);
    Board           *result;
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
                          CH_ERROR("unkdown character in fen '%c'", c);
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
    _board_footer_in(result, &str[i]);
    result->board = bitboard;
    result->pcount = k;
    memcpy(result->pieces, pieces, PIECE_SIZE(k));

    PG_RETURN_POINTER(result);
}

/*}}}*/
/*-------------------------------------------------------
 -      functions
 -------------------------------------------------------*/
/*{{{*/

Datum
pcount(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING(b->pcount);
}

Datum
side(PG_FUNCTION_ARGS)
{
    const Board     *b = (Board *) PG_GETARG_POINTER(0);
    PG_RETURN_CHAR(b->whitesgo ? WHITE : BLACK);
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
	Datum *d 		= (Datum *) palloc(sizeof(Datum) * b->pcount);

	for (int i = 0; i<b->pcount; i++) { 
		d[i] = UInt16GetDatum(pieces[i]);
        //CH_NOTICE("i:%i, piecesquare: %i", i,d[i]);
	}
	a = construct_array(d, b->pcount, INT2OID, sizeof(uint16), true, 'c');

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
remove_pieces(PG_FUNCTION_ARGS)
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

    if (b->pcount - n <= 0)
        PG_RETURN_NULL();

    INIT_BOARD(result, b->pcount - n);
    result->board = _board_to_bitboard(result->pieces, board);
    result->whitesgo = b->whitesgo;
    result->pcount = b->pcount - n;
    result->enpassant = b->enpassant;
    result->wk = b->wk;
    result->wq = b->wq;
    result->bk = b->bk;
    result->bq = b->bq;
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
        //CH_NOTICE("i:%i, piecesquare: %i", i,d[i]);
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
        //CH_NOTICE("i:%i, piecesquare: %i", i,d[i]);
	}
	a = construct_array(d, n, INT2OID, sizeof(uint16), true, 's');

    PG_RETURN_ARRAYTYPE_P(a);
}

/*}}}*/



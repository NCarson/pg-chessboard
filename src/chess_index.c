
#include "chess_index.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

const cpiece_t          WHITE_PIECES[] = {WHITE_QUEEN, WHITE_ROOK, WHITE_BISHOP, WHITE_KNIGHT, WHITE_PAWN};
const cpiece_t          BLACK_PIECES[] = {BLACK_QUEEN, BLACK_ROOK, BLACK_BISHOP, BLACK_KNIGHT, BLACK_PAWN};

const piece_t           PIECE_INDEX_PIECES[] = {QUEEN, ROOK, BISHOP, KNIGHT, PAWN};
const int               PIECE_INDEX_COUNTS[] = {1, 2, 2, 2, 8};

//FIXME get rid of this
#define MAKE_SQUARE(file, rank, str) {str[0]=file; str[1]=rank;}

PG_FUNCTION_INFO_V1(char_to_int);

/********************************************************
* 		util
********************************************************/
/*{{{*/
Datum
char_to_int(PG_FUNCTION_ARGS)
{
    char			c = PG_GETARG_CHAR(0);
    PG_RETURN_INT32((int32)c);
}

//http://www.cse.yorku.ca/~oz/hash.html
unsigned int _sdbm_hash(char * str)
{
	unsigned long long hash = 0;
	int c;

	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash;
	return hash;
}

unsigned short _pindex_in(char * str)
{
    char            check[] = "QRRBBNNPPPPPPPP";
    unsigned short  result=0;
    unsigned char   i;

    //CH_NOTICE("_pindex_in: %s, %i", str, strlen(str));

    if (strlen(str) != PIECE_INDEX_SUM)
        BAD_TYPE_IN("pindex", str);

    for (i=0; i<=PIECE_INDEX_SUM; i++)
        if (!(str[i] == check[i] || str[i] == '.'))
            BAD_TYPE_IN("pindex", str);

    for (i=0; i<=PIECE_INDEX_SUM; i++)
        if (str[i] != '.')
            SET_BIT16(result, PIECE_INDEX_SUM -1 - i);

    //CH_NOTICE("val out: %i, size:%ui", result, sizeof(result));
    //
    return result;
}/*}}}*/

char _square_in(char file, char rank)
{
    char			c=0;
    char            square[] = "..";

    if (file < 'a' || file > 'h') {
        MAKE_SQUARE(file, rank, square)
        BAD_TYPE_IN("square", square);
    }
    if (rank < '1' ||  rank > '8') {
        MAKE_SQUARE(file, rank, square)
        BAD_TYPE_IN("square", square);
    }

    c = (file - 'a') + ( 8 * (rank - '1'));

    CH_DEBUG5("_square_in: file:%c rank:%c char:%i", file, rank, c);
#ifdef EXTRA_DEBUG
    CH_DEBUG5("_square_in char of file rank: %c%c:", CHAR_CFILE(c), CHAR_RANK(c));
    CH_DEBUG5("c file:%i c rank:%i", TO_FILE(c), TO_RANK(c));
    CH_DEBUG5("file - 'a':%i rank-'1':%i", file -'a', rank-'1');
#endif

    if (c < 0 || c > 63) {
        MAKE_SQUARE(file, rank, square);
        CH_ERROR("bad conversion for square %s" ,square);
    }
    return c;
}
/********************************************************
* 		piece
********************************************************/
/*{{{*/
piece_t _piece_type(const cpiece_t p) 
{
    piece_t          result;

    switch (p) {
        case WHITE_PAWN:    result=PAWN; break;
        case WHITE_KNIGHT:  result=KNIGHT; break;
        case WHITE_BISHOP:  result=BISHOP; break;
        case WHITE_ROOK:    result=ROOK; break;
        case WHITE_QUEEN:   result=QUEEN; break;
        case WHITE_KING:    result=KING; break;
        case BLACK_PAWN:    result=PAWN; break;
        case BLACK_KNIGHT:  result=KNIGHT; break;
        case BLACK_BISHOP:  result=BISHOP; break;
        case BLACK_ROOK:    result=ROOK; break;
        case BLACK_QUEEN:   result=QUEEN; break;
        case BLACK_KING:    result=KING; break;
        default:
            CH_ERROR("bad cpiece_t: %i", p); break;
    }
    return result;
}

cpiece_t _cpiece_type(const piece_t p, bool iswhite) 
{
    piece_t          result;

    if (iswhite) {
        switch (p) {
            case PAWN:    result=WHITE_PAWN; break;
            case KNIGHT:  result=WHITE_KNIGHT; break;
            case BISHOP:  result=WHITE_BISHOP; break;
            case ROOK:    result=WHITE_ROOK; break;
            case QUEEN:   result=WHITE_QUEEN; break;
            case KING:    result=WHITE_KING; break;
            default:
                CH_ERROR("bad piece_t: %i", p); break;
        }
    } else {
        switch (p) {
            case PAWN:    result=BLACK_PAWN; break;
            case KNIGHT:  result=BLACK_KNIGHT; break;
            case BISHOP:  result=BLACK_BISHOP; break;
            case ROOK:    result=BLACK_ROOK; break;
            case QUEEN:   result=BLACK_QUEEN; break;
            case KING:    result=BLACK_KING; break;
            default:
                CH_ERROR("bad piece_t: %i", p); break;
        }
    }
    return result;
}

char _cpiece_char(const cpiece_t p) 
{
    char                result;
    switch (p) {
        case WHITE_PAWN:    result='P'; break;
        case WHITE_KNIGHT:  result='N'; break;
        case WHITE_BISHOP:  result='B'; break;
        case WHITE_ROOK:    result='R'; break;
        case WHITE_QUEEN:   result='Q'; break;
        case WHITE_KING:    result='K'; break;
        case BLACK_PAWN:    result='p'; break;
        case BLACK_KNIGHT:  result='n'; break;
        case BLACK_BISHOP:  result='b'; break;
        case BLACK_ROOK:    result='r'; break;
        case BLACK_QUEEN:   result='q'; break;
        case BLACK_KING:    result='k'; break;
        default:
            CH_ERROR("bad cpiece_t: %i", p); break;
    }
    return result;
}

cpiece_t _cpiece_in(char c)
{
    cpiece_t            result;
    char                piece[] = ".";

    switch(c) {
        case 'P': result=WHITE_PAWN; break;
        case 'R': result=WHITE_ROOK; break;
        case 'N': result=WHITE_KNIGHT; break;
        case 'B': result=WHITE_BISHOP; break;
        case 'Q': result=WHITE_QUEEN; break;
        case 'K': result=WHITE_KING; break;
        case 'p': result=BLACK_PAWN; break;
        case 'r': result=BLACK_ROOK; break;
        case 'n': result=BLACK_KNIGHT; break;
        case 'b': result=BLACK_BISHOP; break;
        case 'q': result=BLACK_QUEEN; break;
        case 'k': result=BLACK_KING; break;
		default:
            piece[0] = c;
			BAD_TYPE_IN("cpiece", piece);
			break;
    }
    return result;
}

piece_t    _piece_in(char c)
{
    piece_t             result;
    char                piece[] = ".";

    switch(c) {
        case 'P': result=PAWN; break;
        case 'R': result=ROOK; break;
        case 'N': result=KNIGHT; break;
        case 'B': result=BISHOP; break;
        case 'Q': result=QUEEN; break;
        case 'K': result=KING; break;
        case 'p': result=PAWN; break;
        case 'r': result=ROOK; break;
        case 'n': result=KNIGHT; break;
        case 'b': result=BISHOP; break;
        case 'q': result=QUEEN; break;
        case 'k': result=KING; break;
		default:
            piece[0] = c;
			BAD_TYPE_IN("piece", piece);
			break;
    }
    return result;
}

char _piece_char(const piece_t p) 
{
    char        result;
    switch (p) {
        case NO_PIECE:  result='.'; break;
        case PAWN:      result='P'; break;
        case KNIGHT:    result='N'; break;
        case BISHOP:    result='B'; break;
        case ROOK:      result='R'; break;
        case QUEEN:     result='Q'; break;
        case KING:      result='K'; break;
        default:
            CH_ERROR("bad piece_t: %i", p); break;
    }
    return result;
}
/*}}}*/
/********************************************************
* 		board
********************************************************/
/*{{{*/
void _board_footer_in(Board * b, const char * str)
{
    char        c, p=0, enpassant=-1;
    int         i = 0;

    switch (str[i++]) {
        case 'w': b->whitesgo=1; break;
        case 'b': b->whitesgo=0; break;
        default: CH_ERROR("bad move side in fen"); break;
    }
    i++;
    while (str[i] != '\0' && str[i] != ' ') {
        switch (str[i++]) {
            case 'K': b->wk=1; break;
            case 'Q': b->wq=1; break;
            case 'k': b->bk=1; break;
            case 'q': b->bq=1; break;
            case '-': break;
            default: CH_ERROR("bad castle availability in fen"); break;
        }
    }
    c = str[++i];
    if (c >= 'a' && c <= 'h') {
        p = str[++i];
        if (p != '3' && p != '6')
            CH_ERROR("bad enpassant rank in fen '%c'", p);
        enpassant = _square_in(c, p);

        CH_DEBUG5("_fen_in: enpessant: %c%c:", CHAR_CFILE(enpassant), CHAR_RANK(enpassant));

        // enpassant is set on the capture not on the pawn
        // so we have have to go up a rank to find the pawn
        // enpassant += (p=='3' ? 8 : -8);

    } else if (c=='-') {
    } else {
        CH_DEBUG5("fen:'%s':", str);
        CH_ERROR("bad enpassant in fen: '%c%i':", c, p);
    }
    b->enpassant = enpassant;
}
//XXX needs to be pfreed
board_t *_bitboard_to_board(const Board * b)
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

bitboard_t _board_to_bitboard(pieces_t * pieces, const board_t * board)
{

    bitboard_t          bboard=0;

    for (int i=0, k=0; i<SQUARE_MAX; i++) {
        if (board[i] != NO_CPIECE) {
            SET_BIT64(bboard, i);
            SET_PIECE(pieces, k, board[i]);
            k++;
        }
        if (k> PIECES_MAX)
            CH_ERROR("_board_to_bitboard: internal error: too many pieces :%i", k);
    }
    return bboard;
}
/*}}}*/
/********************************************************
* 		debug
********************************************************/
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

#ifdef EXTRA_DEBUG


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
	CH_DEBUG5("bitboard: %ld: |%s|", a, str);
}

 
#endif

/*}}}*/


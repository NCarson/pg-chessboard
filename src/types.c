/* Copyright Nate Carson 2018 */

#include "chess_index.h"

/*
   chess=# select to_hex(idx), idx::bit(4)  from generate_series(0, 15) as idx;
   to_hex | idx  
   --------+------
   0	  | 0000
   1	  | 0001
   2	  | 0010
   3	  | 0011
   4	  | 0100
   5	  | 0101
   6	  | 0110
   7	  | 0111
   8	  | 1000
   9	  | 1001
   a	  | 1010
   b	  | 1011
   c	  | 1100
   d	  | 1101
   e	  | 1110
   f	  | 1111
   (16 rows)


square numbers

 * 8    56      57      58      59      60      61      62      63
 * 7    48      49      50      51      52      53      54      55
 * 6    40      41      42      43      44      45      46      47
 * 5    32      33      34      35      36      37      38      39 
 * 4    24      25      26      27      28      29      30      31
 * 3    16      17      18      19      20      21      22      23
 * 2    08      09      10      11      12      13      14      15
 * 1    00      01      02      03      04      05      06      07
 *
 *      a       b       c       d       e       f       g       h   
 */

board_t * _bitboard_to_board(const Board * b);

/********************************************************
 * 		defines
 ********************************************************/

#define MAKE_SQUARE(file, rank, str) {str[0]=file; str[1]=rank;}

// function info
///*{{{*/

PG_FUNCTION_INFO_V1(pindex_in);
PG_FUNCTION_INFO_V1(pindex_out);
PG_FUNCTION_INFO_V1(pindex_to_int32);

PG_FUNCTION_INFO_V1(side_in);
PG_FUNCTION_INFO_V1(side_out);
PG_FUNCTION_INFO_V1(not);

PG_FUNCTION_INFO_V1(square_in);
PG_FUNCTION_INFO_V1(square_out);
PG_FUNCTION_INFO_V1(int_to_square);
PG_FUNCTION_INFO_V1(file_rank_to_square);

PG_FUNCTION_INFO_V1(piecesquare_in);
PG_FUNCTION_INFO_V1(piecesquare_out);
PG_FUNCTION_INFO_V1(piecesquare_cpiece);
PG_FUNCTION_INFO_V1(piecesquare_square);

PG_FUNCTION_INFO_V1(piece_in);
PG_FUNCTION_INFO_V1(piece_out);
PG_FUNCTION_INFO_V1(piece_value);

PG_FUNCTION_INFO_V1(cpiece_in);
PG_FUNCTION_INFO_V1(cpiece_out);
PG_FUNCTION_INFO_V1(cpiece_value);
PG_FUNCTION_INFO_V1(cpiece_to_piece);
PG_FUNCTION_INFO_V1(cpiece_side);

PG_FUNCTION_INFO_V1(cfile_in);
PG_FUNCTION_INFO_V1(cfile_out);
PG_FUNCTION_INFO_V1(square_to_cfile);

PG_FUNCTION_INFO_V1(rank_in);
PG_FUNCTION_INFO_V1(rank_out);
PG_FUNCTION_INFO_V1(square_to_rank);

PG_FUNCTION_INFO_V1(diagonal_in);
PG_FUNCTION_INFO_V1(diagonal_out);
PG_FUNCTION_INFO_V1(square_to_diagonal);

PG_FUNCTION_INFO_V1(adiagonal_in);
PG_FUNCTION_INFO_V1(adiagonal_out);
PG_FUNCTION_INFO_V1(square_to_adiagonal);

PG_FUNCTION_INFO_V1(pfilter_in);
PG_FUNCTION_INFO_V1(pfilter_out);

PG_FUNCTION_INFO_V1(move_in);
PG_FUNCTION_INFO_V1(move_out);
PG_FUNCTION_INFO_V1(move_from);
PG_FUNCTION_INFO_V1(move_san);
PG_FUNCTION_INFO_V1(move_to);
PG_FUNCTION_INFO_V1(move_check);
PG_FUNCTION_INFO_V1(move_mate);
PG_FUNCTION_INFO_V1(move_capture);
PG_FUNCTION_INFO_V1(move_piece);
PG_FUNCTION_INFO_V1(move_promotion);

PG_FUNCTION_INFO_V1(ucimove_in);
PG_FUNCTION_INFO_V1(ucimove_out);
PG_FUNCTION_INFO_V1(ucimove_from);
PG_FUNCTION_INFO_V1(ucimove_to);
PG_FUNCTION_INFO_V1(ucimove_promotion);
PG_FUNCTION_INFO_V1(ucimove_san);

PG_FUNCTION_INFO_V1(timecontrol_in);
PG_FUNCTION_INFO_V1(timecontrol_out);

PG_FUNCTION_INFO_V1(eval_in);
PG_FUNCTION_INFO_V1(eval_out);
PG_FUNCTION_INFO_V1(eval_ismate);
PG_FUNCTION_INFO_V1(eval_value);

/*}}}*/
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
        case NO_CPIECE:     result=NO_PIECE; break;

        default:
            CH_ERROR("bad cpiece_t: %i", p); break;
    }
    return result;
}

int _piece_value(const piece_t p) 
{
    char                result;
    switch (p) {
        case PAWN:    result= 1; break;
        case KNIGHT:  result= 3; break;
        case BISHOP:  result= 3; break;
        case ROOK:    result= 5; break;
        case QUEEN:   result= 9; break;
        case KING:    result= 0; break;
        default:
            CH_ERROR("bad cpiece_t: %i", p); break;
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

static 
piece_t    _soft_piece_in(char c)
{
    piece_t             result;

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

		default: result=NO_PIECE; break;
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

Datum
piece_in(PG_FUNCTION_ARGS)
{
	char        *str = PG_GETARG_CSTRING(0);

	if (strlen(str) != 1)
		BAD_TYPE_IN("piece", str);

	PG_RETURN_CHAR(_piece_in(str[0]));
}


Datum
piece_out(PG_FUNCTION_ARGS)
{
	piece_t         piece = PG_GETARG_CHAR(0);
	char			*result = (char *) palloc(2);

	result[0] = _piece_char(piece);
	result[1] = '\0';

	PG_RETURN_CSTRING(result);
}

Datum
piece_value(PG_FUNCTION_ARGS)
{
	piece_t	    piece = PG_GETARG_CHAR(0);
	PG_RETURN_INT32(_piece_value(piece));
}

/*}}}*/
/********************************************************
* 		cpiece
********************************************************/
/*{{{*/

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
int _cpiece_value(const cpiece_t p) 
{
    char                result;
    switch (p) {
        case WHITE_PAWN:    result= 1; break;
        case WHITE_KNIGHT:  result= 3; break;
        case WHITE_BISHOP:  result= 3; break;
        case WHITE_ROOK:    result= 5; break;
        case WHITE_QUEEN:   result= 9; break;
        case WHITE_KING:    result= 0; break;
        case BLACK_PAWN:    result=-1; break;
        case BLACK_KNIGHT:  result=-3; break;
        case BLACK_BISHOP:  result=-3; break;
        case BLACK_ROOK:    result=-5; break;
        case BLACK_QUEEN:   result=-9; break;
        case BLACK_KING:    result= 0; break;
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


side_t _cpiece_side(const cpiece_t p) 
{
    side_t          result;

    switch (p) {
        case WHITE_PAWN:
        case WHITE_KNIGHT:
        case WHITE_BISHOP:
        case WHITE_ROOK:
        case WHITE_QUEEN:
        case WHITE_KING:
            result=WHITE;
            break;
        case BLACK_PAWN:
        case BLACK_KNIGHT:
        case BLACK_BISHOP:
        case BLACK_ROOK:
        case BLACK_QUEEN:
        case BLACK_KING:
            result=BLACK;
            break;
        default:
            CH_ERROR("bad cpiece_t: %i", p); break;
    }
    return result;
}

Datum
cpiece_in(PG_FUNCTION_ARGS)
{
	char        *str = PG_GETARG_CSTRING(0);

	if (strlen(str) != 1)
		BAD_TYPE_IN("cpiece", str);

	PG_RETURN_CHAR(_cpiece_in(str[0]));
}


Datum
cpiece_out(PG_FUNCTION_ARGS)
{
	cpiece_t        piece = PG_GETARG_CHAR(0);
	char			*result = (char *) palloc(2);

	result[0] = _cpiece_char(piece);
	result[1] = '\0';

	PG_RETURN_CSTRING(result);
}

Datum
cpiece_to_piece(PG_FUNCTION_ARGS)
{
	cpiece_t        piece = PG_GETARG_CHAR(0);
    piece_t         result;

    result = _piece_type(piece);


	PG_RETURN_CHAR(result);
}

Datum
cpiece_value(PG_FUNCTION_ARGS)
{
	cpiece_t	    piece = PG_GETARG_CHAR(0);
	PG_RETURN_INT32(_cpiece_value(piece));
}

Datum
cpiece_side(PG_FUNCTION_ARGS)
{
    PG_RETURN_CHAR(_cpiece_side(PG_GETARG_CHAR(0)));
}

/*}}}*/
/********************************************************
 * 		cfile
 ********************************************************/
/*{{{*/
char 
_cfile_in(char f) 
{
    if (f < 'a' || f > 'h') {
        CH_ERROR("cfile '%c' not in range a-h", f);
    }
    return f - 'a';
}

Datum
square_to_cfile(PG_FUNCTION_ARGS)
{
    char 			s = PG_GETARG_CHAR(0);
    PG_RETURN_CHAR(TO_FILE(s));
}

Datum
cfile_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);

    if (strlen(str) != 1) BAD_TYPE_IN("cfile", str); 
    PG_RETURN_CHAR(_cfile_in(str[0]));
}

Datum
cfile_out(PG_FUNCTION_ARGS)
{
    char 			f = PG_GETARG_CHAR(0);
    char            *result = palloc(2);

    if (f < 0 || f > 7)
        BAD_TYPE_OUT("cfile", f);

    result[0] = f + 'a';
    result[1] = '\0';

    PG_RETURN_CSTRING(result);
}

/*}}}*/
/********************************************************
 * 		rank
 ********************************************************/
/*{{{*/
static char _rank_in(char r) 
{
    if (r < '1' || r > '8') 
        CH_ERROR("chess rank '%c' not in range 1-8", r);
    return r - '1';
}

    Datum
square_to_rank(PG_FUNCTION_ARGS)
{
    char 			s = PG_GETARG_CHAR(0);

    if (s < 0 || s > 7)
        BAD_TYPE_OUT("rank", s);

    PG_RETURN_CHAR(TO_RANK(s));
}

    Datum
rank_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);
    if (strlen(str) != 1)
        BAD_TYPE_IN("rank", str); 
    PG_RETURN_CHAR(_rank_in(str[0]));
}

    Datum
rank_out(PG_FUNCTION_ARGS)
{
    char 			f = PG_GETARG_CHAR(0);
    char            *result = palloc(2);

    result[0] = f + '1';
    result[1] = '\0';

    PG_RETURN_CSTRING(result);
}

/*}}}*/
/********************************************************
 * 		square
 ********************************************************/
/*{{{*/

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


Datum
square_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);

    if (strlen(str) != 2)
        BAD_TYPE_IN("square", str); 

    PG_RETURN_CHAR(_square_in(str[0], str[1]));
}



static char *_square_out(char c, char *str)
{

    if (c < 0 || c > 63)
        BAD_TYPE_OUT("square", c);

    switch (c % 8) {
        case 0: str[0]='a'; break;
        case 1: str[0]='b'; break;
        case 2: str[0]='c'; break;
        case 3: str[0]='d'; break;
        case 4: str[0]='e'; break;
        case 5: str[0]='f'; break;
        case 6: str[0]='g'; break;
        case 7: str[0]='h'; break;

        default: CH_ERROR("bad switch statement %d", c);
    }

    switch (c / 8) {
        case 0: str[1]='1'; break;
        case 1: str[1]='2'; break;
        case 2: str[1]='3'; break;
        case 3: str[1]='4'; break;
        case 4: str[1]='5'; break;
        case 5: str[1]='6'; break;
        case 6: str[1]='7'; break;
        case 7: str[1]='8'; break;

        default: CH_ERROR("bad switch statement %d", c);
    }
    return str;
}

Datum
square_out(PG_FUNCTION_ARGS)
{
    char			square = PG_GETARG_CHAR(0);
    char			*result = (char *) palloc(3);

    _square_out(square, result);
    result[2] = '\0';

    PG_RETURN_CSTRING(result);
}

Datum
file_rank_to_square(PG_FUNCTION_ARGS)
{
    char			file = PG_GETARG_CHAR(0);
    char			rank = PG_GETARG_CHAR(1);

    PG_RETURN_CHAR(file) + ( 8 * rank);
}

Datum
int_to_square(PG_FUNCTION_ARGS)
{
    char            c = (char)PG_GETARG_INT32(0);
    char			*result = (char *) palloc(3);

    if (c < 0 || c > 63)
        CH_ERROR("value must be in between 0 and 63");
    PG_RETURN_CHAR(c);

    _square_out(c, result);
    result[2] = '\0';
    PG_RETURN_CSTRING(result);
}
/*}}}*/
/********************************************************
 * 		diagonal
 ********************************************************/
/*{{{*/


Datum
square_to_diagonal(PG_FUNCTION_ARGS)
{
    char 			s = PG_GETARG_CHAR(0);
    PG_RETURN_CHAR(_diagonal_in(s));
}

Datum
diagonal_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);
    if (strlen(str) != 3)
        BAD_TYPE_IN("diagonal", str); 

    PG_RETURN_CHAR(_diagonal_in(_square_in(str[0],str[1])));
}

Datum
diagonal_out(PG_FUNCTION_ARGS)
{
    char 			d = PG_GETARG_CHAR(0);
    char            *result = palloc(3);

    switch (d) {
        case -7: strcpy(result, "a1"); break;
        case -6: strcpy(result, "a2"); break;
        case -5: strcpy(result, "a3"); break;
        case -4: strcpy(result, "a4"); break;
        case -3: strcpy(result, "a5"); break;
        case -2: strcpy(result, "a6"); break;
        case -1: strcpy(result, "a7"); break;
        case  0: strcpy(result, "a8"); break;
        case  1: strcpy(result, "b8"); break;
        case  2: strcpy(result, "c8"); break;
        case  3: strcpy(result, "d8"); break;
        case  4: strcpy(result, "e8"); break;
        case  5: strcpy(result, "f8"); break;
        case  6: strcpy(result, "g8"); break;
        case  7: strcpy(result, "h8"); break;
    }
    result[3] = '\0';
    PG_RETURN_CSTRING(result);
}


/*}}}*/
/********************************************************
 * 		adiagonal
 ********************************************************/
/*{{{*/

    Datum
square_to_adiagonal(PG_FUNCTION_ARGS)
{
    char 			s = PG_GETARG_CHAR(0);
    PG_RETURN_CHAR(_adiagonal_in(s));
}

    Datum
adiagonal_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);
    if (strlen(str) != 2)
        BAD_TYPE_IN("diagonal", str); 

    PG_RETURN_CHAR(_adiagonal_in(_square_in(str[0],str[1])));
}

    Datum
adiagonal_out(PG_FUNCTION_ARGS)
{
    char 			d = PG_GETARG_CHAR(0);
    char            *result = palloc(3);

    switch (d) {
        case -7: strcpy(result, "a8"); break;
        case -6: strcpy(result, "a7"); break;
        case -5: strcpy(result, "a6"); break;
        case -4: strcpy(result, "a5"); break;
        case -3: strcpy(result, "a4"); break;
        case -2: strcpy(result, "a3"); break;
        case -1: strcpy(result, "a2"); break;
        case  0: strcpy(result, "a1"); break;
        case  1: strcpy(result, "b1"); break;
        case  2: strcpy(result, "c1"); break;
        case  3: strcpy(result, "d1"); break;
        case  4: strcpy(result, "e1"); break;
        case  5: strcpy(result, "f1"); break;
        case  6: strcpy(result, "g1"); break;
        case  7: strcpy(result, "h1"); break;
    }
    result[3] = '\0';
    PG_RETURN_CSTRING(result);
}
/*}}}*/
/********************************************************
 * 	    pindex: piece index	
 ********************************************************/
/*{{{*/


Datum
pindex_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);
    PG_RETURN_INT16(_pindex_in(str));
}

Datum
pindex_out(PG_FUNCTION_ARGS)
{
    unsigned short  pindex = PG_GETARG_INT16(0);
    char 			*result = palloc(PIECE_INDEX_MAX+1);

    //CH_NOTICE("val in: %i", pindex);

    memset(result, 0, PIECE_INDEX_MAX+1);
    result[0] = GET_BIT16(pindex, 15 - 1)  ? 'Q' : '.';
    result[1] = GET_BIT16(pindex, 14 - 1)  ? 'R' : '.';
    result[2] = GET_BIT16(pindex, 13 - 1)  ? 'R' : '.';
    result[3] = GET_BIT16(pindex, 12 - 1)  ? 'B' : '.';
    result[4] = GET_BIT16(pindex, 11 - 1)  ? 'B' : '.';
    result[5] = GET_BIT16(pindex, 10 - 1)  ? 'N' : '.';
    result[6] = GET_BIT16(pindex,  9 - 1)  ? 'N' : '.';
    result[7] = GET_BIT16(pindex,  8 - 1)  ? 'P' : '.';
    result[8] = GET_BIT16(pindex,  7 - 1)  ? 'P' : '.';
    result[9] = GET_BIT16(pindex,  6 - 1)  ? 'P' : '.';
    result[10] = GET_BIT16(pindex, 5 - 1) ? 'P' : '.';
    result[11] = GET_BIT16(pindex, 4 - 1) ? 'P' : '.';
    result[12] = GET_BIT16(pindex, 3 - 1) ? 'P' : '.';
    result[13] = GET_BIT16(pindex, 2 - 1) ? 'P' : '.';
    result[14] = GET_BIT16(pindex, 1 - 1) ? 'P' : '.';

    PG_RETURN_CSTRING(result);
}

Datum
pindex_to_int32(PG_FUNCTION_ARGS)
{
    unsigned short  pindex = PG_GETARG_INT16(0);
    PG_RETURN_INT32((int32)pindex);
}

/*}}}*/
/********************************************************
 * 		side
 ********************************************************/
/*{{{*/

Datum
side_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);
    unsigned char   val;

    if (strlen(str) == 1) {
        if (str[0] == 'w')
            val = WHITE;
        else if (str[0] == 'b')
            val = BLACK;
        else
            BAD_TYPE_IN("side", str);
    }
    else if (strlen(str) == 5) {
        if (strcmp(str, "white") == 0 || strcmp(str, "WHITE") == 0)
            val = WHITE;
        else if (strcmp(str, "black") == 0 || strcmp(str, "BLACK") == 0)
            val = BLACK;
        else
            BAD_TYPE_IN("side", str);
    } else 
        BAD_TYPE_IN("side", str);

    PG_RETURN_CHAR(val);
}


Datum
side_out(PG_FUNCTION_ARGS)
{
    char			side = PG_GETARG_CHAR(0);
    char			*result = (char *) palloc(2);

    if (side == WHITE)
        result[0] = 'w';
    else if (side == BLACK)
        result[0] = 'b';
    else
        BAD_TYPE_OUT("side", side);
    result[1] = '\0';

    PG_RETURN_CSTRING(result);
}

Datum
not(PG_FUNCTION_ARGS)
{
    char			side = PG_GETARG_CHAR(0);

    if (side==BLACK) PG_RETURN_CHAR(WHITE);
    else if (side==WHITE) PG_RETURN_CHAR(BLACK);
    else BAD_TYPE_OUT("side", side);

}


/*}}}*/
/********************************************************
 * 		piecesquare
 ********************************************************/
/*{{{*/
Datum
piecesquare_in(PG_FUNCTION_ARGS)
{
	char 		    	*str = PG_GETARG_CSTRING(0);
    uint16              result=0;
    int                 kind;
	
	if (strlen(str) == 3)
        INIT_PS(result, _cpiece_in(str[0]), _square_in(str[1], str[2]));
    else if (strlen(str)==4) {
        if (str[0] != '+')
            BAD_TYPE_IN("piecesquare", str); 
        INIT_PS(result, _cpiece_in(str[1]), _square_in(str[2], str[3]));
        SET_PS_SUBJECT(result, CPIECE_MOBILITY);

    } else if (strlen(str) == 5) {
        INIT_PS(result, _cpiece_in(str[2]), _square_in(str[3], str[4]));
        SET_PS_SUBJECT(result, _cpiece_in(str[0]));
        switch (str[1]) {
            case '>': kind=PS_ATTACKS; break;
            case '/': kind=PS_DEFENDS; break;
            case '-': kind=PS_XRAY; break;
            default: 
                BAD_TYPE_IN("piecesquare", str); 
                break;
        }
        SET_PS_KIND(result, kind);
    } else
		BAD_TYPE_IN("piecesquare", str);

    //debug_bits(result, 16);
	PG_RETURN_INT16(result);
}

Datum
piecesquare_out(PG_FUNCTION_ARGS)
{
	uint16          ps = PG_GETARG_UINT16(0);
	char			*result = (char *) palloc(6);
    char            square, piece;
    int             j=0;

    //debug_bits(ps, 16);

    square = GET_PS_SQUARE(ps);
    //CH_NOTICE("square:%i", square);
    if (square < 0 || square >= SQUARE_MAX)
        BAD_TYPE_OUT("piecesquare", ps);

    piece = GET_PS_PIECE(ps);
    //CH_NOTICE("piece:%i", piece);
    if (piece < 0 || piece >= CPIECE_MAX)
        BAD_TYPE_OUT("piecesquare", ps);

    //mobility type 
    if (GET_PS_SUBJECT(ps)==CPIECE_MOBILITY)
        result[j++]='+';

    else if (GET_PS_KIND(ps)) {
        result[j++] = _cpiece_char(GET_PS_SUBJECT(ps));
        switch (GET_PS_KIND(ps)) {
            case PS_ATTACKS:    result[j++]='>'; break;
            case PS_DEFENDS:    result[j++]='/'; break;
            case PS_XRAY:       result[j++]='-'; break;
            default: 
                BAD_TYPE_OUT("piecesquare", ps); 
                break;
        }
    }

	result[j++] = _cpiece_char(piece);
	result[j++] = CHAR_CFILE(square);
	result[j++] = CHAR_RANK(square);
	result[j++] = '\0';
	PG_RETURN_CSTRING(result);
}

Datum
piecesquare_square(PG_FUNCTION_ARGS)
{
	uint16			ps = PG_GETARG_UINT16(0);
	PG_RETURN_CHAR((char)GET_PS_SQUARE(ps));
}


Datum
piecesquare_cpiece(PG_FUNCTION_ARGS)
{
	uint16			ps = PG_GETARG_UINT16(0);
	PG_RETURN_CHAR((char)GET_PS_PIECE(ps));
}

/*}}}*/
/********************************************************
 * 		pfilter
 ********************************************************/
/*{{{*/
Datum
pfilter_in(PG_FUNCTION_ARGS)
{
	char 		    	*str = PG_GETARG_CSTRING(0);
    bool                *result = palloc(PIECE_MAX-1); // empty pieces not allowed
    size_t              n = strlen(str);
    char                p;
    bool                found;

    memset(result, 0, PIECE_MAX-1);
	
	if (n < 1 || n > 6)
		BAD_TYPE_IN("pquery", str);

    for (int j=0; j<strlen(str); j++) {
        found = false;
        for (int i=PAWN; i<PIECE_MAX; i++) {
            p = _piece_in(str[j]);

            if (p == i) {
                result[i-1] = true;
                found = true;
                break;
            }
        }
        if (!found) BAD_TYPE_IN("pquery", str);
    }

	PG_RETURN_POINTER(result);
}

Datum
pfilter_out(PG_FUNCTION_ARGS)
{
	bool            *pfilter = PG_GETARG_POINTER(0);
	char			*result = (char *) palloc(PIECE_MAX);
    int             j=0;

    for (int i=PAWN; i<PIECE_MAX; i++) {
        if (pfilter[i-1]) result[j++] = _piece_char(i);
    }
    result[j]= '\0';

	PG_RETURN_CSTRING(result);
}/*}}}*/
/********************************************************
 * 		move
 ********************************************************/
/*{{{*/
Datum
move_in(PG_FUNCTION_ARGS)
{
	const char 		    *str = PG_GETARG_CSTRING(0);
    char                *cpy = palloc(7);
    const char          *p = cpy;
    Move                *result = palloc0(sizeof(Move));
    size_t              i = strlen(str) - 1;
    piece_t             piece;


    /* Qd8-h4+ 15. Kh2-g1 Bb7xg2 16. Bc4xe6+ Kg8-h8 
     * promotion: e7xd8Q+
     * */


	if (strlen(str) > 8 || strlen(str) < 5)
        BAD_TYPE_IN("move", str); 
    cpy = strcpy(cpy, str);

    if (cpy[i] ==  '+') {
        result->is_check = 1;
        cpy[i--] = '\0';
    }
    if (cpy[i] ==  '#') {
        result->is_mate = 1;
        cpy[i--] = '\0';
    }
    if (_soft_piece_in(cpy[i])) {
        result->promotion = _soft_piece_in(cpy[i]);
        cpy[i--] = '\0';
    }

    if (result->is_check + result->is_mate > 1)
        BAD_TYPE_IN("move", str); 

    result->piece = PAWN;
    piece = _soft_piece_in(p[0]);
    switch(piece) {
        case PAWN:
        case ROOK:
        case KNIGHT:
        case BISHOP:
        case QUEEN:
        case KING:
                  result->piece = piece;
                  p++;
                  break;
		default:
                  break;
    }

    if (p[2] == 'x') {
        result->is_capture = 1;
    } else if (p[2] == '-') {
        // no op
    } else {
        BAD_TYPE_IN("move", str); 
    }

    if (result->promotion && result->piece != PAWN)
        BAD_TYPE_IN("move", str); 

    if (result->promotion == PAWN)
        BAD_TYPE_IN("move", str); 

    result->from = _square_in(p[0], p[1]);
    result->to = _square_in(p[3], p[4]);

    if (strlen(p+5)) {
        BAD_TYPE_IN("move", str); 
    }

    pfree(cpy);
	PG_RETURN_POINTER(result);
}

Datum
move_out(PG_FUNCTION_ARGS)
{
	const Move      *move = (Move *)PG_GETARG_POINTER(0);
	char			*result = (char *) palloc0(8);
    char            *p = result;

    /* Qd8-h4+ 15. Kh2-g1 Bb7xg2 16. Bc4xe6+ Kg8-h8 
     * promotion: e7xd8Q+
     * */

    if (move->piece != PAWN) {
        result[0] = _piece_char(move->piece);
        p++;
    }
    _square_out(move->from, p);
    p = p + 2;
    (p++)[0] = move->is_capture ? 'x' : '-';

    _square_out(move->to, p);
    p = p + 2;

    if (move->promotion)
        (p++)[0] = _piece_char(move->promotion);
    if (move->is_check)
        (p++)[0] = '+';
    if (move->is_mate)
        (p++)[0] = '#';

    (p++)[0] = '\0';

	PG_RETURN_CSTRING(result);
}

Datum
move_san(PG_FUNCTION_ARGS)
{
	const Move      *move = (Move *)PG_GETARG_POINTER(0);
	char			*result = (char *) palloc0(8);
    char            *p = result;

    //exf8=Q#

    if (move->piece != PAWN) {
        result[0] = _piece_char(move->piece);
        p++;
    }
    else if (move->piece == PAWN && move->is_capture) {
        result[0] = CHAR_CFILE(move->from);
        p++;
    }

    if (move->is_capture) {
        p[0] = 'x';
        p++;
    }

    _square_out(move->to, p);
    p = p + 2;

    if (move->promotion) {
        (p++)[0] = '=';
        (p++)[0] = _piece_char(move->promotion);
    }

    if (move->is_check)
        (p++)[0] = '+';
    if (move->is_mate)
        (p++)[0] = '#';

    (p++)[0] = '\0';

    //XXX Im not sure if this leaks
    PG_RETURN_TEXT_P(cstring_to_text(result));
}

Datum
move_from(PG_FUNCTION_ARGS)
{
	const Move      *move = (Move *)PG_GETARG_POINTER(0);
	PG_RETURN_CHAR(move->from);
}

Datum
move_to(PG_FUNCTION_ARGS)
{
	const Move      *move = (Move *)PG_GETARG_POINTER(0);
	PG_RETURN_CHAR(move->to);
}

Datum
move_check(PG_FUNCTION_ARGS)
{
	const Move      *move = (Move *)PG_GETARG_POINTER(0);
	PG_RETURN_BOOL(move->is_check==1);
}

Datum
move_mate(PG_FUNCTION_ARGS)
{
	const Move      *move = (Move *)PG_GETARG_POINTER(0);
	PG_RETURN_BOOL(move->is_mate==1);
}

Datum
move_capture(PG_FUNCTION_ARGS)
{
	const Move      *move = (Move *)PG_GETARG_POINTER(0);
	PG_RETURN_BOOL(move->is_capture==1);
}

Datum
move_piece(PG_FUNCTION_ARGS)
{
	const Move      *move = (Move *)PG_GETARG_POINTER(0);
	PG_RETURN_CHAR(move->piece);
}


Datum
move_promotion(PG_FUNCTION_ARGS)
{
	const Move      *move = (Move *)PG_GETARG_POINTER(0);
    if (move->promotion)
        PG_RETURN_CHAR(move->promotion);
    else
        PG_RETURN_NULL();
}

/*}}}*/
/********************************************************
 * 		ucimove
 ********************************************************/
/*{{{*/
Datum
ucimove_in(PG_FUNCTION_ARGS)
{
	const char 		    *str = PG_GETARG_CSTRING(0);
    char                *cpy = palloc(6);
    const char          *p = cpy;
    UciMove             *result = (UciMove *) palloc0(sizeof(UciMove));
    size_t              i = strlen(str) - 1;


	if (strlen(str) < 4 || strlen(str) > 5)
        BAD_TYPE_IN("ucimove", str); 

    cpy = strcpy(cpy, str);
    if (_soft_piece_in(cpy[i])) {
        result->promotion = _soft_piece_in(cpy[i]);
        cpy[i--] = '\0';
    }

    if (result->promotion == PAWN) {
        BAD_TYPE_IN("ucimove", str); 
    }
    if (strlen(p+4)) {
        BAD_TYPE_IN("ucimove", str); 
    }

    result->from = _square_in(p[0], p[1]);
    result->to = _square_in(p[2], p[3]);

    pfree(cpy);
	PG_RETURN_POINTER(result);
}

Datum
ucimove_out(PG_FUNCTION_ARGS)
{

	const UciMove   *move = (UciMove *)PG_GETARG_POINTER(0);
	char			*result = (char *) palloc0(6);
    char            *p = result;

    /* Qd8-h4+ 15. Kh2-g1 Bb7xg2 16. Bc4xe6+ Kg8-h8 
     * promotion: e7xd8Q+
     * */

    _square_out(move->from, p);
    p = p + 2;
    _square_out(move->to, p);
    p = p + 2;

    if (move->promotion)
        (p++)[0] = _piece_char(move->promotion);

    (p++)[0] = '\0';

	PG_RETURN_CSTRING(result);
}

Datum
ucimove_from(PG_FUNCTION_ARGS)
{
	const UciMove      *move = (UciMove *)PG_GETARG_POINTER(0);
	PG_RETURN_CHAR(move->from);
}

Datum
ucimove_to(PG_FUNCTION_ARGS)
{
	const UciMove      *move = (UciMove *)PG_GETARG_POINTER(0);
	PG_RETURN_CHAR(move->to);
}

Datum
ucimove_promotion(PG_FUNCTION_ARGS)
{
	const UciMove      *move = (UciMove *)PG_GETARG_POINTER(0);
    if (!move->promotion)
        PG_RETURN_NULL();
    else
        PG_RETURN_CHAR(move->promotion);
}

Datum
ucimove_san(PG_FUNCTION_ARGS)
{
	const UciMove       *move = (UciMove *)PG_GETARG_POINTER(0);
    const Board         *b = (Board *)PG_GETARG_POINTER(1); 
    board_t             *board = _bitboard_to_board(b);
    piece_t             source = _piece_type(board[move->from]);
    piece_t             target = _piece_type(board[move->to]);
    char                *result = palloc(10);
    char                *p = result;

    pfree(board);

    if (!source)
        PG_RETURN_NULL();

    if (source== KING) {
        if (   (move->from == 4 && move->to == 2) 
            || (move->from == 60 && move->to == 58)) {
            sprintf(result, "%s", "O-O");
            PG_RETURN_TEXT_P(cstring_to_text(result));
        }
        if (   (move->from == 4 && move->to == 6) 
            || (move->from == 60 && move->to == 62)) {
            sprintf(result, "%s", "O-O-O");
            PG_RETURN_TEXT_P(cstring_to_text(result));
        }

    } else if (source != PAWN) {
        result[0] = _piece_char(source);
        p++;
    }
    else if (source == PAWN && target) {
        result[0] = CHAR_CFILE(move->from);
        p++;
    }

    if (target) {
        p[0] = 'x';
        p++;
    }

    _square_out(move->to, p);
    p = p + 2;

    if (move->promotion) {
        (p++)[0] = '=';
        (p++)[0] = _piece_char(move->promotion);
    }

    (p++)[0] = '\0';

    //XXX Im not sure if this leaks
    PG_RETURN_TEXT_P(cstring_to_text(result));

}

/*}}}*/
/********************************************************
 * 		time_control
 ********************************************************/
/*{{{*/
typedef enum {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
} str2int_errno;

/* Convert string s to int out.
 *
 * @param[out] out The converted int. Cannot be NULL.
 *
 * @param[in] s Input string to be converted.
 *
 *     The format is the same as strtol,
 *     except that the following are inconvertible:
 *
 *     - empty string
 *     - leading whitespace
 *     - any trailing characters that are not part of the number
 *
 *     Cannot be NULL.
 *
 * @param[in] base Base to interpret string in. Same range as strtol (2 to 36).
 *
 * @return Indicates if the operation succeeded, or why it failed.
 */

static str2int_errno str2int(int *out, char *s, int base) {
    char *end;
    long l;

    if (s[0] == '\0' || isspace(s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;

    l = strtol(s, &end, base);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return STR2INT_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return STR2INT_UNDERFLOW;
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    *out = l;
    return STR2INT_SUCCESS;
}

/* from the spec:
9.6.1 Tag: TimeControl
This uses a list of one or more time control fields. Each field contains a descriptor for each time control period; if more than one descriptor is present then they are separated by the colon character (":"). The descriptors appear in the order in which they are used in the game. The last field appearing is considered to be implicitly repeated for further control periods as needed.
There are six kinds of TimeControl fields.

The first kind is a single question mark ("?") which means that the time control
mode is unknown. When used, it is usually the only descriptor present.

The second kind is a single hyphen ("-") which means that there was no time
control mode in use. When used, it is usually the only descriptor present.

The third Time control field kind is formed as two positive integers separated
by a solidus ("/") character. The first integer is the number of moves in the
period and the second is the number of seconds in the period. Thus, a time
control period of 40 moves in 2 1/2 hours would be represented as "40/9000".

The fourth TimeControl field kind is used for a "sudden death" control period.
It should only be used for the last descriptor in a TimeControl tag value. It is
sometimes the only descriptor present. The format consists of a single integer
that gives the number of seconds in the period. Thus, a blitz game would be
represented with a TimeControl tag value of "300".

The fifth TimeControl field kind is used for an "incremental" control period. It
should only be used for the last descriptor in a TimeControl tag value and is
usually the only descriptor in the value. The format consists of two positive
integers separated by a plus sign ("+") character. The first integer gives the
minimum number of seconds allocated for the period and the second integer gives
the number of extra seconds added after each move is made. So, an incremental
time control of 90 minutes plus one extra minute per move would be given by
"4500+60" in the TimeControl tag value.

The sixth TimeControl field kind is used for a "sandclock" or "hourglass"
control period. It should only be used for the last descriptor in a TimeControl
tag value and is usually the only descriptor in the value. The format consists
of an asterisk ("*") immediately followed by a positive integer. The integer
gives the total number of seconds in the sandclock period. The time control is
implemented as if a sandclock were set at the start of the period with an equal
amount of sand in each of the two chambers and the players invert the sandclock
after each move with a time forfeit indicated by an empty upper chamber.
Electronic implementation of a physical sandclock may be used. An example
sandclock specification for a common three minute egg timer sandclock would have
a tag value of "*180".

Additional TimeControl field kinds will be defined as necessary.
*/

Datum
timecontrol_in(PG_FUNCTION_ARGS)
{
	char                *str = PG_GETARG_CSTRING(0);
	char 				cpy[17];
    char                *p = str;
    char                *t = str;
    int                 val;
    TimeControl         *result = palloc0(sizeof(TimeControl));

    if (strlen(str) > 16)
        BAD_TYPE_IN("timecontrol", str);

    strcpy(cpy, str);

    // sandclock
	if(str[0] == '-' && str[1] == '\0') {
        result->kind = CH_TIME_NONE;
    } else if(str[0] == '*') {

        if (str2int(&val, ++p, 10)) {
            BAD_TYPE_IN("timecontrol", str);
        }
        if (val > USHRT_MAX)
            BAD_TYPE_IN("timecontrol", str);

        result->primary = val;
        result->kind = CH_TIME_SANDCLOCK;

    // seconds with tickback
    } else if((t = strchr(cpy, '+'))) {

        if (str2int(&val, t, 10)) {
            BAD_TYPE_IN("timecontrol", str);
        }
        if (val > UCHAR_MAX)
            BAD_TYPE_IN("timecontrol", str);
        result->secondary = val;

        p = strtok(cpy, "+");
        if (str2int(&val, p, 10))
            BAD_TYPE_IN("timecontrol", str);
        if (val > USHRT_MAX)
            BAD_TYPE_IN("timecontrol", str);

        result->primary = val;
        result->kind = CH_TIME_SECS;


    // fide type moves
    } else if((t = strchr(cpy, '/'))) {

        if (str2int(&val, ++t, 10)) {
            BAD_TYPE_IN("timecontrol", str);
        }
        if (val > USHRT_MAX)
            BAD_TYPE_IN("timecontrol", str);
        result->primary = val;

        p = strtok(cpy, "/");
        if (str2int(&val, p, 10))
            BAD_TYPE_IN("timecontrol", str);
        if (val > UCHAR_MAX)
            BAD_TYPE_IN("timecontrol", str);

        result->secondary= val;
        result->kind = CH_TIME_MOVES;

    //sudden death
    } else {
        if (str2int(&val, str, 10))
            BAD_TYPE_IN("timecontrol", str);
        result->primary = val;
        result->kind = CH_TIME_SECS;
    }

	PG_RETURN_POINTER(result);
}

Datum
timecontrol_out(PG_FUNCTION_ARGS)
{
	const TimeControl   *control= (TimeControl*)PG_GETARG_POINTER(0);
	char			    *result = (char *) palloc0(16);
    char                *p = result;

    if (control->kind == CH_TIME_NONE) {
        sprintf(p, "-");
    } else if (control->kind == CH_TIME_MOVES) {
            sprintf(p, "%d/%d", control->secondary, control->primary);
    } else if (control->kind == CH_TIME_SANDCLOCK) {
        sprintf(p, "*%d", control->primary);
    } else {
        if (control->secondary) {
            sprintf(p, "%d+%d", control->primary, control->secondary);
        } else {
            sprintf(p, "%d", control->primary);
        }
    }
	PG_RETURN_CSTRING(result);
}
/*}}}*/
/********************************************************
 * 		eval
 ********************************************************/
/*{{{*/
Datum
eval_in(PG_FUNCTION_ARGS)
{
	char                *str = PG_GETARG_CSTRING(0);
    char                *p = str;
    char                *end;
    float               result;
    int                 mate;

    if (p[0]=='#') {
        CH_NOTICE("pound");
        p++;
        if (str2int(&mate, p, 10)) {
            CH_NOTICE("str2int faild");
            BAD_TYPE_IN("eval", str);
        }
        CH_NOTICE("mate %d", mate);
        result = (float)((mate < 0 ? -1 : 1) * 1000 + mate);
    } else {
        result = strtod(p, &end);
        if (errno == ERANGE) {
            CH_NOTICE("error no %d", errno);
            BAD_TYPE_IN("eval", str);
        }
        if (abs(result) >= 1000) {
            CH_NOTICE("too big");
            BAD_TYPE_IN("eval", str);
        }
        if (!end[0]==0) {
            CH_NOTICE("extra chars");
            BAD_TYPE_IN("eval", str);
        }
    }

	PG_RETURN_FLOAT4(result);
}

Datum
eval_out(PG_FUNCTION_ARGS)
{
	const float         eval = PG_GETARG_FLOAT4(0);
	char			    *result = (char *) palloc0(16);
    char                *p = result;

    if (abs(eval) >=1000) {
        (p++)[0] = '#';
        sprintf(p, "%d", (int)(eval >= 0 ? (eval-1000) : (eval+1000)) );

    } else {
        sprintf(p, "%f", eval);
    }

	PG_RETURN_CSTRING(result);
}

Datum
eval_ismate(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(abs(PG_GETARG_FLOAT4(0)) >= 1000);
}

Datum
eval_value(PG_FUNCTION_ARGS)
{

	const float         eval = PG_GETARG_FLOAT4(0);

    CH_NOTICE("value : %f", eval);
    if (abs(eval) >=1000) {
        PG_RETURN_FLOAT4(eval >= 0 ? (eval-1000) : (eval+1000));

    } else {
        PG_RETURN_FLOAT4(eval);
    }
}/*}}}*/

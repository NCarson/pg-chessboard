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

/********************************************************
 * 		defines
 ********************************************************/
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

PG_FUNCTION_INFO_V1(piecesquare_in);
PG_FUNCTION_INFO_V1(piecesquare_out);
PG_FUNCTION_INFO_V1(piecesquare_cpiece);
PG_FUNCTION_INFO_V1(piecesquare_square);
PG_FUNCTION_INFO_V1(piecesquares_to_board);

PG_FUNCTION_INFO_V1(cpiece_in);
PG_FUNCTION_INFO_V1(cpiece_out);

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

/*}}}*/

/********************************************************
* 		piece
********************************************************/
/*{{{*/

Datum
cpiece_in(PG_FUNCTION_ARGS)
{
	char 			*str = PG_GETARG_CSTRING(0);

	if (strlen(str) != 1)
		BAD_TYPE_IN("piece", str);

	PG_RETURN_CHAR(_cpiece_in(str[0]));
}


Datum
cpiece_out(PG_FUNCTION_ARGS)
{
	char			piece = PG_GETARG_CHAR(0);
	char			*result = (char *) palloc(2);

	result[0] = _cpiece_char(piece);
	result[1] = '\0';

	PG_RETURN_CSTRING(result);
}

/*}}}*/
/********************************************************
 * 		file
 ********************************************************/
static char _cfile_in(char f) /*{{{*/
{
    if (f < 'a' || f > 'h') 
        CH_ERROR("chess file not in range %c", f);
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
    if (strlen(str) != 2)
        BAD_TYPE_IN("file", str); 
    PG_RETURN_CHAR(_cfile_in(str[0]));
}

Datum
cfile_out(PG_FUNCTION_ARGS)
{
    char 			f = PG_GETARG_CHAR(0);
    char            *result = palloc(2);

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
        CH_ERROR("chess rank '%c' not in range", r);
    return r - '1';
}

    Datum
square_to_rank(PG_FUNCTION_ARGS)
{
    char 			s = PG_GETARG_CHAR(0);
    PG_RETURN_CHAR(TO_RANK(s));
}

    Datum
rank_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);
    if (strlen(str) != 2)
        BAD_TYPE_IN("rank", str); 
    PG_RETURN_CHAR(_rank_in(str[1]));
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
square_in(PG_FUNCTION_ARGS)
{
    char 			*str = PG_GETARG_CSTRING(0);

    if (strlen(str) != 2)
        BAD_TYPE_IN("square", str); 

    PG_RETURN_CHAR(_square_in(str[0], str[1]));
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
    if (strlen(str) != 2)
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
    unsigned short      result=0;
	
	if (strlen(str) != 3)
		BAD_TYPE_IN("piecesquare", str);
	
	SET_PS(result, _cpiece_in(str[0]), _square_in(str[1], str[2]));
    //debug_bits(result, 16);

	PG_RETURN_INT16(result);
}

Datum
piecesquare_out(PG_FUNCTION_ARGS)
{
	unsigned short  ps = PG_GETARG_UINT16(0);
	char			*result = (char *) palloc(4);
    char            square, piece;

    square = GET_PS_SQUARE(ps);
    if (square < 0 || square >= SQUARE_MAX)
        BAD_TYPE_OUT("piecesquare", ps);
    piece = GET_PS_PIECE(ps);
    if (piece < 0 || piece >= CPIECE_MAX)
        BAD_TYPE_OUT("piecesquare", ps);

	result[0] = _cpiece_char(piece);
	result[1] = CHAR_CFILE(square);
	result[2] = CHAR_RANK(square);
	result[3] = '\0';
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

static Board *_piecesquares_to_board(const char *footer, const Datum * input, const int size, const bool * valsNullFlags)
{

    uint16              ps;
    cpiece_t            p;
    char                s;
    Board               *b; 
    int                 k=0;

    INIT_BOARD(b, size);
    b->pcount = size;
    _board_footer_in(b, footer);

    if (size < 1)
        CH_ERROR("_piecesquares_to_board: internal error:pieces less than 1 :%i", size);
    if (size > PIECES_MAX)
        CH_ERROR("_piecesquares_to_board: internal error: too many pieces :%i", size);

	for (int i = 0; i < size; i++) {
		if (valsNullFlags[i]) {
            continue;
		} 
        ps = DatumGetInt16(input[i]);
        s = GET_PS_SQUARE(ps);
        p = GET_PS_PIECE(ps);
        if (s < 0 || s >= SQUARE_MAX) CH_ERROR("_piecesquares_to_board: internal error: invalid square: %i", s);
        if (p < 0 || p >= CPIECE_MAX) CH_ERROR("_piecesquares_to_board: internal error: invalid piece:%i", p);
        SET_BIT64(b->board, TO_SQUARE_IDX(s));
        SET_PIECE(b->pieces, k, p);
        k++;
    }
    if (k != size)
        CH_ERROR("_piecesquares_to_board: internal error: size:%d != pcount:%d", size, b->pcount);
    //debug_bitboard(b->board);
    return b;
}

Datum 
piecesquares_to_board(PG_FUNCTION_ARGS)
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

    Board               *board;
    char                *str = text_to_cstring(PG_GETARG_TEXT_P(1));

	if (PG_ARGISNULL(0)) { ereport(ERROR, (errmsg("Null arrays not accepted"))); } 
	vals = PG_GETARG_ARRAYTYPE_P(0);
	if (ARR_NDIM(vals) == 0) { PG_RETURN_NULL(); }
	if (ARR_NDIM(vals) > 1) { ereport(ERROR, (errmsg("One-dimesional arrays are required"))); }

	// Determine the array element types.
	valsType = ARR_ELEMTYPE(vals);
	valsLength = (ARR_DIMS(vals))[0];
	get_typlenbyvalalign(valsType, &valsTypeWidth, &valsTypeByValue, &valsTypeAlignmentCode);

	// Extract the array contents (as Datum objects).
	deconstruct_array(vals, valsType, valsTypeWidth, valsTypeByValue, valsTypeAlignmentCode, &valsContent, &valsNullFlags, &valsLength);
	if (valsLength == 0)
		  CH_ERROR("no values in array");

    board = _piecesquares_to_board(str, valsContent, valsLength, valsNullFlags);
    pfree(str);
    PG_RETURN_POINTER(board);
}
/*}}}*/
/********************************************************
 * 		pfilter
 ********************************************************/
Datum/*{{{*/
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


/* +------+------+------+------+------+------+------+------+
 * | BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 |
 * +------+------+------+------+------+------+------+------+
 * |      Number of     | PAWN |KNIGHT| ROOK | QUEEN| KING |
 * |      ATTACKERS     |      |BISHOP|      |      |      |
 * +------+------+------+------+------+------+------+------+
 */

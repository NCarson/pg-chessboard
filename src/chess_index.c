
#include "chess_index.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

const piece_t           PIECE_INDEX_PIECES[] = {QUEEN, ROOK, BISHOP, KNIGHT, PAWN};
const int               PIECE_INDEX_COUNTS[] = {1, 2, 2, 2, 8};

const cpiece_t          WHITE_PIECES[] = {WHITE_QUEEN, WHITE_ROOK, WHITE_BISHOP, WHITE_KNIGHT, WHITE_PAWN};
const cpiece_t          BLACK_PIECES[] = {BLACK_QUEEN, BLACK_ROOK, BLACK_BISHOP, BLACK_KNIGHT, BLACK_PAWN};


/********************************************************
* 		old masks
********************************************************/
/*{{{*/
/*
// file a - h
const int64 File_Mask = {
       -9187201950435737472,
        4629771061636907072,
        2314885530818453536,
        1157442765409226768,
         578721382704613384,
         289360691352306692,
         144680345676153346,
          72340172838076673,
};

// rank 1 - 8
const int64 Rank_Mask = {
                        255,
                      65280,
                   16711680,
                 4278190080,
              1095216660480,
            280375465082880,
          71776119061217280,
         -72057594037927936,
};

// diagonal sw -> ne
const int64 Diagonal_Mask = {
                         128,
                       32832,
                     8405024,
                  2151686160,
                550831656968,
             141012904183812,
           36099303471055874,
        -9205322385119247871,
         4620710844295151872,
         2310355422147575808,
         1155177711073755136,
          577588855528488960,
          288794425616760832,
          144396663052566528,
           72057594037927936
}

// diagagnols nw -> se
// https://en.wikipedia.org/wiki/Anti-diagonal_matrix
const int64 ADiagonal_Mask = {
         -9223372036854775808,
          4647714815446351872,
          2323998145211531264,
          1161999622361579520,
           580999813328273408,
           290499906672525312,
           145249953336295424,
            72624976668147840,
              283691315109952,
                1108169199648,
                   4328785936,
                     16909320,
                        66052,
                          258,
                            1,
}
*/

/*}}}*/

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
unsigned int 
_sdbm_hash(char * str)
{
	unsigned long long hash = 0;
	int c;

	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash;
	return hash;
}

unsigned 
short _pindex_in(char * str)
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
}

/**
 * Ansi C "itoa" based on Kernighan & Ritchie's "Ansi C":
 */
	
static void 
strreverse(char* begin, char* end) {
	char aux;
	while(end>begin)
		aux=*end, *end--=*begin, *begin++=aux;
}
	
void 
ch_itoa(int value, char* str, int base) {
	
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr=str;
	int sign;

	// Validate base
	if (base<2 || base>35){ *wstr='\0'; return; }
	
	// Take care of sign
	if ((sign=value) < 0) value = -value;

	// Conversion. Number is reversed.
	do *wstr++ = num[value%base]; while(value/=base);
	if(sign<0) *wstr++='-';
	*wstr='\0';
	// Reverse string
	strreverse(str,wstr-1);
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
}/*}}}*/
/********************************************************
* 		piece
********************************************************/
/*{{{*/

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
* 		diagonals
********************************************************/
char _diagonal_in(char square)/*{{{*/
{
    char            d;

    switch (square+1) {
        case 1:
            d = -7; break;
        case 9: case 2:
            d = -6; break;
        case 17: case 10: case 3:
            d = -5; break;
        case 25: case 18: case 11: case 4:
            d = -4; break;
        case 33: case 26: case 19: case 12: case 5:
            d = -3; break;
        case 41: case 34: case 27: case 20: case 13: case 6:
            d = -2; break;
        case 49: case 42: case 35: case 28: case 21: case 14: case 7:
            d = -1; break;
        case 57: case 50: case 43: case 36: case 29: case 22: case 15: case 8:
            d = 0; break;
        case 58: case 51: case 44: case 37: case 30: case 23: case 16:
            d = 1; break;
        case 59: case 52: case 45: case 38: case 31: case 24:
            d = 2; break;
        case 60: case 53: case 46: case 39: case 32:
            d = 3; break;
        case 61: case 54: case 47: case 40:
            d = 4; break;
        case 62: case 55: case 48:
            d = 5; break;
        case 63: case 56:
            d = 6; break;
        case 64:
            d = 7; break;
        default:
            CH_ERROR("bad square %d for diagonal", square);
            break;
    }
    return d;
}

char _adiagonal_in(char square)
{
    char            d;

    switch (square+1) {
        case 57:
            d = -7; break;
        case 49: case 58:
            d = -6; break;
        case 41: case 50: case 59:
            d = -5; break;
        case 33: case 42: case 51: case 60:
            d = -4; break;
        case 25: case 34: case 43: case 52: case 61:
            d = -3; break;
        case 17: case 26: case 35: case 44: case 53: case 62:
            d = -2; break;
        case 9: case 18: case 27: case 36: case 45: case 54: case 63:
            d = -1; break;
        case 1: case 10: case 19: case 28: case 37: case 46: case 55: case 64:
            d = 0; break;
        case 2: case 11: case 20: case 29: case 38: case 47: case 56:
            d = 1; break;
        case 3: case 12: case 21: case 30: case 39: case 48:
            d = 2; break;
        case 4: case 13: case 22: case 31: case 40:
            d = 3; break;
        case 5: case 14: case 23: case 32:
            d = 4; break;
        case 6: case 15: case 24:
            d = 5; break;
        case 7: case 16:
            d = 6; break;
        case 8:
            d = 7; break;
        default:
            CH_ERROR("bad square %d for adiagonal", square);
            break;
    }
    return d;
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
}


/*}}}*/


int
_get_array_arg(PG_FUNCTION_ARGS, const size_t idx, Datum ** valsContent, bool ** valsNullFlags)
{
    //https://github.com/pjungwir/aggs_for_arrays/blob/master/array_to_max.c
	ArrayType 			*vals;            // Our arguments:
	Oid 				valsType;           // The array element type:
	int16 				valsTypeWidth;      // The array element type widths for our input array:
	bool 				valsTypeByValue;    // The array element type "is passed by value" flags (not really used):
	char 				valsTypeAlignmentCode; // The array element type alignment codes (not really used):
	int32               valsLength;         // The size of the input array:

	if (PG_ARGISNULL(idx)) { ereport(ERROR, (errmsg("Null arrays not accepted"))); } 
	vals = PG_GETARG_ARRAYTYPE_P(idx);
	if (ARR_NDIM(vals) == 0) { ereport(ERROR, (errmsg("array has zero dimensions"))); }
	if (ARR_NDIM(vals) > 1) { ereport(ERROR, (errmsg("One-dimesional arrays are required"))); }

	valsType = ARR_ELEMTYPE(vals);
	valsLength = (ARR_DIMS(vals))[0];
	get_typlenbyvalalign(valsType, &valsTypeWidth, &valsTypeByValue, &valsTypeAlignmentCode);
	deconstruct_array(vals, valsType, valsTypeWidth, valsTypeByValue, valsTypeAlignmentCode, valsContent, valsNullFlags, &valsLength);
    return valsLength;
}

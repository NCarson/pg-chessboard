
-- complain if script is sourced in psql, rather than via CREATE EXTENSION

--\echo Use "CREATE EXTENSION chess_index" to load this file. \quit

--set client_min_messages=DEBUG5;

--\set ON_ERROR_STOP;


/****************************************************************************
-- side : white or black
 ****************************************************************************/
/*{{{*/
CREATE FUNCTION side_in(cstring)
RETURNS side AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION side_out(side)
RETURNS cstring AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE side(
    INPUT          = side_in,
    OUTPUT         = side_out,

    LIKE           = char,
    INTERNALLENGTH = 1,     
    ALIGNMENT      = char,
    STORAGE        = PLAIN,
    PASSEDBYVALUE         
);
COMMENT ON TYPE side IS '
The side TO GO OR the side OF a piece.

Input format is ''w'' or ''b''. Uses 1 byte of storage
Supports =, <>, and hash operations.
```sql
SELECT pieces(''8/8/8/8/8/8/8/6Pp''::board, ''b''::side);
```
```
 pieces 
------------------------
 {ph1}
(1 row)
```
';

CREATE FUNCTION side_eq(side, side)
RETURNS bool LANGUAGE internal IMMUTABLE as 'chareq';
CREATE FUNCTION side_ne(side, side)
RETURNS bool LANGUAGE internal IMMUTABLE as 'charne';
CREATE FUNCTION "not"(side)
RETURNS side AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR = (
    LEFTARG = side,
    RIGHTARG = side,
    PROCEDURE = side_eq,
    COMMUTATOR = '=',
    NEGATOR = '<>',
    RESTRICT = eqsel,
    JOIN = eqjoinsel,
    HASHES, MERGES
);

CREATE OPERATOR <> (
    LEFTARG = side,
    RIGHTARG = side,
    PROCEDURE = side_ne,
    COMMUTATOR = '<>',
    NEGATOR = '=',
    RESTRICT = neqsel,
    JOIN = neqjoinsel
);
/*}}}*//*}}}*/
/****************************************************************************
-- pindex : piece index
 ****************************************************************************/
/*{{{*/
CREATE FUNCTION pindex_in(cstring)
RETURNS pindex AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION pindex_out(pindex)
RETURNS cstring AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE pindex(
     INPUT          = pindex_in
    ,OUTPUT         = pindex_out
    ,LIKE           = int2
);
COMMENT ON TYPE pindex IS '
**Ordered string of piece types.**

Uses 2 bytes of storage.
Supports =, <>, and hash operations.
';

CREATE FUNCTION pindex_to_int32(pindex)
RETURNS int4 AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (pindex as int4) WITH FUNCTION pindex_to_int32;

CREATE FUNCTION pindex_eq(pindex, pindex)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2eq';
CREATE FUNCTION pindex_ne(pindex, pindex)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2ne';
CREATE FUNCTION pindex_lt(pindex, pindex)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2lt';
CREATE FUNCTION pindex_le(pindex, pindex)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2le';
CREATE FUNCTION pindex_gt(pindex, pindex)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2gt';
CREATE FUNCTION pindex_ge(pindex, pindex)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2ge';
CREATE FUNCTION pindex_cmp(pindex, pindex)
RETURNS integer LANGUAGE internal IMMUTABLE AS 'btint2cmp';
CREATE FUNCTION hash_pindex(pindex)
RETURNS integer LANGUAGE internal IMMUTABLE AS 'hashint2';

CREATE OPERATOR = (
    LEFTARG = pindex,
    RIGHTARG = pindex,
    PROCEDURE = pindex_eq,
    COMMUTATOR = '=',
    NEGATOR = '<>',
    RESTRICT = eqsel,
    JOIN = eqjoinsel,
    HASHES, MERGES
);

CREATE OPERATOR <> (
    LEFTARG = pindex,
    RIGHTARG = pindex,
    PROCEDURE = pindex_ne,
    COMMUTATOR = '<>',
    NEGATOR = '=',
    RESTRICT = neqsel,
    JOIN = neqjoinsel
);

CREATE OPERATOR < (
  LEFTARG = pindex,
  RIGHTARG = pindex,
  PROCEDURE = pindex_lt,
  COMMUTATOR = > ,
  NEGATOR = >= ,
  RESTRICT = scalarltsel,
  JOIN = scalarltjoinsel
);

CREATE OPERATOR <= (
  LEFTARG = pindex,
  RIGHTARG = pindex,
  PROCEDURE = pindex_le,
  COMMUTATOR = >= ,
  NEGATOR = > ,
  RESTRICT = scalarltsel,
  JOIN = scalarltjoinsel
);

CREATE OPERATOR > (
  LEFTARG = pindex,
  RIGHTARG = pindex,
  PROCEDURE = pindex_gt,
  COMMUTATOR = < ,
  NEGATOR = <= ,
  RESTRICT = scalargtsel,
  JOIN = scalargtjoinsel
);

CREATE OPERATOR >= (
  LEFTARG = pindex,
  RIGHTARG = pindex,
  PROCEDURE = pindex_ge,
  COMMUTATOR = <= ,
  NEGATOR = < ,
  RESTRICT = scalargtsel,
  JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS btree_pindex_ops
DEFAULT FOR TYPE pindex USING btree
AS
        OPERATOR        1       <  ,
        OPERATOR        2       <= ,
        OPERATOR        3       =  ,
        OPERATOR        4       >= ,
        OPERATOR        5       >  ,
        FUNCTION        1       pindex_cmp(pindex, pindex);

CREATE OPERATOR CLASS hash_pindex_ops
DEFAULT FOR TYPE pindex USING hash AS
OPERATOR        1       = ,
FUNCTION        1       hash_pindex(pindex);

/*}}}*/
/****************************************************************************
-- square:
 ****************************************************************************/
/*{{{*/
CREATE FUNCTION square_in(cstring)
RETURNS square
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION square_out(square)
RETURNS cstring
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE square(
  INPUT          = square_in,
  OUTPUT         = square_out,
  LIKE           = char,
	INTERNALLENGTH = 1, 
	ALIGNMENT      = char,
	STORAGE        = PLAIN,
	PASSEDBYVALUE         
);
COMMENT ON TYPE square IS '
A square on the board.

The sort order is in fourth 
[quadrant](https://en.wikipedia.org/wiki/Quadrant_\(plane_geometry\))
where a8 is 0.
Input format is ''e4''.
Uses 1 byte of storage. Squares can be cast to chars and ints to work with the raw
number.  Supports =, <>, <, >, >=, <=, hash operations and btree operations.

Select pieces occupying square h1:
```sql
SELECT pieces(''8/8/8/8/8/8/8/6Pp''::board, ''h1''::square);
```
```
 pieces 
------------------------
 ph1
(1 row)
```
';

CREATE CAST ("char" AS square) WITHOUT FUNCTION;
CREATE CAST (square AS "char") WITHOUT FUNCTION;
CREATE FUNCTION square(int) RETURNS square AS $$ SELECT $1::"char"::square $$ LANGUAGE SQL IMMUTABLE STRICT;
CREATE FUNCTION "int"(square) RETURNS int AS $$ SELECT $1::"char"::int $$ LANGUAGE SQL IMMUTABLE STRICT;

CREATE CAST (int AS square) WITH FUNCTION square(int);
CREATE CAST (square AS int) WITH FUNCTION "int"(square);

CREATE FUNCTION square_eq(square, square)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'chareq';
CREATE FUNCTION square_ne(square, square)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'charne';
CREATE FUNCTION square_lt(square, square)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'charlt';
CREATE FUNCTION square_le(square, square)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'charle';
CREATE FUNCTION square_gt(square, square)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'chargt';
CREATE FUNCTION square_ge(square, square)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'charge';
CREATE FUNCTION square_cmp(square, square)
RETURNS integer LANGUAGE internal IMMUTABLE AS 'btcharcmp';
CREATE FUNCTION hash_square(square)
RETURNS integer LANGUAGE internal IMMUTABLE AS 'hashchar';

CREATE OPERATOR = (
  LEFTARG = square,
  RIGHTARG = square,
  PROCEDURE = square_eq,
  COMMUTATOR = '=',
  NEGATOR = '<>',
  RESTRICT = eqsel,
  JOIN = eqjoinsel,
  HASHES, MERGES
);

CREATE OPERATOR <> (
  LEFTARG = square,
  RIGHTARG = square,
  PROCEDURE = square_ne,
  COMMUTATOR = '<>',
  NEGATOR = '=',
  RESTRICT = neqsel,
  JOIN = neqjoinsel
);

CREATE OPERATOR < (
  LEFTARG = square,
  RIGHTARG = square,
  PROCEDURE = square_lt,
  COMMUTATOR = > ,
  NEGATOR = >= ,
  RESTRICT = scalarltsel,
  JOIN = scalarltjoinsel
);

CREATE OPERATOR <= (
  LEFTARG = square,
  RIGHTARG = square,
  PROCEDURE = square_le,
  COMMUTATOR = >= ,
  NEGATOR = > ,
  RESTRICT = scalarltsel,
  JOIN = scalarltjoinsel
);

CREATE OPERATOR > (
  LEFTARG = square,
  RIGHTARG = square,
  PROCEDURE = square_gt,
  COMMUTATOR = < ,
  NEGATOR = <= ,
  RESTRICT = scalargtsel,
  JOIN = scalargtjoinsel
);

CREATE OPERATOR >= (
  LEFTARG = square,
  RIGHTARG = square,
  PROCEDURE = square_ge,
  COMMUTATOR = <= ,
  NEGATOR = < ,
  RESTRICT = scalargtsel,
  JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS btree_square_ops
DEFAULT FOR TYPE square USING btree
AS
        OPERATOR        1       <  ,
        OPERATOR        2       <= ,
        OPERATOR        3       =  ,
        OPERATOR        4       >= ,
        OPERATOR        5       >  ,
        FUNCTION        1       square_cmp(square, square);

CREATE OPERATOR CLASS hash_square_ops
    DEFAULT FOR TYPE square USING hash AS
        OPERATOR        1       = ,
        FUNCTION        1       hash_square(square);

/*}}}*/
/****************************************************************************
-- piece
****************************************************************************/
/*{{{*/
CREATE FUNCTION piece_in(cstring)
RETURNS piece AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION piece_out(piece)
RETURNS cstring AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE piece(
    INPUT          = piece_in,
    OUTPUT         = piece_out,
    LIKE           = char,
    INTERNALLENGTH = 1,    
	ALIGNMENT      = char, 
	STORAGE        = PLAIN,
	PASSEDBYVALUE         
);
COMMENT ON TYPE piece IS '
Represents a chess piece without color.

Input format is ''P'' or ''p'' (case does not matter).
Uses 1 byte of storage. 
Supports =, <>, and hash operations.

```sql
SELECT pieces(''8/8/8/8/8/8/8/6Pp''::board, ''p''::piece);
```
```
  pieces   
---------------------------------
 {Pg1,ph1}
(1 row)
```
';

CREATE FUNCTION value(piece)
RETURNS int AS '$libdir/chess_index', 'piece_value' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION value(piece) IS '
Returns the scoring value of the piece.

values = {1,3,3,4,9,0} for {p,n,b,r,q,k}

```sql
SELECT value(''p''::piece);
```
```
 value 
---------------------
     1
    (1 row)
```
';

CREATE OR REPLACE FUNCTION pretty(piece)
RETURNS text AS $$
    SELECT translate($1::text, 'KQRBNP', U&'\2654\2655\2656\2657\2658\2659')
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pretty(piece) IS '
Returns the unicode character of the piece.

```sql
SELECT pretty(''p''::piece);
 pretty 
----------
 ♙
(1 row)
';


-----------------------------------------------------------------------------
-- ops
-----------------------------------------------------------------------------

CREATE FUNCTION piece_eq(piece, piece)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'chareq';
CREATE FUNCTION piece_ne(piece, piece)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'charne';
CREATE FUNCTION hash_square(piece)
RETURNS integer LANGUAGE internal IMMUTABLE AS 'hashchar';

CREATE OPERATOR = (
    LEFTARG = piece,
    RIGHTARG = piece,
    PROCEDURE = piece_eq,
    COMMUTATOR = '=',
    NEGATOR = '<>',
    RESTRICT = eqsel,
    JOIN = eqjoinsel,
    HASHES, MERGES
);

CREATE OPERATOR <> (
    LEFTARG = piece,
    RIGHTARG = piece,
    PROCEDURE = piece_ne,
    COMMUTATOR = '<>',
    NEGATOR = '=',
    RESTRICT = neqsel,
    JOIN = neqjoinsel
);

CREATE OPERATOR CLASS hash_piece_ops
DEFAULT FOR TYPE piece USING hash AS
OPERATOR        1       = ,
FUNCTION        1       hash_square(piece);

/*}}}*/
/****************************************************************************
-- cpiece
****************************************************************************/
/*{{{*/
CREATE FUNCTION cpiece_in(cstring)
RETURNS cpiece AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION cpiece_out(cpiece)
RETURNS cstring AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE cpiece(
    INPUT          = cpiece_in,
    OUTPUT         = cpiece_out,
    LIKE           = char,
    INTERNALLENGTH = 1,    
	ALIGNMENT      = char, 
	STORAGE        = PLAIN,
	PASSEDBYVALUE         
);
COMMENT ON TYPE cpiece IS '
Represents a chess piece with color.

Input format is ''P'' or ''p'' (capitalized pieces are white).
Uses 1 byte of storage. 
Supports =, <>, and hash operations.
Can be cast to piece.

```sql
SELECT pieces(''8/8/8/8/8/8/8/6Pp''::board, ''p''::cpiece);
```
```
 pieces 
------------------------
 {ph1}
(1 row)
```
';

CREATE FUNCTION value(cpiece)
RETURNS int AS '$libdir/chess_index', 'cpiece_value' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION value(cpiece) IS '
Returns the scoring value of the colored piece.

Black pieces will be have negative values.

values = {1,3,3,4,9,0} for {P,N,B,R,Q,K}
and      {1,3,3,4,9,0}\*-1 for {p,n,b,r,q,k}

```sql
 SELECT value(''p''::cpiece);
```
```
 value 
------------
    -1
    (1 row)
```
';

CREATE FUNCTION side(cpiece)
RETURNS side AS '$libdir/chess_index', 'cpiece_side' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION side(cpiece) IS '
Returns the side type of the piece.
```sql
SELECT side(''p''::cpiece);
```
```
 side 
---------
 b
(1 row)
```
';

CREATE FUNCTION piece(cpiece)
RETURNS piece AS '$libdir/chess_index', 'cpiece_to_piece' LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (cpiece as piece) WITH FUNCTION piece;

CREATE OR REPLACE FUNCTION pretty(cpiece)
RETURNS text AS $$
    SELECT translate($1::text, 'KQRBNPkqrbnp', U&'\2654\2655\2656\2657\2658\2659\265A\265B\265C\265D\265E\265F')
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pretty(cpiece) IS '
Returns the unicode character of the piece.

```sql
SELECT pretty(''p''::cpiece);
```
```
 pretty 
--------
 ♟
(1 row)
```
';


-----------------------------------------------------------------------------
-- ops
-----------------------------------------------------------------------------

CREATE FUNCTION cpiece_eq(cpiece, cpiece)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'chareq';
CREATE FUNCTION cpiece_ne(cpiece, cpiece)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'charne';
CREATE FUNCTION hash_square(cpiece)
RETURNS integer LANGUAGE internal IMMUTABLE AS 'hashchar';

CREATE OPERATOR = (
    LEFTARG = cpiece,
    RIGHTARG = cpiece,
    PROCEDURE = cpiece_eq,
    COMMUTATOR = '=',
    NEGATOR = '<>',
    RESTRICT = eqsel,
    JOIN = eqjoinsel,
    HASHES, MERGES
);

CREATE OPERATOR <> (
    LEFTARG = cpiece,
    RIGHTARG = cpiece,
    PROCEDURE = cpiece_ne,
    COMMUTATOR = '<>',
    NEGATOR = '=',
    RESTRICT = neqsel,
    JOIN = neqjoinsel
);

CREATE OPERATOR CLASS hash_cpiece_ops
DEFAULT FOR TYPE cpiece USING hash AS
OPERATOR        1       = ,
FUNCTION        1       hash_square(cpiece);


/*}}}*/
/****************************************************************************
-- pfilter
****************************************************************************/
/*{{{*/
CREATE FUNCTION pfilter_in(cstring)
RETURNS pfilter AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION pfilter_out(pfilter)
RETURNS cstring AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE pfilter(
     INPUT          = pfilter_in
    ,OUTPUT         = pfilter_out
    ,INTERNALLENGTH = 6
    ,ALIGNMENT      = char
    ,STORAGE        = PLAIN
);
COMMENT ON TYPE cpiece IS '
String of pieces to filter out pieces.

Uses 6 byte of storage. 
';
/*}}}*/
/****************************************************************************
-- piecesquare
****************************************************************************/
/*{{{*/
CREATE FUNCTION piecesquare_in(cstring)
RETURNS piecesquare AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION piecesquare_out(piecesquare)
RETURNS cstring AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE piecesquare(
     INPUT          = piecesquare_in
    ,OUTPUT         = piecesquare_out
    ,STORAGE        = PLAIN
    ,LIKE           = int2
    ,INTERNALLENGTH = 2    
	,ALIGNMENT      = int2  
	,PASSEDBYVALUE         
);
COMMENT ON TYPE piecesquare IS '
A colored piece occuping a square on the board.

Uses 2 bytes of storage. Input format is ''Ke4''.
Supports =, <>, <, >, >=, <=, hash operations and btree operations.
Can be cast to square, cpiece and int2.
';

CREATE CAST (piecesquare as int2) WITHOUT FUNCTION;
CREATE CAST (int2 as piecesquare) WITHOUT FUNCTION;

CREATE FUNCTION square(piecesquare)
RETURNS square AS '$libdir/chess_index', 'piecesquare_square' LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (piecesquare as square) WITH FUNCTION square(piecesquare);
COMMENT ON FUNCTION square(piecesquare) IS '
Casts piecesquare to square.
';

CREATE FUNCTION cpiece(piecesquare)
RETURNS cpiece AS '$libdir/chess_index', 'piecesquare_cpiece' LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (piecesquare as cpiece) WITH FUNCTION cpiece(piecesquare);
COMMENT ON FUNCTION cpiece(piecesquare) IS '
Casts piecesquare to cpiece.
';

CREATE OR REPLACE FUNCTION pretty(piecesquare)
RETURNS text AS $$
    SELECT pretty($1::cpiece) || $1::square::text
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pretty(piecesquare) IS '
Returns unicode chess symbol of piece. [sql]

```sql
 SELECT pretty(''pa2''::piecesquare);
```
```
 pretty 
--------
 ♟a2
(1 row)
```
';

CREATE OR REPLACE FUNCTION pretty(piecesquare[])
RETURNS text[] AS $$
    SELECT array_agg(pretty) from (SELECT pretty(unnest($1))) t
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pretty(piecesquare[]) IS '
Returns unicode chess symbol of pieces. [sql]
';

CREATE FUNCTION piecesquare_eq(piecesquare, piecesquare)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2eq';
CREATE FUNCTION piecesquare_ne(piecesquare, piecesquare)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2ne';
CREATE FUNCTION piecesquare_lt(piecesquare, piecesquare)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2lt';
CREATE FUNCTION piecesquare_le(piecesquare, piecesquare)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2le';
CREATE FUNCTION piecesquare_gt(piecesquare, piecesquare)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2gt';
CREATE FUNCTION piecesquare_ge(piecesquare, piecesquare)
RETURNS boolean LANGUAGE internal IMMUTABLE as 'int2ge';
CREATE FUNCTION piecesquare_cmp(piecesquare, piecesquare)
RETURNS integer LANGUAGE internal IMMUTABLE AS 'btint2cmp';
CREATE FUNCTION hash_piecesquare(piecesquare)
RETURNS integer LANGUAGE internal IMMUTABLE AS 'hashint2';

CREATE OPERATOR = (
  LEFTARG = piecesquare,
  RIGHTARG = piecesquare,
  PROCEDURE = piecesquare_eq,
  COMMUTATOR = '=',
  NEGATOR = '<>',
  RESTRICT = eqsel,
  JOIN = eqjoinsel,
  HASHES, MERGES
);

CREATE OPERATOR <> (
  LEFTARG = piecesquare,
  RIGHTARG = piecesquare,
  PROCEDURE = piecesquare_ne,
  COMMUTATOR = '<>',
  NEGATOR = '=',
  RESTRICT = neqsel,
  JOIN = neqjoinsel
);

CREATE OPERATOR < (
  LEFTARG = piecesquare,
  RIGHTARG = piecesquare,
  PROCEDURE = piecesquare_lt,
  COMMUTATOR = > ,
  NEGATOR = >= ,
  RESTRICT = scalarltsel,
  JOIN = scalarltjoinsel
);

CREATE OPERATOR <= (
  LEFTARG = piecesquare,
  RIGHTARG = piecesquare,
  PROCEDURE = piecesquare_le,
  COMMUTATOR = >= ,
  NEGATOR = > ,
  RESTRICT = scalarltsel,
  JOIN = scalarltjoinsel
);

CREATE OPERATOR > (
  LEFTARG = piecesquare,
  RIGHTARG = piecesquare,
  PROCEDURE = piecesquare_gt,
  COMMUTATOR = < ,
  NEGATOR = <= ,
  RESTRICT = scalargtsel,
  JOIN = scalargtjoinsel
);

CREATE OPERATOR >= (
  LEFTARG = piecesquare,
  RIGHTARG = piecesquare,
  PROCEDURE = piecesquare_ge,
  COMMUTATOR = <= ,
  NEGATOR = < ,
  RESTRICT = scalargtsel,
  JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS btree_piecesquare_ops
DEFAULT FOR TYPE piecesquare USING btree
AS
        OPERATOR        1       <  ,
        OPERATOR        2       <= ,
        OPERATOR        3       =  ,
        OPERATOR        4       >= ,
        OPERATOR        5       >  ,
        FUNCTION        1       piecesquare_cmp(piecesquare, piecesquare);


CREATE OPERATOR CLASS hash_piecesquare_ops
DEFAULT FOR TYPE piecesquare USING hash AS
OPERATOR        1       = ,
FUNCTION        1       hash_piecesquare(piecesquare);

/*}}}*/
/****************************************************************************
-- board: displays as fen, holds position
 ****************************************************************************/
/*{{{*/
CREATE FUNCTION board_in(cstring)
RETURNS board AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION board_out(board)
RETURNS cstring AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE board(
    INPUT          = board_in,
    OUTPUT         = board_out,
    STORAGE        = PLAIN
);
COMMENT ON TYPE board IS '
Represents a chess position.

A fast and space efficient chess board type
There three ways to initialize the board:
```sql
SELECT ''rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1''::board;
SELECT ''rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -''::board;
SELECT ''rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR''::board;
```
If the halfmove clock and move number are not present they will be set to zero
and not printed.  If the side, castling and en passant are not present, then
the board will be set to whites go and castling and en passant will be unset.

The size of the datatype is variable depending on the pieces. An empty board
takes up 16 bytes, where for every two pieces needs another byte. So the
starting position with 32 pieces in standard chess would take up another 16
bytes for a total of 32. Comparatively, the starting fen string stored as text
would take 56 bytes of storage.

Supports =, <>, <, >, >=, <=, hash operations and btree operations.

#### Equality

Equality and sorting operators do not take into account the halfmove clock and
the move number. If they did, the boards would have to be recreated with the
same move number to check for duplicate positions.  To create a unique index or
primary key with the move number you could: `CREATE INDEX idx_board ON
position (theboard, move(theboard)); `.

#### Notes on Implementation

Postgres requires a 4 byte size field (vl_len) to be the first member of a struct for
variable size data types. On a 64 bit machine this will create a 4 byte
[alignment hole](https://www.geeksforgeeks.org/structure-member-alignment-padding-and-data-packing/).
This is where board state such as en passant, move number, half move clock, etc. are kept.
Then an 8 byte bitboard or bitmap keeps the piece occupancy
\([see the second answer here](https://codegolf.stackexchange.com/questions/19397/smallest-chess-board-compression). 
The last field keeps the variable size piece nibbles.

But if you look at the code golf discussion you will see that 
[Huffman Encoding](https://www.geeksforgeeks.org/greedy-algorithms-set-3-huffman-coding/)
would lead to smaller sizes. With the smallest design listed at 160 bits or 20
bytes. That does not account for move and halfmove clock which need another 15
bits. We could have overloaded castling and en passant into the piece encoding to save
16 bits. piece count could be partially deduced from vl_len but we wont know if the last
nible is empty or not since the 192 bit solution uses all symbols. And all solutions seem
to ignore that you will need an EOF marker to see if your in 
[padding](https://www2.cs.duke.edu/csed/poop/huff/info/) or not. 
so all solutions need to add at least 4 bits.


Something to keep in mind also is that Postgres stores info on
[each row](https://stackoverflow.com/questions/13570613/making-sense-of-postgres-row-sizes)
of 23 btyes! So in Postgres the bare minimum *row size* would be:

```
      23 bytes HeapTupleHeader 
    +  1 byte for alignment
    +  4 bytes for vl_len
    + 20 bytes (160 bit huffman encoded board)

         +   9 bits move number (longest fide chess game 219 moves!)
         +   7 bits half move clock
         +   4 bits eof marker
    +  3 bytes for bits

    = 51 btyes
```

While 32 bytes data size + 24 tuple header = 56 bytes so about 10%
reduction in storage for maximum sized boards.

Also keep in mind operations on the pieces would require
O(n\*log(n)) to decompress the code. While this implimentation is O(n)
for all the pieces or O(1) for querying certain squares.

For simplicity and faster operations I think this is a pretty good way to
do it. lichess.org is producing 20 million games per month that you can
[download](https://database.lichess.org/). If an average game is about 30
moves with 60 positions for black and white moves then it would take
about 3k to store one game or 3GB per million games. Then a years worth of 
games would take 720 GB. Which are tractable numbers.

';

/*---------------------------------------/
/  pieces functions                      /
/---------------------------------------*/
/*{{{*/

CREATE FUNCTION _pieces(board)
RETURNS int2[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION _pieces(board, side)
RETURNS int2[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION _pieces_cpiece(board, cpiece)
RETURNS int2[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION _pieces_piece(board, piece)
RETURNS int2[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION _pieces_squares(board, square[])
RETURNS int2[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pieces(board)
RETURNS piecesquare[] AS $$
    SELECT "_pieces"($1)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board) IS 
'Returns an array of piecesquares occupying the board in fen order.
The sort order is in fourth
[quadrant](https://en.wikipedia.org/wiki/Quadrant_\(plane_geometry\))
where  a8 is 0. The pieces family of functions all have the same sort behavior except for pieces_so.

Search for pieces occupying a1 or are white kings:
```sql
    SELECT ps FROM 
    (
       SELECT unnest(pieces(''rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR''::board)) ps
    ) t 
   WHERE ps::square=''a1'' 
   OR ps::cpiece=''K'';
```
```
 ps  
---------------
 Ra1
 Ke1
(2 rows)
```
';

CREATE FUNCTION pieces(board, square)
RETURNS piecesquare AS '$libdir/chess_index', '_pieces_square' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board, square) IS 
'Returns a piecesquare given the square or null if there is no piece.';

CREATE OR REPLACE FUNCTION pieces(board, side)
RETURNS piecesquare[] AS $$
    SELECT "_pieces"($1, $2)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board, side) IS 
'Returns an array of piecesquares for the side.';

CREATE OR REPLACE FUNCTION pieces(board, cpiece)
RETURNS piecesquare[] AS $$
    SELECT "_pieces_cpiece"($1, $2)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board, cpiece) IS 
'Returns an array of piecesquares for the colored pieces that are present.';

CREATE OR REPLACE FUNCTION pieces(board, piece)
RETURNS piecesquare[] AS $$
    SELECT "_pieces_piece"($1, $2)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board, piece) IS 
'Returns an array of piecesquares for the pieces that are present.';

CREATE OR REPLACE FUNCTION pieces(board, square[])
RETURNS piecesquare[] AS $$
    SELECT "_pieces_squares"($1, $2)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board, square[]) IS 
'Returns an array of piecesquares given the squares that are occupied.';

CREATE CAST (board as piecesquare[]) WITH FUNCTION pieces(board);

CREATE OR REPLACE FUNCTION pieces_so(board) -- piecesquare order
RETURNS piecesquare[] AS $$
    SELECT 
        array_agg((p::text || s::text)::piecesquare)  
    from 
    (
        SELECT pieces::piecesquare s , pieces::cpiece p from 
        (
            SELECT unnest(pieces($1)) pieces
        ) tt order by s
    ) ttt ;
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces_so(board) IS 
'Returns an array of piecesquares in square order [sql].
The sort order is in first
[quadrant](https://en.wikipedia.org/wiki/Quadrant_\(plane_geometry\))
where a1 is 0.
';

CREATE FUNCTION _attacks(board)
RETURNS int2[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION attacks(board)
RETURNS piecesquare[] AS $$
    SELECT "_attacks"($1)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;

CREATE FUNCTION _mobility(board)
RETURNS int2[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION mobility(board)
RETURNS piecesquare[] AS $$
    SELECT "_mobility"($1)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
/*}}}*/

/*---------------------------------------/
/  functions                             /
/---------------------------------------*/
/*{{{*/
CREATE FUNCTION footer(board)
RETURNS cstring AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION footer(board) IS 
'Returns the the fen string after the first board field.';

CREATE FUNCTION pcount(board)
RETURNS int AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION pcount(board) IS 
'Returns aumount of pieces on the board.';

CREATE FUNCTION pcount(board, piece)
RETURNS int AS '$libdir/chess_index', 'pcount_piece' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION pcount(board, piece) IS 
'Returns aumount of pieces on the board of a certain type piece.';

CREATE FUNCTION pcount(board, cpiece)
RETURNS int AS '$libdir/chess_index', 'pcount_cpiece' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION pcount(board, cpiece) IS 
'Returns aumount of pieces on the board of a certain type cpiece.';

CREATE FUNCTION side(board)
RETURNS side AS '$libdir/chess_index', 'board_side' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION side(board) IS 
'Returns the side which has the go.';

CREATE FUNCTION move(board)
RETURNS int AS '$libdir/chess_index', 'board_move' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION move(board) IS 
'Returns the move number.
It will be zero if it was not set in the fen';

CREATE FUNCTION halfmove(board)
RETURNS int AS '$libdir/chess_index', 'board_halfmove' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION halfmove(board) IS 
'Returns the halfmove clock number.
It will be zero if it was not set in the fen';

CREATE FUNCTION score(board)
RETURNS int AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION score(board) IS 
'Returns the sum of the piece values.
Sums the piece values using {1,3,3,5,9} for {P,B,N,R,Q} respectivly with negative values for the black pieces.';

CREATE FUNCTION pieceindex(board, side)
RETURNS pindex AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION pieceindex(board, side) IS 
'Returns the pieceindex given a side.';

CREATE FUNCTION board(piecesquare[], text)
RETURNS board AS '$libdir/chess_index', 'piecesquares_to_board' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION board(piecesquare[], text) IS 
'Returns a board given the pieces and the footer part of the fen string.';

CREATE FUNCTION board(piecesquare[])
RETURNS board AS $$ 
    SELECT board($1, 'w - -'::text)
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION board(piecesquare[]) IS 
'Returns a board given the pieces with initial state.';

CREATE CAST (piecesquare[] as board) WITH FUNCTION board(piecesquare[]);

CREATE FUNCTION pfilter(board, pfilter)
RETURNS board AS '$libdir/chess_index', 'remove_pieces' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION pfilter(board, pfilter) IS 
'Returns a board given the pfilter.';

CREATE FUNCTION heatmap(board)
RETURNS cstring AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION bitboard(board, cpiece)
RETURNS bit(64) AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION bitboard(board, cpiece) IS 
'Returns a 64 bit string with 1''s representing the occupancy of the piece in fen order.';

CREATE FUNCTION bitboard_array(board, cpiece)
RETURNS bit[] AS $$
    SELECT string_to_array(bitboard($1, $2)::text, NULL)::bit[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION bitboard_array(board, cpiece) IS 
'Returns an array of size 64 with 1''s representing the occupancy of the piece in fen order. (sql)';

CREATE FUNCTION int_array(board)
RETURNS int[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION int_array(board) IS 
'Returns an array of size 64 with unique integers representing the occupancy of the colored piece in fen order.

Intergers values: {P=1,N=2,B=3,R=4, Q=5, K=7, p=8, n=9, b=10, r=11, q=12, k=13}

Useful for external machine learning and statical analysis.
';

/*}}}*/
/*---------------------------------------/
/  ops                                   /
/---------------------------------------*/
/*{{{*/
CREATE FUNCTION board_eq(board, board)
RETURNS boolean AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION board_ne(board, board)
RETURNS boolean AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION board_lt(board, board)
RETURNS boolean AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION board_le(board, board)
RETURNS boolean AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION board_gt(board, board)
RETURNS boolean AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION board_ge(board, board)
RETURNS boolean AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION board_cmp(board, board)
RETURNS int AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION board_hash(board)
RETURNS int AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR = (
    LEFTARG = board,
    RIGHTARG = board,
    PROCEDURE = board_eq,
    COMMUTATOR = '=',
    NEGATOR = '<>',
    RESTRICT = eqsel,
    JOIN = eqjoinsel,
    HASHES, MERGES
);

CREATE OPERATOR <> (
    LEFTARG = board,
    RIGHTARG = board,
    PROCEDURE = board_ne,
    COMMUTATOR = '<>',
    NEGATOR = '=',
    RESTRICT = neqsel,
    JOIN = neqjoinsel
);

CREATE OPERATOR < (
    LEFTARG = board,
    RIGHTARG = board,
    PROCEDURE = board_lt,
    COMMUTATOR = > ,
    NEGATOR = >= ,
    RESTRICT = scalarltsel,
    JOIN = scalarltjoinsel
);

CREATE OPERATOR <= (
    LEFTARG = board,
    RIGHTARG = board,
    PROCEDURE = board_le,
    COMMUTATOR = >= ,
    NEGATOR = > ,
    RESTRICT = scalarltsel,
    JOIN = scalarltjoinsel
);

CREATE OPERATOR > (
    LEFTARG = board,
    RIGHTARG = board,
    PROCEDURE = board_gt,
    COMMUTATOR = < ,
    NEGATOR = <= ,
    RESTRICT = scalargtsel,
    JOIN = scalargtjoinsel
);

CREATE OPERATOR >= (
    LEFTARG = board,
    RIGHTARG = board,
    PROCEDURE = board_ge,
    COMMUTATOR = <= ,
    NEGATOR = < ,
    RESTRICT = scalargtsel,
    JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS btree_board_ops
DEFAULT FOR TYPE board USING btree
AS
OPERATOR        1       <  ,
OPERATOR        2       <= ,
OPERATOR        3       =  ,
OPERATOR        4       >= ,
OPERATOR        5       >  ,
FUNCTION        1       board_cmp(board, board);

CREATE OPERATOR CLASS hash_board_ops
DEFAULT FOR TYPE board USING hash AS
OPERATOR        1       = ,
FUNCTION        1       board_hash(board);/*}}}*/
/*}}}*/
/****************************************************************************
-- file
 ****************************************************************************/
/*{{{*/
CREATE FUNCTION cfile_in(cstring)
RETURNS cfile 
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION cfile_out(cfile)
RETURNS cstring
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE cfile(
    INPUT          = cfile_in,
    OUTPUT         = cfile_out,

    INTERNALLENGTH = 1,     
    ALIGNMENT      = char, 
    STORAGE        = PLAIN, -- always store data inline uncompressed (not toasted)
    PASSEDBYVALUE           -- pass data by value rather than by reference
);
COMMENT ON TYPE cfile IS '
Represents a chess board file a-h.

Input format is ''a''.
Uses 1 byte of storage.
Can be cast to an int where ''a''=0.
';

CREATE FUNCTION square_to_cfile(square)
RETURNS cfile
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (square AS cfile) WITH FUNCTION square_to_cfile(square);

CREATE FUNCTION char_to_int(cfile)
RETURNS int4
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (cfile AS int4) WITH FUNCTION char_to_int(cfile);

/*}}}*/
/****************************************************************************
-- rank
 ****************************************************************************/
/*{{{*/
CREATE FUNCTION rank_in(cstring)
RETURNS rank
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION rank_out(rank)
RETURNS cstring
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE rank(
    INPUT          = rank_in,
    OUTPUT         = rank_out,

    INTERNALLENGTH = 1,     -- use 4 bytes to store data
    ALIGNMENT      = char,  -- align to 4 bytes
    STORAGE        = PLAIN, -- always store data inline uncompressed (not toasted)
    PASSEDBYVALUE           -- pass data by value rather than by reference
);
COMMENT ON TYPE "rank" IS '
Represents a chess rank file 1-8.

Input format is ''1''.
Uses 1 byte of storage.
Can be cast to an int where ''1''=0.
';


CREATE FUNCTION square_to_rank(square)
RETURNS rank
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (square AS rank) WITH FUNCTION square_to_rank(square);

CREATE FUNCTION char_to_int(rank)
RETURNS int4
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (rank AS int4) WITH FUNCTION char_to_int(rank);
/*}}}*/
/****************************************************************************
-- diagonal
 ****************************************************************************/
/*{{{*/
CREATE FUNCTION diagonal_in(cstring)
RETURNS diagonal
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION diagonal_out(diagonal)
RETURNS cstring
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE diagonal(
    INPUT          = diagonal_in,
    OUTPUT         = diagonal_out,

    INTERNALLENGTH = 1,     -- use 4 bytes to store data
    ALIGNMENT      = char,  -- align to 4 bytes
    STORAGE        = PLAIN, -- always store data inline uncompressed (not toasted)
    PASSEDBYVALUE           -- pass data by value rather than by reference
);
COMMENT ON TYPE diagonal IS '
Represents a diagonal on the board.

Uses 1 byte of storage.
TODO
';

CREATE FUNCTION square_to_diagonal(square)
RETURNS diagonal
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (square AS diagonal) WITH FUNCTION square_to_diagonal(square);

CREATE FUNCTION char_to_int(diagonal)
RETURNS int4
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (diagonal AS int4) WITH FUNCTION char_to_int(diagonal);
/*}}}*/
/****************************************************************************
-- adiagonal
 ****************************************************************************/
/*{{{*/
CREATE FUNCTION adiagonal_in(cstring)
RETURNS adiagonal
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION adiagonal_out(adiagonal)
RETURNS cstring
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE adiagonal(
    INPUT          = adiagonal_in,
    OUTPUT         = adiagonal_out,

    INTERNALLENGTH = 1,     -- use 4 bytes to store data
    ALIGNMENT      = char,  -- align to 4 bytes
    STORAGE        = PLAIN, -- always store data inline uncompressed (not toasted)
    PASSEDBYVALUE           -- pass data by value rather than by reference
);
COMMENT ON TYPE adiagonal IS '
Represents a anti-diagonal on the board.

https://en.wikipedia.org/wiki/Anti-diagonal_matrix
Uses 1 byte of storage.
TODO
';

CREATE FUNCTION square_to_adiagonal(square)
RETURNS adiagonal
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (square AS adiagonal) WITH FUNCTION square_to_adiagonal(square);

CREATE FUNCTION char_to_int(adiagonal)
RETURNS int4
AS '$libdir/chess_index'
LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (adiagonal AS int4) WITH FUNCTION char_to_int(adiagonal);
/*}}}*/
/****************************************************************************
-- sql functions
 ****************************************************************************/
/*{{{*/

CREATE OR REPLACE FUNCTION start_board()
RETURNS board AS $$
    SELECT 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION start_board() IS '
Returns initial position of standard chess. [sql]

```sql
chess_test=# select pretty(start_board());
```
```
           pretty           
----------------------------
 rnbqkbnr                  +
 pppppppp                  +
 ........                  +
 ........                  +
 ........                  +
 ........                  +
 PPPPPPPP                  +
 RNBQKBNR  w  KQkq  -  0  1+
                           +
 
(1 row)
```
';

CREATE OR REPLACE FUNCTION empty_board()
RETURNS board AS $$
    SELECT '8/8/8/8/8/8/8/8'::board
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION empty_board() IS '
Returns a board with no pieces. [sql]

```sql
select pretty(empty_board());
```
```
        pretty         
-----------------------
 ........             +
 ........             +
 ........             +
 ........             +
 ........             +
 ........             +
 ........             +
 ........  w  -  -    +
                      +
 
(1 row)
```
';


CREATE OR REPLACE FUNCTION pretty(text, uni bool default false, showfen bool default false)
RETURNS text AS $$
    SELECT replace(replace(replace(replace(replace(replace(replace(replace(
            translate
            (
                 split_part($1, ' ', 1)
                , case when $2 then '/KQRBNPkqrbnp' else '/' end
                , case when $2
                    then E'\n' || U&'\2654\2655\2656\2657\2658\2659\265A\265B\265C\265D\265E\265F'
                    else E'\n'  
                  end
            )
            , '8', case when not $2 then '........' else U&'.\200A.\200A.\200A.\200A.\200A.\200A.\200A.\200A' end) 
            , '7', case when not $2 then '.......' else U&'.\200A.\200A.\200A.\200A.\200A.\200A.\200A' end) 
            , '6', case when not $2 then '......' else U&'.\200A.\200A.\200A.\200A.\200A.\200A' end) 
            , '5', case when not $2 then '.....' else U&'.\200A.\200A.\200A.\200A.\200A' end) 
            , '4', case when not $2 then '....' else U&'.\200A.\200A.\200A.\200A' end) 
            , '3', case when not $2 then '...' else U&'.\200A.\200A.\200A' end) 
            , '2', case when not $2 then '..' else U&'.\200A.\200A' end) 
            , '1', case when not $2 then '.' else U&'.\200A' end) 

        || '  ' || split_part($1, ' ', 2)
        || '  ' || split_part($1, ' ', 3)
        || '  ' || split_part($1, ' ', 4)
        || '  ' || split_part($1, ' ', 5)
        || '  ' || split_part($1, ' ', 6)
        || case when $3 then E'\n' || split_part($1::text, ' ', 1) else '' end
        || E'\n\n'
        
$$ LANGUAGE SQL IMMUTABLE STRICT;

COMMENT ON FUNCTION pretty(text, bool, bool) IS '
Converts fen string to a printable board. [sql]

If uni is true then use unicode.
If showfen is true add the fen string at the bottom of the board.
';

CREATE OR REPLACE FUNCTION pretty(board, uni bool default false, showfen bool default false)
RETURNS text AS $$
SELECT pretty($1::text, $2, $3)
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pretty(board, bool, bool) IS '
Converts fen string to a printable board. [sql]

if *uni* is true then use unicode
if *showfen* is true add the fen string at the bottom of the board
';

CREATE OR REPLACE FUNCTION invert(board)
RETURNS board AS $$
select REPLACE(TRANSLATE($1::text, 'pnbrqkPNBRQKw', 'PNBRQKpnbrqkb'), ' B ', ' w ')::board
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION invert(board) IS '
Reverses fen string, colors, and side to move. [sql]

A white piece on a1 will become a black piece on h8.
With this method we can treat all positions with
white now meaning: *the side with the move* 
\(Classifying Chess Positions, De Sa, 2012\).

Black''s go after 1. e4:
```sql
SELECT pretty(invert(''rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1''));
```
```
           pretty            
        -----------------------------
         RNBQKBNR                   +
         PPPPPPPP                   +
         ........                   +
         ........                   +
         ....p...                   +
         ........                   +
         pppp.ppp                   +
         rnbqkbnr  w  KQkq  e3  0  1+
```

`TODO: handle en passant.`
';

CREATE OR REPLACE FUNCTION squares()
RETURNS setof square AS $$
     SELECT (56 - (i/8)*8 + (i%8))::square  from generate_series(0, 63) as i;
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pretty(board, bool, bool) IS '
Generates a set of squares in fen order.';


CREATE OR REPLACE FUNCTION white(piece)
RETURNS cpiece AS
$$
    SELECT upper($1::piece::text)::cpiece;

$$ LANGUAGE SQL STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION white(piece[])
RETURNS cpiece[] AS
$$
    SELECT array_agg(white) from 
    (
        SELECT white(unnest) from (SELECT unnest($1))t 
    )tt

$$ LANGUAGE SQL STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION black(piece)
RETURNS cpiece AS
$$
    SELECT lower($1::piece::text)::cpiece;

$$ LANGUAGE SQL STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION black(piece[])
RETURNS cpiece[] AS
$$
    SELECT array_agg(black) from 
    (
        SELECT black(unnest) from (SELECT unnest($1))t 
    )tt

$$ LANGUAGE SQL STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bitarray(bit(64))
RETURNS bit[] AS $$
    SELECT string_to_array(($1)::text, NULL)::bit[]
$$ LANGUAGE SQL IMMUTABLE STRICT;


/*}}}*/

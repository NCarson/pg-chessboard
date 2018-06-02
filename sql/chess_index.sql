
-- complain if script is sourced in psql, rather than via CREATE EXTENSION

--\echo Use "CREATE EXTENSION chess_index" to load this file. \quit

--\set client_min_messages=DEBUG5;


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

CREATE FUNCTION value(piece)
RETURNS int AS '$libdir/chess_index', 'piece_value' LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pretty(piece)
RETURNS text AS $$
    select translate($1::text, 'KQRBNP', U&'\265A\265B\265C\265D\265E\265F')
$$ LANGUAGE SQL IMMUTABLE STRICT;


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

CREATE FUNCTION value(cpiece)
RETURNS int AS '$libdir/chess_index', 'cpiece_value' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION side(cpiece)
RETURNS side AS '$libdir/chess_index', 'cpiece_side' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION piece(cpiece)
RETURNS piece AS '$libdir/chess_index', 'cpiece_to_piece' LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (cpiece as piece) WITH FUNCTION piece;

CREATE OR REPLACE FUNCTION pretty(cpiece)
RETURNS text AS $$
    select translate($1::text, 'KQRBNPkqrbnp', U&'\2654\2655\2656\2657\2658\2659\265A\265B\265C\265D\265E\265F')
$$ LANGUAGE SQL IMMUTABLE STRICT;


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
);/*}}}*/
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

CREATE CAST (piecesquare as int2) WITHOUT FUNCTION;
CREATE CAST (int2 as piecesquare) WITHOUT FUNCTION;

CREATE FUNCTION square(piecesquare)
RETURNS square AS '$libdir/chess_index', 'piecesquare_square' LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (piecesquare as square) WITH FUNCTION square(piecesquare);

CREATE FUNCTION cpiece(piecesquare)
RETURNS cpiece AS '$libdir/chess_index', 'piecesquare_cpiece' LANGUAGE C IMMUTABLE STRICT;
CREATE CAST (piecesquare as cpiece) WITH FUNCTION cpiece(piecesquare);

CREATE OR REPLACE FUNCTION pretty(piecesquare)
RETURNS text AS $$
    select pretty($1::cpiece) || $1::square::text
$$ LANGUAGE SQL IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pretty(piecesquare[])
RETURNS text[] AS $$
    select array_agg(pretty) from (select pretty(unnest($1))) t
$$ LANGUAGE SQL IMMUTABLE STRICT;

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
COMMENT ON TYPE board IS 
'### Represents a chess position.

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

Equality and sorting operators do not take into account the halfmove clock and
the move number. If they did, the boards would have to be recreated with the
same move number to check for duplicate positions.  To create a unique index or
primary key with the move number you would: `sql CREATE INDEX idx_board ON
films (theboard, move(theboard)); `.

The size of the datatype is variable depending on the pieces. An empty board
takes up 16 bytes, where for every two pieces needs another byte. So the
starting position with 32 pieces in standard chess would take up another 16
bytes for a total of 32. Comparatively, the starting fen string stored as text
would take 56 bytes of storage.



';

;
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
    select "_pieces"($1)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board) IS 
'Returns an array of piecesquares occupying the board in fen order.
The sort order is in fourth quadrant where  a8 is 0. The pieces family of functions all have the same sort behavior except for pieces_so.
https://en.wikipedia.org/wiki/Quadrant_(plane_geometry)

Search for pieces occupying a1 or are black kings:
```sql
    SELECT ps FROM 
    (
       select unnest(pieces(''rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR''::board)) ps
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
    select "_pieces"($1, $2)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board, side) IS 
'Returns an array of piecesquares for the side.';

CREATE OR REPLACE FUNCTION pieces(board, cpiece)
RETURNS piecesquare[] AS $$
    select "_pieces_cpiece"($1, $2)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board, cpiece) IS 
'Returns an array of piecesquares for the colored pieces that are present.';

CREATE OR REPLACE FUNCTION pieces(board, piece)
RETURNS piecesquare[] AS $$
    select "_pieces_piece"($1, $2)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board, piece) IS 
'Returns an array of piecesquares for the pieces that are present.';

CREATE OR REPLACE FUNCTION pieces(board, square[])
RETURNS piecesquare[] AS $$
    select "_pieces_squares"($1, $2)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces(board, square[]) IS 
'Returns an array of piecesquares given the squares that are occupied.';

CREATE CAST (board as piecesquare[]) WITH FUNCTION pieces(board);

CREATE OR REPLACE FUNCTION pieces_so(board) -- piecesquare order
RETURNS piecesquare[] AS $$
    select 
        array_agg((p::text || s::text)::piecesquare)  
    from 
    (
        select pieces::piecesquare s , pieces::cpiece p from 
        (
            select unnest(pieces($1)) pieces
        ) tt order by s
    ) ttt ;
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION pieces_so(board) IS 
'Returns an array of piecesquares in square order [sql].
The sort order is in first quadrant where a1 is 0.
https://en.wikipedia.org/wiki/Quadrant_(plane_geometry)';

CREATE FUNCTION _attacks(board)
RETURNS int2[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION attacks(board)
RETURNS piecesquare[] AS $$
    select "_attacks"($1)::piecesquare[]
$$ LANGUAGE SQL IMMUTABLE STRICT;

CREATE FUNCTION _mobility(board)
RETURNS int2[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION mobility(board)
RETURNS piecesquare[] AS $$
    select "_mobility"($1)::piecesquare[]
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
'Returns a 64 bit string with 1''s representing the occupancy of the piece.';

CREATE FUNCTION bitboard_array(board, cpiece)
RETURNS bit[] AS $$
    select string_to_array(bitboard($1, $2)::text, NULL)::bit[]
$$ LANGUAGE SQL IMMUTABLE STRICT;
COMMENT ON FUNCTION bitboard_array(board, cpiece) IS 
'Returns a 64 bit array with 1''s representing the occupancy of the piece. (sql)';

CREATE FUNCTION int_array(board)
RETURNS int[] AS '$libdir/chess_index' LANGUAGE C IMMUTABLE STRICT;
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
CREATE OR REPLACE FUNCTION pretty(text, uni bool default false, showfen bool default true)
RETURNS text AS $$
    select replace(replace(replace(replace(replace(replace(replace(replace(
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
        || '  '  || split_part($1, ' ', 3)
        || '  '  || split_part($1, ' ', 4)
        || '  '  || split_part($1, ' ', 5)
        || case when $3 then E'\n' || split_part($1::text, ' ', 1) else '' end
        || E'\n\n'
        
    ;
$$ LANGUAGE SQL IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pretty(board, uni bool default false, showfen bool default true)
RETURNS text AS $$
select pretty($1::text, $2, $3)
$$ LANGUAGE SQL IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION invert(board)
RETURNS text AS $$
    select translate($1::text, 'KQRBNPkqrbnpwb', 'kqrbnpKQRBNPbw')
$$ LANGUAGE SQL IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION squares()
RETURNS setof square AS $$
     select (56 - (i/8)*8 + (i%8))::square  from generate_series(0, 63) as i;
$$ LANGUAGE SQL IMMUTABLE STRICT;


CREATE OR REPLACE FUNCTION white(piece)
RETURNS cpiece AS
$$
    SELECT upper($1::piece::text)::cpiece;

$$ LANGUAGE SQL STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION white(piece[])
RETURNS cpiece[] AS
$$
    select array_agg(white) from 
    (
        select white(unnest) from (select unnest($1))t 
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
    select array_agg(black) from 
    (
        select black(unnest) from (select unnest($1))t 
    )tt

$$ LANGUAGE SQL STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bitarray(bit(64))
RETURNS bit[] AS $$
    select string_to_array(($1)::text, NULL)::bit[]
$$ LANGUAGE SQL IMMUTABLE STRICT;


/*}}}*/

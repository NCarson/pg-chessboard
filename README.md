
# pg-chessboard

## Fast, Compact Types for Chess Board Positions

```sql
select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board;

board                           
--------------------------------------------------------
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
(1 row)
```
#### footer(board) -> cstring

Returns the the fen string after the first board field.

#### halfmove(board) -> integer

Returns the halfmove clock number.
It will be zero if it was not set in the fen

#### move(board) -> integer

Returns the move number.
It will be zero if it was not set in the fen

#### pcount(board) -> integer

Returns aumount of pieces on the board.

#### pieces(board) -> piecesquare[]

Returns an array of piecesquares occupying the board in fen order.
The sort order is in fourth quadrant where  a8 is 0. The pieces family of functions all have the same sort behavior except for pieces_so.
https://en.wikipedia.org/wiki/Quadrant_(plane_geometry)

Search for pieces occupying a1 or are black kings:
```sql
    SELECT ps FROM 
    (
       select unnest(pieces('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR'::board)) ps
    ) t 
   WHERE ps::square='a1' 
   OR ps::cpiece='K';
```
```
 ps  
---------------
 Ra1
 Ke1
(2 rows)
```


#### pieces_so(board) -> piecesquare[]

Returns an array of piecesquares in square order [sql].
The sort order is in first quadrant where a1 is 0.
https://en.wikipedia.org/wiki/Quadrant_(plane_geometry)

#### score(board) -> integer

Returns the sum of the piece values.
Sums the piece values using {1,3,3,5,9} for {P,B,N,R,Q} respectivly with negative values for the black pieces.

#### side(board) -> side

Returns the side which has the go.

#### bitboard(board, cpiece) -> bit

Returns a 64 bit string with 1's representing the occupancy of the piece.

#### bitboard_array(board, cpiece) -> bit[]

Returns a 64 bit array with 1's representing the occupancy of the piece. (sql)

#### pcount(board, cpiece) -> integer

Returns aumount of pieces on the board of a certain type cpiece.

#### pieces(board, cpiece) -> piecesquare[]

Returns an array of piecesquares for the colored pieces that are present.

#### pfilter(board, pfilter) -> board

Returns a board given the pfilter.

#### pcount(board, piece) -> integer

Returns aumount of pieces on the board of a certain type piece.

#### pieces(board, piece) -> piecesquare[]

Returns an array of piecesquares for the pieces that are present.

#### pieceindex(board, side) -> pindex

Returns the pieceindex given a side.

#### pieces(board, side) -> piecesquare[]

Returns an array of piecesquares for the side.

#### pieces(board, square) -> piecesquare

Returns a piecesquare given the square or null if there is no piece.

#### pieces(board, square[]) -> piecesquare[]

Returns an array of piecesquares given the squares that are occupied.

#### cube_cmp(cube, cube) -> integer

btree comparison function

#### cube_contained(cube, cube) -> boolean

contained in

#### cube_contains(cube, cube) -> boolean

contains

#### cube_eq(cube, cube) -> boolean

same as

#### cube_ge(cube, cube) -> boolean

greater than or equal to

#### cube_gt(cube, cube) -> boolean

greater than

#### cube_le(cube, cube) -> boolean

lower than or equal to

#### cube_lt(cube, cube) -> boolean

lower than

#### cube_ne(cube, cube) -> boolean

different

#### cube_overlap(cube, cube) -> boolean

overlaps

#### board(piecesquare[]) -> board

Returns a board given the pieces with initial state.

#### board(piecesquare[], text) -> board

Returns a board given the pieces and the footer part of the fen string.


# pg-chessboard

## Fast, Compact Types for Chess Board Positions

```sql
select pretty('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board);```
```
                   pretty                    
---------------------------------------------
 rnbqkbnr                                   +
 pppppppp                                   +
 ........                                   +
 ........                                   +
 ........                                   +
 ........                                   +
 PPPPPPPP                                   +
 RNBQKBNR  w  KQkq  -  0                    +
 rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR+
                                            +
(1 row)
``


## Types
#### adiagonal

TODO

#### board

### Represents a chess position.

A fast and space efficient chess board type
There three ways to initialize the board:
```sql
SELECT 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board;
SELECT 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -'::board;
SELECT 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR'::board;
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





#### cfile

TODO

#### cpiece

TODO

#### cube

multi-dimensional cube '(FLOAT-1, FLOAT-2, ..., FLOAT-N), (FLOAT-1, FLOAT-2, ..., FLOAT-N)'

#### diagonal

TODO

#### gameresult

TODO

#### gsmlsign

TODO

#### pfilter

TODO

#### piece

TODO

#### piecesquare

TODO

#### pindex

TODO

#### rank

TODO

#### side

TODO

#### square

TODO

#### t_search_result

TODO
## Functions
#### bitboard(board, cpiece) -> bit

Returns a 64 bit string with 1's representing the occupancy of the piece.

#### bitboard_array(board, cpiece) -> bit[]

Returns a 64 bit array with 1's representing the occupancy of the piece. (sql)

#### board(piecesquare[]) -> board

Returns a board given the pieces with initial state.

#### board(piecesquare[], text) -> board

Returns a board given the pieces and the footer part of the fen string.

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

#### pcount(board, cpiece) -> integer

Returns aumount of pieces on the board of a certain type cpiece.

#### pcount(board, piece) -> integer

Returns aumount of pieces on the board of a certain type piece.

#### pfilter(board, pfilter) -> board

Returns a board given the pfilter.

#### pieceindex(board, side) -> pindex

Returns the pieceindex given a side.

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


#### pieces(board, cpiece) -> piecesquare[]

Returns an array of piecesquares for the colored pieces that are present.

#### pieces(board, piece) -> piecesquare[]

Returns an array of piecesquares for the pieces that are present.

#### pieces(board, side) -> piecesquare[]

Returns an array of piecesquares for the side.

#### pieces(board, square) -> piecesquare

Returns a piecesquare given the square or null if there is no piece.

#### pieces(board, square[]) -> piecesquare[]

Returns an array of piecesquares given the squares that are occupied.

#### pieces_so(board) -> piecesquare[]

Returns an array of piecesquares in square order [sql].
The sort order is in first quadrant where a1 is 0.
https://en.wikipedia.org/wiki/Quadrant_(plane_geometry)

#### score(board) -> integer

Returns the sum of the piece values.
Sums the piece values using {1,3,3,5,9} for {P,B,N,R,Q} respectivly with negative values for the black pieces.

#### side(board) -> side

Returns the side which has the go.


# pg-chessboard

## Fast, Compact Types for Chess Board Positions

```sql
select pretty('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board);
```
```                  pretty                    
 --------------------------------------------
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
```

pg-chessboard is an extension for Postgres. https://github.com/theory/pgtap has
good general instructions on how to install an extension for Postgres. It does 
not generate moves or play chess \(see [pgchess](https://pgxn.org/dist/pgchess/0.1.0/) for
and extension that does play chess\).

It is designed to efficiently store board positions from 
[fen](https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation)
fen strings. The boards are then very searchable and can be used
with gin indexes to find certain types of positions quickly.

Its main types are board, square, piece, cpiece, and piecesquare.
Positions can be searched by unnesting the pieces:

```sql
SELECT 
	ps::square AS square, ps::cpiece AS cpiece, ps::cpiece::piece AS piece 
FROM (
	SELECT unnest(pieces('8/8/8/8/8/8/8/5NnR'::board)) ps
)t ;
```
``` square | cpiece | piece 
--------+--------+-------
 f1     | N      | N
 g1     | n      | N
 h1     | R      | R
(3 rows)
```

There are also many fast c search functions that can search the board.


## Types
### adiagonal


Represents a anti-diagonal on the board.

https://en.wikipedia.org/wiki/Anti-diagonal_matrix
Uses 1 byte of storage.
TODO


### board


Represents a chess position.

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



### cfile


Represents a chess board file a-h.

Input format is 'a'.
Uses 1 byte of storage.
Can be cast to an int where 'a'=0.


### cpiece


String of pieces to filter out pieces.

Uses 6 byte of storage. 


### diagonal


Represents a diagonal on the board.

Uses 1 byte of storage.
TODO


### pfilter

TODO

### piece


Represents a chess piece without color.

Input format is 'P' or 'p' (case does not matter).
Uses 1 byte of storage. 
Supports =, <>, and hash operations.

```sql
SELECT pieces('8/8/8/8/8/8/8/6Pp'::board, 'p'::piece);
```
```
  pieces   
---------------------------------
 {Pg1,ph1}
(1 row)
```


### piecesquare


A colored piece occuping a square on the board.

Uses 2 bytes of storage. Input format is 'Ke4'.
Supports =, <>, <, >, >=, <=, hash operations and btree operations.
Can be cast to square, cpiece and int2.


### pindex


**Ordered string of piece types.**

Uses 2 bytes of storage.
Supports =, <>, and hash operations.


### rank


Represents a chess rank file 1-8.

Input format is '1'.
Uses 1 byte of storage.
Can be cast to an int where '1'=0.


### side


The side TO GO OR the side OF a piece.

Input format is 'w' or 'b'. Uses 1 byte of storage
Supports =, <>, and hash operations.
```sql
SELECT pieces('8/8/8/8/8/8/8/6Pp'::board, 'b'::side);
```
```
 pieces 
------------------------
 {ph1}
(1 row)
```


### square


A square on the board.

The sort order is in fourth 
[quadrant](https://en.wikipedia.org/wiki/Quadrant_\(plane_geometry\))
where a8 is 0.
Input format is 'e4'.
Uses 1 byte of storage. Squares can be cast to chars and ints to work with the raw
number.  Supports =, <>, <, >, >=, <=, hash operations and btree operations.

Select pieces occupying square h1:
```sql
SELECT pieces('8/8/8/8/8/8/8/6Pp'::board, 'h1'::square);
```
```
 pieces 
------------------------
 ph1
(1 row)
```

## Functions
### footer(board) -> cstring

Returns the the fen string after the first board field.

### halfmove(board) -> integer

Returns the halfmove clock number.
It will be zero if it was not set in the fen

### int_array(board) -> integer[]

Returns an array of size 64 with unique integers representing the occupancy of the colored piece in fen order.

Intergers values: {P=1,N=2,B=3,R=4, Q=5, K=7, p=8, n=9, b=10, r=11, q=12, k=13}

Useful for external machine learning and statical analysis.


### invert(board) -> board


Reverses fen string, colors, and side to move. [sql]

A white piece on a1 will become a black piece on h8.
With this method we can treat all positions as 
white meaning the side the move \(Classifying Chess Positions, De Sa, 2012\).

Blacks go after 1. e4:
```sql
SELECT pretty(invert('rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1'));
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


### move(board) -> integer

Returns the move number.
It will be zero if it was not set in the fen

### pcount(board) -> integer

Returns aumount of pieces on the board.

### pieces(board) -> piecesquare[]

Returns an array of piecesquares occupying the board in fen order.
The sort order is in fourth
[quadrant](https://en.wikipedia.org/wiki/Quadrant_\(plane_geometry\))
where  a8 is 0. The pieces family of functions all have the same sort behavior except for pieces_so.

Search for pieces occupying a1 or are white kings:
```sql
    SELECT ps FROM 
    (
       SELECT unnest(pieces('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR'::board)) ps
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


### pieces_so(board) -> piecesquare[]

Returns an array of piecesquares in square order [sql].
The sort order is in first
[quadrant](https://en.wikipedia.org/wiki/Quadrant_\(plane_geometry\))
where a1 is 0.


### score(board) -> integer

Returns the sum of the piece values.
Sums the piece values using {1,3,3,5,9} for {P,B,N,R,Q} respectivly with negative values for the black pieces.

### side(board) -> side

Returns the side which has the go.

### bitboard(board, cpiece) -> bit

Returns a 64 bit string with 1's representing the occupancy of the piece in fen order.

### bitboard_array(board, cpiece) -> bit[]

Returns an array of size 64 with 1's representing the occupancy of the piece in fen order. (sql)

### pcount(board, cpiece) -> integer

Returns aumount of pieces on the board of a certain type cpiece.

### pieces(board, cpiece) -> piecesquare[]

Returns an array of piecesquares for the colored pieces that are present.

### pfilter(board, pfilter) -> board

Returns a board given the pfilter.

### pcount(board, piece) -> integer

Returns aumount of pieces on the board of a certain type piece.

### pieces(board, piece) -> piecesquare[]

Returns an array of piecesquares for the pieces that are present.

### pieceindex(board, side) -> pindex

Returns the pieceindex given a side.

### pieces(board, side) -> piecesquare[]

Returns an array of piecesquares for the side.

### pieces(board, square) -> piecesquare

Returns a piecesquare given the square or null if there is no piece.

### pieces(board, square[]) -> piecesquare[]

Returns an array of piecesquares given the squares that are occupied.

### pretty(board, uni boolean DEFAULT false, showfen boolean DEFAULT false) -> text


Generates a set of squares in fen order.

### pretty(cpiece) -> text


Returns the unicode character of the piece.

```sql
SELECT pretty('p'::cpiece);
```
```
 pretty 
--------
 ♟
(1 row)
```


### side(cpiece) -> side


Returns the side type of the piece.
```sql
SELECT side('p'::cpiece);
```
```
 side 
---------
 b
(1 row)
```


### value(cpiece) -> integer


Returns the scoring value of the colored piece.

Black pieces will be have negative values.

values = {1,3,3,4,9,0} for {P,N,B,R,Q,K}
and      {1,3,3,4,9,0}\*-1 for {p,n,b,r,q,k}

```sql
 SELECT value('p'::cpiece);
```
```
 value 
------------
    -1
    (1 row)
```


### pretty(piece) -> text


Returns the unicode character of the piece.

```sql
SELECT pretty('p'::piece);
 pretty 
----------
 ♟
(1 row)


### value(piece) -> integer


Returns the scoring value of the piece.

values = {1,3,3,4,9,0} for {p,n,b,r,q,k}

```sql
SELECT value('p'::piece);
```
```
 value 
---------------------
     1
    (1 row)
```


### cpiece(piecesquare) -> cpiece


Casts piecesquare to cpiece.


### pretty(piecesquare) -> text


Returns unicode chess symbol of piece. [sql]

```sql
 SELECT pretty('pa2'::piecesquare);
```
```
 pretty 
--------
 ♟a2
(1 row)
```


### square(piecesquare) -> square


Casts piecesquare to square.


### board(piecesquare[]) -> board

Returns a board given the pieces with initial state.

### pretty(piecesquare[]) -> text[]


Returns unicode chess symbol of pieces. [sql]


### board(piecesquare[], text) -> board

Returns a board given the pieces and the footer part of the fen string.

### pretty(text, uni boolean DEFAULT false, showfen boolean DEFAULT false) -> text


Converts fen string to a printable board. [sql]

If uni is true then use unicode.
If showfen is true add the fen string at the bottom of the board.


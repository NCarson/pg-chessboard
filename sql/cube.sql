
CREATE or replace FUNCTION cube(board)
RETURNS cube AS $$
    select (cube(
              bitboard_array($1, 'K') 
           || bitboard_array($1, 'Q')
           || bitboard_array($1, 'R') 
           || bitboard_array($1, 'B')
           || bitboard_array($1, 'N') 
           || bitboard_array($1, 'P')
           || bitboard_array($1, 'k') 
           || bitboard_array($1, 'q')
           || bitboard_array($1, 'r') 
           || bitboard_array($1, 'b')
           || bitboard_array($1, 'n') 
           || bitboard_array($1, 'p')
    )) 
$$ LANGUAGE SQL IMMUTABLE STRICT;

create or replace function cube(piecesquare[]) 
returns cube as $$
    select cube(piecesquares_to_board($1))
$$ LANGUAGE SQL IMMUTABLE STRICT;

create or replace function distances(piecesquare[]) 
returns setof double precision as $$
    select cube(piecesquares_to_board(keypieces)) <-> cube(piecesquares_to_board($1))
    from position
$$ LANGUAGE SQL IMMUTABLE STRICT;


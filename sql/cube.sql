
CREATE FUNCTION cube(board)
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

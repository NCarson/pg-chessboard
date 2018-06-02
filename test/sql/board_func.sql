

select expected_or_fail_bool(
    (select footer('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -'::board)::text = 'w KQkq -'), true);

select expected_or_fail_bool(
    (select pcount('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board) = 32), true);

select expected_or_fail_bool(
    (select pcount('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board, 'p'::cpiece) = 8), true);

select expected_or_fail_bool(
    (select pcount('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board, 'P'::cpiece) = 8), true);

select expected_or_fail_bool(
    (select pcount('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board, 'P'::piece) = 16), true);

select expected_or_fail_bool(
    (select side('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board) = 'w'), true);

select expected_or_fail_bool(
    (select side('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1'::board) = 'b'), true);

select expected_or_fail_bool(
    (select pieceindex('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board, 'w') = 'QRRBBNNPPPPPPPP'), true);

select expected_or_fail_bool(
    (select pieceindex('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board, 'b') = 'QRRBBNNPPPPPPPP'), true);

select ('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board)::piecesquare[];

select pfilter('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board, 'p');

select expected_or_fail_bool(
    (select pieces('p7/8/8/8/8/8/8/K7 w KQkq -'::board) = '{pa8,Ka1}'::piecesquare[]), true);

select expected_or_fail_bool(
    (select pieces('pp6/8/8/8/8/8/8/K7 w KQkq -'::board, 'p'::cpiece) =  '{pa8,pb8}'::piecesquare[]), true);

select expected_or_fail_bool(
    (select pieces('pp6/8/8/8/8/8/8/P7 w KQkq -'::board, 'p'::cpiece) =  '{pa8,pb8}'::piecesquare[]), true);

select expected_or_fail_bool(
    (select pieces('pp6/8/8/8/8/8/8/P7 w KQkq -'::board, 'p'::piece) =  '{pa8,pb8,Pa1}'::piecesquare[]), true);

select expected_or_fail_bool(
    (select pieces('pp6/8/8/8/8/8/8/P7 w KQkq -'::board, 'w'::side) =  '{Pa1}'::piecesquare[]), true);

select expected_or_fail_bool(
    (select pieces('pp6/8/8/8/8/8/8/P7 w KQkq -'::board, 'a1'::square) =  'Pa1'::piecesquare), true);

select expected_or_fail_bool(
    (select pieces('pp6/8/8/8/8/8/8/P7 w KQkq -'::board, '{a1,a2}'::square[]) =  '{Pa1}'::piecesquare[]), true);

select expected_or_fail_bool(
    (select bitboard('pp6/8/8/8/8/8/8/7P w KQkq -'::board, 'P'::cpiece) = 1::bit(64)), true);

select expected_or_fail_bool(
    (select int_array('p7/8/8/8/8/8/8/7P w KQkq -'::board) =
     '{7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}'::INT[]), TRUE);




select expected_or_fail_bool(
    (select footer('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board)::text = 'w KQkq -'), true);

select expected_or_fail_bool(
    (select pcount('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board) = 32), true);

select expected_or_fail_bool(
    (select side('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board) = 'w'), true);

select expected_or_fail_bool(
    (select side('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1'::board) = 'b'), true);

select expected_or_fail_bool(
    (select pieceindex('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board, 'w') = 'QRRBBNNPPPPPPPP'), true);

select expected_or_fail_bool(
    (select pieceindex('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board, 'b') = 'QRRBBNNPPPPPPPP'), true);


select ('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board)::piecesquare[];

select remove_pieces('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board, 'p');

/*
select expected_or_fail_bool(
    (select pieces('p7/8/8/8/8/8/8/K7 w KQkq -'::board) = '{Kh8,ph1}'), true);
*/

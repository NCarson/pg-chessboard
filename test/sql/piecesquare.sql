

select expected_or_fail_int(
    (select count(*) from (select  fen = piecesquares_to_board(pieces(fen), footer(fen)::text )p from position) as t where p=false)::int, 0);

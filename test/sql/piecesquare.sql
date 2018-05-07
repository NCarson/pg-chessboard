

select pieces('k6K/8/8/8/8/8/8/p6P/ w - -'::board);
select expected_or_fail_int(
    (select count(*) from (select  fen = piecesquares_to_board(pieces(fen), footer(fen)::text )p from position) as t where p=false)::int, 0);

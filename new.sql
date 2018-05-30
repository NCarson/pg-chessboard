

/*
select *
from
(
    select
          pretty(remove_pieces(fen, 'kqrnb'), false, false)
        , kmeans(b, 10) OVER ()
    from
    (
        select
            fen, 
            (bitarray(bitboard(fen, 'p')) || bitarray(bitboard(fen, 'P')))::int[]::float[] as b 
        from position limit 10000
    ) t
) tt
where kmeans = 1
;
*/

create or replace function pawn_array(board)
returns int[] as
$$
    select 
        array_agg(
        case
            when a = 0 and b = 0 then 0
            when a != 0 then 1
            else 2
        end
    )
    from
    (
        select
             unnest(array_replace(bitarray(bitboard($1, 'p'))::int[], 1 , 2)) a
            ,unnest(bitarray(bitboard($1, 'P'))::int[]) b
    ) tt;

$$ language sql strict immutable;



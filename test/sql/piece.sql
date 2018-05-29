
select 'p'::cpiece::piece::text = 'P';

select pretty('p'::piece);
select pretty('n'::piece);
select pretty('b'::piece);
select pretty('r'::piece);
select pretty('q'::piece);
select pretty('k'::piece);

select expected_or_fail_bool('p'::piece = 'P'::piece, true);
select expected_or_fail_bool('p'::piece != 'P'::piece, false);
select expected_or_fail_bool('p'::piece != 'K'::piece, true);
select expected_or_fail_bool('p'::piece = 'K'::piece, false);

select expected_or_fail_bool(value('P'::piece) =  1, true);
select expected_or_fail_bool(value('N'::piece) =  3, true);
select expected_or_fail_bool(value('B'::piece) =  3, true);
select expected_or_fail_bool(value('R'::piece) =  5, true);
select expected_or_fail_bool(value('Q'::piece) =  9, true);
select expected_or_fail_bool(value('K'::piece) =  0, true);



select pretty('p'::cpiece);
select pretty('n'::cpiece);
select pretty('b'::cpiece);
select pretty('r'::cpiece);
select pretty('q'::cpiece);
select pretty('k'::cpiece);
select pretty('P'::cpiece);
select pretty('N'::cpiece);
select pretty('B'::cpiece);
select pretty('R'::cpiece);
select pretty('Q'::cpiece);
select pretty('K'::cpiece);

select expected_or_fail_bool('p'::piece  = 'p'::piece, true);
select expected_or_fail_bool('p'::piece != 'p'::piece, false);
select expected_or_fail_bool('p'::piece != 'K'::piece, true);
select expected_or_fail_bool('p'::piece  = 'K'::piece, false);

select expected_or_fail_bool(value('P'::cpiece) =  1, true);
select expected_or_fail_bool(value('N'::cpiece) =  3, true);
select expected_or_fail_bool(value('B'::cpiece) =  3, true);
select expected_or_fail_bool(value('R'::cpiece) =  5, true);
select expected_or_fail_bool(value('Q'::cpiece) =  9, true);
select expected_or_fail_bool(value('K'::cpiece) =  0, true);
select expected_or_fail_bool(value('p'::cpiece) = -1, true);
select expected_or_fail_bool(value('n'::cpiece) = -3, true);
select expected_or_fail_bool(value('b'::cpiece) = -3, true);
select expected_or_fail_bool(value('r'::cpiece) = -5, true);
select expected_or_fail_bool(value('q'::cpiece) = -9, true);
select expected_or_fail_bool(value('k'::cpiece) =  0, true);

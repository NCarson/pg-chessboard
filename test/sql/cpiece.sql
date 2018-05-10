
select expected_or_fail_bool(cpiece_value('P'::cpiece) =  1, true);
select expected_or_fail_bool(cpiece_value('N'::cpiece) =  3, true);
select expected_or_fail_bool(cpiece_value('B'::cpiece) =  3, true);
select expected_or_fail_bool(cpiece_value('R'::cpiece) =  5, true);
select expected_or_fail_bool(cpiece_value('Q'::cpiece) =  9, true);
select expected_or_fail_bool(cpiece_value('K'::cpiece) =  0, true);
select expected_or_fail_bool(cpiece_value('p'::cpiece) = -1, true);
select expected_or_fail_bool(cpiece_value('n'::cpiece) = -3, true);
select expected_or_fail_bool(cpiece_value('b'::cpiece) = -3, true);
select expected_or_fail_bool(cpiece_value('r'::cpiece) = -5, true);
select expected_or_fail_bool(cpiece_value('q'::cpiece) = -9, true);
select expected_or_fail_bool(cpiece_value('k'::cpiece) =  0, true);

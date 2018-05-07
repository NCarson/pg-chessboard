create extension chess_index;
\set ON_ERROR_STOP off
\o /dev/null

\echo 'fail'
select 'a0'::square;
select 'a9'::square;
select 'A8'::square;
select 'i1'::square;

\set ON_ERROR_STOP on

\echo 'false'
select expected_or_fail_bool('a1'::square = 'b1'::square,  false);
select expected_or_fail_bool('a2'::square < 'b1'::square,  false);
select expected_or_fail_bool('b1'::square <= 'a1'::square, false);
select expected_or_fail_bool('a1'::square > 'b2'::square,  false);
select expected_or_fail_bool('a1'::square >= 'b2'::square, false);

\echo 'succeed'
select expected_or_fail_bool('a1'::square =  'a1'::square, true);
select expected_or_fail_bool('a2'::square >  'b1'::square, true);
select expected_or_fail_bool('a1'::square >= 'a1'::square, true);
select expected_or_fail_bool('a2'::square >= 'b1'::square, true);
select expected_or_fail_bool('a1'::square <  'a2'::square, true);
select expected_or_fail_bool('a1'::square <= 'a1'::square, true);
select expected_or_fail_bool('a1'::square <= 'a2'::square, true);

\echo 'norows'
select expected_or_fail_int((select count(a)  from (select a, a = a::square::int as t from generate_series(0, 63) as a) as tt where t=false)::int, 0);



    /* Qd8-h4+ 15. Kh2-g1 Bb7xg2 16. Bc4xe6+ Kg8-h8 
     * promotion: e7xd8Q+
     * */

select expected_or_fail_bool( (select 'a1-a2'::MOVE::TEXT  =  'a1-a2'), TRUE);
select expected_or_fail_bool(  (select 'Ba1-a2'::MOVE::TEXT  = 'Ba1-a2'  ), TRUE);
select expected_or_fail_bool(  (select 'Na1-a2'::MOVE::TEXT  = 'Na1-a2'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'Ra1-a2'::MOVE::TEXT  = 'Ra1-a2'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'Qa1-a2'::MOVE::TEXT  = 'Qa1-a2'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'Ka1-a2'::MOVE::TEXT  = 'Ka1-a2'  ), TRUE);
                             
SELECT expected_or_fail_bool(  (select  'a1xa2'::MOVE::TEXT  =  'a1xa2'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'Ba1xa2'::MOVE::TEXT  = 'Ba1xa2'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'Na1xa2'::MOVE::TEXT  = 'Na1xa2'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'Ra1xa2'::MOVE::TEXT  = 'Ra1xa2'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'Qa1xa2'::MOVE::TEXT  = 'Qa1xa2'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'Ka1xa2'::MOVE::TEXT  = 'Ka1xa2'  ), TRUE);
                             
SELECT expected_or_fail_bool(  (select 'a1-a2+'::MOVE::TEXT  = 'a1-a2+'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'Ba1-a2+'::MOVE::TEXT  = 'Ba1-a2+'), TRUE);
SELECT expected_or_fail_bool(  (select 'Na1-a2+'::MOVE::TEXT  = 'Na1-a2+'), TRUE);
SELECT expected_or_fail_bool(  (select 'Ra1-a2+'::MOVE::TEXT  = 'Ra1-a2+'), TRUE);
SELECT expected_or_fail_bool(  (select 'Qa1-a2+'::MOVE::TEXT  = 'Qa1-a2+'), TRUE);
SELECT expected_or_fail_bool(  (select 'Ka1-a2+'::MOVE::TEXT  = 'Ka1-a2+'), TRUE);
                             
SELECT expected_or_fail_bool(  (select 'a1-a2#'::MOVE::TEXT  = 'a1-a2#'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'Ba1-a2#'::MOVE::TEXT  = 'Ba1-a2#'), TRUE);
SELECT expected_or_fail_bool(  (select 'Na1-a2#'::MOVE::TEXT  = 'Na1-a2#'), TRUE);
SELECT expected_or_fail_bool(  (select 'Ra1-a2#'::MOVE::TEXT  = 'Ra1-a2#'), TRUE);
SELECT expected_or_fail_bool(  (select 'Qa1-a2#'::MOVE::TEXT  = 'Qa1-a2#'), TRUE);
SELECT expected_or_fail_bool(  (select 'Ka1-a2#'::MOVE::TEXT  = 'Ka1-a2#'), TRUE);
                             
SELECT expected_or_fail_bool(  (select 'a1-a2Q'::MOVE::TEXT  = 'a1-a2Q'  ), TRUE);
SELECT expected_or_fail_bool(  (select 'a1-a2Q#'::MOVE::TEXT  = 'a1-a2Q#'  ), TRUE);



SELECT expected_or_fail_bool(  (select promotion('a1-a2Q#'::MOVE) = 'Q'  ), TRUE);
SELECT expected_or_fail_bool(  (select FROM_('a1-a2Q#'::MOVE) = 'a1'  ), TRUE);
SELECT expected_or_fail_bool(  (select TO_('a1-a2Q#'::MOVE) = 'a2'  ), TRUE);
SELECT expected_or_fail_bool(  (select iscapture('a1-a2Q#'::MOVE) = FALSE  ), TRUE);
SELECT expected_or_fail_bool(  (select iscapture('a1xa2Q#'::MOVE) = TRUE ), TRUE);
SELECT expected_or_fail_bool(  (select ismate('a1xa2Q#'::MOVE) = TRUE ), TRUE);
SELECT expected_or_fail_bool(  (select ismate('a1xa2Q+'::MOVE) = FALSE ), TRUE);
SELECT expected_or_fail_bool(  (select isCHECK('a1xa2Q+'::MOVE) = TRUE), TRUE);
SELECT expected_or_fail_bool(  (select isCHECK('a1xa2Q'::MOVE) = false), TRUE);
SELECT expected_or_fail_bool(  (select piece('a1xa2Q+'::MOVE) = 'P'), TRUE);
SELECT expected_or_fail_bool(  (select piece('Ba1xa2+'::MOVE) = 'B'), TRUE);

SELECT expected_or_fail_bool(  (select san('Ba1xa2+'::MOVE) = 'Bxa2+'), TRUE);
SELECT expected_or_fail_bool(  (select san('Ba1-a2+'::MOVE) = 'Ba2+'), TRUE);
SELECT expected_or_fail_bool(  (select san('a1-a2+'::MOVE) = 'a2+'), TRUE);
SELECT expected_or_fail_bool(  (select san('a1xb2+'::MOVE) = 'axb2+'), TRUE);



select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'::board::text
      ='rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';

select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -'::board;
select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -'::board::text
      ='rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -';

select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - -'::board::text
      ='rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - -';

select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b - e3'::board::text
      ='rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b - e3';

select '8/8/8/8/8/8/8/8'::board::text;

--XXX probably should not allow this
select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KKKKKQ -'::board::text;

-- should fail
select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - -1 1'::board::text;
select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 500 1'::board::text;
select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 1 550'::board::text;
select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 1 -1'::board::text;
select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR . KQkq -'::board::text;
select 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w . -'::board::text;


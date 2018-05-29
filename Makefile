
EXTENSION 		= chess_index
MODULE_big   	= chess_index
OBJS         	= src/chess_index.o src/board.o src/types.o
DATA 		 	= sql/chess_index--0.0.1.sql

REGRESS_OPTS  	= --inputdir=test         \
                  --load-extension=chess_index
REGRESS       	= setup piece cpiece square board board_func piecesquare pfilter

#DATA        	= $(filter-out $(wildcard sql/*--*.sql),$(wildcard sql/*.sql))
#MODULES      	= $(patsubst %.c,%,$(wildcard src/*.c))
#SHLIB_LINK += $(filter -lm, $(LIBS))

PG_CONFIG 	= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# need postgres built with coverage flags
#coverage: 
#	lcov -d . -c -o lcov.info
#	genhtml --show-details --legend --output-directory=coverage --title=PostgreSQL --num-spaces=4 --prefix=./src/ `find . -name lcov.info -print`

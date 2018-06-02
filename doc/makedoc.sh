
 
cat doc/README.header > README.md
psql -qtAX -d chess_test -f doc/doc.sql >> README.md


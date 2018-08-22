
 
cat doc/README.header > README.md
psql -qtAX -d chess-test -f doc/doc.sql >> README.md


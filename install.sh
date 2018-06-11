
set -e

DB=chess_test

#if [ -z "$PGDATABASE" ]
#then
#    (>&2 echo "please set the env var \$PGDATABASE for the db you want to install in. example... 'export PGDATABASE=chess'")
#    exit 1
#fi

CHECK=1

while getopts ":n" opt; do
  case $opt in
    n)
      echo "setting check to 0"
      CHECK=0
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done

#make clean
make
sudo make uninstall
sudo make install
#psql -dchess_test -c 'drop extension chess_index cascade'
#psql -dchess_test -f sql/chess_index.sql

if [[ "$CHECK" -eq 1 ]]; then
    make installcheck || psql -d$DB -f sql/chess_index.sql
fi

dropdb $DB
createdb $DB
psql -d$DB -c"drop extension if exists chess_index cascade" >/dev/null
psql -d$DB -c"create extension chess_index" >/dev/null
sh doc/makedoc.sh
psql -d$DB -c"create extension cube" >/dev/null
psql -d$DB -c"create extension smlar" >/dev/null

cd ../index && make test DIREC="./data/test" 

set -ex

#if [ -z "$PGDATABASE" ]
#then
#    (>&2 echo "please set the env var \$PGDATABASE for the db you want to install in. example... 'export PGDATABASE=chess'")
#    exit 1
#fi

DB=chess_test
CHECK=1
USER='\"www-data\"'

GAMES_DIREC='../games'
SCIPY_DIREC='../scipy'

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

dropdb $DB
createdb $DB
make
sudo make uninstall
sudo make install

if [[ "$CHECK" -eq 1 ]]; then
    make installcheck #|| psql -d$DB -f sql/chess_index.sql -v ON_ERROR_STOP=1
fi

bash doc/makedoc.sh

cd $GAMES_DIREC && make DATA_DIREC="./data/test" DB="$DB"
cd $SCIPY_DIREC && make USER=$USER DB=$DB

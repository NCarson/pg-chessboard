set -ex

#if [ -z "$PGDATABASE" ]
#then
#    (>&2 echo "please set the env var \$PGDATABASE for the db you want to install in. example... 'export PGDATABASE=chess'")
#    exit 1
#fi

DB=chess-test
CHECK=1
GAMES=1
USER='web_app'

GAMES_DIREC='../games'
SCIPY_DIREC='../scipy'

while getopts ":nx" opt; do
  case $opt in
    n)
      echo "setting check to 0"
      CHECK=0
      ;;
    x)
      echo "not make games"
      GAMES=0
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

if [[ "$GAMES" -eq 1 ]]; then
    cd $GAMES_DIREC && make DATA_DIR="./data/test" DB="$DB"
    #cd $SCIPY_DIREC && make USER=$USER DB=$DB
else
    psql -d $DB  -c 'drop extension if exists chess_index'
    psql -d $DB  -c 'create extension chess_index'
fi

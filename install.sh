
set -e

if [ -z "$PGDATABASE" ]
then
    (>&2 echo "please set the env var \$PGDATABASE for the db you want to install in. example... 'export PGDATABASE=chess'")
    exit 1
fi

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

if [[ "$CHECK" -eq 1 ]]; then
    make installcheck;
fi

psql -c"drop extension if exists chess_index cascade" >/dev/null
psql -c"create extension chess_index" >/dev/null
psql -f ./test/sql/setup.sql

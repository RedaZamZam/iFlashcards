#!/bin/sh

# Automaticaly check the exit status
# http://petereisentraut.blogspot.com/2010/11/pipefail.html
set -e
set -u
set -o pipefail
set -E

err()
{
    echo "Error has been deteted" 1>&2
    exit 1
}

quit()
{
    echo "quit"
}

trap quit EXIT
trap err ERR


VERSION="Boost_1_53_0"
CURDIR=`pwd`
REPO_DIR="$CURDIR/../$VERSION"

if [ "$OSTYPE" == "msys" ]; then
  WIN="Windows"
fi

if [ "$WIN" ]; then
  LIB_DIR="lib"
else
  LIB_DIR="lib_macx"
fi

cd $REPO_DIR

if [ ! -f ./b2 ]; then
  if [ "$WIN" ]; then
    cmd.exe /c "bootstrap.bat"
  else
    ./bootstrap.sh
  fi
fi

BOOST_LIBS_LIST="--with-date_time --with-exception --with-filesystem --with-program_options --with-regex --with-serialization --with-system --with-thread"

if [ "$WIN" ]; then
  #http://stackoverflow.com/questions/4521252/qt-msvc-and-zcwchar-t-i-want-to-blow-up-the-world
  ./b2 $BOOST_LIBS_LIST link=static cxxflags="-Zc:wchar_t-" stage debug release
else
  ./b2 $BOOST_LIBS_LIST --toolset=darwin --layout=tagged link=static cxxflags="-arch x86_64" stage debug release
fi

echo All operations have been compleated sucessfuly!
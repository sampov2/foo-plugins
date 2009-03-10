#!/bin/sh

SVN_URL=http://localhost/svn/foo-plugins/trunk

RELEASE_NUMBER=$1
RELEASE_NAME=foo-plugins-$RELEASE_NUMBER
REMOVE_LIST="graph.sh architect_graph.sh tarball.sh basari.raw snare.raw"

if [ "$RELEASE_NUMBER" = "" -o "$(echo $RELEASE_NUMBER | sed 's/[0-9.]//g')" != "" ]; then
	echo "usage: tarball.sh [version number]"
	exit
fi

if [ -f releases/$RELEASE_NAME.tar.bz2 ]; then
	echo "releases/$RELEASE_NAME.tar.bz2: exists already"
	exit
fi


TMPDIR=$(dirname $0)/tmp

if [ ! -d $TMPDIR ]; then
	mkdir $TMPDIR
else
	echo $TMPDIR: directory exists already, cannot continue
	exit
fi

cd $TMPDIR

svn checkout $SVN_URL $RELEASE_NAME
cd $RELEASE_NAME

rm -rf $(find -type d -name .svn)
rm $REMOVE_LIST
rmdir plot
rmdir releases

cd ..

tar cjf ../releases/$RELEASE_NAME.tar.bz2 $RELEASE_NAME

cd ..

rm -rf tmp

echo Release tarball releases/$RELEASE_NAME.tar.bz is done





#!/bin/sh

# Make and release maiko for one os / arch

tag=$1
if [ -z "$tag" ] ; then
    tag=maiko-`date +%y%m%d`
fi

cd ../maiko/bin
export PATH=.:"$PATH"
osarch=`osversion`.`machinetype`

./makeright x
./makeright init

cd ../..
mkdir -p maiko/build
echo making $tag-$osarch.tgz

tar cfz maiko/build/$tag-$osarch.tgz   \
    maiko/bin/osversion			     \
    maiko/bin/machinetype		     \
    maiko/bin/config.guess		     \
    maiko/bin/config.sub		     \
    maiko/$osarch/lde*

echo uploading
cd maiko
gh release upload --clobber $tag build/$tag-$osarch.tgz

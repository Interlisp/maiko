#!/bin/sh

# Make and release maiko for one os / arch

tag=$1
if [ -z "$tag" ] ; then
    tag=nightly-`date +%y%m%d`
fi

cd ../maiko/bin
export PATH=.:"$PATH"
osarch=`osversion`.`machinetype`

./makeright x
./makeright init

cd ../..

echo making maiko-$tag-$osarch.tgz

tar cfz maiko/build/maiko-$tag-$osarch.tgz   \
    maiko/bin/osversion			     \
    maiko/bin/machinetype		     \
    maiko/bin/config.guess		     \
    maiko/bin/config.sub		     \
    maiko/$osarch/lde*

echo uploading
gh release upload --clobber $tag tmp/maiko-$tag-$osarch.tgz

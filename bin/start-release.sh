#!/bin/sh

# Start Maiko release

tag=$1
if [ -z "$tag" ] ; then
    tag=maiko-`date +%y%m%d`
fi

gh release create $tag -p -t $tag -n "See release notes in medley repo"

echo Now run [32m./release-one.sh $tag[0m in maiko/bin on every os/machine
echo when done, edit the release in your browser and uncheck the "prerelease" box		

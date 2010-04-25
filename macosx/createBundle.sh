#!/bin/sh

# First we create the bundle directory to host the bundle
rm -rf GCompris
echo "Creating the bundle dir"
mkdir -p GCompris/GComprisAdmin.app/Contents/MacOS
mkdir -p GCompris/GComprisAdmin.app/Contents/Resources
echo -n "APPL????" > GCompris/GComprisAdmin.app/Contents/PkgInfo
cp GComprisAdmin.icns GCompris/GComprisAdmin.app/Contents/Resources
cp GComprisAdmin GCompris/GComprisAdmin.app/Contents/MacOS
cp InfoAdmin.plist GCompris/GComprisAdmin.app/Contents/Info.plist
cd GCompris
ln -s /Applications .
cd -

# Create the bundle of the complete application
echo "Create the bundle of the complete application"
ige-mac-bundler gcompris.bundle

# Create the distributable .dmg
echo "Creating the final GCompris.dmg"
rm -f GCompris.dmg
./mkdmg GCompris

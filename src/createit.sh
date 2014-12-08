#!/bin/sh
#set -x
if [ -z $1 ]; then
  echo "Usage: createit.sh [new GCompris activity]"
  echo "   e.g.: createit.sh myactivity"
  exit 1
fi

activity=$1
activitydir=$1-activity

template=pythontemplate
templatedir=pythontemplate-activity

path=`dirname $0`

cp -r $templatedir $activitydir
cp newactivity.svg $activitydir/$activity.svg
cd $activitydir

# Cleanup, remove generated files
rm -f Makefile pythontemplate.pyc \
    Makefile Makefile.in python.svg

mv $template.py $activity.py
sed -i s/$template/$activity/g init_path.sh Makefile.am $activity.py
sed -i s/python.svg/$activity.svg/ Makefile.am

mv $template.xml.in $activity.xml.in
mv $template.xml $activity.xml

# Let's process also $activity.xml so that running 'make'
# is not mandatory to run the activity.
for menu in $activity.xml.in $activity.xml
do
  sed -i s/$template/$activity/g init_path.sh $menu .gitignore
  sed -i s/python.svg/$activity.svg/ $menu

  sed -i "s:Bruno Coudoin:your name here:" $menu
  sed -i "s:\\(<_title>.*</_title>\\):<_title>set a title for $activity</_title>:" $menu
  sed -i "s:\\(<_description>.*</_description>\\):<_description>set a description for $activity</_description>:" $menu
  sed -i "s:\\(<_prerequisite>.*</_prerequisite>\\):<_prerequisite>set a prerequisite for $activity</_prerequisite>:" $menu
  sed -i "s:\\(<_goal>.*</_goal>\\):<_goal>set a goal for $activity</_goal>:" $menu
  sed -i "s:\\(<_manual>.*</_manual>\\):<_manual>set a manual for $activity</_manual>:" $menu
  sed -i "s:\\(<_credit>.*</_credit>\\):<_credit>set a credit for $activity</_credit>:" $menu
done

cd -

# Now add the activity to our compilation chain
sed -i "s:\\(AC_OUTPUT.*\\):\\1 src/$activitydir/Makefile:" ../configure.ac
sed -i "s:SUBDIRS = :SUBDIRS = $activitydir :" Makefile.am
echo src/$activitydir/$activity.xml.in >> ../po/POTFILES.in
echo src/$activitydir/$activity.py >> ../po/POTFILES.in

echo "Now you can test your activity with:"
echo "./runit $activitydir"
echo "To run it in a complete GCompris, run:"
echo "make && sudo make install"
echo "gcompris --experimental"
echo "(by default the activity is in the experimental section."
echo " To change is, edit the file $activitydir/$activity.xml.in"

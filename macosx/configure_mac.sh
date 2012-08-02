cd ..
export PYTHONPATH=$HOME/gtk/inst/lib/python2.7/site-packages
./configure --prefix $HOME/gtk/inst/ --libdir $HOME/gtk/inst/lib --enable-shared --disable-static --enable-py-build-only --with-python=$HOME/gtk/inst/bin/python --enable-nsbundle --disable-binreloc


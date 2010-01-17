from distutils.core import setup
import py2exe

setup(
    name = 'gcompris',
    description = 'gcompris',
    version = '9.0',

    windows = [
                  {
                      'script': 'gcompris.py',
                  }
              ],

    options = {
                  'py2exe': {
                      'packages':'encodings',
                      'includes': 'cairo, pango, pangocairo, atk, gobject, sqlite3',
                  }
              },

    data_files= []
)

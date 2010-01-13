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
                      'includes': 'cairo, pango, pangocairo, atk, gobject',
                  }
              },

    data_files= [
                  ("Microsoft.VC90.CRT", py26MSdll),
                  ("lib\Microsoft.VC90.CRT", py26MSdll)
                ]
)

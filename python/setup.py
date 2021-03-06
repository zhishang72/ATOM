from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules = [
    Extension ( "pyatom",
              [
                  'pyatom.pyx',
                  'PythonStream.cpp'
              ],
              language = 'c++',
              extra_compile_args=["-std=c++11"],
              libraries = [ 'atom' ],
              include_dirs = [ '../atmosphere', '../hydrosphere', '../lib', '../tinyxml2' ],
              library_dirs = [ '..' ]
              ) ]

setup (
    name = 'pyatom',
    cmdclass = { 'build_ext': build_ext },
    ext_modules = ext_modules )

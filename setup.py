from distutils.core import setup, Extension


setup(name = 'hwcounter',
      version = '0.1',
      description = 'Hardware timestamp counter',
      ext_modules = [Extension('hwcounter',
                               sources = ['hwcounter.c'])])

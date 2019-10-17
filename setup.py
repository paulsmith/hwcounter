from setuptools import setup, Extension
from os import path
import sys

here = path.abspath(path.dirname(__file__))

if sys.version_info[0] > 3:
    with open(path.join(here, 'README.md'), encoding='utf-8') as f:
        long_description = f.read()
else:
    with open(path.join(here, 'README.md')) as f:
        long_description = f.read()

setup(name = 'hwcounter',
      version = '0.1.1',
      description = 'Highly accurate counter for measuring elapsed time in Python',
      long_description = long_description,
      url = 'https://github.com/paulsmith/hwcounter',
      author = 'Paul Smith',
      author_email = 'paulsmith@pobox.com',
      classifiers = [
          'Development Status :: 3 - Alpha',
          'Intended Audience :: Developers',
          'Topic :: System :: Benchmark',
          'License :: OSI Approved :: Apache Software License',
          'Programming Language :: Python :: 2',
          'Programming Language :: Python :: 2.7',
          'Programming Language :: Python :: 3',
          'Programming Language :: Python :: 3.4',
          'Programming Language :: Python :: 3.5',
          'Programming Language :: Python :: 3.6',
      ],
      keywords = 'benchmark x86 rdtsc timing',
      ext_modules = [Extension('hwcounter',
                               sources = ['hwcounter.c'])])


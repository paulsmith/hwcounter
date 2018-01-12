install:
	./bin/python setup.py install

package:
	./bin/python setup.py sdist
	./bin/python setup.py bdist_wheel

upload:
	twine upload dist/*

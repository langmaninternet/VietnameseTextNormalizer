
CPP_FILES = $(wildcard *.cpp)
OBJECT_PYTHON2_FILES  = $(patsubst %.cpp, ObjectPython2/%.o, $(CPP_FILES))
OBJECT_PYTHON3_FILES  = $(patsubst %.cpp, ObjectPython3/%.o, $(CPP_FILES))
GPP = g++-5

all: $(OBJECT_PYTHON2_FILES) $(OBJECT_PYTHON3_FILES)
	$(GPP) -std=c++17 -shared -o ReleasePython2/VietnameseTextNormalizer.so $(OBJECT_PYTHON2_FILES) -L. ; \
	cp -f UnitTestVietnameseTextNormalizer.py ReleasePython2/ ; \
	cd ReleasePython2 ; \
	echo "Build Release Python2 Done" ; \
	echo "Test Python2 - VietnameseTextNormalizer " ; \
	python2 UnitTestVietnameseTextNormalizer.py ; \
	cd .. ; \
	$(GPP)  -std=c++17 -shared -o ReleasePython3/VietnameseTextNormalizer.so $(OBJECT_PYTHON3_FILES) -L. ; \
	cp -f UnitTestVietnameseTextNormalizer.py ReleasePython3/ ; \
	cd ReleasePython3 ; \
	echo "Build Release Python3 Done" ; \
	echo "Test Python3 - VietnameseTextNormalizer " ; \
	python3 UnitTestVietnameseTextNormalizer.py ; )
	


release-dirs:
	@ ( mkdir -p ObjectPython2 ObjectPython3 ReleasePython2 ReleasePython3 ; )
	
ObjectPython2/%.o : %.cpp | release-dirs
	$(GPP) -std=c++17 -c -fPIC -O3 -Wall $*.cpp -I/usr/include/python2.7 -o ObjectPython2/$*.o  ;

ObjectPython3/%.o : %.cpp | release-dirs
	$(GPP) -std=c++17 -c -fPIC -O3 -Wall $*.cpp -I/usr/include/python3.5 -o ObjectPython3/$*.o  ;
	
clean: 
	rm -rf ObjectPython2 ObjectPython3 ReleasePython2 ReleasePython3 ; 
	clear ;


.PHONY: all clean



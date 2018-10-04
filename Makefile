
CPP_FILES = $(wildcard *.cpp)
OBJECT_FILES  = $(patsubst %.cpp, object/%.o, $(CPP_FILES))

all: $(OBJECT_FILES)
	g++-5 -std=c++17 -shared -o release/VietnameseTextNormalizer.so $(OBJECT_FILES) -L. ; \
	cp -f UnitTestVietnameseTextNormalizer.py release/ ; \
	cd release ; \
	echo "Done" ; \
	echo "Test VietnameseTextNormalizer " ; \
	python3 UnitTestVietnameseTextNormalizer.py ; )
	


release-dirs:
	@ ( mkdir -p object release ; )
	
object/%.o : %.cpp | release-dirs
	g++-5 -std=c++17 -c -fPIC -O3 $*.cpp -I/usr/include/python3.5 -o object/$*.o  ;

clean: 
	rm -rf object release ; 
	clear ;


.PHONY: all clean



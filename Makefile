
# Export this line in bash to compile
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/opt/X11/lib/pkgconfig
CXXFLAGS+=-std=c++11
CXXFLAGS+=-g -Wall
CPPFLAGS+=$(shell pkg-config --cflags cairo)
LDLIBS=$(shell pkg-config --libs cairo)

CC=${CXX}


draw : draw.o main.o input.o blob.o b2.o vec2d.o

blob.o : types.h blob.h
draw.o : types.h draw.h config.h
main.o : types.h draw.h blob.h config.h
vec2d.o : vec2d.h
b2.o : types.h vec2d.h b2.h config.h




#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c 
#	$(CC) $(LDFLAGS) draw.o  $(LOADLIBES) $(LDLIBS)

.PHONY : clean

clean :
	- rm -f draw draw.o main.o input.o b2.o blob.o vec2d.o

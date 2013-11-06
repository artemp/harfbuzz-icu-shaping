HB_CFLAGS=$(shell pkg-config harfbuzz --cflags) $(shell freetype-config --cflags) $(shell pkg-config icu-uc --cflags)
HB_LDFLAGS=$(shell pkg-config harfbuzz --libs) $(shell freetype-config --libs) $(shell pkg-config icu-uc --libs)
CXXFLAGS := $(CXXFLAGS) # inherit from env
LDFLAGS := $(LDFLAGS) # inherit from env

all: harfbuzz-icu-bench

harfbuzz-icu-bench: main.cpp Makefile
	$(CXX) -o ./harfbuzz-icu-bench main.cpp -O2 -DNDEBUG -Wall $(CXXFLAGS) $(HB_CFLAGS) $(HB_LDFLAGS) $(LDFLAGS)

test: harfbuzz-icu-bench
	./harfbuzz-icu-bench 1000

clean:
	@rm -f ./harfbuzz-icu-bench

.PHONY: test

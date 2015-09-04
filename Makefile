# OSX Makefile

build=build
target=$(build)/devops-detective-agency

CXXFLAGS=\
-Wall \
-g \
-pg \
-Imruby/include

CFLAGS=$(CXXFLAGS)
LDFLAGS=

objects = $(patsubst %,build/%, $(patsubst %.c,%.o, $(wildcard *.c)))
ruby_headers = $(patsubst %,build/%, $(patsubst lib/%.rb,%.h, $(wildcard lib/*.rb)))

$(target): $(build) $(objects) mruby/build/host/lib/libmruby.a $(ruby_headers)
	$(CXX) $(LDFLAGS) -o $@ $(objects) mruby/build/host/lib/libmruby.a

mruby/build/host/lib/libmruby.a:
	cd mruby && make

$(build)/%.o: %.c $(ruby_headers)
	$(CC) $(CXXFLAGS) -c $< -o $@

#mruby/bin/mrbc -B init -o build/init.h lib/init.rb
$(build)/%.h: lib/%.rb
	mruby/bin/mrbc -B $(patsubst build/%.h,%, $@) -o $@ $<

$(build):
	mkdir -p $(build)

clean:
	touch $(build) && rm -R $(build)

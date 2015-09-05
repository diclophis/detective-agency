# OSX Makefile

build=build
target=$(build)/detective-agency

CXXFLAGS=\
-Wall \
-g \
-pg \
-Imruby/include \
-Ilibyaml/include

CFLAGS=$(CXXFLAGS)
LDFLAGS=

objects = $(patsubst %,build/%, $(patsubst %.c,%.o, $(wildcard *.c)))
ruby_headers = $(patsubst %,build/%, $(patsubst lib/%.rb,%.h, $(wildcard lib/*.rb)))

$(target): $(build) $(objects) yaml/src/.libs/libyaml.a mruby/build/host/lib/libmruby.a $(ruby_headers)
	$(CXX) $(LDFLAGS) -o $@ $(objects) yaml/src/.libs/libyaml.a mruby/build/host/lib/libmruby.a

test: $(build)/ansible.log

build/ansible.log: $(build)/test.yml
	ansible-playbook -i inventory $(build)/test.yml | tee $@

$(build)/test.yml: $(target)
	$(target) > $@

mruby/bin/mrbc: mruby/build/host/lib/libmruby.a

mruby/build/host/lib/libmruby.a:
	cd mruby && MRUBY_CONFIG=../config/mruby.rb make

yaml/src/.libs/libyaml.a:
	cd yaml && ./configure CFLAGS="-DYAML_DECLARE_STATIC" --enable-static --disable-shared && make

$(build)/%.o: %.c $(ruby_headers)
	$(CC) $(CXXFLAGS) -c $< -o $@

#mruby/bin/mrbc -B init -o build/init.h lib/init.rb
$(build)/%.h: lib/%.rb mruby/bin/mrbc
	mruby/bin/mrbc -B $(patsubst build/%.h,%, $@) -o $@ $<

$(build):
	mkdir -p $(build)

yaml/Makefile:
	cd yaml && ./bootstrap && ./configure

clean: yaml/Makefile
	cd yaml && make clean
	cd mruby && make clean
	touch $(build) && rm -R $(build)

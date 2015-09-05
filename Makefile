# OSX Makefile

product=detective-agency
build=build
target=$(build)/$(product)
fpm_tmp:=$(shell mktemp -d -u -t fpm_XXXXX)

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
	$(CC) $(LDFLAGS) -o $@ $(objects) mruby/build/host/lib/libmruby.a yaml/src/.libs/libyaml.a -lm

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

install: $(target)
	mkdir -p $(DESTDIR)/usr/bin
	install $(target) $(DESTDIR)/usr/bin/$(product)

package:
	make install DESTDIR=$(fpm_tmp)
	fpm -s dir -t deb -n detective-agency -v 0.1.0 -C $(fpm_tmp) -d ansible usr/bin

clean: yaml/Makefile
	cd yaml && make clean
	cd mruby && make clean
	touch $(build) && rm -R $(build)

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

test: $(build)/test.yml
	ansible-playbook -vvv -i inventory $(build)/test.yml

$(build)/test.yml: $(target) Detectivefile
	$(target) > $@

mruby/bin/mrbc: mruby/build/host/lib/libmruby.a

mruby/build/host/lib/libmruby.a:
	cd mruby && MRUBY_CONFIG=../config/mruby.rb make

yaml/src/.libs/libyaml.a:
	cd yaml && ./configure CFLAGS="-DYAML_DECLARE_STATIC" --enable-static --disable-shared && make

$(build)/%.o: %.c $(ruby_headers)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(build)/%.h: lib/%.rb mruby/bin/mrbc
	mruby/bin/mrbc -g -B $(patsubst build/%.h,%, $@) -o $@ $<

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

ppa:
#make clean
# remove .git dirs
#dh_make --createorig -c gpl -e diclophis+ubuntu@gmail.com -s -a -p detective-agency_0.2.0
#debuild -S -sa
#cd ..
#dput -f -u ppa:diclophis/detective-agency detective-agency_0.2.0-1_source.changes 
	echo read Makefile for manual instructions

clean: yaml/Makefile
	cd yaml && make clean
	cd mruby && make clean
	touch $(build) && rm -R $(build)

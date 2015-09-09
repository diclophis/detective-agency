# Makefile

product=detective-agency
build=/tmp/$(product)-build
target=$(build)/$(product)
mruby_static_lib=mruby/build/host/lib/libmruby.a
mrbc=mruby/bin/mrbc
yaml_static_lib=yaml/src/.libs/libyaml.a

objects = $(patsubst %,$(build)/%, $(patsubst %.c,%.o, $(wildcard *.c)))
static_ruby_headers = $(patsubst %,$(build)/%, $(patsubst lib/%.rb,%.h, $(wildcard lib/*.rb)))
.SECONDARY: $(static_ruby_headers) $(objects)
objects += $(mruby_static_lib) $(yaml_static_lib)

CFLAGS=-Imruby/include -I$(build)

$(target): $(build) $(objects)
	$(CC) $(LDFLAGS) -o $@ $(objects)

test: $(build)/test.yml
	ansible-playbook --list-tasks -v -i 'localhost,' -c local $(build)/test.yml

$(build)/test.yml: $(target) Detectivefile
	$(target) > $@

clean:
	cd mruby && make clean
	cd yaml && make clean
	touch $(build) && rm -R $(build)

$(build):
	mkdir -p $(build)

$(build)/%.o: %.c $(static_ruby_headers)
	$(CC) $(CFLAGS) -c $< -o $@

$(mruby_static_lib): config/mruby.rb
	cd mruby && MRUBY_CONFIG=../config/mruby.rb make

$(mrbc): $(mruby_static_lib)

$(build)/%.h: lib/%.rb $(mrbc)
	mruby/bin/mrbc -g -B $(patsubst $(build)/%.h,%, $@) -o $@ $<

$(yaml_static_lib):
	cd yaml && ./configure CFLAGS="-DYAML_DECLARE_STATIC" --enable-static --disable-shared && make

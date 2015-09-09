# Makefile

product=detective-agency
build=/tmp/$(product)-build
target=$(build)/$(product)
mruby_static_lib=mruby/build/host/lib/libmruby.a
mrbc=mruby/bin/mrbc

objects = $(patsubst %,$(build)/%, $(patsubst %.c,%.o, $(wildcard *.c)))
static_ruby_headers = $(patsubst %,$(build)/%, $(patsubst lib/%.rb,%.h, $(wildcard lib/*.rb)))
.SECONDARY: $(static_ruby_headers) $(objects)

CFLAGS=-Imruby/include -I$(build)

$(target): $(build) $(objects) $(mruby_static_lib)
	$(CC) $(LDFLAGS) -o $@ $(objects) $(mruby_static_lib)

test: $(build)/test.yml
	ansible-playbook --list-tasks -v -i 'localhost,' -c local $(build)/test.yml

$(build)/test.yml: $(target) Detectivefile
	$(target) > $@

clean:
	cd mruby && make clean
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

# Makefile

product=detective-agency
build=/tmp/$(product)-build
target=$(build)/$(product)
mruby_static_lib=mruby/build/host/lib/libmruby.a

objects = $(patsubst %,$(build)/%, $(patsubst %.c,%.o, $(wildcard *.c)))

CFLAGS=-Imruby/include

$(target): $(build) $(objects) $(mruby_static_lib)
	$(CC) $(LDFLAGS) -o $@ $(objects) $(mruby_static_lib)

clean:
	cd mruby && make clean
	touch $(build) && rm -R $(build)

$(build):
	mkdir -p $(build)

$(build)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(mruby_static_lib):
	cd mruby && MRUBY_CONFIG=../config/mruby.rb make


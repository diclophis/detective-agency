# Makefile

product=detective-agency
build=/tmp/$(product)-build
target=$(build)/$(product)

objects = $(patsubst %,$(build)/%, $(patsubst %.c,%.o, $(wildcard *.c)))

$(target): $(build) $(objects)
	$(CC) $(LDFLAGS) -o $@ $(objects)

clean:
	touch $(build) && rm -R $(build)

$(build):
	mkdir -p $(build)

$(build)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


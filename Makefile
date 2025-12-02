CFLAGS := -Wall -Wextra -ggdb
LDFLAGS :=

bin/ycc: CFLAGS += -pedantic
bin/ycc: src/main.c | bin/
	cc $(CFLAGS) -o $@ $(LDFLAGS) $^

bin/:
	mkdir -p bin

clean:
	rm bin/*

CFLAGS := -Wall -Wextra -ggdb
LDFLAGS :=

bin/bcc: CFLAGS += -pedantic
bin/bcc: src/main.c src/flag.h | bin/
	cc $(CFLAGS) -o $@ $(LDFLAGS) $^

bin/:
	mkdir -p bin

clean:
	rm bin/*

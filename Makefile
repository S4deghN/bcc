sources := $(wildcard src/*.c)
depends := $(sources:src/%.c=bin/d/%.d)

CFLAGS := -Wall -Wextra -ggdb
LDFLAGS :=

bin/bcc: CFLAGS += -pedantic
bin/bcc: $(sources) | $(depends) bin/
	cc -MMD -MP $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(depends): bin/d
$(depends): bin/d/%.d: | src/%.c
	cc -MM -MP -MT $(@:.d=.c) $(CFLAGS) $| -MF $@

include $(depends)

bin/:
	mkdir -p bin

bin/d:
	mkdir -p bin/d

clean:
	rm -rf bin

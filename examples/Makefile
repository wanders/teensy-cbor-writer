EXAMPLES:=from-readme small
CFLAGS:=-Wall -Wextra -Wno-unused-parameter -Werror -O3 -I..

all: $(EXAMPLES)

$(EXAMPLES): %: %.c
	$(CC) -o $@ $< $(CFLAGS)


.PHONY: test-%
test-%: %
	./$< | python3 -mcbor2.tool # todo: check expected output

.PHONY: test
test: $(addprefix test-,$(EXAMPLES))

.PHONY: clean
clean:
	rm -f $(EXAMPLES)

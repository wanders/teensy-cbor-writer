
#include <teensy-cbor.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

void
myfunc(int spam_amount) {
	char buf[256];

	struct tcbor tcbor;
	tcbor_begin(&tcbor, buf, sizeof(buf));
	tcbor_map_begin(&tcbor);

	/* key */
	tcbor_str(&tcbor, "spam");
	/* value */
	tcbor_int(&tcbor, spam_amount);

	/* key */
	tcbor_str(&tcbor, "ham");
	/* value */
	tcbor_array_begin(&tcbor);
	tcbor_str(&tcbor, "egg");
	tcbor_str(&tcbor, "sausage");
	tcbor_str(&tcbor, "bacon");
	tcbor_array_end(&tcbor);

	tcbor_map_end(&tcbor);

	size_t len = tcbor_end(&tcbor);
	assert(len > 0);

	ssize_t res = write(STDOUT_FILENO, buf, len);
	assert(res > 0);
}

int
main(int argc, char **argv) {
	myfunc(42);
	return 0;
}

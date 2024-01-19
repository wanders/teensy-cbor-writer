
#include <teensy-cbor.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv) {
	char buf[1024];

	struct tcbor tcbor;
	tcbor_begin(&tcbor, buf, sizeof(buf));
	tcbor_map_begin(&tcbor);
	{
		tcbor_str(&tcbor, "foo");
		tcbor_int(&tcbor, 123);
		tcbor_str(&tcbor, "bar");
		tcbor_array_begin(&tcbor);
		{
			tcbor_int(&tcbor, 456);
			tcbor_int(&tcbor, 789);
			tcbor_map_begin(&tcbor);
			{
				/* Keys can be int type */
				tcbor_int(&tcbor, 1);
				tcbor_int(&tcbor, 100000);

				tcbor_int(&tcbor, 2);
				tcbor_int(&tcbor, 20000000000000);

				tcbor_int(&tcbor, 3);
				tcbor_true(&tcbor);

				tcbor_int(&tcbor, 4);
				tcbor_false(&tcbor);

				tcbor_int(&tcbor, 5);
				tcbor_null(&tcbor);

				tcbor_int(&tcbor, 6);
				/* tag 260 == ipaddress, as defined in
				 * https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml */
				tcbor_tag(&tcbor, 260);
				unsigned char ip[4] = {192, 168, 1, 10};
				tcbor_data(&tcbor, ip, 4);
			}
			tcbor_map_end(&tcbor);
			tcbor_uint(&tcbor, 0xffffffffffffffff);
		}
		tcbor_array_end(&tcbor);
	}
	tcbor_map_end(&tcbor);

	size_t len = tcbor_end(&tcbor);
	assert(len > 0);
	ssize_t res = write(STDOUT_FILENO, buf, len);
	assert(res > 0);

	return 0;
}

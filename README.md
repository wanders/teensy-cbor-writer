Teensy CBOR writer for C
========================

A teensy C-library for serializing (writing) data in CBOR (Concise
Binary Object Representation) as defined in
[RFC-8949](https://datatracker.ietf.org/doc/html/rfc8949)

No deserialization (reading) at all.

Do not confuse with "[tiny-cbor](https://github.com/intel/tinycbor)"
which is a full fledged CBOR library.

Design goals:
* Truly no external dependencies (that means not even calls to strlen
  or memory allocations).
* Serialization only.
* Single header file that can be copied into other projects.
* Simple implementation.

Note that performance is NOT a goal. The hope is that the simple
implementation allows the compiler to optimize the code, but the
serialization of integers of different sizes gets really branchy.


Example use:
------------

```
#include <teensy-cbor.h>

void myfunc(int spam_amount) {
	char buf[1024];

	struct tcbor tcbor;
	tcbor_begin (&tcbor, buf, sizeof (buf));
	tcbor_map_begin (&tcbor);

	/* key */
	tcbor_str (&tcbor, "spam");
	/* value */
	tcbor_int (&tcbor, spam_amount);

	/* key */
	tcbor_str (&tcbor, "ham");
	/* value */
	tcbor_array_begin (&tcbor);
	tcbor_str (&tcbor, "egg");
	tcbor_str (&tcbor, "sausage");
	tcbor_str (&tcbor, "bacon");
	tcbor_array_end (&tcbor);

	tcbor_map_end (&tcbor);

	size_t len = tcbor_end (&tcbor);

	write (STDOUT_FILENO, buf, len);
}
```

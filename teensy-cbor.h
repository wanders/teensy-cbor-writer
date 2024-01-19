/*
 * teensy-cbor.h
 *
 * A teensy C-library for serializing (writing) data in CBOR (Concise
 * Binary Object Representation) as defined in
 * [RFC-8949](https://datatracker.ietf.org/doc/html/rfc8949)
 *
 * Copyright 2024 Anders Waldenborg
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef _TEENSY_CBOR_H
#define _TEENSY_CBOR_H 1

#include <stddef.h>
#include <stdint.h>

/* For internal use only */
#define TCBOR_MAJOR_TYPE_POSITIVE_INT 0
#define TCBOR_MAJOR_TYPE_NEGATIVE_INT 1
#define TCBOR_MAJOR_TYPE_BYTES 2
#define TCBOR_MAJOR_TYPE_TEXT 3
#define TCBOR_MAJOR_TYPE_ARRAY 4
#define TCBOR_MAJOR_TYPE_MAP 5
#define TCBOR_MAJOR_TYPE_TAG 6
#define TCBOR_MAJOR_TYPE_SIMPLE 7

struct _tcbor_buffer {
	size_t used;
	char *buf;
	size_t size;
};

struct tcbor {
	struct _tcbor_buffer buf;
};

/**
 * Initialize serializer #tcbor for writing into #buf that is (at
 * least) #bufsize bytes.
 */
static inline void
tcbor_begin(struct tcbor *tcbor, char *buf, size_t bufsize) {
	tcbor->buf = (struct _tcbor_buffer){0, buf, bufsize};
}

/**
 * Finish serialization. Returns number of bytes used in buffer, or 0
 * on error.
 */
static inline size_t
tcbor_end(struct tcbor *tcbor) {
	if (tcbor->buf.used > tcbor->buf.size)
		return 0;
	return tcbor->buf.used;
}

static inline void
_tcbor_byte(struct _tcbor_buffer *buf, uint8_t val) {
	if (buf->used < buf->size) {
		buf->buf[buf->used] = val;
	}
	buf->used++;
}

static inline void
_tcbor_initial_byte(struct _tcbor_buffer *buf, uint8_t major_type, uint8_t additional_value) {
	_tcbor_byte(buf, (major_type << 5) | additional_value);
}

static inline void
_tcbor_type_arg(struct _tcbor_buffer *buf, uint8_t major_type, uint64_t val) {
	if (val < 24) {
		_tcbor_initial_byte(buf, major_type, val);
	} else if (val < 256) {
		_tcbor_initial_byte(buf, major_type, 24);
		_tcbor_byte(buf, val);
	} else if (val < 65536) {
		_tcbor_initial_byte(buf, major_type, 25);
		_tcbor_byte(buf, val >> 8);
		_tcbor_byte(buf, val);
	} else if (val < 4294967296) {
		_tcbor_initial_byte(buf, major_type, 26);
		_tcbor_byte(buf, val >> 24);
		_tcbor_byte(buf, val >> 16);
		_tcbor_byte(buf, val >> 8);
		_tcbor_byte(buf, val);
	} else {
		_tcbor_initial_byte(buf, major_type, 27);
		_tcbor_byte(buf, val >> 56);
		_tcbor_byte(buf, val >> 48);
		_tcbor_byte(buf, val >> 40);
		_tcbor_byte(buf, val >> 32);
		_tcbor_byte(buf, val >> 24);
		_tcbor_byte(buf, val >> 16);
		_tcbor_byte(buf, val >> 8);
		_tcbor_byte(buf, val);
	}
}

/**
 * Put an unsigned integer into serializer #tcbor.
 */
static inline void
tcbor_uint(struct tcbor *tcbor, uint64_t val) {
	_tcbor_type_arg(&tcbor->buf, TCBOR_MAJOR_TYPE_POSITIVE_INT, val);
}

/**
 * Put a signed integer into serializer #tcbor.
 */
static inline void
tcbor_int(struct tcbor *tcbor, int64_t val) {
	if (val < 0) {
		_tcbor_type_arg(&tcbor->buf, TCBOR_MAJOR_TYPE_NEGATIVE_INT, -(val + 1));
	} else {
		_tcbor_type_arg(&tcbor->buf, TCBOR_MAJOR_TYPE_POSITIVE_INT, val);
	}
}

/**
 * Put a NUL terminated string into serializer #tcbor.
 */
static inline void
tcbor_str(struct tcbor *tcbor, const char *str) {
	size_t len = 0;
	for (; str[len]; len++)
		;

	_tcbor_type_arg(&tcbor->buf, TCBOR_MAJOR_TYPE_TEXT, len);
	for (len = 0; str[len]; len++)
		_tcbor_byte(&tcbor->buf, str[len]);
}

/**
 * Put #len bytes of binary data (pointed to by #d) into serializer
 * #tcbor.
 */
static inline void
tcbor_data(struct tcbor *tcbor, const void *d, size_t len) {
	const char *data = d;
	_tcbor_type_arg(&tcbor->buf, TCBOR_MAJOR_TYPE_BYTES, len);
	for (size_t i = 0; i < len; i++)
		_tcbor_byte(&tcbor->buf, data[i]);
}

/**
 * Start serializing an array into #tcbor. Following entries
 * serialized up to next call to #tcbor_array_end will be part of the
 * array.
 */
static inline void
tcbor_array_begin(struct tcbor *tcbor) {
	/* start indetermine length array */
	_tcbor_initial_byte(&tcbor->buf, TCBOR_MAJOR_TYPE_ARRAY, 31);
}

/**
 * End previous array started with #tcbor_array_begin.
 */
static inline void
tcbor_array_end(struct tcbor *tcbor) {
	/* "break" marker */
	_tcbor_initial_byte(&tcbor->buf, TCBOR_MAJOR_TYPE_SIMPLE, 31);
}

/**
 * Start serializing a map into #tcbor. Following pair of entries
 * serialized up to next call to #tcbor_array_end will be key and
 * values in the map.
 */
static inline void
tcbor_map_begin(struct tcbor *tcbor) {
	/* start indetermine length map */
	_tcbor_initial_byte(&tcbor->buf, TCBOR_MAJOR_TYPE_MAP, 31);
}

/**
 * End previous map started with #tcbor_map_begin.
 */
static inline void
tcbor_map_end(struct tcbor *tcbor) {
	/* "break" marker */
	_tcbor_initial_byte(&tcbor->buf, TCBOR_MAJOR_TYPE_SIMPLE, 31);
}

static inline void
tcbor_false(struct tcbor *tcbor) {
	_tcbor_initial_byte(&tcbor->buf, TCBOR_MAJOR_TYPE_SIMPLE, 20);
}

static inline void
tcbor_true(struct tcbor *tcbor) {
	_tcbor_initial_byte(&tcbor->buf, TCBOR_MAJOR_TYPE_SIMPLE, 21);
}

static inline void
tcbor_null(struct tcbor *tcbor) {
	_tcbor_initial_byte(&tcbor->buf, TCBOR_MAJOR_TYPE_SIMPLE, 22);
}

static inline void
tcbor_tag(struct tcbor *tcbor, uint64_t tag) {
	_tcbor_type_arg(&tcbor->buf, 6, tag);
}

#endif

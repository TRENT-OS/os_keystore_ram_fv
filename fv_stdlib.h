/*
 ----------------------------------------------------------------------
Copyright © 2005-2014 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
----------------------------------------------------------------------
*/

#pragma once

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef long unsigned int uint64_t;
typedef long unsigned int size_t;

typedef unsigned long int uintptr_t;

#define NULL 0
#define true 1
#define false 0

#define UCHAR_MAX	255

#define Memory_Config_STATIC
#define Debug_ASSERT(x)
#define Debug_ASSERT_SELF(x)
#define Debug_LOG_ERROR(...)
#define Debug_LOG_INFO(...)

size_t strlen(const char *s)
{
	size_t w = 0;
	for (; *s; s++, w++);
	return w;
}

void *memset(void *dest, int c, size_t n)
{
	unsigned char *s = dest;
	for (; n > 0; s++, n--)
		*s = c;
	return dest;
}

int memcmp(const void *vl, const void *vr, size_t n)
{
	const unsigned char *l=vl, *r=vr;
	for (; n && *l == *r; n--, l++, r++);
	return n ? *l-*r : 0;
}

char *strncpy (char *dest, const char *src, size_t n)
{
	char *d;
	const char *s;
	for (d = dest, s = src; n && *s; d++, s++, n--)
	  *d = *s;
	for (; n; d++, n--)
		*d = '\0';
	return dest;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	for (; n; n--)
	{
		*d = *s;
		s++;
		d++;
	}
	return dest;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	const unsigned char *p1 = (void *) s1, *p2 = (void *) s2;
	if (!n) return 0;
	for (n = n - 1; *p1 && *p2 && n && *p1 == *p2; p1++, p2++, n--);
	return *p1 - *p2;
}

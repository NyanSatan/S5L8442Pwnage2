/*
 * stolen from @xerub and @apple
 */

#ifndef LIBC_C
#define LIBC_C

#include <stddef.h>

#define MAYBE_UNUSED __attribute__((unused)) static

MAYBE_UNUSED
int
memcmp(const void *b1, const void *b2, size_t len)
{
  register long n = len;
  unsigned char *m1 = (unsigned char *)b1;
  unsigned char *m2 = (unsigned char *)b2;
  
  while (n-- && (*m1 == *m2)) {
    m1++;
    m2++;
  }
  
  return ((n < 0) ? 0 : (*m1 - *m2));
}

#define UCHAR_MAX 255

MAYBE_UNUSED
unsigned char *
boyermoore_horspool_memmem(const unsigned char* haystack, size_t hlen,
                           const unsigned char* needle,   size_t nlen)
{
    size_t last, scan = 0;
    size_t bad_char_skip[UCHAR_MAX + 1]; /* Officially called:
                                          * bad character shift */

    /* Sanity checks on the parameters */
    if (nlen <= 0 || !haystack || !needle)
        return NULL;

    /* ---- Preprocess ---- */
    /* Initialize the table to default value */
    /* When a character is encountered that does not occur
     * in the needle, we can safely skip ahead for the whole
     * length of the needle.
     */
    for (scan = 0; scan <= UCHAR_MAX; scan = scan + 1)
        bad_char_skip[scan] = nlen;

    /* C arrays have the first byte at [0], therefore:
     * [nlen - 1] is the last byte of the array. */
    last = nlen - 1;

    /* Then populate it with the analysis of the needle */
    for (scan = 0; scan < last; scan = scan + 1)
        bad_char_skip[needle[scan]] = last - scan;

    /* ---- Do the matching ---- */

    /* Search the haystack, while the needle can still be within it. */
    while (hlen >= nlen)
    {
        /* scan from the end of the needle */
        for (scan = last; haystack[scan] == needle[scan]; scan = scan - 1)
            if (scan == 0) /* If the first byte matches, we've found it. */
                return (void *)haystack;

        /* otherwise, we need to skip some bytes and start again.
           Note that here we are getting the skip value based on the last byte
           of needle, no matter where we didn't match. So if needle is: "abcd"
           then we are skipping based on 'd' and that value will be 4, and
           for "abcdd" we again skip on 'd' but the value will be only 1.
           The alternative of pretending that the mismatched character was
           the last character is slower in the normal case (E.g. finding
           "abcd" in "...azcd..." gives 4 by using 'd' but only
           4-2==2 using 'z'. */
        hlen     -= bad_char_skip[haystack[last]];
        haystack += bad_char_skip[haystack[last]];
    }

    return NULL;
}

MAYBE_UNUSED
void *
memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen)
{
    const unsigned char *h;
    const unsigned char *n;
    if (!nlen) {
        return (void *)haystack;
    }
    if (nlen > hlen) {
        return NULL;
    }
    if (nlen >= 4 && hlen >= 256) {
        return boyermoore_horspool_memmem(haystack, hlen, needle, nlen);
    }
    for (h = haystack, n = needle; hlen >= nlen; hlen--, h++) {
        if (*h == *n && !memcmp(h + 1, n + 1, nlen - 1)) {
            return (char *)h;
        }
    }
    return NULL;
}

MAYBE_UNUSED
void
*memcpy(void *dst, const void *src, size_t len)
{
  const char *s = src;
  char       *d = dst;
  int        pos = 0, dir = 1;

  if (d > s) {
    dir = -1;
    pos = len - 1;
  }

  while (len--) {
    d[pos] = s[pos];
    pos += dir;
  }

  return dst;
}

MAYBE_UNUSED
size_t
strlen(const char *str)
{
	register const char *s;

	for (s = str; *s; ++s);
	return(s - str);
}

#endif

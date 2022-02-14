/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void *memset(void *b, int c, uintptr_t len)
{
    for (uintptr_t i = 0; i < len; i++)
    {
        ((uint8_t *)b)[i] = (uint8_t)c;
    }

    return b;
}

int(memcmp)(const void *s1, const void *s2, size_t n)
{
    if (n != 0)
    {
        const unsigned char *p1 = s1, *p2 = s2;

        do
        {
            if (*p1++ != *p2++)
                return (*--p1 - *--p2);
        } while (--n != 0);
    }
    return (0);
}

void *memcpy(void *dst, const void *src, uintptr_t n)
{
    for (uintptr_t i = 0; i < n; i++)
    {
        ((uint8_t *)dst)[i] = ((uint8_t *)src)[i];
    }

    return dst;
}

size_t strlen(const char *s)
{
    size_t cnt = 0;

    while (*s)
    {
        cnt++;
        s++;
    }

    return cnt;
}

int strcmp(const char *s1, const char *s2)
{
    do
    {
        if (*s1 != *s2++)
            return (*(const unsigned char *)s1 -
                    *(const unsigned char *)(s2 - 1));
        if (*s1++ == '\0')
            break;
    } while (1);
    return (0);
}

int strncmp(const char *s1, const char *s2, size_t n)
{

    if (n == 0)
        return (0);
    do
    {
        if (*s1 != *s2++)
            return (*(const unsigned char *)s1 -
                    *(const unsigned char *)(s2 - 1));
        if (*s1++ == '\0')
            break;
    } while (--n != 0);
    return (0);
}

int strcasecmp(const char *s1, const char *s2)
{
    const uint8_t *us1 = (const uint8_t *)s1, *us2 = (const uint8_t *)s2;

    while (tolower(*us1) == tolower(*us2))
    {
        if (*us1++ == '\0')
            return (0);
        us2++;
    }
    return (tolower(*us1) - tolower(*us2));
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{

    if (n != 0)
    {
        const uint8_t *us1 = (const uint8_t *)s1;
        const uint8_t *us2 = (const uint8_t *)s2;

        do
        {
            if (tolower(*us1) != tolower(*us2))
                return (tolower(*us1) - tolower(*us2));
            if (*us1++ == '\0')
                break;
            us2++;
        } while (--n != 0);
    }
    return (0);
}

char *strdup(const char *s1)
{
    size_t len = strlen(s1) + 1;
    char *ret = (char *)malloc(len);
    memcpy(ret, s1, len);
    return ret;
}

char *strstr(const char *s, const char *find)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != 0)
    {
        len = strlen(find);
        do
        {
            do
            {
                if ((sc = *s++) == '\0')
                    return (NULL);
            } while (sc != c);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

char *(strcpy)(char *to, const char *from)
{
    char *save = to;

    for (; (*to = *from) != 0; ++from, ++to)
        ;
    return (save);
}

size_t strlcpy(char *dst, const char *src, size_t dsize)
{
    const char *osrc = src;
    size_t nleft = dsize;

    /* Copy as many bytes as will fit. */
    if (nleft != 0)
    {
        while (--nleft != 0)
        {
            if ((*dst++ = *src++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src. */
    if (nleft == 0)
    {
        if (dsize != 0)
            *dst = '\0'; /* NUL-terminate dst */
        while (*src++)
            ;
    }

    return (src - osrc - 1); /* count does not include NUL */
}

char *strcat(char *s, const char *append)
{
    char *save = s;

    for (; *s; ++s)
        ;
    while ((*s++ = *append++) != 0)
        ;
    return (save);
}

long strtol(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    unsigned long acc;
    unsigned char c;
    unsigned long cutoff;
    int neg = 0, any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do
    {
        c = *s++;
    } while (isspace(c));
    if (c == '-')
    {
        neg = 1;
        c = *s++;
    }
    else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X'))
    {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? -(unsigned long)0x80000000 : 0x7FFFFFFF;
    cutlim = cutoff % (unsigned long)base;
    cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++)
    {
        if (!isascii(c))
            break;
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else
        {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0)
    {
        acc = neg ? 0x80000000 : 0x7FFFFFFF;
    }
    else if (neg)
        acc = -acc;
    if (endptr != NULL)
        *endptr = (char *)(any ? s - 1 : nptr);
    return (acc);
}

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-unicode.c - utf-8 support functions for gal
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>

#include "e-unicode.h"
#include "e-iconv.h"

#include <string.h>
#include <iconv.h>
#include <stdlib.h>

gchar *
e_utf8_to_iconv_string_sized (iconv_t ic, const gchar *string, gint bytes)
{
	char *new, *ob;
	const char *ib;
	size_t ibl, obl;

	if (!string) return NULL;

	if (ic == (iconv_t) -1) {
		gint len;
		const gchar *u;
		gunichar uc;

		new = g_new (unsigned char, bytes * 4 + 1);
		u = string;
		len = 0;

		while ((u) && (u - string < bytes)) {
			u = e_unicode_get_utf8 (u, &uc);
			new[len++] = uc & 0xff;
		}
		new[len] = '\0';
		return new;
	}

	ib = string;
	ibl = bytes;
	new = ob = g_new (gchar, ibl * 4 + 1);
	obl = ibl * 4 + 1;

	while (ibl > 0) {
		e_iconv (ic, &ib, &ibl, &ob, &obl);
		if (ibl > 0) {
			gint len;
			if ((*ib & 0x80) == 0x00) len = 1;
			else if ((*ib &0xe0) == 0xc0) len = 2;
			else if ((*ib &0xf0) == 0xe0) len = 3;
			else if ((*ib &0xf8) == 0xf0) len = 4;
			else {
				g_warning ("Invalid UTF-8 sequence");
				break;
			}
			ib += len;
			ibl = bytes - (ib - string);
			if (ibl > bytes) ibl = 0;
			*ob++ = '_';
			obl--;
		}
	}

	*ob = '\0';

	return new;
}

gchar *
e_utf8_to_locale_string_sized (const gchar *string, gint bytes)
{
	iconv_t ic;
	char *ret;

	if (!string) return NULL;

	ic = e_iconv_open(e_iconv_locale_charset(), "utf-8");
	ret = e_utf8_to_iconv_string_sized (ic, string, bytes);
	e_iconv_close(ic);

	return ret;
}

gchar *
e_utf8_to_locale_string (const gchar *string)
{
	if (!string) return NULL;
	return e_utf8_to_locale_string_sized (string, strlen (string));
}

gchar *
e_unicode_get_utf8 (const gchar *text, gunichar *out)
{
	*out = g_utf8_get_char (text);
	return (*out == (gunichar)-1) ? NULL : g_utf8_next_char (text);
}


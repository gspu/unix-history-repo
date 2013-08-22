/*
 * Copyright (C) 2011  Internet Systems Consortium, Inc. ("ISC")
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: clientinfo.c,v 1.3 2011/10/11 00:25:12 marka Exp $ */

/*! \file */

#include "config.h"

#include <dns/clientinfo.h>

void
dns_clientinfomethods_init(dns_clientinfomethods_t *methods,
			   dns_clientinfo_sourceip_t sourceip)
{
	methods->version = DNS_CLIENTINFOMETHODS_VERSION;
	methods->age = DNS_CLIENTINFOMETHODS_AGE;
	methods->sourceip = sourceip;
}

void
dns_clientinfo_init(dns_clientinfo_t *ci, void *data) {
	ci->version = DNS_CLIENTINFO_VERSION;
	ci->data = data;
}

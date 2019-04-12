/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "inner.h"

#define ORDER_LEN   ((BR_MAX_EC_SIZE + 7) >> 3)

/* see bearssl_ec.h */
size_t
br_ecdsa_i31_sign_asn1(const br_ec_impl *impl,
	const br_hash_class *hf, const void *hash_value,
	const br_ec_private_key *sk, void *sig)
{
	unsigned char rsig[(ORDER_LEN << 1) + 12];
	size_t sig_len;

	sig_len = br_ecdsa_i31_sign_raw(impl, hf, hash_value, sk, rsig);
	if (sig_len == 0) {
		return 0;
	}
	sig_len = br_ecdsa_raw_to_asn1(rsig, sig_len);
	memcpy(sig, rsig, sig_len);
	return sig_len;
}

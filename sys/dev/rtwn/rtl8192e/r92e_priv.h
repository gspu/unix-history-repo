/*-
 * Copyright (c) 2017 Kevin Lo <kevlo@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef R92E_PRIV_H
#define R92E_PRIV_H

/*
 * MAC initialization values.
 */
static const struct rtwn_mac_prog rtl8192eu_mac[] = {
	{ 0x011, 0xeb }, { 0x012, 0x07 }, { 0x014, 0x75 }, { 0x303, 0xa7 },
	{ 0x428, 0x0a }, { 0x429, 0x10 }, { 0x430, 0x00 }, { 0x431, 0x00 },
	{ 0x432, 0x00 }, { 0x433, 0x01 }, { 0x434, 0x04 }, { 0x435, 0x05 },
	{ 0x436, 0x07 }, { 0x437, 0x08 }, { 0x43c, 0x04 }, { 0x43d, 0x05 },
	{ 0x43e, 0x07 }, { 0x43f, 0x08 }, { 0x440, 0x5d }, { 0x441, 0x01 },
	{ 0x442, 0x00 }, { 0x444, 0x10 }, { 0x445, 0x00 }, { 0x446, 0x00 },
	{ 0x447, 0x00 }, { 0x448, 0x00 }, { 0x449, 0xf0 }, { 0x44a, 0x0f },
	{ 0x44b, 0x3e }, { 0x44c, 0x10 }, { 0x44d, 0x00 }, { 0x44e, 0x00 },
	{ 0x44f, 0x00 }, { 0x450, 0x00 }, { 0x451, 0xf0 }, { 0x452, 0x0f },
	{ 0x453, 0x00 }, { 0x456, 0x5e }, { 0x460, 0x66 }, { 0x461, 0x66 },
	{ 0x4c8, 0xff }, { 0x4c9, 0x08 }, { 0x4cc, 0xff }, { 0x4cd, 0xff },
	{ 0x4ce, 0x01 }, { 0x500, 0x26 }, { 0x501, 0xa2 }, { 0x502, 0x2f },
	{ 0x503, 0x00 }, { 0x504, 0x28 }, { 0x505, 0xa3 }, { 0x506, 0x5e },
	{ 0x507, 0x00 }, { 0x508, 0x2b }, { 0x509, 0xa4 }, { 0x50a, 0x5e },
	{ 0x50b, 0x00 }, { 0x50c, 0x4f }, { 0x50d, 0xa4 }, { 0x50e, 0x00 },
	{ 0x50f, 0x00 }, { 0x512, 0x1c }, { 0x514, 0x0a }, { 0x516, 0x0a },
	{ 0x525, 0x4f }, { 0x540, 0x12 }, { 0x541, 0x64 }, { 0x550, 0x10 },
	{ 0x551, 0x10 }, { 0x559, 0x02 }, { 0x55c, 0x50 }, { 0x55d, 0xff },
	{ 0x605, 0x30 }, { 0x608, 0x0e }, { 0x609, 0x2a }, { 0x620, 0xff },
	{ 0x621, 0xff }, { 0x622, 0xff }, { 0x623, 0xff }, { 0x624, 0xff },
	{ 0x625, 0xff }, { 0x626, 0xff }, { 0x627, 0xff }, { 0x638, 0x50 },
	{ 0x63c, 0x0a }, { 0x63d, 0x0a }, { 0x63e, 0x0e }, { 0x63f, 0x0e },
	{ 0x640, 0x40 }, { 0x642, 0x40 }, { 0x643, 0x00 }, { 0x652, 0xc8 },
	{ 0x66e, 0x05 }, { 0x700, 0x21 }, { 0x701, 0x43 }, { 0x702, 0x65 },
	{ 0x703, 0x87 }, { 0x708, 0x21 }, { 0x709, 0x43 }, { 0x70a, 0x65 },
	{ 0x70b, 0x87 }
};


/*
 * Baseband initialization values.
 */
static const uint16_t rtl8192eu_bb_regs[] = {
	0x800, 0x804, 0x808, 0x80c, 0x810, 0x814, 0x818, 0x81c, 0x820,
	0x824, 0x828, 0x82c, 0x830, 0x834, 0x838, 0x83c, 0x840, 0x844,
	0x848, 0x84c, 0x850, 0x854, 0x858, 0x85c, 0x860, 0x864, 0x868,
	0x86c, 0x870, 0x874, 0x878, 0x87c, 0x880, 0x884, 0x888, 0x88c,
	0x890, 0x894, 0x898, 0x900, 0x904, 0x908, 0x90c, 0x910, 0x914,
	0x918, 0x91c, 0x924, 0x928, 0x92c, 0x930, 0x934, 0x938, 0x93c,
	0x940, 0x944, 0x94c, 0xa00, 0xa04, 0xa08, 0xa0c, 0xa10, 0xa14,
	0xa18, 0xa1c, 0xa20, 0xa24, 0xa28, 0xa2c, 0xa70, 0xa74, 0xa78,
	0xa7c, 0xa80, 0xb38, 0xc00, 0xc04, 0xc08, 0xc0c, 0xc10, 0xc14,
	0xc18, 0xc1c, 0xc20, 0xc24, 0xc28, 0xc2c, 0xc30, 0xc34, 0xc38,
	0xc3c, 0xc40, 0xc44, 0xc48, 0xc4c, 0xc50, 0xc54, 0xc58, 0xc5c,
	0xc60, 0xc64, 0xc68, 0xc6c, 0xc70, 0xc74, 0xc78, 0xc7c, 0xc80,
	0xc84, 0xc88, 0xc8c, 0xc90, 0xc94, 0xc98, 0xc9c, 0xca0, 0xca4,
	0xca8, 0xcac, 0xcb0, 0xcb4, 0xcb8, 0xcbc, 0xcc0, 0xcc4, 0xcc8,
	0xccc, 0xcd0, 0xcd4, 0xcd8, 0xcdc, 0xce0, 0xce4, 0xce8, 0xcec,
	0xd00, 0xd04, 0xd08, 0xd0c, 0xd10, 0xd14, 0xd18, 0xd1c, 0xd2c,
	0xd30, 0xd34, 0xd38, 0xd3c, 0xd40, 0xd44, 0xd48, 0xd4c, 0xd50,
	0xd54, 0xd58, 0xd5c, 0xd60, 0xd64, 0xd68, 0xd6c, 0xd70, 0xd74,
	0xd78, 0xd80, 0xd84, 0xd88, 0xe00, 0xe04, 0xe08, 0xe10, 0xe14,
	0xe18, 0xe1c, 0xe28, 0xe30, 0xe34, 0xe38, 0xe3c, 0xe40, 0xe44,
	0xe48, 0xe4c, 0xe50, 0xe54, 0xe58, 0xe5c, 0xe60, 0xe68, 0xe6c,
	0xe70, 0xe74, 0xe78, 0xe7c, 0xe80, 0xe84, 0xe88, 0xe8c, 0xed0,
	0xed4, 0xed8, 0xedc, 0xee0, 0xeec, 0xee4, 0xee8, 0xf14, 0xf4c,
	0xf00
};

static const uint32_t rtl8192eu_bb_vals[] = {
	0x80040000, 0x00000003, 0x0000fc00, 0x0000000a, 0x10001331,
	0x020c3d10, 0x02220385, 0x00000000, 0x01000100, 0x00390204,
	0x01000100, 0x00390204, 0x32323232, 0x30303030, 0x30303030,
	0x30303030, 0x00010000, 0x00010000, 0x28282828, 0x28282828,
	0x00000000, 0x00000000, 0x009a009a, 0x01000014, 0x66f60000,
	0x061f0000, 0x30303030, 0x30303030, 0x00000000, 0x55004200,
	0x08080808, 0x00000000, 0xb0000c1c, 0x00000001, 0x00000000,
	0xcc0000c0, 0x00000800, 0xfffffffe, 0x40302010, 0x00000000,
	0x00000023, 0x00000000, 0x81121313, 0x806c0001, 0x00000001,
	0x00000000, 0x00010000, 0x00000001, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000008, 0x00d0c7c8, 0x81ff000c, 0x8c838300,
	0x2e68120f, 0x95009b78, 0x1114d028, 0x00881117, 0x89140f00,
	0x1a1b0000, 0x090e1317, 0x00000204, 0x00d30000, 0x101fff00,
	0x00000007, 0x00000900, 0x225b0606, 0x218075b1, 0x00000000,
	0x48071d40, 0x03a05633, 0x000000e4, 0x6c6c6c6c, 0x08800000,
	0x40000100, 0x08800000, 0x40000100, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x69e9ac47, 0x469652af, 0x49795994,
	0x0a97971c, 0x1f7c403f, 0x000100b7, 0xec020107, 0x007f037f,
	0x00340020, 0x0080801f, 0x00000020, 0x00248492, 0x00000000,
	0x7112848b, 0x47c00bff, 0x00000036, 0x00000600, 0x02013169,
	0x0000001f, 0x00b91612, 0x40000100, 0x21f60000, 0x40000100,
	0xa0e40000, 0x00121820, 0x00000000, 0x00121820, 0x00007f7f,
	0x00000000, 0x000300a0, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x28000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x64b22427,
	0x00766932, 0x00222222, 0x00040000, 0x77644302, 0x2f97d40c,
	0x00080740, 0x00020403, 0x0000907f, 0x20010201, 0xa0633333,
	0x3333bc43, 0x7a8f5b6b, 0x0000007f, 0xcc979975, 0x00000000,
	0x80608000, 0x00000000, 0x00127353, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x6437140a, 0x00000000, 0x00000282,
	0x30032064, 0x4653de68, 0x04518a3c, 0x00002101, 0x2a201c16,
	0x1812362e, 0x322c2220, 0x000e3c24, 0x01081008, 0x00000800,
	0xf0b50000, 0x30303030, 0x30303030, 0x03903030, 0x30303030,
	0x30303030, 0x30303030, 0x30303030, 0x00000000, 0x1000dc1f,
	0x10008c1f, 0x02140102, 0x681604c2, 0x01007c00, 0x01004800,
	0xfb000000, 0x000028d1, 0x1000dc1f, 0x10008c1f, 0x02140102,
	0x28160d05, 0x00000008, 0x0fc05656, 0x03c09696, 0x03c09696,
	0x0c005656, 0x0c005656, 0x0c005656, 0x0c005656, 0x03c09696,
	0x0c005656, 0x03c09696, 0x03c09696, 0x03c09696, 0x03c09696,
	0x0000d6d6, 0x0000d6d6, 0x0fc01616, 0xb0000c1c, 0x00000001,
	0x00000003, 0x00000000, 0x00000300
};

static const struct rtwn_bb_prog rtl8192eu_bb[] = {
	{
		nitems(rtl8192eu_bb_regs),
		rtl8192eu_bb_regs,
		rtl8192eu_bb_vals,
		{ 0 },
		NULL
	}
};


static const uint32_t rtl8192eu_agc_vals[] = {
	0xfb000001, 0xfb010001, 0xfb020001, 0xfb030001, 0xfb040001,
	0xfb050001, 0xfa060001, 0xf9070001, 0xf8080001, 0xf7090001,
	0xf60a0001, 0xf50b0001, 0xf40c0001, 0xf30d0001, 0xf20e0001,
	0xf10f0001, 0xf0100001, 0xef110001, 0xee120001, 0xed130001,
	0xec140001, 0xeb150001, 0xea160001, 0xe9170001, 0xe8180001,
	0xe7190001, 0xc81a0001, 0xc71b0001, 0xc61c0001, 0x071d0001,
	0x061e0001, 0x051f0001, 0x04200001, 0x03210001, 0xaa220001,
	0xa9230001, 0xa8240001, 0xa7250001, 0xa6260001, 0x85270001,
	0x84280001, 0x83290001, 0x252a0001, 0x242b0001, 0x232c0001,
	0x222d0001, 0x672e0001, 0x662f0001, 0x65300001, 0x64310001,
	0x63320001, 0x62330001, 0x61340001, 0x45350001, 0x44360001,
	0x43370001, 0x42380001, 0x41390001, 0x403a0001, 0x403b0001,
	0x403c0001, 0x403d0001, 0x403e0001, 0x403f0001, 0xfb400001,
	0xfb410001, 0xfb420001, 0xfb430001, 0xfb440001, 0xfb450001,
	0xfa460001, 0xf9470001, 0xf8480001, 0xf7490001, 0xf64a0001,
	0xf54b0001, 0xf44c0001, 0xf34d0001, 0xf24e0001, 0xf14f0001,
	0xf0500001, 0xef510001, 0xee520001, 0xed530001, 0xec540001,
	0xeb550001, 0xea560001, 0xe9570001, 0xe8580001, 0xe7590001,
	0xe65a0001, 0xe55b0001, 0xe45c0001, 0xe35d0001, 0xe25e0001,
	0xe15f0001, 0x8a600001, 0x89610001, 0x88620001, 0x87630001,
	0x86640001, 0x85650001, 0x84660001, 0x83670001, 0x82680001,
	0x6b690001, 0x6a6a0001, 0x696b0001, 0x686c0001, 0x676d0001,
	0x666e0001, 0x656f0001, 0x64700001, 0x63710001, 0x62720001,
	0x61730001, 0x49740001, 0x48750001, 0x47760001, 0x46770001,
	0x45780001, 0x44790001, 0x437a0001, 0x427b0001, 0x417c0001,
	0x407d0001, 0x407e0001, 0x407f0001
};

static const struct rtwn_agc_prog rtl8192eu_agc[] = {
	{
		nitems(rtl8192eu_agc_vals),
		rtl8192eu_agc_vals,
		{ 0 },
		NULL
	}
};

/*
 * RF initialization values.
 */
static const uint8_t rtl8192eu_rf0_regs[] = {
	0x7f, 0x81, 0x00, 0x08, 0x18, 0x19, 0x1b, 0x1e, 0x1f, 0x2f, 0x3f,
	0x42, 0x57, 0x58, 0x67, 0x83, 0xb0, 0xb1, 0xb2, 0xb4, 0xb5, 0xb6,
	0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbf, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6,
	0xc7, 0xc8, 0xc9, 0xca, 0xdf, 0xef, 0x51, 0x52, 0x53, 0x56, 0x35,
	0x35, 0x35, 0x36, 0x36, 0x36, 0x36, 0x18, 0x5a, 0x19, 0x34, 0x34,
	0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x00, 0x84,
	0x86, 0x87, 0x8e, 0x8f, 0xef, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b,
	0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0xef,
	0x18, 0x1e, 0x1f, 0x00
}, rtl8192eu_rf1_regs[] = {
	0x7f, 0x81, 0x00, 0x08, 0x18, 0x19, 0x1b, 0x1e, 0x1f, 0x2f, 0x3f,
	0x42, 0x57, 0x58, 0x67, 0x7f, 0x81, 0x83, 0xdf, 0xef, 0x51, 0x52,
	0x53, 0x56, 0x35, 0x35, 0x35, 0x36, 0x36, 0x36, 0x36, 0x18, 0x5a,
	0x19, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34,
	0x34, 0x00, 0x84, 0x86, 0x87, 0x8e, 0x8f, 0xef, 0x3b, 0x3b, 0x3b,
	0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b,
	0x3b, 0x3b, 0xef, 0x00, 0x1e, 0x1f, 0x00
};

static const uint32_t rtl8192eu_rf0_vals[] = {
	0x00082, 0x3fc00, 0x30000, 0x08400, 0x00407, 0x00012, 0x00064,
	0x80009, 0x00880, 0x1a060, 0x00000, 0x060c0, 0xd0000, 0xbe180,
	0x01552, 0x00000, 0xff9f1, 0x55418, 0x8cc00, 0x43083, 0x08166,
	0x0803e, 0x1c69f, 0x0407f, 0x80001, 0x40001, 0x00400, 0xc0000,
	0x02400, 0x00009, 0x40c91, 0x99999, 0x000a3, 0x88820, 0x76c06,
	0x00000, 0x80000, 0x00180, 0x001a0, 0x69545, 0x7e45e, 0x00071,
	0x51ff3, 0x000a8, 0x001e2, 0x002a8, 0x01c24, 0x09c24, 0x11c24,
	0x19c24, 0x00c07, 0x48000, 0x739d0, 0x0add7, 0x09dd4, 0x08dd1,
	0x07dce, 0x06dcb, 0x05dc8, 0x04dc5, 0x034cc, 0x0244f, 0x0144c,
	0x00014, 0x30159, 0x68180, 0x0014e, 0x48e00, 0x65540, 0x88000,
	0x020a0, 0xf02b0, 0xef7b0, 0xd4fb0, 0xcf060, 0xb0090, 0xa0080,
	0x90080, 0x8f780, 0x78730, 0x60fb0, 0x5ffa0, 0x40620, 0x37090,
	0x20080, 0x1f060, 0x0ffb0, 0x000a0, 0x0fc07, 0x00001, 0x80000,
	0x33e70
}, rtl8192eu_rf1_vals[] = {
	0x00082, 0x3fc00, 0x30000, 0x08400, 0x00407, 0x00012, 0x00064,
	0x80009, 0x00880, 0x1a060, 0x00000, 0x060c0, 0xd0000, 0xbe180,
	0x01552, 0x00082, 0x3f000, 0x00000, 0x00180, 0x001a0, 0x69545,
	0x7e42e, 0x00071, 0x51ff3, 0x000a8, 0x001e0, 0x002a8, 0x01ca8,
	0x09c24, 0x11c24, 0x19c24, 0x00c07, 0x48000, 0x739d0, 0x0add7,
	0x09dd4, 0x08dd1, 0x07dce, 0x06dcb, 0x05dc8, 0x04dc5, 0x034cc,
	0x0244f, 0x0144c, 0x00014, 0x30159, 0x68180, 0x000ce, 0x48a00,
	0x65540, 0x88000, 0x020a0, 0xf02b0, 0xef7b0, 0xd4fb0, 0xcf060,
	0xb0090, 0xa0080, 0x90080, 0x8f780, 0x78730, 0x60fb0, 0x5ffa0,
	0x40620, 0x37090, 0x20080, 0x1f060, 0x0ffb0, 0x000a0, 0x10159,
	0x00001, 0x80000, 0x33e70
};

static const struct rtwn_rf_prog rtl8192eu_rf[] = {
	/* RF chain 0. */
	{
		nitems(rtl8192eu_rf0_regs),
		rtl8192eu_rf0_regs,
		rtl8192eu_rf0_vals,
		{ 0 },
		NULL
	},
	{ 0, NULL, NULL, { 0 },	NULL },
	/* RF chain 1. */
	{
		nitems(rtl8192eu_rf1_regs),
		rtl8192eu_rf1_regs,
		rtl8192eu_rf1_vals,
		{ 0 },
		NULL
	},
	{ 0, NULL, NULL, { 0 }, NULL }
};

#endif	/* R92E_PRIV_H */

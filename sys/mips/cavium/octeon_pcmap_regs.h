/***********************license start***************
 *  Copyright (c) 2003-2008 Cavium Networks (support@cavium.com). All rights
 *  reserved.
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials provided
 *        with the distribution.
 *
 *      * Neither the name of Cavium Networks nor the names of
 *        its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written
 *        permission.
 *
 *  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND CAVIUM NETWORKS MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
 *  REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
 *  DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR
 *  PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET
 *  POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING OUT
 *  OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 *
 *
 *  For any questions regarding licensing please contact marketing@caviumnetworks.com
 *
 ***********************license end**************************************/

/*
 *    This product includes software developed by the University of
 *    California, Berkeley and its contributors."
 */

/* $FreeBSD$ */

#ifndef __OCTEON_PCMAP_REGS_H__
#define __OCTEON_PCMAP_REGS_H__

#ifndef LOCORE

/*
 * Utility inlines & macros
 */

#if defined(__mips_n64)
#define	oct_write64(a, v)	(*(volatile uint64_t *)(a) = (uint64_t)(v))

#define	OCT_READ(n, t)							\
static inline t oct_read ## n(uintptr_t a)				\
{									\
	volatile t *p = (volatile t *)a;				\
	return (*p);							\
}

OCT_READ(64, uint64_t);

#elif defined(__mips_n32) || defined(__mips_o32)
#if defined(__mips_n32)
static inline void oct_write64 (uint64_t csr_addr, uint64_t val64)
{
    __asm __volatile (
	    ".set push\n"
            ".set mips64\n"
            "sd     %0, 0(%1)\n"
            ".set pop\n"
            :
	    : "r"(val64), "r"(csr_addr));
}

#define	OCT_READ(n, t, insn)						\
static inline t oct_read ## n(uint64_t a)				\
{									\
    uint64_t tmp;							\
									\
    __asm __volatile (							\
	".set push\n"							\
        ".set mips64\n"							\
        insn "\t%0, 0(%1)\n"						\
        ".set pop\n"							\
        : "=r"(tmp)							\
        : "r"(a));							\
    return ((t)tmp);							\
}

OCT_READ(64, uint64_t, "ld");
#else

/*
 * XXX
 * Add o32 variants that load the address into a register and the result out
 * of a register properly, and simply disable interrupts before and after and
 * hope that we don't need to refill or modify the TLB to access the address.
 * I'd be a lot happier if csr_addr were a physical address and we mapped it
 * into XKPHYS here so that we could guarantee that interrupts were the only
 * kind of exception we needed to worry about.
 *
 * Also, some of this inline assembly is needlessly verbose.  Oh, well.
 */
static inline void oct_write64 (uint64_t csr_addr, uint64_t val64)
{
	uint32_t csr_addrh = csr_addr >> 32;
	uint32_t csr_addrl = csr_addr;
	uint32_t valh = val64 >> 32;
	uint32_t vall = val64;
	uint32_t tmp1;
	uint32_t tmp2;
	uint32_t tmp3;
	register_t sr;

	sr = intr_disable();

	__asm __volatile (
	    ".set push\n"
            ".set mips64\n"
	    ".set noreorder\n"
	    ".set noat\n"
	    "dsll   %0, %3, 32\n"
	    "dsll   %1, %5, 32\n"
	    "dsll   %2, %4, 32\n"
	    "dsrl   %2, %2, 32\n"
	    "or     %0, %0, %2\n"
	    "dsll   %2, %6, 32\n"
	    "dsrl   %2, %2, 32\n"
	    "or     %1, %1, %2\n"
	    "sd     %0, 0(%1)\n"
            ".set pop\n"
	    : "=&r" (tmp1), "=&r" (tmp2), "=&r" (tmp3)
	    : "r" (valh), "r" (vall), "r" (csr_addrh), "r" (csr_addrl));

	intr_restore(sr);
}

static inline uint64_t oct_read64 (uint64_t csr_addr)
{
	uint32_t csr_addrh = csr_addr >> 32;
	uint32_t csr_addrl = csr_addr;
	uint32_t valh;
	uint32_t vall;
	register_t sr;

	sr = intr_disable();

	__asm __volatile (
	    ".set push\n"
            ".set mips64\n"
	    ".set noreorder\n"
	    ".set noat\n"
	    "dsll   %0, %2, 32\n"
	    "dsll   %1, %3, 32\n"
	    "dsrl   %1, %1, 32\n"
	    "or     %0, %0, %1\n"
	    "ld     %1, 0(%0)\n"
	    "dsrl   %0, %1, 32\n"
	    "dsll   %1, %1, 32\n"
	    "dsrl   %1, %1, 32\n"
	    ".set pop\n"
	    : "=&r" (valh), "=&r" (vall)
	    : "r" (csr_addrh), "r" (csr_addrl));

	intr_restore(sr);

	return ((uint64_t)valh << 32) | vall;
}
#endif

#endif

/*
 * octeon_machdep.c
 *
 * Direct to Board Support level.
 */
extern void octeon_reset(void);
extern void octeon_debug_symbol(void);
extern void octeon_ciu_reset(void);
extern int octeon_is_simulation(void);
#endif	/* LOCORE */

/*
 * Default FLASH device (physical) base address
 */
#define  OCTEON_FLASH_BASE_ADDR		(0x1d040000ull)

#endif /* !OCTEON_PCMAP_REGS_H__ */

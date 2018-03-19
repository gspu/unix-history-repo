/*-
 * Copyright 1996, 1997, 1998, 1999 John D. Polstra.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

/*
 * Dynamic linker for ELF.
 *
 * John Polstra <jdp@polstra.com>.
 */

#include <sys/param.h>
#include <sys/mman.h>
#include <machine/ia64_cpu.h>

#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "rtld.h"

extern Elf_Dyn _DYNAMIC;

/*
 * Macros for loading/storing unaligned 64-bit values.  These are
 * needed because relocations can point to unaligned data.  This
 * occurs in the DWARF2 exception frame tables generated by the
 * compiler, for instance.
 *
 * We don't use these when relocating jump slots and GOT entries,
 * since they are guaranteed to be aligned.
 *
 * XXX dfr stub for now.
 */
#define load64(p)	(*(u_int64_t *) (p))
#define store64(p, v)	(*(u_int64_t *) (p) = (v))

/* Allocate an @fptr. */

#define FPTR_CHUNK_SIZE		64

struct fptr_chunk {
	struct fptr fptrs[FPTR_CHUNK_SIZE];
};

static struct fptr_chunk first_chunk;
static struct fptr_chunk *current_chunk = &first_chunk;
static struct fptr *next_fptr = &first_chunk.fptrs[0];
static struct fptr *last_fptr = &first_chunk.fptrs[FPTR_CHUNK_SIZE];

/*
 * We use static storage initially so that we don't have to call
 * malloc during init_rtld().
 */
static struct fptr *
alloc_fptr(Elf_Addr target, Elf_Addr gp)
{
	struct fptr* fptr;

	if (next_fptr == last_fptr) {
		current_chunk = xmalloc(sizeof(struct fptr_chunk));
		next_fptr = &current_chunk->fptrs[0];
		last_fptr = &current_chunk->fptrs[FPTR_CHUNK_SIZE];
	}
	fptr = next_fptr;
	next_fptr++;
	fptr->target = target;
	fptr->gp = gp;
	return fptr;
}

static struct fptr **
alloc_fptrs(Obj_Entry *obj, bool mapped)
{
	struct fptr **fptrs;
	size_t fbytes;

	fbytes = obj->dynsymcount * sizeof(struct fptr *);

	/*
	 * Avoid malloc, if requested. Happens when relocating
	 * rtld itself on startup.
	 */
	if (mapped) {
		fptrs = mmap(NULL, fbytes, PROT_READ|PROT_WRITE,
	    	    MAP_ANON, -1, 0);
		if (fptrs == MAP_FAILED)
			fptrs = NULL;
	} else {
		fptrs = xcalloc(1, fbytes);
	}

	/*
	 * This assertion is necessary to guarantee function pointer
	 * uniqueness
 	 */
	assert(fptrs != NULL);

	return (obj->priv = fptrs);
}

static void
free_fptrs(Obj_Entry *obj, bool mapped)
{
	struct fptr **fptrs;
	size_t fbytes;

	fptrs  = obj->priv;
	if (fptrs == NULL)
		return;

	fbytes = obj->dynsymcount * sizeof(struct fptr *);
	if (mapped)
		munmap(fptrs, fbytes);
	else
		free(fptrs);
	obj->priv = NULL;
}

/* Relocate a non-PLT object with addend. */
static int
reloc_non_plt_obj(Obj_Entry *obj_rtld, Obj_Entry *obj, const Elf_Rela *rela,
    SymCache *cache, int flags, RtldLockState *lockstate)
{
	struct fptr **fptrs;
	Elf_Addr *where = (Elf_Addr *) (obj->relocbase + rela->r_offset);

	switch (ELF_R_TYPE(rela->r_info)) {
	case R_IA_64_REL64LSB:
		/*
		 * We handle rtld's relocations in rtld_start.S
		 */
		if (obj != obj_rtld)
			store64(where,
				load64(where) + (Elf_Addr) obj->relocbase);
		break;

	case R_IA_64_DIR64LSB: {
		const Elf_Sym *def;
		const Obj_Entry *defobj;
		Elf_Addr target;

		def = find_symdef(ELF_R_SYM(rela->r_info), obj, &defobj,
		    flags, cache, lockstate);
		if (def == NULL)
			return -1;

		target = (def->st_shndx != SHN_UNDEF)
		    ? (Elf_Addr)(defobj->relocbase + def->st_value) : 0;
		store64(where, target + rela->r_addend);
		break;
	}

	case R_IA_64_FPTR64LSB: {
		/*
		 * We have to make sure that all @fptr references to
		 * the same function are identical so that code can
		 * compare function pointers.
		 */
		const Elf_Sym *def;
		const Obj_Entry *defobj;
		struct fptr *fptr = 0;
		Elf_Addr target, gp;
		int sym_index;

		def = find_symdef(ELF_R_SYM(rela->r_info), obj, &defobj,
		    SYMLOOK_IN_PLT | flags, cache, lockstate);
		if (def == NULL) {
			/*
			 * XXX r_debug_state is problematic and find_symdef()
			 * returns NULL for it. This probably has something to
			 * do with symbol versioning (r_debug_state is in the
			 * symbol map). If we return -1 in that case we abort
			 * relocating rtld, which typically is fatal. So, for
			 * now just skip the symbol when we're relocating
			 * rtld. We don't care about r_debug_state unless we
			 * are being debugged.
			 */
			if (obj != obj_rtld)
				return -1;
			break;
		}

		if (def->st_shndx != SHN_UNDEF) {
			target = (Elf_Addr)(defobj->relocbase + def->st_value);
			gp = (Elf_Addr)defobj->pltgot;

			/* rtld is allowed to reference itself only */
			assert(!obj->rtld || obj == defobj);
			fptrs = defobj->priv;
			if (fptrs == NULL)
				fptrs = alloc_fptrs((Obj_Entry *) defobj, 
				    obj->rtld);

			sym_index = def - defobj->symtab;

			/*
			 * Find the @fptr, using fptrs as a helper.
			 */
			if (fptrs)
				fptr = fptrs[sym_index];
			if (!fptr) {
				fptr = alloc_fptr(target, gp);
				if (fptrs)
					fptrs[sym_index] = fptr;
			}
		} else
			fptr = NULL;

		store64(where, (Elf_Addr)fptr);
		break;
	}

	case R_IA_64_IPLTLSB: {
		/*
		 * Relocation typically used to populate C++ virtual function
		 * tables. It creates a 128-bit function descriptor at the
		 * specified memory address.
		 */
		const Elf_Sym *def;
		const Obj_Entry *defobj;
		struct fptr *fptr;
		Elf_Addr target, gp;

		def = find_symdef(ELF_R_SYM(rela->r_info), obj, &defobj,
		    flags, cache, lockstate);
		if (def == NULL)
			return -1;

		if (def->st_shndx != SHN_UNDEF) {
			target = (Elf_Addr)(defobj->relocbase + def->st_value);
			gp = (Elf_Addr)defobj->pltgot;
		} else {
			target = 0;
			gp = 0;
		}

		fptr = (void*)where;
		store64(&fptr->target, target);
		store64(&fptr->gp, gp);
		break;
	}

	case R_IA_64_DTPMOD64LSB: {
		const Elf_Sym *def;
		const Obj_Entry *defobj;

		def = find_symdef(ELF_R_SYM(rela->r_info), obj, &defobj,
		    flags, cache, lockstate);
		if (def == NULL)
			return -1;

		store64(where, defobj->tlsindex);
		break;
	}

	case R_IA_64_DTPREL64LSB: {
		const Elf_Sym *def;
		const Obj_Entry *defobj;

		def = find_symdef(ELF_R_SYM(rela->r_info), obj, &defobj,
		    flags, cache, lockstate);
		if (def == NULL)
			return -1;

		store64(where, def->st_value + rela->r_addend);
		break;
	}

	case R_IA_64_TPREL64LSB: {
		const Elf_Sym *def;
		const Obj_Entry *defobj;

		def = find_symdef(ELF_R_SYM(rela->r_info), obj, &defobj,
		    flags, cache, lockstate);
		if (def == NULL)
			return -1;

		/*
		 * We lazily allocate offsets for static TLS as we
		 * see the first relocation that references the
		 * TLS block. This allows us to support (small
		 * amounts of) static TLS in dynamically loaded
		 * modules. If we run out of space, we generate an
		 * error.
		 */
		if (!defobj->tls_done) {
			if (!allocate_tls_offset((Obj_Entry*) defobj)) {
				_rtld_error("%s: No space available for static "
				    "Thread Local Storage", obj->path);
				return -1;
			}
		}

		store64(where, defobj->tlsoffset + def->st_value + rela->r_addend);
		break;
	}

	case R_IA_64_NONE:
		break;

	default:
		_rtld_error("%s: Unsupported relocation type %u"
			    " in non-PLT relocations\n", obj->path,
			    (unsigned int)ELF_R_TYPE(rela->r_info));
		return -1;
	}

	return(0);
}

/* Process the non-PLT relocations. */
int
reloc_non_plt(Obj_Entry *obj, Obj_Entry *obj_rtld, int flags,
    RtldLockState *lockstate)
{
	const Elf_Rel *rellim;
	const Elf_Rel *rel;
	const Elf_Rela *relalim;
	const Elf_Rela *rela;
	SymCache *cache;
	int bytes = obj->dynsymcount * sizeof(SymCache);
	int r = -1;

	if ((flags & SYMLOOK_IFUNC) != 0)
		/* XXX not implemented */
		return (0);

	/*
	 * The dynamic loader may be called from a thread, we have
	 * limited amounts of stack available so we cannot use alloca().
	 */
	cache = mmap(NULL, bytes, PROT_READ|PROT_WRITE, MAP_ANON, -1, 0);
	if (cache == MAP_FAILED)
		cache = NULL;

	/* Perform relocations without addend if there are any: */
	rellim = (const Elf_Rel *) ((caddr_t) obj->rel + obj->relsize);
	for (rel = obj->rel;  obj->rel != NULL && rel < rellim;  rel++) {
		Elf_Rela locrela;

		locrela.r_info = rel->r_info;
		locrela.r_offset = rel->r_offset;
		locrela.r_addend = 0;
		if (reloc_non_plt_obj(obj_rtld, obj, &locrela, cache, flags,
		    lockstate))
			goto done;
	}

	/* Perform relocations with addend if there are any: */
	relalim = (const Elf_Rela *) ((caddr_t) obj->rela + obj->relasize);
	for (rela = obj->rela;  obj->rela != NULL && rela < relalim;  rela++) {
		if (reloc_non_plt_obj(obj_rtld, obj, rela, cache, flags,
		    lockstate))
			goto done;
	}

	r = 0;
done:
	if (cache)
		munmap(cache, bytes);

	/*
	 * Release temporarily mapped fptrs if relocating
	 * rtld object itself. A new table will be created
	 * in make_function_pointer using malloc when needed.
	 */
	if (obj->rtld && obj->priv)
		free_fptrs(obj, true);

	return (r);
}

/* Process the PLT relocations. */
int
reloc_plt(Obj_Entry *obj)
{
	/* All PLT relocations are the same kind: Elf_Rel or Elf_Rela. */
	if (obj->pltrelsize != 0) {
		const Elf_Rel *rellim;
		const Elf_Rel *rel;

		rellim = (const Elf_Rel *)
			((char *)obj->pltrel + obj->pltrelsize);
		for (rel = obj->pltrel;  rel < rellim;  rel++) {
			Elf_Addr *where;

			assert(ELF_R_TYPE(rel->r_info) == R_IA_64_IPLTLSB);

			/* Relocate the @fptr pointing into the PLT. */
			where = (Elf_Addr *)(obj->relocbase + rel->r_offset);
			*where += (Elf_Addr)obj->relocbase;
		}
	} else {
		const Elf_Rela *relalim;
		const Elf_Rela *rela;

		relalim = (const Elf_Rela *)
			((char *)obj->pltrela + obj->pltrelasize);
		for (rela = obj->pltrela;  rela < relalim;  rela++) {
			Elf_Addr *where;

			assert(ELF_R_TYPE(rela->r_info) == R_IA_64_IPLTLSB);

			/* Relocate the @fptr pointing into the PLT. */
			where = (Elf_Addr *)(obj->relocbase + rela->r_offset);
			*where += (Elf_Addr)obj->relocbase;
		}
	}
	return 0;
}

int
reloc_iresolve(Obj_Entry *obj, struct Struct_RtldLockState *lockstate)
{

	/* XXX not implemented */
	return (0);
}

int
reloc_gnu_ifunc(Obj_Entry *obj, int flags,
    struct Struct_RtldLockState *lockstate)
{

	/* XXX not implemented */
	return (0);
}

/* Relocate the jump slots in an object. */
int
reloc_jmpslots(Obj_Entry *obj, int flags, RtldLockState *lockstate)
{
	if (obj->jmpslots_done)
		return 0;
	/* All PLT relocations are the same kind: Elf_Rel or Elf_Rela. */
	if (obj->pltrelsize != 0) {
		const Elf_Rel *rellim;
		const Elf_Rel *rel;

		rellim = (const Elf_Rel *)
			((char *)obj->pltrel + obj->pltrelsize);
		for (rel = obj->pltrel;  rel < rellim;  rel++) {
			Elf_Addr *where;
			const Elf_Sym *def;
			const Obj_Entry *defobj;

			assert(ELF_R_TYPE(rel->r_info) == R_IA_64_IPLTLSB);
			where = (Elf_Addr *)(obj->relocbase + rel->r_offset);
			def = find_symdef(ELF_R_SYM(rel->r_info), obj,
			    &defobj, SYMLOOK_IN_PLT | flags, NULL, lockstate);
			if (def == NULL)
				return -1;
			reloc_jmpslot(where,
				      (Elf_Addr)(defobj->relocbase
						 + def->st_value),
				      defobj, obj, rel);
		}
	} else {
		const Elf_Rela *relalim;
		const Elf_Rela *rela;

		relalim = (const Elf_Rela *)
			((char *)obj->pltrela + obj->pltrelasize);
		for (rela = obj->pltrela;  rela < relalim;  rela++) {
			Elf_Addr *where;
			const Elf_Sym *def;
			const Obj_Entry *defobj;

			where = (Elf_Addr *)(obj->relocbase + rela->r_offset);
			def = find_symdef(ELF_R_SYM(rela->r_info), obj,
			    &defobj, SYMLOOK_IN_PLT | flags, NULL, lockstate);
			if (def == NULL)
				return -1;
			reloc_jmpslot(where,
				      (Elf_Addr)(defobj->relocbase
						 + def->st_value),
				      defobj, obj, (Elf_Rel *)rela);
		}
	}
	obj->jmpslots_done = true;
	return 0;
}

/* Fixup the jump slot at "where" to transfer control to "target". */
Elf_Addr
reloc_jmpslot(Elf_Addr *where, Elf_Addr target, const Obj_Entry *obj,
	      const Obj_Entry *refobj, const Elf_Rel *rel)
{
	Elf_Addr stubaddr;

	dbg(" reloc_jmpslot: where=%p, target=%p, gp=%p",
	    (void *)where, (void *)target, (void *)obj->pltgot);
	stubaddr = *where;
	if (stubaddr != target) {

		/*
		 * Point this @fptr directly at the target. Update the
		 * gp value first so that we don't break another cpu
		 * which is currently executing the PLT entry.
		 */
		where[1] = (Elf_Addr) obj->pltgot;
		ia64_mf();
		where[0] = target;
		ia64_mf();
	}

	/*
	 * The caller needs an @fptr for the adjusted entry. The PLT
	 * entry serves this purpose nicely.
	 */
	return (Elf_Addr) where;
}

/*
 * XXX ia64 doesn't seem to have copy relocations.
 *
 * Returns 0 on success, -1 on failure.
 */
int
do_copy_relocations(Obj_Entry *dstobj)
{

	return 0;
}

/*
 * Return the @fptr representing a given function symbol.
 */
void *
make_function_pointer(const Elf_Sym *sym, const Obj_Entry *obj)
{
	struct fptr **fptrs = obj->priv;
	int index = sym - obj->symtab;

	if (!fptrs) {
		/*
		 * This should only happen for something like
		 * dlsym("dlopen"). Actually, I'm not sure it can ever 
		 * happen.
		 */
		fptrs = alloc_fptrs((Obj_Entry *) obj, false);
	}
	if (!fptrs[index]) {
		Elf_Addr target, gp;
		target = (Elf_Addr) (obj->relocbase + sym->st_value);
		gp = (Elf_Addr) obj->pltgot;
		fptrs[index] = alloc_fptr(target, gp);
	}
	return fptrs[index];
}

void
call_initfini_pointer(const Obj_Entry *obj, Elf_Addr target)
{
	struct fptr fptr;

	fptr.gp = (Elf_Addr) obj->pltgot;
	fptr.target = target;
	dbg(" initfini: target=%p, gp=%p",
	    (void *) fptr.target, (void *) fptr.gp);
	((InitFunc) &fptr)();
}

void
call_init_pointer(const Obj_Entry *obj, Elf_Addr target)
{
	struct fptr fptr;

	fptr.gp = (Elf_Addr) obj->pltgot;
	fptr.target = target;
	dbg(" initfini: target=%p, gp=%p",
	    (void *) fptr.target, (void *) fptr.gp);
	((InitArrFunc) &fptr)(main_argc, main_argv, environ);
}

void  
ifunc_init(Elf_Auxinfo aux_info[__min_size(AT_COUNT)] __unused)
{

}

void
pre_init(void)
{

}

/* Initialize the special PLT entries. */
void
init_pltgot(Obj_Entry *obj)
{
	const Elf_Dyn *dynp;
	Elf_Addr *pltres = 0;

	/*
	 * When there are no PLT relocations, the DT_IA_64_PLT_RESERVE entry
	 * is bogus. Do not setup the BOR pointers in that case. An example
	 * of where this happens is /usr/lib/libxpg4.so.3.
	 */
	if (obj->pltrelasize == 0 && obj->pltrelsize == 0)
		return;

	/*
	 * Find the PLT RESERVE section.
	 */
	for (dynp = obj->dynamic;  dynp->d_tag != DT_NULL;  dynp++) {
		if (dynp->d_tag == DT_IA_64_PLT_RESERVE)
			pltres = (u_int64_t *)
				(obj->relocbase + dynp->d_un.d_ptr);
	}
	if (!pltres)
		errx(1, "Can't find DT_IA_64_PLT_RESERVE entry");

	/*
	 * The PLT RESERVE section is used to get values to pass to
	 * _rtld_bind when lazy binding.
	 */
	pltres[0] = (Elf_Addr) obj;
	pltres[1] = FPTR_TARGET(_rtld_bind_start);
	pltres[2] = FPTR_GP(_rtld_bind_start);
}

void
allocate_initial_tls(Obj_Entry *list)
{
    void *tpval;

    /*
     * Fix the size of the static TLS block by using the maximum
     * offset allocated so far and adding a bit for dynamic modules to
     * use.
     */
    tls_static_space = tls_last_offset + tls_last_size + RTLD_STATIC_TLS_EXTRA;

    tpval = allocate_tls(list, NULL, TLS_TCB_SIZE, 16);
    __asm __volatile("mov r13 = %0" :: "r"(tpval));
}

void *__tls_get_addr(unsigned long module, unsigned long offset)
{
    register Elf_Addr** tp __asm__("r13");

    return tls_get_addr_common(tp, module, offset);
}

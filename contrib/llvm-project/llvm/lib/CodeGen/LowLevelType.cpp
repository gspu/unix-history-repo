//===-- llvm/CodeGen/LowLevelType.cpp -------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file This file implements the more header-heavy bits of the LLT class to
/// avoid polluting users' namespaces.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/LowLevelType.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

LLT llvm::getLLTForType(Type &Ty, const DataLayout &DL) {
  if (auto VTy = dyn_cast<VectorType>(&Ty)) {
    auto NumElements = VTy->getNumElements();
    LLT ScalarTy = getLLTForType(*VTy->getElementType(), DL);
    if (NumElements == 1)
      return ScalarTy;
    return LLT::vector(NumElements, ScalarTy);
  }

  if (auto PTy = dyn_cast<PointerType>(&Ty)) {
    unsigned AddrSpace = PTy->getAddressSpace();
    return LLT::pointer(AddrSpace, DL.getPointerSizeInBits(AddrSpace));
  }

  if (Ty.isSized()) {
    // Aggregates are no different from real scalars as far as GlobalISel is
    // concerned.
    auto SizeInBits = DL.getTypeSizeInBits(&Ty);
    assert(SizeInBits != 0 && "invalid zero-sized type");
    return LLT::scalar(SizeInBits);
  }

  return LLT();
}

MVT llvm::getMVTForLLT(LLT Ty) {
  if (!Ty.isVector())
    return MVT::getIntegerVT(Ty.getSizeInBits());

  return MVT::getVectorVT(
      MVT::getIntegerVT(Ty.getElementType().getSizeInBits()),
      Ty.getNumElements());
}

LLT llvm::getLLTForMVT(MVT Ty) {
  if (!Ty.isVector())
    return LLT::scalar(Ty.getSizeInBits());

  return LLT::vector(Ty.getVectorNumElements(),
                     Ty.getVectorElementType().getSizeInBits());
}

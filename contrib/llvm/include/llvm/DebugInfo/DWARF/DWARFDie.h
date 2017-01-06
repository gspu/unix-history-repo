//===-- DWARFDie.h --------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_DEBUGINFO_DWARFDIE_H
#define LLVM_LIB_DEBUGINFO_DWARFDIE_H

#include "llvm/ADT/iterator.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/ADT/Optional.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugInfoEntry.h"

namespace llvm {
    
class DWARFUnit;
class DWARFDebugInfoEntry;
class raw_ostream;
  
//===----------------------------------------------------------------------===//
/// Utility class that carries the DWARF compile/type unit and the debug info
/// entry in an object.
///
/// When accessing information from a debug info entry we always need to DWARF
/// compile/type unit in order to extract the info correctly as some information
/// is relative to the compile/type unit. Prior to this class the DWARFUnit and
/// the DWARFDebugInfoEntry was passed around separately and there was the
/// possibility for error if the wrong DWARFUnit was used to extract a unit
/// relative offset. This class helps to ensure that this doesn't happen and
/// also simplifies the attribute extraction calls by not having to specify the
/// DWARFUnit for each call.
class DWARFDie {
  DWARFUnit *U;
  const DWARFDebugInfoEntry *Die;
public:
  DWARFDie() : U(nullptr), Die(nullptr) {}
  DWARFDie(DWARFUnit *Unit, const DWARFDebugInfoEntry * D) : U(Unit), Die(D) {}
  
  bool isValid() const { return U && Die; }
  explicit operator bool() const { return isValid(); }
  const DWARFDebugInfoEntry *getDebugInfoEntry() const { return Die; }
  DWARFUnit *getDwarfUnit() const { return U; }


  /// Get the abbreviation declaration for this DIE.
  ///
  /// \returns the abbreviation declaration or NULL for null tags.
  const DWARFAbbreviationDeclaration *getAbbreviationDeclarationPtr() const {
    assert(isValid() && "must check validity prior to calling");
    return Die->getAbbreviationDeclarationPtr();
  }

  /// Get the absolute offset into the debug info or types section.
  ///
  /// \returns the DIE offset or -1U if invalid.
  uint32_t getOffset() const {
    assert(isValid() && "must check validity prior to calling");
    return Die->getOffset();
  }
  
  dwarf::Tag getTag() const {
    auto AbbrevDecl = getAbbreviationDeclarationPtr();
    if (AbbrevDecl)
      return AbbrevDecl->getTag();
    return dwarf::DW_TAG_null;
  }

  bool hasChildren() const {
    assert(isValid() && "must check validity prior to calling");
    return Die->hasChildren();
  }
  
  /// Returns true for a valid DIE that terminates a sibling chain.
  bool isNULL() const {
    return getAbbreviationDeclarationPtr() == nullptr;
  }
  /// Returns true if DIE represents a subprogram (not inlined).
  bool isSubprogramDIE() const;

  /// Returns true if DIE represents a subprogram or an inlined subroutine.
  bool isSubroutineDIE() const;

  /// Get the parent of this DIE object.
  ///
  /// \returns a valid DWARFDie instance if this object has a parent or an
  /// invalid DWARFDie instance if it doesn't.
  DWARFDie getParent() const;
  
  /// Get the sibling of this DIE object.
  ///
  /// \returns a valid DWARFDie instance if this object has a sibling or an
  /// invalid DWARFDie instance if it doesn't.
  DWARFDie getSibling() const;
  
  /// Get the first child of this DIE object.
  ///
  /// \returns a valid DWARFDie instance if this object has children or an
  /// invalid DWARFDie instance if it doesn't.
  DWARFDie getFirstChild() const {
    if (isValid() && Die->hasChildren())
      return DWARFDie(U, Die + 1);
    return DWARFDie();
  }
  
  /// Dump the DIE and all of its attributes to the supplied stream.
  ///
  /// \param OS the stream to use for output.
  /// \param recurseDepth the depth to recurse to when dumping this DIE and its
  /// children.
  /// \param indent the number of characters to indent each line that is output.
  void dump(raw_ostream &OS, unsigned recurseDepth, unsigned indent = 0) const;
  
  /// Extract the specified attribute from this DIE.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \returns an optional DWARFFormValue that will have the form value if the
  /// attribute was successfully extracted.
  Optional<DWARFFormValue> getAttributeValue(dwarf::Attribute Attr) const;
  
  /// Extract the specified attribute from this DIE as a C string.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \param FailValue the value to return if this DIE doesn't have this
  /// attribute.
  /// \returns the NULL terminated C string value owned by the DWARF section
  /// that contains the string or FailValue if the attribute doesn't exist or
  /// if the attribute's form isn't a form that describes an string.
  const char *getAttributeValueAsString(dwarf::Attribute Attr,
                                        const char *FailValue) const;
  
  /// Extract the specified attribute from this DIE as an address.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \param FailValue the value to return if this DIE doesn't have this
  /// attribute.
  /// \returns the address value of the attribute or FailValue if the
  /// attribute doesn't exist or if the attribute's form isn't a form that
  /// describes an address.
  uint64_t getAttributeValueAsAddress(dwarf::Attribute Attr,
                                      uint64_t FailValue) const;

  /// Extract the specified attribute from this DIE as an address.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \returns an optional value for the attribute.
  Optional<uint64_t> getAttributeValueAsAddress(dwarf::Attribute Attr) const;
  
  /// Extract the specified attribute from this DIE as a signed integer.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \param FailValue the value to return if this DIE doesn't have this
  /// attribute.
  /// \returns the signed integer constant value of the attribute or FailValue
  /// if the attribute doesn't exist or if the attribute's form isn't a form
  /// that describes a signed integer.
  int64_t getAttributeValueAsSignedConstant(dwarf::Attribute Attr,
                                            int64_t FailValue) const;

  /// Extract the specified attribute from this DIE as a signed integer.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \returns an optional value for the attribute.
  Optional<int64_t>
  getAttributeValueAsSignedConstant(dwarf::Attribute Attr) const;
  
  /// Extract the specified attribute from this DIE as an unsigned integer.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \param FailValue the value to return if this DIE doesn't have this
  /// attribute.
  /// \returns the unsigned integer constant value of the attribute or FailValue
  /// if the attribute doesn't exist or if the attribute's form isn't a form
  /// that describes an unsigned integer.
  uint64_t getAttributeValueAsUnsignedConstant(dwarf::Attribute Attr,
                                               uint64_t FailValue) const;
  
  /// Extract the specified attribute from this DIE as an unsigned integer.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \returns an optional value for the attribute.
  Optional<uint64_t>
  getAttributeValueAsUnsignedConstant(dwarf::Attribute Attr) const;

  /// Extract the specified attribute from this DIE as absolute DIE Offset.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \param FailValue the value to return if this DIE doesn't have this
  /// attribute.
  /// \returns the unsigned integer constant value of the attribute or FailValue
  /// if the attribute doesn't exist or if the attribute's form isn't a form
  /// that describes a reference.
  uint64_t getAttributeValueAsReference(dwarf::Attribute Attr,
                                        uint64_t FailValue) const;
  
  /// Extract the specified attribute from this DIE as absolute DIE Offset.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \returns an optional value for the attribute.
  Optional<uint64_t> getAttributeValueAsReference(dwarf::Attribute Attr) const;
  
  /// Extract the specified attribute from this DIE as absolute section offset.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \param FailValue the value to return if this DIE doesn't have this
  /// attribute.
  /// \returns the unsigned integer constant value of the attribute or FailValue
  /// if the attribute doesn't exist or if the attribute's form isn't a form
  /// that describes a section offset.
  uint64_t getAttributeValueAsSectionOffset(dwarf::Attribute Attr,
                                            uint64_t FailValue) const;
  /// Extract the specified attribute from this DIE as absolute section offset.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \returns an optional value for the attribute.
  Optional<uint64_t>
  getAttributeValueAsSectionOffset(dwarf::Attribute Attr) const;
  
  /// Extract the specified attribute from this DIE as the referenced DIE.
  ///
  /// Regardless of the reference type, return the correct DWARFDie instance if
  /// the attribute exists. The returned DWARFDie object might be from another
  /// DWARFUnit, but that is all encapsulated in the new DWARFDie object.
  ///
  /// Extract an attribute value from this DIE only. This call doesn't look
  /// for the attribute value in any DW_AT_specification or
  /// DW_AT_abstract_origin referenced DIEs.
  ///
  /// \param Attr the attribute to extract.
  /// \returns a valid DWARFDie instance if the attribute exists, or an invalid
  /// DWARFDie object if it doesn't.
  DWARFDie getAttributeValueAsReferencedDie(dwarf::Attribute Attr) const;

  /// Extract the range base attribute from this DIE as absolute section offset.
  ///
  /// This is a utility function that checks for either the DW_AT_rnglists_base
  /// or DW_AT_GNU_ranges_base attribute.
  ///
  /// \returns anm optional absolute section offset value for the attribute.
  Optional<uint64_t> getRangesBaseAttribute() const;
  
  /// Get the DW_AT_high_pc attribute value as an address.
  ///
  /// In DWARF version 4 and later the high PC can be encoded as an offset from
  /// the DW_AT_low_pc. This function takes care of extracting the value as an
  /// address or offset and adds it to the low PC if needed and returns the
  /// value as an optional in case the DIE doesn't have a DW_AT_high_pc
  /// attribute.
  ///
  /// \param LowPC the low PC that might be needed to calculate the high PC.
  /// \returns an optional address value for the attribute.
  Optional<uint64_t> getHighPC(uint64_t LowPC) const;

  /// Retrieves DW_AT_low_pc and DW_AT_high_pc from CU.
  /// Returns true if both attributes are present.
  bool getLowAndHighPC(uint64_t &LowPC, uint64_t &HighPC) const;
  
  /// Get the address ranges for this DIE.
  ///
  /// Get the hi/low PC range if both attributes are available or exrtracts the
  /// non-contiguous address ranges from the DW_AT_ranges attribute.
  ///
  /// Extracts the range information from this DIE only. This call doesn't look
  /// for the range in any DW_AT_specification or DW_AT_abstract_origin DIEs.
  ///
  /// \returns a address range vector that might be empty if no address range
  /// information is available.
  DWARFAddressRangesVector getAddressRanges() const;
  
  /// Get all address ranges for any DW_TAG_subprogram DIEs in this DIE or any
  /// of its children.
  ///
  /// Get the hi/low PC range if both attributes are available or exrtracts the
  /// non-contiguous address ranges from the DW_AT_ranges attribute for this DIE
  /// and all children.
  ///
  /// \param Ranges the addres range vector to fill in.
  void collectChildrenAddressRanges(DWARFAddressRangesVector &Ranges) const;
  
  bool addressRangeContainsAddress(const uint64_t Address) const;
  
  /// If a DIE represents a subprogram (or inlined subroutine), returns its
  /// mangled name (or short name, if mangled is missing). This name may be
  /// fetched from specification or abstract origin for this subprogram.
  /// Returns null if no name is found.
  const char *getSubroutineName(DINameKind Kind) const;
  
  /// Return the DIE name resolving DW_AT_sepcification or DW_AT_abstract_origin
  /// references if necessary. Returns null if no name is found.
  const char *getName(DINameKind Kind) const;
  
  /// Retrieves values of DW_AT_call_file, DW_AT_call_line and DW_AT_call_column
  /// from DIE (or zeroes if they are missing). This function looks for
  /// DW_AT_call attributes in this DIE only, it will not resolve the attribute
  /// values in any DW_AT_specification or DW_AT_abstract_origin DIEs.
  /// \param CallFile filled in with non-zero if successful, zero if there is no
  /// DW_AT_call_file attribute in this DIE.
  /// \param CallLine filled in with non-zero if successful, zero if there is no
  /// DW_AT_call_line attribute in this DIE.
  /// \param CallColumn filled in with non-zero if successful, zero if there is
  /// no DW_AT_call_column attribute in this DIE.
  void getCallerFrame(uint32_t &CallFile, uint32_t &CallLine,
                      uint32_t &CallColumn) const;
  
  /// Get inlined chain for a given address, rooted at the current DIE.
  /// Returns empty chain if address is not contained in address range
  /// of current DIE.
  void
  getInlinedChainForAddress(const uint64_t Address,
                            SmallVectorImpl<DWARFDie> &InlinedChain) const;

  class iterator;
  
  iterator begin() const;
  iterator end() const;
  iterator_range<iterator> children() const;
};

  
inline bool operator==(const DWARFDie &LHS, const DWARFDie &RHS) {
  return LHS.getDebugInfoEntry() == RHS.getDebugInfoEntry() &&
      LHS.getDwarfUnit() == RHS.getDwarfUnit();
}

inline bool operator!=(const DWARFDie &LHS, const DWARFDie &RHS) {
  return !(LHS == RHS);
}

class DWARFDie::iterator : public iterator_facade_base<iterator,
                                                      std::forward_iterator_tag,
                                                      const DWARFDie> {
  DWARFDie Die;
  void skipNull() {
    if (Die && Die.isNULL())
      Die = DWARFDie();
  }
public:
  iterator() = default;
  explicit iterator(DWARFDie D) : Die(D) {
    // If we start out with only a Null DIE then invalidate.
    skipNull();
  }
  iterator &operator++() {
    Die = Die.getSibling();
    // Don't include the NULL die when iterating.
    skipNull();
    return *this;
  }
  explicit operator bool() const { return Die.isValid(); }
  const DWARFDie &operator*() const { return Die; }
  bool operator==(const iterator &X) const { return Die == X.Die; }
};

// These inline functions must follow the DWARFDie::iterator definition above
// as they use functions from that class.
inline DWARFDie::iterator DWARFDie::begin() const {
  return iterator(getFirstChild());
}

inline DWARFDie::iterator DWARFDie::end() const {
  return iterator();
}

inline iterator_range<DWARFDie::iterator> DWARFDie::children() const {
  return make_range(begin(), end());
}

} // end namespace llvm

#endif  // LLVM_LIB_DEBUGINFO_DWARFDIE_H

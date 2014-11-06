//===-- Z80TargetObjectFile.cpp - TargetObjectFile overrides for the Z80 --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Z80 specific subclass of TargetObjectFile.
// Mainly so we can set the sections to Z80Section.
//
//===----------------------------------------------------------------------===//

#ifndef Z80_TARGETOBJECTFILE_H
#define Z80_TARGETOBJECTFILE_H

#include "Z80Section.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include <string>

namespace llvm {
class GlobalVariable;
class Module;

class Z80TargetObjectFile : public TargetLoweringObjectFile {

public:
  Z80TargetObjectFile() {
    TextSection = 0;
    DataSection = 0;
    BSSSection = 0;
    ReadOnlySection = 0;

    StaticCtorSection = 0;
    StaticDtorSection = 0;
    LSDASection = 0;
    EHFrameSection = 0;
    DwarfAbbrevSection = 0;
    DwarfInfoSection = 0;
    DwarfLineSection = 0;
    DwarfFrameSection = 0;
    DwarfPubTypesSection = 0;
    DwarfDebugInlineSection = 0;
    DwarfStrSection = 0;
    DwarfLocSection = 0;
    DwarfARangesSection = 0;
    DwarfRangesSection = 0;
    DwarfMacroInfoSection = 0;
  }

  ~Z80TargetObjectFile() {
    delete TextSection;
    delete DataSection;
    delete BSSSection;
    delete ReadOnlySection;

    delete StaticCtorSection;
    delete StaticDtorSection;
    delete LSDASection;
    delete EHFrameSection;
    delete DwarfAbbrevSection;
    delete DwarfInfoSection;
    delete DwarfLineSection;
    delete DwarfFrameSection;
    delete DwarfPubTypesSection;
    delete DwarfDebugInlineSection;
    delete DwarfStrSection;
    delete DwarfLocSection;
    delete DwarfARangesSection;
    delete DwarfRangesSection;
    delete DwarfMacroInfoSection;
  }

  virtual void Initialize(MCContext &ctx, const TargetMachine &TM) {
    TargetLoweringObjectFile::Initialize(ctx, TM);
    TextSection = new Z80Section(MCSection::SV_ELF, SectionKind::getText());
    DataSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getDataRel());
    BSSSection = new Z80Section(MCSection::SV_ELF, SectionKind::getBSS());
    ReadOnlySection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getReadOnly());

    StaticCtorSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    StaticDtorSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    LSDASection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    EHFrameSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfAbbrevSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfInfoSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfLineSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfFrameSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfPubTypesSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfDebugInlineSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfStrSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfLocSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfARangesSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfRangesSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
    DwarfMacroInfoSection =
        new Z80Section(MCSection::SV_ELF, SectionKind::getMetadata());
  }

  virtual const MCSection *getSectionForConstant(SectionKind Kind) const {
    return ReadOnlySection;
  }

  virtual const MCSection *
  getExplicitSectionGlobal(const GlobalValue *GV, SectionKind Kind,
                           Mangler *Mang, const TargetMachine &TM) const {
    return DataSection;
  }

};

} // end namespace llvm

#endif

//===--      Z80Section.h - Z80 target section overrides        -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Z80 specific subclass of MCSection.
//
//===----------------------------------------------------------------------===//

#ifndef Z80SECTION_H
#define Z80SECTION_H

#include "llvm/MC/MCSection.h"

namespace llvm
{
	class Z80Section : public MCSection {
		public:
			Z80Section(SectionVariant V, SectionKind K) : MCSection(V, K) {}
			~Z80Section() {}

			//We don't have this shit.
			virtual void PrintSwitchToSection(const MCAsmInfo &MAI, raw_ostream &OS, const MCExpr *Subsection) const {}

			virtual bool isBaseAddressKnownZero() const { return true; }
			virtual bool UseCodeAlign() const { return false; }
			virtual bool isVirtualSection() const { return false; }
			virtual std::string getLabelBeginName() const { return ""; }
			virtual std::string getLabelEndName() const { return ""; }
	};
}

#endif
//===- llvm/CodeGen/GCMetadataPrinter.h - Prints asm GC tables --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// The abstract base class GCMetadataPrinter supports writing GC metadata tables
// as assembly code. This is a separate class from GCStrategy in order to allow
// users of the LLVM JIT to avoid linking with the AsmWriter.
//
// Subclasses of GCMetadataPrinter must be registered using the
// GCMetadataPrinterRegistry. This is separate from the GCStrategy itself
// because these subclasses are logically plugins for the AsmWriter.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_GCMETADATAPRINTER_H
#define LLVM_CODEGEN_GCMETADATAPRINTER_H

#include "llvm/Support/Registry.h"

namespace llvm {

class AsmPrinter;
class GCMetadataPrinter;
class GCModuleInfo;
class GCStrategy;
class Module;
class StackMaps;

/// GCMetadataPrinterRegistry - The GC assembly printer registry uses all the
/// defaults from Registry.
using GCMetadataPrinterRegistry = Registry<GCMetadataPrinter>;

/// GCMetadataPrinter - Emits GC metadata as assembly code.  Instances are
/// created, managed, and owned by the AsmPrinter.
class GCMetadataPrinter {
private:
  friend class AsmPrinter;

  GCStrategy *S;

protected:
  // May only be subclassed.
  GCMetadataPrinter();

public:
  GCMetadataPrinter(const GCMetadataPrinter &) = delete;
  GCMetadataPrinter &operator=(const GCMetadataPrinter &) = delete;
  virtual ~GCMetadataPrinter();

  GCStrategy &getStrategy() { return *S; }

  /// Called before the assembly for the module is generated by
  /// the AsmPrinter (but after target specific hooks.)
  virtual void beginAssembly(Module &M, GCModuleInfo &Info, AsmPrinter &AP) {}

  /// Called after the assembly for the module is generated by
  /// the AsmPrinter (but before target specific hooks)
  virtual void finishAssembly(Module &M, GCModuleInfo &Info, AsmPrinter &AP) {}

  /// Called when the stack maps are generated. Return true if
  /// stack maps with a custom format are generated. Otherwise
  /// returns false and the default format will be used.
  virtual bool emitStackMaps(StackMaps &SM, AsmPrinter &AP) { return false; }
};

} // end namespace llvm

#endif // LLVM_CODEGEN_GCMETADATAPRINTER_H

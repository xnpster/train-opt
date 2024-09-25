//===- SCCP.h - Pass that implements --------------------------------------===//
//              Sparse Conditional Constant Propagation
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <llvm/ADT/Statistic.h>
#include <llvm/Analysis/ConstantFolding.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/ValueLattice.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/User.h>
#include <llvm/Pass.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/Local.h>

using namespace llvm;

namespace llvm {
void initializeSCCPPass(PassRegistry &);
}

namespace llvm::trainOpt {

/// LatticeVal class - This class represents the different lattice values that
/// an LLVM value may occupy.  It is a simple class with value semantics.
///
class LatticeVal {
  enum LatticeValueTy {
    /// unknown - This LLVM Value has no known value yet.
    unknown,

    /// constant - This LLVM Value has a specific constant value.
    constant,

    /// forcedconstant - This LLVM Value was thought to be undef until
    /// ResolvedUndefsIn.  This is treated just like 'constant', but if merged
    /// with another (different) constant, it goes to overdefined, instead of
    /// asserting.
    forcedconstant,

    /// overdefined - This instruction is not known to be constant, and we know
    /// it has a value.
    overdefined
  };

  /// Val: This stores the current lattice value along with the Constant* for
  /// the constant if this is a 'constant' or 'forcedconstant' value.
  PointerIntPair<Constant *, 2, LatticeValueTy> Val;

  LatticeValueTy getLatticeValue() const { return Val.getInt(); }

public:
  LatticeVal() : Val(nullptr, unknown) {}

  bool isUnknown() const { return getLatticeValue() == unknown; }

  bool isConstant() const {
    return getLatticeValue() == constant || getLatticeValue() == forcedconstant;
  }

  bool isOverdefined() const { return getLatticeValue() == overdefined; }

  Constant *getConstant() const {
    assert(isConstant() && "Cannot get the constant of a non-constant!");
    return Val.getPointer();
  }

  /// markOverdefined - Return true if this is a change in status.
  bool markOverdefined() {
    if (isOverdefined()) {
      return false;
    }

    Val.setInt(overdefined);
    return true;
  }

  /// markConstant - Return true if this is a change in status.
  bool markConstant(Constant *V) {
    if (getLatticeValue() == constant) { // Constant but not forcedconstant.
      assert(getConstant() == V && "Marking constant with different value");
      return false;
    }

    if (isUnknown()) {
      Val.setInt(constant);
      assert(V && "Marking constant with NULL");
      Val.setPointer(V);
    } else {
      assert(getLatticeValue() == forcedconstant &&
             "Cannot move from overdefined to constant!");
      // Stay at forcedconstant if the constant is the same.
      if (V == getConstant()) {
        return false;
      }

      // Otherwise, we go to overdefined.  Assumptions made based on the
      // forced value are possibly wrong.  Assuming this is another constant
      // could expose a contradiction.
      Val.setInt(overdefined);
    }
    return true;
  }

  /// getConstantInt - If this is a constant with a ConstantInt value, return it
  /// otherwise return null.
  ConstantInt *getConstantInt() const {
    if (isConstant()) {
      return dyn_cast<ConstantInt>(getConstant());
    }
    return nullptr;
  }

  /// getBlockAddress - If this is a constant with a BlockAddress value, return
  /// it, otherwise return null.
  BlockAddress *getBlockAddress() const {
    if (isConstant()) {
      return dyn_cast<BlockAddress>(getConstant());
    }
    return nullptr;
  }

  void markForcedConstant(Constant *V) {
    assert(isUnknown() && "Can't force a defined value!");
    Val.setInt(forcedconstant);
    Val.setPointer(V);
  }

  ValueLatticeElement toValueLattice() const {
    if (isOverdefined()) {
      return ValueLatticeElement::getOverdefined();
    }
    if (isConstant()) {
      return ValueLatticeElement::get(getConstant());
    }
    return ValueLatticeElement();
  }
};

class Solver : public InstVisitor<Solver> {
  friend InstVisitor<Solver>;

public:
  Solver(const DataLayout &DL, const TargetLibraryInfo *TLI)
      : DL(DL), TLI(TLI) {}

  void solve();

  /**
   *  markBlockExecutable - This method can be used by clients to mark all of
   *  the blocks that are known to be intrinsically live in the processed unit.

   *  This returns true if the block was not considered live before.
   */
  bool markBlockExecutable(BasicBlock *BB);

  /**
   *  markOverdefined - Mark the specified value overdefined.  This
   *  works with both scalars and structs.
   */
  void markOverdefined(Value *V);

  bool isBlockExecutable(BasicBlock *BB);

  const LatticeVal &getLatticeValueFor(Value *V) const;

private:
  void visitBinaryOperator(Instruction &I);
  void visitCmpInst(CmpInst &I);
  void visitTerminator(Instruction &I);
  void visitPHINode(PHINode &PN);
  void visitReturnInst(ReturnInst &I);

  bool isEdgeFeasible(BasicBlock *From, BasicBlock *To);
  void getFeasibleSuccessors(Instruction &I, SmallVector<bool, 16> &Succs);
  bool markEdgeExecutable(BasicBlock *Source, BasicBlock *Dest);
  void pushToWorkList(LatticeVal &IV, Value *V);

  bool markConstant(LatticeVal &IV, Value *V, Constant *C);
  bool markConstant(Value *V, Constant *C);

  bool markOverdefined(LatticeVal &IV, Value *V);
  void markUsersAsChanged(Value *I);

  /**
   *  getValueState - Return the LatticeVal object that corresponds to the
   *  value.  This function handles the case when the value hasn't been seen yet
   *  by properly seeding constants etc.
   */
  LatticeVal &getValueState(Value *V);

private:
  const DataLayout &DL;
  const TargetLibraryInfo *TLI;
  DenseMap<Value *, LatticeVal> ValueState;

  /**
   *  The worklists
   */
  SmallVector<BasicBlock *, 64> BBWorkList;
  SmallVector<Value *, 64> OverdefinedInstWorkList;
  SmallVector<Value *, 64> InstWorkList;

  /**
   *  The BBs that are executable.
   */
  SmallPtrSet<BasicBlock *, 8> BBExecutable;
  /**
   * KnownFeasibleEdges - Entries in this set are edges which have already had
   * PHI nodes retriggered.
   */
  using Edge = std::pair<BasicBlock *, BasicBlock *>;
  DenseSet<Edge> KnownFeasibleEdges;
};

} // namespace llvm::trainOpt

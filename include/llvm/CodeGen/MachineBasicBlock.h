//===-- llvm/CodeGen/MachineBasicBlock.h ------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Collect the sequence of machine instructions for a basic block.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_MACHINEBASICBLOCK_H
#define LLVM_CODEGEN_MACHINEBASICBLOCK_H

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/ilist.h"
#include "llvm/Support/Streams.h"

namespace llvm {
  class MachineFunction;

// ilist_traits
template <>
struct ilist_traits<MachineInstr> {
protected:
  // this is only set by the MachineBasicBlock owning the ilist
  friend class MachineBasicBlock;
  MachineBasicBlock* parent;

public:
  ilist_traits<MachineInstr>() : parent(0) { }

  static MachineInstr* getPrev(MachineInstr* N) { return N->Prev; }
  static MachineInstr* getNext(MachineInstr* N) { return N->Next; }

  static const MachineInstr*
  getPrev(const MachineInstr* N) { return N->Prev; }

  static const MachineInstr*
  getNext(const MachineInstr* N) { return N->Next; }

  static void setPrev(MachineInstr* N, MachineInstr* prev) { N->Prev = prev; }
  static void setNext(MachineInstr* N, MachineInstr* next) { N->Next = next; }

  static MachineInstr* createSentinel();
  static void destroySentinel(MachineInstr *MI) { delete MI; }
  void addNodeToList(MachineInstr* N);
  void removeNodeFromList(MachineInstr* N);
  void transferNodesFromList(
      iplist<MachineInstr, ilist_traits<MachineInstr> >& toList,
      ilist_iterator<MachineInstr> first,
      ilist_iterator<MachineInstr> last);
};

class BasicBlock;

class MachineBasicBlock {
  typedef ilist<MachineInstr> Instructions;
  Instructions Insts;
  MachineBasicBlock *Prev, *Next;
  const BasicBlock *BB;
  int Number;
  MachineFunction *xParent;
  
  void setParent(MachineFunction *P) { xParent = P; }

  /// Predecessors/Successors - Keep track of the predecessor / successor
  /// basicblocks.
  std::vector<MachineBasicBlock *> Predecessors;
  std::vector<MachineBasicBlock *> Successors;

  /// LiveIns - Keep track of the physical registers that are livein of
  /// the basicblock.
  std::vector<unsigned> LiveIns;

  /// Alignment - Alignment of the basic block. Zero if the basic block does
  /// not need to be aligned.
  unsigned Alignment;
  
  /// IsLandingPad - Indicate that this basic block is entered via an
  /// exception handler.
  bool IsLandingPad;

public:
  explicit MachineBasicBlock(const BasicBlock *bb = 0)
    : Prev(0), Next(0), BB(bb), Number(-1), xParent(0),
      Alignment(0), IsLandingPad(false) {
    Insts.parent = this;
  }

  ~MachineBasicBlock();

  /// getBasicBlock - Return the LLVM basic block that this instance
  /// corresponded to originally.
  ///
  const BasicBlock *getBasicBlock() const { return BB; }

  /// getParent - Return the MachineFunction containing this basic block.
  ///
  const MachineFunction *getParent() const { return xParent; }
  MachineFunction *getParent() { return xParent; }

  typedef ilist<MachineInstr>::iterator                       iterator;
  typedef ilist<MachineInstr>::const_iterator           const_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::reverse_iterator<iterator>             reverse_iterator;

  unsigned size() const { return (unsigned)Insts.size(); }
  bool empty() const { return Insts.empty(); }

  MachineInstr& front() { return Insts.front(); }
  MachineInstr& back()  { return Insts.back(); }

  iterator                begin()       { return Insts.begin();  }
  const_iterator          begin() const { return Insts.begin();  }
  iterator                  end()       { return Insts.end();    }
  const_iterator            end() const { return Insts.end();    }
  reverse_iterator       rbegin()       { return Insts.rbegin(); }
  const_reverse_iterator rbegin() const { return Insts.rbegin(); }
  reverse_iterator       rend  ()       { return Insts.rend();   }
  const_reverse_iterator rend  () const { return Insts.rend();   }

  // Machine-CFG iterators
  typedef std::vector<MachineBasicBlock *>::iterator       pred_iterator;
  typedef std::vector<MachineBasicBlock *>::const_iterator const_pred_iterator;
  typedef std::vector<MachineBasicBlock *>::iterator       succ_iterator;
  typedef std::vector<MachineBasicBlock *>::const_iterator const_succ_iterator;
  typedef std::vector<MachineBasicBlock *>::reverse_iterator
                                                         pred_reverse_iterator;
  typedef std::vector<MachineBasicBlock *>::const_reverse_iterator
                                                   const_pred_reverse_iterator;
  typedef std::vector<MachineBasicBlock *>::reverse_iterator
                                                         succ_reverse_iterator;
  typedef std::vector<MachineBasicBlock *>::const_reverse_iterator
                                                   const_succ_reverse_iterator;

  pred_iterator        pred_begin()       { return Predecessors.begin(); }
  const_pred_iterator  pred_begin() const { return Predecessors.begin(); }
  pred_iterator        pred_end()         { return Predecessors.end();   }
  const_pred_iterator  pred_end()   const { return Predecessors.end();   }
  pred_reverse_iterator        pred_rbegin()
                                          { return Predecessors.rbegin();}
  const_pred_reverse_iterator  pred_rbegin() const
                                          { return Predecessors.rbegin();}
  pred_reverse_iterator        pred_rend()
                                          { return Predecessors.rend();  }
  const_pred_reverse_iterator  pred_rend()   const
                                          { return Predecessors.rend();  }
  unsigned             pred_size()  const {
    return (unsigned)Predecessors.size();
  }
  bool                 pred_empty() const { return Predecessors.empty(); }
  succ_iterator        succ_begin()       { return Successors.begin();   }
  const_succ_iterator  succ_begin() const { return Successors.begin();   }
  succ_iterator        succ_end()         { return Successors.end();     }
  const_succ_iterator  succ_end()   const { return Successors.end();     }
  succ_reverse_iterator        succ_rbegin()
                                          { return Successors.rbegin();  }
  const_succ_reverse_iterator  succ_rbegin() const
                                          { return Successors.rbegin();  }
  succ_reverse_iterator        succ_rend()
                                          { return Successors.rend();    }
  const_succ_reverse_iterator  succ_rend()   const
                                          { return Successors.rend();    }
  unsigned             succ_size()  const {
    return (unsigned)Successors.size();
  }
  bool                 succ_empty() const { return Successors.empty();   }

  // LiveIn management methods.

  /// addLiveIn - Add the specified register as a live in.  Note that it
  /// is an error to add the same register to the same set more than once.
  void addLiveIn(unsigned Reg)  { LiveIns.push_back(Reg); }

  /// removeLiveIn - Remove the specified register from the live in set.
  ///
  void removeLiveIn(unsigned Reg);

  /// isLiveIn - Return true if the specified register is in the live in set.
  ///
  bool isLiveIn(unsigned Reg) const;

  // Iteration support for live in sets.  These sets are kept in sorted
  // order by their register number.
  typedef std::vector<unsigned>::iterator       livein_iterator;
  typedef std::vector<unsigned>::const_iterator const_livein_iterator;
  livein_iterator       livein_begin()       { return LiveIns.begin(); }
  const_livein_iterator livein_begin() const { return LiveIns.begin(); }
  livein_iterator       livein_end()         { return LiveIns.end(); }
  const_livein_iterator livein_end()   const { return LiveIns.end(); }
  bool            livein_empty() const { return LiveIns.empty(); }

  /// getAlignment - Return alignment of the basic block.
  ///
  unsigned getAlignment() const { return Alignment; }

  /// setAlignment - Set alignment of the basic block.
  ///
  void setAlignment(unsigned Align) { Alignment = Align; }

  /// isLandingPad - Returns true if the block is a landing pad. That is
  /// this basic block is entered via an exception handler.
  bool isLandingPad() const { return IsLandingPad; }

  /// setIsLandingPad - Indicates the block is a landing pad.  That is
  /// this basic block is entered via an exception handler.
  void setIsLandingPad() { IsLandingPad = true; }

  // Code Layout methods.
  
  /// moveBefore/moveAfter - move 'this' block before or after the specified
  /// block.  This only moves the block, it does not modify the CFG or adjust
  /// potential fall-throughs at the end of the block.
  void moveBefore(MachineBasicBlock *NewAfter);
  void moveAfter(MachineBasicBlock *NewBefore);
  
  // Machine-CFG mutators
  
  /// addSuccessor - Add succ as a successor of this MachineBasicBlock.
  /// The Predecessors list of succ is automatically updated.
  ///
  void addSuccessor(MachineBasicBlock *succ);

  /// removeSuccessor - Remove successor from the successors list of this
  /// MachineBasicBlock. The Predecessors list of succ is automatically updated.
  ///
  void removeSuccessor(MachineBasicBlock *succ);

  /// removeSuccessor - Remove specified successor from the successors list of
  /// this MachineBasicBlock. The Predecessors list of succ is automatically
  /// updated.  Return the iterator to the element after the one removed.
  ///
  succ_iterator removeSuccessor(succ_iterator I);
  
  /// transferSuccessors - Transfers all the successors from MBB to this
  /// machine basic block (i.e., copies all the successors fromMBB and
  /// remove all the successors fromBB).
  void transferSuccessors(MachineBasicBlock *fromMBB);
  
  /// isSuccessor - Return true if the specified MBB is a successor of this
  /// block.
  bool isSuccessor(MachineBasicBlock *MBB) const;

  /// getFirstTerminator - returns an iterator to the first terminator
  /// instruction of this basic block. If a terminator does not exist,
  /// it returns end()
  iterator getFirstTerminator();

  void pop_front() { Insts.pop_front(); }
  void pop_back() { Insts.pop_back(); }
  void push_back(MachineInstr *MI) { Insts.push_back(MI); }
  template<typename IT>
  void insert(iterator I, IT S, IT E) { Insts.insert(I, S, E); }
  iterator insert(iterator I, MachineInstr *M) { return Insts.insert(I, M); }

  // erase - Remove the specified element or range from the instruction list.
  // These functions delete any instructions removed.
  //
  iterator erase(iterator I)             { return Insts.erase(I); }
  iterator erase(iterator I, iterator E) { return Insts.erase(I, E); }
  MachineInstr *remove(MachineInstr *I)  { return Insts.remove(I); }
  void clear()                           { Insts.clear(); }

  /// splice - Take a block of instructions from MBB 'Other' in the range [From,
  /// To), and insert them into this MBB right before 'where'.
  void splice(iterator where, MachineBasicBlock *Other, iterator From,
              iterator To) {
    Insts.splice(where, Other->Insts, From, To);
  }

  /// ReplaceUsesOfBlockWith - Given a machine basic block that branched to
  /// 'Old', change the code and CFG so that it branches to 'New' instead.
  void ReplaceUsesOfBlockWith(MachineBasicBlock *Old, MachineBasicBlock *New);

  /// CorrectExtraCFGEdges - Various pieces of code can cause excess edges in
  /// the CFG to be inserted.  If we have proven that MBB can only branch to
  /// DestA and DestB, remove any other MBB successors from the CFG. DestA and
  /// DestB can be null. Besides DestA and DestB, retain other edges leading
  /// to LandingPads (currently there can be only one; we don't check or require
  /// that here). Note it is possible that DestA and/or DestB are LandingPads.
  bool CorrectExtraCFGEdges(MachineBasicBlock *DestA,
                            MachineBasicBlock *DestB,
                            bool isCond);

  // Debugging methods.
  void dump() const;
  void print(std::ostream &OS) const;
  void print(std::ostream *OS) const { if (OS) print(*OS); }

  /// getNumber - MachineBasicBlocks are uniquely numbered at the function
  /// level, unless they're not in a MachineFunction yet, in which case this
  /// will return -1.
  ///
  int getNumber() const { return Number; }
  void setNumber(int N) { Number = N; }

private:   // Methods used to maintain doubly linked list of blocks...
  friend struct ilist_traits<MachineBasicBlock>;

  MachineBasicBlock *getPrev() const { return Prev; }
  MachineBasicBlock *getNext() const { return Next; }
  void setPrev(MachineBasicBlock *P) { Prev = P; }
  void setNext(MachineBasicBlock *N) { Next = N; }

  // Machine-CFG mutators

  /// addPredecessor - Remove pred as a predecessor of this MachineBasicBlock.
  /// Don't do this unless you know what you're doing, because it doesn't
  /// update pred's successors list. Use pred->addSuccessor instead.
  ///
  void addPredecessor(MachineBasicBlock *pred);

  /// removePredecessor - Remove pred as a predecessor of this
  /// MachineBasicBlock. Don't do this unless you know what you're
  /// doing, because it doesn't update pred's successors list. Use
  /// pred->removeSuccessor instead.
  ///
  void removePredecessor(MachineBasicBlock *pred);
};

std::ostream& operator<<(std::ostream &OS, const MachineBasicBlock &MBB);

//===--------------------------------------------------------------------===//
// GraphTraits specializations for machine basic block graphs (machine-CFGs)
//===--------------------------------------------------------------------===//

// Provide specializations of GraphTraits to be able to treat a
// MachineFunction as a graph of MachineBasicBlocks...
//

template <> struct GraphTraits<MachineBasicBlock *> {
  typedef MachineBasicBlock NodeType;
  typedef MachineBasicBlock::succ_iterator ChildIteratorType;

  static NodeType *getEntryNode(MachineBasicBlock *BB) { return BB; }
  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->succ_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->succ_end();
  }
};

template <> struct GraphTraits<const MachineBasicBlock *> {
  typedef const MachineBasicBlock NodeType;
  typedef MachineBasicBlock::const_succ_iterator ChildIteratorType;

  static NodeType *getEntryNode(const MachineBasicBlock *BB) { return BB; }
  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->succ_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->succ_end();
  }
};

// Provide specializations of GraphTraits to be able to treat a
// MachineFunction as a graph of MachineBasicBlocks... and to walk it
// in inverse order.  Inverse order for a function is considered
// to be when traversing the predecessor edges of a MBB
// instead of the successor edges.
//
template <> struct GraphTraits<Inverse<MachineBasicBlock*> > {
  typedef MachineBasicBlock NodeType;
  typedef MachineBasicBlock::pred_iterator ChildIteratorType;
  static NodeType *getEntryNode(Inverse<MachineBasicBlock *> G) {
    return G.Graph;
  }
  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->pred_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->pred_end();
  }
};

template <> struct GraphTraits<Inverse<const MachineBasicBlock*> > {
  typedef const MachineBasicBlock NodeType;
  typedef MachineBasicBlock::const_pred_iterator ChildIteratorType;
  static NodeType *getEntryNode(Inverse<const MachineBasicBlock*> G) {
    return G.Graph;
  }
  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->pred_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->pred_end();
  }
};

} // End llvm namespace

#endif

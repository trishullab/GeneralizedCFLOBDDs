#include "gcflobdd_bdd_node.h"
#include "../ops/bdd_node_ops.h"


using namespace G_CFL_OBDD;

// ************************************************************
// G_CFLOBDDBDDNode
// ************************************************************

G_CFLOBDDBDDNode::G_CFLOBDDBDDNode(const unsigned int numVars)
  :  G_CFLOBDDNode(0), numVars(numVars)
{
}

G_CFLOBDDBDDNode::~G_CFLOBDDBDDNode()
{
    if (isNumPathsMemAllocated) {
        delete[] numPathsToExit;
    }
}

unsigned int G_CFLOBDDBDDNode::Hash(unsigned int modsize) const
{
//   if (hash_set_997 && modsize == 997) {
//     return cachedHash_997;
//   }
  unsigned int hashValue = 0;
  hashValue = bddSection.Hash(modsize);
//   SetHashCache(modsize, hashValue);
  return hashValue;
}

G_CFLOBDDNodeHandle G_CFLOBDDBDDNode::Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce)
{
  G_CFLOBDDBDDNode* reducedNode = new G_CFLOBDDBDDNode(numVars);
  auto reducedEntryPointHandle = bddSection.entryPointHandle->Reduce(redMapHandle, replacementNumExits, forceReduce);
  G_CFLOBDDReturnMapHandle mI;
  for (int i = 0; i < replacementNumExits; i++) {
    mI.AddToEnd(i);
  }
  mI.Canonicalize();
  reducedNode->bddSection = Section(reducedEntryPointHandle, mI);
  G_CFLOBDDNodeHandle reducedNodeHandle(reducedNode);
  return reducedNodeHandle;
}

// Overloaded !=
bool G_CFLOBDDBDDNode::operator!= (const G_CFLOBDDNode & n) const
{
  return !(*this == n);
}

// Overloaded ==
bool G_CFLOBDDBDDNode::operator== (const G_CFLOBDDNode & n) const
{
  if (n.NodeKind() != G_CFLOBDD_BDD)
    return false;
  G_CFLOBDDBDDNode &m = (G_CFLOBDDBDDNode &)n;
  if (numVars != m.numVars)
    return false;
  if (bddSection != m.bddSection)
    return false;
  return true;
}

void G_CFLOBDDBDDNode::IncrRef()
{
  refCount++;    // Warning: Saturation not checked
}

void G_CFLOBDDBDDNode::DecrRef()
{
  if (--refCount == 0) {    // Warning: Saturation not checked
    if (isCanonical) {
      G_CFLOBDDNodeHandle::canonicalNodeTable->DeleteEq(this);
    }
    delete this;
  }
}

// print
std::ostream& G_CFLOBDDBDDNode::print(std::ostream & out) const
{
  out << "G_CFLOBDDBDDNode(numVars=" << numVars << ", bddSection=" << bddSection << ")";
  return out;
}

void G_CFLOBDDBDDNode::InstallPathCounts()
{
    abort(); // Not implemented for BDD nodes
}

void G_CFLOBDDBDDNode::CountNodesAndEdges(Hashset<G_CFLOBDDNodeHandle>* visitedNodes, Hashset<G_CFLOBDDReturnMapBody>* visitedEdges,
    unsigned int& nodeCount, unsigned int& edgeCount)
{
    // Not implemented for BDD nodes
    abort();
}

void G_CFLOBDDBDDNode::CountPaths(Hashset<G_CFLOBDDNodeHandle>* visitedNodes)
{
    // Not implemented for BDD nodes
    abort();
}

void G_CFLOBDDBDDNode::PrintYield(std::unordered_map<int, std::vector<std::string>>& yield_strings) const
{
  std::unordered_map<int, std::vector<std::string>> tmpYield;
  bddSection.entryPointHandle->handleContents->PrintYield(tmpYield);
  unsigned int yield_size = tmpYield.empty() ? 0 : (tmpYield.begin()->second.empty() ? 0 : tmpYield.begin()->second[0].size());

  for (const auto& [key, vec] : tmpYield) {
      for (const auto& s : vec) {
          std::string prefix_s = s;
          for (unsigned int i = 0; i < (numVars - yield_size); i++) {
              prefix_s = "*" + prefix_s;
          }
          yield_strings[key].push_back(prefix_s);
      }
      if (vec.size() == 0) {
          std::string prefix_s = "";
          for (unsigned int i = 0; i < (numVars - yield_size); i++) {
              prefix_s = "*" + prefix_s;
          }
          yield_strings[key].push_back(prefix_s);
      }
  }
}

Hashset<BDDNode> *BDDNodeHandle::canonicalBDDNodeTable = new Hashset<BDDNode>(10000);

// ***************************************
// BDDNodeHandle
// ***************************************

BDDNodeHandle::BDDNodeHandle()
  : handleContents(NULL)
{
}

BDDNodeHandle::BDDNodeHandle(BDDNode *n)
  : handleContents(n)
{
  if (handleContents != NULL) {
    handleContents->IncrRef();
    Canonicalize();
  }
}

BDDNodeHandle::BDDNodeHandle(const BDDNodeHandle &nh)
  : handleContents(nh.handleContents)
{
  if (handleContents != NULL) {
    handleContents->IncrRef();
  }
}

BDDNodeHandle::~BDDNodeHandle()
{
  if (handleContents != NULL)
    handleContents->DecrRef();
}

unsigned int BDDNodeHandle::Hash(unsigned int modsize) const
{
  return (reinterpret_cast<std::uintptr_t>(handleContents) >> 2) % modsize;
}

bool BDDNodeHandle::operator!= (const BDDNodeHandle &nh) const
{
  return handleContents != nh.handleContents;
}

bool BDDNodeHandle::operator== (const BDDNodeHandle &nh) const
{
  return handleContents == nh.handleContents;
}

BDDNodeHandle & BDDNodeHandle::operator= (const BDDNodeHandle &nh)
{
  if (this != &nh)      // don't assign to self!
  {
    BDDNode* temp = handleContents;
    handleContents = nh.handleContents;
    if (handleContents != NULL) {
      handleContents->IncrRef();
    }
    if (temp != NULL) {
      temp->DecrRef();
    }
  }
  return *this;        
}

void BDDNodeHandle::Canonicalize()
{
  BDDNode *answerContents;
  if (!handleContents->IsCanonical()) {
    unsigned hash = canonicalBDDNodeTable->GetHash(handleContents);
    answerContents = canonicalBDDNodeTable->Lookup(handleContents, hash);;
    if (answerContents == NULL) {
      canonicalBDDNodeTable->Insert(handleContents, hash);
      handleContents->SetCanonical();
    } else {
        answerContents->IncrRef();
        handleContents->DecrRef();
        handleContents = answerContents;
    }
  }
}

std::ostream& BDDNodeHandle::print(std::ostream & out) const
{
  out << *handleContents << std::endl;
  return out;
}

//********************************************************************
// BDDReduceKey
//********************************************************************

// Constructor
BDDReduceKey::BDDReduceKey(BDDNodeHandle nodeHandle, ReductionMapHandle redMapHandle)
  :  nodeHandle(nodeHandle), redMapHandle(redMapHandle)
{
}

// Hash
unsigned int BDDReduceKey::Hash(unsigned int modsize) const
{
  unsigned int hvalue = 0;
  hvalue = (997 * nodeHandle.Hash(modsize) + redMapHandle.Hash(modsize)) % modsize;
  return hvalue;
}

// print
std::ostream& BDDReduceKey::print(std::ostream & out) const
{
  out << "(" << nodeHandle << ", " << redMapHandle << ")";
  return out;
}

std::ostream& operator<< (std::ostream & out, const BDDReduceKey &p)
{
  p.print(out);
  return(out);
}

BDDReduceKey& BDDReduceKey::operator= (const BDDReduceKey& i)
{
  if (this != &i)      // don't assign to self!
  {
    nodeHandle = i.nodeHandle;
    redMapHandle = i.redMapHandle;
  }
  return *this;        
}

// Overloaded !=
bool BDDReduceKey::operator!=(const BDDReduceKey& p)
{
  return (nodeHandle != p.nodeHandle) || (redMapHandle != p.redMapHandle);
}

// Overloaded ==
bool BDDReduceKey::operator==(const BDDReduceKey& p)
{
  return (nodeHandle == p.nodeHandle) && (redMapHandle == p.redMapHandle);
}

static Hashtable<BDDReduceKey, BDDNodeHandle> *reduceCache = NULL;
BDDNodeHandle BDDNodeHandle::Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce)
{
  // if (replacementNumExits == 1 && !forceReduce) {
	// 	return MkNoDistinction_BDD(handleContents->NumVars());
	// }

	// if (redMapHandle.mapContents->isIdentityMap && !forceReduce) {
	// 	return *this;
	// }

	BDDNodeHandle cachedNodeHandle;
	bool isCached = reduceCache->Fetch(BDDReduceKey(*this, redMapHandle), cachedNodeHandle);
	if (isCached && this->handleContents->NodeKind() != BDD_LEAF) {
		return cachedNodeHandle;
	}
	else {
		BDDNodeHandle temp;
		temp = handleContents->Reduce(redMapHandle, replacementNumExits, forceReduce);
		reduceCache->Insert(BDDReduceKey(*this, redMapHandle), temp);
		return temp;
	}
}

void BDDNodeHandle::InitReduceCache()
{
  reduceCache = new Hashtable<BDDReduceKey, BDDNodeHandle>(HASH_NUM_BUCKETS);
}

void BDDNodeHandle::DisposeOfReduceCache()
{
	delete reduceCache;
	reduceCache = NULL;
}

// ********************************
// BDDNode
// ********************************

BDDNode::BDDNode()
  : numVars(0), refCount(0), isCanonical(false)
{
}

BDDNode::BDDNode(const unsigned int numVars, const unsigned int varID)
  : numVars(numVars), varID(varID), refCount(0), isCanonical(false)
{
}

BDDNode::~BDDNode()
{
}

void BDDNode::IncrRef()
{
  refCount++;    // Warning: Saturation not checked
}

void BDDNode::DecrRef()
{
  if (--refCount == 0) {    // Warning: Saturation not checked
    if (isCanonical) {
      BDDNodeHandle::canonicalBDDNodeTable->DeleteEq(this);
    }
    delete this;
  }
}

namespace G_CFL_OBDD {
std::ostream& operator<< (std::ostream & out, const BDDNode &d)
{
  d.print(out);
  return(out);
}
}

// ***************************************
// BDDInternalNode
// ***************************************

BDDInternalNode::BDDInternalNode(const unsigned int numVars, const unsigned int varID)
  : BDDNode(numVars, varID)
{
}

BDDInternalNode::~BDDInternalNode()
{
}

unsigned int BDDInternalNode::Hash(unsigned int modsize) const
{
  unsigned int hashValue = 0;
  hashValue = (997 * thenBranch.Hash(modsize) + elseBranch.Hash(modsize) + numVars) % modsize;
  return hashValue;
}

// Overloaded !=
bool BDDInternalNode::operator!= (const BDDNode & n) const
{
  return !(*this == n);
}

// Overloaded ==
bool BDDInternalNode::operator== (const BDDNode & n) const
{
  if (n.NumVars() != numVars)
    return false;
  BDDInternalNode &m = (BDDInternalNode &)n;
  if (thenBranch != m.thenBranch)
    return false;
  if (elseBranch != m.elseBranch)
    return false;
  return true;
}

 G_CFLOBDDReturnMapHandle GetInducedReductionMapHandle(ReductionMapHandle& redMapHandle, G_CFLOBDDReturnMapHandle& returnMapHandle, ReductionMapHandle& inducedRedMapHandle) {
  G_CFLOBDDReturnMapHandle newReturnMapHandle;
  std::unordered_map<int, int> returnValToIndex;
    for (unsigned int i = 0; i < returnMapHandle.Size(); i++) {
        unsigned int returnVal = returnMapHandle.Lookup(i);
        unsigned int redReturnVal = redMapHandle.Lookup(returnVal);
        if (returnValToIndex.find(redReturnVal) == returnValToIndex.end()) {
            returnValToIndex.emplace(redReturnVal, newReturnMapHandle.Size());
            inducedRedMapHandle.AddToEnd(redReturnVal);
            newReturnMapHandle.AddToEnd(redReturnVal);
        }
        else {
            inducedRedMapHandle.AddToEnd(redReturnVal);
        }
    }
    inducedRedMapHandle.Canonicalize();
    newReturnMapHandle.Canonicalize();
    return newReturnMapHandle;
}

// Reduce
BDDNodeHandle BDDInternalNode::Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce)
{
  ReductionMapHandle thenBranchRedMapHandle, elseBranchRedMapHandle;
  G_CFLOBDDReturnMapHandle newThenReturnMapHandle = GetInducedReductionMapHandle(redMapHandle, thenBranch.returnMapHandle, thenBranchRedMapHandle);
  G_CFLOBDDReturnMapHandle newElseReturnMapHandle = GetInducedReductionMapHandle(redMapHandle, elseBranch.returnMapHandle, elseBranchRedMapHandle);
  auto reducedThenBranch = thenBranch.entryPointHandle->Reduce(redMapHandle, replacementNumExits, forceReduce);
  auto reducedElseBranch = elseBranch.entryPointHandle->Reduce(redMapHandle, replacementNumExits, forceReduce);
  if (reducedThenBranch == reducedElseBranch) {
      return reducedThenBranch;
  }
  BDDInternalNode* reducedNode = new BDDInternalNode(numVars, varID);
  reducedNode->thenBranch = Section(reducedThenBranch, newThenReturnMapHandle);
  reducedNode->elseBranch = Section(reducedElseBranch, newElseReturnMapHandle);
  BDDNodeHandle reducedNodeHandle(reducedNode);
  return reducedNodeHandle;
}

// print
std::ostream& BDDInternalNode::print(std::ostream & out) const
{
  out << "BDDInternalNode(numVars=" << numVars << ", thenBranch=" << thenBranch << ", elseBranch=" << elseBranch << ")";
  return out;
}

void BDDInternalNode::PrintYield(std::unordered_map<int, std::vector<std::string>>& yield_strings) const {
  std::unordered_map<int, std::vector<std::string>> thenYield, elseYield;
  thenBranch.entryPointHandle->handleContents->PrintYield(thenYield);
  elseBranch.entryPointHandle->handleContents->PrintYield(elseYield);
  // size of a yeild string
  unsigned int then_yield_size = thenYield.empty() ? 0 : (thenYield.begin()->second.empty() ? 0 : thenYield.begin()->second[0].size());
  unsigned int else_yield_size = elseYield.empty() ? 0 : (elseYield.begin()->second.empty() ? 0 : elseYield.begin()->second[0].size());
  for (const auto& [key, vec] : thenYield) {
      for (const auto& s : vec) {
        std::string prefix_s = s;
        for (unsigned int i = 0; i < (numVars - then_yield_size - 1); i++) {
            prefix_s = "*" + prefix_s;
        }
        yield_strings[key].push_back("0" + prefix_s);
      }
      if (vec.size() == 0) {
          std::string prefix_s = "";
          for (unsigned int i = 0; i < (numVars - then_yield_size - 1); i++) {
              prefix_s = "*" + prefix_s;
          }
          yield_strings[key].push_back("0" + prefix_s);
      }
  }
  for (const auto& [key, vec] : elseYield) {
      for (const auto& s : vec) {
          std::string prefix_s = s;
          for (unsigned int i = 0; i < (numVars - else_yield_size - 1); i++) {
              prefix_s = "*" + prefix_s;
          }
          yield_strings[key].push_back("1" + prefix_s);
      }
      if (vec.size() == 0) {
          std::string prefix_s = "";
          for (unsigned int i = 0; i < (numVars - else_yield_size - 1); i++) {
              prefix_s = "*" + prefix_s;
          }
          yield_strings[key].push_back("1" + prefix_s);
      }
  }
}

// ***************************************
// BDDLeafNode
// *************************************** 
BDDLeafNode::BDDLeafNode(const unsigned int value, const unsigned int varID, const unsigned int numVars)
  : BDDNode(numVars, varID), value(value)
{
}

BDDLeafNode::~BDDLeafNode()
{
}

unsigned int BDDLeafNode::Hash(unsigned int modsize) const
{
  unsigned int hashValue = 0;
  hashValue = (value) % modsize;
  return hashValue;
}

// Overloaded !=
bool BDDLeafNode::operator!= (const BDDNode & n) const
{
  return !(*this == n);
}

// Overloaded ==
bool BDDLeafNode::operator== (const BDDNode & n) const
{
  if (n.NumVars() != 0)
    return false;
  BDDLeafNode &m = (BDDLeafNode &)n;
  if (value != m.value)
    return false;
  return true;
}

BDDNodeHandle BDDLeafNode::Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce)
{
  // std::cout << "Reducing leaf node with value " << value << std::endl;
  // std::cout << "Using reduction map: " << redMapHandle << std::endl;
  if (redMapHandle.Lookup(value) == value) {
      return BDDNodeHandle(this);
  } else {
      BDDLeafNode* reducedNode = new BDDLeafNode(redMapHandle.Lookup(value), varID, numVars);
      BDDNodeHandle reducedNodeHandle(reducedNode);
      return reducedNodeHandle;
  }
}

// print
std::ostream& BDDLeafNode::print(std::ostream & out) const
{
  out << "BDDLeafNode(value=" << value << ")";
  return out;
}

void BDDLeafNode::PrintYield(std::unordered_map<int, std::vector<std::string>>& yield_strings) const {
  std::vector<std::string> vec;
  yield_strings.insert({value, vec});
}

// ***************************************
// BDDNodeHandle operator<<
// ***************************************
namespace G_CFL_OBDD {
std::ostream& operator<< (std::ostream & out, const BDDNodeHandle &c)
{
  c.print(out);
  return(out);
}
}
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <cstdarg>
#include <unordered_set>
#include <map>
#include <unordered_map>

#include "gcflobdd_node.h"
#include "connectionT.h"
#include "connectionListT.h"
#include "../utils/list_T.h"
#include "../utils/list_TPtr.h"
#include "../utils/intpair.h"
#include "../utils/bool_op.h"
#include "return_map_T.h"
#include "reduction_map.h"
#include "../utils/hash.h"
#include "../utils/hashset.h"
#include "../ops/gcflobdd_node_ops.h"
#include "../utils/pair_T.h"

using namespace G_CFL_OBDD;

//********************************************************************
// G_CFLOBDDNodeHandle
//
// Contains a canonical G_CFLOBDDNode*
//********************************************************************

// Initializations of static members ---------------------------------

// size_t G_CFLOBDDNodeHash::operator()(const std::shared_ptr<G_CFLOBDDNode>& c) const {
//   return c ? c->Hash(997) : 0;  // expired nodes hash to 0
// }

// bool G_CFLOBDDNodeEqual::operator()(const std::shared_ptr<G_CFLOBDDNode>& a,
//                                     const std::shared_ptr<G_CFLOBDDNode>& b) const {

//     if (!a || !b) return false; // treat expired as unequal
//       return *a == *b;
// }


// --------------------------------------------------------------------
// NoDistinctionCacheKey methods
// --------------------------------------------------------------------
NoDistinctionCacheKey::NoDistinctionCacheKey(unsigned int level, std::shared_ptr<GrammarNode>& grammar)
    : level(level), grammar(grammar) {}

unsigned int NoDistinctionCacheKey::Hash(unsigned int modsize) const {
    size_t h1 = std::hash<unsigned int>{}(level);
    size_t h2 = std::hash<std::shared_ptr<GrammarNode>>{}(grammar);
    return (h1 ^ (h2 << 1)) % modsize;
}

NoDistinctionCacheKey& NoDistinctionCacheKey::operator= (const NoDistinctionCacheKey& p) {
    if (this != &p) {
        level = p.level;
        grammar = p.grammar;
    }
    return *this;
}

bool NoDistinctionCacheKey::operator!= (const NoDistinctionCacheKey& p) const {
    return (level != p.level || grammar != p.grammar);
}

bool NoDistinctionCacheKey::operator== (const NoDistinctionCacheKey& p) const {
    return (level == p.level && grammar == p.grammar);
}


Hashset<G_CFLOBDDNode> *G_CFLOBDDNodeHandle::canonicalNodeTable = new Hashset<G_CFLOBDDNode>(HASHSET_NUM_BUCKETS);
// std::unordered_set<std::shared_ptr<G_CFLOBDDNode>, G_CFLOBDDNodeHash, G_CFLOBDDNodeEqual> G_CFLOBDDNodeHandle::canonicalNodeTable;
G_CFLOBDDNodeHandle G_CFLOBDDNodeHandle::G_CFLOBDDForkNodeHandle;
G_CFLOBDDNodeHandle G_CFLOBDDNodeHandle::G_CFLOBDDDontCareNodeHandle;
std::unordered_map<NoDistinctionCacheKey, G_CFLOBDDNodeHandle, NoDistinctionCacheKey::NoDistinctionCacheKey_Hash, NoDistinctionCacheKey::NoDistinctionCacheKey_Equal> G_CFLOBDDNodeHandle::NoDistinctionNode;

void G_CFLOBDDNodeHandle::InitLeafNodes() {
    if (G_CFLOBDDForkNodeHandle.handleContents == NULL) {
        G_CFLOBDDForkNodeHandle.handleContents = new G_CFLOBDDForkNode();
    }
    if (G_CFLOBDDDontCareNodeHandle.handleContents == NULL) {
        G_CFLOBDDDontCareNodeHandle.handleContents = new G_CFLOBDDDontCareNode();
    }
    NoDistinctionCacheKey key0(1, G_CFLOBDDNodeHandle::G_CFLOBDDDontCareNodeHandle.handleContents->grammar);
    NoDistinctionNode[key0] = G_CFLOBDDNodeHandle::G_CFLOBDDDontCareNodeHandle;
}

// Constructors/Destructor -------------------------------------------

// Default constructor
G_CFLOBDDNodeHandle::G_CFLOBDDNodeHandle()
  : handleContents(NULL)
{
}

// Constructor
//
// Construct and canonicalize
//
G_CFLOBDDNodeHandle::G_CFLOBDDNodeHandle(G_CFLOBDDNode *n)
  : handleContents(n)
{
  assert(n != NULL);
  handleContents->IncrRef();
  Canonicalize();
}

// Copy constructor
G_CFLOBDDNodeHandle::G_CFLOBDDNodeHandle(const G_CFLOBDDNodeHandle &c)
{
  handleContents = c.handleContents;
  if (handleContents != NULL) {
    handleContents->IncrRef();
  }
}

// // Constructor from shared_ptr
// G_CFLOBDDNodeHandle::G_CFLOBDDNodeHandle(const std::shared_ptr<G_CFLOBDDNode>& n)
//   : handleContents(n)
// {
//   assert(n != NULL);
//   handleContents->IncrRef();
//   Canonicalize();
// }

G_CFLOBDDNodeHandle::~G_CFLOBDDNodeHandle()
{
  if (handleContents != NULL) {
    handleContents->DecrRef();
  }
}

void G_CFLOBDDNodeHandle::GarbageCollectCanonicalNodeTable()
{
    // for (auto it = canonicalNodeTable.begin(); it != canonicalNodeTable.end(); ) {
    //     if (it->use_count() == 1) { // only held by the canonical table
    //         it = canonicalNodeTable.erase(it);
    //     } else {
    //         ++it;
    //     }
    // }
}

// Hash
unsigned int G_CFLOBDDNodeHandle::Hash(unsigned int modsize) const
{
  return ((unsigned int) reinterpret_cast<uintptr_t>(handleContents) >> 2) % modsize;
  // return ((unsigned int) reinterpret_cast<uintptr_t>(handleContents.get()) >> 2) % modsize;
}

// Overloaded !=
bool G_CFLOBDDNodeHandle::operator!= (const G_CFLOBDDNodeHandle & C) const
{
  return handleContents != C.handleContents;
}

// Overloaded ==
bool G_CFLOBDDNodeHandle::operator== (const G_CFLOBDDNodeHandle & C) const
{
  return handleContents == C.handleContents;
}

// Overloaded assignment
G_CFLOBDDNodeHandle & G_CFLOBDDNodeHandle::operator= (const G_CFLOBDDNodeHandle &c)
{
  if (this != &c)      // don't assign to self!
  {
    G_CFLOBDDNode *temp = handleContents;
    handleContents = c.handleContents;
    if (handleContents != NULL) {
      handleContents->IncrRef();
    }
    if (temp != NULL) {
      temp->DecrRef();
    }
  }
  return *this;        
    // if (this != &c)      // don't assign to self!
    // {
    //     handleContents = c.handleContents;
    // }
    // return *this;
}

//********************************************************************
// CFLReduceKey
//********************************************************************

// Constructor
CFLReduceKey::CFLReduceKey(G_CFLOBDDNodeHandle nodeHandle, ReductionMapHandle redMapHandle)
  :  nodeHandle(nodeHandle), redMapHandle(redMapHandle)
{
}

// Hash
unsigned int CFLReduceKey::Hash(unsigned int modsize) const
{
  unsigned int hvalue = 0;
  hvalue = (997 * nodeHandle.Hash(modsize) + redMapHandle.Hash(modsize)) % modsize;
  return hvalue;
}

// print
std::ostream& CFLReduceKey::print(std::ostream & out) const
{
  out << "(" << nodeHandle << ", " << redMapHandle << ")";
  return out;
}

std::ostream& operator<< (std::ostream & out, const CFLReduceKey &p)
{
  p.print(out);
  return(out);
}

CFLReduceKey& CFLReduceKey::operator= (const CFLReduceKey& i)
{
  if (this != &i)      // don't assign to self!
  {
    nodeHandle = i.nodeHandle;
    redMapHandle = i.redMapHandle;
  }
  return *this;        
}

// Overloaded !=
bool CFLReduceKey::operator!=(const CFLReduceKey& p)
{
  return (nodeHandle != p.nodeHandle) || (redMapHandle != p.redMapHandle);
}

// Overloaded ==
bool CFLReduceKey::operator==(const CFLReduceKey& p)
{
  return (nodeHandle == p.nodeHandle) && (redMapHandle == p.redMapHandle);
}

// Reduce and its associated cache ----------------------------------

static Hashtable<CFLReduceKey, G_CFLOBDDNodeHandle> *reduceCache = NULL;

G_CFLOBDDNodeHandle G_CFLOBDDNodeHandle::Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce)
{
	// if (replacementNumExits == 1 && !forceReduce) {
	// 	return MkNoDistinction(handleContents->level, handleContents->grammar);
	// }

	if (redMapHandle.mapContents->isIdentityMap && !forceReduce) {
		return *this;
	}

	G_CFLOBDDNodeHandle cachedNodeHandle;
	bool isCached = reduceCache->Fetch(CFLReduceKey(*this, redMapHandle), cachedNodeHandle);
	if (isCached) {
		return cachedNodeHandle;
	}
	else {
		G_CFLOBDDNodeHandle temp;
		temp = handleContents->Reduce(redMapHandle, replacementNumExits, forceReduce);
		reduceCache->Insert(CFLReduceKey(*this, redMapHandle), temp);
		return temp;
	}
}

void G_CFLOBDDNodeHandle::InitReduceCache()
{
  reduceCache = new Hashtable<CFLReduceKey, G_CFLOBDDNodeHandle>(HASH_NUM_BUCKETS);
}

void G_CFLOBDDNodeHandle::DisposeOfReduceCache()
{
	delete reduceCache;
	reduceCache = NULL;
}

// Canonicalization --------------------------------------------
void G_CFLOBDDNodeHandle::Canonicalize()
{
  // handleContents->SetHashCache(997, handleContents->Hash(997));
  G_CFLOBDDNode *answerContents;

  if (!handleContents->IsCanonical()) {
	  unsigned int hash = canonicalNodeTable->GetHash(handleContents);
    answerContents = canonicalNodeTable->Lookup(handleContents, hash);
    if (answerContents == NULL) {
      canonicalNodeTable->Insert(handleContents, hash);
      handleContents->SetCanonical();
    }
    else {
      answerContents->IncrRef();
      handleContents->DecrRef();
      handleContents = answerContents;
    }
  }
}

// print
std::ostream& G_CFLOBDDNodeHandle::print(std::ostream & out) const
{
  out << *handleContents << std::endl;
  return out;
}

namespace G_CFL_OBDD {

std::ostream& operator<< (std::ostream & out, const G_CFLOBDDNodeHandle &d)
{
  d.print(out);
  return(out);
}

}

//********************************************************************
// G_CFLOBDDNode
//********************************************************************

// Initializations of static members ---------------------------------

unsigned int const G_CFLOBDDNode::maxLevel = G_CFLOBDDMaxLevel;

// Constructors/Destructor -------------------------------------------

// Default constructor
G_CFLOBDDNode::G_CFLOBDDNode()
	: level(maxLevel), refCount(0), isCanonical(false), isNumPathsMemAllocated(false)
{
}

// Constructor
G_CFLOBDDNode::G_CFLOBDDNode(const unsigned int l)
	: level(l), refCount(0), isCanonical(false), isNumPathsMemAllocated(false)
{
}

G_CFLOBDDNode::~G_CFLOBDDNode()
{
}

void G_CFLOBDDNode::SetHashCache(unsigned int modsize,  unsigned int hashValue)
{
  if (modsize == 997) {
    cachedHash_997 = hashValue;
    hash_set_997 = true;
  }
}

// print
namespace G_CFL_OBDD {
std::ostream& operator<< (std::ostream & out, const G_CFLOBDDNode &n)
{
  n.print(out);
  return(out);
}
}

//********************************************************************
// G_CFLOBDDInternalNode
//********************************************************************

// Constructors/Destructor -------------------------------------------

G_CFLOBDDInternalNode::G_CFLOBDDInternalNode(const unsigned int l)
  :  G_CFLOBDDNode(l)
{
}

G_CFLOBDDInternalNode::~G_CFLOBDDInternalNode()
{
  delete [] connections;
  if (isNumPathsMemAllocated)
	delete [] numPathsToExit;
}

G_CFLOBDDReturnMapHandle ComposeAndReduce(G_CFLOBDDReturnMapHandle& mapHandle, ReductionMapHandle& redMapHandle, ReductionMapHandle& inducedRedMapHandle)
{
	int c2, c3;
	int size = mapHandle.mapContents->mapArray.size();
	G_CFLOBDDReturnMapHandle answer (size);
	if (redMapHandle.mapContents->isIdentityMap){
		inducedRedMapHandle = redMapHandle;
		return mapHandle;
	}
	std::unordered_map<int, unsigned int> reductionMap (size);
	for (int i = 0; i < size; i++)
	{
		c2 = mapHandle.mapContents->mapArray[i];
		c3 = redMapHandle.Lookup(c2);
		if (reductionMap.find(c3) == reductionMap.end()){
			answer.AddToEnd(c3); 	  // Why not answer.AddToEnd(c3);
			reductionMap.emplace(c3, answer.Size() - 1);
			inducedRedMapHandle.AddToEnd(answer.Size() - 1);
      // inducedRedMapHandle.mapContents->mapArray[i] = answer.Size() - 1;
		}
		else{
			inducedRedMapHandle.AddToEnd(reductionMap[c3]);
      // inducedRedMapHandle.mapContents->mapArray[i] = reductionMap[c3];
		}
	}
	inducedRedMapHandle.Canonicalize();
	answer.Canonicalize();
	return answer;
}

G_CFLOBDDNodeHandle G_CFLOBDDInternalNode::Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce)
{
  G_CFLOBDDInternalNode* n = new G_CFLOBDDInternalNode(level);
  n->numLayers = numLayers;
  n->connections = new ConnectionList[n->numLayers];
  auto currentRedMapHandle = redMapHandle;
  for (int layer = numLayers - 1; layer >= 0; layer--) {
      ConnectionList& currentConnections = connections[layer];
      ReductionMapHandle nextRedMapHandle (currentConnections.Size());
      unsigned int currPosition = 0;
      // std::unordered_map<Connection, unsigned int, Connection::ConnectionHash, Connection::ConnectionEqual> connMap (currentConnections.Size());
      std::unordered_map<Pair_T<G_CFLOBDDNodeHandle, G_CFLOBDDReturnMapHandle>, unsigned int, Pair_T<G_CFLOBDDNodeHandle, G_CFLOBDDReturnMapHandle>::pair_hash, Pair_T<G_CFLOBDDNodeHandle, G_CFLOBDDReturnMapHandle>::pair_equal> connMap (currentConnections.Size());
      ConnectionList tempConnections(currentConnections.Size());
      for (unsigned int i = 0; i < currentConnections.Size(); i++) {
          auto& conn = currentConnections[i];
          ReductionMapHandle inducedRedMapHandle;
          G_CFLOBDDReturnMapHandle inducedReturnMap = ComposeAndReduce(conn.returnMapHandle, currentRedMapHandle, inducedRedMapHandle);
          G_CFLOBDDNodeHandle reducedNodeHandle = conn.entryPointHandle->Reduce(inducedRedMapHandle, inducedReturnMap.Size(), forceReduce);
          Pair_T<G_CFLOBDDNodeHandle, G_CFLOBDDReturnMapHandle> newConnPair(reducedNodeHandle, inducedReturnMap);
          auto it = connMap.find(newConnPair);
          // auto currPositionInNextMap = n->connections[layer].LookupInv(newConn);
          // if (currPositionInNextMap != -1) {
          if (it != connMap.end()) {
              // nextRedMapHandle.AddToEnd(currPositionInNextMap);
              nextRedMapHandle.AddToEnd(it->second);
          }
          else {
            // n->connections[layer].AddConnection(newConn);
            Connection newConn(reducedNodeHandle, inducedReturnMap);
            tempConnections.AddConnection(newConn);
            nextRedMapHandle.AddToEnd(currPosition);
            currPosition++;
            connMap.insert(std::make_pair(newConnPair, currPosition - 1));
          } 
      }
      n->connections[layer].Reserve(tempConnections.Size());
      for (int i = 0; i < tempConnections.Size(); i++) {
          n->connections[layer].AddConnection(tempConnections[i]);
      }
      // std::cout << "Num elements: " << connMap.size() << " " << "load factor: " << connMap.load_factor() << " bucket count: " << connMap.bucket_count() << std::endl;
      // No need to delete tempConnections as it's not dynamically allocated
      nextRedMapHandle.Canonicalize();
      currentRedMapHandle = nextRedMapHandle;
      // n->connections[layer].Canonicalize();
  }
  // Other material that has to be filled in
  n->numExits = replacementNumExits;
  n->grammar = grammar;
#ifdef PATH_COUNTING_ENABLED
     n->InstallPathCounts();
#endif
  return G_CFLOBDDNodeHandle(n);
} // G_CFLOBDDInternalNode::Reduce

unsigned int G_CFLOBDDInternalNode::Hash(unsigned int modsize) const
{
  // if (modsize == 997 && hash_set_997) {
  //   return cachedHash_997;
  // }
  unsigned int hvalue = 0;
  for (unsigned int j = 0; j < numLayers; j++) {
    hvalue = (997 * hvalue + connections[j].Hash(modsize)) % modsize;
  }
  return hvalue;
}

// Overloaded !=
bool G_CFLOBDDInternalNode::operator!= (const G_CFLOBDDNode & n) const
{
  return !(*this == n);
}

// Overloaded ==
bool G_CFLOBDDInternalNode::operator== (const G_CFLOBDDNode & n) const
{
  if (n.NodeKind() != G_CFLOBDD_INTERNAL)
    return false;
  G_CFLOBDDInternalNode &m = (G_CFLOBDDInternalNode &)n;
  if (level != m.level)
    return false;
  if (numExits != m.numExits)
    return false;
  if (numLayers != m.numLayers)
    return false;
  for (unsigned int j = 0; j < numLayers; j++) {
    if (connections[j] != m.connections[j])
      return false;
  }
  return true;
}

void G_CFLOBDDInternalNode::IncrRef()
{
  refCount++;    // Warning: Saturation not checked
}

void G_CFLOBDDInternalNode::DecrRef()
{
  if (--refCount == 0) {    // Warning: Saturation not checked
    if (isCanonical) {
      G_CFLOBDDNodeHandle::canonicalNodeTable->DeleteEq(this);
    }
    delete this;
  }
}

// print
std::ostream& G_CFLOBDDInternalNode::print(std::ostream & out) const
{
    unsigned int indentation_level = 5;
    unsigned int i, j;
    
    for (j = 0; j < numLayers; j++) {
	    for (i = level; i < indentation_level; i++) {  // Indentation
	        out << "  ";
        }
        out << "Level: " << level << " ";
        out << "Layer[" << j << "]:" << std::endl;
		    connections[j].print(out);
		  for (i = level; i < indentation_level; i++) {  // Indentation
            out << "  ";
        }
    }
    return out;
}

void G_CFLOBDDInternalNode::CountNodesAndEdges(Hashset<G_CFLOBDDNodeHandle>* visitedNodes, Hashset<G_CFLOBDDReturnMapBody>* visitedEdges,
	unsigned int& nodeCount, unsigned int& edgeCount)
{
  if (visitedNodes->Lookup(new G_CFLOBDDNodeHandle(this)) == NULL) {
    visitedNodes->Insert(new G_CFLOBDDNodeHandle(this));
    nodeCount++;
    for (unsigned int layer = 0; layer < numLayers; layer++) {
      for (unsigned int i = 0; i < connections[layer].Size(); i++)
      {
        Connection& conn = connections[layer][i];
        conn.entryPointHandle->handleContents->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
        if (visitedEdges->Lookup(conn.returnMapHandle.mapContents) == NULL) {
          visitedEdges->Insert(conn.returnMapHandle.mapContents);
          edgeCount += conn.returnMapHandle.Size();
        }
      }
      edgeCount += connections[layer].Size(); // for the connections from this node to the layer
    }
  }
}

void G_CFLOBDDInternalNode::InstallPathCounts() {
  numPathsToExit = new long double[numExits];
  isNumPathsMemAllocated = true;
  for (unsigned int i = 0; i < numExits; i++) {
    numPathsToExit[i] = 0;
  }

  std::vector<long double> tempPathsToExitLayerI (1, 1);
  std::vector<long double> tempPathsToExitLayerIPlus1;

  for (int layer = 0; layer < (int) numLayers; layer++) {
    ConnectionList& currentConnections = connections[layer];
    tempPathsToExitLayerIPlus1.clear();
    if (layer + 1 < (int) numLayers)
      tempPathsToExitLayerIPlus1.resize(connections[layer + 1].Size(), 0);
    else
      tempPathsToExitLayerIPlus1.resize(numExits, 0);
    for (unsigned int i = 0; i < currentConnections.Size(); i++) {
      Connection& conn = currentConnections[i];
      G_CFLOBDDNode* childNode = conn.entryPointHandle->handleContents;
      for (unsigned int j = 0; j < conn.returnMapHandle.Size(); j++) {
        unsigned int exitIndexInChild = conn.returnMapHandle.mapContents->mapArray[j];
        tempPathsToExitLayerIPlus1[exitIndexInChild] += tempPathsToExitLayerI[i] * childNode->numPathsToExit[j];
      }
    }
    tempPathsToExitLayerI = tempPathsToExitLayerIPlus1;
  }

  for (unsigned int i = 0; i < tempPathsToExitLayerI.size(); i++) {
    numPathsToExit[i] = tempPathsToExitLayerI[i];
  }
}

void G_CFLOBDDInternalNode::CountPaths(Hashset<G_CFLOBDDNodeHandle> *visitedNodes)
{
	G_CFLOBDDNodeHandle* handle = new G_CFLOBDDNodeHandle(this);
	if (visitedNodes->Lookup(handle) == NULL) {
		visitedNodes->Insert(handle);
    for (unsigned int layer = 0; layer < numLayers; layer++) {
      for (unsigned int i = 0; i < connections[layer].Size(); i++)
      {
          Connection& conn = connections[layer][i];
          conn.entryPointHandle->handleContents->CountPaths(visitedNodes);
      }
    }
		InstallPathCounts();
	}
}

void G_CFLOBDDInternalNode::PrintYield(std::unordered_map<int, std::vector<std::string>>& yield_strings) const
{
  for (unsigned int layer = 0; layer < numLayers; layer++) {
    std::unordered_map<int, std::vector<std::string>> new_yield_strings;
    for (unsigned int i = 0; i < connections[layer].Size(); i++)
    {
        Connection& conn = connections[layer][i];
        std::unordered_map<int, std::vector<std::string>> yield_strings_for_layer; 
        conn.entryPointHandle->handleContents->PrintYield(yield_strings_for_layer);
        if (i == 0) {
          new_yield_strings = yield_strings_for_layer;
        }
        else {
          for (const auto& pair : yield_strings_for_layer) {
            int exitIndexInChild = pair.first;
            const std::vector<std::string>& yield_strings_for_child_exit = pair.second;
            int exitIndexInCurrentNode = conn.returnMapHandle[exitIndexInChild];
            for (const std::string& yield_string : yield_strings_for_child_exit) {
              for (const std::string& existing_yield_string : yield_strings[i]) {
                new_yield_strings[exitIndexInCurrentNode].push_back(existing_yield_string + yield_string);
              }
            }
          }
        }
    }
    yield_strings = new_yield_strings;
  }
}

//********************************************************************
// G_CFLOBDDLeafNode
//********************************************************************

// Constructors/Destructor -------------------------------------------

// Default constructor
G_CFLOBDDLeafNode::G_CFLOBDDLeafNode()
  :  G_CFLOBDDNode(0)
{
  refCount = 1;
  GrammarNonTerminalNode* gntn = new GrammarNonTerminalNode();
  gntn->addChild(Grammar::terminalNode);
  gntn->level = 0;
  gntn->numVars = 1;
  grammar = std::shared_ptr<GrammarNode>(gntn);
}

G_CFLOBDDLeafNode::~G_CFLOBDDLeafNode()
{
}

void G_CFLOBDDLeafNode::IncrRef() { }
void G_CFLOBDDLeafNode::DecrRef() { }

void G_CFLOBDDLeafNode::CountNodesAndEdges(Hashset<G_CFLOBDDNodeHandle>* visitedNodes, Hashset<G_CFLOBDDReturnMapBody> *, unsigned int& nodeCount,
	unsigned int& edgeCount)
{
  if (visitedNodes->Lookup(new G_CFLOBDDNodeHandle(this)) == NULL) {
    visitedNodes->Insert(new G_CFLOBDDNodeHandle(this));
    nodeCount++;
  }
}

void G_CFLOBDDLeafNode::CountPaths(Hashset<G_CFLOBDDNodeHandle> *visitedNodes)
{
  G_CFLOBDDNodeHandle* handle = new G_CFLOBDDNodeHandle(this);
	if (visitedNodes->Lookup(handle) == NULL) {
		visitedNodes->Insert(handle);
	}
}

//********************************************************************
// G_CFLOBDDForkNode
//********************************************************************

// Constructors/Destructor -------------------------------------------

// Default constructor
G_CFLOBDDForkNode::G_CFLOBDDForkNode()
  :  G_CFLOBDDLeafNode()
{
  numExits = 2;
  numPathsToExit = new long double[2];
  numPathsToExit[0] = 1;
  numPathsToExit[1] = 1;
}

G_CFLOBDDForkNode::~G_CFLOBDDForkNode()
{
}

// print
std::ostream& G_CFLOBDDForkNode::print(std::ostream & out) const
{
//   for (unsigned int i = level; i < maxLevel; i++) {
//     out << "  ";
//   }
  out << "Fork";
  return out;
}

G_CFLOBDDNodeHandle G_CFLOBDDForkNode::Reduce(ReductionMapHandle&, unsigned int replacementNumExits, bool forceReduce)
{
	if (forceReduce){
		if (replacementNumExits == 1) {
			return G_CFLOBDDNodeHandle::G_CFLOBDDDontCareNodeHandle;
		}
		else {
			//assert(replacementNumExits == 2);
			return G_CFLOBDDNodeHandle::G_CFLOBDDForkNodeHandle;
		}
	}
	else{
		if (replacementNumExits == 1) {
      return G_CFLOBDDNodeHandle::G_CFLOBDDDontCareNodeHandle;
    }
		return G_CFLOBDDNodeHandle::G_CFLOBDDForkNodeHandle;
	}
}

unsigned int G_CFLOBDDForkNode::Hash(unsigned int modsize) const
{
  return ((unsigned int)reinterpret_cast<uintptr_t>(this) >> 2) % modsize;
}

// Overloaded !=
bool G_CFLOBDDForkNode::operator!= (const G_CFLOBDDNode & n) const
{
  return n.NodeKind() != G_CFLOBDD_FORK;
}

// Overloaded ==
bool G_CFLOBDDForkNode::operator== (const G_CFLOBDDNode & n) const
{
  return n.NodeKind() == G_CFLOBDD_FORK;
}

void G_CFLOBDDForkNode::PrintYield(std::unordered_map<int, std::vector<std::string>>& yield_strings) const
{
  yield_strings.insert({0, {"0"}});
  yield_strings.insert({1, {"1"}});
}

//********************************************************************
// G_CFLOBDDDontCareNode
//********************************************************************

// Constructors/Destructor -------------------------------------------

// Default constructor
G_CFLOBDDDontCareNode::G_CFLOBDDDontCareNode()
  :  G_CFLOBDDLeafNode()
{
  numExits = 1;
  numPathsToExit = new long double[1];
  numPathsToExit[0] = 2;
}

G_CFLOBDDDontCareNode::~G_CFLOBDDDontCareNode()
{
}

// print
std::ostream& G_CFLOBDDDontCareNode::print(std::ostream & out) const
{
//   for (unsigned int i = level; i < maxLevel; i++) {
//     out << "  ";
//   }
  out << "Don't care";
  return out;
}

G_CFLOBDDNodeHandle G_CFLOBDDDontCareNode::Reduce(ReductionMapHandle&, unsigned int, bool)
{
  return G_CFLOBDDNodeHandle::G_CFLOBDDDontCareNodeHandle;
}

unsigned int G_CFLOBDDDontCareNode::Hash(unsigned int modsize) const
{
  return ((unsigned int) reinterpret_cast<uintptr_t>(this) >> 2) % modsize;
}

// Overloaded !=
bool G_CFLOBDDDontCareNode::operator!= (const G_CFLOBDDNode & n) const
{
  return n.NodeKind() != G_CFLOBDD_DONTCARE;
}

// Overloaded ==
bool G_CFLOBDDDontCareNode::operator== (const G_CFLOBDDNode & n) const
{
  return n.NodeKind() == G_CFLOBDD_DONTCARE;
}

void G_CFLOBDDDontCareNode::PrintYield(std::unordered_map<int, std::vector<std::string>>& yield_strings) const
{
  yield_strings.insert({0, {"*"}});
}
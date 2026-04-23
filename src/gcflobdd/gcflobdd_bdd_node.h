#ifndef G_CFLOBDD_BDD_NODE_GUARD
#define G_CFLOBDD_BDD_NODE_GUARD

#include <iostream>
#include "gcflobdd_node.h"

#include "sectionT.h"
namespace G_CFL_OBDD {
  typedef SectionT<G_CFLOBDDReturnMapHandle> Section;
}

namespace G_CFL_OBDD {

//********************************************************************
// G_CFLOBDDBDDNode
//********************************************************************

class G_CFLOBDDBDDNode : public G_CFLOBDDNode {
 public:
  G_CFLOBDDBDDNode(const unsigned int numVars);   // Constructor
  ~G_CFLOBDDBDDNode();                      // Destructor
  G_CFLOBDD_NODEKIND NodeKind() const { return G_CFLOBDD_BDD; }
  unsigned int Hash(unsigned int modsize) const;
  G_CFLOBDDNodeHandle Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce = false);
  bool operator!= (const G_CFLOBDDNode & n) const;        // Overloaded !=
  bool operator== (const G_CFLOBDDNode & n) const;        // Overloaded ==
  void IncrRef();
  void DecrRef();
  
 public:
    std::ostream& print(std::ostream & out = std::cout) const;
  void PrintYield(std::unordered_map<int, std::vector<std::string>>& yield_strings) const;

  unsigned int numVars;
  Section bddSection;

  void InstallPathCounts();
  void CountNodesAndEdges(Hashset<G_CFLOBDDNodeHandle>* visitedNodes, Hashset<G_CFLOBDDReturnMapBody>* visitedEdges,
      unsigned int& nodeCount, unsigned int& edgeCount);
  void CountPaths(Hashset<G_CFLOBDDNodeHandle>* visitedNodes);

 private:
  G_CFLOBDDBDDNode(const G_CFLOBDDBDDNode &n);   // Copy constructor (hidden)
  G_CFLOBDDBDDNode& operator= (const G_CFLOBDDBDDNode &n); // Overloaded = (hidden)
};

class BDDNodeHandle {
#define BDD_NODE_HANDLE_GUARD
    public:
        BDDNodeHandle();                                        // Default constructor
        BDDNodeHandle(BDDNode *n);                          // Constructor
        BDDNodeHandle(const BDDNodeHandle &nh);              // Copy constructor
        ~BDDNodeHandle();                                       // Destructor
        unsigned int Hash(unsigned int modsize) const;
        bool operator!= (const BDDNodeHandle &nh) const;              // Overloaded !=
        bool operator== (const BDDNodeHandle &nh) const;              // Overloaded ==
        BDDNodeHandle & operator= (const BDDNodeHandle &nh); // assignment
    
        // The data member
        BDDNode* handleContents;

        public:
            std::ostream& print(std::ostream & out = std::cout) const;
        
            struct BDDNodeHandle_Hash {
            public:
                size_t operator()(const BDDNodeHandle& c) const {
                    return ((reinterpret_cast<std::uintptr_t>(c.handleContents) >> 2) % 997);
                }
            };

        static Hashset<BDDNode> *canonicalBDDNodeTable;
        void Canonicalize();
        BDDNodeHandle Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce = false);
        static void InitReduceCache();
        static void DisposeOfReduceCache();
};

std::ostream& operator<< (std::ostream & out, const BDDNodeHandle &d);

enum BDD_NODEKIND { BDD_INTERNAL, BDD_LEAF };

class BDDNode {
 public:
  BDDNode();                       // Constructor
  BDDNode(const unsigned int numVars, const unsigned int varID);   // Constructor
  virtual ~BDDNode();              // Destructor
  virtual unsigned int Hash(unsigned int modsize) const = 0;
  virtual BDD_NODEKIND NodeKind() const = 0;

  virtual bool operator!= (const BDDNode & n) const = 0;  // Overloaded !=
  virtual bool operator== (const BDDNode & n) const = 0;  // Overloaded ==
  void IncrRef();
  void DecrRef();
  const unsigned int NumVars() const { return numVars; }
  const unsigned int VarID() const { return varID; }
  const bool IsCanonical() const { return isCanonical; }
  void SetCanonical() { isCanonical = true;  }
  unsigned int GetRefCount(){ return refCount; }

 public:
    virtual std::ostream& print(std::ostream & out = std::cout) const = 0;
    virtual void PrintYield(std::unordered_map<int, std::vector<std::string>>& yield_strings) const = 0;
    virtual BDDNodeHandle Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce = false) = 0;

 protected:
  unsigned int numVars;
  unsigned int varID;    // Variable ID for the variable at this node
  unsigned int refCount;
  bool isCanonical;              // Is this BDDNode in canonicalBDDNodeTable?
};

std::ostream& operator<< (std::ostream & out, const BDDNode &d);

class BDDInternalNode : public BDDNode {
 public:
  BDDInternalNode(const unsigned int numVars, const unsigned int varID);   // Constructor
  ~BDDInternalNode();                      // Destructor
  unsigned int Hash(unsigned int modsize) const;
  BDD_NODEKIND NodeKind() const { return BDD_INTERNAL; };
  bool operator!= (const BDDNode & n) const;        // Overloaded !=
  bool operator== (const BDDNode & n) const;        // Overloaded ==

 public:
    std::ostream& print(std::ostream & out = std::cout) const;
    void PrintYield(std::unordered_map<int, std::vector<std::string>>& yield_strings) const;
    BDDNodeHandle Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce = false);
  Section thenBranch;
  Section elseBranch;

 private:
  BDDInternalNode(const BDDInternalNode &n);   // Copy constructor (hidden)
  BDDInternalNode& operator= (const BDDInternalNode &n); // Overloaded = (hidden)
};

class BDDLeafNode : public BDDNode {
 public:
  BDDLeafNode(const unsigned int value, const unsigned int varID, const unsigned int numVars);   // Constructor
  ~BDDLeafNode();                      // Destructor
  unsigned int Hash(unsigned int modsize) const;
  BDD_NODEKIND NodeKind() const { return BDD_LEAF; };
  bool operator!= (const BDDNode & n) const;        // Overloaded !=
  bool operator== (const BDDNode & n) const;        // Overloaded ==

 public:
    std::ostream& print(std::ostream & out = std::cout) const;
    void PrintYield(std::unordered_map<int, std::vector<std::string>>& yield_strings) const;
    BDDNodeHandle Reduce(ReductionMapHandle& redMapHandle, unsigned int replacementNumExits, bool forceReduce = false);

  unsigned int value;

 private:
  BDDLeafNode(const BDDLeafNode &n);   // Copy constructor (hidden)
  BDDLeafNode& operator= (const BDDLeafNode &n); // Overloaded = (hidden)
};

}

namespace G_CFL_OBDD {
  
  class BDDReduceKey {

	public:
		BDDReduceKey(BDDNodeHandle nodeHandle, ReductionMapHandle redMap); // Constructor
		unsigned int Hash(unsigned int modsize) const;
		BDDReduceKey& operator= (const BDDReduceKey& p);  // Overloaded assignment
		bool operator!= (const BDDReduceKey& p);        // Overloaded !=
		bool operator== (const BDDReduceKey& p);        // Overloaded ==
		BDDNodeHandle NodeHandle() const { return nodeHandle; }      // Access function
		ReductionMapHandle RedMapHandle() const { return redMapHandle; } // Access function
		std::ostream& print(std::ostream & out) const;

	private:
		BDDNodeHandle nodeHandle;
		ReductionMapHandle redMapHandle;
		BDDReduceKey();                                 // Default constructor (hidden)
	};

	std::ostream& operator<< (std::ostream & out, const BDDReduceKey &p);

}

#endif // G_CFLOBDD_BDD_NODE_GUARD
#include "bdd_node_ops.h"
#include "../gcflobdd/gcflobdd_node.h"

namespace G_CFL_OBDD {

    // MkNoDistinction_BDD
    G_CFLOBDDNodeHandle MkNoDistinction_BDD(unsigned int numVars) {
        G_CFLOBDDBDDNode* gcflobddNode = new G_CFLOBDDBDDNode(numVars);
        G_CFLOBDDReturnMapHandle m0;
        m0.AddToEnd(0); m0.Canonicalize();

        BDDLeafNode* node = new BDDLeafNode(
            0, // Value
            -1, // varID
            0 // numVars
        );
        auto bddNodeHandle = BDDNodeHandle(node);

        BDDNodeHandle nodeHandle = bddNodeHandle;
        gcflobddNode->bddSection = Section(nodeHandle, m0);
        gcflobddNode->numExits = 1;
        return G_CFLOBDDNodeHandle(gcflobddNode);
    }

    // MkDistinction_BDD
    G_CFLOBDDNodeHandle MkDistinction_BDD(unsigned int numVars, unsigned int i) {
        G_CFLOBDDBDDNode* gcflobddNode = new G_CFLOBDDBDDNode(numVars);

        BDDInternalNode* internalNode = new BDDInternalNode(numVars - i, i);
        
        G_CFLOBDDReturnMapHandle m0; m0.AddToEnd(0); m0.Canonicalize();
        G_CFLOBDDReturnMapHandle m1; m1.AddToEnd(1); m1.Canonicalize();
        auto thenMapHandle = BDDNodeHandle(new BDDLeafNode(
            0, // Value
            -1, // varID
            0 // numVars
        ));
        auto elseMapHandle = BDDNodeHandle(new BDDLeafNode(
            1, // Value
            -1, // varID
            0 // numVars
        ));
        internalNode->thenBranch = Section(thenMapHandle, m0);
        internalNode->elseBranch = Section(elseMapHandle, m1);

        BDDNodeHandle bddNodeHandle(internalNode);

        G_CFLOBDDReturnMapHandle m01; m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
        
        Section section = Section(bddNodeHandle, m01);
        gcflobddNode->bddSection = section;
        gcflobddNode->numExits = 2;
        return G_CFLOBDDNodeHandle(gcflobddNode);
    }

    // MkParity_BDD
    G_CFLOBDDNodeHandle MkParity_BDD(unsigned int numVars) {
        abort(); // Not implemented yet
        return G_CFLOBDDNodeHandle();
    }
}
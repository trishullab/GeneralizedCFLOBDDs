#include "cross_product_bdd.h"
#include "cross_product_cache_utils.h"

namespace G_CFL_OBDD {

static Hashtable<PairProductKey<BDDNodeHandle>, PairProductMemo<BDDNodeHandle>> *bddPairProductCache = NULL;

BDDNodeHandle PairProduct(BDDNodeHandle n1,
                          BDDNodeHandle n2,
                          PairProductMapHandle &pairProductMapHandle,
                          std::unordered_map<intpair, unsigned int, intpair::intpair_hash, intpair::intpair_equal> &pairProductMap
                        )
{
    PairProductMemo<BDDNodeHandle> cachedPairProductMemo;
    auto key1 = PairProductKey<BDDNodeHandle>(n1, n2);
    auto key2 = PairProductKey<BDDNodeHandle>(n2, n1);
    bool isCached = bddPairProductCache->Fetch(key1, cachedPairProductMemo);
    // if (isCached) {
    //     // pairProductMapHandle.Extend(cachedPairProductMemo.pairProductMapHandle);
    //     pairProductMapHandle = cachedPairProductMemo.pairProductMapHandle;
    //     for (unsigned int i = 0; i < pairProductMapHandle.Size(); i++) {
    //         if (pairProductMap.find(pairProductMapHandle[i]) == pairProductMap.end()) {
    //             auto index = pairProductMap.size();
    //             pairProductMap.insert({pairProductMapHandle[i], index});
    //         }
    //     }
    //     return cachedPairProductMemo.nodeHandle;
    // }
    // else if (bddPairProductCache->Fetch(key2, cachedPairProductMemo)) {
    //     pairProductMapHandle = cachedPairProductMemo.pairProductMapHandle.Flip();
    //     for (unsigned int i = 0; i < pairProductMapHandle.Size(); i++) {
    //         if (pairProductMap.find(pairProductMapHandle[i]) == pairProductMap.end()) {
    //             auto index = pairProductMap.size();
    //             pairProductMap.insert({pairProductMapHandle[i], index});
    //         }
    //     }
    //     return cachedPairProductMemo.nodeHandle;
    // }
    // else {
        BDDNodeHandle answer;
        if (n1.handleContents->NodeKind() == BDD_INTERNAL && n2.handleContents->NodeKind() == BDD_INTERNAL) {
            answer  = PairProductBDDInternal(static_cast<BDDInternalNode*>(n1.handleContents),
                                             static_cast<BDDInternalNode*>(n2.handleContents),
                                             pairProductMapHandle,
                                             pairProductMap);
        }
        else if (n1.handleContents->NodeKind() == BDD_LEAF && n2.handleContents->NodeKind() == BDD_LEAF) {
            // Both are leaf nodes
            BDDLeafNode* leaf1 = static_cast<BDDLeafNode*>(n1.handleContents);
            BDDLeafNode* leaf2 = static_cast<BDDLeafNode*>(n2.handleContents);
            auto pair = intpair(leaf1->value, leaf2->value);
            if (pairProductMap.find(pair) == pairProductMap.end()) {
                auto index = pairProductMap.size();
                answer = BDDNodeHandle(new BDDLeafNode(
                    index, // Value
                    -1, // varID
                    0 // numVars
                ));
                pairProductMapHandle.AddToEnd(intpair(leaf1->value, leaf2->value));
                pairProductMapHandle.Canonicalize();
                pairProductMap.insert({pair, index});
            }
            else
                answer = BDDNodeHandle(new BDDLeafNode(
                    pairProductMap[pair], // Value
                    -1, // varID
                    0 // numVars
                ));
            
        }
        else if (n1.handleContents->NodeKind() == BDD_LEAF) {
            BDDInternalNode* internal2 = static_cast<BDDInternalNode*>(n2.handleContents);
            BDDInternalNode* resultNode = new BDDInternalNode(internal2->NumVars(), internal2->VarID());
            PairProductMapHandle thenMapHandle;
            PairProductMapHandle elseMapHandle;
            BDDNodeHandle thenBranch = PairProduct(n1,
                                                *(internal2->thenBranch.entryPointHandle),
                                                thenMapHandle,  
                                                pairProductMap);
            BDDNodeHandle elseBranch = PairProduct(n1,
                                                *(internal2->elseBranch.entryPointHandle),
                                                elseMapHandle,
                                                pairProductMap);
            
            // for (int i = 0; i < thenMapHandle.Size(); i++) {
            //     std::cout << "thenMapHandle[" << i << "] = " << thenMapHandle[i] << std::endl;
            // }

            // for (int i = 0; i < elseMapHandle.Size(); i++) {
            //     std::cout << "elseMapHandle[" << i << "] = " << elseMapHandle[i] << std::endl;
            // }

            pairProductMapHandle.Extend(thenMapHandle);
            pairProductMapHandle.Extend(elseMapHandle);
            pairProductMapHandle.Canonicalize();
            if (thenBranch == elseBranch) {
                // Reduction rule: skip nodes whose then and else branches are the same
                answer = thenBranch;
            }
            else {
                G_CFLOBDDReturnMapHandle mThen, mElse;
                for (unsigned int i = 0; i < thenMapHandle.Size(); i++) {
                    mThen.AddToEnd(pairProductMap[thenMapHandle[i]]);
                }
                mThen.Canonicalize();
                for (unsigned int i = 0; i < elseMapHandle.Size(); i++) {
                    mElse.AddToEnd(pairProductMap[elseMapHandle[i]]);
                }
                mElse.Canonicalize();
                // for (int i = 0; i < mThen.Size(); i++) {
                //     std::cout << "mThen[" << i << "] = " << mThen[i] << std::endl;
                // }
                // for (int i = 0; i < mElse.Size(); i++) {
                //     std::cout << "mElse[" << i << "] = " << mElse[i] << std::endl;
                // }
                resultNode->thenBranch = Section(thenBranch, mThen);
                resultNode->elseBranch = Section(elseBranch, mElse);
                answer = BDDNodeHandle(resultNode);
            }
        }
        else { // n2 is leaf
            BDDInternalNode* internal1 = static_cast<BDDInternalNode*>(n1.handleContents);
            BDDInternalNode* resultNode = new BDDInternalNode(internal1->NumVars(), internal1->VarID());
            PairProductMapHandle thenMapHandle;
            PairProductMapHandle elseMapHandle;
            BDDNodeHandle thenBranch = PairProduct(*(internal1->thenBranch.entryPointHandle),
                                                n2,
                                                thenMapHandle,  
                                                pairProductMap);
            BDDNodeHandle elseBranch = PairProduct(*(internal1->elseBranch.entryPointHandle),
                                                n2,
                                                elseMapHandle,
                                                pairProductMap);
            
            pairProductMapHandle.Extend(thenMapHandle);
            pairProductMapHandle.Extend(elseMapHandle);
            pairProductMapHandle.Canonicalize();
            if (thenBranch == elseBranch) {
                // Reduction rule: skip nodes whose then and else branches are the same
                answer = thenBranch;
            }
            else {
                G_CFLOBDDReturnMapHandle mThen, mElse;
                for (unsigned int i = 0; i < thenMapHandle.Size(); i++) {
                    mThen.AddToEnd(pairProductMap[thenMapHandle[i]]);
                }
                mThen.Canonicalize();
                for (unsigned int i = 0; i < elseMapHandle.Size(); i++) {
                    mElse.AddToEnd(pairProductMap[elseMapHandle[i]]);
                }
                mElse.Canonicalize();
                resultNode->thenBranch = Section(thenBranch, mThen);
                resultNode->elseBranch = Section(elseBranch, mElse);
                answer = BDDNodeHandle(resultNode);
            }
        }
        PairProductMemo<BDDNodeHandle> memo;
        memo.nodeHandle = answer;
        memo.pairProductMapHandle = pairProductMapHandle;
        bddPairProductCache->Insert(key1, memo);
        return answer;
    // }
}

BDDNodeHandle PairProductBDDInternal(BDDInternalNode *n1,
                          BDDInternalNode *n2,
                          PairProductMapHandle &pairProductMapHandle,
                          std::unordered_map<intpair, unsigned int, intpair::intpair_hash, intpair::intpair_equal> &pairProductMap
                         )
{
    if (n1->VarID() == n2->VarID()) {
        PairProductMapHandle thenMapHandle;
        PairProductMapHandle elseMapHandle;
        BDDNodeHandle thenBranch = PairProduct(*(n1->thenBranch.entryPointHandle),
                                               *(n2->thenBranch.entryPointHandle),
                                               thenMapHandle,
                                               pairProductMap);
        BDDNodeHandle elseBranch = PairProduct(*(n1->elseBranch.entryPointHandle),
                                               *(n2->elseBranch.entryPointHandle),
                                               elseMapHandle,
                                               pairProductMap);
        pairProductMapHandle.Extend(thenMapHandle);
        pairProductMapHandle.Extend(elseMapHandle);
        pairProductMapHandle.Canonicalize();
        if (thenBranch == elseBranch) {
            // Reduction rule: skip nodes whose then and else branches are the same
            return thenBranch;
        }
        BDDInternalNode* resultNode = new BDDInternalNode(n1->NumVars(), n1->VarID());
        G_CFLOBDDReturnMapHandle mThen, mElse;
        for (unsigned int i = 0; i < thenMapHandle.Size(); i++) {
            mThen.AddToEnd(i);
        }
        mThen.Canonicalize();
        for (unsigned int i = 0; i < elseMapHandle.Size(); i++) {
            int index = pairProductMapHandle.Lookup(elseMapHandle[i]);
            mElse.AddToEnd(index);
        }
        mElse.Canonicalize();
        resultNode->thenBranch = Section(thenBranch, mThen);
        resultNode->elseBranch = Section(elseBranch, mElse);
        BDDNodeHandle resultHandle(resultNode);
        return resultHandle;
    }
    else if (n1->VarID() < n2->VarID()) {
        PairProductMapHandle thenMapHandle;
        PairProductMapHandle elseMapHandle;
        BDDNodeHandle thenBranch = PairProduct(*(n1->thenBranch.entryPointHandle),
                                              BDDNodeHandle(n2),
                                              thenMapHandle,
                                              pairProductMap);
        BDDNodeHandle elseBranch = PairProduct(*(n1->elseBranch.entryPointHandle),
                                              BDDNodeHandle(n2),
                                              elseMapHandle,
                                              pairProductMap);

        pairProductMapHandle.Extend(thenMapHandle);
        pairProductMapHandle.Extend(elseMapHandle);
        pairProductMapHandle.Canonicalize();
        if (thenBranch == elseBranch) {
            // Reduction rule: skip nodes whose then and else branches are the same
            return thenBranch;
        }
        BDDInternalNode* resultNode = new BDDInternalNode(n1->NumVars(), n1->VarID());
        G_CFLOBDDReturnMapHandle mThen, mElse;
        for (unsigned int i = 0; i < thenMapHandle.Size(); i++) {
            mThen.AddToEnd(pairProductMap[thenMapHandle[i]]);
        }
        mThen.Canonicalize();
        for (unsigned int i = 0; i < elseMapHandle.Size(); i++) {
            mElse.AddToEnd(pairProductMap[elseMapHandle[i]]);
        }
        mElse.Canonicalize();
        resultNode->thenBranch = Section(thenBranch, mThen);
        resultNode->elseBranch = Section(elseBranch, mElse);
        BDDNodeHandle resultHandle(resultNode);
        return resultHandle;
    }
    else { // n1->VarID > n2->VarID
        PairProductMapHandle thenMapHandle;
        PairProductMapHandle elseMapHandle;
        BDDNodeHandle thenBranch = PairProduct(BDDNodeHandle(n1),
                                              *(n2->thenBranch.entryPointHandle),
                                              thenMapHandle,
                                              pairProductMap);
        BDDNodeHandle elseBranch = PairProduct(BDDNodeHandle(n1),
                                              *(n2->elseBranch.entryPointHandle),
                                              elseMapHandle,
                                              pairProductMap);
        pairProductMapHandle.Extend(thenMapHandle);
        pairProductMapHandle.Extend(elseMapHandle);
        pairProductMapHandle.Canonicalize();
        if (thenBranch == elseBranch) {
            // Reduction rule: skip nodes whose then and else branches are the same
            return thenBranch;
        }
        BDDInternalNode* resultNode = new BDDInternalNode(n2->NumVars(), n2->VarID());
        G_CFLOBDDReturnMapHandle mThen, mElse;
        for (unsigned int i = 0; i < thenMapHandle.Size(); i++) {
            mThen.AddToEnd(pairProductMap[thenMapHandle[i]]);
        }
        mThen.Canonicalize();
        for (unsigned int i = 0; i < elseMapHandle.Size(); i++) {
            mElse.AddToEnd(pairProductMap[elseMapHandle[i]]);
        }
        mElse.Canonicalize(); 
        resultNode->thenBranch = Section(thenBranch, mThen);
        resultNode->elseBranch = Section(elseBranch, mElse);
        BDDNodeHandle resultHandle(resultNode);
        return resultHandle;
    }
}

G_CFLOBDDNodeHandle PairProduct(G_CFLOBDDBDDNode* n1,
                                G_CFLOBDDBDDNode* n2,
                                PairProductMapHandle &pairProductMapHandle
                               )
{
    BDDNodeHandle n1Handle = *(n1->bddSection.entryPointHandle);
    BDDNodeHandle n2Handle = *(n2->bddSection.entryPointHandle);
    std::unordered_map<intpair, unsigned int, intpair::intpair_hash, intpair::intpair_equal> pairProductMap;
    auto result = PairProduct(n1Handle,
                              n2Handle,
                              pairProductMapHandle,
                              pairProductMap);

    G_CFLOBDDBDDNode* resultNode = new G_CFLOBDDBDDNode(n1->numVars);
    G_CFLOBDDReturnMapHandle mI;
    for (unsigned int i = 0; i < pairProductMapHandle.Size(); i++) {
        mI.AddToEnd(i);
    }
    mI.Canonicalize();
    // for (const auto& entry : pairProductMap) {
    //     BDDLeafNode* leafNode1 = static_cast<BDDLeafNode*>(entry.first.NodeHandle1().handleContents);
    //     BDDLeafNode* leafNode2 = static_cast<BDDLeafNode*>(entry.first.NodeHandle2().handleContents);
    //     pairProductMapHandle.AddToEnd(intpair(leafNode1->value,
    //                                           leafNode2->value));
    // }
    pairProductMapHandle.Canonicalize();
    resultNode->bddSection = Section(result, mI);
    G_CFLOBDDNodeHandle resultHandle(resultNode);
    return resultHandle;
}

void InitBDDPairProductCache()
{
    if (bddPairProductCache == NULL) {
        bddPairProductCache = new Hashtable<PairProductKey<BDDNodeHandle>, PairProductMemo<BDDNodeHandle>>(HASHSET_NUM_BUCKETS);
    }
}

void DisposeOfBDDPairProductCache()
{
    if (bddPairProductCache != NULL) {
        delete bddPairProductCache;
        bddPairProductCache = NULL;
    }
}
} // namespace G_CFL_OBDD
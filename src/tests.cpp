#include "tests.h"
#include <iostream>
#include "grammar/grammar.h"
#include "gcflobdd/gcflobdd_t.h"
#include "ops/gcflobdd_int.h"
#include "visualization/visualize.h"
#include "hardware_benchmarks/hardware_tests.h"
#include "ops/cross_product.h"
#include "ops/cross_product_bdd.h"
#include <chrono>
using namespace G_CFL_OBDD;
using namespace std;
using namespace std::chrono;

std::shared_ptr<Grammar> generateSampleGrammar() {
    std::vector<std::string> productions = {
        "S 3 -> S 0 S 2",
        "S 2 -> S 0 S 1",
        "S 1 -> S 0 S 0",
        "S 0 -> a"
    };

    Grammar grammar;
    grammar.constructGrammar(productions, "S 3");
    grammar.InstallNumVars();
    grammar.updateLevel();
    return std::make_shared<Grammar>(grammar);
}

std::shared_ptr<Grammar> generateBalancedSampleGrammar() {
    std::vector<std::string> productions = {
        "S 3 -> S 2 S 2",
        "S 2 -> S 1 S 1",
        "S 1 -> S 0 S 0",
        "S 0 -> a"
    };

    Grammar grammar;
    grammar.constructGrammar(productions, "S 3");
    grammar.InstallNumVars();
    grammar.updateLevel();
    return std::make_shared<Grammar>(grammar);
}

void Tests::testMkTrueAndFalse() {
    std::shared_ptr<Grammar> grammar = generateSampleGrammar();
    G_CFLOBDD trueFunc = MkTrue(3, grammar);
    G_CFLOBDD falseFunc = MkFalse(3, grammar);

    cout << "True Function:" << endl;
    PrintCFLOBDD(trueFunc);
    cout << "False Function:" << endl;
    PrintCFLOBDD(falseFunc);

    std::cout << "trueFunc == falseFunc nodeHandle: " 
              << (*(trueFunc.root->rootConnection.entryPointHandle) == *(falseFunc.root->rootConnection.entryPointHandle) ? "True" : "False") << std::endl;
}

void Tests::testMkProjection() {
    // std::shared_ptr<Grammar> grammar = generateSampleGrammar();
    // G_CFLOBDD projFunc = MkProjection(2, 3, grammar);

    // cout << "Projection Function (x_2):" << endl;
    // PrintCFLOBDD(projFunc);
    // unsigned int nodeCount = 0, edgeCount = 0;
    // projFunc.CountNodesAndEdges(nodeCount, edgeCount);
    // std::cout << "Node Count: " << nodeCount << ", Edge Count: " << edgeCount << std::endl;

    std::vector<std::string> productions = {
        // "S 21 -> S 20 S 20", // 2097152
        // "S 20 -> S 19 S 19", // 1048576
        // "S 19 -> S 18 S 18", // 524288
        // "S 18 -> S 17 S 17", // 262144
        // "S 17 -> S 16 S 16", // 131072
        "S 16 -> S 15 S 15", // 65536
        "S 15 -> S 14 S 14", // 32768
        "S 14 -> S 13 S 13", // 16384
        "S 13 -> S 12 S 12", // 8192
        "S 12 -> S 11 S 11", // 4096
        "S 11 -> S 10 S 10", // 2048
        "S 10 -> S 9 S 9", // 1024
        "S 9 -> S 8 S 8", // 512
        "S 8 -> S 7 S 7", // 256
        "S 7 -> S 6 S 6", // 128
        "S 6 -> S 5 S 5", // 64
        "S 5 -> S 4 S 4", // 32
        "S 4 -> S 3 S 3", // 16
        "S 3 -> S 2 S 2", // 8
        "S 2 -> S 1 S 1", // 4
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 16");
    grammar->InstallNumVars();
    grammar->updateLevel();

    unsigned int numVars = std::pow(2, 16);
    unsigned int level = grammar->root->level;
    auto start = high_resolution_clock::now();
    for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
        G_CFLOBDD projFunc = MkProjection(i, level, grammar);
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start);
    std::cout << "Duration for creating " << numVars << " projections: " << duration.count() << " seconds" << std::endl;
}

void Tests::testParity() {
    std::shared_ptr<Grammar> grammar = generateSampleGrammar();
    G_CFLOBDD parityFunc = MkParity(3, grammar);

    cout << "Parity Function:" << endl;
    PrintCFLOBDD(parityFunc);
}

void Tests::testDataVisualization() {
    std::shared_ptr<Grammar> grammar = generateSampleGrammar();
    G_CFLOBDD parityFunc = MkParity(3, grammar);

    cout << "Visualizing Parity Function:" << endl;
    Visualization::visualizeGCFLOBDD(parityFunc);
}

void Tests::testMkNot() {
    std::shared_ptr<Grammar> grammar = generateSampleGrammar();
    G_CFLOBDD trueFunc = MkTrue(3, grammar);
    G_CFLOBDD notTrueFunc = MkNot(trueFunc);
    G_CFLOBDD falseFunc = MkFalse(3, grammar);

    cout << "Not True Function:" << endl;
    PrintCFLOBDD(notTrueFunc);

    cout << "False Function:" << endl;
    PrintCFLOBDD(falseFunc);

    std::cout << "notTrueFunc == falseFunc: " 
              << (notTrueFunc == falseFunc ? "True" : "False") << std::endl;
}

void Tests::testCrossProduct() {
    std::shared_ptr<Grammar> grammar = generateSampleGrammar();
    G_CFLOBDD x0 = MkProjection(0, 3, grammar);
    G_CFLOBDD x1 = MkProjection(1, 3, grammar);
    G_CFLOBDD x2 = MkProjection(2, 3, grammar);
    G_CFLOBDD x0_and_x1 = MkAnd(x0, x1);
    G_CFLOBDD x0_and_x1_and_x2 = MkAnd(x0_and_x1, x2);
    G_CFLOBDD x1_and_x2 = MkAnd(x1, x2);
    G_CFLOBDD x0_and_x1_x2_p = MkAnd(x0, x1_and_x2);
    std::cout << "x0_and_x1_x2_p == x1_and_x0_and_x2: " 
              << (x0_and_x1_x2_p == x0_and_x1_and_x2 ? "True" : "False") << std::endl;

    unsigned int nodeCount = 0, edgeCount = 0;
    x0_and_x1_and_x2.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "Node Count: " << nodeCount << ", Edge Count: " << edgeCount << std::endl;

    nodeCount = 0; edgeCount = 0;
    x0_and_x1_x2_p.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "Node Count: " << nodeCount << ", Edge Count: " << edgeCount << std::endl;

}

void Tests::testNand() {
    std::shared_ptr<Grammar> grammar = generateBalancedSampleGrammar();
    G_CFLOBDD x0 = MkProjection(0, 3, grammar);
    G_CFLOBDD x1 = MkProjection(2, 3, grammar);
    G_CFLOBDD nandFunc = MkNand(x0, x1);
    G_CFLOBDD x2 = MkProjection(1, 3, grammar);
    G_CFLOBDD andFunc = MkAnd(x0, x1);
    G_CFLOBDD notAndFunc = MkNot(andFunc);
    notAndFunc.print(std::cout);
}

void Tests::testRandomFunction() {
	std::cout << "Random start..." << std::endl;

    // std::vector<std::string> productions = {
    //     // "S 15 -> S 14 S 14 S 14", // 14348907
    //     // "S 14 -> S 13 S 13 S 13", // 4782969
    //     // "S 13 -> S 12 S 12 S 12", // 1594323
    //     // "S 12 -> S 11 S 11 S 11", // 531441
    //     "S 11 -> S 10 S 10 S 10", // 59049
    //     "S 10 -> S 9 S 9 S 9", // 59049
    //     "S 9 -> S 8 S 8 S 8", // 19683
    //     "S 8 -> S 7 S 7 S 7", // 6561
    //     "S 7 -> S 6 S 6 S 6", // 2187
    //     "S 6 -> S 5 S 5 S 5", // 729
    //     "S 5 -> S 4 S 4 S 4", // 243
    //     "S 4 -> S 3 S 3 S 3", // 81
    //     "S 3 -> S 2 S 2 S 2", // 27
    //     "S 2 -> S 1 S 1 S 1", // 9
    //     "S 1 -> S 0 S 0 S 0", // 3
    //     "S 0 -> a"
    // };

    std::vector<std::string> productions = {
        // "S 21 -> S 20 S 20", // 2097152
        // "S 20 -> S 19 S 19", // 1048576
        // "S 19 -> S 18 S 18", // 524288
        "S 18 -> S 17 S 17", // 262144
        "S 17 -> S 16 S 16", // 131072
        "S 16 -> S 15 S 15", // 65536
        "S 15 -> S 14 S 14", // 32768
        "S 14 -> S 13 S 13", // 16384
        "S 13 -> S 12 S 12", // 8192
        "S 12 -> S 11 S 11", // 4096
        "S 11 -> S 10 S 10", // 2048
        "S 10 -> S 9 S 9", // 1024
        "S 9 -> S 8 S 8", // 512
        "S 8 -> S 7 S 7", // 256
        "S 7 -> S 6 S 6", // 128
        "S 6 -> S 5 S 5", // 64
        "S 5 -> S 4 S 4", // 32
        "S 4 -> S 3 S 3", // 16
        "S 3 -> S 2 S 2", // 8
        "S 2 -> S 1 S 1", // 4
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 18");
    grammar->InstallNumVars();
    grammar->updateLevel();

	auto start = high_resolution_clock::now();
	unsigned int numVars = std::pow(3, 11); // 4782969
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	}

	G_CFLOBDD F = MkTrue(level, grammar);
	for (unsigned int i = 0; i < numVars / 3; i++) {
        if (i % 10000 == 0) {
            std::cout << "Processing variable set " << i << " / " << (numVars / 3) << std::endl;
        }
		unsigned int a = 3 * i;
		unsigned int b = 3 * i + 1;;
		unsigned int c = 3 * i + 2;
		G_CFLOBDD A = vars[a];
		G_CFLOBDD B = vars[b];
		G_CFLOBDD C = vars[c];
		G_CFLOBDD A_and_B = MkAnd(A, B);
		G_CFLOBDD Not_A = MkNot(A);
		G_CFLOBDD Not_A_and_C = MkAnd(Not_A, C);
		G_CFLOBDD A_and_B_or_Not_A_and_C = MkOr(A_and_B, Not_A_and_C);
		F = MkAnd(F, A_and_B_or_Not_A_and_C);
	}

	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start);
	std::cout << "Duration: " << duration.count() << " ms" << std::endl;

	unsigned int nodeCount = 0, edgeCount = 0;

	F.CountNodesAndEdges(nodeCount, edgeCount);
	std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl;

}

void Tests::testSynFun2() {
    // std::vector<std::string> productions = {
    //     // "S 22 -> S 1 S 21",
    //     // "S 21 -> S 20 S 20",
    //     // "S 20 -> S 19 S 19",
    //     // "S 19 -> S 18 S 18",
    //     // "S 18 -> S 17 S 17",
    //     "S 18 -> S 1 S 17",
    //     "S 17 -> S 16 S 16",
    //     "S 16 -> S 15 S 15",
    //     "S 15 -> S 14 S 14",
    //     "S 14 -> S 13 S 13",
    //     "S 13 -> S 12 S 12",
    //     "S 12 -> S 11 S 11",
    //     "S 11 -> S 10 S 10",
    //     "S 10 -> S 9 S 9",
    //     "S 9 -> S 8 S 8",
    //     "S 8 -> S 7 S 7",
    //     "S 7 -> S 6 S 6",
    //     "S 6 -> S 5 S 5",
    //     "S 5 -> S 4 S 4",
    //     "S 4 -> S 3 S 3",
    //     "S 3 -> S 2 S 2",
    //     "S 2 -> S 1 S 1",
    //     "S 1 -> S 0 S 0", // 4
    //     "S 0 -> a"
    // };

    std::vector<std::string> productions = {
        // "S 21 -> S 20 S 20", // 2097152
        // "S 20 -> S 19 S 19", // 1048576
        "S 19 -> S 18 S 18", // 524288
        "S 18 -> S 17 S 17", // 262144
        "S 17 -> S 16 S 16", // 131072
        "S 16 -> S 15 S 15", // 65536
        "S 15 -> S 14 S 14", // 32768
        "S 14 -> S 13 S 13", // 16384
        "S 13 -> S 12 S 12", // 8192
        "S 12 -> S 11 S 11", // 4096
        "S 11 -> S 10 S 10", // 2048
        "S 10 -> S 9 S 9", // 1024
        "S 9 -> S 8 S 8", // 512
        "S 8 -> S 7 S 7", // 256
        "S 7 -> S 6 S 6", // 128
        "S 6 -> S 5 S 5", // 64
        "S 5 -> S 4 S 4", // 32
        "S 4 -> S 3 S 3", // 16
        "S 3 -> S 2 S 2", // 8
        "S 2 -> S 1 S 1", // 4
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 19");
    grammar->InstallNumVars();
    grammar->updateLevel();

	auto start = high_resolution_clock::now();
	unsigned int numVars = std::pow(2, 18) + 2; // 18
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	}

	G_CFLOBDD F = MkTrue(level, grammar);
    G_CFLOBDD F1 = MkTrue(level, grammar);
    G_CFLOBDD F2 = MkFalse(level, grammar);
    for (unsigned int i = 2; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Processing variable pair " << i << " / " << (numVars) << std::endl;
        }
        F1 = MkAnd(F1, vars[i]);
        F2 = MkOr(F2, vars[i]);
    }

    F = MkExclusiveOr(vars[0], vars[1]);
    F = MkOr(MkAnd(F, F1), MkAnd(MkNot(F), F2));

	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start);
	std::cout << "Duration: " << duration.count() << " ms" << std::endl;

	unsigned int nodeCount = 0, edgeCount = 0;

	F.CountNodesAndEdges(nodeCount, edgeCount);
	std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl; 
}

void Tests::testSynFun3() {
    std::vector<std::string> productions = {
        // "S 22 -> S 1 S 21",
        // "S 21 -> S 20 S 20",
        // "S 20 -> S 19 S 19",
        // "S 19 -> S 18 S 18",
        "S 19 -> S 1 S 18 S 1 S 18",
        "S 18 -> S 17 S 17",
        "S 17 -> S 16 S 16",
        "S 16 -> S 15 S 15",
        "S 15 -> S 14 S 14",
        "S 14 -> S 13 S 13",
        "S 13 -> S 12 S 12",
        "S 12 -> S 11 S 11",
        "S 11 -> S 10 S 10",
        "S 10 -> S 9 S 9",
        "S 9 -> S 8 S 8",
        "S 8 -> S 7 S 7",
        "S 7 -> S 6 S 6",
        "S 6 -> S 5 S 5",
        "S 5 -> S 4 S 4",
        "S 4 -> S 3 S 3",
        "S 3 -> S 2 S 2",
        "S 2 -> S 1 S 1",
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 19");
    grammar->InstallNumVars();
    grammar->updateLevel();

	auto start = high_resolution_clock::now();
	unsigned int numVars = std::pow(2, 19) + 4; // 18
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	}

	auto F1 = 1 * MkAnd(vars[0], vars[1]) + (-1) * MkNot(MkAnd(vars[0], vars[1]));
    auto I1 = MkTrue(level, grammar);

    for (unsigned int i = 2; i < (numVars - 4)/2; i+=2) {
        if (i % 10000 == 0) {
            std::cout << "Processing variable " << i << " / " << (numVars) << std::endl;
        }
        I1 = MkAnd(MkExclusiveOr(vars[i], vars[i + 1]), I1);
    }

    int v = 2 + (numVars - 4)/2;
    auto F2 = 1 * MkAnd(vars[v], vars[v + 1]) + (-1) * MkNot(MkAnd(vars[v], vars[v + 1]));

    auto I2 = MkTrue(level, grammar);
    for (unsigned int i = v + 2; i < numVars; i+=2) {
        if (i % 10000 == 0) {
            std::cout << "Processing variable " << i << " / " << (numVars) << std::endl;
        }
        I2 = MkAnd(MkExclusiveOr(vars[i], vars[i + 1]), I2);
    }

    G_CFLOBDD F = MkAnd(MkAnd(F1, I1), MkAnd(F2, I2));
    

	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start);
	std::cout << "Duration: " << duration.count() << " ms" << std::endl;

	unsigned int nodeCount = 0, edgeCount = 0;

	F.CountNodesAndEdges(nodeCount, edgeCount);
	std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl; 
}

void Tests::testSynFun4() {

    std::vector<std::string> productions = {
        // "S 18 -> S 17 S 17 S 17", // 10077696
        // "S 17 -> S 16 S 16", // 3359232
        // "S 16 -> S 15 S 15 S 15", // 1679616
        // "S 15 -> S 14 S 14", // 559872
        "S 14 -> S 13 S 13 S 13", // 279936
        "S 13 -> S 12 S 12", // 93312
        "S 12 -> S 11 S 11 S 11", // 46656
        "S 11 -> S 10 S 10", // 15552
        "S 10 -> S 9 S 9 S 9", // 7776
        "S 9 -> S 8 S 8", // 2592
        "S 8 -> S 7 S 7 S 7", // 1296
        "S 7 -> S 6 S 6", // 432
        "S 6 -> S 5 S 5 S 5", // 216
        "S 5 -> S 4 S 4", // 72
        "S 4 -> S 3 S 3 S 3", // 36
        "S 3 -> S 2 S 2", // 12
        "S 2 -> S 1 S 1 S 1", // 6
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 14");
    grammar->InstallNumVars();
    grammar->updateLevel();

	auto start = high_resolution_clock::now();
	unsigned int numVars = 279936; // 6765
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	} 


    std::vector<G_CFLOBDD> groupFuncs = vars;
    for (int i = 0; i < level; i++) {
        std::cout << "Starting iteration " << i << std::endl;
        std::vector<G_CFLOBDD> newGroupFuncs;
        if (i % 2 == 0) {
            for (size_t j = 0; j < groupFuncs.size(); j += 2) {
                G_CFLOBDD f1 = groupFuncs[j];
                G_CFLOBDD f2 = groupFuncs[j + 1];
                G_CFLOBDD combined = MkExclusiveOr(f1, f2);
                newGroupFuncs.push_back(combined);
            }
        } else {
            for (size_t j = 0; j < groupFuncs.size(); j += 3) {
                G_CFLOBDD f1 = groupFuncs[j];
                G_CFLOBDD f2 = groupFuncs[j + 1];
                G_CFLOBDD f3 = groupFuncs[j + 2];
                G_CFLOBDD A_and_B = MkAnd(f1, f2);
                G_CFLOBDD Not_A = MkNot(f1);
                G_CFLOBDD Not_A_and_C = MkAnd(Not_A, f3);
                G_CFLOBDD A_and_B_or_Not_A_and_C = MkOr(A_and_B, Not_A_and_C);
                newGroupFuncs.push_back(A_and_B_or_Not_A_and_C);
            }
        }

        groupFuncs = newGroupFuncs;
    }

    G_CFLOBDD F = groupFuncs[0];

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;
    unsigned int nodeCount = 0, edgeCount = 0;
    F.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl;
}

G_CFLOBDD computeFibFunction(unsigned int n, const std::vector<G_CFLOBDD>& vars, unsigned int level, std::shared_ptr<Grammar> grammar, int startIndex, std::vector<int>& fibIndices) {

    // std::cout << "Computing Fibonacci function for n = " << n << " at index " << startIndex << std::endl;
    if (fibIndices[n] == 0) {
        return MkNot(vars[0]);
    } else if (fibIndices[n] == 1) {
        return vars[startIndex];
    } else if (fibIndices[n] == 2) {
        return MkExclusiveOr(vars[startIndex], vars[startIndex + 1]);
    } else if (fibIndices[n] == 3) {
        return MkAnd(vars[startIndex], MkExclusiveOr(vars[startIndex + 1], vars[startIndex + 2]));
    } else {
        auto F1 = computeFibFunction(n - 2, vars, level, grammar, startIndex, fibIndices);
        auto F2 = computeFibFunction(n - 1, vars, level, grammar, startIndex + fibIndices[n - 2], fibIndices);
        return MkAnd(F1, F2);
    }
}

void Tests::testSynFun5() {
    std::vector<std::string> productions = {
        "S 27 -> S 25 S 26", // 514229
        "S 26 -> S 24 S 25", // 317811
        "S 25 -> S 23 S 24", // 196418
        "S 24 -> S 22 S 23", // 121393
        "S 23 -> S 21 S 22", // 75025
        "S 22 -> S 20 S 21", // 46368
        "S 21 -> S 19 S 20", // 28657
        "S 20 -> S 18 S 19", // 17711
        "S 19 -> S 17 S 18", // 10946
        "S 18 -> S 16 S 17", // 6765
        "S 17 -> S 15 S 16", // 4181
        "S 16 -> S 14 S 15", // 2584
        "S 15 -> S 13 S 14", // 1597
        "S 14 -> S 12 S 13", // 987
        "S 13 -> S 11 S 12", // 610
        "S 12 -> S 10 S 11", // 377
        "S 11 -> S 9 S 10", // 233
        "S 10 -> S 8 S 9", // 144
        "S 9 -> S 7 S 8", // 89
        "S 8 -> S 6 S 7", // 55
        "S 7 -> S 5 S 6", // 34
        "S 6 -> S 4 S 5", // 21
        "S 5 -> S 3 S 4", // 13
        "S 4 -> S 2 S 3", // 8
        "S 3 -> S 1 S 2", // 5
        "S 2 -> S 0 S 1", // 3
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 27");
    grammar->InstallNumVars();
    grammar->updateLevel();

	auto start = high_resolution_clock::now();
	unsigned int numVars = 514229; // 514229
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	} 

    std::vector<int> fibIndices = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025, 121393, 196418, 317811, 514229};


    G_CFLOBDD F = computeFibFunction(3, vars, level, grammar, 0, fibIndices);
    for (unsigned int i = 5; i < fibIndices.size() - 1; i++) {
        std::cout << "Computing Fibonacci function for n = " << i << std::endl;
        G_CFLOBDD Fi = computeFibFunction(i, vars, level, grammar, fibIndices[i - 1], fibIndices);
        F = MkAnd(F, Fi);
    }

    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;
    unsigned int nodeCount = 0, edgeCount = 0;
    F.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl; 
}

void Tests::testSynFun6() {
    // Implementation for testSynFun6 goes here
    // std::vector<std::string> productions = {
    //     // Define the grammar productions here
    //     "S 10 -> S 9 S 9", // 1024
    //     "S 9 -> S 8 S 8", // 512
    //     "S 8 -> S 7 S 7", // 256
    //     "S 7 -> S 6 S 6", // 128
    //     "S 6 -> S 5 S 5", // 64
    //     "S 5 -> S 4 S 4", // 32
    //     "S 4 -> S 3 S 3", // 16
    //     "S 3 -> S 2 S 2", // 8
    //     "S 2 -> S 1 S 1", // 4
    //     "S 1 -> S 0 S 0", // 2
    //     "S 0 -> a"
    // };

    // std::vector<std::string> productions = {
    //     // Define the grammar productions here
    //     "S 5 -> S 4 S 4 S 4 S 4 S 4 S 4",
    //     "S 4 -> S 3 S 3 S 3 S 3 S 3",
    //     "S 3 -> S 2 S 2 S 2",
    //     "S 2 -> S 1 S 1 S 1 S 1 S 1", // 10
    //     "S 1 -> S 0 S 0", // 2
    //     "S 0 -> a"
    // };

    // std::vector<std::string> productions = {
    //     // Define the grammar productions here
    //     "S 4 -> S 3 S 3 S 3 S 3",
    //     "S 3 -> S 2 S 2 S 2 S 2",
    //     "S 2 -> S 1 S 1 S 1 S 1 S 1 S 1 S 1 S 1", // 16
    //     "S 1 -> S 0 S 0", // 2
    //     "S 0 -> a"
    // };

    std::vector<std::string> productions = {
        // Define the grammar productions here
        "S 5 -> S 4 S 4 S 4 S 4 S 4 S 4",
        "S 4 -> S 3 S 3 S 3 S 3 S 3",
        "S 3 -> S 2 S 2 S 2",
        "S 2 -> S 1 S 1 S 1 S 1 S 1", // 10
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 5");
    grammar->InstallNumVars();
    grammar->updateLevel();

    auto start = high_resolution_clock::now();
    unsigned int k = 30;
    unsigned int n = 30;
	unsigned int numVars = k * n; // 40
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	}
    
    std::vector<G_CFLOBDD> groups;
    for (unsigned int i = 0; i < k; i++) {
        std::cout << "Creating group function " << i << " / " << k << std::endl;
        G_CFLOBDD groupFunc = vars[i * n] + 2 * vars[i * n + 1];
        for (unsigned int j = 2; j < numVars / k; j++) {
            int coeff = (j + 1);
            groupFunc = groupFunc + coeff * vars[i * n + j];
        }
        groups.push_back(groupFunc);
    }
    
    G_CFLOBDD F = groups[0];
    for (unsigned int i = 1; i < groups.size(); i++) {
        std::cout << "Combining group function " << i << " / " << k << std::endl;
        // F = MkAnd(F, groups[i]);
        F = F * groups[i];
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;

    unsigned int nodeCount = 0, edgeCount = 0;
    F.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl;
}

void Tests::testSynFun7() {
    // std::vector<std::string> productions = {
    // //     // Define the grammar productions here
    // //     "S 17 -> S 16 S 16", // 131072
    //     "S 16 -> S 15 S 15", // 65536
    //     "S 15 -> S 14 S 14", // 32768
    //     "S 14 -> S 13 S 13", // 16384
    //     "S 13 -> S 12 S 12", // 8192
    //     "S 12 -> S 11 S 11", // 4096
    //     "S 11 -> S 10 S 10", // 2048
    //     "S 10 -> S 9 S 9", // 1024
    //     "S 9 -> S 8 S 8", // 512
    //     "S 8 -> S 7 S 7", // 256
    //     "S 7 -> S 6 S 6", // 128
    //     "S 6 -> S 5 S 5", // 64
    //     "S 5 -> S 4 S 4", // 32
    //     "S 4 -> S 3 S 3", // 16
    //     "S 3 -> S 2 S 2", // 8
    //     "S 2 -> S 1 S 1", // 4
    //     "S 1 -> S 0 S 0", // 2
    //     "S 0 -> a"
    // };

    // std::vector<std::string> productions = {
    //     // Define the grammar productions here
    //     "S 6 -> S 5 S 5", // 1250 * 2 = 2500
    //     "S 5 -> S 4 S 4 S 4 S 4 S 4", // 250 * 5 = 1250
    //     "S 4 -> S 3 S 3 S 3 S 3 S 3", // 50 * 5 = 250
    //     "S 3 -> S 2 S 2 S 2 S 2 S 2", // 50
    //     "S 2 -> S 1 S 1 S 1 S 1 S 1", // 10
    //     "S 1 -> S 0 S 0", // 2
    //     "S 0 -> a"
    // };

    // std::vector<std::string> productions = {
    //     // Define the grammar productions here
    //     "S 7 -> S 6 S 6 S 6 S 6", // 2500 * 4 = 10000
    //     "S 6 -> S 5 S 5 S 5 S 5 S 5", // 500 * 5 = 2500
    //     "S 5 -> S 4 S 4 S 4 S 4 S 4", // 100 * 5
    //     "S 4 -> S 3 S 3", // 50 * 2 = 100
    //     "S 3 -> S 2 S 2 S 2 S 2 S 2", // 50
    //     "S 2 -> S 1 S 1 S 1 S 1 S 1", // 10
    //     "S 1 -> S 0 S 0", // 2
    //     "S 0 -> a"
    // };

    // std::vector<std::string> productions = {
    //     // Define the grammar productions here
    //     "S 8 -> S 7 S 7 S 7 S 7", // 10000 * 4 = 40000
    //     "S 7 -> S 6 S 6 S 6 S 6", // 2500 * 4 = 10000
    //     "S 6 -> S 5 S 5 S 5 S 5 S 5", // 500 * 5 = 2500
    //     "S 5 -> S 4 S 4 S 4 S 4 S 4", // 100 * 5
    //     "S 4 -> S 3 S 3", // 50 * 2 = 100
    //     "S 3 -> S 2 S 2 S 2 S 2 S 2", // 50
    //     "S 2 -> S 1 S 1 S 1 S 1 S 1", // 10
    //     "S 1 -> S 0 S 0", // 2
    //     "S 0 -> a"
    // };

    std::vector<std::string> productions = {
        // Define the grammar productions here
        "S 9 -> S 8 S 8 S 8", // 30000 * 3 = 90000
        "S 8 -> S 7 S 7 S 7", // 10000 * 3 = 30000
        "S 7 -> S 6 S 6 S 6 S 6", // 2500 * 4 = 10000
        "S 6 -> S 5 S 5 S 5 S 5 S 5", // 500 * 5 = 2500
        "S 5 -> S 4 S 4 S 4 S 4 S 4", // 100 * 5
        "S 4 -> S 3 S 3", // 50 * 2 = 100
        "S 3 -> S 2 S 2 S 2 S 2 S 2", // 50
        "S 2 -> S 1 S 1 S 1 S 1 S 1", // 10
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };


    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 9");
    grammar->InstallNumVars();
    grammar->updateLevel();

    auto start = high_resolution_clock::now();
    unsigned int k = 300;
    unsigned int n = 300;
	unsigned int numVars = k * n; // 40
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	}
    
    std::vector<G_CFLOBDD> groups;
    for (unsigned int i = 0; i < k; i++) {
        std::cout << "Creating group function " << i << " / " << k << std::endl;
        G_CFLOBDD groupFunc = MkExclusiveOr(vars[i * n], vars[i * n + 1]);
        for (unsigned int j = 2; j < numVars / k; j++) {
            G_CFLOBDD tmp = MkExclusiveOr(groupFunc, vars[i * n + j]);
            groupFunc = tmp;
        }
        groups.push_back(groupFunc);
    }

    std::vector<std::string> boolOps = {"MkAnd", "MkOr", "MkExclusiveOr", "MkNor", "MkNand"};
    G_CFLOBDD F = MkTrue(level, grammar);
    for (unsigned int i = 0; i < groups.size(); i+=2) {
        if (i + 1 < groups.size()) {
            std::cout << "Combining group function " << i << " / " << k << std::endl;
        } else {
            std::cout << "Processing last unpaired group function " << i << " / " << k << std::endl;
            F = MkAnd(F, groups[i]);
            break;
        }
        G_CFLOBDD a = groups[i];
        G_CFLOBDD b = groups[i + 1];
        auto op1_s = boolOps[i % boolOps.size()];
        auto op2_s = boolOps[(i + 1) % boolOps.size()];
        G_CFLOBDD c = MkTrue(level, grammar);
        if (op1_s == "MkAnd") {
            c = MkAnd(a, b);
        } else if (op1_s == "MkOr") {
            c = MkOr(a, b);
        } else if (op1_s == "MkExclusiveOr") {
            c = MkExclusiveOr(a, b);
        } else if (op1_s == "MkNor") {
            c = MkNor(a, b);
        } else if (op1_s == "MkNand") {
            c = MkNand(a, b);
        }

        G_CFLOBDD d = MkTrue(level, grammar);
        if (op2_s == "MkAnd") {
            d = MkAnd(a, b);
        } else if (op2_s == "MkOr") {
            d = MkOr(a, b);
        } else if (op2_s == "MkExclusiveOr") {
            d = MkExclusiveOr(a, b);
        } else if (op2_s == "MkNor") {
            d = MkNor(a, b);
        } else if (op2_s == "MkNand") {
            d = MkNand(a, b);
        }
        F = MkAnd(F, MkOr(c, d));
    }
    

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;

    unsigned int nodeCount = 0, edgeCount = 0;
    F.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl;
}

void Tests::testSynFun8() {
    std::vector<std::string> productions = {
        // Define the grammar productions here
        // "S 20 -> S 19 S 19", // 1048576
        // "S 19 -> S 18 S 18", // 524288
        "S 18 -> S 17 S 17", // 262144
        "S 17 -> S 16 S 16", // 131072
        "S 16 -> S 15 S 15", // 65536
        "S 15 -> S 14 S 14", // 32768
        "S 14 -> S 13 S 13", // 16384
        "S 13 -> S 12 S 12", // 8192
        "S 12 -> S 11 S 11", // 4096
        "S 11 -> S 10 S 10", // 2048
        "S 10 -> S 9 S 9", // 1024
        "S 9 -> S 8 S 8", // 512
        "S 8 -> S 7 S 7", // 256
        "S 7 -> S 6 S 6", // 128
        "S 6 -> S 5 S 5", // 64
        "S 5 -> S 4 S 4", // 32
        "S 4 -> S 3 S 3", // 16
        "S 3 -> S 2 S 2", // 8
        "S 2 -> S 1 S 1", // 4
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };

    // std::vector<std::string> productions = {
    //     // Define the grammar productions here
    //     // "S 17 -> S 16 S 16", // 131072
    //     // "S 16 -> S 15 S 15", // 65536
    //     // "S 15 -> S 0 S 14", // 32768
    //     // "S 14 -> S 13 S 13", // 16384
    //     // "S 13 -> S 0 S 12", // 8192
    //     // "S 12 -> S 11 S 11 S 11", // 4096
    //     "S 12 -> S 0 S 11",
    //     "S 11 -> S 10 S 10 S 10", // 2048
    //     "S 10 -> S 9 S 9 S 9", // 1024
    //     "S 9 -> S 8 S 8 S 8", // 512
    //     "S 8 -> S 7 S 7 S 7", // 256
    //     "S 7 -> S 6 S 6 S 6", // 128
    //     "S 6 -> S 5 S 5 S 5", // 64
    //     "S 5 -> S 4 S 4 S 4", // 32
    //     "S 4 -> S 3 S 3 S 3", // 16
    //     "S 3 -> S 2 S 2 S 2", // 27
    //     "S 2 -> S 1 S 1 S 1", // 9
    //     "S 1 -> S 0 S 0 S 0", // 3
    //     "S 0 -> a"
    // };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 18");
    grammar->InstallNumVars();
    grammar->updateLevel();

    auto start = high_resolution_clock::now();
	unsigned int numVars = pow(3, 11) + 1; // 59049 + 1
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	}
    
    G_CFLOBDD F1 = MkTrue(level, grammar);
    for (unsigned int i = 1; i < numVars; i+=3) {
        if (i % 10000 == 0) {
            std::cout << "Processing clause for variables " << i << ", " << i+1 << ", " << i+2 << std::endl;
        }
        auto tmp = MkOr(MkAnd(MkAnd(vars[i+1], vars[i]), MkNot(vars[i+2])), MkAnd(MkAnd(MkNot(vars[i]), vars[i+2]), MkNot(vars[i+1])));
        F1 = MkAnd(F1, tmp);
    }

    G_CFLOBDD F2 = MkTrue(level, grammar);
    for (unsigned int i = 1; i < numVars; i+=3) {
        if (i % 10000 == 0) {
            std::cout << "Processing clause for variables " << i << ", " << i+1 << ", " << i+2 << std::endl;
        }
        auto tmp = MkOr(MkAnd(vars[i], MkExclusiveOr(vars[i+1], vars[i+2])), MkAnd(MkNot(vars[i]), MkNot(MkExclusiveOr(vars[i+1], vars[i+2]))));
        F2 = MkAnd(F2, tmp);
    }

    G_CFLOBDD F = MkOr(MkAnd(vars[0], F1), MkAnd(MkNot(vars[0]), F2));
    

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;

    unsigned int nodeCount = 0, edgeCount = 0;
    F.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl;
}

G_CFLOBDD computeSyn9Function(const std::vector<G_CFLOBDD>& vars, int startIndex, int n, unsigned int level, std::shared_ptr<Grammar> grammar) {
    G_CFLOBDD F1 = MkTrue(level, grammar);
    std::vector<std::string> boolOps = {"MkAnd", "MkOr", "MkExclusiveOr", "MkNor", "MkNand"};
    for (int i = 0; i < n; i++) {
        // std::cout << "Combining variable " << (startIndex + i) << " / " << (startIndex + n - 1) << std::endl;
        G_CFLOBDD var = vars[startIndex + i];
        if (i % 2 == 0) {
            var = MkNot(var);
        }
        auto op_s = boolOps[i % boolOps.size()];
        if (op_s == "MkAnd") {
            F1 = MkAnd(F1, var);
        } else if (op_s == "MkOr") {
            F1 = MkOr(F1, var);
        } else if (op_s == "MkExclusiveOr") {
            F1 = MkExclusiveOr(F1, var);
        } else if (op_s == "MkNor") {
            F1 = MkNor(F1, var);
        } else if (op_s == "MkNand") {
            F1 = MkNand(F1, var);
        }
    }

    // vars: G0 G1 G2 G3 G4
    // ((((NOT(G0) OR G1) ExOR NOT(G2)) NOR G3) NAND NOT(G4))

    auto F = F1;
    return F;
}

void Tests::testSynFun9() {
    std::vector<std::string> productions = {
        // Define the grammar productions here
        // "S 20 -> S 19 S 19", // 1048576
        // "S 19 -> S 18 S 18", // 524288
        // "S 18 -> S 17 S 17", // 262144
        // "S 17 -> S 16 S 16", // 131072
        // "S 16 -> S 15 S 15", // 65536
        // "S 15 -> S 14 S 14", // 32768
        // "S 14 -> S 13 S 13", // 16384
        // "S 13 -> S 12 S 12", // 8192
        "S 12 -> S 11 S 11", // 4096
        "S 11 -> S 10 S 10", // 2048
        "S 10 -> S 9 S 9", // 1024
        "S 9 -> S 8 S 8", // 512
        "S 8 -> S 7 S 7", // 256
        "S 7 -> S 6 S 6", // 128
        "S 6 -> S 5 S 5", // 64
        "S 5 -> S 4 S 4", // 32
        "S 4 -> S 3 S 3", // 16
        "S 3 -> S 2 S 2", // 8
        "S 2 -> S 1 S 1", // 4
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };

    // std::vector<std::string> productions = {
    //     // "S 7 -> S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6", // 30030 * 17 = 510510
    //     // "S 6 -> S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5", // 2310 * 13 = 30030
    //     "S 5 -> S 4 S 4 S 4 S 4 S 4 S 4 S 4 S 4 S 4 S 4 S 4", // 210 * 11 = 2310
    //     "S 4 -> S 3 S 3 S 3 S 3 S 3 S 3 S 3", // 30 * 7 = 210
    //     "S 3 -> S 2 S 2 S 2 S 2 S 2", // 6 * 5 = 30
    //     "S 2 -> S 1 S 1 S 1", // 2 * 3 = 6
    //     "S 1 -> S 0 S 0", // 2
    //     "S 0 -> a"
    // };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 12");
    grammar->InstallNumVars();
    grammar->updateLevel();

    auto start = high_resolution_clock::now();
	unsigned int numVars = 2310; // 59049 + 1
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	}
    
    std::vector<int> groupIndices = {2, 3, 5, 7, 11};
    
    std::vector<G_CFLOBDD> groupFunctions = vars;
    for (unsigned int i = 0; i < groupIndices.size(); i++) {
        // vars[0] vars[1] vars[2] vars[3]...
        // std::cout << "Combining group functions in groups of " << groupIndices[i] << std::endl;
        std::vector<G_CFLOBDD> tmpGroupFunctions;
        std::cout << "Number of group functions to process: " << groupFunctions.size() << std::endl;
        for (int j = 0; j < groupFunctions.size(); j += groupIndices[i]) {
            // std::cout << "Processing group starting at index " << j << std::endl;
            G_CFLOBDD gf = computeSyn9Function(groupFunctions, j, groupIndices[i], level, grammar);
            tmpGroupFunctions.push_back(gf);
        }
        groupFunctions = tmpGroupFunctions;
        // F(vars[0], vars[1]) F(vars[2], vars[3]), ...
    }

    auto F = groupFunctions[0];

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;

    unsigned int nodeCount = 0, edgeCount = 0;
    F.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl;
}

void Tests::testSynFun10() {
    std::vector<std::string> productions = {
        // Define the grammar productions here
        // "S 20 -> S 19 S 19", // 1048576
        // "S 19 -> S 18 S 18", // 524288
        // "S 18 -> S 17 S 17", // 262144
        // "S 17 -> S 16 S 16", // 131072
        // "S 16 -> S 15 S 15", // 65536
        // "S 15 -> S 14 S 14", // 32768
        // "S 14 -> S 13 S 13", // 16384
        // "S 13 -> S 12 S 12", // 8192
        // "S 12 -> S 11 S 11", // 4096
        // "S 11 -> S 10 S 10", // 2048
        "S 10 -> S 9 S 9", // 1024
        "S 9 -> S 8 S 8", // 512
        "S 8 -> S 7 S 7", // 256
        "S 7 -> S 6 S 6", // 128
        "S 6 -> S 5 S 5", // 64
        "S 5 -> S 4 S 4", // 32
        "S 4 -> S 3 S 3", // 16
        "S 3 -> S 2 S 2", // 8
        "S 2 -> S 1 S 1", // 4
        "S 1 -> S 0 S 0", // 2
        "S 0 -> a"
    };

    // std::vector<std::string> productions = {
    //     // "S 7 -> S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6 S 6", // 30030 * 17 = 510510
    //     // "S 6 -> S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5", // 10395 * 13 = 135135
    //     // "S 5 -> S 4 S 4 S 4 S 4 S 4 S 4 S 4 S 4 S 4 S 4 S 4", // 945 * 11 = 10395
    //     "S 4 -> S 3 S 3 S 3 S 3 S 3 S 3 S 3 S 3 S 3", // 105 * 9 = 945
    //     "S 3 -> S 2 S 2 S 2 S 2 S 2 S 2 S 2", // 15 * 7 = 105
    //     "S 2 -> S 1 S 1 S 1 S 1 S 1", // 3 * 5 = 15
    //     "S 1 -> S 0 S 0 S 0", // 3
    //     "S 0 -> a"
    // };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 10");
    grammar->InstallNumVars();
    grammar->updateLevel();

    auto start = high_resolution_clock::now();
	unsigned int numVars = 945; // 59049 + 1
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	}
    
    std::vector<int> groupIndices = {3, 5, 7, 9};
    
    std::vector<G_CFLOBDD> groupFunctions = vars;
    for (unsigned int i = 0; i < groupIndices.size(); i++) {
        // std::cout << "Combining group functions in groups of " << groupIndices[i] << std::endl;
        std::vector<G_CFLOBDD> tmpGroupFunctions;
        std::cout << "Number of group functions to process: " << groupFunctions.size() << std::endl;
        for (int j = 0; j < groupFunctions.size(); j += groupIndices[i]) {
            // std::cout << "Processing group starting at index " << j << std::endl;
            G_CFLOBDD gf = computeSyn9Function(groupFunctions, j, groupIndices[i], level, grammar);
            tmpGroupFunctions.push_back(gf);
        }
        groupFunctions = tmpGroupFunctions;
    }

    auto F = groupFunctions[0];

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;

    unsigned int nodeCount = 0, edgeCount = 0;
    F.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl;
}

void Tests::testSynFun11() {
    // std::vector<std::string> productions = {
    //     // Define the grammar productions here
    //     // "S 20 -> S 19 S 19", // 1048576
    //     // "S 19 -> S 18 S 18", // 524288
    //     "S 18 -> S 17 S 17", // 262144
    //     "S 17 -> S 16 S 16", // 131072
    //     "S 16 -> S 15 S 15", // 65536
    //     "S 15 -> S 14 S 14", // 32768
    //     "S 14 -> S 13 S 13", // 16384
    //     "S 13 -> S 12 S 12", // 8192
    //     "S 12 -> S 11 S 11", // 4096
    //     "S 11 -> S 10 S 10", // 2048
    //     "S 10 -> S 9 S 9", // 1024
    //     "S 9 -> S 8 S 8", // 512
    //     "S 8 -> S 7 S 7", // 256
    //     "S 7 -> S 6 S 6", // 128
    //     "S 6 -> S 5 S 5", // 64
    //     "S 5 -> S 4 S 4", // 32
    //     "S 4 -> S 3 S 3", // 16
    //     "S 3 -> S 2 S 2", // 8
    //     "S 2 -> S 1 S 1", // 4
    //     "S 1 -> S 0 S 0", // 2
    //     "S 0 -> a"
    // };

    std::vector<std::string> productions = {
        "S 8 -> S 7 S 7 S 7 S 7 S 7 S 7", // 32400 * 6 = 194400
        "S 7 -> S 6 S 6 S 6", // 10800 * 3 = 32400
        "S 6 -> S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5 S 5", // 720 * 15 = 10800
        "S 5 -> S 4 S 4 S 4", // 240 * 3 = 720
        "S 4 -> S 3 S 3 S 3 S 3", // 60 * 4 = 240
        "S 3 -> S 2 S 2 S 2 S 2 S 2 S 2", // 10 * 6 = 60
        "S 2 -> S 1 S 1", // 5 * 2 = 10
        "S 1 -> S 0 S 0 S 0 S 0 S 0", // 5
        "S 0 -> a"
    };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 8");
    grammar->InstallNumVars();
    grammar->updateLevel();

    auto start = high_resolution_clock::now();
	unsigned int numVars = 194400; // 59049 + 1
	unsigned int level = grammar->root->level;
	std::vector<G_CFLOBDD> vars;
	for (unsigned int i = 0; i < numVars; i++) {
        if (i % 10000 == 0) {
            std::cout << "Creating projection for variable " << i << " / " << numVars << std::endl;
        }
		vars.push_back(MkProjection(i, level, grammar));
	}
    
    std::vector<int> groupIndices = {5, 2, 6, 4, 3, 15, 3, 6};
    
    std::vector<G_CFLOBDD> groupFunctions = vars;
    for (unsigned int i = 0; i < groupIndices.size(); i++) {
        // std::cout << "Combining group functions in groups of " << groupIndices[i] << std::endl;
        std::vector<G_CFLOBDD> tmpGroupFunctions;
        std::cout << "Number of group functions to process: " << groupFunctions.size() << std::endl;
        for (int j = 0; j < groupFunctions.size(); j += groupIndices[i]) {
            // std::cout << "Processing group starting at index " << j << std::endl;
            G_CFLOBDD gf = computeSyn9Function(groupFunctions, j, groupIndices[i], level, grammar);
            tmpGroupFunctions.push_back(gf);
        }
        groupFunctions = tmpGroupFunctions;
    }

    auto F = groupFunctions[0];

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;

    unsigned int nodeCount = 0, edgeCount = 0;
    F.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl;
}

void Tests::testC17() {
    HardwareBenchmarks::c17();
}

void Tests::testC432(unsigned int grammarChoice) {
    HardwareBenchmarks::c432(grammarChoice);
}

void Tests::testC880(unsigned int grammarChoice) {
    HardwareBenchmarks::c880(grammarChoice);
}

void Tests::testC6288_8(unsigned int grammarChoice) {
    HardwareBenchmarks::c6288_8(grammarChoice);
}

void Tests::testC6288_12() {
    HardwareBenchmarks::c6288_12();
}

void Tests::testC6288_9(unsigned int grammarChoice) {
    HardwareBenchmarks::c6288_9(grammarChoice);
}

void Tests::testC6288_10(unsigned int grammarChoice) {
    HardwareBenchmarks::c6288_10(grammarChoice);
}

void Tests::testC6288_16(unsigned int grammarChoice) {
    HardwareBenchmarks::c6288_16(grammarChoice);
}

void createBalancedGrammar(std::shared_ptr<Grammar>& grammar, unsigned int depth) {
    std::vector<std::string> productions;
    for (int i = depth; i > 0; i--) {
        productions.push_back("S " + std::to_string(i) + " -> S " + std::to_string(i - 1) + " S " + std::to_string(i - 1));
    }
    productions.push_back("S 0 -> a");

    grammar->constructGrammar(productions, "S " + std::to_string(depth));
    grammar->InstallNumVars();
    grammar->updateLevel();
}

void createFrontFibonacciGrammar(std::shared_ptr<Grammar>& grammar, unsigned int n) {
    std::vector<std::string> productions;
    for (int i = n; i >= 3; i--) {
        productions.push_back("S " + std::to_string(i) + " -> S " + std::to_string(i - 1) + " S " + std::to_string(i - 2));
    }
    productions.push_back("S 2 -> S 1 S 0");
    productions.push_back("S 1 -> S 0 S 0");
    productions.push_back("S 0 -> a");

    grammar->constructGrammar(productions, "S " + std::to_string(n));
    grammar->InstallNumVars();
    grammar->updateLevel();
}

void createBackFibonacciGrammar(std::shared_ptr<Grammar>& grammar, unsigned int n) {
    std::vector<std::string> productions;
    for (int i = n; i >= 3; i--) {
        productions.push_back("S " + std::to_string(i) + " -> S " + std::to_string(i - 2) + " S " + std::to_string(i - 1));
    }
    productions.push_back("S 2 -> S 0 S 1");
    productions.push_back("S 1 -> S 0 S 0");
    productions.push_back("S 0 -> a");

    grammar->constructGrammar(productions, "S " + std::to_string(n));
    grammar->InstallNumVars();
    grammar->updateLevel();
}

void createLinearGrammar(std::shared_ptr<Grammar>& grammar, unsigned int n) {
    std::vector<std::string> productions;
    std::string production = "S 1 -> ";
    for (int i = n; i > 1; i--) {
        production += "S 0 ";
    }
    production += "S 0";
    productions.push_back(production);
    productions.push_back("S 0 -> a");

    grammar->constructGrammar(productions, "S 1");
    grammar->InstallNumVars();
    grammar->updateLevel();
}

void Tests::testNQueens(unsigned int n, unsigned int grammarChoice) {

    std::cout << "Testing " << n << "-Queens with grammar choice " << grammarChoice << std::endl;

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    switch (grammarChoice) {
        case 0:
            createBalancedGrammar(grammar, std::ceil(std::log2(n * n)));
            break;
        case 1:
            {
                std::vector<std::string> productions = {
                    "S 4 -> S 3 S 3",
                    "S 3 -> S 2 S 2 S 2 S 2 S 2 S 2 S 2 S 2 S 2 S 2", // 16 * 8 = 128
                    "S 2 -> S 1 S 1", // 16
                    "S 1 -> S 0 S 0 S 0 S 0 S 0 S 0 S 0 S 0 S 0 S 0", // 8
                    "S 0 -> a"
                };
                grammar->constructGrammar(productions, "S 4");
                grammar->InstallNumVars();
                grammar->updateLevel(); 
            }
            break;
        case 2:
            {
                std::vector<std::string> productions = {
                    "S 10 -> S 9 S S 4 S 2", // 100
                    "S 9 -> S 8 S 7", // 89
                    "S 8 -> S 7 S 6", // 55
                    "S 7 -> S 6 S 5", // 34
                    "S 6 -> S 5 S 4", // 21
                    "S 5 -> S 4 S 3", // 13
                    "S 4 -> S 3 S 2", // 8
                    "S 3 -> S 2 S 1", // 5
                    "S 2 -> S 1 S 0", // 3
                    "S 1 -> S 0 S 0", // 2
                    "S 0 -> a"
                };
                grammar->constructGrammar(productions, "S 10");
                grammar->InstallNumVars();
                grammar->updateLevel(); 
            }
            break;
        case 3:
            createLinearGrammar(grammar, n * n);
            break;
        case 4:
            {
                std::vector<std::string> productions = {
                    "S 8 -> S 7 S 5", // 100
                    "S 7 -> S 6 S 6", // 80
                    "S 6 -> S 5 S 5", // 40
                    "S 5 -> S 4 S 4", // 20
                    "S 4 -> S 3 S 1", // 10
                    "S 3 -> S 2 S 2", // 8
                    "S 2 -> S 1 S 1", // 4
                    "S 1 -> S 0 S 0", // 2
                    "S 0 -> a"
                };
                grammar->constructGrammar(productions, "S 8");
                grammar->InstallNumVars();
                grammar->updateLevel(); 
            } 
            break;
        case 5:
            {
                std::vector<std::string> productions = {
                    "S 4 -> S 3 S 3 S 3 S 3 S 3 S 3 S 3 S 3 S 3 S 3", // 100
                    "S 3 -> S 2 S 2 S 1", // 10
                    "S 2 -> S 1 S 1", // 4
                    "S 1 -> S 0 S 0", // 2
                    "S 0 -> a"
                };
                grammar->constructGrammar(productions, "S 4");
                grammar->InstallNumVars();
                grammar->updateLevel(); 
            } 
            break;
        case 6:
            {
                std::vector<std::string> productions = {
                    "S 6 -> S 5 S 5 S 4", // 100
                    "S 5 -> S 4 S 4", // 40
                    "S 4 -> S 3 S 3", // 20
                    "S 3 -> S 2 S 2 S 1", // 10
                    "S 2 -> S 1 S 1", // 4
                    "S 1 -> S 0 S 0", // 2
                    "S 0 -> a"
                };
                grammar->constructGrammar(productions, "S 6");
                grammar->InstallNumVars();
                grammar->updateLevel(); 
            } 
            break;
        case 7:
            {
                std::vector<std::string> productions = {
                    "S 4 -> S 3 S 3 S 3 S 3 S 3 S 3 S 3 S 3", // 16
                    "S 3 -> S 2 S 2", // 8
                    "S 2 -> S 1 S 1", // 4
                    "S 1 -> S 0 S 0", // 2
                    "S 0 -> a"
                };
                grammar->constructGrammar(productions, "S 4");
                grammar->InstallNumVars();
                grammar->updateLevel(); 
            } 
            break;

        case 8:
            {
                std::vector<std::string> productions = {
                    "S 3 -> S 2 S 2 S 2 S 2", // 16
                    "S 2 -> S 1 S 1", // 4
                    "S 1 -> S 0 S 0", // 2
                    "S 0 -> a"
                };
                grammar->constructGrammar(productions, "S 3");
                grammar->InstallNumVars();
                grammar->updateLevel(); 
            } 
            break;
    }

    auto start = high_resolution_clock::now();
	std::vector<std::vector<G_CFLOBDD>> vars;
	unsigned int numVars = n * n;
	for (unsigned int i = 0; i < n; i++) {
		vars.push_back(std::vector<G_CFLOBDD>());
		for (unsigned int j = 0; j < n; j++) {
			vars[i].push_back(MkProjection(i * n + j, std::ceil(std::log2(numVars)), grammar));
		}
	}

	std::vector<G_CFLOBDD> orBatch;
	for (int i = 0; i < n; i++) {
		G_CFLOBDD condition = MkFalse(std::ceil(std::log2(numVars)), grammar);
		for (int j = 0; j < n; j++) {
			condition = MkOr(condition, vars[i][j]);
		}
		orBatch.push_back(condition);
	}

	std::vector<std::vector<G_CFLOBDD>> impBatch;

	for (int i = 0; i < n; i++) {
        std::cout << "Processing implications for row " << i << " / " << n << std::endl;
        std::vector<G_CFLOBDD> row;
		for (int j = 0; j < n; j++) {
			G_CFLOBDD a = MkTrue(std::ceil(std::log2(numVars)), grammar);
            G_CFLOBDD b = MkTrue(std::ceil(std::log2(numVars)), grammar);
            G_CFLOBDD c = MkTrue(std::ceil(std::log2(numVars)), grammar);
            G_CFLOBDD d = MkTrue(std::ceil(std::log2(numVars)), grammar);

			int k, l;

			/* No one in the same column */
			for (l = 0; l < n; l++) {
				if (l != j) {
					G_CFLOBDD mp = MkImplies(vars[i][j], MkNot(vars[i][l]));
					a = MkAnd(a, mp);
				}
			}

			/* No one in the same row */
			for (k = 0; k < n; k++) {
				if (k != i) {
					G_CFLOBDD mp = MkImplies(vars[i][j], MkNot(vars[k][j]));
					b = MkAnd(b, mp);
				}
			}

			/* No one in the same up-right diagonal */
			for (k = 0; k < n; k++) {
				int ll = k - i + j;
				if (ll >= 0 && ll < n) {
					if (k != i) {
						G_CFLOBDD mp = MkImplies(vars[i][j], MkNot(vars[k][ll]));
						c = MkAnd(c, mp);
					}
				}
			}

			/* No one in the same down-right diagonal */
			for (k = 0; k < n; k++) {
				int ll = i + j - k;
				if (ll >= 0 && ll < n) {
					if (k != i) {
						G_CFLOBDD mp = MkImplies(vars[i][j], MkNot(vars[k][ll]));
						d = MkAnd(d, mp);
					}
				}
			}

			c = MkAnd(c, d);
			b = MkAnd(b, c);
			a = MkAnd(a, b);
			row.push_back(a);
		}
        impBatch.push_back(row);
	}

	G_CFLOBDD queen = MkTrue(std::ceil(std::log2(numVars)), grammar);

	for (int i = 0; i < n; i++) {
        std::cout << "Combining OR conditions for row " << i << " / " << n << std::endl;
		queen = MkAnd(queen, orBatch[i]);
	}

	for (int i = 0; i < n; i++) {
        G_CFLOBDD tmp_queen = MkTrue(std::ceil(std::log2(numVars)), grammar);
		for (int j = 0; j < n; j++) {
            std::cout << "Combining implication conditions for position (" << i << ", " << j << ") " << " / " << n << std::endl;
			tmp_queen = MkAnd(tmp_queen, impBatch[i][j]);
		}
        queen = MkAnd(queen, tmp_queen);
	}

	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start);
	std::cout << "Duration: " << duration.count() << " ms" << std::endl;
    unsigned int numDummyVars = std::pow(2, std::ceil(std::log2(numVars))) - numVars;
    std::cout << "Number of dummy variables: " << numDummyVars << " " << std::pow(2, std::ceil(std::log2(numVars))) << " " << numVars << std::endl;
	queen.CountPaths();
    unsigned int nodeCount = 0, edgeCount = 0;
    queen.CountNodesAndEdges(nodeCount, edgeCount);
    std::cout << "nodeCount: " << nodeCount << " edgeCount: " << edgeCount << " totalCount: " << (nodeCount + edgeCount) << std::endl;
	G_CFLOBDDInternalNode* queen_node = (G_CFLOBDDInternalNode*) queen.root->rootConnection.entryPointHandle->handleContents;
	// std::cout << "Number of non-solutions for " << n << "-Queens: " << queen_node->numPathsToExit[0] << std::endl;
	std::cout << "Number of solutions for " << n << "-Queens: " << queen_node->numPathsToExit[1] / std::pow(2, numDummyVars) << std::endl;
    std::cout << "Number of solutions for " << n << "-Queens: " << queen_node->numPathsToExit[1] << std::endl;
}

void Tests::testBDDGrammar() {
    std::vector<std::string> productions = {
        "S 0 -> BDD(5)",
    };

    std::shared_ptr<Grammar> grammar = std::make_shared<Grammar>();
    grammar->constructGrammar(productions, "S 0");
    grammar->InstallNumVars();
    grammar->updateLevel(); 

    G_CFLOBDD x0 = MkProjection(0, grammar->root->level, grammar);
    G_CFLOBDD x1 = MkProjection(1, grammar->root->level, grammar);
    G_CFLOBDD x2 = MkProjection(2, grammar->root->level, grammar);
    G_CFLOBDD x3 = MkProjection(3, grammar->root->level, grammar);
    G_CFLOBDD x4 = MkProjection(4, grammar->root->level, grammar);

    // complicated function
    G_CFLOBDD and_x0_x1 = MkAnd(x0, x1);
    G_CFLOBDD or_x3_x4 = MkOr(x3, x4);
    G_CFLOBDD and_x2_or_x3_x4 = MkAnd(x2, or_x3_x4);
    G_CFLOBDD F = MkNor(and_x0_x1, and_x2_or_x3_x4);
    F.PrintYield(std::cout);
}

void RunInit() {
    G_CFLOBDDNodeHandle::InitLeafNodes();
    InitPairProductCache();
    InitBDDPairProductCache();
    G_CFLOBDDNodeHandle::InitReduceCache();
    BDDNodeHandle::InitReduceCache();
}

void ClearUp() {
    G_CFLOBDDNodeHandle::DisposeOfReduceCache();
    DisposeOfPairProductCache();
    DisposeOfBDDPairProductCache();
    BDDNodeHandle::DisposeOfReduceCache();
}

void Tests::runTests(std::string testName, unsigned int grammarChoice, unsigned int n) {

    RunInit();
    if (testName == "testMkTrueAndFalse") {
        testMkTrueAndFalse();
    } else if (testName == "testMkProjection") {
        testMkProjection();
    } else if (testName == "testParity") {
        testParity();
    } else if (testName == "testDataVisualization") {
        testDataVisualization();
    } else if (testName == "testMkNot") {
        testMkNot();
    } else if (testName == "testCrossProduct") {
        testCrossProduct();
    } else if (testName == "testNand") {
        testNand();
    } else if (testName == "testRandomFunction") {
        testRandomFunction();
    } else if (testName == "testSynFun2") {
        testSynFun2();
    } else if (testName == "testSynFun3") {
        testSynFun3();
    } else if (testName == "testSynFun4") {
        testSynFun4();
    } else if (testName == "testSynFun5") {
        testSynFun5();
    } else if (testName == "testSynFun5") {
        testSynFun5();
    } else if (testName == "testSynFun6") {
        testSynFun6();
    } else if (testName == "testSynFun7") {
        testSynFun7();
    } else if (testName == "testSynFun8") {
        testSynFun8();
    } else if (testName == "testSynFun9") {
        testSynFun9();
    } else if (testName == "testSynFun10") {
        testSynFun10();
    } else if (testName == "testSynFun11") {
        testSynFun11();
    }   else if (testName == "testC17") {
        testC17();
    } else if (testName == "testC432") {
        testC432(grammarChoice);
    } else if (testName == "testC880") {
        testC880(grammarChoice);
    } else if (testName == "testC6288_8") {
        testC6288_8(grammarChoice);
    } else if (testName == "testC6288_9") {
        testC6288_9(grammarChoice);
    } else if (testName == "testC6288_10") {
        testC6288_10(grammarChoice);
    } else if (testName == "testC6288_12") {
        testC6288_12();
    } else if (testName == "testC6288_16") {
        testC6288_16(grammarChoice);
    } else if (testName == "testNQueens") {
        testNQueens(n, grammarChoice);
    } else if (testName == "testBDDGrammar") {
        testBDDGrammar();
    }
    else {
        std::cout << "Unknown test name: " << testName << std::endl;
    }
    ClearUp();
    // Add calls to other test functions here
}
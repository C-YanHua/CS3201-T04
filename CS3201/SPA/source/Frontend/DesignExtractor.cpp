#include <map>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Constants.h"
#include "Frontend/DesignExtractor.h"
#include "PKB/PKB.h"
#include "PKB/Table.h"
#include "Utils.h"

using std::map;
using std::queue;
using std::set;
using std::string;
using std::vector;

DesignExtractor DesignExtractor::instance;

DesignExtractor::DesignExtractor() {}

DesignExtractor& DesignExtractor::getInstance() {
    return instance;
}

void DesignExtractor::resetInstance() {
    instance = DesignExtractor();
}

void DesignExtractor::populatePKB(map<StmtNumber, unsigned int> stmtsLevels, unsigned int tableMaximumSize) {
    stmtsLevels_ = stmtsLevels;

    PKB::SetTableMaximumSize(tableMaximumSize);

    /* Generic tables population. */
    populateGenericTables();

    /* Stmt related tables population. Have dependencies on generic tables. */
    populateStmtRelatedTables();

    /* Expression tables population. */
    populateExpressionTables();

    /* Design abstraction tables population. */
    populateDesignAbstractionTables();

    /* Set design abstractions priority after all the design abstraction tables are populated. */
    PKB::PopulatePriorityTable();
}

void DesignExtractor::populateGenericTables() {
    unsigned int i;

    /* Populate Constant Table. */
    i = 1;
    for (ConstantValue constant : constants_) {
        PKB::InsertConstantValue(i++, constant);
    }

    /* Populate Variable Table. */
    i = 1;
    for (VariableName variableName : variableNames_) {
        PKB::InsertVariableName(i++, variableName);
    }

    /* Populate Procedure Table. */
    i = 1;
    for (ProcedureName procedureName : procedureNames_) {
        PKB::InsertProcedureName(i++, procedureName);
    }
}

void DesignExtractor::populateStmtRelatedTables() {
    for (auto &pair : controlVariableNames_) {
        PKB::InsertControlVariable(pair.first, PKB::GetVariableIndex(pair.second));
    }

    for (auto &pair : call_) {
        PKB::InsertCallStmt(pair.first, PKB::GetProcedureIndex(pair.second));
    }

    for (auto &pair : stmts_) {
        PKB::InsertStmt(pair.first, pair.second);
    }

    for (auto &pair : stmtlists_) {
        PKB::InsertStmtlist(pair.first, pair.second);
    }
}

void DesignExtractor::populateExpressionTables() {
    for (const auto &pair : expressions_) {
        unsigned int stmtNumber = pair.first;
        queue<string> postfixExpression = Utils::GetPostfixExpression(pair.second);

        PKB::InsertExpression(stmtNumber, Utils::GetExactExpressionWithBrackets(postfixExpression));

        std::set<SubExpression> subExpressions = Utils::GetSubExpressionsWithBrackets(postfixExpression);
        for (SubExpression subExpression : subExpressions) {
            PKB::InsertSubExpression(stmtNumber, subExpression);
        }
    }
}

void DesignExtractor::populateDesignAbstractionTables() {
    /* Calls table have to be computed first as other tables have dependencies on it. */
    precomputeCalls();

    precomputeModifies();
    precomputeUses();
    precomputeParent();
    precomputeFollows();
    precomputeNext();

    /* Initialize compute on demand matrixes. */
    PKB::InitializeNextTransitiveMatrixSize();
}

void DesignExtractor::setProcedureData(ProcedureName procedureName, StmtNumber firstStmtNumber, StmtNumber lastStmtNumber) {
    /* For PKB procedure table population. */
    procedureNames_.insert(procedureName);

    /* For PKB generic and design abstraction tables population. */
    proceduresFirstStmt_.insert(std::make_pair(firstStmtNumber, procedureName));
    proceduresLastStmt_.insert(std::make_pair(lastStmtNumber, procedureName));
}

void DesignExtractor::setStmtListData(Symbol stmtSymbol, StmtNumber stmtNumber) {
    /* For PKB stmtlist table population. */
    stmtlists_.insert(std::make_pair(stmtNumber, stmtSymbol));
}

void DesignExtractor::setContainerStmtData(Symbol stmtSymbol, StmtNumber stmtNumber, VariableName controlVariableName) {
    /* For PKB stmt table population. */
    stmts_.insert(std::make_pair(stmtNumber, stmtSymbol));

    /* For PKB variable table population. */
    variableNames_.insert(controlVariableName);

    /* For PKB control variable table population. */
    controlVariableNames_.insert(std::make_pair(stmtNumber, controlVariableName));

    /* For PKB uses table population. */
    uses_[stmtNumber].insert(controlVariableName);
}

void DesignExtractor::setIfStmtData(StmtNumber stmtNumber, StmtNumber thenLastStmtNumber, StmtNumber elseFirstStmtNumber) {
    thenLastStmt_.insert(std::make_pair(stmtNumber, thenLastStmtNumber));
    elseFirstStmt_.insert(std::make_pair(stmtNumber, elseFirstStmtNumber));
}

void DesignExtractor::setCallStmtData(StmtNumber stmtNumber, ProcedureName currentProcedureName, ProcedureName procedureName) {
    /* For PKB stmt table population. */
    stmts_.insert(std::make_pair(stmtNumber, CALL));

    /* For PKB calls table population. */
    calls_[currentProcedureName].insert(procedureName);

    /* For PKB call table population. */
    call_.insert(std::make_pair(stmtNumber, procedureName));

    /* For PKB modifies table population. */
    modifies_[stmtNumber] = "";

    /* For PKB uses table population. */
    uses_[stmtNumber] = {};
}

/* "variable" parameter can be either a control variable name, a constant value or a variable name. */
void DesignExtractor::setAssignStmtData(Symbol stmtSymbol, StmtNumber stmtNumber, VariableName variable) {
    switch (stmtSymbol) {
        default:
            break;

        case ASSIGN:
            /* For PKB stmt table population. */
            stmts_.insert(std::make_pair(stmtNumber, stmtSymbol));

            /* For PKB variable table population. */
            variableNames_.insert(variable);

            /* For PKB control variable table population. */
            controlVariableNames_.insert(std::make_pair(stmtNumber, variable));

            /* For PKB modifies table population. */
            modifies_[stmtNumber] = variable;

            break;

        case VARIABLE:
            /* For PKB variable table population. */
            variableNames_.insert(variable);

            /* For PKB uses table population. */
            uses_[stmtNumber].insert(variable);

            break;

        case CONSTANT:
            /* For PKB constant table population. */
            constants_.insert(variable);

            break;
    }
}

void DesignExtractor::setAssignExpressionData(StmtNumber stmtNumber, vector<string> expression) {
    /* For PKB expression tables population. */
    expressions_.insert(std::make_pair(stmtNumber, expression));
}

void DesignExtractor::precomputeCalls() {
    for (auto &pair : calls_) {
        ProcedureIndex procedureIndex = PKB::GetProcedureIndex(pair.first);

        for (ProcedureName procedureName : pair.second) {
            PKB::InsertCalls(procedureIndex, PKB::GetProcedureIndex(procedureName));
        }
    }

    PKB::PopulateCallsTransitiveTable();
}

void DesignExtractor::precomputeModifies() {
    /* Populate modifies that are procedure local into the modifies procedure table. */
    for (const auto &pair : modifies_) {
        VariableName variableName = pair.second;

        if (variableName.empty()) {
            continue;
        }

        for (auto iter = proceduresFirstStmt_.rbegin(); iter != proceduresFirstStmt_.rend(); iter++) {
            if (iter->first <= pair.first) {
                PKB::InsertModifiesProcedure(PKB::GetProcedureIndex(iter->second), PKB::GetVariableIndex(variableName));
                break;
            }
        }
    }

    /* Populate modifies that are from other procedures. */
    vector<ProcedureIndex> procedureIndexes = PKB::GetAllProcedureIndexes();
    for (const auto &procedureIndex : procedureIndexes) {
        set<ProcedureIndex> procedures = PKB::GetCalled(procedureIndex);

        /* Procedure does not have any call. */
        if (procedures.empty()) {
            continue;
        }

        for (const auto &index : procedures) {
            set<VariableIndex> variableIndexes = PKB::GetProcedureModifiedVariables(index);

            /* If called procedure does not modify anything. */
            if (variableIndexes.empty()) {
                continue;
            }

            for (VariableIndex variableIndex : variableIndexes) {
                PKB::InsertModifiesProcedure(procedureIndex, variableIndex);
            }
        }
    }

    for (auto iter = modifies_.begin(); iter != modifies_.end();) {
        unsigned int stmtNumber = iter->first;
        set<VariableIndex> variableIndexes;

        /* If call statement, get variables from modifies procedure table. */
        if (call_.count(stmtNumber) == 1) {
            ProcedureIndex procedureIndex = PKB::GetProcedureIndex(call_[stmtNumber]);
            variableIndexes = PKB::GetProcedureModifiedVariables(procedureIndex);

            /* If called procedure does not modify anything. */
            if (variableIndexes.empty()) {
                iter = modifies_.erase(iter);
                continue;
            }

        } else {
            variableIndexes.insert(PKB::GetVariableIndex(iter->second));
        }

        /* Populate container statements. */
        if (stmtsLevels_[stmtNumber] != 1) {
            unsigned int parentStmtNumber = getParentOfStmtNumber(stmtNumber);

            while (parentStmtNumber > 0) {
                for (VariableIndex variableIndex : variableIndexes) {
                    PKB::InsertModifies(parentStmtNumber, variableIndex);
                }

                parentStmtNumber = getParentOfStmtNumber(parentStmtNumber);
            }
        }

        /* Populate assign and call statements. */
        for (VariableIndex variableIndex : variableIndexes) {
            PKB::InsertModifies(stmtNumber, variableIndex);
        }

        iter++;
    }
}

void DesignExtractor::precomputeUses() {
    /* Populate uses that are procedure local into the uses procedure table. */
    for (const auto &pair : uses_) {
        set<string> variableNames = pair.second;

        for (auto iter = proceduresFirstStmt_.rbegin(); iter != proceduresFirstStmt_.rend(); iter++) {
            if (iter->first <= pair.first) {
                for (VariableName variableName : variableNames) {
                    PKB::InsertUsesProcedure(PKB::GetProcedureIndex(iter->second), PKB::GetVariableIndex(variableName));
                }

                break;
            }
        }
    }

    /* Populate uses that are from other procedures. */
    vector<ProcedureIndex> procedureIndexes = PKB::GetAllProcedureIndexes();
    for (const auto &procedureIndex : procedureIndexes) {
        set<ProcedureIndex> procedures = PKB::GetCalled(procedureIndex);

        /* Procedure does not have any call. */
        if (procedures.empty()) {
            continue;
        }

        for (const auto &index : procedures) {
            set<VariableIndex> variableIndexes = PKB::GetProcedureUsedVariables(index);

            if (variableIndexes.empty()) {
                continue;
            }

            for (VariableIndex variableIndex : variableIndexes) {
                PKB::InsertUsesProcedure(procedureIndex, variableIndex);
            }
        }
    }

    for (auto iter = uses_.begin(); iter != uses_.end();) {
        unsigned int stmtNumber = iter->first;
        set<VariableIndex> variableIndexes;

        /* If call statement, get variables from uses procedure table. */
        if (call_.count(stmtNumber) == 1) {
            ProcedureIndex procedureIndex = PKB::GetProcedureIndex(call_[stmtNumber]);
            variableIndexes = PKB::GetProcedureUsedVariables(procedureIndex);

            /* If called procedure does not use anything. */
            if (variableIndexes.empty()) {
                iter = uses_.erase(iter);
                continue;
            }

        } else {
            for (VariableName variableName : iter->second) {
                variableIndexes.insert(PKB::GetVariableIndex(variableName));
            }
        }

        /* Populate container statements. */
        if (stmtsLevels_[stmtNumber] != 1) {
            unsigned int parentStmtNumber = getParentOfStmtNumber(stmtNumber);

            while (parentStmtNumber > 0) {
                for (VariableIndex variableIndex : variableIndexes) {
                    PKB::InsertUses(parentStmtNumber, variableIndex);
                }

                parentStmtNumber = getParentOfStmtNumber(parentStmtNumber);
            }
        }

        /* Populate assign and call statements. */
        for (VariableIndex variableIndex : variableIndexes) {
            PKB::InsertUses(stmtNumber, variableIndex);
        }

        iter++;
    }
}

void DesignExtractor::precomputeParent() {
    for (unsigned int i = 1; i <= stmtsLevels_.size(); i++) {
        unsigned int parentStmtNumber = getParentOfStmtNumber(i);

        if (parentStmtNumber > 0) {
            PKB::InsertParent(parentStmtNumber, i);
        }
    }

    PKB::PopulateParentTransitiveTable();
}

void DesignExtractor::precomputeFollows() {
    for (unsigned int i = 1; i <= stmtsLevels_.size(); i++) {
        unsigned int stmtNumber = getFollowOfStmtNumber(i);

        if (stmtNumber > 0) {
            PKB::InsertFollows(stmtNumber, i);
        }
    }

    PKB::PopulateFollowsTransitiveTable();
}

void DesignExtractor::precomputeNext() {
    for (auto &pair : proceduresFirstStmt_) {
        set<unsigned int> visitedStmtNumbers;
        set<CFGNode> visitedNodes;

        queue<CFGNode> queue;

        CFGNode rootNode = new TreeNode(pair.first);
        queue.push(rootNode);

        visitedNodes.insert(rootNode);

        while (!queue.empty()) {
            CFGNode currentNode = queue.front();
            queue.pop();

            set<CFGNode> nextNodes;
            set<unsigned int> nextStmtNumbers = getNextStmtNumbers(currentNode->getStmtNumber());

            for (StmtNumber stmtNumber : nextStmtNumbers) {
                PKB::InsertNext(currentNode->getStmtNumber(), stmtNumber);

                CFGNode newNode = NULL;

                for (CFGNode visitedNode : visitedNodes) {
                    if (visitedNode->getStmtNumber() == stmtNumber) {
                        newNode = visitedNode;
                        break;
                    }
                }

                if (newNode == NULL) {
                    newNode = new TreeNode(stmtNumber);

                    queue.push(newNode);
                    nextNodes.insert(newNode);
                }

                currentNode->addChild(newNode);
            }

            visitedNodes.insert(nextNodes.begin(), nextNodes.end());
        }

        PKB::InsertControlFlowGraph(rootNode);
    }
}

int DesignExtractor::getParentOfStmtNumber(StmtNumber stmtNumber) {
    if (stmtNumber == 0) {
        return -1;
    }

    unsigned int stmtLevel = stmtsLevels_[stmtNumber];

    /* Root node level, no parent. */
    if (stmtLevel == 1) {
        return 0;
    }

    /* Search for the nearest (stmtLevel - 1) value; that, will be the parent. */
    for (unsigned int i = stmtNumber - 1; i > 0; i--) {
        if (stmtsLevels_[i] == (stmtLevel - 1)) {
            return i;
        }
    }

    return -1;
}

int DesignExtractor::getFollowOfStmtNumber(StmtNumber stmtNumber) {
    if (stmtNumber == 0) {
        return -1;
    }

    /* Get stmtNumber's procedure's first statement position. */
    unsigned int firstStmtNumber;
    for (auto iter = proceduresFirstStmt_.rbegin(); iter != proceduresFirstStmt_.rend(); iter++) {
        firstStmtNumber = iter->first;

        /* Nothing is before the first statement of a procedure. */
        if (firstStmtNumber == stmtNumber) {
            return 0;
        }

        if (firstStmtNumber < stmtNumber) {
            break;
        }
    }

    unsigned int stmtLevel = stmtsLevels_[stmtNumber];

    for (unsigned int i = stmtNumber - 1; i >= firstStmtNumber; i--) {
        if (stmtsLevels_[i] == (stmtLevel - 1)) {
            return 0;
        }

        if (stmtsLevels_[i] == stmtLevel) {
            /* Catch "if" statement being in same level but different statement list. */
            for (auto &pair : thenLastStmt_) {
                if (pair.second == i) {
                    return 0;
                }
            }

            return i;
        }
    }

    return 0;
}

int DesignExtractor::getFollowingOfStmtNumber(StmtNumber stmtNumber) {
    if (stmtNumber == 0) {
        return -1;
    }

    /* Get stmtNumber's procedure's last statement position. */
    unsigned int lastStmtNumber;
    for (auto &pair : proceduresLastStmt_) {
        lastStmtNumber = pair.first;

        /* Nothing is after the last statement of a procedure. */
        if (lastStmtNumber == stmtNumber) {
            return 0;
        }

        if (stmtNumber < lastStmtNumber) {
            break;
        }
    }

    unsigned int stmtLevel = stmtsLevels_[stmtNumber];

    for (unsigned int i = stmtNumber + 1; i <= lastStmtNumber; i++) {
        if (stmtsLevels_[i] == (stmtLevel - 1)) {
            return 0;
        }

        if (stmtsLevels_[i] == stmtLevel) {
            /* Catch "if" statement being in same level but different statement list. */
            for (auto &pair : elseFirstStmt_) {
                if (pair.second == i) {
                    return 0;
                }
            }

            return i;
        }
    }

    return 0;
}

set<unsigned int> DesignExtractor::getNextStmtNumbers(unsigned int stmtNumber) {
    set<unsigned int> stmtNumbers;

    switch (stmts_[stmtNumber]) {
        default:
            break;

        case WHILE:
            stmtNumbers.insert(stmtNumber + 1);
            break;

        case IF:
            stmtNumbers.insert(stmtNumber + 1);
            stmtNumbers.insert(elseFirstStmt_[stmtNumber]);

            /* Don't need check for following or parent flow path. */
            return stmtNumbers;
    }

    while (true) {
        int followingStmtNumber = getFollowingOfStmtNumber(stmtNumber);

        /* While, assign and call not at the end of flow path is caught here.*/
        if (followingStmtNumber > 0) {
            stmtNumbers.insert(followingStmtNumber);
            return stmtNumbers;
        }

        int parentStmtNumber = getParentOfStmtNumber(stmtNumber);

        /* While, assign and call at root level is caught here. */
        if (parentStmtNumber < 1) {
            return stmtNumbers;
        }

        /* While, assign and call at the end of flow path is caught here. */
        switch (stmts_[parentStmtNumber]) {
            default:
                break;

            case WHILE:
                stmtNumbers.insert(parentStmtNumber);
                return stmtNumbers;

            /* Loop up to parent to look for next flow path. */
            case IF:
                stmtNumber = parentStmtNumber;
                break;
        }
    }
}

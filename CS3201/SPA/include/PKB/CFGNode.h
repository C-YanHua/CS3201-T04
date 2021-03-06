#pragma once

#include <string>
#include <vector>

#include "Constants.h"

class CFGNode {
 public:
    CFGNode(Symbol symbol);
    CFGNode(unsigned int stmtNumber);
    CFGNode(Symbol symbol, unsigned int stmtNumber);
    CFGNode(Symbol symbol, unsigned int stmtNumber, unsigned int stmtLevel);
    ~CFGNode();

    void setSymbol(Symbol symbol);
    Symbol getSymbol();

    void setStmtNumber(unsigned int stmtNumber);
    unsigned int getStmtNumber();

    void setStmtLevel(unsigned int stmtLevel);
    unsigned int getStmtLevel();

    void setWhileNext(unsigned int whileNext);
    unsigned int getWhileNext();

    void setVisited(bool visited);
    bool isVisited();

    void addChild(CFGNode* child);
    std::vector<CFGNode*> getChildren();
    unsigned int getNumberOfChildren();

    void addParent(CFGNode* parent);
    std::vector<CFGNode*> getParents();
    unsigned int getNumberOfParents();

    void setModify(unsigned int modify);
    unsigned int getModify();

    void setModifies(std::vector<unsigned int> modifies);
    std::vector<unsigned int> getModifies();

    void setUses(std::vector<unsigned int> uses);
    std::vector<unsigned int> getUses();

    void printCFGNode(unsigned int indent);

 private:
    Symbol symbol_;
    unsigned int stmtNumber_;
    unsigned int stmtLevel_;
    unsigned int whileNext_;
    bool visited_;

    std::vector<CFGNode*> children_;
    std::vector<CFGNode*> parents_;

    unsigned int modify_;
    std::vector<unsigned int> modifies_;
    std::vector<unsigned int> uses_;
};

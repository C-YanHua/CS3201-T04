#include <iomanip>
#include <iostream>
#include <string>

#include "Constants.h"
#include "Tree.h"

Tree::Tree() {}

Tree::~Tree() {}

void Tree::setRoot(TreeNode *root) {
    this->root_ = root;
}

TreeNode* Tree::getRoot() {
    return root_;
}

TreeNode* Tree::createNode(Symbol symbol) {
    return new TreeNode(symbol);
}

TreeNode* Tree::createNode(Symbol symbol, std::string value) {
    return new TreeNode(symbol, value);
}

TreeNode* Tree::createNode(Symbol symbol, int lineNumber) {
    return new TreeNode(symbol, lineNumber);
}

TreeNode* Tree::createNode(Symbol symbol, int lineNumber, std::string value) {
    return new TreeNode(symbol, lineNumber, value);
}

void Tree::printTree() {
    printTreeNode(root_, 0);
}

void Tree::printTreeNode(TreeNode* node, unsigned int indent) {
    if (node != NULL) {
        std::cout << std::setw(indent) << ' ';

        if (node->getLineNumber() != -1) {
            std::cout << node->getLineNumber() << ". ";
        }

        if (node->getValue() != "") {
            std::cout << node->getValue() << ':';
        }

        std::cout << Constants::SymbolToString(node->getSymbol()) << std::endl;
        for (unsigned int i = 0; i < node->getChildren().size(); i++) {
            printTreeNode(node->getChildren()[i], (indent + 4));
        }
    }
}

#include <string>
#include <vector>

#include "Constants.h"
#include "PKB/AST.h"
#include "PKB/PKB.h"
#include "TreeNode.h"
#include "PKB/Table.h"

using std::string;
using std::vector;

PKB::PKB() {}

PKB::~PKB() {}

void PKB::SetASTRoot(TreeNode *root) {
    AST::getInstance()->setRoot(root);
}

TreeNode* PKB::GetASTRoot() {
    return AST::getInstance()->getRoot();
}

TreeNode* PKB::CreateASTNode(Symbol symbol) {
    return AST::getInstance()->createNode(symbol);
}

TreeNode* PKB::CreateASTNode(Symbol symbol, string value) {
    return AST::getInstance()->createNode(symbol, value);
}

TreeNode* PKB::CreateASTNode(Symbol symbol, int lineNumber) {
    return AST::getInstance()->createNode(symbol, lineNumber);
}

TreeNode* PKB::CreateASTNode(Symbol symbol, int lineNumber, string value) {
    return AST::getInstance()->createNode(symbol, lineNumber, value);
}

void PKB::PrintASTTree() {
    AST::getInstance()->printTree();
}

void PKB::InsertVariable(string variable_name, int line_number) {
  PKB::variable_table.insert(variable_name, line_number);
}

void PKB::InsertVariable(string variable_name, vector<int> line_numbers) {
  for (auto line_number : line_numbers) {
    PKB::InsertVariable(variable_name, line_number);
  }
}
// TODO(pixelducky): update this impl
bool PKB::IsVariableExist(string variable_name) {
}

void PKB::InsertProcedure(string procedure_name, int line_number) {
  PKB::procedure_table.insert(procedure_name, line_number);
}

void PKB::InsertProcedure(string procedure_name, vector<int> line_numbers) {
  for (auto line_number : line_numbers) {
    PKB::InsertProcedure(procedure_name, line_number);
  }
}
// TODO(pixelducky): update this impl
bool PKB::IsProcedureExist(string procedure_name) {
  return false;
}

void PKB::InsertUses(int line_number, string variable_name) {
  PKB::uses_table.insert(line_number, variable_name);
}

void PKB::InsertUses(int line_number, vector<string> variable_names) {
  for (auto variable_name : variable_names) {
    PKB::InsertUses(line_number, variable_name);
  }
}

void PKB::InsertUses(string procedure_name, string variable_name) {
  PKB::procedure_uses_table.insert(procedure_name, variable_name);
}

void PKB::InsertUses(string procedure_name, vector<string> variable_names) {
  for (auto variable_name : variable_names) {
    PKB::InsertUses(procedure_name, variable_name);
  }
}

vector<string> PKB::UsesVariable(int line_number) {
  return PKB::uses_table.getValues(line_number);
}

vector<string> PKB::UsesVariable(string procedure_name) {
  return PKB::procedure_uses_table.getValues(procedure_name);
}

vector<int> PKB::UsedByLineNumber(string variable_name) {
  return PKB::uses_table.getKeys(variable_name);
}

vector<string> PKB::UsedByProcedure(string variable_name) {
  return PKB::procedure_uses_table.getKeys(variable_name);
}

void PKB::InsertModifies(int line_number, string variable_name) {
  PKB::modifies_table.insert(line_number, variable_name);
}

void PKB::InsertModifies(int line_number, vector<string> variable_names) {
  for (auto variable_name : variable_names) {
    PKB::InsertModifies(line_number, variable_name);
  }
}

void PKB::InsertModifies(string procedure_name, string variable_name) {
  PKB::procedure_modifies_table.insert(procedure_name, variable_name);
}

void PKB::InsertModifies(string procedure_name, vector<string> variable_names) {
  for (auto variable_name : variable_names) {
    PKB::InsertModifies(procedure_name, variable_name);
  }
}

vector<string> PKB::ModifiesVariable(int line_number) {
  return PKB::modifies_table.getValues(line_number);
}

vector<string> PKB::ModifiesVariable(string procedure_name) {
  return PKB::procedure_modifies_table.getValues(procedure_name);
}

vector<int> PKB::ModifiedByLineNumber(string variable_name) {
  return PKB::modifies_table.getKeys(variable_name);
}

vector<string> PKB::ModifiedByProcedure(string variable_name) {
  return PKB::procedure_modifies_table.getKeys(variable_name);
}

void PKB::InsertCalls(string procedure_name1, string procedure_name2) {
  PKB::calls_table.insert(procedure_name1, procedure_name2);
}

void PKB::InsertCalls(string procedure_name1, vector<string> procedure_names) {
  for (auto procedure_name2 : procedure_names) {
    PKB::InsertCalls(procedure_name1, procedure_name2);
  }
}

void PKB::InsertCallsTransitive(string procedure_name1, string procedure_name2) {
  PKB::calls_transitive_table.insert(procedure_name1, procedure_name2);
}

void PKB::InsertCallsTransitive(string procedure_name1, vector<string> procedure_names) {
  for (auto procedure_name2 : procedure_names) {
    PKB::InsertCallsTransitive(procedure_name1, procedure_name2);
  }
}
// TODO(pixelducky): update this impl
bool PKB::IsCalls(string procedure_name1, string procedure_name2) {
  return false;
}
// TODO(pixelducky): update this impl
bool PKB::IsCallsTransitive(string procedure_name1, string procedure_name2) {
  return false;
}

void PKB::InsertParent(int line_number1, int line_number2) {
  PKB::parent_table.insert(line_number1, line_number2);
}

void PKB::InsertParent(int line_number1, vector<int> line_numbers) {
  for (auto line_number2 : line_numbers) {
    PKB::InsertParent(line_number1, line_number2);
  }
}

void PKB::InsertParentTransitive(int line_number1, int line_number2) {
  PKB::parent_transitive_table.insert(line_number1, line_number2);
}

void PKB::InsertParentTransitive(int line_number1, vector<int> line_numbers) {
  for (auto line_number2 : line_numbers) {
    PKB::InsertParentTransitive(line_number1, line_number2);
  }
}
// TODO(pixelducky): update this impl
bool PKB::IsParent(int line_number1, int line_number2) {
  return false;
}
// TODO(pixelducky): update this impl
bool PKB::IsParent(int line_number1, vector<int> line_numbers) {
  return false;
}
// TODO(pixelducky): update this impl
bool PKB::IsParentTransitive(int line_number1, int line_number2) {
  return false;
}
// TODO(pixelducky): update this impl
bool PKB::IsParentTransitive(int line_number1, vector<int> line_numbers) {
  return false;
}

void PKB::InsertFollows(int line_number1, int line_number2) {
  PKB::follows_table.insert(line_number1, line_number2);
}

void PKB::InsertFollows(int line_number1, vector<int> line_numbers) {
  for (auto line_number2 : line_numbers) {
    PKB::InsertFollows(line_number1, line_number2);
  }
}

void PKB::InsertFollowsTransitive(int line_number1, int line_number2) {
  PKB::follows_transitive_table.insert(line_number1, line_number2);
}

void PKB::InsertFollowsTransitive(int line_number1, vector<int> line_numbers) {
  for (auto line_number2 : line_numbers) {
    PKB::InsertFollowsTransitive(line_number1, line_number2);
  }
}
// TODO(pixelducky): update this impl
bool PKB::IsFollows(int line_number1, int line_number2) {
  return false;
}
// TODO(pixelducky): update this impl
bool PKB::IsFollows(int line_number1, vector<int> line_numbers) {
  return false;
}
// TODO(pixelducky): update this impl
bool PKB::IsFollowsTransitive(int line_number1, int line_number2) {
  return false;
}
// TODO(pixelducky): update this impl
bool PKB::IsFollowsTransitive(int line_number1, vector<int> line_numbers) {
  return false;
}

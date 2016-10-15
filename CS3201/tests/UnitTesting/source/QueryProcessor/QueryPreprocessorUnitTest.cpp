#include <string>
#include <vector>
#include <unordered_map>

#include "stdafx.h"
#include "targetver.h"

#include "Constants.h"
#include "QueryProcessor/QueryPreprocessor.h"
#include "QueryProcessor/RelationTable.h"
#include "Utils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTesting {
    TEST_CLASS(QueryPreprocessorTest) {
public:
    TEST_METHOD(QueryParserSelect) {
        std::string expected, actual, query;
        QueryPreprocessor qp;
        QueryTree qt;

        query = "assign a,a1; while w1,w2;Select a";
        try {
            qp.preprocessQuery(query);
        }
        catch (std::exception& ex) {
            actual = ex.what();
        }
        qt = qp.getQueryTree();
        std::vector<std::string> varList;

        varList = qt.getResults();
        expected = "a ";
        for (unsigned int i = 0; i < varList.size(); i++) {
            actual += varList[i] + " ";
        }
        Assert::AreEqual(expected, actual);
    }
    TEST_METHOD(QueryParserMultipleAnd) {
        std::string expected, actual, query;
        QueryPreprocessor qp;
        QueryTree qt;

        query = "assign a1,a2,a3; stmt s1,s2,s3; variable v1,v2,v3;";
        query += "Select <s1,s2,v2> such that Uses(s3,v1) and Modifies(s3,\"x\") and ";
        query += "Follows(s1,s2) and Parent(s3,s1) such that Uses(s2,v1) and Uses(5,\"y\") and ";
        query += "Follows(3,4) pattern a1(v2,\"x+y\") and a3(\"z\",_) such that Modifies(a3,v3)";
        try {
            qp.preprocessQuery(query);
        }
        catch (std::exception& ex) {
            actual = ex.what();
        }
        qt = qp.getQueryTree();
        std::vector<std::string> varList;

        varList = qt.getResults();
        for (unsigned int i = 0; i < varList.size(); i++) {
            actual += varList[i] + " ";
        }
        std::vector<Clause> clauseList = qt.getClauses();

        expected = "s1 s2 v2 ";
        expected += "Uses s3 v1 Modifies s3 \"x\" Follows s1 s2 Parent s3 s1 Uses s2 v1 Uses 5 \"y\" ";
        expected += "Follows 3 4 Modifies a3 v3 ";
        expected += "pattern a1 v2 pattern a3 \"z\" ";
        for (Clause c : clauseList) {
            actual += c.getClauseType() + " ";
            actual += c.getArg()[0] + " ";
            actual += c.getArg()[1] + " ";
        }
        Assert::AreEqual(expected, actual);
    }
    TEST_METHOD(QueryParserSuchThat) {
        std::string expected, actual;
        QueryPreprocessor qp;
        QueryTree qt;

        std::string query;
        query = "assign a,a1; while w1,w2; variable x;";
        query += "Select a such that Uses(a1, x)";
        try {
            qp.preprocessQuery(query);
        }
        catch (std::exception& ex) {
            actual = ex.what();
        }
        qt = qp.getQueryTree();
        std::vector<Clause> resList;

        resList = qt.getClauses();
        expected = "Uses a1 x ";
        for (unsigned int i = 0; i < resList.size(); i++) {
            actual += resList[i].getClauseType() + " ";
            for (unsigned int j = 0; j < resList[i].getArg().size(); j++) {
                actual += resList[i].getArg()[j] + " ";
            }
        }
        Assert::AreEqual(expected, actual);
    }
    TEST_METHOD(QueryParserPatternSelectTwo) {
        std::string expected, actual;
        QueryPreprocessor qp;
        QueryTree qt;

        std::string query;
        query = "assign a,a1;";
        query += "Select a pattern a1(\"x\",_)";
        try {
            qp.preprocessQuery(query);
        }
        catch (std::exception& ex) {
            actual = ex.what();
        }
        qt = qp.getQueryTree();
        std::vector<Clause> resList;

        resList = qt.getClauses();
        expected = "pattern a1 \"x\" _ ";
        for (unsigned int i = 0; i < resList.size(); i++) {
            actual += resList[i].getClauseType() + " ";
            for (unsigned int j = 0; j < resList[i].getArg().size(); j++) {
                actual += resList[i].getArg()[j] + " ";
            }
        }
        Assert::AreEqual(expected, actual);
    }
    TEST_METHOD(QueryParserSuchThatWithStringConstant) {
        std::string expected, actual;
        QueryPreprocessor qp;
        QueryTree qt;

        std::string query;
        query = "assign a,a1; variable x;";
        query += "Select a such that Uses(a1, \"x\")";
        try {
            qp.preprocessQuery(query);
        }
        catch (std::exception& ex) {
            actual = ex.what();
        }
        qt = qp.getQueryTree();
        std::vector<Clause> resList;

        resList = qt.getClauses();
        expected = "Uses a1 \"x\" ";
        for (unsigned int i = 0; i < resList.size(); i++) {
            actual += resList[i].getClauseType() + " ";
            for (unsigned int j = 0; j < resList[i].getArg().size(); j++) {
                actual += resList[i].getArg()[j] + " ";
            }
        }
        Assert::AreEqual(expected, actual);
    }
    TEST_METHOD(QueryParserSuchThatWithNumericConstant) {
        std::string expected, actual;
        QueryPreprocessor qp;
        QueryTree qt;

        std::string query;
        query = "assign a,a1; variable x;";
        query += "Select a such that Uses(1, x)";
        try {
            qp.preprocessQuery(query);
        }
        catch (std::exception& ex) {
            actual = ex.what();
        }
        qt = qp.getQueryTree();
        std::vector<Clause> resList;

        resList = qt.getClauses();
        expected = "Uses 1 x ";
        for (unsigned int i = 0; i < resList.size(); i++) {
            actual += resList[i].getClauseType() + " ";
            for (unsigned int j = 0; j < resList[i].getArg().size(); j++) {
                actual += resList[i].getArg()[j] + " ";
            }
        }
        Assert::AreEqual(expected, actual);
    }
    TEST_METHOD(QueryParserInvalidQuery) {
        std::string expected, actual;
        QueryPreprocessor qp;
        QueryTree qt;

        bool isValid;

        std::string query;
        query = "assign a,a1; variable x;";
        query += "Select a such that uses(\"1a\", x)";

        try {
            qp.preprocessQuery(query);
            isValid = true;
        }
        catch (std::exception& ex) {
            isValid = false;
        }
        Assert::AreEqual(isValid, false);
    }
    TEST_METHOD(QueryParserSelectOneVars) {
        std::string expected, actual, query;
        QueryPreprocessor qp;
        QueryTree qt;

        query = "assign a,a1; while w1,w2;Select a";
        try {
            qp.preprocessQuery(query);
        }
        catch (std::exception& ex) {
            actual = ex.what();
        }
        qt = qp.getQueryTree();
        std::vector<std::string> varList;

        varList = qt.getResults();
        expected = "a ";
        for (unsigned int i = 0; i < varList.size(); i++) {
            actual += varList[i] + " ";
        }
        Assert::AreEqual(expected, actual);
    }
    TEST_METHOD(QueryParserSuchThatTwoVars) {
        std::string expected, actual;
        QueryPreprocessor qp;
        QueryTree qt;

        std::string query;
        query = "assign a,a1; variable x;";
        query += "Select a such that Uses(a1, x)";
        try {
            qp.preprocessQuery(query);
        }
        catch (std::exception& ex) {
            actual = ex.what();
        }
        qt = qp.getQueryTree();
        std::vector<Clause> resList;

        resList = qt.getClauses();
        expected = "Uses a1 x ";
        for (unsigned int i = 0; i < resList.size(); i++) {
            actual += resList[i].getClauseType() + " ";
            for (unsigned int j = 0; j < resList[i].getArg().size(); j++) {
                actual += resList[i].getArg()[j] + " ";
            }
        }
    }
    TEST_METHOD(QueryParserPattern) {
        std::string expected, actual;
        QueryPreprocessor qp;
        QueryTree qt;

        std::string query;
        query = "assign a,a1;";
        query += "Select a pattern a1(\"x\",_)";
        try {
            qp.preprocessQuery(query);
        }
        catch (std::exception& ex) {
            actual = ex.what();
        }
        qt = qp.getQueryTree();
        std::vector<Clause> resList;

        resList = qt.getClauses();
        expected = "pattern a1 \"x\" _ ";
        for (unsigned int i = 0; i < resList.size(); i++) {
            actual += resList[i].getClauseType() + " ";
            for (unsigned int j = 0; j < resList[i].getArg().size(); j++) {
                actual += resList[i].getArg()[j] + " ";
            }
        }
        Assert::AreEqual(expected, actual);
    }
    TEST_METHOD(ExtractQueryTreeOneClause) {
        std::string query = "assign a; Select a pattern a(\"a\", _)";
        QueryPreprocessor queryPreprocessor = QueryPreprocessor();
        queryPreprocessor.preprocessQuery(query);
        QueryTree queryTree = queryPreprocessor.getQueryTree();

        std::string actual, expected;

        std::unordered_map<std::string, Symbol> varMap = queryTree.getVarMap();
        for (auto kv : varMap) {
            actual += Constants::SymbolToString(kv.second) + " " + kv.first + "; ";
        }

        std::vector<std::string> selectList = queryTree.getResults();
        for (auto varName : selectList) {
            actual += Constants::SymbolToString(varMap[varName]) + " " + varName + "; ";
        }

        std::vector<Clause> clauses = queryTree.getClauses();
        for (Clause c : clauses) {
            actual += c.getClauseType() + " ";
            for (int i = 0; i < c.getArgCount(); i++) {
                actual += c.getArg()[i] + " ";
            }
        }

        expected = "assign a; assign a; pattern a \"a\" _ ";  // declaration; select a; pattern
        Assert::AreEqual(expected, actual);
    }
    TEST_METHOD(ExtractQueryTreeTwoClauses) {
        std::string query = "assign a, a1; Select a pattern a(\"e\", _) such that Follows(a, a1)";
        QueryPreprocessor queryPreprocessor = QueryPreprocessor();
        queryPreprocessor.preprocessQuery(query);
        QueryTree queryTree = queryPreprocessor.getQueryTree();

        std::string actual, expected;

        std::unordered_map<std::string, Symbol> varMap = queryTree.getVarMap();
        for (auto kv : varMap) {
            actual += Constants::SymbolToString(kv.second) + " " + kv.first + "; ";
        }

        std::vector<std::string> selectList = queryTree.getResults();
        for (auto varName : selectList) {
            actual += Constants::SymbolToString(varMap[varName]) + " " + varName + "; ";
        }

        std::vector<Clause> clauses = queryTree.getClauses();
        for (Clause c : clauses) {
            actual += c.getClauseType() + " ";
            for (int i = 0; i < c.getArgCount(); i++) {
                actual += c.getArg()[i] + " ";
            }
        }

        expected = "assign a; assign a1; assign a; Follows a a1 pattern a \"e\" _ ";  // declaration; select a; Follows
        Assert::AreEqual(actual, expected);
    }
    };
}   // namespace UnitTest
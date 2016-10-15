#include <string>
#include <vector>
#include <stdlib.h>

#include "stdafx.h"
#include "targetver.h"

#include "Exceptions.h"
#include "Frontend/FrontendParser.h"
#include "PKB/PKB.h"
#include "QueryProcessor/QueryPreprocessor.h"
#include "Constants.h"
#include "QueryProcessor/QueryEvaluator.h"
#include "QueryProcessor/QueryOptimizer.h"

using std::string;
using std::vector;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SystemTesting {
	TEST_CLASS(SystemTest) {
public:
	void parse(string filePath) {
		FrontendParser frontendParser = FrontendParser();

		try {
			frontendParser.parseProgram(filePath);

		}
		catch (std::exception& ex) {
			Logger::WriteMessage("Parse Error");
			Logger::WriteMessage(ex.what());
			exit(EXIT_FAILURE);
		}
	}
	void getSampleProgram() {
		string fileName = "..\\tests\\IntegrationTesting\\Integration-Test-Source1.txt";
        PKB::Clear();
		parse(fileName);
	}
	QueryTree getQueryTree(std::string query) {
		QueryPreprocessor qp;
		qp.preprocessQuery(query);
		QueryTree qt(qp.getQueryTree());
		return qt;
	}
	std::vector<std::string> resultToString(ResultList &result) {
		std::vector<std::vector<std::string>> &list = (*result.begin()).second;
		std::vector<std::string> res;
		for (std::vector<std::string> combi : list) {
			res.push_back(Utils::VectorToString(combi));
		}
		return res;
	}

    TEST_METHOD(Integration_Parser_and_PKB) {
        getSampleProgram();
        Assert::AreEqual(42, int(PKB::GetNumberOfAssign()));
        Assert::AreEqual(0, int(PKB::GetNumberOfCall()));
        Assert::AreEqual(0, int(PKB::GetNumberOfIf()));
        Assert::AreEqual(1, int(PKB::GetNumberOfProcedure()));
        Assert::AreEqual(7, int(PKB::GetNumberOfWhile()));
    }
	TEST_METHOD(Integration_QE_GetCandidatesTest) {
		getSampleProgram();
		QueryTree qt(getQueryTree("variable v; Select v;"));
		
		QueryEvaluator qe;
		//std::unordered_map<std::string, Symbol> map(qt.getSelect());

		std::vector<std::string> result = resultToString(qe.selectQueryResults(qt));
		Logger::WriteMessage(Utils::VectorToString(result).c_str());
	}
	TEST_METHOD(TestGetCandidates) {
		getSampleProgram();
		QueryTree qt(getQueryTree("stmt a; Select a such that Modifies(a, \"a\")"));
		QueryEvaluator qe;

		TotalCombinationList total(qe.getTotalCandidateList(qt));
		Logger::WriteMessage(total.toString().c_str());
		for (unsigned i : PKB::GetSymbolStmtNumbers(STMT)) {
			Logger::WriteMessage(std::to_string(i).c_str());
		}

		/**
		std::string actual(Utils::VectorToString(qe.selectQueryResults(qt)));
		std::string expected("<<1>,<8>,<9>,<10>,<16>,<23>,<29>>");
		**/
		//Assert::AreEqual(expected, actual);
	}
	TEST_METHOD(TestEvaluateModifies) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; Select a such that Modifies(a, \"a\")"));
		QueryEvaluator qe;

		std::string actual(Utils::VectorToString(resultToString(qe.selectQueryResults(qt))));
		std::string expected("<<1>,<8>,<9>,<10>,<16>,<23>,<29>>");
		Assert::AreEqual(expected, actual);
	}
	TEST_METHOD(TestEvaluateFollows) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; while w; Select a such that Follows(a, w)"));
		QueryEvaluator qe;
		
		std::string actual(Utils::VectorToString(resultToString(qe.selectQueryResults(qt))));
		std::string expected("<<5>,<11>,<29>,<32>,<34>,<37>>");
		Assert::AreEqual(expected, actual);
	}
	TEST_METHOD(TestEvaluateFollowsWithConst) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; while w; Select w such that Follows(3, a)"));
		QueryEvaluator qe;

		std::string actual(Utils::VectorToString(resultToString(qe.selectQueryResults(qt))));
		std::string expected("<<6>,<7>,<12>,<30>,<33>,<35>,<38>>");
		Assert::AreEqual(expected, actual);
	}
	TEST_METHOD(TestGetModifies) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; variable v; Select v such that Modifies(1, v)"));
		QueryEvaluator qe;

		std::string actual(Utils::VectorToString(resultToString(qe.selectQueryResults(qt))));
		std::string expected("<<a>>");
		Assert::AreEqual(expected, actual);
	}
	TEST_METHOD(TestGetUses) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; variable v; Select a such that Uses(a, \"a\")"));
		QueryEvaluator qe;

		std::string actual(Utils::VectorToString(resultToString(qe.selectQueryResults(qt))));
		std::string expected("<<10>,<21>,<22>,<36>,<42>,<44>,<46>>");
		Assert::AreEqual(expected, actual);
	}
	TEST_METHOD(TestExactPattern) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; variable v; Select a pattern a(\"c\",\"c\")"));
		QueryEvaluator qe;

		std::string actual(Utils::VectorToString(resultToString(qe.selectQueryResults(qt))));
		std::string expected("<<2>>");
		Assert::AreEqual(expected, actual);
	}
	TEST_METHOD(TestExactRHSPattern) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; variable v; Select a pattern a(_,\"c\")"));
		QueryEvaluator qe;

		std::string actual(Utils::VectorToString(resultToString(qe.selectQueryResults(qt))));
		std::string expected("<<2>,<16>>");
		Assert::AreEqual(expected, actual);
	}
	TEST_METHOD(TestUnderscorePattern) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; variable v; Select a pattern a(\"c\",_)"));
		QueryEvaluator qe;

		std::string actual(Utils::VectorToString(resultToString(qe.selectQueryResults(qt))));
		std::string expected("<<2>,<3>,<5>,<24>,<26>,<41>,<48>>");
		Assert::AreEqual(expected, actual);
	}
	TEST_METHOD(TestSubPattern) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; variable v; Select a pattern a(_, _\"c\"_)"));
		QueryEvaluator qe;

		std::string actual(Utils::VectorToString(resultToString(qe.selectQueryResults(qt))));
		std::string expected("<<2>,<4>,<10>,<16>,<18>,<37>,<42>,<47>>");
		Assert::AreEqual(expected, actual);
	}
	TEST_METHOD(TestEvaluateExactPattern) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; variable v; Select a pattern a(_, \"c\")"));
		QueryEvaluator qe;

		bool f(qe.evaluatePatternClause("2", "c", "\"c\""));
		Assert::IsTrue(f);
	}
	TEST_METHOD(TestEvaluateSubPattern) {
		getSampleProgram();
		QueryTree qt(getQueryTree("assign a; variable v; Select a pattern a(_, _\"c\"_)"));
		QueryEvaluator qe;

		Assert::IsTrue(qe.evaluatePatternClause("37", "e", "_\"c\"_"));
	}
    TEST_METHOD(TestSubPatternConstant) {
        getSampleProgram();
        QueryTree qt(getQueryTree("assign a; Select a pattern a(_, _\"97\"_)"));
        QueryEvaluator qe;
    
        Assert::IsTrue(qe.evaluatePatternClause("1", "_", "_\"97\"_"));
    }
    TEST_METHOD(TestExactPatternConstant) {
        getSampleProgram();
        QueryTree qt(getQueryTree("assign a; Select a pattern a(_, \"42\")"));
        QueryEvaluator qe;

        Assert::IsTrue(qe.evaluatePatternClause("25", "_", "\"42\""));
    }
    TEST_METHOD(QueryOptimizerTestOne) {
        std::string query, expectedBooleanClauses, actualBooleanClauses;
        std::string expectedUnselectedClauses, actualUnselectedClauses;
        std::string expectedSelectedClauses, actualSelectedClauses;

        string fileName = "..\\tests\\IntegrationTesting\\IntegrationTest-OptimizerSource.txt";
        PKB::Clear();
        parse(fileName);

        QueryPreprocessor qp;
        QueryOptimizer qo;
        QueryTree qt;

        query = "assign a1,a2,a3,a4; stmt s1,s2,s3,s4; variable v1,v2,v3,v4,v5;";
        query += "Select <s1, s2, v2> such that Follows*(s1, s2) and Parent(s3, s1) and Uses(s2, v1) ";
        query += "and Uses(s3, v1) such that Uses(5, \"y\") such that Follows(3, 4) such that Uses(a3, v4) ";
        query += "such that Modifies(s3, \"x\") and Follows(s2, 3) pattern a1(v2, _\"x+y\"_) and a3(\"z\", _) ";
        query += "such that Uses(s4, v3) and Uses(s4, v4) and Modifies(a3, v3)";

        try {
            qp.preprocessQuery(query);
        }
        catch (std::exception& ex) {
            Logger::WriteMessage(ex.what());
        }

        qt = qo.optimize(qp.getQueryTree());

        for (Clause c : qt.getBooleanClauses()) {
            actualBooleanClauses += c.toString() + " ";
        }

        std::vector<Clause> unselectedGroup = qt.getUnselectedGroups().at(0);
        for (Clause c : unselectedGroup) {
            actualUnselectedClauses += c.toString() + " ";
        }

        //testing for group 1 only, ignore pattern a1(v2,_"x+y"_) in group 2
        std::vector<Clause> selectedGroup = qt.getSelectedGroups().at(0);
        for (Clause c : selectedGroup) {
            actualSelectedClauses += c.toString() + " ";
        }

        expectedBooleanClauses = "Uses(5,\"y\") Follows(3,4) ";
        expectedUnselectedClauses = "pattern a3(\"z\",_) Modifies(a3,v3) Uses(a3,v4) Uses(s4,v4) Uses(s4,v3) ";
        expectedSelectedClauses = "Follows(s2,3) Modifies(s3,\"x\") Parent(s3,s1) Uses(s2,v1) Uses(s3,v1) Follows*(s1,s2) ";

        Assert::AreEqual(expectedBooleanClauses, actualBooleanClauses);
        Assert::AreEqual(expectedUnselectedClauses, actualUnselectedClauses);
        Assert::AreEqual(expectedSelectedClauses, actualSelectedClauses);
    }
	};
}

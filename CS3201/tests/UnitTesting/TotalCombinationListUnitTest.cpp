#include <string>
#include <vector>
#include <sstream>

#include "stdafx.h"
#include "targetver.h"

#include "Constants.h"
#include "Utils.h"
#include "QueryProcessor/QueryEvaluator.h"
#include "QueryProcessor/QueryUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTesting {
	TEST_CLASS(TotalCombinationListTest) {
public:
	std::string PartialToString(PartialCombinationList &partialCombi) {
		std::string actual("<");
		PartialCombinationList::iterator it(partialCombi.begin());
		while (it != partialCombi.end()) {
			std::string str = Utils::MapToString(*it);
			actual.append(str);
			it++;
			if (it != partialCombi.end()) {
				actual.append(",");
			}
		}
		actual.append(">");
		return actual;
	}

	TotalCombinationList getSampleList() {
		CandidateCombination candidateCombi1({ { "a", "1" },{ "b", "2" } });
		CandidateCombination candidateCombi2({ { "a", "1" },{ "b", "3" } });
		PartialCombinationList partialCombi1{ candidateCombi1, candidateCombi2 };

		CandidateCombination candidateCombi3({ { "c", "4" } });
		CandidateCombination candidateCombi4({ { "c", "5" } });
		PartialCombinationList partialCombi2{ candidateCombi3, candidateCombi4 };

		TotalCombinationList totalCombi;

		totalCombi.addSynonym("a", partialCombi1);
		totalCombi.addSynonym("b", 0);
		totalCombi.addSynonym("c", partialCombi2);

		return totalCombi;
	}
	TEST_METHOD(ReduceSingleFactorTest) {

		TotalCombinationList totalCombi(getSampleList());
		std::vector<Synonym> synList{ "a" };
		totalCombi.reduceSingleFactor(synList, totalCombi["a"]);

		std::string actual(PartialToString(totalCombi["a"]));
		std::string expected("<<a:1>>");
		Assert::AreEqual(expected, actual);
	}

	TEST_METHOD(CartesianProductTest) {

		TotalCombinationList totalCombi(getSampleList());
		PartialCombinationList product(totalCombi.cartesianProduct(totalCombi["a"], totalCombi["c"]));

		std::string actual(PartialToString(product));
		std::string expected("<<a:1,b:2,c:4>,<a:1,b:2,c:5>,<a:1,b:3,c:4>,<a:1,b:3,c:5>>");
		Assert::AreEqual(expected, actual);
	}

	TEST_METHOD(GetSelectCombinationsTest) {

		TotalCombinationList totalCombi(getSampleList());
		std::vector<std::string> selectList{ "a", "b" , "c" };

		PartialCombinationList
			selectedCombs(totalCombi.getCombinationList(selectList));
		std::string actual(PartialToString(selectedCombs));

		std::string expected("<<a:1,b:2,c:4>,<a:1,b:2,c:5>,<a:1,b:3,c:4>,<a:1,b:3,c:5>>");
		Assert::AreEqual(expected, actual);
	}

	TEST_METHOD(GetContentTest) {

		TotalCombinationList totalCombi(getSampleList());
		std::unordered_map<Synonym, unsigned> content(totalCombi.getContent());

		std::stringstream ss;
		ss << "<";
		std::unordered_map<Synonym, unsigned>::iterator it(content.begin());
		while (it != content.end()) {
			ss << (*it).first << ":" << (*it).second;
			it++;
			if (it != content.end()) ss << ",";
		}
		ss << ">";

		std::string actual(ss.str());
		std::string expected("<a:0,b:0,c:1>");
		Assert::AreEqual(expected, actual);
	}

	TEST_METHOD(OperatorLookupTest) {

		TotalCombinationList totalCombi(getSampleList());
		PartialCombinationList factor(totalCombi["a"]);
		std::string actual(PartialToString(factor));
		std::string expected("<<a:1,b:2>,<a:1,b:3>>");
		Assert::AreEqual(expected, actual);
	}

	TEST_METHOD(GetFactorListTest) {

		TotalCombinationList totalCombi(getSampleList());
		std::vector<std::string> selectList{ "a", "b" };

		std::map<unsigned, PartialCombinationList> factorList(totalCombi.getFactorList());

		std::string actual("{");
		for (auto kv : factorList) {
			actual.append(PartialToString(kv.second));
			actual.append(";");
		}
		actual.append("}");
		std::string expected("{<<a:1,b:2>,<a:1,b:3>>;<<c:4>,<c:5>>;}");
		Assert::AreEqual(expected, actual);
	}

	TEST_METHOD(ReduceTotalContentTest) {
		TotalCombinationList totalCombi(getSampleList());
		std::vector<std::string> selectList{ "a", "b" };

		totalCombi.reduceTotalContent(selectList);
		PartialCombinationList result(totalCombi["a"]);
		//result = totalCombi.cartesianProduct(result, result);
		//PartialCombinationList result(totalCombi.getCombinationList(selectList));
		//totalCombi.reduceSingleFactor(selectList, result);

		std::string actual(PartialToString(result));
		std::string expected("<<a:1,b:2>,<a:1,b:3>>");
		Assert::AreEqual(expected, actual);
	}
	};
}
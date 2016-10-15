#include <iostream>
#include <set>

#include "QueryProcessor/QueryOptimizer.h"
#include "QueryProcessor/Clause.h"
#include "PKB/PKB.h"
#include "Utils.h"

using std::string;
using std::vector;

QueryOptimizer::QueryOptimizer() {}

QueryOptimizer::~QueryOptimizer() {}

struct SortByNumOfSynonyms {
    bool operator()(Clause& x, Clause& y) const {
        return x.getSynonyms().size() < y.getSynonyms().size();
    }
};

struct SortByPriority {
    std::unordered_map<std::string, Symbol> varMap;
    SortByPriority(std::unordered_map<std::string, Symbol> vm) {
        this->varMap = vm;
    }

    int getPriority(Clause clause) const {
        Symbol clauseSymbol = Constants::StringToSymbol(clause.getClauseType());
        switch (clauseSymbol) {
            case PATTERN:
            {
                //Check if exact or sub-expression match.
                string rhs = clause.getArg()[1];
                if ((Utils::StartsWith(rhs, CHAR_SYMBOL_DOUBLEQUOTES) && Utils::EndsWith(rhs, CHAR_SYMBOL_DOUBLEQUOTES))) {
                    //Exact expression match
                    return PKB::GetPriority(MODIFIES) + 1;
                }
                else if ((Utils::StartsWith(rhs, CHAR_SYMBOL_UNDERSCORE) && Utils::EndsWith(rhs, CHAR_SYMBOL_UNDERSCORE))) {
                    //Sub-expression match
                    return PKB::GetPriority(MODIFIES) + 2;
                }
                break;
            }
            case MODIFIES:
            {
                //Check if lhs is a procedure type
                string lhs = clause.getArg()[0];
                
                if (varMap.find(lhs)->second == PROCEDURE) {
                    return PKB::GetPriority(MODIFIES_PROCEDURE);
                }
                else {
                    return PKB::GetPriority(clauseSymbol);
                }
                break;
            }
            case USES:
            {
                //Check if lhs is a procedure type
                string lhs = clause.getArg()[0];

                if (varMap.find(lhs)->second == PROCEDURE) {
                    return PKB::GetPriority(USES_PROCEDURE);
                }
                else {
                    return PKB::GetPriority(clauseSymbol);
                }
                break;
            }
            default:
                return PKB::GetPriority(clauseSymbol);
                break;
        }
    }

    bool operator()(Clause& x, Clause& y) const {
        if (x.getClauseType() == Constants::SymbolToString(WITH)) {
            return true;
        }
        else if (y.getClauseType() == Constants::SymbolToString(WITH)) {
            return false;
        }
        else {
            //Both are not WITH clauses
            return getPriority(x) < getPriority(y);
        }
    }
};

QueryTree QueryOptimizer::optimize(QueryTree qt) {
    this->queryTree = qt;

    vector<Clause> booleanClauses;
    vector<vector<Clause>> genericGroups;
    vector<vector<Clause>> selectedGroups;

    vector<Clause> allClauses(queryTree.getClauses());

    for (unsigned i = 0; i < allClauses.size(); i++) {
        Clause clause = allClauses.at(i);
        
        if (clause.getSynonyms().size() == 0) {
            booleanClauses.push_back(clause);
            allClauses.erase(allClauses.begin() + i);
            i--;
        }
    }
    
    //Insert first clause into group if group is empty
    if (genericGroups.empty() && !allClauses.empty()) {
        vector<Clause> group;
        group.push_back(allClauses.at(0));
        genericGroups.push_back(group);
        allClauses.erase(allClauses.begin() + 0);
    }
    
    //First common synonym detection loop
    //For every clause, compare with clauses that are already in groups. If common synonyms are found, add the clause into the group
    for (vector<Clause>::iterator it1 = allClauses.begin(); it1 != allClauses.end();) {
        Clause clause = *it1;
        bool isInGroup = false;
        for (vector<vector<Clause>>::iterator it2 = genericGroups.begin(); it2 != genericGroups.end(); ++it2) {
            vector<Clause> group = *it2;
            for (Clause c : group) {
                bool match = false;
                vector<string> args = clause.getArg();
                for (string arg : c.getArg()) {
                    if (clause.getClauseType() == "pattern") {
                        if (arg == args.at(0) || arg == args.at(1) || arg == args.at(2)) {
                            match = true;
                        }
                    }
                    else {
                        if (arg == args.at(0) || arg == args.at(1)) {
                            match = true;
                        }
                    }
                }
                if (match) {
                    (*it2).push_back(clause);
                    isInGroup = true;
                    break;
                }
            }
            if (isInGroup) {
                break;
            }
        }
        if (!isInGroup) {
            vector<Clause> newGroup;
            newGroup.push_back(clause);
            genericGroups.push_back(newGroup);
            isInGroup = true;
        }
        
        if (isInGroup) {
            it1 = allClauses.erase(it1);
        }
        else {
            ++it1;
        }
    }
    
    //Second common synonym detection loop
    //Combine groups that have common synonyms, that may not be detected in the first loop
    for (vector<vector<Clause>>::iterator it1 = genericGroups.begin(); it1 != genericGroups.end(); ++it1) {
        vector<Clause> group1 = *it1;
        std::set<string> synonyms;

        for (Clause clause : group1) {
            for (string synonym : clause.getSynonyms()) {
                synonyms.insert(synonym);
            }
        }

        for (vector<vector<Clause>>::iterator it2 = it1 + 1; it2 != genericGroups.end();) {
            vector<Clause> group2 = *it2;
            bool isCombined = false;
            for (Clause c : group2) {
                bool match = false;
                for (string args : c.getArg()) {
                    std::set<string>::const_iterator it3 = synonyms.find(args);
                    if (it3 != synonyms.end()) {
                        //found matching synonym
                        match = true;
                        break;
                    }
                }
                if (match) {
                    (*it1).insert((*it1).end(), (*it2).begin(), (*it2).end()); //copy all clauses from group 2 into group 1
                    isCombined = true;
                    break;
                }
            }
            if (isCombined) {
                it2 = genericGroups.erase(it2);
            }
            else {
                ++it2;
            }
        }
    }
    
    //Check synonyms used in groups against those that are selected by query
    //If none, insert to unselected groups
    //If selected, insert to selected groups
    for (vector<vector<Clause>>::iterator it1 = genericGroups.begin(); it1 != genericGroups.end();) {
        vector<Clause> group = *it1;
        std::set<string> synonyms;

        for (Clause clause : group) {
            for (string synonym : clause.getSynonyms()) {
                synonyms.insert(synonym);
            }
        }
        
        bool isSelected = false;
        for (string var : queryTree.getResults()) {
            std::set<string>::const_iterator it2 = synonyms.find(var);
            if (it2 != synonyms.end()) {
                //at least one of the synonyms are selected by the query
                isSelected = true;
                break;
            }
        }
        if (isSelected) {
            selectedGroups.push_back(*it1);
            it1 = genericGroups.erase(it1);
        }
        else {
            ++it1;
        }
        
    }
    
    //Sort clauses within groups
    for (vector<vector<Clause>>::iterator it = genericGroups.begin(); it != genericGroups.end(); ++it) {
        *it = sortGroup(*it);
    }

    for (vector<vector<Clause>>::iterator it = selectedGroups.begin(); it != selectedGroups.end(); ++it) {
        *it = sortGroup(*it);
    }
    
    //Groups that remain in genericGroups are unselectedGroups
    queryTree.setBooleanClauses(booleanClauses);
    queryTree.setUnselectedGroups(genericGroups);
    queryTree.setSelectedGroups(selectedGroups);

    return queryTree;
}

std::vector<Clause> QueryOptimizer::sortGroup(std::vector<Clause> group) {
    //Sort according to number of synonyms
    std::sort(group.begin(), group.end(), SortByNumOfSynonyms());

    //Reorder based on synonyms evaluated. e.g. Modifies(s3,"x") will evaluate s3. Next query should consist of a synonym with s3
    std::vector<Clause> sortedGroup;
    std::set<string> evaluatedSynonyms;

    //Evaluate all clauses with one synonym first
    while (!group.empty()) {
        Clause clause = group.at(0);
        if (clause.getSynonyms().size() == 1) {
            evaluatedSynonyms.insert(clause.getSynonyms().at(0));
            sortedGroup.push_back(clause);
            group.erase(group.begin());
        }
        else {
            break;
        }
    }
    //Since sortedGroup now only contains clauses with one synonym, sort it according to priority to arrange WITH clauses first
    std::sort(sortedGroup.begin(), sortedGroup.end(), SortByPriority(queryTree.getVarMap()));

    //Process the remaining queries with two synonyms
    while (!group.empty()) {
        //Possible clauses are those that have at least one synonym evaluated.
        vector<Clause> possibleClauses;

        for (vector<Clause>::iterator it1 = group.begin(); it1 != group.end();) {
            vector<string> clauseSynonyms = (*it1).getSynonyms();
            
            bool match = false;
            for (string synonym : clauseSynonyms) {
                std::set<string>::const_iterator it2 = evaluatedSynonyms.find(synonym);
                if (it2 != evaluatedSynonyms.end()) {
                    //found matching synonym
                    match = true;
                    break;
                }
            }

            if (match) {
                possibleClauses.push_back(*it1);
                it1 = group.erase(it1);
            }
            else {
                ++it1;
            }
        }

        //For every clause in possibleClauses, insert them in priority order into sortedGroup
        if (!possibleClauses.empty()) {
            std::sort(possibleClauses.begin(), possibleClauses.end(), SortByPriority(queryTree.getVarMap()));
            for (Clause c : possibleClauses) {
                for (string synonym : c.getSynonyms()) {
                    evaluatedSynonyms.insert(synonym);
                }
                sortedGroup.push_back(c);
            }
        }
        else {
            //This group of selected clauses do not have synonyms evaluated from one-synonym clauses
            //Sort group, then remove all clauses from group and insert into sortedGroup
            std::sort(group.begin(), group.end(), SortByPriority(queryTree.getVarMap()));
            for (Clause c : group) {
                sortedGroup.push_back(c);
            }
            group.clear();
        }
    }

    return sortedGroup;
}
#pragma once

#include <string>
#include <vector>
#include <utility>

#include "QueryProcessor/QueryTree.h"

class QueryOptimizer {
 public:
     QueryOptimizer();
     ~QueryOptimizer();

     QueryTree optimize(QueryTree queryTree);

 private:
     QueryTree queryTree;

     std::pair<std::vector<std::string>, std::vector<Clause>> sortGroup(std::vector<Clause> group);
     std::vector<std::pair<std::vector<std::string>, std::vector<Clause>>> uniqueSimplifiedPairs(std::vector<std::pair<std::vector<std::string>, std::vector<Clause>>>unselectedPairs);
};


#pragma once

#include "QueryProcessor/QueryTree.h"

class QueryOptimizer {
 public:
     QueryOptimizer();
     ~QueryOptimizer();

     QueryTree optimize(QueryTree queryTree);
     
 private:
     QueryTree queryTree;

     std::vector<Clause> sortGroup(std::vector<Clause> group);
     
};

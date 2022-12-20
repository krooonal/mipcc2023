#include <iostream>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/struct_var.h>
#include <scip/struct_history.h>
#include "scip/history.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <stdio.h>
#include <time.h>

#ifndef __KP_SOLUTIONS_H__
#define __KP_SOLUTIONS_H__

using namespace std;

class Solution
{
public:
    void Populate(SCIP *scip, const std::vector<SCIP_VAR *> &scip_variables,
                  SCIP_SOL *solution);

    SCIP_RETCODE AddToModel(SCIP *scip,
                            std::vector<SCIP_VAR *> &scip_variables);
    std::map<string, double> GetVarValue();

private:
    std::map<string, double> varvalues_;
    double fscore_ = 0.0;
};

class SolutionPool
{
public:
    void AddSolution(Solution solution);

    SCIP_RETCODE AddToModel(SCIP *scip,
                            std::vector<SCIP_VAR *> &scip_variables);

    Solution GetSolution(int index);
    int GetNumSolutions();

private:
    std::vector<Solution> solutions_;
    std::map<string, double> varvaluefreq_;
};

#endif
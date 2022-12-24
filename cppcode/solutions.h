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

    // Adds solution to currently stored scip vars.
    SCIP_RETCODE AddSolutionToModel(int sol_index, SCIP *scip);
    SCIP_RETCODE AddNextSolutionToModel(SCIP *scip);
    void SetCurrentScipVars(std::vector<SCIP_VAR *> *current_scip_variables)
    {
        current_scip_variables_ = current_scip_variables;
        last_added_solution_index_ = -1;
    }

    void ReduceCommonSolFac()
    {
        common_sol_factor_ *= decrese_factor_;
    }

private:
    std::vector<Solution> solutions_;
    Solution common_sol_;
    double common_sol_factor_ = 0.9;
    double decrese_factor_ = 0.99;
    std::map<string, std::map<double, int>> varvaluefreq_;
    std::vector<SCIP_VAR *> *current_scip_variables_;
    int last_added_solution_index_ = -1;
};

#endif
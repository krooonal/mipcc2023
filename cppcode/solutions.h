#ifndef __KP_SOLUTIONS_H__
#define __KP_SOLUTIONS_H__
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

using namespace std;

class Solution
{
public:
    // Get the solution from SCIP to this class.
    void Populate(SCIP *scip, const std::vector<SCIP_VAR *> &scip_variables,
                  SCIP_SOL *solution);

    // Populate SCIP partial solution hint using solution values stored in
    // this class. Continuous variables are ignored. The values are clipped
    // if they lie outside the current bounds.
    SCIP_RETCODE AddToModel(SCIP *scip,
                            std::vector<SCIP_VAR *> &scip_variables);

    // Populate SCIP complete solution hint using solution values stored in
    // this class. This method was used for experiments only. It is not used
    // in the final version.
    SCIP_RETCODE AddToModelComplete(SCIP *scip,
                                    std::vector<SCIP_VAR *> &scip_variables);

    // Get the stored solution.
    std::map<string, double> GetVarValue();

private:
    std::map<string, double> varvalues_;
};

class SolutionPool
{
public:
    // Add new solution to the pool. The first solution is set as the common
    // solution. Also updates the frequency of observed values for each variable.
    void AddSolution(Solution solution);

    // Adds up to maximum of num_hint_solns_ solution hints and a common hint to
    // SCIP. Also adds a common solution hint if there are more than one solutions
    // in the pool.
    SCIP_RETCODE AddToModel(SCIP *scip,
                            std::vector<SCIP_VAR *> &scip_variables);

    // Number of solutions added as hints by 'AddToModel' method.
    void SetNumHintSolns(int num)
    {
        num_hint_solns_ = num;
    }

private:
    std::vector<Solution> solutions_;
    Solution common_sol_;
    double common_sol_factor_ = 0.9;
    std::map<string, std::map<double, int>> varvaluefreq_;
    int num_hint_solns_ = 9;
};

#endif
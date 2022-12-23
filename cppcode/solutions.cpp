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

#include "solutions.h"

using namespace std;

void Solution::Populate(SCIP *scip,
                        const std::vector<SCIP_VAR *> &scip_variables,
                        SCIP_SOL *solution)
{
    for (SCIP_VAR *var : scip_variables)
    {
        const string name = SCIPvarGetName(var);
        const double val = SCIPgetSolVal(scip, solution, var);
        varvalues_[name] = val;
    }
}

SCIP_RETCODE Solution::AddToModel(SCIP *scip,
                                  std::vector<SCIP_VAR *> &scip_variables)
{
    SCIP_SOL *solution;
    SCIP_CALL(SCIPcreatePartialSol(scip, &solution, NULL));
    for (SCIP_VAR *var : scip_variables)
    {
        const string name = SCIPvarGetName(var);
        double val = varvalues_.at(name);
        double var_lb = var->locdom.lb;
        double var_ub = var->locdom.ub;
        if (SCIPvarGetType(var) == SCIP_VARTYPE_CONTINUOUS)
            continue;
        if (val < var_lb)
            val = var_lb;
        if (val > var_ub)
            val = var_ub;
        SCIP_CALL(SCIPsetSolVal(scip, solution, var, val));
    }
    SCIP_Bool is_stored;
    SCIP_CALL(SCIPaddSolFree(scip, &solution, &is_stored));
    if (is_stored)
    {
        cout << "Added a partial solution\n";
        // cout << "Number of partial solutions: "
        // << SCIPgetNPartialSols(scip) << "\n";
    }
    return SCIP_OKAY;
}

std::map<string, double> Solution::GetVarValue()
{
    return varvalues_;
}

void SolutionPool::AddSolution(Solution solution)
{
    solutions_.push_back(solution);
    if (solutions_.size() == 1)
    {
        common_sol_ = solution;
    }
    std::map<string, double> varvalues = solution.GetVarValue();
    for (auto var_val : varvalues)
    {
        string var_name = var_val.first;
        double value = var_val.second;
        if (varvaluefreq_.find(var_name) == varvaluefreq_.end())
        {
            varvaluefreq_[var_name] = std::map<double, int>();
        }
        else if (
            varvaluefreq_[var_name].find(value) == varvaluefreq_[var_name]
                                                       .end())
        {
            varvaluefreq_[var_name][value] = 1;
        }
        else
        {
            varvaluefreq_[var_name][value] += 1;
        }
    }
}

SCIP_RETCODE SolutionPool::AddToModel(SCIP *scip,
                                      std::vector<SCIP_VAR *> &scip_variables)
{
    if (solutions_.empty())
        return SCIP_OKAY;
    for (Solution solution : solutions_)
    {
        SCIP_CALL(solution.AddToModel(scip, scip_variables));
    }

    int num_solutions = solutions_.size();
    if (num_solutions <= 1)
        return SCIP_OKAY;
    SCIP_SOL *common_solution;
    SCIP_CALL(SCIPcreatePartialSol(scip, &common_solution, NULL));
    std::map<string, double> varvalues = common_sol_.GetVarValue();
    int num_var_hinted = 0;
    for (SCIP_VAR *var : scip_variables)
    {
        string var_name = var->name;
        double value = varvalues[var_name];
        if (SCIPvarGetType(var) == SCIP_VARTYPE_CONTINUOUS)
            continue;

        if (varvaluefreq_[var_name][value] >= num_solutions * 0.8)
        {
            num_var_hinted++;
            double var_lb = var->locdom.lb;
            double var_ub = var->locdom.ub;
            if (value < var_lb)
                value = var_lb;
            if (value > var_ub)
                value = var_ub;
            SCIP_CALL(SCIPsetSolVal(scip, common_solution, var, value));
        }
    }
    cout << "Number of vars hinted = " << num_var_hinted << endl;
    SCIP_Bool is_stored;
    SCIP_CALL(SCIPaddSolFree(scip, &common_solution, &is_stored));
    if (is_stored)
    {
        cout << "Added a common partial solution\n";
        // cout << "Number of partial solutions: "
        // << SCIPgetNPartialSols(scip) << "\n";
    }
    return SCIP_OKAY;
}

Solution SolutionPool::GetSolution(int index)
{
    assert(index >= 0);
    assert(index < solutions_.size());
    return solutions_[index];
}
int SolutionPool::GetNumSolutions()
{
    return solutions_.size();
}

SCIP_RETCODE SolutionPool::AddSolutionToModel(int sol_index, SCIP *scip)
{
    assert(sol_index < solutions_.size());
    SCIP_CALL(solutions_[sol_index].AddToModel(scip, *current_scip_variables_));
    last_added_solution_index_ = sol_index;
    return SCIP_OKAY;
}

SCIP_RETCODE SolutionPool::AddNextSolutionToModel(SCIP *scip)
{
    int index = last_added_solution_index_ - 1;
    if (last_added_solution_index_ == -1)
    {
        index = solutions_.size() - 1;
    }
    if (index < 0 || index > solutions_.size())
    {
        return SCIP_OKAY;
    }
    cout << "Adding solution " << index << " to model\n";
    SCIP_CALL(solutions_[index].AddToModel(scip, *current_scip_variables_));
    last_added_solution_index_ = index;
    return SCIP_OKAY;
}
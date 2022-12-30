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

#include "var_history.h"

using namespace std;

void VarHistories::Populate(SCIP *scip,
                            std::vector<SCIP_VAR *> &scip_variables)
{
    var_histories_.clear();
    for (SCIP_VAR *var : scip_variables)
    {
        const string name = SCIPvarGetName(var);
        SCIP_VAR *trans_var = SCIPvarGetTransVar(var);
        SCIP_HISTORY var_history = *(trans_var->history);
        var_history.pscostcount[0] = min(var_history.pscostcount[0], 4.0);
        var_history.pscostcount[1] = min(var_history.pscostcount[1], 4.0);
        var_histories_[name] = var_history;

        // Reduce counts of cumpscost
        var_cumpscost_count_[name] = min(var_cumpscost_count_[name], 4ll);
        var_cumpscost_down_count_[name] = min(var_cumpscost_down_count_[name], 4ll);
    }
}

void VarHistories::AddToModel(SCIP *scip,
                              std::vector<SCIP_VAR *> &scip_variables)
{
    for (SCIP_VAR *var : scip_variables)
    {
        const string name = SCIPvarGetName(var);
        SCIP_HISTORY var_history = var_histories_.at(name);
        *(var->history) = var_histories_.at(name);
    }
}

double VarHistories::GetCumpscost(string name)
{
    double threshold = 1e-6;
    double upcost = threshold;
    double downcost = threshold;
    if (var_cumpscost_.find(name) != var_cumpscost_.end())
        upcost = max(threshold, var_cumpscost_[name]);
    if (var_cumpscost_down_.find(name) != var_cumpscost_down_.end())
        downcost = max(threshold, var_cumpscost_down_[name]);
    return upcost * downcost;
}

long long VarHistories::GetCumpscostCount(string name)
{
    long long upcount = 0;
    long long downcount = 0;
    if (var_cumpscost_count_.find(name) != var_cumpscost_count_.end())
        upcount = var_cumpscost_count_.at(name);
    if (var_cumpscost_down_count_.find(name) != var_cumpscost_down_count_.end())
        downcount = var_cumpscost_down_count_.at(name);
    return upcount + downcount;
}

void VarHistories::UpdateCumpscost(string name, double cost_update, bool update_count)
{
    long long count = 0;
    double current_cost = 0.0;
    if (var_cumpscost_.find(name) != var_cumpscost_.end())
    {
        count = var_cumpscost_count_[name];
        current_cost = var_cumpscost_[name];
    }
    if (update_count)
        count++;

    current_cost = (current_cost * count + cost_update) / (count);
    // cout << "Update count and cost of "
    //      << name << " " << count << " " << current_cost << endl;
    var_cumpscost_[name] = current_cost;
    var_cumpscost_count_[name] = count;
}

void VarHistories::UpdateCumpscostDown(string name, double cost_update, bool update_count)
{
    long long count = 0;
    double current_cost = 0.0;
    if (var_cumpscost_down_.find(name) != var_cumpscost_down_.end())
    {
        count = var_cumpscost_down_count_[name];
        current_cost = var_cumpscost_down_[name];
    }
    if (update_count)
        count++;

    current_cost = (current_cost * count + cost_update) / (count);
    // cout << "Update count and cost of "
    //      << name << " " << count << " " << current_cost << endl;
    var_cumpscost_down_[name] = current_cost;
    var_cumpscost_down_count_[name] = count;
}

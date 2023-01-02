#include <iostream>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/struct_var.h>
#include <scip/struct_scip.h>
#include <scip/struct_stat.h>
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
    global_history_ = *(scip->stat->glbhistory);
    global_history_.pscostcount[0] = min(global_history_.pscostcount[0], history_count_reset_);
    global_history_.pscostcount[1] = min(global_history_.pscostcount[1], history_count_reset_);

    // var_histories_.clear();
    for (SCIP_VAR *var : scip_variables)
    {
        const string name = SCIPvarGetName(var);
        SCIP_VAR *trans_var = SCIPvarGetTransVar(var);
        SCIP_HISTORY var_history = *(trans_var->history);
        // cout << "pseudocost counts: "
        //      << var_history.pscostcount[0] << " "
        //      << var_history.pscostcount[1] << endl;
        var_history.pscostcount[0] = min(var_history.pscostcount[0], history_count_reset_);
        var_history.pscostcount[1] = min(var_history.pscostcount[1], history_count_reset_);
        var_histories_[name] = var_history;

        // Reduce counts of cumpscost
        // cout << "Current counts " << var_cumpscost_count_[name] << " "
        //      << var_cumpscost_down_count_[name] << endl;
        // var_cumpscost_count_[name] = min(var_cumpscost_count_[name], 4ll);
        // var_cumpscost_down_count_[name] = min(var_cumpscost_down_count_[name], 4ll);
        // If they didn't get any updates, set their cumpscost to pscost.
        // if (var_cumpscost_count_[name] == 0 && var_history.pscostcount[0] > 0)
        // {
        //     var_cumpscost_[name] = var_history.pscostweightedmean[0];
        //     var_cumpscost_count_[name] = 1;
        // }
        // if (var_cumpscost_down_count_[name] == 0 && var_history.pscostcount[1] > 0)
        // {
        //     var_cumpscost_down_count_[name] = var_history.pscostweightedmean[1];
        //     var_cumpscost_down_count_[name] = 1;
        // }
    }
}

void VarHistories::AddToModel(SCIP *scip,
                              std::vector<SCIP_VAR *> &scip_variables)
{
    *(scip->stat->glbhistory) = global_history_;
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
    long long oldcount = count;
    if (update_count)
        count++;

    current_cost = (current_cost * oldcount + cost_update) / (count);
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
    long long oldcount = count;
    if (update_count)
        count++;

    current_cost = (current_cost * oldcount + cost_update) / (count);
    // cout << "Update count and cost of "
    //      << name << " " << count << " " << current_cost << endl;
    var_cumpscost_down_[name] = current_cost;
    var_cumpscost_down_count_[name] = count;
}

double VarHistories::GetLevelpscost(string name, int level)
{
    double threshold = 1e-6;
    double upcost = threshold;
    double downcost = threshold;
    if (var_levelpscost_.find(name) != var_levelpscost_.end())
    {
        vector<double> &level_pscosts = var_levelpscost_[name];
        if (level < level_pscosts.size())
        {
            upcost = max(threshold, level_pscosts[level]);
        }
    }
    if (var_levelpscost_down_.find(name) != var_levelpscost_down_.end())
    {
        vector<double> &level_pscosts = var_levelpscost_down_[name];
        if (level < level_pscosts.size())
        {
            downcost = max(threshold, level_pscosts[level]);
        }
    }
    return upcost * downcost;
}

long long VarHistories::GetLevelpscostCount(string name, int level)
{
    long long upcount = 0;
    long long downcount = 0;
    if (var_levelpscost_count_.find(name) != var_levelpscost_count_.end())
    {
        vector<long long> &level_pscosts_count = var_levelpscost_count_[name];
        if (level < level_pscosts_count.size())
        {
            upcount = level_pscosts_count[level];
        }
    }
    if (var_levelpscost_down_count_.find(name) != var_levelpscost_down_count_.end())
    {
        vector<long long> &level_pscosts_count = var_levelpscost_down_count_[name];
        if (level < level_pscosts_count.size())
        {
            downcount = level_pscosts_count[level];
        }
    }
    return upcount + downcount;
}

void VarHistories::UpdateLevelpscost(string name, int level, double cost_update,
                                     bool update_count)
{
    long long count = 0;
    double current_cost = 0.0;
    if (var_levelpscost_.find(name) != var_levelpscost_.end())
    {
        vector<double> &level_pscosts = var_levelpscost_[name];
        vector<long long> &level_pscosts_count = var_levelpscost_count_[name];
        int current_size = level_pscosts.size();
        if (level < current_size)
        {
            count = level_pscosts_count[level];
            current_cost = level_pscosts[level];
        }
        else
        {
            for (; current_size <= level + 1; ++current_size)
            {
                level_pscosts.push_back(0.0);
                level_pscosts_count.push_back(0ll);
            }
        }
    }
    else
    {
        vector<double> level_pscosts;
        vector<long long> level_pscosts_count;
        int current_size = level_pscosts.size();
        for (; current_size <= level + 1; ++current_size)
        {
            level_pscosts.push_back(0.0);
            level_pscosts_count.push_back(0ll);
        }
        var_levelpscost_[name] = level_pscosts;
        var_levelpscost_count_[name] = level_pscosts_count;
    }
    long long oldcount = count;
    if (update_count)
        count++;

    vector<double> &level_pscosts = var_levelpscost_[name];
    vector<long long> &level_pscosts_count = var_levelpscost_count_[name];

    current_cost = (current_cost * oldcount + cost_update) / (count);
    // cout << "Update count and cost of "
    //      << name << " " << count << " " << current_cost << endl;
    level_pscosts[level] = current_cost;
    level_pscosts_count[level] = count;
}

void VarHistories::UpdateLevelpscostDown(string name, int level, double cost_update,
                                         bool update_count)
{
    long long count = 0;
    double current_cost = 0.0;
    if (var_levelpscost_down_.find(name) != var_levelpscost_down_.end())
    {
        vector<double> &level_pscosts = var_levelpscost_down_[name];
        vector<long long> &level_pscosts_count = var_levelpscost_down_count_[name];
        int current_size = level_pscosts.size();
        if (level < current_size)
        {
            count = level_pscosts_count[level];
            current_cost = level_pscosts[level];
        }
        else
        {
            for (; current_size <= level + 1; ++current_size)
            {
                level_pscosts.push_back(0.0);
                level_pscosts_count.push_back(0ll);
            }
        }
    }
    else
    {
        vector<double> level_pscosts;
        vector<long long> level_pscosts_count;
        int current_size = level_pscosts.size();
        for (; current_size <= level + 1; ++current_size)
        {
            level_pscosts.push_back(0.0);
            level_pscosts_count.push_back(0ll);
        }
        var_levelpscost_down_[name] = level_pscosts;
        var_levelpscost_down_count_[name] = level_pscosts_count;
    }
    long long oldcount = count;
    if (update_count)
        count++;

    vector<double> &level_pscosts = var_levelpscost_down_[name];
    vector<long long> &level_pscosts_count = var_levelpscost_down_count_[name];

    current_cost = (current_cost * oldcount + cost_update) / (count);
    // cout << "Update count and cost of "
    //      << name << " " << count << " " << current_cost << endl;
    level_pscosts[level] = current_cost;
    level_pscosts_count[level] = count;
}

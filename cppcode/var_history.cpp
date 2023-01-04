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

        var_oldpscost0_count_[name] = var_history.pscostcount[0];
        var_oldpscost1_count_[name] = var_history.pscostcount[1];

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
// OLDPSCOST----------------------------------------
double VarHistories::GetOldpscost0(string name)
{
    double pscost = 0.0;
    if (var_oldpscost0_.find(name) != var_oldpscost0_.end())
    {
        pscost = var_oldpscost0_[name];
    }
    return pscost;
}
double VarHistories::GetOldpscostCount0(string name)
{
    double count = 0.0;
    if (var_oldpscost0_count_.find(name) != var_oldpscost0_count_.end())
    {
        count = var_oldpscost0_count_[name];
    }
    return count;
}
double VarHistories::GetOldpscost1(string name)
{
    double pscost = 0.0;
    if (var_oldpscost1_.find(name) != var_oldpscost1_.end())
    {
        pscost = var_oldpscost1_[name];
    }
    return pscost;
}
double VarHistories::GetOldpscostCount1(string name)
{
    double count = 0.0;
    if (var_oldpscost1_count_.find(name) != var_oldpscost1_count_.end())
    {
        count = var_oldpscost1_count_[name];
    }
    return count;
}
void VarHistories::SetOldpscost0(string name, double value)
{
    var_oldpscost0_[name] = value;
}
void VarHistories::SetOldpscostCount0(string name, double value)
{
    var_oldpscost0_count_[name] = value;
}
void VarHistories::SetOldpscost1(string name, double value)
{
    var_oldpscost1_[name] = value;
}
void VarHistories::SetOldpscostCount1(string name, double value)
{
    var_oldpscost1_count_[name] = value;
}
// OLDPSCOST----------------------------------------

// LEVELPSCOST----------------------------------------
double VarHistories::GetLevelpscost0(string name, int level)
{
    double threshold = 1e-6;
    double upcost = threshold;
    if (var_levelpscost0_.find(name) != var_levelpscost0_.end())
    {
        vector<double> &level_pscosts = var_levelpscost0_[name];
        if (level < level_pscosts.size())
        {
            upcost = max(threshold, level_pscosts[level]);
        }
    }
    return upcost;
}
double VarHistories::GetLevelpscost1(string name, int level)
{
    double threshold = 1e-6;
    double downcost = threshold;
    if (var_levelpscost1_.find(name) != var_levelpscost1_.end())
    {
        vector<double> &level_pscosts = var_levelpscost1_[name];
        if (level < level_pscosts.size())
        {
            downcost = max(threshold, level_pscosts[level]);
        }
    }
    return downcost;
}
double VarHistories::GetLevelpscost(string name, int level)
{
    double upcost = GetLevelpscost1(name, level);
    double downcost = GetLevelpscost0(name, level);
    return upcost * downcost;
}

double VarHistories::GetLevelpscostCount0(string name, int level)
{
    double upcount = 0;
    if (var_levelpscost0_count_.find(name) != var_levelpscost0_count_.end())
    {
        vector<double> &level_pscosts_count = var_levelpscost0_count_[name];
        if (level < level_pscosts_count.size())
        {
            upcount = level_pscosts_count[level];
        }
    }
    return upcount;
}
double VarHistories::GetLevelpscostCount1(string name, int level)
{
    double downcount = 0;
    if (var_levelpscost1_count_.find(name) != var_levelpscost1_count_.end())
    {
        vector<double> &level_pscosts_count = var_levelpscost1_count_[name];
        if (level < level_pscosts_count.size())
        {
            downcount = level_pscosts_count[level];
        }
    }
    return downcount;
}
double VarHistories::GetLevelpscostCount(string name, int level)
{
    double upcount = GetLevelpscostCount1(name, level);
    double downcount = GetLevelpscostCount0(name, level);
    // cout << name << " " << upcount << " " << downcount << endl;
    return upcount + downcount;
}

void VarHistories::UpdateLevelpscost0(string name, int level, double cost_update,
                                      double weight)
{
    double count = 0;
    double current_cost = 0.0;
    if (var_levelpscost0_.find(name) != var_levelpscost0_.end())
    {
        vector<double> &level_pscosts = var_levelpscost0_[name];
        vector<double> &level_pscosts_count = var_levelpscost0_count_[name];
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
        vector<double> level_pscosts_count;
        int current_size = level_pscosts.size();
        for (; current_size <= level + 1; ++current_size)
        {
            level_pscosts.push_back(0.0);
            level_pscosts_count.push_back(0ll);
        }
        var_levelpscost0_[name] = level_pscosts;
        var_levelpscost0_count_[name] = level_pscosts_count;
    }
    double oldcount = count;
    double newcount = oldcount + weight;

    vector<double> &level_pscosts = var_levelpscost0_[name];
    vector<double> &level_pscosts_count = var_levelpscost0_count_[name];

    current_cost = (current_cost * oldcount + weight * cost_update) / (newcount);
    // cout << "Update count and cost of "
    //      << name << " " << newcount << " " << current_cost << endl;
    level_pscosts[level] = current_cost;
    level_pscosts_count[level] = newcount;
}

void VarHistories::UpdateLevelpscost1(string name, int level, double cost_update,
                                      double weight)
{
    double count = 0;
    double current_cost = 0.0;
    if (weight > 1.0)
        cout << "Called with weight " << weight << endl;
    if (var_levelpscost1_.find(name) != var_levelpscost1_.end())
    {
        vector<double> &level_pscosts = var_levelpscost1_[name];
        vector<double> &level_pscosts_count = var_levelpscost1_count_[name];
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
        vector<double> level_pscosts_count;
        int current_size = level_pscosts.size();
        for (; current_size <= level + 1; ++current_size)
        {
            level_pscosts.push_back(0.0);
            level_pscosts_count.push_back(0ll);
        }
        var_levelpscost1_[name] = level_pscosts;
        var_levelpscost1_count_[name] = level_pscosts_count;
    }
    double oldcount = count;
    double newcount = oldcount + weight;

    vector<double> &level_pscosts = var_levelpscost1_[name];
    vector<double> &level_pscosts_count = var_levelpscost1_count_[name];

    current_cost = (current_cost * oldcount + weight * cost_update) / (newcount);
    // cout << "Update count and cost of "
    //      << name << " " << count << " " << current_cost << endl;
    level_pscosts[level] = current_cost;
    level_pscosts_count[level] = newcount;
}
// LEVELPSCOST----------------------------------------

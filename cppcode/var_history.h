#ifndef __KP_VHISTORY_H__
#define __KP_VHISTORY_H__

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

class VarHistories
{
public:
    void Populate(SCIP *scip, std::vector<SCIP_VAR *> &scip_variables);

    void AddToModel(SCIP *scip, std::vector<SCIP_VAR *> &scip_variables);

    double GetCumpscost(string name);
    long long GetCumpscostCount(string name);
    void UpdateCumpscost(string name, double cost_update, bool update_count);
    void UpdateCumpscostDown(string name, double cost_update, bool update_count);

    double GetLevelpscost(string name, int level);
    long long GetLevelpscostCount(string name, int level);
    void UpdateLevelpscost(string name, int level, double cost_update,
                           bool update_count);
    void UpdateLevelpscostDown(string name, int level, double cost_update,
                               bool update_count);

private:
    std::map<string, SCIP_HISTORY> var_histories_;
    std::map<string, double> var_cumpscost_;
    std::map<string, double> var_cumpscost_down_;
    std::map<string, long long> var_cumpscost_count_;
    std::map<string, long long> var_cumpscost_down_count_;

    std::map<string, vector<double>> var_levelpscost_;
    std::map<string, vector<double>> var_levelpscost_down_;
    std::map<string, vector<long long>> var_levelpscost_count_;
    std::map<string, vector<long long>> var_levelpscost_down_count_;
};

#endif
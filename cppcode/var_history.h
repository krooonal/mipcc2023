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

    void SetHistoryResetCount(double count_val)
    {
        history_count_reset_ = count_val;
    }

    double GetCumpscost(string name);
    long long GetCumpscostCount(string name);
    void UpdateCumpscost(string name, double cost_update, bool update_count);
    void UpdateCumpscostDown(string name, double cost_update, bool update_count);

    double GetOldpscost0(string name);
    double GetOldpscostCount0(string name);
    double GetOldpscost1(string name);
    double GetOldpscostCount1(string name);
    void SetOldpscost0(string name, double value);
    void SetOldpscostCount0(string name, double value);
    void SetOldpscost1(string name, double value);
    void SetOldpscostCount1(string name, double value);

    double GetLevelpscost(string name, int level);
    double GetLevelpscost0(string name, int level);
    double GetLevelpscost1(string name, int level);
    double GetLevelpscostCount(string name, int level);
    double GetLevelpscostCount0(string name, int level);
    double GetLevelpscostCount1(string name, int level);
    void UpdateLevelpscost0(string name, int level, double cost_update,
                            double weight);
    void UpdateLevelpscost1(string name, int level, double cost_update,
                            double weight);

private:
    double history_count_reset_ = 4.0;
    SCIP_HISTORY global_history_;
    std::map<string, SCIP_HISTORY> var_histories_;

    std::map<string, double> var_cumpscost_;
    std::map<string, double> var_cumpscost_down_;
    std::map<string, long long> var_cumpscost_count_;
    std::map<string, long long> var_cumpscost_down_count_;

    std::map<string, double> var_oldpscost0_;
    std::map<string, double> var_oldpscost1_;
    std::map<string, double> var_oldpscost0_count_;
    std::map<string, double> var_oldpscost1_count_;

    std::map<string, vector<double>> var_levelpscost0_;
    std::map<string, vector<double>> var_levelpscost1_;
    std::map<string, vector<double>> var_levelpscost0_count_;
    std::map<string, vector<double>> var_levelpscost1_count_;
};

#endif
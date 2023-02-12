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

private:
    double history_count_reset_ = 4.0;
    SCIP_HISTORY global_history_;
    std::map<string, SCIP_HISTORY> var_histories_;
};

#endif
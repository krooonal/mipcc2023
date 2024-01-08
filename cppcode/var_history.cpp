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
    // Get the global history.
    global_history_ = *(scip->stat->glbhistory);
    // Clip the pseudocost count to history_count_reset_ if it is larger.
    global_history_.pscostcount[0] = min(global_history_.pscostcount[0], history_count_reset_);
    global_history_.pscostcount[1] = min(global_history_.pscostcount[1], history_count_reset_);

    // Get history for each variable.
    for (SCIP_VAR *var : scip_variables)
    {
        const string name = SCIPvarGetName(var);
        SCIP_VAR *trans_var = SCIPvarGetTransVar(var);
        SCIP_HISTORY var_history = *(trans_var->history);
        // Clip the pseudocost count to history_count_reset_ if it is larger.
        var_history.pscostcount[0] = min(var_history.pscostcount[0], history_count_reset_);
        var_history.pscostcount[1] = min(var_history.pscostcount[1], history_count_reset_);
        var_histories_[name] = var_history;
    }
}

void VarHistories::AddToModel(SCIP *scip,
                              std::vector<SCIP_VAR *> &scip_variables)
{
    // Load the global history.
    *(scip->stat->glbhistory) = global_history_;
    // Load the history for each variable.
    for (SCIP_VAR *var : scip_variables)
    {
        const string name = SCIPvarGetName(var);
        SCIP_HISTORY var_history = var_histories_.at(name);
        *(var->history) = var_histories_.at(name);
    }
}
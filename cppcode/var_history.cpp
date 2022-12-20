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
        var_histories_[name] = var_history;
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

#include <algorithm>
#include "cuts_pool.h"
#include <scip/struct_cuts.h>
#include "scip/struct_var.h"

using namespace std;

bool cut_sorter(Cut const &c1, Cut const &c2)
{
    if (c1.age == c2.age)
    {
        return c1.active_lps > c2.active_lps;
    }
    return c1.age < c2.age;
}

void CutsPool::CaptureCuts(SCIP *scip, SCIP_SOL *sol)
{
    // Gather cuts.
    SCIP_CUT **scip_cuts;
    scip_cuts = SCIPgetPoolCuts(scip);
    int scip_n_cuts = SCIPgetNPoolCuts(scip);
    std::cout << "Number of cuts: " << scip_n_cuts << endl;
    for (int i = 0; i < scip_n_cuts; ++i)
    {
        Cut cut;
        SCIP_CUT *scip_cut = scip_cuts[i];
        cut.age = SCIPcutGetAge(scip_cut);
        SCIP_ROW *row = SCIPcutGetRow(scip_cut);
        cut.lhs = SCIProwGetLhs(row);
        cut.rhs = SCIProwGetRhs(row);
        cut.active_lps = SCIProwGetActiveLPCount(row);
        SCIP_COL **cols = SCIProwGetCols(row);
        int n_nonzeros = SCIProwGetNNonz(row);
        double *coeffs = SCIProwGetVals(row);
        double expr_val = 0.0;

        for (int j = 0; j < n_nonzeros; ++j)
        {
            double coeff = coeffs[j];
            SCIP_COL *col = cols[j];
            SCIP_VAR *var = SCIPcolGetVar(col);

            while (var->nparentvars >= 1 && SCIPvarGetStatus(var) != SCIP_VARSTATUS_ORIGINAL)
                var = var->parentvars[0];
            string name = SCIPvarGetName(var);
            cut.vars.push_back(name);
            cut.coeffs.push_back(coeff);
            double sol_val = SCIPgetSolVal(scip, sol, var);
            expr_val += coeff * sol_val;
        }
        if (expr_val + 1e-6 < cut.lhs || expr_val > cut.rhs + 1e-6)
        {
            cout << "Cut violated in original space\n";
            cout << cut.lhs << " " << cut.rhs << " " << expr_val << endl;
            assert(false);
        }
        all_cuts_.push_back(cut);
    }
    std::sort(all_cuts_.begin(), all_cuts_.end(), cut_sorter);
    if (all_cuts_.size() > 100)
    {
        all_cuts_.resize(100);
    }
    cout << "Current cut size: " << all_cuts_.size() << endl;
}

void CutsPool::AddCutsToModel(SCIP *scip, std::vector<SCIP_VAR *> &scip_variables)
{
    name_var_.clear();
    for (SCIP_VAR *var : scip_variables)
    {
        const string name = SCIPvarGetName(var);
        name_var_[name] = var;
    }
    int num_cuts = all_cuts_.size();
    int added_cuts_count = 0;
    for (int i = 0; i < min(10, num_cuts); ++i)
    {
        Cut &cut = all_cuts_[i];
        SCIP_CONS *cons;
        string name = "cut_" + std::to_string(i);
        SCIPcreateConsLinear(scip, &cons, name.c_str(), 0, NULL, NULL,
                             cut.lhs, cut.rhs,
                             /*initial=*/false, /*separate=*/TRUE,
                             /*enforce=*/TRUE, /*check=*/false,
                             /*propagate=*/TRUE, /*local=*/FALSE,
                             /*modifiable=*/FALSE, /*dynamic=*/FALSE,
                             /*removable=*/true, /*stickingatnode=*/FALSE);
        for (int j = 0; j < cut.vars.size(); ++j)
        {
            SCIP_VAR *var = name_var_[cut.vars[j]];
            double coeff = cut.coeffs[j];
            SCIPaddCoefLinear(scip, cons, var, coeff);
        }
        SCIPaddCons(scip, cons);
        added_cuts_count++;
        // cout << "Added cut\n";
    }
    cout << "Added cuts: " << added_cuts_count << endl;
}
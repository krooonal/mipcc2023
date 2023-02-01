#include <scip/struct_cuts.h>
#include "scip/struct_var.h"
#include "cuts_pool.h"

using namespace std;

void CutsPool::CaptureCuts(SCIP *scip, SCIP_SOL *sol)
{
    // Gather cuts.
    SCIP_CUT **scip_cuts;
    scip_cuts = SCIPgetPoolCuts(scip);
    int scip_n_cuts = SCIPgetNPoolCuts(scip);
    std::cout << "Number of cuts: " << scip_n_cuts << endl;
    for (int i = 0; i < scip_n_cuts; ++i)
    {
        SCIP_CUT *cut = scip_cuts[i];
        int age = SCIPcutGetAge(cut);
        SCIP_ROW *row = SCIPcutGetRow(cut);
        double lhs = SCIProwGetLhs(row);
        double rhs = SCIProwGetRhs(row);
        SCIP_COL **cols = SCIProwGetCols(row);
        int n_nonzeros = SCIProwGetNNonz(row);
        double *coeffs = SCIProwGetVals(row);
        double expr_val = 0.0;
        for (int j = 0; j < n_nonzeros; ++j)
        {
            double coeff = coeffs[j];
            SCIP_COL *col = cols[j];
            SCIP_VAR *var = SCIPcolGetVar(col);
            // if (var->nparentvars > 0)
            // {
            //     var = var->parentvars[0];
            // }
            string name = SCIPvarGetName(var->parentvars[0]);
            if (i == 0)
            {
                cout << coeff << name << " ";
            }
            double sol_val = SCIPgetSolVal(scip, sol, var);
            expr_val += coeff * sol_val;
        }
        if (i == 0)
        {
            cout << lhs << " " << rhs << endl;
        }
        if (expr_val < lhs || expr_val > rhs)
        {
            cout << lhs << " " << rhs << " " << expr_val << endl;
            assert(false);
        }
    }
}
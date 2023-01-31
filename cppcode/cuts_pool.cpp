#include <scip/struct_cuts.h>
#include "cuts_pool.h"

using namespace std;

void CutsPool::CaptureCuts(SCIP *scip)
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
        double *vals = SCIProwGetVals(row);
        for (int j = 0; j < n_nonzeros; ++j)
        {
            double coeff = vals[j];
            SCIP_COL *col = cols[j];
            SCIP_VAR *var = SCIPcolGetVar(col);
            string name = SCIPvarGetName(var);
            if (i == 0)
            {
                cout << coeff << name << " ";
            }
        }
        if (i == 0)
        {
            cout << lhs << " " << rhs << endl;
        }
    }
}
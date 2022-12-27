#include <iostream>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/struct_var.h>
#include <scip/struct_history.h>
#include "scip/history.h"
#include "scip/sol.h"
#include "scip/struct_stat.h"
#include "scip/struct_scip.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <stdio.h>
#include <time.h>

using namespace std;

SCIP_RETCODE execmain(int argc, const char **argv)
{
    string meta_file_path = argv[1];
    string solution_file = argv[2];
    cout << meta_file_path << endl;
    int timeout = 600;

    SCIP *scip = nullptr;
    SCIP_READER *reader;
    SCIP_CALL(SCIPcreate(&scip)); // Creating the SCIP environment
    // include default plugins.
    SCIP_CALL(SCIPincludeDefaultPlugins(scip));
    // Creating the SCIP Problem.
    SCIP_CALL(SCIPcreateProbBasic(scip, "Reoptimization"));

    // disable scip output to stdout
    // SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip), TRUE);

    SCIP_RESULT *result;
    result = new SCIP_RESULT[3];

    SCIP_CALL(SCIPreadMps(scip, reader, meta_file_path.c_str(), result, NULL, NULL,
                          NULL, NULL, NULL, NULL));

    SCIP_CALL(SCIPsetRealParam(scip, "limits/time", timeout - 2));
    SCIP_CALL(SCIPsetIntParam(scip, "presolving/maxrestarts", 0));

    // Read solution from file.
    if (!solution_file.empty())
        SCIPreadSol(scip, solution_file.c_str());

    // Solve
    SCIP_CALL(SCIPsolve(scip));
    if (SCIPgetNSols(scip) <= 0)
    {
        return SCIP_OKAY;
    }
    SCIP_SOL *sol;
    sol = SCIPgetBestSol(scip);

    // Dual bound.
    double dual_bound = SCIPgetDualbound(scip);
    cout << "[DUALBOUND] " << std::fixed << std::setprecision(9)
         << dual_bound << "\n"
         << std::flush;

    // Statistics
    double relative_gap = SCIPgetGap(scip);
    relative_gap = min(relative_gap, 1.0);
    double time_score = SCIPgetSolvingTime(scip) / timeout;
    if (relative_gap > 1e-6)
    {
        time_score = 1.0;
    }
    double total_score = time_score + relative_gap;
    double first_sol_gap = scip->stat->firstsolgap;
    double first_solution_val = scip->stat->firstprimalbound;
    double best_solution_val = SCIPgetPrimalbound(scip);
    double first_solution_dist = abs(first_solution_val - best_solution_val);
    double first_sol_time = scip->stat->firstprimaltime;
    cout << "Relative gap: " << relative_gap << endl;
    cout << "time_score: " << time_score << endl;
    cout << "first_solution_dist: " << first_solution_dist << endl;
    cout << "first_sol_time: " << first_sol_time << endl;
    cout << "total_score: " << total_score << endl;

    // Solution feedback
    SCIP_HEUR *comp_sol_heur = SCIPfindHeur(scip, "completesol");
    assert(comp_sol_heur != NULL);
    SCIP_Longint comp_sol_calls = SCIPheurGetNCalls(comp_sol_heur);
    SCIP_Longint comp_sol_solns = SCIPheurGetNSolsFound(comp_sol_heur);
    SCIP_Real comp_soln_time = SCIPheurGetTime(comp_sol_heur);
    cout << "Calls " << comp_sol_calls
         << " Solns " << comp_sol_solns
         << " Time " << comp_soln_time
         << endl;

    return SCIP_OKAY;
}

int main(int argc, const char *argv[])
{
    return execmain(argc, argv) != SCIP_OKAY ? 1 : 0;
}

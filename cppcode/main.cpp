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

#include "var_history.h"
#include "solutions.h"
#include "event_solfeedback.h"
#include "parameters.h"

using namespace std;

// Get current date/time, format is YYYY-MM-DDTHH:mm:ss
const std::string CurrentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%dT%X", &tstruct);

    return buf;
}

SCIP_RETCODE execmain(int argc, const char **argv)
{
    string meta_file_path = argv[1];
    cout << meta_file_path << endl;
    int pos = meta_file_path.find("datasets");
    // TODO: Fix this. base_dir is current working directory.
    string base_dir = meta_file_path.substr(0, pos);
    cout << base_dir << endl;
    pos = meta_file_path.find_last_of("/");
    string meta_file_name = meta_file_path.substr(pos + 1, string::npos);
    pos = meta_file_name.find(".");
    string meta_file_name_wo_ext = meta_file_name.substr(0, pos);

    int timeout = 0;
    vector<string> instances;
    ifstream meta_file(meta_file_path);
    if (meta_file.is_open())
    {
        string timeout_str;
        string line;
        getline(meta_file, line);
        stringstream ss(line);
        ss >> timeout_str >> timeout;
        cout << timeout << endl;
        while (getline(meta_file, line))
        {
            instances.push_back(line);
        }

        // DO NOT PRINT INSTANCE NAMES. IT SPOILS THE EVAL.
        // for (string instance : instances)
        // {
        //     cout << instance << endl;
        // }
        meta_file.close();
    }

    SCIP *scip = nullptr;
    SCIP_READER *reader;
    SCIP_CALL(SCIPcreate(&scip)); // Creating the SCIP environment
    // include default plugins.
    SCIP_CALL(SCIPincludeDefaultPlugins(scip));
    // Creating the SCIP Problem.
    SCIP_CALL(SCIPcreateProbBasic(scip, "Reoptimization"));

    // disable scip output to stdout
    SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip), TRUE);

    // Provide prev solution?
    Parameter<bool> provide_hint(0.7, "provide_hint");
    provide_hint.AddValue(true);
    provide_hint.AddValue(false);

    Parameter<int> max_cuts(0.7, "max_cuts");
    max_cuts.AddValue(100);
    max_cuts.AddValue(0);

    // Parameter<int> bfs_priority(0.7, "bfs_priority");
    // bfs_priority.AddValue(100000);
    // bfs_priority.AddValue(300000);

    SCIP_RESULT *result;
    result = new SCIP_RESULT[3];

    SolutionPool solution_pool;
    VarHistories var_histories;

    // Event handler.
    // SCIP_CALL(SCIPincludeEventHdlrSolFeedback(scip, &solution_pool));

    for (int index = 0; index < instances.size(); ++index)
    {
        string instance = instances[index];
        int pos = instance.find_last_of('/');
        string instance_name = instance.substr(pos + 1, string::npos);
        string filename = base_dir + instance;

        cout << "[INSTANCE] " << instance_name << "\n"
             << std::flush;
        // Read in *.MPS file
        SCIP_CALL(SCIPreadMps(scip, reader, filename.c_str(), result, NULL, NULL,
                              NULL, NULL, NULL, NULL));

        // Print the time
        // system("echo -n \"[START] \"; date -Iseconds");
        cout << "[START] " << CurrentDateTime() << "\n"
             << std::flush;

        SCIP_CALL(SCIPsetRealParam(scip, "limits/time", timeout - 2));
        SCIP_CALL(SCIPsetIntParam(scip, "presolving/maxrestarts", 0));
        SCIP_CALL(SCIPsetIntParam(scip, "separating/maxcuts", max_cuts.GetBestValue()));

        SCIP_VAR **vars;
        vars = SCIPgetOrigVars(scip);
        int num_vars = SCIPgetNOrigVars(scip);
        std::vector<SCIP_VAR *> scip_variables;
        for (int i = 0; i < num_vars; ++i)
        {
            SCIP_VAR *var = vars[i];
            scip_variables.push_back(var);
        }
        if (index > 0)
        {
            if (provide_hint.GetBestValue())
            {
                solution_pool.AddToModel(scip, scip_variables);
            }

            solution_pool.SetCurrentScipVars(&scip_variables);
            // for (int i = 0; i < 5; ++i)
            // {
            //     solution_pool.AddNextSolutionToModel(scip);
            // }
            var_histories.AddToModel(scip, scip_variables);
        }

        // Solve
        SCIP_CALL(SCIPsolve(scip));
        // TODO: Write solution
        if (SCIPgetNSols(scip) <= 0)
        {
            continue;
        }
        SCIP_SOL *sol;
        sol = SCIPgetBestSol(scip);

        // Dual bound.
        double dual_bound = SCIPgetDualbound(scip);
        cout << "[DUALBOUND] " << std::fixed << std::setprecision(9)
             << dual_bound << "\n"
             << std::flush;
        // TODO: Add solution to file and pool.
        ofstream solution_file;
        solution_file.open("solutions/" + meta_file_name_wo_ext + "/" + instance_name + ".sol");
        solution_file << "#Writing this to a file.\n"
                      << std::flush;
        for (SCIP_VAR *scip_var : scip_variables)
        {
            const string name = SCIPvarGetName(scip_var);
            const double val = SCIPgetSolVal(scip, sol, scip_var);
            solution_file << name << " " << std::fixed << std::setprecision(9)
                          << val << "\n"
                          << std::flush;
        }
        solution_file.close();
        Solution solution;
        solution.Populate(scip, scip_variables, sol);
        solution_pool.AddSolution(solution);
        var_histories.Populate(scip, scip_variables);

        // system("echo -n \"[END] \";date -Iseconds");
        cout << "[END] " << CurrentDateTime() << "\n"
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
        double adjusted_score = total_score * (1 + 0.1 * index);
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
        cout << "adjusted_score: " << adjusted_score << endl;

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
        if (comp_sol_solns > 0)
        {
            // It automatically provides less hints due to added solutions.
            // solution_pool.ReduceCommonSolFac();
        }
        else
        {
            // TODO.
        }

        // Update parameters
        if (index > 0)
        {
            double hint_score = -total_score;
            if (comp_sol_calls > 0 && comp_sol_solns == 0)
                hint_score -= comp_soln_time;
            provide_hint.AdjustScore(hint_score);
        }
        max_cuts.AdjustScore(-total_score);
    }
    provide_hint.PrintStats();
    max_cuts.PrintStats();

    return SCIP_OKAY;
}

int main(int argc, const char *argv[])
{
    return execmain(argc, argv) != SCIP_OKAY ? 1 : 0;
}

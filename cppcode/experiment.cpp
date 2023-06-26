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
#include <random>

#include "var_history.h"
#include "solutions.h"
#include "parameters.h"

#define DEBUG false

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
    std::cout << meta_file_path << endl;
    int pos = meta_file_path.find("datasets");
    string base_dir = meta_file_path.substr(0, pos);
    std::cout << base_dir << endl;
    pos = meta_file_path.find_last_of("/");
    string meta_file_name = meta_file_path.substr(pos + 1, string::npos);
    pos = meta_file_name.find(".");
    string meta_file_name_wo_ext = meta_file_name.substr(0, pos);

    // Create solutions directory
    string command = "mkdir -p solutions/" + meta_file_name_wo_ext;
    system(command.c_str());

    int timeout = 0;
    bool obj_only_change = true;
    bool rhs_only_change = true;
    bool obj_change = false;
    vector<string> instances;
    ifstream meta_file(meta_file_path);
    if (meta_file.is_open())
    {
        string timeout_str;
        string line;
        getline(meta_file, line);
        stringstream ss(line);
        ss >> timeout_str >> timeout;
        std::cout << timeout << endl;
        getline(meta_file, line); // OBJ
        string obj_str = "[OBJ] ";
        std::string::size_type i = line.find(obj_str);
        if (i != std::string::npos)
            line.erase(i, obj_str.length());
        if (line != "-")
        {
            obj_change = true;
            rhs_only_change = false;
        }

        getline(meta_file, line); // LO
        string lo_str = "[LO] ";
        i = line.find(lo_str);
        if (i != std::string::npos)
            line.erase(i, lo_str.length());
        if (line != "-")
        {
            std::cout << "LO changed " << endl;
            obj_only_change = false;
            rhs_only_change = false;
        }

        getline(meta_file, line); // UP
        string up_str = "[UP] ";
        i = line.find(up_str);
        if (i != std::string::npos)
            line.erase(i, up_str.length());
        if (line != "-")
        {
            std::cout << "UP changed " << endl;
            obj_only_change = false;
            rhs_only_change = false;
        }

        getline(meta_file, line); // LHS
        string lhs_str = "[LHS] ";
        i = line.find(lhs_str);
        if (i != std::string::npos)
            line.erase(i, lhs_str.length());
        if (line != "-")
        {
            std::cout << "LHS changed " << endl;
            obj_only_change = false;
        }

        getline(meta_file, line); // RHS
        string rhs_str = "[RHS] ";
        i = line.find(rhs_str);
        if (i != std::string::npos)
            line.erase(i, rhs_str.length());
        if (line != "-")
        {
            std::cout << "RHS changed " << endl;
            obj_only_change = false;
        }

        getline(meta_file, line); // MAT
        string mat_str = "[MAT] ";
        i = line.find(mat_str);
        if (i != std::string::npos)
            line.erase(i, mat_str.length());
        if (line != "-")
        {
            std::cout << "MAT changed " << endl;
            obj_only_change = false;
            rhs_only_change = false;
        }

        while (getline(meta_file, line))
        {
            instances.push_back(line);
        }
        // DO NOT PRINT INSTANCE NAMES. IT SPOILS THE EVAL.
        meta_file.close();
    }

    SCIP *scip = nullptr;
    SCIP_READER *reader;
    SCIP_CALL(SCIPcreate(&scip)); // Creating the SCIP environment
    // include default plugins.
    SCIP_CALL(SCIPincludeDefaultPlugins(scip));
    std::cout << "SCIP Version: " << SCIPversion() << endl;
    // Creating the SCIP Problem.
    SCIP_CALL(SCIPcreateProbBasic(scip, "Reoptimization"));

    // disable scip output to stdout
    SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip), TRUE);

    // Parameters to tune. Use fixed seed for reproducibility.
    // Provide prev solution?
    Parameter<bool> provide_hint(0.3, "provide_hint", /*seed=*/56166895);
    provide_hint.SetExploreCount(8); // Hints are not always converted.
    provide_hint.SetSwitchFlag(1 << 0);
    provide_hint.AddValue(true);
    provide_hint.AddValue(false);
    int hint_success = 0;
    int hint_total = 0;

    // Tune after index 4.
    Parameter<int> max_cuts(0.3, "max_cuts", /*seed=*/984321);
    max_cuts.SetExploreCount(8);
    max_cuts.SetSwitchFlag(1 << 1);
    max_cuts.AddValue(100);
    max_cuts.AddValue(0);

    // Tune after index 4.
    Parameter<int> max_cuts_root(0.3, "max_cuts_root", /*seed=*/5198413);
    max_cuts_root.SetExploreCount(8);
    max_cuts_root.SetSwitchFlag(1 << 2);
    max_cuts_root.AddValue(2000);
    max_cuts_root.AddValue(0);

    SCIP_RESULT *result;
    result = new SCIP_RESULT[3];

    SolutionPool solution_pool;
    VarHistories var_histories;
    if (obj_only_change)
    {
        // All previous solutions are feasible. Spend less time exploring them.
        solution_pool.SetNumHintSolns(5); // Default is 9.
    }

    for (int index = 0; index < instances.size(); ++index)
    {
        string instance = instances[index];
        int pos = instance.find_last_of('/');
        string instance_name = instance.substr(pos + 1, string::npos);
        string filename = instance;

        std::cout << "[INSTANCE] " << instance_name << "\n"
                  << std::flush;
        // Read in *.MPS file
        SCIP_CALL(SCIPreadMps(scip, reader, filename.c_str(), result, NULL, NULL,
                              NULL, NULL, NULL, NULL));

        // Print the time
        // system("echo -n \"[START] \"; date -Iseconds");
        std::cout << "[START] " << CurrentDateTime() << "\n"
                  << std::flush;

        // Use little less than the time limit to ensure that we have
        // sufficient time left for stats collection and solution writing.
        // TODO: use timeout -1
        SCIP_CALL(SCIPsetRealParam(scip, "limits/time", timeout - 1));

        if (index == 0)
        {
            // Solve first instance with pure strong branching.
            SCIP_CALL(SCIPsetIntParam(scip, "branching/fullstrong/priority", 40000)); // default 0
        }
        if (index > 0)
        {
            // Back to normal.
            SCIP_CALL(SCIPsetIntParam(scip, "branching/fullstrong/priority", 0)); // default 0
            // Increase the efforts of completesol heuristic.
            if (!obj_only_change)
            {
                // Regular efforts works if only the objective has changed. Otherwise, increase the effort.
                SCIP_CALL(SCIPsetLongintParam(scip, "heuristics/completesol/nodesofs", 5000)); // default 500
                SCIP_CALL(SCIPsetIntParam(scip, "heuristics/completesol/solutions", -1));      // default 5
            }
            if (index > 4)
            {
                // Tune some parameters after a delay.
                SCIP_CALL(SCIPsetIntParam(scip, "separating/maxcuts", max_cuts.GetBestValue()));
                SCIP_CALL(SCIPsetIntParam(scip, "separating/maxcutsroot", max_cuts_root.GetBestValue()));
            }
            if (rhs_only_change && index > 4)
            {
                // Pseudocosts are well trained if objective and bounds are not changed.
                SCIP_CALL(SCIPsetIntParam(scip, "branching/pscost/priority", 40000)); // default 2000
            }
            // Disable restarts. We are already using enough information from previous instances.
            SCIP_CALL(SCIPsetIntParam(scip, "presolving/maxrestarts", 0));
        }

        // Store the original model variables.
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
            var_histories.AddToModel(scip, scip_variables);
        }

        // Solve
        SCIP_CALL(SCIPsolve(scip));
        // Write solution
        if (SCIPgetNSols(scip) <= 0)
        {
            continue;
        }
        SCIP_SOL *sol;
        sol = SCIPgetBestSol(scip);

        // Dual bound.
        double dual_bound = SCIPgetDualbound(scip);
        std::cout << "[DUALBOUND] " << std::fixed << std::setprecision(9)
                  << dual_bound << "\n"
                  << std::flush;
        // Add solution to file and pool.
        ofstream solution_file;
        solution_file.open("solutions/" + meta_file_name_wo_ext + "/" + instance_name + ".sol");
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

        // Record solve statistics
        // TODO: Compute as per competition rules.
        double primal_bound = SCIPgetPrimalbound(scip);
        // double relative_gap = SCIPgetGap(scip);
        // relative_gap = min(relative_gap, 1.0);
        double relative_gap = 1.0;
        if (SCIPisFinite(primal_bound) && SCIPisFinite(dual_bound) && primal_bound * dual_bound >= 0)
        {
            if (primal_bound == 0 && dual_bound == 0)
            {
                relative_gap = 0.0;
            }
            else
            {
                relative_gap = abs(primal_bound - dual_bound) / max(abs(primal_bound), abs(dual_bound));
            }
        }
        double time_score = SCIPgetSolvingTime(scip) / timeout;
        if (relative_gap > 1e-6)
        {
            time_score = 1.0;
        }
        double total_score = time_score + relative_gap;
        if (DEBUG)
        {
            double adjusted_score = total_score * (1 + 0.1 * index);
            double first_sol_gap = scip->stat->firstsolgap;
            double first_solution_val = scip->stat->firstprimalbound;
            double best_solution_val = SCIPgetPrimalbound(scip);
            double first_solution_dist = abs(first_solution_val - best_solution_val);
            double first_sol_time = scip->stat->firstprimaltime;
            std::cout << "Relative gap: " << relative_gap << endl;
            std::cout << "time_score: " << time_score << endl;
            std::cout << "first_solution_dist: " << first_solution_dist << endl;
            std::cout << "first_sol_time: " << first_sol_time << endl;
            std::cout << "total_score: " << total_score << endl;
            std::cout << "adjusted_score: " << adjusted_score << endl;
            std::cout << "Number of runs: " << scip->stat->nruns << endl;
        }

        // Solution feedback
        SCIP_HEUR *comp_sol_heur = SCIPfindHeur(scip, "completesol");
        assert(comp_sol_heur != NULL);
        SCIP_Longint comp_sol_calls = SCIPheurGetNCalls(comp_sol_heur);
        SCIP_Longint comp_sol_solns = SCIPheurGetNSolsFound(comp_sol_heur);
        SCIP_Real comp_soln_time = SCIPheurGetTime(comp_sol_heur);
        if (DEBUG)
        {
            std::cout << "Calls " << comp_sol_calls
                      << " Solns " << comp_sol_solns
                      << " Time " << comp_soln_time
                      << endl;
        }
        if (comp_sol_calls > 0)
        {
            hint_total++;
            if (comp_sol_solns > 0)
            {
                hint_success++;
            }
        }

        // Update parameter scores.
        if (index > 0)
        {
            if (provide_hint.GetCurrentIndex() == 0) // True value for hint.
            {
                if (comp_sol_solns > 0)
                {
                    provide_hint.AdjustScore(-total_score);
                }
                else
                {
                    // Update the false value of hint if hint is not successful.
                    provide_hint.AdjustScore(-total_score, 1);
                }
            }
            else
            {
                provide_hint.AdjustScore(-total_score);
            }
            // Cut tuning starts at index 4, but we can still
            // update the scores as the first value is the default value.
            max_cuts.AdjustScore(-total_score);
            max_cuts_root.AdjustScore(-total_score);
        }
        std::cout << "[END] " << CurrentDateTime() << "\n\n"
                  << std::flush;
        if (index % 10 == 9)
        {
            provide_hint.PrintStats();
            max_cuts.PrintStats();
            max_cuts_root.PrintStats();
        }
    }
    provide_hint.PrintStats();
    max_cuts.PrintStats();
    max_cuts_root.PrintStats();
    std::cout << "Provided hints: " << hint_total
              << " successful hints: " << hint_success << endl;

    return SCIP_OKAY;
}

int main(int argc, const char *argv[])
{
    return execmain(argc, argv) != SCIP_OKAY ? 1 : 0;
}

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

struct HeuristicStats
{
    string name;
    long long n_calls = 0ll;
    int n_solns = 0;
    int n_best_solns = 0;
    double time_spent = 0.0;

    void printstat()
    {
        std::cout << setw(25) << left << name
                  << "calls " << setw(10) << left << n_calls
                  << " solns " << setw(5) << left << n_solns
                  << " bestsolns " << setw(5) << left << n_best_solns
                  << " time " << time_spent
                  << endl;
    }
};

struct BranchingStats
{
    string name;
    long long n_calls = 0ll;
    int max_depth = 0;
    double time_spent = 0.0;

    void printstat()
    {
        std::cout << setw(25) << left << name
                  << "calls " << setw(10) << left << n_calls
                  << " maxdepth " << setw(10) << left << max_depth
                  << " time " << time_spent
                  << endl;
    }
};

struct PresolveStats
{
    string name;
    long long n_calls = 0ll;
    int total_changes = 0;
    double time_spent = 0.0;

    void printstat()
    {
        std::cout << setw(25) << left << name
                  << "calls " << setw(10) << left << n_calls
                  << " changes " << setw(10) << left << total_changes
                  << " time " << time_spent
                  << endl;
    }
};

struct SpearatorStats
{
    string name;
    long long n_calls = 0ll;
    long long cuts_found = 0ll;
    long long cuts_added = 0ll;
    double time_spent = 0.0;

    void printstat()
    {
        std::cout << setw(25) << left << name
                  << "calls " << setw(10) << left << n_calls
                  << " cuts_found " << setw(10) << left << cuts_found
                  << " cuts_added " << setw(10) << left << cuts_added
                  << " time " << time_spent
                  << endl;
    }
};

SCIP_RETCODE execmain(int argc, const char **argv)
{
    // The meta file contains the list of all instance file paths in the series.
    string meta_file_path = argv[1];
    std::cout << meta_file_path << endl;

    // Extract the series name from the meta file path. We assume that the meta
    // file is present in the directory 'datasets'.
    int pos = meta_file_path.find("datasets");
    string base_dir = meta_file_path.substr(0, pos);
    std::cout << base_dir << endl;
    pos = meta_file_path.find_last_of("/");
    string meta_file_name = meta_file_path.substr(pos + 1, string::npos);
    pos = meta_file_name.find(".");
    string meta_file_name_wo_ext = meta_file_name.substr(0, pos);

    // Create solutions directory using the series name.
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
        // The first line in the file contains the time limit to solve each instance.
        string timeout_str;
        string line;
        getline(meta_file, line);
        stringstream ss(line);
        ss >> timeout_str >> timeout;
        std::cout << timeout << endl;

        // The following 7 lines contain the information about the changing
        // components (OBJ, LO, UP, LHS, RHS, and MAT). In this code, we do not
        // use the detailed information about changing components i.e. which
        // variables/constraints are changing. We only need to know which
        // component is changing.

        // Objective change.
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

        // Variable lower bound change.
        getline(meta_file, line); // LO
        string lo_str = "[LO] ";
        i = line.find(lo_str);
        if (i != std::string::npos)
            line.erase(i, lo_str.length());
        if (line != "-")
        {
            obj_only_change = false;
            rhs_only_change = false;
        }

        // Variable upper bound change.
        getline(meta_file, line); // UP
        string up_str = "[UP] ";
        i = line.find(up_str);
        if (i != std::string::npos)
            line.erase(i, up_str.length());
        if (line != "-")
        {
            obj_only_change = false;
            rhs_only_change = false;
        }

        // Constraint lower bound change.
        getline(meta_file, line); // LHS
        string lhs_str = "[LHS] ";
        i = line.find(lhs_str);
        if (i != std::string::npos)
            line.erase(i, lhs_str.length());
        if (line != "-")
        {
            obj_only_change = false;
        }

        // Constraint upper bound change.
        getline(meta_file, line); // RHS
        string rhs_str = "[RHS] ";
        i = line.find(rhs_str);
        if (i != std::string::npos)
            line.erase(i, rhs_str.length());
        if (line != "-")
        {
            obj_only_change = false;
        }

        // Constraint coefficients change.
        getline(meta_file, line); // MAT
        string mat_str = "[MAT] ";
        i = line.find(mat_str);
        if (i != std::string::npos)
            line.erase(i, mat_str.length());
        if (line != "-")
        {
            obj_only_change = false;
            rhs_only_change = false;
        }

        // The next 50 lines are the paths of instnace files in this series.
        while (getline(meta_file, line))
        {
            instances.push_back(line);
        }
        // WARNING: DO NOT PRINT INSTANCE NAMES. IT SPOILS THE EVAL.
        meta_file.close();
    }

    SCIP *scip = nullptr;
    SCIP_READER *reader;
    // Create the SCIP environment.
    SCIP_CALL(SCIPcreate(&scip));
    // include default plugins.
    SCIP_CALL(SCIPincludeDefaultPlugins(scip));
    std::cout << "SCIP Version: " << SCIPversion() << endl;
    // Create the SCIP Problem.
    SCIP_CALL(SCIPcreateProbBasic(scip, "Reoptimization"));

    // Disable scip output to stdout.
    SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip), TRUE);

    // Parameters to tune. Use fixed seed for reproducibility.
    // Provide prev solution? Hints are not always converted.
    Parameter<bool> provide_hint(0.3, "provide_hint", /*seed=*/56166895);
    provide_hint.SetExploreCount(8);
    provide_hint.SetSwitchFlag(1 << 0);
    provide_hint.AddValue(true); // Default value.
    provide_hint.AddValue(false);
    int hint_success = 0;
    int hint_total = 0;

    // Use cuts? Tune after index 4.
    Parameter<int> max_cuts(0.3, "max_cuts", /*seed=*/984321);
    max_cuts.SetExploreCount(8);
    max_cuts.SetSwitchFlag(1 << 1);
    max_cuts.AddValue(100); // Default value.
    max_cuts.AddValue(0);

    // Use root node cuts? Tune after index 4.
    Parameter<int> max_cuts_root(0.3, "max_cuts_root", /*seed=*/5198413);
    max_cuts_root.SetExploreCount(8);
    max_cuts_root.SetSwitchFlag(1 << 2);
    max_cuts_root.AddValue(2000); // Default value.
    max_cuts_root.AddValue(0);

    SCIP_RESULT *result;
    result = new SCIP_RESULT[3];

    // Store best found solution for each instance.
    SolutionPool solution_pool;
    // Store branching histories for each variable.
    VarHistories var_histories;
    if (obj_only_change)
    {
        // All previous solutions are feasible. Spend less time exploring them.
        solution_pool.SetNumHintSolns(5); // Default is 9.
    }

    // Store statistics about heurisitcs, branhcing, presolvers, and cuts.
    // The keys are names of the component and the values are their aggregated
    // statistics collected from the solver.
    std::map<string, HeuristicStats> heuristic_stats;
    std::map<string, BranchingStats> branching_stats;
    std::map<string, PresolveStats> presolve_stats;
    std::map<string, SpearatorStats> sepa_stats;

    // Solve each instance sequentially.
    for (int index = 0; index < instances.size(); ++index)
    {
        // Extract the instnace name.
        string instance = instances[index];
        int pos = instance.find_last_of('/');
        string instance_name = instance.substr(pos + 1, string::npos);
        string filename = instance;

        // Print the instance name.
        std::cout << "[INSTANCE] " << instance_name << "\n"
                  << std::flush;
        // Read in *.MPS file
        SCIP_CALL(SCIPreadMps(scip, reader, filename.c_str(), result, NULL, NULL,
                              NULL, NULL, NULL, NULL));

        // Print the start time for solving.
        std::cout << "[START] " << CurrentDateTime() << "\n"
                  << std::flush;

        // Use little less than the time limit to ensure that we have
        // sufficient time left for stats collection and solution writing.
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
                // No need to use reliability branching.
                SCIP_CALL(SCIPsetIntParam(scip, "branching/pscost/priority", 40000)); // default 2000
            }
            // Disable restarts. We are already using enough information from previous instances.
            SCIP_CALL(SCIPsetIntParam(scip, "presolving/maxrestarts", 0));
        }
        if (index >= 25)
        {
            // Turn off non performing heuristics.
            for (auto heuristic : heuristic_stats)
            {
                int num_best_solns = max(1, heuristic.second.n_best_solns);
                double time_per_best_soln = heuristic.second.time_spent / num_best_solns;
                if (heuristic.second.n_solns == 0)
                {
                    // It failed to find any solution in any instance so far.
                    string heuristic_param = "heuristics/" + heuristic.first + "/freq";
                    SCIP_CALL(SCIPsetIntParam(scip, heuristic_param.c_str(), -1));
                }
                else if (time_per_best_soln / timeout > 0.2)
                {
                    // It found solutions, but spent too much time for it.
                    string heuristic_param = "heuristics/" + heuristic.first + "/freq";
                    SCIP_CALL(SCIPsetIntParam(scip, heuristic_param.c_str(), -1));
                }
            }

            // // Turn off non performing separators
            for (auto separator : sepa_stats)
            {
                if (separator.second.cuts_added == 0 && separator.second.time_spent > 1.0)
                {
                    // No cuts have been added by this separator and it has spent too much time.
                    string spea_param = "separating/" + separator.first + "/freq";
                    SCIP_CALL(SCIPsetIntParam(scip, spea_param.c_str(), -1));
                }
            }
        }

        if (index >= 15)
        {
            // Turn off non performing presolves
            for (auto presolve : presolve_stats)
            {
                if (presolve.second.total_changes == 0 && presolve.second.time_spent > 2.0)
                {
                    // No changes have been made by this presolve rule and it has spent too much time.
                    string presolve_param = "presolving/" + presolve.first + "/maxrounds";
                    SCIP_CALL(SCIPsetIntParam(scip, presolve_param.c_str(), 0));
                }
            }
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

        // After the first instance, use the previous instance solutions as hints (if parameter tuning allows it)
        // and warm start the pseudocosts using the branching history of previous instance.
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

        // Print the dual bound.
        double dual_bound = SCIPgetDualbound(scip);
        std::cout << "[DUALBOUND] " << std::fixed << std::setprecision(9)
                  << dual_bound << "\n"
                  << std::flush;

        // Write solution
        if (SCIPgetNSols(scip) <= 0)
        {
            // TODO: Throw an error when no solution was found?
            continue;
        }
        SCIP_SOL *sol;
        sol = SCIPgetBestSol(scip);

        // Write solution to file.
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

        // Add solution to the solution pool.
        Solution solution;
        solution.Populate(scip, scip_variables, sol);
        solution_pool.AddSolution(solution);
        var_histories.Populate(scip, scip_variables);

        // Record solve statistics.
        // Compute scores as per competition rules. The total score is used for parameter tuning.
        double primal_bound = SCIPgetPrimalbound(scip);
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

        // Record branching stats. This is only used for observations and not used
        // for tuning the performance of the solver.
        SCIP_BRANCHRULE **branch_rules = SCIPgetBranchrules(scip);
        int n_branch_rules = SCIPgetNBranchrules(scip);
        for (int i = 0; i < n_branch_rules; ++i)
        {
            int n_calls = SCIPbranchruleGetNLPCalls(branch_rules[i]);
            if (n_calls > 0)
            {
                string name = SCIPbranchruleGetName(branch_rules[i]);
                double time_spent = SCIPbranchruleGetTime(branch_rules[i]);
                branching_stats[name].name = name;
                branching_stats[name].n_calls += n_calls;
                branching_stats[name].time_spent += time_spent;
            }
        }

        // Record heuristic stats.
        SCIP_HEUR **heuristics = SCIPgetHeurs(scip);
        int n_heuristics = SCIPgetNHeurs(scip);
        for (int i = 0; i < n_heuristics; ++i)
        {
            int n_calls = SCIPheurGetNCalls(heuristics[i]);
            if (n_calls > 0)
            {
                string name = SCIPheurGetName(heuristics[i]);
                heuristic_stats[name].name = name;
                heuristic_stats[name].n_calls += n_calls;
                heuristic_stats[name].n_solns += SCIPheurGetNSolsFound(heuristics[i]);
                heuristic_stats[name].n_best_solns += SCIPheurGetNBestSolsFound(heuristics[i]);
                heuristic_stats[name].time_spent += SCIPheurGetTime(heuristics[i]);
            }
        }

        // Record Presolve stats.
        SCIP_PRESOL **presolvers = SCIPgetPresols(scip);
        int n_presolvers = SCIPgetNPresols(scip);
        for (int i = 0; i < n_presolvers; ++i)
        {
            int n_calls = SCIPpresolGetNCalls(presolvers[i]);
            if (n_calls > 0)
            {
                string name = SCIPpresolGetName(presolvers[i]);
                presolve_stats[name].name = name;
                presolve_stats[name].time_spent += SCIPpresolGetTime(presolvers[i]);
                presolve_stats[name].n_calls += n_calls;
                int total_changes = SCIPpresolGetNChgBds(presolvers[i]) +
                                    SCIPpresolGetNAddConss(presolvers[i]) +
                                    SCIPpresolGetNAddHoles(presolvers[i]) +
                                    SCIPpresolGetNAggrVars(presolvers[i]) +
                                    SCIPpresolGetNChgCoefs(presolvers[i]) +
                                    SCIPpresolGetNChgSides(presolvers[i]) +
                                    SCIPpresolGetNChgVarTypes(presolvers[i]) +
                                    SCIPpresolGetNDelConss(presolvers[i]) +
                                    SCIPpresolGetNFixedVars(presolvers[i]) +
                                    SCIPpresolGetNUpgdConss(presolvers[i]);
                presolve_stats[name].total_changes += total_changes;
            }
        }

        // Record Cuts stats.
        SCIP_SEPA **separators = SCIPgetSepas(scip);
        int n_sepas = SCIPgetNSepas(scip);
        for (int i = 0; i < n_sepas; ++i)
        {
            int n_calls = SCIPsepaGetNCalls(separators[i]);
            if (n_calls > 0)
            {
                string name = SCIPsepaGetName(separators[i]);
                sepa_stats[name].name = name;
                sepa_stats[name].n_calls += n_calls;
                sepa_stats[name].time_spent += SCIPsepaGetTime(separators[i]);
                sepa_stats[name].cuts_found += SCIPsepaGetNCutsFound(separators[i]);
                sepa_stats[name].cuts_added += SCIPsepaGetNCutsApplied(separators[i]);
            }
        }

        // Solution hint feedback.
        // Solution hints are processed by completesol heuristic. We extract the
        // statistics of this heuristic to know if the hint was converted into
        // a feasible solution.
        SCIP_HEUR *comp_sol_heur = SCIPfindHeur(scip, "completesol");
        assert(comp_sol_heur != NULL);
        SCIP_Longint comp_sol_calls = SCIPheurGetNCalls(comp_sol_heur);
        SCIP_Longint comp_sol_solns = SCIPheurGetNSolsFound(comp_sol_heur);
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
                // The hint was provided. We update the true value if the hint
                // was converted to a feasible solution. Othwerwise, we update
                // the score of false value (equivalent to not providing the hint).
                if (comp_sol_solns > 0)
                {
                    provide_hint.AdjustScore(-total_score);
                }
                else
                {
                    provide_hint.AdjustScore(-total_score, 1);
                }
            }
            else // The hint was not provided.
            {
                provide_hint.AdjustScore(-total_score);
            }
            // Cut tuning starts at index 4, but we can still
            // update the scores as the first value is the default value.
            max_cuts.AdjustScore(-total_score);
            max_cuts_root.AdjustScore(-total_score);
        }

        // Print the end time for this instance.
        std::cout << "[END] " << CurrentDateTime() << "\n\n"
                  << std::flush;
    }
    // Print the parameter tuning stats after solving all instances.
    provide_hint.PrintStats();
    max_cuts.PrintStats();
    max_cuts_root.PrintStats();
    std::cout << "Provided hints: " << hint_total
              << " successful hints: " << hint_success << endl;

    // Also print aggregated stats for different solver components.
    for (auto branching : branching_stats)
    {
        branching.second.printstat();
    }

    for (auto heuristic : heuristic_stats)
    {
        heuristic.second.printstat();
    }

    for (auto presolve : presolve_stats)
    {
        presolve.second.printstat();
    }

    for (auto separator : sepa_stats)
    {
        separator.second.printstat();
    }

    return SCIP_OKAY;
}

int main(int argc, const char *argv[])
{
    return execmain(argc, argv) != SCIP_OKAY ? 1 : 0;
}

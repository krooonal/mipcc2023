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
    void Populate(SCIP *scip, std::vector<SCIP_VAR *> &scip_variables)
    {
        for (SCIP_VAR *var : scip_variables)
        {
            var_histories_.clear();
            const string name = SCIPvarGetName(var);
            SCIP_VAR *trans_var = SCIPvarGetTransVar(var);
            SCIP_HISTORY var_history = *(trans_var->history);
            var_histories_[name] = var_history;
            if (var_history.pscostweightedmean[0] > 0)
            {
                cout << name << " " << var_history.pscostweightedmean[0]
                     << "\n";
            }
        }
    }

    void AddToModel(SCIP *scip, std::vector<SCIP_VAR *> &scip_variables)
    {
        for (SCIP_VAR *var : scip_variables)
        {
            const string name = SCIPvarGetName(var);
            // SCIP_VAR *trans_var = SCIPvarGetTransVar(var);
            SCIP_HISTORY var_history = var_histories_.at(name);
            // SCIPhistoryUnite(var->history, &var_history, FALSE);
            var->history = &var_histories_.at(name);
            if (var_history.pscostweightedmean[0] > 0)
            {
                cout << name << " " << var->history->pscostweightedmean[0]
                     << "\n";
            }
        }
    }

private:
    std::map<string, SCIP_HISTORY> var_histories_;
};

class Solution
{
public:
    void Populate(SCIP *scip, const std::vector<SCIP_VAR *> &scip_variables,
                  SCIP_SOL *solution)
    {
        for (SCIP_VAR *var : scip_variables)
        {
            const string name = SCIPvarGetName(var);
            const double val = SCIPgetSolVal(scip, solution, var);
            varvalues_[name] = val;
        }
    }

    SCIP_RETCODE AddToModel(SCIP *scip, std::vector<SCIP_VAR *> &scip_variables)
    {
        SCIP_SOL *solution;
        SCIP_CALL(SCIPcreatePartialSol(scip, &solution, NULL));
        SCIP_VAR **vars;
        vars = SCIPgetOrigVars(scip);
        int num_vars = SCIPgetNOrigVars(scip);
        for (SCIP_VAR *var : scip_variables)
        {
            const string name = SCIPvarGetName(var);
            double val = varvalues_.at(name);
            SCIP_CALL(SCIPsetSolVal(scip, solution, var, val));
        }
        SCIP_Bool is_stored;
        SCIP_CALL(SCIPaddSolFree(scip, &solution, &is_stored));
        if (is_stored)
        {
            cout << "Added a partial solution\n";
            // cout << "Number of partial solutions: " << SCIPgetNPartialSols(scip) << "\n";
        }
        return SCIP_OKAY;
    }
    std::map<string, double> GetVarValue()
    {
        return varvalues_;
    }

private:
    std::map<string, double> varvalues_;
    double fscore_ = 0.0;
};

class SolutionPool
{
public:
    void AddSolution(Solution solution)
    {
        solutions_.push_back(solution);
        std::map<string, double> varvalues = solution.GetVarValue();
        for (auto var_val : varvalues)
        {
            string var_name = var_val.first;
            double value = var_val.second;
            if (varvaluefreq_.find(var_name) == varvaluefreq_.end())
            {
                varvaluefreq_[var_name] = 1;
            }
            else
            {
                varvaluefreq_[var_name] += 1;
            }
        }
    }

    SCIP_RETCODE AddToModel(SCIP *scip, std::vector<SCIP_VAR *> &scip_variables)
    {
        for (Solution solution : solutions_)
        {
            SCIP_CALL(solution.AddToModel(scip, scip_variables));
        }
        return SCIP_OKAY;
    }

private:
    std::vector<Solution> solutions_;
    std::map<string, double> varvaluefreq_;
};

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
    SCIP_RESULT *result;
    result = new SCIP_RESULT[3];

    SolutionPool solution_pool;
    VarHistories var_histories;

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

        SCIP_CALL(SCIPsetRealParam(scip, "limits/time", timeout - 1));
        SCIP_CALL(SCIPsetIntParam(scip, "presolving/maxrestarts", 0));
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
            solution_pool.AddToModel(scip, scip_variables);
            var_histories.AddToModel(scip, scip_variables);
        }

        // Print the time
        // cout << "[START] ";
        // system("echo -n \"[START] \"; date -Iseconds");
        cout << "[START] " << CurrentDateTime() << "\n"
             << std::flush;

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

        // cout << "[END] ";
        // system("echo -n \"[END] \";date -Iseconds");
        cout << "[END] " << CurrentDateTime() << "\n"
             << std::flush;
    }

    return SCIP_OKAY;
}

int main(int argc, const char *argv[])
{
    return execmain(argc, argv) != SCIP_OKAY ? 1 : 0;
}

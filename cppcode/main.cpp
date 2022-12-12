#include <iostream>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

using namespace std;

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

        for (string instance : instances)
        {
            cout << instance << endl;
        }
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
    // SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip), TRUE);
    SCIP_RESULT *result;
    result = new SCIP_RESULT[3];

    SolutionPool solution_pool;

    for (int index = 0; index < instances.size(); ++index)
    {
        string instance = instances[index];
        string filename = base_dir + instance;
        // Read in *.MPS file
        SCIP_CALL(SCIPreadMps(scip, reader, filename.c_str(), result, NULL, NULL,
                              NULL, NULL, NULL, NULL));

        SCIP_CALL(SCIPsetRealParam(scip, "limits/time", timeout - 5));
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
            // TODO: Add previous solution.
            solution_pool.AddToModel(scip, scip_variables);
        }

        // Print the time
        cout << "[START] ";
        system("date -Iseconds");

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
        cout << "[DUALBOUND] " << dual_bound << "\n";
        // TODO: Add solution to file and pool.
        ofstream solution_file;
        solution_file.open("solutions/" + meta_file_name + ".sol");
        solution_file << "#Writing this to a file.\n";
        for (SCIP_VAR *scip_var : scip_variables)
        {
            const string name = SCIPvarGetName(scip_var);
            const double val = SCIPgetSolVal(scip, sol, scip_var);
            solution_file << name << " " << val << "\n";
        }
        Solution solution;
        solution.Populate(scip, scip_variables, sol);
        solution_pool.AddSolution(solution);
        solution_file.close();

        cout << "[END] ";
        system("date -Iseconds");
    }

    return SCIP_OKAY;
}

int main(int argc, const char *argv[])
{
    return execmain(argc, argv) != SCIP_OKAY ? 1 : 0;
}

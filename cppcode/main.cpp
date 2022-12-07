#include <iostream>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;


SCIP_RETCODE execmain(int argc, const char **argv)
{
    string meta_file_name = argv[1];
    cout << meta_file_name << endl;
    int pos = meta_file_name.find("datasets");
    string base_dir = meta_file_name.substr(0, pos);
    cout << base_dir << endl;

    int timeout = 0;
    vector<string> instances;
    ifstream meta_file(meta_file_name);
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

    // TODO: ADD TIME LIMIT
    //
    // disable scip output to stdout
    // SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip), TRUE);
    SCIP_RESULT *result;
    result = new SCIP_RESULT[3];

    for (string instance : instances)
    {
        string filename = base_dir + instance;
        // Read in *.MPS file
        SCIP_CALL(SCIPreadMps(scip, reader, filename.c_str(), result, NULL, NULL,
                    NULL, NULL, NULL, NULL));
        
        // Print the time
        system("date -Iseconds");

        // Solve
        SCIP_CALL(SCIPsolve(scip));
        // TODO: Write solution
        SCIP_VAR **vars;
        vars = SCIPgetVars(scip);
        SCIP_SOL *sol;
        sol = SCIPgetBestSol(scip);
        system("date -Iseconds");
    }

    return SCIP_OKAY;
}

int main(int argc, const char *argv[])
{
    return execmain(argc, argv) != SCIP_OKAY ? 1 : 0;
}

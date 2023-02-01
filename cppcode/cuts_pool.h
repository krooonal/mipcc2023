#ifndef __KP_CUTS_H__
#define __KP_CUTS_H__
#include <iostream>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/struct_var.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <stdio.h>
#include <time.h>

using namespace std;

struct Cut
{
    std::vector<string> vars;
    std::vector<double> coeffs;
    double lhs;
    double rhs;
    double age;
    long long active_lps;
};
class CutsPool
{
public:
    void CaptureCuts(SCIP *scip, SCIP_SOL *sol);
    // void AddCutsToModel(SCIP *scip);

private:
    std::map<string, SCIP_VAR *> name_var_;
    std::vector<Cut> all_cuts_;
};

#endif
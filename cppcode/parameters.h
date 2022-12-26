#ifndef __KP_PARAMETERS_H__
#define __KP_PARAMETERS_H__

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

template <typename T>
class Parameter
{
public:
    Parameter(double impact_factor, string name);
    void AddValue(T value);
    void AdjustScore(double score);
    T GetBestValue();

private:
    double impact_factor_ = 1.0;
    string name_ = "";
    std::vector<T> values_;
    std::vector<double> scores_;
    std::vector<double> final_scores_;
    std::vector<int> counts_;
    double c_fac_ = 0.3;
    int current_index_ = 0;
    int total_counts_ = 0;
};
#endif
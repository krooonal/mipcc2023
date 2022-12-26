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

template <typename T>
Parameter<T>::Parameter(double impact_factor, string name)
{
    name_ = name;
    impact_factor_ = impact_factor;
    c_fac_ = 1.0 - impact_factor_;
}

template <typename T>
void Parameter<T>::AdjustScore(double score)
{
    scores_[current_index_] = (scores_[current_index_] * counts_[current_index_] + score) / (counts_[current_index_] + 1);
    counts_[current_index_] += 1;
    total_counts_ += 1;
    final_scores_[current_index_] = scores_[current_index_] + (c_fac_ / counts_[current_index_]);
    cout << name_ << ": Updated score of " << values_[current_index_]
         << " by " << score << endl;
}

template <typename T>
void Parameter<T>::AddValue(T value)
{
    values_.push_back(value);
    scores_.push_back(0.0);
    // High scores for the value that hasn't been tried yet.
    final_scores_.push_back(5.0);
    counts_.push_back(0);
}

template <typename T>
T Parameter<T>::GetBestValue()
{
    int best_index = 0;
    for (int i = 0; i < values_.size(); ++i)
    {
        if (final_scores_[i] > final_scores_[best_index])
        {
            best_index = i;
        }
    }
    current_index_ = best_index;
    cout << name_ << ": Trying value: " << values_[best_index]
         << " at index: " << best_index
         << " Score: " << final_scores_[best_index]
         << endl;
    return values_[best_index];
}
#endif
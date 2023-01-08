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
    void AdjustScore(double score, int index);
    T GetBestValue();
    int GetCurrentIndex()
    {
        return current_index_;
    }
    void PrintStats();

private:
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
Parameter<T>::Parameter(double c_fac, string name)
{
    name_ = name;
    c_fac_ = c_fac;
}

template <typename T>
void Parameter<T>::AdjustScore(double score)
{
    AdjustScore(score, current_index_);
}

template <typename T>
void Parameter<T>::AdjustScore(double score, int index)
{
    scores_[index] = (scores_[index] * counts_[index] + score) / (counts_[index] + 1);
    counts_[index] += 1;
    total_counts_ += 1;
    final_scores_[index] = scores_[index] + (c_fac_ / counts_[index]);
    cout << name_ << ": Updated score of " << values_[index]
         << " to " << final_scores_[index] << endl;
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
    vector<int> bucket;
    for (int i = 0; i < values_.size(); ++i)
    {
        if (counts_[i] < 5)
            bucket.push_back(i);
    }
    int best_index = 0;
    if (bucket.empty())
    {
        for (int i = 0; i < values_.size(); ++i)
        {
            if (final_scores_[i] > final_scores_[best_index])
            {
                best_index = i;
            }
        }
    }
    else
    {
        if (bucket.size() > 1)
            best_index = bucket[rand() % bucket.size()];
        else
            best_index = bucket[0];
    }
    current_index_ = best_index;
    cout << name_ << ": Trying value: " << values_[best_index]
         << " at index: " << best_index
         << " Score: " << final_scores_[best_index]
         << endl;
    return values_[best_index];
}

template <typename T>
void Parameter<T>::PrintStats()
{
    cout << name_ << endl;
    for (int i = 0; i < values_.size(); ++i)
    {
        cout << "Value " << values_[i]
             << " count " << counts_[i]
             << " Qscore " << scores_[i]
             << " Final score " << final_scores_[i] << endl;
    }
}
#endif
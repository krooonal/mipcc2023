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
#include <random>

#define PARDEBUG false

using namespace std;

// https://www.johndcook.com/blog/standard_deviation/
class RunningStat
{
public:
    RunningStat() : num_data_(0) {}

    void Clear()
    {
        num_data_ = 0;
    }

    void Push(double x)
    {
        num_data_++;

        // See Knuth TAOCP vol 2, 3rd edition, page 232
        if (num_data_ == 1)
        {
            old_mean_ = new_mean_ = x;
            old_std_ = 0.0;
        }
        else
        {
            new_mean_ = old_mean_ + (x - old_mean_) / num_data_;
            new_std_ = old_std_ + (x - old_mean_) * (x - new_mean_);

            // set up for next iteration
            old_mean_ = new_mean_;
            old_std_ = new_std_;
        }
    }

    int NumDataValues() const
    {
        return num_data_;
    }

    double Mean() const
    {
        return (num_data_ > 0) ? new_mean_ : 0.0;
    }

    double Variance() const
    {
        return ((num_data_ > 1) ? new_std_ / (num_data_ - 1) : 0.0);
    }

    double StandardDeviation() const
    {
        return sqrt(Variance());
    }

private:
    int num_data_;
    double old_mean_, new_mean_, old_std_, new_std_;
};

template <typename T>
class Parameter
{
public:
    Parameter(double impact_factor, string name, int seed);
    void AddValue(T value);
    void AdjustScore(double score);
    void AdjustScore(double score, int index);
    T GetBestValue();
    int GetCurrentIndex()
    {
        return current_index_;
    }
    void PrintStats();
    void SetExploreCount(int explore_count)
    {
        explore_count_ = explore_count;
    }
    void SetSwitchFlag(int switch_flag)
    {
        switch_flag_ = switch_flag;
    }

private:
    string name_ = "";
    std::vector<T> values_;
    std::vector<RunningStat> scores_;
    std::vector<double> final_scores_;
    std::vector<int> counts_;
    double c_fac_ = 0.3;
    int current_index_ = 0;
    int total_counts_ = 0;
    mt19937 mt_;
    int explore_count_ = 10;
    int switch_flag_ = 0; // Only for binary parameters.
};

template <typename T>
Parameter<T>::Parameter(double c_fac, string name, int seed) : mt_(seed)
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
    scores_[index].Push(score);
    counts_[index] += 1;
    total_counts_ += 1;
    final_scores_[index] = scores_[index].Mean() + (c_fac_ / counts_[index]);
    if (PARDEBUG)
    {
        cout << name_ << ": Updated score of " << values_[index]
             << " to " << final_scores_[index] << endl;
    }
}

template <typename T>
void Parameter<T>::AddValue(T value)
{
    values_.push_back(value);
    RunningStat s;
    scores_.push_back(s);
    // High scores for the value that hasn't been tried yet.
    final_scores_.push_back(5.0);
    counts_.push_back(0);
}

template <typename T>
T Parameter<T>::GetBestValue()
{
    int best_index = 0;

    if (explore_count_ > 0)
    {
        if (values_.size() == 2)
            best_index = (switch_flag_ & explore_count_) ? 1 : 0;
        else
            best_index = mt_() % values_.size();
        explore_count_--;
    }
    else // Exploit.
    {
        vector<int> bucket;
        for (int i = 0; i < values_.size(); ++i)
        {
            if (counts_[i] < 4)
                bucket.push_back(i);
        }
        if (bucket.empty())
        {
            for (int i = 0; i < values_.size(); ++i)
            {
                if (final_scores_[i] > final_scores_[best_index])
                {
                    best_index = i;
                }
            }
            for (int i = 0; i < values_.size(); ++i)
            {
                // Only converge if gains are significant.
                if (final_scores_[i] + scores_[i].StandardDeviation() / 10.0 >
                    final_scores_[best_index])
                {
                    bucket.push_back(i);
                }
            }
        }
        // bucket is never empty. The best param is always in it.
        if (bucket.size() > 1)
            best_index = bucket[mt_() % bucket.size()];
        else
            best_index = bucket[0];
    }

    current_index_ = best_index;
    if (PARDEBUG)
    {
        cout << name_ << ": Trying value: " << values_[best_index]
             << " at index: " << best_index
             << " Score: " << final_scores_[best_index]
             << " Stdev: " << scores_[best_index].StandardDeviation()
             << endl;
    }
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
             << " Qscore " << scores_[i].Mean()
             << " Final score " << final_scores_[i] << endl;
    }
}
#endif
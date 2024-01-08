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
    // Add a value to be tuned.
    void AddValue(T value);

    // Update the score of the current value.
    void AdjustScore(double score);

    // Update the score of the value at given index.
    void AdjustScore(double score, int index);

    // Get the best value.
    T GetBestValue();

    // Get the best value index.
    int GetCurrentIndex()
    {
        return current_index_;
    }

    // Print scores for all values.
    void PrintStats();

    // Set count for deterministic exploration.
    void SetExploreCount(int explore_count)
    {
        explore_count_ = explore_count;
    }

    // Switch flag is used for fixing the deterministic exploration pattern for binary parameters.
    // 1 -> Alternate between 'on' and 'off'.
    // 2 -> Use 'on' twice then 'off' twice. Repeat
    // 4 -> Try 'on' for 4 instances then 'off' for next 4 instance. Repeat
    void SetSwitchFlag(int switch_flag)
    {
        switch_flag_ = switch_flag;
    }

private:
    string name_ = "";
    std::vector<T> values_;
    std::vector<RunningStat> scores_;  // Provided scores for each value.
    std::vector<double> final_scores_; // Computed UCB scores for each value.
    std::vector<int> counts_;          // Score update counts for each value.
    double c_fac_ = 0.3;               // A hyperparameter to balance between exploration and exploitation.
    int current_index_ = 0;
    int total_counts_ = 0; // Total number of score updates.
    mt19937 mt_;           // For generating random numbers.
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
    // Final UCB score is the average of provided scores adjusted with the
    // confidence score (which decreases with each update.)
    final_scores_[index] = scores_[index].Mean() + (c_fac_ / counts_[index]);
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

    if (explore_count_ > 0) // We are still in the exploration stage.
    {
        if (values_.size() == 2) // Pick based on the switch flag.
            best_index = (switch_flag_ & explore_count_) ? 1 : 0;
        else // Randomly pick value to explore. Non deterministic.
            best_index = mt_() % values_.size();
        explore_count_--;
    }
    else // Exploit.
    {
        vector<int> bucket;
        // If there is a value that is not sufficiently explored, try that first.
        for (int i = 0; i < values_.size(); ++i)
        {
            if (counts_[i] < 4)
                bucket.push_back(i);
        }

        // When all values are sufficiently explored...
        if (bucket.empty())
        {
            // Pick the value with best UCB score.
            for (int i = 0; i < values_.size(); ++i)
            {
                if (final_scores_[i] > final_scores_[best_index])
                {
                    best_index = i;
                }
            }
            // Also pick values that are near to the best UCB score.
            // This depends on the standard deviation of the provided score.
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
    return values_[best_index];
}

template <typename T>
void Parameter<T>::PrintStats()
{
    cout << name_ << endl; // Name of the parameter tuned.
    for (int i = 0; i < values_.size(); ++i)
    {
        // Scores and other stats for each value.
        cout << "Value " << values_[i]
             << " count " << counts_[i]
             << " Qscore " << scores_[i].Mean()
             << " Stdev " << scores_[i].StandardDeviation()
             << " Final score " << final_scores_[i] << endl;
    }
}
#endif
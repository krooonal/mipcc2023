import os
import sys
from datetime import datetime
import matplotlib.pyplot as plt

meta_file = sys.argv[-1]
if not os.path.isfile(meta_file):
    print("Usage: python run.py /path/to/meta/file")
    exit()

with open(meta_file) as file:
    all_lines = [line.rstrip() for line in file]

if not os.path.isfile(meta_file):
    print("Meta file is invalid")
    print(meta_file)
    exit()

# read instances from meta file
instances = all_lines[1:len(all_lines)]
print("number of instances:", len(instances))

# read time limit from meta file
time_limit = int(all_lines[0].split(' ')[-1])

solution_folder = os.path.join(
    "solutions/", os.path.basename(os.path.splitext(meta_file)[0]))

if not os.path.isdir(solution_folder):
    os.mkdir(solution_folder)

base_folder = os.path.join(os.path.dirname(meta_file), "../..")

variables = {}
for instance in instances:
    instance_base = os.path.basename(instance)
    with open(os.path.join(solution_folder, f"{instance_base}.sol")) as f:
        all_vars = [line.rstrip() for line in f]
        # print(instance, len(all_vars))
        for var in range(len(all_vars)):
            if all_vars[var].startswith("#"):
                continue
            var_name, value = all_vars[var].split()
            # print(var_name, value)
            value = float(value)
            if var_name not in variables:
                variables[var_name] = {}
            if value not in variables[var_name]:
                variables[var_name][value] = 1
            else:
                variables[var_name][value] += 1

max_distance = 0.0
avg_distance = 0.0
min_distance = 99999999999.0
avg_best_freq = 0.0
distances = []
num_non_zero_commons = 0
num_commons = 0
for var in variables:
    # print(var)
    max_val = -9999999999.0
    min_val = 9999999999.0
    most_freq_val = 0.0
    best_val_freq = 0
    for value in variables[var]:
        max_val = max(max_val, value)
        min_val = min(min_val, value)
        if variables[var][value] > best_val_freq:
            best_val_freq = variables[var][value]
            most_freq_val = value

    # print(min_val, max_val, most_freq_val, best_val_freq)
    distance = max_val - min_val
    if distance == 0:
        num_commons += 1
        if max_val > 0:
            num_non_zero_commons += 1
    distances.append(distance)
    max_distance = max(max_distance, distance)
    avg_distance += distance
    min_distance = min(min_distance, distance)
    avg_best_freq += best_val_freq

avg_best_freq /= len(variables)
avg_distance /= len(variables)
print("Total num vars: ", len(variables))
print("Max distance: ", max_distance)
print("Min distance: ", min_distance)
print("Avg distance: ", avg_distance)
print("Avg besst freq: ", avg_best_freq)
print("num_non_zero_commons: ", num_non_zero_commons)
print("num_commons: ", num_commons)

distances.sort()

plt.hist(distances, 100)
# plt.show()

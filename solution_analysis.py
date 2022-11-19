import os
import sys
from datetime import datetime

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
        print(instance, len(all_vars))
        for var in range(len(all_vars)):
            var_name, value = all_vars[var].split()
            # print(var_name, value)
            if var_name not in variables:
                variables[var_name] = {}
            if value not in variables[var_name]:
                variables[var_name][value] = 1
            else:
                variables[var_name][value] += 1
        for var in variables:
            print(var)
            for value in variables[var]:
                print(value, variables[var][value])

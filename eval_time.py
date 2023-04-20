# Simple evaluation script for MIPcc23

# The evaluation script verifies the correctness and scores a submission for an instance series.
# Errors are thrown if something critical is missing or incorrect.

import os
import sys
import datetime
from dateutil import parser

print("Running time evaluation script for MIPcc23 . . .\n")

if len(sys.argv) < 4:
    print("Usage:")
    print("python", os.path.basename(__file__),
          "logfile solution_folder instance_file")
    quit()


log_path = sys.argv[1]
solution_folder = sys.argv[2]
instance_file = sys.argv[3]
#base_path = os.path.join(os.path.dirname(instance_file), "../..")
base_path = os.getcwd()

with open(instance_file) as f:
    ifile = f.read().splitlines()

with open(log_path) as f:
    log_lines = f.read().splitlines()


timeout_line = ifile[0]
assert "TIMEOUT" in timeout_line

timeout_value = int(timeout_line.split()[-1])
instances = ifile[7:]

num_instance_lines = len(list(filter(lambda l: "[INSTANCE]" in l, log_lines)))
num_start_lines = len(list(filter(lambda l: "[START]" in l, log_lines)))
num_end_lines = len(list(filter(lambda l: "[END]" in l, log_lines)))
num_bound_lines = len(list(filter(lambda l: "[DUALBOUND]" in l, log_lines)))

results = []

if num_instance_lines != num_start_lines or num_instance_lines != num_end_lines or num_instance_lines != num_bound_lines:
    raise ValueError(
        'Inconsistent log: number of lines starting with [INSTANCE], [START], [END], [DUALBOUND] are different.')

for instance in instances:
    instance_base = instance.split('/')[-1]
    instance_line_idx_pair = next(
        filter(lambda t: instance_base in t[1], enumerate(log_lines)))
    instance_line_idx = instance_line_idx_pair[0]
    current_line = instance_line_idx
    while not "[START]" in log_lines[current_line]:
        current_line += 1
    assert len(log_lines[current_line].split(' ')) == 2
    start_time = parser.parse(log_lines[current_line].split(' ')[-1])
    while not "[END]" in log_lines[current_line]:
        current_line += 1
    end_time = parser.parse(log_lines[current_line].split(' ')[-1])
    time_difference = end_time - start_time
    # leaving a tolerance for timeout
    if time_difference.total_seconds() > timeout_value + 5:
        raise ValueError("Overtime for instance ", instance_base,
                         ": ", time_difference.total_seconds())
    # resetting to starting point since dual bound might be before [END]
    current_line = instance_line_idx
    while not "[DUALBOUND]" in log_lines[current_line]:
        current_line += 1
    dual_bound = float(log_lines[current_line].split(' ')[-1])
    sol_file = solution_folder + instance_base + ".sol"
    if not os.path.isfile(sol_file):
        print("No solution file for ", instance_base, " at path ", sol_file)
        results.append((instance_base, dual_bound,
                       time_difference.total_seconds(), False, 0.0))
        continue
    # check creation date for file
    time_modified = os.path.getmtime(sol_file)
    time_modification_solfile = datetime.datetime.fromtimestamp(time_modified)
    # leaving 2sec margin
    if (time_modification_solfile - end_time).total_seconds() > 2:
        raise ValueError("Solution file written after the end timestamp: ",
                         (time_modification_solfile - end_time).total_seconds())

print("")

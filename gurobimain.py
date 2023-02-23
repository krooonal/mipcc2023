import gurobipy as gp
from gurobipy import GRB
import os
import sys
import math
from datetime import datetime
from time import time


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
instances = all_lines[7:len(all_lines)]
print("number of instances:", len(instances))

# read time limit from meta file
time_limit = int(all_lines[0].split(' ')[-1])

solution_folder = os.path.join(
    "optsolutions/", os.path.basename(os.path.splitext(meta_file)[0]))

if not os.path.isdir(solution_folder):
    os.mkdir(solution_folder)

# base_folder = os.path.join(os.path.dirname(meta_file), "../..")
base_folder = os.getcwd()

overall_total_score = 0

for index, instance in enumerate(instances):
    instance_base = os.path.basename(instance)
    print("[INSTANCE]", instance_base)

    # read instance
    instance_path = os.path.join(base_folder, instance)

    model = gp.read(instance_path)

    # set time limit
    # model.Params.timelimit = time_limit
    model.Params.Threads = 1
    model.Params.OutputFlag = 0
    model.Params.MIPGap = 1e-6

    # optimize
    t_start = time()
    print("[START]", datetime.now().isoformat())
    model.optimize()
    print("[END]", datetime.now().isoformat())
    t_end = time()

    primal_bound = GRB.INFINITY
    dual_bound = model.ObjBoundC

    # print dual bound
    print("[DUALBOUND]", dual_bound)

    # write solution to instance_name.sol in 'solutions' directory and store
    # it for future use.
    if model.SolCount > 0:
        primal_bound = model.ObjVal
        with open(os.path.join(solution_folder, f"{instance_base}.sol"), 'w') as f:
            for v in model.getVars():
                name = v.VarName
                val = v.X
                f.write(name)
                f.write("    ")
                f.write(str(val))
                f.write("\n")

    else:
        print("No solution found")
        feasibility_penalty = 1

    time_score = (t_end - t_start)/time_limit
    gap_score = 0
    feasibility_penalty = 0

    is_optimal = (model.Status == GRB.OPTIMAL)

    if not is_optimal:
        time_score = max(1, time_score)
        gap_score = abs(primal_bound - dual_bound) / \
            max(abs(primal_bound), abs(dual_bound))

    if abs(primal_bound) >= GRB.INFINITY or abs(dual_bound) >= GRB.INFINITY:
        gap_score = 1
    elif primal_bound * dual_bound < 0:
        gap_score = 1

    total_score_instance = time_score + gap_score + feasibility_penalty
    print("Time score: ", time_score)
    print("Gap score: ", gap_score)
    print("Feasibility penalty: ", feasibility_penalty)
    print("Instance score: ", total_score_instance)
    overall_total_score += (1 + 0.1*index)*total_score_instance
    print("Overall total score so far: ", overall_total_score)

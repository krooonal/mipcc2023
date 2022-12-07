import pyscipopt
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
instances = all_lines[1:len(all_lines)]
print("number of instances:", len(instances))

# read time limit from meta file
time_limit = int(all_lines[0].split(' ')[-1])

model = pyscipopt.Model("reoptimization")

solution_folder = os.path.join(
    "solutions/", os.path.basename(os.path.splitext(meta_file)[0]))

if not os.path.isdir(solution_folder):
    os.mkdir(solution_folder)

# base_folder = os.path.join(os.path.dirname(meta_file), "../..")
base_folder = os.getcwd()

overall_total_score = 0
solutions = {}
for index, instance in enumerate(instances):
    instance_base = os.path.basename(instance)
    print("[INSTANCE]", instance_base)

    # optionally disable SCIP output
    model.setIntParam("display/verblevel", 0)

    # set time limit
    model.setRealParam("limits/time", time_limit)

    # read instance
    instance_path = os.path.join(base_folder, instance)
    model.readProblem(instance_path)
    scip_vars = []
    for j in range(model.getNVars()):
        v = model.getVars()[j]
        scip_vars.append(v)

    # Give hint using previous solutions.
    solution_hint = model.createPartialSol()
    partial_solution_generated = False
    for j in range(model.getNVars()):
        v = model.getVars()[j]
        if v.vtype() == "CONTINUOUS":
            continue
        name = v.name
        if name not in solutions:
            continue

        best_val = 0
        best_freq = 0
        for val in solutions[name]:

            if solutions[name][val] > best_freq:
                best_freq = solutions[name][val]
                best_val = val
        if best_freq > 0:
            partial_solution_generated = True
            model.setSolVal(solution_hint, v, best_val)
            # print(name, best_val)
    if partial_solution_generated:
        try:
            print("Trying solution")
            # stored = model.trySol(solution_hint, printreason=True, free=False)
            stored = model.addSol(solution_hint, free=True)
            if stored:
                print("Solution stored!")
        except Exception as e:
            print(e)

    # optimize
    t_start = time()
    print("[START]", datetime.now().isoformat())
    model.optimize()
    print("[END]", datetime.now().isoformat())
    t_end = time()

    primal_bound = model.infinity()
    dual_bound = model.getDualbound()

    # print dual bound
    print("[DUALBOUND]", dual_bound)

    # write solution to instance_name.sol in 'solutions' directory and store
    # it for future use.
    if model.getNSols() > 0:
        primal_bound = model.getObjVal()
        sol = model.getBestSol()
        with open(os.path.join(solution_folder, f"{instance_base}.sol"), 'w') as f:
            for v in scip_vars:
                name = v.name
                val = model.getVal(v)
                f.write(name)
                f.write("    ")
                f.write(str(val))
                f.write("\n")
                if name not in solutions:
                    solutions[name] = {}
                if val not in solutions[name]:
                    solutions[name][val] = 1
                else:
                    solutions[name][val] += 1

    else:
        print("No solution found")
        feasibility_penalty = 1

    time_score = (t_end - t_start)/time_limit
    gap_score = 0
    feasibility_penalty = 0
    if model.isInfinity(primal_bound) or model.isInfinity(dual_bound):
        gap_score = 1
    elif primal_bound * dual_bound < 0:
        gap_score = 1

    is_optimal = (model.getStatus() == "optimal")

    if not is_optimal:
        time_score = max(1, time_score)
        gap_score = abs(primal_bound - dual_bound) / \
            max(abs(primal_bound), abs(dual_bound))

    total_score_instance = time_score + gap_score + feasibility_penalty
    print("Time score: ", time_score)
    print("Gap score: ", gap_score)
    print("Feasibility penalty: ", feasibility_penalty)
    print("Instance score: ", total_score_instance)
    overall_total_score += (1 + 0.1*index)*total_score_instance
    print("Overall total score so far: ", overall_total_score)

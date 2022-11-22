import numpy as np
import math
import random

from ortools.init import pywrapinit
from ortools.linear_solver import pywraplp
from ortools.linear_solver import linear_solver_pb2
from ortools.model_builder.python import pywrap_model_builder_helper
from ortools.model_builder.python import model_builder_helper
import sys
import csv
import gzip


def main(argv):

    # Read a mps.gz file
    mps_file_path = "/Users/krunal/Downloads/mip_competition/mipcc2023/datasets/vary_rhs/series_2/rhs_s2_i01.mps.gz"
    # mps_file_path = "datasets/vary_matrix/series_1/mat_s1_i01.mps.gz"
    mps_file = gzip.open(mps_file_path, "rb")
    mps_content = mps_file.read()
    # model_builder_helper.
    model_string = pywrap_model_builder_helper.ImportFromMpsString(mps_content)
    model_proto = linear_solver_pb2.MPModelProto.FromString(model_string)
    # model_proto = model_builder_helper.ImportFromMpsString(mps_content)
    # print(model_proto)
    print("Solving proto")
    # Solve it with scip/cp-sat
    sat_params = "interleave_search: true"

    solver = pywraplp.Solver.CreateSolver('scip')
    if not solver:
        return
    params_set = solver.SetSolverSpecificParametersAsString(sat_params)
    print("Params set: ", params_set)

    solver.set_time_limit(300*1000)
    solver.LoadModelFromProtoWithUniqueNamesOrDie(model_proto)
    # solver.EnableOutput()
    solver.SetNumThreads(1)
    solver.Solve()

    print("Solved")
    verified = solver.VerifySolution(tolerance=1e-6, log_errors=True)
    print("Verification: ", verified)

    # Print solution
    print('Solution:')
    print('Objective value =', solver.Objective().Value())

    for var in solver.variables():
        print(var.name(), var.solution_value())
    return


if __name__ == "__main__":
    pywrapinit.CppBridge.InitLogging('main.py')
    cpp_flags = pywrapinit.CppFlags()
    cpp_flags.logtostderr = True
    cpp_flags.log_prefix = False
    pywrapinit.CppBridge.SetFlags(cpp_flags)
    main(sys.argv[1:])

#!/bin/bash
# Old python code.
#python scipmain.py $1

if ! [ -f cppcode/build/cppex ];
then
    mkdir -p cppcode/build
    cd cppcode/build/
    cmake .. -DSCIP_DIR=/home/krunal/scipoptsuite-8.0.2/build/scip/
    make
    cd ../../
fi

#./cppcode/build/cppex $1

# Command to compute base benchmarks.
./cppcode/build/basecppex $1
# With gurobi
# python gurobimain.py $1

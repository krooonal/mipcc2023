# mipcc2023

Code for MIP computational competition 2023 (C++).

The execution file mipcomp.sh already has commands to compile the code if the code is not compiled.

The compiled code is in cppcode/build directory.

Use experiment.cpp file for any experimental evaluation. Modify the mipcomp.sh accordingly.

Ignore the python code. It is strictly for analysis and comparision. It is not used for the competition.

Instructions to run the code:
Provide the desired series '.test' file to mipcomp.sh. Record the logs in a text file.
Example: 
sh mipcomp.sh datasets/testfiles/rhs\_series\_2.test > rhs\_series\_2\_logs.txt

The solutions are stored in the solutions directory.

To evaluate the performance use eval.py. Use eval\_mat.py for the mat\_rhs\_bnd\_series\_1 because that series is numerically challenging. There are three arguments. The first argument is the logs file. The second argument is the path to the solutoions directory. The third argument is the '.test' file for the series.
Example: 
python eval.py rhs\_series\_2\_logs.txt solutions/rhs\_series\_2/ datasets/testfiles/rhs\_series\_2.test

# mipcc2023

Code for MIP computational competition 2023 (C++). The preprint of the paper is available at https://arxiv.org/abs/2308.08986

NOTE: This code is ment to prove the effectiveness of the approach presented in the paper. It is not meant to be used outside of the competition context. The developers should use this code as reference to write application specific code for reoptimization.

The execution file mipcomp.sh already has commands to compile the code if the code is not compiled.

The compiled code is in cppcode/build directory.

Instructions to run the code:
Provide the desired series '.test' file (containing the list of instances in the series) to mipcomp.sh. Record the logs in a text file.
Example: 
sh mipcomp.sh datasets/testfiles/rhs\_series\_2.test > rhs\_series\_2\_logs.txt

The solutions are stored in the solutions directory.

To evaluate the performance use eval.py. Use eval\_mat.py for the mat\_rhs\_bnd\_series\_1 because that series is numerically challenging. There are three arguments. The first argument is the logs file. The second argument is the path to the solutoions directory. The third argument is the '.test' file for the series.
Example: 
python eval.py rhs\_series\_2\_logs.txt solutions/rhs\_series\_2/ datasets/testfiles/rhs\_series\_2.test

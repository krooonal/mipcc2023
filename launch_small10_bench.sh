
nohup sh mipcomp.sh datasets/testfiles/small10/bnd_series_1.test > logs/small10/$1/bnd_series_1.txt &

nohup sh mipcomp.sh datasets/testfiles/small10/bnd_series_2.test > logs/small10/$1/bnd_series_2.txt &

nohup sh mipcomp.sh datasets/testfiles/small10/obj_series_1.test > logs/small10/$1/obj_series_1.txt &

nohup sh mipcomp.sh datasets/testfiles/small10/obj_series_2.test > logs/small10/$1/obj_series_2.txt &

nohup sh mipcomp.sh datasets/testfiles/small10/rhs_series_1.test > logs/small10/$1/rhs_series_1.txt &

nohup sh mipcomp.sh datasets/testfiles/small10/rhs_series_2.test > logs/small10/$1/rhs_series_2.txt &

nohup sh mipcomp.sh datasets/testfiles/small10/rhs_obj_series_1.test > logs/small10/$1/rhs_obj_series_1.txt &


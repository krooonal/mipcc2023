
nohup sh mipcomp.sh datasets/testfiles/bnd_series_1.test > logs/full/$1/bnd_series_1.txt &

nohup sh mipcomp.sh datasets/testfiles/bnd_series_2.test > logs/full/$1/bnd_series_2.txt &

nohup sh mipcomp.sh datasets/testfiles/obj_series_1.test > logs/full/$1/obj_series_1.txt &

nohup sh mipcomp.sh datasets/testfiles/obj_series_2.test > logs/full/$1/obj_series_2.txt &

nohup sh mipcomp.sh datasets/testfiles/rhs_series_1.test > logs/full/$1/rhs_series_1.txt &

nohup sh mipcomp.sh datasets/testfiles/rhs_series_2.test > logs/full/$1/rhs_series_2.txt &

nohup sh mipcomp.sh datasets/testfiles/rhs_obj_series_1.test > logs/full/$1/rhs_obj_series_1.txt &

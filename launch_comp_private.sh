
nohup sh mipcomp.sh datasets/testfiles/bnd_series_3.test > logs/full/$1/bnd_series_3.txt &

nohup sh mipcomp.sh datasets/testfiles/obj_series_3.test > logs/full/$1/obj_series_3.txt &

nohup sh mipcomp.sh datasets/testfiles/rhs_series_3.test > logs/full/$1/rhs_series_3.txt &

nohup sh mipcomp.sh datasets/testfiles/rhs_series_4.test > logs/full/$1/rhs_series_4.txt &

nohup sh mipcomp.sh datasets/testfiles/rhs_obj_series_2.test > logs/full/$1/rhs_obj_series_2.txt &

nohup sh mipcomp.sh datasets/testfiles/mat_rhs_bnd_series_1.test > logs/full/$1/mat_rhs_bnd_series_1.txt &

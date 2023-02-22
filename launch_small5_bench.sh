
nohup sh mipcomp.sh datasets/testfiles/small5/bnd_series_1.test > logs/small5/$1/bnd_series_1.txt &

nohup sh mipcomp.sh datasets/testfiles/small5/bnd_series_2.test > logs/small5/$1/bnd_series_2.txt &

nohup sh mipcomp.sh datasets/testfiles/small5/obj_series_1.test > logs/small5/$1/obj_series_1.txt &

nohup sh mipcomp.sh datasets/testfiles/small5/obj_series_2.test > logs/small5/$1/obj_series_2.txt &

nohup sh mipcomp.sh datasets/testfiles/small5/rhs_series_1.test > logs/small5/$1/rhs_series_1.txt &

nohup sh mipcomp.sh datasets/testfiles/small5/rhs_series_2.test > logs/small5/$1/rhs_series_2.txt &

nohup sh mipcomp.sh datasets/testfiles/small5/rhs_obj_series_1.test > logs/small5/$1/rhs_obj_series_1.txt &


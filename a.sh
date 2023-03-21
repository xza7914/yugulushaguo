
p=`pwd`
cd SDK/cpp_project/build && make
cd $p
./Robot_gui ./SDK/cpp_project/build/main -m maps/1.txt -f

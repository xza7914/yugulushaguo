
map=$1
if [ ! $map ]; then
    map=1
fi

p=`pwd`
cd SDK/cpp_project/build && make
cd $p
./Robot_gui ./SDK/cpp_project/build/main -m maps/${map}.txt -f

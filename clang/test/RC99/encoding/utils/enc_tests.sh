#!/bin/bash
function TestGeneration() {
  arch=$1
  kernels=$2/src/kernels/$arch
  file=$3/$arch/$arch.txt
  pushd $kernels > /dev/null
  find . -name "*.o" | grep -v _x86 > $file
  popd > /dev/null

  echo "Encoding tests generation for $arch ..."
  while IFS= read -r line
  do
    d=$(echo "$line" | sed -r 's/.{2}$//')
    dst=$3/$arch/$d
    mkdir -p $dst
    dest=$(echo $dst)
    f=$(basename "$line")
    cp $kernels/$line $dest/$f
    f=$(echo $f.enc)
    cp $arch.tst  $dest/$f
  done <"$file"
}

arg1=$1
arg2=$2
if [ "$#" -eq 2 ]; then
   if ! [ -d $var1 ]; then
       echo TPC kernels build directory ${arg1} does not exists
	exit 1
   fi
   if [ -d $arg2 ]; then
       echo Target directory ${arg2} is not empty
       exit 1
   fi
   read -p "Do you wish to create the target directory $arg2 ?" yn
   case $yn in
       [Yy]* ) mkdir -p $arg2; break;;
       [Nn]* ) exit;;
       * ) echo "Please answer yes or no.";;
   esac
else
   echo "Usage: enc_tests.sh <TPC kernels build directory> <Target directory to keep the generated tests>"
   exit 1
fi

echo "Generation of encoding tests from $1 to $2"
mkdir $2/goya $2/gaudi $2/goya2

TestGeneration goya  $1 $2
TestGeneration gaudi $1 $2
TestGeneration goya2 $1 $2

echo "Done !!!"

#!/bin/sh

case $MODE in
  release)
  BUILD_MODE=Release
  NO_BAS=OFF
  PROF=OFF
  ;;

  release-no-bas)
  BUILD_MODE=Release
  NO_BAS=ON
  PROF=OFF
  ;;

  debug)
  NO_BAS=OFF
  BUILD_MODE=Debug
  PROF=ON
  ;;

  debug-no-bas)
  NO_BAS=ON
  BUILD_MODE=Debug
  PROF=ON
  ;;

  *)
    echo "Unknown mode $MODE"
    echo "MODE = release | release-no-bas | debug | debug-no-bas"
    exit 1
esac



#
cmake .. \
  -DCMAKE_BUILD_TYPE=$BUILD_MODE \
  -DBASALISC_USE_OPENFHE_DEFAULT=$NO_BAS \
  -DWITH_PROFILING=$PROF \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DBUILD_UNITTESTS=OFF \
  -DBUILD_EXAMPLES=OFF \
  -DBUILD_BENCHMARKS=OFF \
  -DBUILD_STATIC=ON \
  -DBUILD_SHARED=OFF \
  -DWITH_OPENMP=OFF \
  -DNATIVE_SIZE=64


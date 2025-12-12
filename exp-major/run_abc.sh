#!/bin/bash

# run abc for each file in list
# bash run_abc.sh <path_abc> <path_benchmark> <timeout> <path_lib>
# eg: bash run_abc.sh /home/liujunfeng/NewCriterion/abc_criterion/abc_criterion/build/abc_efficient  /home/liujunfeng/benchmarks/mtm/exp/ 10000m

# bash run_abc.sh /home/liujunfeng/NewCriterion/abc_criterion/abc_criterion/build/abc_static  /home/liujunfeng/benchmarks/mtm/exp/ 10000m
# bash run_abc.sh /home/liujunfeng/NewCriterion/abc_criterion/abc_criterion/build/abc  /home/liujunfeng/benchmarks/larger_aig/sim-LSV_exp/benchmarks_dc2syn2/ 10000m
 
####################################################################
binary=$(echo "$1" | awk -F "/" '{print $NF}')
dataname=$(basename "${2%/}")
timestamp=$(date +%Y%m%d%H%M%S)
  
log="${binary}_${dataname}_${libname}_${timestamp}.log"
touch "$log"

files=$(find "$2" -name "*.aig")
# files=("/home/liujunfeng/benchmarks/mtm/sixteen.aig")

for element in ${files[@]}
do
    echo "process $element"
    command="read $element;  strash; print_stats; rewrite -v; print_stats;"
    outputs=$(timeout $3 $1 -c "$command";)
    echo $outputs >> $log 
done
  

 

## test abc_redesign: 
# bash run_abc.sh /home/liujunfeng/NewCriterion/abc_criterion/abc_criterion/build/abc_redesign  /home/liujunfeng/NewCriterion/mockturtle/mockturtle/experiments/benchmarks_new 10000m
## test abc static: 
# bash run_abc.sh /home/liujunfeng/NewCriterion/abc_criterion/abc_criterion/build/abc_static  /home/liujunfeng/NewCriterion/mockturtle/mockturtle/experiments/benchmarks_new 10000m

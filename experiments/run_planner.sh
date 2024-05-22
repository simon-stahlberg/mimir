#!/bin/bash

set -euo pipefail

if [[ $# != 6 ]]; then
    echo "usage: $(basename "$0") <planner_file> <domain_file> <problem_file> <plan_file> <enable_grounded:0/1> <enable_debug:0/1>" 1>&2
    exit 2
fi

planner_exe=$1
domain_file=$PWD/$2   # Assuming full path or relative to current dir
problem_file=$PWD/$3  # Assuming full path or relative to current dir
plan_file=$PWD/$4     # Assuming full path or relative to current dir
flag1=$5
flag2=$6

# Check if the plan file already exists and prompt for removal
if [ -f "$plan_file" ]; then
    echo "Error: remove $plan_file" 1>&2
    exit 2
fi

# Ensure that strings like "CPU time limit exceeded" and "Killed" are in English.
export LANG=C

# Run planner
"$planner_exe" "$domain_file" "$problem_file" "$plan_file" "$flag1" "$flag2"

# Run VAL
echo -e "\nRun VAL\n"

# After running the planner, check if the plan file was created
if [ -f "$plan_file" ]; then
    echo "Found plan file."
    validate -v "$domain_file" "$problem_file" "$plan_file"
else
    echo "No plan file found."
    exit 99
fi

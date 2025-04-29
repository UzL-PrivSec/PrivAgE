#!/bin/bash

# check if argument is provided.
if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <number of instances>"
	return
fi

num_clients=$1

# check if the argument provided is an integer.
if ! [[ "$num_clients" =~ ^[0-9]+$ ]]; then
	echo "Error: No integer provided for the number of instances."
	return
fi

# compile the program again.
gradle clean build
tar -xf build/distributions/clientsim.tar

for i in $(seq 1 $num_clients)
do
  # sleep 0.6
  clientsim/bin/clientsim &
done

# wait for termination of background processes.
wait


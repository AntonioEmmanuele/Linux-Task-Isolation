#!/bin/bash

# Disabling rt throttling
echo -1 > /proc/sys/kernel/sched_rt_runtime_us
# Make the offload of ir
sudo ./offload.sh

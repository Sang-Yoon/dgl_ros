#!/bin/bash
set -e

source /opt/ros/humble/setup.bash
source /home/ros2_ws/src/dgl_ros/install/local_setup.bash

exec "$@"
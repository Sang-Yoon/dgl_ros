#!/bin/bash

# Install ROS2
curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | tee /etc/apt/sources.list.d/ros2.list > /dev/null

apt-key del 421C365BD9FF1F717815A3895523BAEEB01FA116
apt-key adv --keyserver 'hkp://keyserver.ubuntu.com:80' --recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654
apt clean && apt update

sh -c 'echo "deb [arch=amd64,arm64] http://repo.ros2.org/ubuntu/main `lsb_release -cs` main" > /etc/apt/sources.list.d/ros2-latest.list'
curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | apt-key add -

apt-get update && apt-get upgrade -y \
  && apt-get install -y -qq --no-install-recommends \
    ros-humble-desktop \
    ros-humble-vision-msgs \
    ros-humble-rmw-cyclonedds-cpp \
    python3-colcon-common-extensions

# Clean up
apt-get autoremove -y \
&& apt-get clean -y \
&& rm -rf /var/lib/apt/lists/*
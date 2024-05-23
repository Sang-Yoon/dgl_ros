FROM nvcr.io/nvidia/pytorch:23.05-py3
ARG DEBIAN_FRONTEND=noninteractive

ENV PATH="${PATH}:/opt/hpcx/ompi/bin"
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/opt/hpcx/ompi/lib"

LABEL "purpose"="A Docker image of 'PyTorch' and 'ROS 2 humble on ubuntu 22.04' for Deep Grasp Library"
LABEL name="sang_yoon_kim"
LABEL email="sangyoon@g.skku.edu"
LABEL version="0.1.0"

WORKDIR /home/ros2_ws

# Installing 'apt-get' dependencies.
RUN apt-get update && apt-get upgrade -y \
  && apt-get install -y -qq --no-install-recommends \
    build-essential \
    software-properties-common \
    curl \
    wget \
    git \
    python3-pip \
    python3-dev \
    python3-tk \
    python3-pybind11 \
  # Clean up
  && apt-get autoremove -y \
  && apt-get clean -y \
  && rm -rf /var/lib/apt/lists/*

# Installing 'Contact-graspnet'
WORKDIR /home/ros2_ws/src
RUN git clone https://github.com/Sang-Yoon/dgl_ros

# Installing ROS2
RUN /home/ros2_ws/src/dgl_ros/docker/ros2_build.sh

RUN pip3 install --upgrade pip \
    && pip3 install cgn-pytorch \
RUN rosdep install --from-paths src --ignore-src --rosdistro ${ROS_DISTRO} -y
WORKDIR /home/ros2_ws
RUN colcon build --symlink-install --base-paths  src/dgl_ros/

# Cyclone DDS for communication between host and container (FastDDS requires shared memory)
ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

ENTRYPOINT ["/home/dgl_ros/docker/pvn3d_ros2_entrypoint.sh"]
CMD ros2 run dgl_ros_models contact_graspnet --ros-args -p src_topic0:=/points2 -p cgn_config_path:=/home/dgl_ros/src/dgl_ros/dgl_ros_models/config/cgn_config.yaml
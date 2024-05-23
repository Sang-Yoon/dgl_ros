# Docker
### Prerequisites
Recent version of Docker (>=19.03) and nvidia-container-toolkit allow you to use nvidia-driver in the container only with the <span style="color:red"> **'--gpus=all** </span> option.

## 1. Setup
```shell
git clone https://github.com/Sang-Yoon/dgl_ros.git

# Build a docker image
docker build -t contact_graspnet:0.1.0 -f dgl_ros/docker/Dockerfile .
```

## 2. Commands for Demo using docker
```shell
docker run --rm -it --gpus all --ipc=host --network host --privileged \
    contact_graspnet:0.1.0 \
    bash -c "ros2 run dgl_ros_models contact_graspnet --ros-args -p src_topic0:=/points2 -p cgn_config_path:=/home/dgl_ros/src/dgl_ros/dgl_ros_models/config/cgn_config.yaml"
```
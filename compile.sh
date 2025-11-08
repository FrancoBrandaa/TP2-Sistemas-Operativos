#!/bin/bash
# Validates the existance of the TPE-ARQ container, starts it up & compiles the project
# Usage: ./compile.sh [naive|buddy|all]
CONTAINER_NAME="tp2-so"

# Memory manager selection (default: all)
MEMORY_MANAGER=${1:-all}

# Validate memory manager parameter
case "$MEMORY_MANAGER" in
    naive|buddy|all)
        echo "Selected memory manager: $MEMORY_MANAGER"
        ;;
    *)
        echo "Invalid memory manager: $MEMORY_MANAGER"
        echo "Usage: $0 [naive|buddy|all]"
        echo "  naive - Compile with Naive Memory Manager"
        echo "  buddy - Compile with Buddy Memory Manager" 
        echo "  all   - Default compilation (naive by default)"
        exit 1
        ;;
esac

docker ps -a &> /dev/null

if [ $? -ne 0 ]; then
    echo "Docker is not running. Please start Docker and try again."
    exit 1
fi

# Check if container exists
if [ ! "$(docker ps -a | grep "$CONTAINER_NAME")" ]; then
    echo "Container $CONTAINER_NAME does not exist."
    echo "Pulling image..."
    docker pull agodio/itba-so:2.0
    echo "Creating container..."
    # Note: ${PWD}:/root. Using another container to compile might fail as the compiled files would not be guaranteed to be at $PWD
    # Always use TPE-ARQ to compile
    docker run --privileged -d -v ${PWD}:/root --security-opt seccomp:unconfined -it --name "$CONTAINER_NAME" agodio/itba-so:2.0
    echo "Container $CONTAINER_NAME created."
else
    echo "Container $CONTAINER_NAME exists."
fi

# Start container
docker start "$CONTAINER_NAME" &> /dev/null
echo "Container $CONTAINER_NAME started."

# Compiles based on selected memory manager
echo "Starting compilation with $MEMORY_MANAGER memory manager..."

docker exec -it "$CONTAINER_NAME" make clean -C /root/ && \
docker exec -it "$CONTAINER_NAME" make all -C /root/Toolchain && \
docker exec -it "$CONTAINER_NAME" make $MEMORY_MANAGER -C /root/

if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

echo "Compilation finished successfully with $MEMORY_MANAGER memory manager!"

# Show memory manager status
echo "Memory Manager Status:"
docker exec -it "$CONTAINER_NAME" make status -C /root/ 2>/dev/null || echo "Status command not available"

# Fix file permissions (files created by Docker belong to root)
echo "Fixing file permissions..."
HOST_UID=$(id -u)
HOST_GID=$(id -g)
docker exec "$CONTAINER_NAME" sh -c "chown -R ${HOST_UID}:${HOST_GID} /root"
echo "File permissions fixed!"


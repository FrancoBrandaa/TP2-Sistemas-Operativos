#!/bin/bash
# Validates the existance of the TPE-ARQ container, starts it up & compiles the project
# Usage: ./compile.sh [naive|buddy|all]
CONTAINER_NAME="tp2-so"

# Memory manager selection (default: all)
MEMORY_MANAGER=${1:-all}

# COLORS
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

# Validate memory manager parameter
case "$MEMORY_MANAGER" in
    naive|buddy|all)
        echo "${BLUE}🔧 Selected memory manager: $MEMORY_MANAGER${NC}"
        ;;
    *)
        echo "${RED}❌ Invalid memory manager: $MEMORY_MANAGER${NC}"
        echo "Usage: $0 [naive|buddy|all]"
        echo "  naive - Compile with Naive Memory Manager"
        echo "  buddy - Compile with Buddy Memory Manager" 
        echo "  all   - Default compilation (naive by default)"
        exit 1
        ;;
esac

docker ps -a &> /dev/null

if [ $? -ne 0 ]; then
    echo "${RED}Docker is not running. Please start Docker and try again.${NC}"
    exit 1
fi

# Check if container exists
if [ ! "$(docker ps -a | grep "$CONTAINER_NAME")" ]; then
    echo "${YELLOW}Container $CONTAINER_NAME does not exist. ${NC}"
    echo "Pulling image..."
    docker pull agodio/itba-so:2.0
    echo "Creating container..."
    # Note: ${PWD}:/root. Using another container to compile might fail as the compiled files would not be guaranteed to be at $PWD
    # Always use TPE-ARQ to compile
    docker run -d -v ${PWD}:/root --security-opt seccomp:unconfined -it --name "$CONTAINER_NAME" agodio/itba-so:2.0
    echo "${GREEN}Container $CONTAINER_NAME created.${NC}"
else
    echo "${GREEN}Container $CONTAINER_NAME exists.${NC}"
fi

# Start container
docker start "$CONTAINER_NAME" &> /dev/null
echo "${GREEN}Container $CONTAINER_NAME started.${NC}"

# Compiles based on selected memory manager
echo "${BLUE}🚀 Starting compilation with $MEMORY_MANAGER memory manager...${NC}"

docker exec -it "$CONTAINER_NAME" make clean -C /root/ && \
docker exec -it "$CONTAINER_NAME" make all -C /root/Toolchain && \
docker exec -it "$CONTAINER_NAME" make $MEMORY_MANAGER -C /root/

if [ $? -ne 0 ]; then
    echo "${RED}❌ Compilation failed.${NC}"
    exit 1
fi

echo "${GREEN}✅ Compilation finished successfully with $MEMORY_MANAGER memory manager!${NC}"

# Show memory manager status
echo "${BLUE}📊 Memory Manager Status:${NC}"
docker exec -it "$CONTAINER_NAME" make status -C /root/ 2>/dev/null || echo "${YELLOW}ℹ️  Status command not available${NC}"

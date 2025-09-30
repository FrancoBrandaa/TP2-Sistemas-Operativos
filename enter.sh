#!/bin/bash
# Script to enter the tp2-so container

CONTAINER_NAME="tp2-so"

# COLORS
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m'

# Check if Docker is running
docker ps -a &> /dev/null
if [ $? -ne 0 ]; then
    echo "${RED}Docker is not running. Please start Docker and try again.${NC}"
    exit 1
fi

# Check if container exists
if [ ! "$(docker ps -a | grep "$CONTAINER_NAME")" ]; then
    echo "${RED}Container $CONTAINER_NAME does not exist.${NC}"
    echo "${YELLOW}Run ./compile.sh first to create the container.${NC}"
    exit 1
fi

# Check if container is running
if [ ! "$(docker ps | grep "$CONTAINER_NAME")" ]; then
    echo "${YELLOW}Container $CONTAINER_NAME is not running. Starting it...${NC}"
    docker start "$CONTAINER_NAME" &> /dev/null
    if [ $? -ne 0 ]; then
        echo "${RED}Failed to start container.${NC}"
        exit 1
    fi
    echo "${GREEN}Container started.${NC}"
fi

# Enter the container
echo "${GREEN}Entering container $CONTAINER_NAME...${NC}"
docker exec -it "$CONTAINER_NAME" /bin/bash
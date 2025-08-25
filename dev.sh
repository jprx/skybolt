#!/bin/bash
docker run --rm -ti -v .:/skybolt -w /skybolt --add-host=host.docker.internal:host-gateway jprx/osdev-tools

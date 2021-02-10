ifeq ($(origin .RECIPEPREFIX), undefined)
  $(error This Make does not support .RECIPEPREFIX. Please use GNU Make 4.0 or later)
endif
.RECIPEPREFIX = >

CORE ?= avr
BOARD ?= nano
ENV_VARS = CORE=$(CORE) BOARD=$(BOARD) UID=$(shell id -u) GID=$(shell id -g)
DOCKER_COMPOSE = $(ENV_VARS) docker-compose -f docker/docker-compose.yml up --build
DOCKER_BUILD = $(ENV_VARS) docker-compose -f docker/docker-compose.yml build --force-rm
DOCKER_RUN = $(ENV_VARS) docker-compose -f docker/docker-compose.yml run --rm

default: ci

build-docker:
> $(DOCKER_BUILD) arduino-builder

build: build-docker
> $(DOCKER_RUN) arduino-builder scripts/build-project.sh  $(CORE) $(BOARD)

docs:
> $(DOCKER_COMPOSE) --exit-code-from docs-builder docs-builder

ci: build docs

clean:
> rm -rf build

.PHONY: default build docs ci build-docker clean

CUR_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
TOP_DIR ?= $(abspath $(CUR_DIR)/../../)

O := $(DEPS_BUILD_DIR)/perfs
prefix := $(ODP_INSTALL_DIR)

use-modules := installer
project-name := libperfs
cluster-system := cos

cluster-lib := perfs

perfs-srcs := \
	src/timestamper.c

perfs-cflags := -g -O2 -Ilib -Iinclude -D_REENTRANT

install-targets := perfs

headers := \
	include/perfs/timestamper.h

cluster-install-headers := $(headers)

post-build-hooks := copy-files

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray

copy_headers := $(addprefix $(DEPS_BUILD_DIR)/include/perfs/, $(notdir $(headers)))
copy_libs    := $(addprefix $(DEPS_BUILD_DIR)/lib/, libperfs.a)

copy-files: $(copy_headers) $(copy_libs)

$(copy_headers): $(DEPS_BUILD_DIR)/include/perfs/%: include/perfs/%
	install -D $< $@

$(copy_libs): $(DEPS_BUILD_DIR)/lib/%: $(LIB_OUTPUT_DIR)/cluster/%
	install -D $< $@

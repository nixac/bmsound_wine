#!/usr/bin/make -s -f
# Usage: make -Rs <action> TARGET_ARCH=<x64,x86> TARGET_TYPE=<Release,Debug>
# Example: make -Rs build TARGET_ARCH=x64 TARGET_TYPE=Release

##    Available user recipes    ##
.PHONY:  tests distribution build clean build_all clean_all
distribution:
	$(error "Unimplemented")
tests:
	$(error "Unimplemented")
passthrough:
	true
build:	sanity_checks bmsound-pw@post bmsound-wine@post test-client@post
	echo "Build completed: [$(TARGET_TYPE)_$(TARGET_ARCH)]"
	echo
clean:	sanity_checks
	echo "Purging build files at : 'build/$(TARGET_TYPE)/$(TARGET_ARCH)'"
	rm -rf build/$(TARGET_TYPE)/$(TARGET_ARCH)
	echo
build_all:
	$(MAKE) build TARGET_ARCH=x64 TARGET_TYPE=Release
	$(MAKE) build TARGET_ARCH=x86 TARGET_TYPE=Release
	$(MAKE) build TARGET_ARCH=x64 TARGET_TYPE=Debug
	$(MAKE) build TARGET_ARCH=x86 TARGET_TYPE=Debug
clean_all:
	$(MAKE) clean TARGET_ARCH=x64 TARGET_TYPE=Release
	$(MAKE) clean TARGET_ARCH=x86 TARGET_TYPE=Release
	$(MAKE) clean TARGET_ARCH=x64 TARGET_TYPE=Debug
	$(MAKE) clean TARGET_ARCH=x86 TARGET_TYPE=Debug

##    Load extensions    ##
ifneq (,$(findstring all,$(MAKECMDGOALS)))
else
CC_STANDARD				= 99
CXX_STANDARD			= 98
include ./lib/make-various/extension/MakeEx.mk

##    Add projects    ##
# bmsound-pw.so
include $(SRC_DIR)/bmsound-pw/Makefile.mk

# bmsound-wine.{dll:so}
include $(SRC_DIR)/bmsound-wine/Makefile.mk

# test-client.bin
include $(SRC_DIR)/test-client/Makefile.mk

##    Post-setup Info    ##
$(info +----+)
$(info Build revision: $(VERSION))
$(info Configuration name: [$(TARGET_TYPE)_$(TARGET_ARCH)])
$(info Target executed: [$(MAKECMDGOALS)])
$(info CC_FLAGS: [$(CC_FLAGS)])
$(info CXX_FLAGS: [$(CXX_FLAGS)])
$(info +----+)
endif

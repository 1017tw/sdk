ALL_SUBDIRS := $(wildcard */)

# TODO: build videoin after implement software mjpeg compress
BUILD_SUBDIRS = $(filter-out videoin/,$(ALL_SUBDIRS))

.PHONY : all clean $(BUILD_SUBDIRS)
all : $(BUILD_SUBDIRS)

.NOTPARALLEL:
$(BUILD_SUBDIRS) :
	if [ -f "$@/Makefile" ]; then $(MAKE) -C $@ || exit 1; fi

.ONESHELL:
clean:
	@for DIR in $(BUILD_SUBDIRS); do
		if [ -f "$$DIR/Makefile" ]; then $(MAKE) -C $$DIR clean || exit 1; fi
	done

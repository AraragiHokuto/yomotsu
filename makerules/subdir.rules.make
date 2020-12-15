# Makefiles rules for building subdirectiories
# Define targets in ${SUBDIRS} before inclusion

.MAIN: all
all: ${SUBDIRS}

.for SUBDIR in ${SUBDIRS}

${SUBDIR}: .PHONY
	@echo "Building directory ${THIS_PREFIX}${SUBDIR}"
	@$(MAKE) -C ${SUBDIR}
.endfor

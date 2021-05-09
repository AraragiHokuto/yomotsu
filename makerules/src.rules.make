# Makefile rules for building source files

.ifndef BUILD_DIR
.error "In-tree build not supported. Setup your build env first."
.endif

RELA_PATH	!= git rev-parse --show-prefix

OBJ_PATH := ${BUILD_DIR}/${RELA_PATH}

.SUFFIXES: .c .c.o .s .s.o

__make_objdir != echo ${OBJ_PATH}; mkdir -p ${OBJ_PATH}

.OBJDIR: ${OBJ_PATH}

__makedirs: .EXEC
	@mkdir -p ${OBJS:H:O:u:@DIR@${OBJ_PATH}/${DIR}@}

.c.c.o: __makedirs
	@echo -e "\tCC\t$@"
	@$(CC) $(CFLAGS) -c -o$@ $< -MMD -MT$@

.s.s.o: __makedirs
	@echo -e "\tAS\t$@"
	@$(AS) $(ASFLAGS) -c -o$@ $<

OBJS	+=\
	${SRCS:M*.s:@ASMSRC@${ASMSRC:S/.s/.s.o/}@}	\
	${SRCS:M*.c:@CSRC@${CSRC:S/.c/.c.o/}@}

DEPS	= ${SRCS:@SRC@${SRC}.d@}
.for DEP in ${DEPS}
.dinclude "${DEP}"
.endfor

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra
LEVELS = O0 Og O1 Os O2 O3 Ofast

TARGETS = $(addprefix ci_,$(LEVELS))

.PHONY: all clean run constant_immutable_report

all: $(TARGETS)

ci_%: constant_immutable.c
	$(CC) $(CFLAGS) -$* -o $@ $<

run: all
	@for lvl in $(LEVELS); do \
		echo "--- -$$lvl ---"; \
		./ci_$$lvl; \
	done

# Run a shell command at each optimization level and group levels that
# produce identical results.  $(1) = shell fragment that sets $$cur.
define for_each_level
	prev=""; group=""; \
	for lvl in $(LEVELS); do \
		$(1); \
		if [ "$$cur" = "$$prev" ]; then \
			group="$$group, -$$lvl"; \
		else \
			if [ -n "$$prev" ]; then \
				printf "  %-40s %s\n" "$$group" "$$prev"; \
			fi; \
			group="-$$lvl"; \
		fi; \
		prev="$$cur"; \
	done; \
	printf "  %-40s %s\n" "$$group" "$$prev"
endef

# Convenience: grep runtime output of ci_$$lvl for a tagged line.
run_grep = cur=$$(./ci_$$lvl | grep '^\[$(1)\]' | sed 's/^\[$(1)\] //')

# Convenience: try compiling with -D flag; cur becomes one of two labels.
compile_cmd = $(CC) $(CFLAGS) -$$lvl $(4) -D$(1) -c constant_immutable.c \
		-o /dev/null 2>/dev/null \
		&& cur="$(2)" || cur="$(3)"

constant_immutable_report: all
	@echo "constant vs immutable in C99 -- $(CC) $$($(CC) -dumpversion)"
	@echo ""
	@echo "Compile-time: constant expression in case label (C99 6.6, 6.8.4.2)"
	@$(call for_each_level,$(call compile_cmd,TEST_CASE_DEFINE,case_define: compiled,case_define: compile error))
	@$(call for_each_level,$(call compile_cmd,TEST_CASE_ENUM,case_enum: compiled,case_enum: compile error))
	@$(call for_each_level,$(call compile_cmd,TEST_CASE_SIZEOF,case_sizeof: compiled,case_sizeof: compile error))
	@$(call for_each_level,$(call compile_cmd,TEST_CASE_CAST_FLOAT,case_cast_float: compiled,case_cast_float: compile error))
	@$(call for_each_level,$(call compile_cmd,TEST_CASE_CHAR,case_char: compiled,case_char: compile error))
	@$(call for_each_level,$(call compile_cmd,TEST_CASE_CONST_INT,case_const_int: compiled (gcc extension),case_const_int: compile error))
	@$(call for_each_level,$(call compile_cmd,TEST_VLA_CONST,arr[const_int]: no VLA (unexpected),arr[const_int]: VLA warning,-Wvla -Werror=vla))
	@echo ""
	@echo "Compile-time: mutability and addressability"
	@$(call for_each_level,$(call compile_cmd,TEST_ENUM_ASSIGN,enum_assign: compiled (unexpected),enum_assign: compile error (expected)))
	@$(call for_each_level,$(call compile_cmd,TEST_ENUM_ADDR,enum_addr: compiled (unexpected),enum_addr: compile error (expected)))
	@$(call for_each_level,$(call compile_cmd,TEST_CONST_ADDR,const_addr: compiled (expected),const_addr: compile error (unexpected)))
	@$(call for_each_level,$(call compile_cmd,TEST_DEFINE_REDEFINE,define_redefine: compiled (expected),define_redefine: compile error (unexpected)))
	@echo ""
	@echo "Experiment 1: modify const local via cast (UB: C99 6.7.3.5)"
	@$(call for_each_level,$(call run_grep,exp1))
	@echo ""
	@echo "Experiment 2: memory permissions (C99 6.7.3, footnote 114)"
	@$(call for_each_level,$(call run_grep,exp2:g_const))
	@$(call for_each_level,$(call run_grep,exp2:g_mutable))
	@$(call for_each_level,$(call run_grep,exp2:local_const))

clean:
	rm -f $(TARGETS)

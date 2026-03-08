CC = gcc
CFLAGS = -std=c99 -Wall -Wextra
LEVELS = O0 Og O1 Os O2 O3 Ofast

TARGETS = $(addprefix ci_,$(LEVELS))

.PHONY: all clean run constant_immutable_report func_designator_report opaque_report lifetime_ub_report strict_alias_report array_param_report quiz1_q1_report

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

# Convenience: grep runtime output for a tagged line.
# $(1)=tag $(2)=binary (default: ci_$$lvl)
run_grep = cur=$$(./$(if $(2),$(2),ci_$$lvl) | grep '^\[$(1)\]' | sed 's/^\[$(1)\] //')

# Convenience: try compiling with -D flag; cur becomes one of two labels.
# $(1)=flag $(2)=success label $(3)=fail label $(4)=extra flags $(5)=source file (default: constant_immutable.c)
compile_cmd = $(CC) $(CFLAGS) -$$lvl $(4) -D$(1) -c $(if $(5),$(5),constant_immutable.c) \
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

# Convenience: compile_cmd variant for opaque.c
opaque_compile = $(CC) $(CFLAGS) -$$lvl -D$(1) -c opaque.c \
		-o /dev/null 2>/dev/null \
		&& cur="$(2)" || cur="$(3)"

opaque_%: opaque.c
	$(CC) $(CFLAGS) -$* -o $@ $<

opaque_report: $(addprefix opaque_,$(LEVELS))
	@echo "Incomplete type (opaque pointer) test -- $(CC) $$($(CC) -dumpversion)"
	@echo ""
	@echo "Compile-time: what works with incomplete type (C99 6.2.5)"
	@$(call for_each_level,$(call opaque_compile,TEST_POINTER,struct opaque *p: compiled,struct opaque *p: compile error))
	@$(call for_each_level,$(call opaque_compile,TEST_OBJECT,struct opaque x: compiled (unexpected),struct opaque x: compile error))
	@$(call for_each_level,$(call opaque_compile,TEST_ARRAY,struct opaque a[3]: compiled (unexpected),struct opaque a[3]: compile error))
	@$(call for_each_level,$(call opaque_compile,TEST_SIZEOF_STRUCT,sizeof(struct opaque): compiled (unexpected),sizeof(struct opaque): compile error))
	@echo ""
	@echo "Runtime: sizeof pointer to incomplete type"
	@$(call for_each_level,cur=$$(./opaque_$$lvl))
	@echo ""

func_designator: func_designator.c
	$(CC) $(CFLAGS) -o $@ $<

func_designator_report: func_designator
	@echo "Function designator conversion test -- $(CC) $$($(CC) -dumpversion)"
	@echo ""
	@./func_designator

# lifetime_ub: demonstrate UB from accessing object past its lifetime
lifetime_ub_%: lifetime_ub.c
	$(CC) $(CFLAGS) -$* -o $@ $<

lifetime_ub_report: $(addprefix lifetime_ub_,$(LEVELS))
	@echo "Object lifetime UB test -- $(CC) $$($(CC) -dumpversion)"
	@echo ""
	@echo "v1: foo() direct return (GCC replaces with NULL)"
	@$(call for_each_level,$(call run_grep,v1,lifetime_ub_$$lvl))
	@echo ""
	@echo "v2: bar() smuggled via output parameter"
	@$(call for_each_level,$(call run_grep,v2,lifetime_ub_$$lvl))
	@echo ""
	@echo "v3: after second bar(99) — storage reuse"
	@$(call for_each_level,$(call run_grep,v3,lifetime_ub_$$lvl))

# strict_alias: strict aliasing violation with float/int
strict_alias_%: strict_alias.c
	$(CC) $(CFLAGS) -$* -o $@ $<

strict_alias_report: $(addprefix strict_alias_,$(LEVELS))
	@echo "Strict aliasing test -- $(CC) $$($(CC) -dumpversion)"
	@echo ""
	@echo "v1: int* alias to float (strict aliasing violation)"
	@$(call for_each_level,$(call run_grep,v1,strict_alias_$$lvl))
	@echo ""
	@echo "v2: memcpy version (well-defined)"
	@$(call for_each_level,$(call run_grep,v2,strict_alias_$$lvl))
	@echo ""
	@echo "v3: unsigned char* version (always legal)"
	@$(call for_each_level,$(call run_grep,v3,strict_alias_$$lvl))
	@echo ""
	@echo "v4: IEEE 754 bit pattern of 1.0f"
	@$(call for_each_level,$(call run_grep,v4,strict_alias_$$lvl))

# array_param: array parameter decay — f(int a[10]) vs g(int (*a)[10])
array_param: array_param.c
	$(CC) $(CFLAGS) -o $@ $<

array_param_report: array_param
	@echo "Array parameter decay test -- $(CC) $$($(CC) -dumpversion)"
	@echo ""
	@./array_param

# quiz1_q1: delegated to quiz_linked_list/
quiz1_q1_report:
	$(MAKE) -C quiz_linked_list q1_report

# list_arr_bench: linked list vs array insert benchmark
BENCH_MAX_N ?= 50000
BENCH_TRIALS ?= 5
BENCH_SIZES ?= 4 32 128 512
BENCH_TITLE ?=

# Helper: run Python via uv (preferred) or fallback to python3+pip
define run_python
	@if command -v uv >/dev/null 2>&1; then \
		uv run $(1); \
	else \
		python3 -c "import matplotlib" 2>/dev/null || \
			pip3 install --user matplotlib; \
		python3 $(1); \
	fi
endef

list_arr_bench: list_arr_bench.c
	$(CC) $(CFLAGS) -O2 -o $@ $<

bench_%.csv: list_arr_bench
	./list_arr_bench $(BENCH_MAX_N) $(BENCH_TRIALS) $* > $@
	@echo "Saved: $@ ($$(wc -l < $@) lines)"

BENCH_CSVS = $(foreach s,$(BENCH_SIZES),bench_$(s).csv)

bench_sizes.svg: $(BENCH_CSVS) plot_bench.py
	$(call run_python,plot_bench.py . \
		--sizes "$(subst $(eval) ,:,$(BENCH_CSVS))" \
		$(if $(BENCH_TITLE),--title "$(BENCH_TITLE)"))

list_arr_bench_report: $(BENCH_CSVS) bench_sizes.svg
	@echo "Linked list vs Array insert benchmark -- $(CC) $$($(CC) -dumpversion)"
	@echo "Element sizes: $(BENCH_SIZES) bytes"
	@echo "CSVs: $(BENCH_CSVS)"
	@echo "SVG:  bench_sizes.svg"
	@echo ""
	@for f in $(BENCH_CSVS); do echo "=== $$f ===" && cat $$f && echo ""; done

clean:
	rm -f $(TARGETS) $(addprefix opaque_,$(LEVELS)) func_designator \
		$(addprefix lifetime_ub_,$(LEVELS)) \
		$(addprefix strict_alias_,$(LEVELS)) \
		array_param \
		list_arr_bench bench_*.csv bench_combined.svg bench_sizes.svg

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

# Group levels producing identical output for a given grep tag.
define group_by
	prev=""; group=""; \
	for lvl in $(LEVELS); do \
		cur=$$(./ci_$$lvl | grep '^\[$(1)\]' | sed 's/^\[$(1)\] //'); \
		if [ "$$cur" = "$$prev" ]; then \
			group="$$group, -$$lvl"; \
		else \
			if [ -n "$$prev" ]; then \
				printf "  %-40s %s\n" "$$group" "$$prev_out"; \
			fi; \
			group="-$$lvl"; prev_out="$$cur"; \
		fi; \
		prev="$$cur"; \
	done; \
	printf "  %-40s %s\n" "$$group" "$$prev_out"
endef

constant_immutable_report: all
	@echo "constant vs immutable in C99 -- $(CC) $$($(CC) -dumpversion)"
	@echo ""
	@echo "Experiment 1: constant expression values (C99 6.6)"
	@$(call group_by,exp1)
	@printf "  %-40s " "const int in case label:"
	@echo 'const int N=10; int main(void){switch(0){case N:break;}return 0;}' \
		| $(CC) $(CFLAGS) -x c -c - -o /dev/null 2>/dev/null \
		&& echo "compiled (unexpected)" || echo "compile error (expected)"
	@echo ""
	@echo "Experiment 2: modify const local via cast (UB: C99 6.7.3.5)"
	@$(call group_by,result)
	@echo ""
	@echo "Experiment 3: memory permissions (C99 6.7.3, footnote 114)"
	@$(call group_by,exp3:g_const)
	@$(call group_by,exp3:g_mutable)
	@$(call group_by,exp3:local_const)

clean:
	rm -f $(TARGETS)

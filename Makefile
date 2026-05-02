# =============================================================================
# ft_ping — Makefile (Ecole 42, Subject v5.1)
# Subject zorunlu rules: all / clean / fclean / re
# Ek hedefler: run / norm / cppcheck / misra / test / valgrind / asan / cmake
# =============================================================================

# ---------------------------------------------------------------------------
# Compiler & flags
# ---------------------------------------------------------------------------
CC      = gcc
STD     = -std=c11
CFLAGS  = -Wall -Wextra -Werror $(STD) -g -Ihdr
DEPFLAGS = -MMD -MP

# Sanitizer / strict flags
ASAN_FLAGS  = -fsanitize=address -fno-omit-frame-pointer
STRICT_FLAGS = -Wpedantic -Wshadow -Wconversion -Wcast-align -Wstrict-prototypes

# ---------------------------------------------------------------------------
# Directories
# ---------------------------------------------------------------------------
SRCDIR  = src
OBJDIR  = obj
HDRDIR  = hdr
OUTDIR  = output
TESTDIR = test

# ---------------------------------------------------------------------------
# Project name & files
# ---------------------------------------------------------------------------
NAME = $(OUTDIR)/ft_ping
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)

# ---------------------------------------------------------------------------
# Subject mandatory rules
# ---------------------------------------------------------------------------
all: $(NAME)

$(NAME): $(OBJS)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) -lm

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	rm -f $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)
	rm -rf $(OUTDIR)

re: fclean all

# Header dependency tracking (auto-generated *.d files)
-include $(DEPS)

# ---------------------------------------------------------------------------
# Run target — supports ARGS="..." and SUDO=1
# ---------------------------------------------------------------------------
ARGS ?=
SUDO ?= 0

run: $(NAME)
	@if [ "$(SUDO)" = "1" ]; then \
		echo "Running $(NAME) with sudo $(ARGS)"; \
		sudo $(NAME) $(ARGS); \
	else \
		echo "Running $(NAME) $(ARGS)"; \
		$(NAME) $(ARGS); \
	fi

# Apply CAP_NET_RAW so binary runs without sudo
setcap: $(NAME)
	sudo setcap cap_net_raw+ep $(NAME)

# ---------------------------------------------------------------------------
# Quality — Norminette (Ecole 42)
# ---------------------------------------------------------------------------
NORM_PATHS = $(SRCDIR) $(HDRDIR)

norm:
	@command -v norminette >/dev/null 2>&1 || { \
		echo "norminette not found. Install: pip install norminette"; exit 1; }
	@echo "==> Running norminette on $(NORM_PATHS)"
	@norminette $(NORM_PATHS) || (echo "norminette failed"; exit 1)

# ---------------------------------------------------------------------------
# Quality — Cppcheck (general static analysis)
# ---------------------------------------------------------------------------
CPPCHECK_OUT = cppcheck.txt

cppcheck:
	@command -v cppcheck >/dev/null 2>&1 || { \
		echo "cppcheck not found. Install: apt-get install cppcheck"; exit 1; }
	@echo "==> Running cppcheck (general)"
	cppcheck --enable=all --inconclusive --std=c11 --force \
		--suppress=missingIncludeSystem \
		-I $(HDRDIR) \
		--quiet $(SRCDIR) 2> $(CPPCHECK_OUT) || true
	@echo "Output: $(CPPCHECK_OUT)"
	@cat $(CPPCHECK_OUT)

# ---------------------------------------------------------------------------
# Quality — MISRA C:2012 (cppcheck addon)
# ---------------------------------------------------------------------------
MISRA_OUT = misra.txt
MISRA_ADDON ?= /usr/share/cppcheck/addons/misra.py

misra:
	@command -v cppcheck >/dev/null 2>&1 || { \
		echo "cppcheck not found"; exit 1; }
	@command -v python3 >/dev/null 2>&1 || { \
		echo "python3 not found"; exit 1; }
	@echo "==> Generating cppcheck dumps"
	cppcheck --dump --std=c11 -I $(HDRDIR) $(SRCDIR)/*.c >/dev/null 2>&1
	@echo "==> Running MISRA C:2012 addon"
	@if [ -f "$(MISRA_ADDON)" ]; then \
		python3 $(MISRA_ADDON) $(SRCDIR)/*.dump > $(MISRA_OUT) 2>&1 || true; \
	else \
		python3 -m cppcheckdata --help >/dev/null 2>&1 || true; \
		cppcheck --addon=misra --std=c11 -I $(HDRDIR) $(SRCDIR) 2> $(MISRA_OUT) || true; \
	fi
	@echo "Output: $(MISRA_OUT)"
	@cat $(MISRA_OUT)

# ---------------------------------------------------------------------------
# Quality — Strict build (extra warnings)
# ---------------------------------------------------------------------------
strict:
	$(MAKE) re CFLAGS="$(CFLAGS) $(STRICT_FLAGS)"

# ---------------------------------------------------------------------------
# Tests — basic CLI/regression tests
# ---------------------------------------------------------------------------
test: $(NAME)
	@echo "==> ft_ping smoke tests"
	@echo "--- T-505: -? usage"
	@$(NAME) -? >/dev/null 2>&1 && echo "  OK" || echo "  FAIL"
	@echo "--- T-511: no-args usage"
	@$(NAME) >/dev/null 2>&1; \
		if [ $$? -eq 2 ] || [ $$? -eq 1 ]; then echo "  OK"; else echo "  FAIL"; fi
	@echo "--- T-510: unknown host"
	@$(NAME) nonexistent.invalid.tld >/dev/null 2>&1; \
		if [ $$? -ne 0 ]; then echo "  OK"; else echo "  FAIL"; fi
	@echo "--- T-512: invalid flag"
	@$(NAME) -X 8.8.8.8 >/dev/null 2>&1; \
		if [ $$? -ne 0 ]; then echo "  OK"; else echo "  FAIL"; fi
	@echo "==> Smoke tests done. Network tests require root/CAP_NET_RAW."
	@if [ -d "$(TESTDIR)" ]; then \
		echo "==> Running custom test scripts in $(TESTDIR)/"; \
		for t in $(TESTDIR)/*.sh; do \
			[ -f "$$t" ] && sh "$$t" || true; \
		done; \
	fi

# ---------------------------------------------------------------------------
# Valgrind — memory leak / invalid access (requires sudo for ICMP)
# ---------------------------------------------------------------------------
VALGRIND_HOST ?= 127.0.0.1
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes \
                 --error-exitcode=1

valgrind: $(NAME)
	@command -v valgrind >/dev/null 2>&1 || { \
		echo "valgrind not found"; exit 1; }
	sudo valgrind $(VALGRIND_FLAGS) $(NAME) -c 2 $(VALGRIND_HOST)

# ---------------------------------------------------------------------------
# AddressSanitizer build
# ---------------------------------------------------------------------------
asan:
	$(MAKE) re CFLAGS="$(CFLAGS) $(ASAN_FLAGS)"

# ---------------------------------------------------------------------------
# CMake delegation (out-of-source build under build/)
# ---------------------------------------------------------------------------
CMAKE_BUILD_DIR = build

cmake-build:
	@command -v cmake >/dev/null 2>&1 || { echo "cmake not found"; exit 1; }
	cmake -S . -B $(CMAKE_BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(CMAKE_BUILD_DIR) -j

cmake-clean:
	rm -rf $(CMAKE_BUILD_DIR)

# ---------------------------------------------------------------------------
# Reference (inetutils-2.0) diff helper
# ---------------------------------------------------------------------------
DIFF_HOST ?= 127.0.0.1
DIFF_COUNT ?= 3

diff-ref: $(NAME)
	@command -v ping >/dev/null 2>&1 || { echo "system ping not found"; exit 1; }
	@echo "==> Comparing ft_ping vs system ping ($(DIFF_HOST), $(DIFF_COUNT) packets)"
	@ping -c $(DIFF_COUNT) $(DIFF_HOST) > /tmp/ref-ping.txt 2>&1 || true
	@sudo $(NAME) -c $(DIFF_COUNT) $(DIFF_HOST) > /tmp/our-ping.txt 2>&1 || true
	-@diff -u /tmp/ref-ping.txt /tmp/our-ping.txt || true

# ---------------------------------------------------------------------------
# Help
# ---------------------------------------------------------------------------
help:
	@echo "ft_ping Makefile — available targets:"
	@echo ""
	@echo "  Subject mandatory:"
	@echo "    all          Build $(NAME)"
	@echo "    clean        Remove object/dependency files"
	@echo "    fclean       clean + remove executable"
	@echo "    re           fclean + all"
	@echo ""
	@echo "  Run:"
	@echo "    run          Run binary (ARGS=\"...\" SUDO=1)"
	@echo "    setcap       Apply CAP_NET_RAW to binary"
	@echo ""
	@echo "  Quality:"
	@echo "    norm         Run Ecole 42 norminette"
	@echo "    cppcheck     Run cppcheck static analysis"
	@echo "    misra        Run MISRA C:2012 addon"
	@echo "    strict       Rebuild with -Wpedantic -Wshadow -Wconversion"
	@echo ""
	@echo "  Test:"
	@echo "    test         Smoke tests (CLI/exit-code)"
	@echo "    valgrind     Run under valgrind (VALGRIND_HOST=...)"
	@echo "    asan         Rebuild with AddressSanitizer"
	@echo "    diff-ref     Diff against system ping (DIFF_HOST=...)"
	@echo ""
	@echo "  CMake:"
	@echo "    cmake-build  Configure + build via CMake"
	@echo "    cmake-clean  Remove build/ directory"

# ---------------------------------------------------------------------------
# Phony targets
# ---------------------------------------------------------------------------
.PHONY: all clean fclean re run setcap \
        norm cppcheck misra strict \
        test valgrind asan \
        cmake-build cmake-clean \
        diff-ref help

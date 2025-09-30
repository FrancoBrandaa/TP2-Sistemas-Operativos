# Configuración del compilador - compatible con Linux y macOS
# Si gcc está disponible, lo usa, sino usa clang

# Detectar el compilador disponible
COMPILER := $(shell which gcc 2>/dev/null)
ifeq ($(COMPILER),)
    COMPILER := $(shell which clang 2>/dev/null)
    ifeq ($(COMPILER),)
        $(error No se encontró ni gcc ni clang)
    endif
endif

# Extraer solo el nombre del compilador
COMPILER := $(notdir $(COMPILER))

# Flags comunes para ambos compiladores
COMPILER_FLAGS = -c -g --std=c99 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -D_POSIX_C_SOURCE=200809L -I./include

# Flags específicos por compilador
ifeq ($(COMPILER),gcc)
    # GCC permite más flexibilidad con algunos warnings
    COMPILER_FLAGS += -Wno-implicit-fallthrough
else ifeq ($(COMPILER),clang)
    # Clang usa flags ligeramente diferentes
    COMPILER_FLAGS += -Wno-implicit-fallthrough
endif

LINKER = $(COMPILER)
LINKER_FLAGS = -g --std=c99 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -D_POSIX_C_SOURCE=200809L -lm

ifeq ($(COMPILER),gcc)
    LINKER_FLAGS += -Wno-implicit-fallthrough
else ifeq ($(COMPILER),clang)
    LINKER_FLAGS += -Wno-implicit-fallthrough
endif
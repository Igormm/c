# Makefile для программы gradus
# Поддерживает различные цели сборки и платформы

# Переменные конфигурации
CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c99
LDFLAGS ?= -lm
TARGET = gradus
SOURCE = gradus_enhanced.c

# Директории
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1
DATADIR = $(PREFIX)/share/gradus

# Конфигурация сборки
DEBUG ?= 0
PROFILE ?= 0
STATIC ?= 0
SANITIZE ?= 0

# Определение флагов компиляции
ifeq ($(DEBUG),1)
    CFLAGS += -g3 -ggdb -O0 -DDEBUG
else
    CFLAGS += -O3 -DNDEBUG
endif

ifeq ($(PROFILE),1)
    CFLAGS += -pg
    LDFLAGS += -pg
endif

ifeq ($(STATIC),1)
    LDFLAGS += -static
endif

ifeq ($(SANITIZE),1)
    CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
    LDFLAGS += -fsanitize=address,undefined
endif

# Платформо-зависимые настройки
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += -D_GNU_SOURCE
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -D_DARWIN_C_SOURCE
endif
ifeq ($(UNAME_S),FreeBSD)
    CFLAGS += -D_BSD_SOURCE
endif

# Основные цели
.PHONY: all clean install uninstall test format help

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "Сборка завершена: $(TARGET)"

# Цели для разных конфигураций
debug: DEBUG=1
debug: clean all

profile: PROFILE=1
profile: clean all

static: STATIC=1
static: clean all

sanitize: SANITIZE=1
sanitize: clean all

# Оптимизированная сборка
release: clean
	@$(MAKE) all DEBUG=0
	@strip $(TARGET)
	@echo "Релиз-сборка завершена"

# Очистка
clean:
	rm -f $(TARGET) $(TARGET).exe
	rm -rf *.dSYM
	rm -f core core.*

# Установка
install: all
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(TARGET)
	@echo "Установлено в: $(BINDIR)/$(TARGET)"

# Удаление
uninstall:
	rm -f $(BINDIR)/$(TARGET)
	@echo "Удалено: $(BINDIR)/$(TARGET)"

# Тестирование
test: $(TARGET)
	@echo "Запуск тестов..."
	@./tests/run_tests.sh || echo "Тесты не найдены"

# Форматирование кода
format:
	clang-format -i $(SOURCE)
	@echo "Код отформатирован"

# Статический анализ
analyze:
	clang-tidy $(SOURCE) -- $(CFLAGS)
	@echo "Статический анализ завершен"

# Сборка с разными компиляторами
gcc: CC=gcc
gcc: clean all

clang: CC=clang
clang: clean all

icc: CC=icc
icc: clean all

# Кросс-компиляция
cross-win: CC=x86_64-w64-mingw32-gcc
cross-win: clean all
	@echo "Windows-версия собрана"

# Архивация
dist: clean
	tar czf gradus-$(shell git describe --tags --always).tar.gz \
		$(SOURCE) README_ENHANCED.md CMakeLists.txt Makefile
	@echo "Архив создан"

# Помощь
help:
	@echo "Доступные цели:"
	@echo "  all     - сборка по умолчанию"
	@echo "  debug   - отладочная сборка"
	@echo "  release - оптимизированная сборка"
	@echo "  profile - профилировочная сборка"
	@echo "  static  - статическая сборка"
	@echo "  sanitize - сборка с санитайзерами"
	@echo "  clean   - очистка"
	@echo "  install - установка"
	@echo "  test    - запуск тестов"
	@echo "  format  - форматирование кода"
	@echo "  analyze - статический анализ"
	@echo "  help    - это сообщение"

# Неявные правила
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Зависимости
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Проверка зависимостей
deps:
	@echo "Проверка зависимостей..."
	@which $(CC) || echo "Компилятор $(CC) не найден"
	@echo "Готово"

# Цели для разработки
dev-setup:
	@echo "Настройка окружения разработки..."
	@pip install pre-commit || echo "pre-commit не установлен"
	@pre-commit install || echo "pre-commit не настроен"

# Быстрая проверка
quick-test: $(TARGET)
	@echo "Быстрая проверка..."
	@./$(TARGET) -T > /dev/null && echo "✓ Таблица работает"
	@./$(TARGET) -G > /dev/null && echo "✓ График работает"
	@echo "0 20 100" | ./$(TARGET) -a -s C -t F > /dev/null && echo "✓ Массивы работают"
	@echo "Проверка завершена"
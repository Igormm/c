#!/bin/bash

# Сборочный скрипт для программы gradus
# Поддерживает различные конфигурации и платформы

set -e  # Выход при ошибке

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Функции для вывода
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Параметры по умолчанию
COMPILER="gcc"
BUILD_TYPE="release"
ENABLE_SANITIZERS="no"
STATIC_BUILD="no"
VERBOSE="no"
INSTALL_PREFIX="/usr/local"

# Парсинг аргументов
while [[ $# -gt 0 ]]; do
    case $1 in
        --compiler)
            COMPILER="$2"
            shift 2
            ;;
        --debug)
            BUILD_TYPE="debug"
            shift
            ;;
        --release)
            BUILD_TYPE="release"
            shift
            ;;
        --sanitize)
            ENABLE_SANITIZERS="yes"
            shift
            ;;
        --static)
            STATIC_BUILD="yes"
            shift
            ;;
        --verbose)
            VERBOSE="yes"
            shift
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --help)
            echo "Использование: $0 [ОПЦИИ]"
            echo "Опции:"
            echo "  --compiler COMPILER    Использовать указанный компилятор (по умолчанию: gcc)"
            echo "  --debug               Сборка в режиме отладки"
            echo "  --release             Сборка в режиме релиза (по умолчанию)"
            echo "  --sanitize            Включить санитайзеры"
            echo "  --static              Статическая сборка"
            echo "  --verbose             Подробный вывод"
            echo "  --prefix PATH         Префикс установки (по умолчанию: /usr/local)"
            echo "  --help                Показать эту справку"
            exit 0
            ;;
        *)
            print_error "Неизвестная опция: $1"
            exit 1
            ;;
    esac
done

# Информация о сборке
print_info "Начинаем сборку gradus..."
print_info "Компилятор: $COMPILER"
print_info "Тип сборки: $BUILD_TYPE"
print_info "Санитайзеры: $ENABLE_SANITIZERS"
print_info "Статическая сборка: $STATIC_BUILD"
print_info "Префикс установки: $INSTALL_PREFIX"

# Проверка компилятора
if ! command -v $COMPILER &> /dev/null; then
    print_error "Компилятор $COMPILER не найден"
    exit 1
fi

# Определение флагов компиляции
CFLAGS="-Wall -Wextra -Wpedantic -std=c99"
LDFLAGS="-lm"

# Флаги в зависимости от типа сборки
if [[ "$BUILD_TYPE" == "debug" ]]; then
    CFLAGS="$CFLAGS -g3 -ggdb -O0 -DDEBUG"
else
    CFLAGS="$CFLAGS -O3 -DNDEBUG"
fi

# Санитайзеры
if [[ "$ENABLE_SANITIZERS" == "yes" ]]; then
    CFLAGS="$CFLAGS -fsanitize=address,undefined -fno-omit-frame-pointer"
    LDFLAGS="$LDFLAGS -fsanitize=address,undefined"
fi

# Статическая сборка
if [[ "$STATIC_BUILD" == "yes" ]]; then
    LDFLAGS="$LDFLAGS -static"
fi

# Платформо-зависимые настройки
UNAME_S=$(uname -s)
if [[ "$UNAME_S" == "Linux" ]]; then
    CFLAGS="$CFLAGS -D_GNU_SOURCE"
elif [[ "$UNAME_S" == "Darwin" ]]; then
    CFLAGS="$CFLAGS -D_DARWIN_C_SOURCE"
elif [[ "$UNAME_S" == "FreeBSD" ]]; then
    CFLAGS="$CFLAGS -D_BSD_SOURCE"
fi

# Вывод флагов при подробном режиме
if [[ "$VERBOSE" == "yes" ]]; then
    print_info "CFLAGS: $CFLAGS"
    print_info "LDFLAGS: $LDFLAGS"
fi

# Очистка предыдущих сборок
print_info "Очистка предыдущих сборок..."
rm -f gradus gradus.exe

# Сборка
print_info "Сборка программы..."
if [[ "$VERBOSE" == "yes" ]]; then
    $COMPILER $CFLAGS -o gradus gradus_enhanced.c $LDFLAGS
else
    $COMPILER $CFLAGS -o gradus gradus_enhanced.c $LDFLAGS 2>/dev/null
fi

# Проверка результата
if [[ -f gradus ]]; then
    print_success "Сборка завершена успешно!"
    
    # Информация о собранной программе
    print_info "Информация о программе:"
    ls -la gradus
    
    # Быстрая проверка функциональности
    print_info "Проверка функциональности..."
    if ./gradus -T > /dev/null 2>&1; then
        print_success "✓ Табличный вывод работает"
    else
        print_warning "⚠ Табличный вывод не работает"
    fi
    
    if echo "0 20 100" | ./gradus -a -s C -t F > /dev/null 2>&1; then
        print_success "✓ Работа с массивами работает"
    else
        print_warning "⚠ Работа с массивами не работает"
    fi
    
    print_success "Готово! Программа gradus собрана и готова к использованию."
    print_info "Примеры использования:"
    echo "  ./gradus -T                    # Вывести таблицу температур"
    echo "  ./gradus -G                    # Вывести график температур"
    echo "  ./gradus -s C -t F 25          # Конвертировать 25°C в °F"
    echo "  ./gradus -a -s C -t F \"0 20 100\"  # Конвертировать массив"
    
else
    print_error "Сборка не удалась!"
    exit 1
fi

# Функция установки
install_program() {
    print_info "Установка программы..."
    
    if [[ ! -w "$INSTALL_PREFIX/bin" ]] && [[ "$EUID" -ne 0 ]]; then
        print_warning "Нет прав на запись в $INSTALL_PREFIX/bin"
        print_info "Используйте sudo или установите с флагом --prefix"
        return 1
    fi
    
    mkdir -p "$INSTALL_PREFIX/bin"
    cp gradus "$INSTALL_PREFIX/bin/"
    chmod 755 "$INSTALL_PREFIX/bin/gradus"
    
    print_success "Программа установлена в $INSTALL_PREFIX/bin/gradus"
}

# Предложение установки
echo ""
read -p "Установить программу в $INSTALL_PREFIX/bin? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    install_program
fi

print_success "Готово!"
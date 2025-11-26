# Руководство по сборке программы gradus

## 🚀 Быстрый старт

### Самый простой способ
```bash
    # Скачайте и запустите сборочный скрипт
    ./build.sh
    # Или для отладочной сборки
    ./build.sh --debug
```

### Использование Make
```bash
    # Базовая сборка
    make

    # Релиз-сборка
    make release

    # Отладочная сборка
    make debug
```

### Использование CMake (рекомендуется)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## 📋 Системные требования

### Минимальные требования
- **Компилятор:** GCC 4.8+, Clang 3.4+, MSVC 2015+
- **Система:** Linux, macOS, Windows, FreeBSD
- **Библиотеки:** стандартная библиотека C (libc), математическая библиотека (libm)

### Рекомендуемые требования
- **Компилятор:** GCC 9+, Clang 10+, MSVC 2019+
- **CMake:** 3.10+
- **Make:** GNU Make 4.0+

## 🔧 Методы сборки

### 1. Сборочный скрипт (build.sh)

**Базовая сборка:**
```bash
./build.sh
```

**Настройка сборки:**
```bash
# Выбор компилятора
./build.sh --compiler clang

# Отладочная сборка
./build.sh --debug

# Статическая сборка
./build.sh --static

# Сборка с санитайзерами
./build.sh --sanitize

# Подробный вывод
./build.sh --verbose

# Установка в пользовательский каталог
./build.sh --prefix $HOME/.local
```

### 2. Makefile

**Основные цели:**
```bash
make                    # Базовая сборка
make release           # Оптимизированная сборка
make debug             # Отладочная сборка
make sanitize          # Сборка с санитайзерами
make static            # Статическая сборка
```

**Дополнительные цели:**
```bash
make test              # Запуск тестов
make install           # Установка
make clean             # Очистка
make format            # Форматирование кода
make analyze           # Статический анализ
```

**Кросс-компиляция:**
```bash
# Сборка разными компиляторами
make gcc
make clang
make icc

# Сборка для Windows
make cross-win
```

### 3. CMake (рекомендуемый метод)

**Базовая конфигурация:**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**Расширенная конфигурация:**
```bash
# Отладочная сборка
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Статическая сборка
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF

# Сборка с тестами
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON

# Сборка с санитайзерами
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON -DENABLE_UBSAN=ON

# Установка
cmake --install build --prefix /usr/local
```

**Генерация проектов:**
```bash
# Для Ninja
cmake -G Ninja -B build

# Для Xcode (macOS)
cmake -G Xcode -B build

# Для Visual Studio (Windows)
cmake -G "Visual Studio 16 2019" -B build
```

### 4. Docker

**Сборка образа:**
```bash
docker build -t gradus:latest .
```

**Запуск:**
```bash
# Запуск с таблицей температур
docker run --rm gradus:latest -T

# Конвертация температур
echo "25" | docker run --rm -i gradus:latest -s C -t F
```

**Разработка с Docker Compose:**
```bash
# Запуск окружения разработки
docker-compose up -d gradus-dev

# Выполнение команд в контейнере
docker-compose exec gradus-dev make test

# Статический анализ
docker-compose run --rm gradus-analyze
```

## 🏗️ Платформо-специфичная сборка

### Linux
```bash
# Установка зависимостей (Debian/Ubuntu)
sudo apt-get install build-essential cmake

# Установка зависимостей (Fedora)
sudo dnf install gcc cmake make

# Установка зависимостей (Arch)
sudo pacman -S base-devel cmake
```

### macOS
```bash
# Установка Xcode Command Line Tools
xcode-select --install

# Или через Homebrew
brew install cmake

# Сборка
make
```

### Windows
```bash
# Использование MinGW
mingw32-make

# Использование Visual Studio
cmake -G "Visual Studio 16 2019" -B build
cmake --build build --config Release
```

## 🧪 Тестирование

### Базовое тестирование
```bash
# Запуск тестов
make test

# Или с CMake
cd build && ctest --output-on-failure
```

### Ручное тестирование
```bash
# Проверка таблицы
./gradus -T

# Проверка графика
./gradus -G

# Проверка конвертации
./gradus -s C -t F 25

# Проверка массивов
echo "0 20 100" | ./gradus -a -s C -t F
```

## 📦 Установка

### Системная установка
```bash
# С правами root
sudo make install

# Через CMake
sudo cmake --install build
```

### Пользовательская установка
```bash
# В домашний каталог
./build.sh --prefix $HOME/.local

# Через Make
make install PREFIX=$HOME/.local
```

### Удаление
```bash
# Удаление
sudo make uninstall

# Или вручную
sudo rm /usr/local/bin/gradus
```

## 🔍 Отладка

### Отладочная сборка
```bash
# Сборка с отладочной информацией
./build.sh --debug

# Или через Make
make debug

# Или через CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### Использование отладчика
```bash
# GDB
gdb ./gradus
(gdb) run -s C -t F 25

# LLDB
lldb ./gradus
(lldb) run -s C -t F 25
```

### Санитайзеры
```bash
# Сборка с AddressSanitizer
./build.sh --sanitize

# Запуск с выводом ошибок
ASAN_OPTIONS=verbosity=1:abort_on_error=1 ./gradus -s C -t F 25
```

## 📊 Оптимизация

### Профилирование
```bash
# Сборка с профилированием
make profile

# Запуск с профилированием
./gradus -s C -t F 25
gprof gradus gmon.out > profile.txt
```

### Оптимизация компилятора
```bash
# Максимальная оптимизация
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O3 -march=native"

# Link Time Optimization
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

## 🛠️ Разработка

### Форматирование кода
```bash
# Форматирование
make format

# Или вручную
clang-format -i gradus_enhanced.c
```

### Статический анализ
```bash
# Анализ
make analyze

# Или отдельно
clang-tidy gradus_enhanced.c --
cppcheck --enable=all gradus_enhanced.c
```

### Pre-commit хуки
```bash
# Установка pre-commit
pip install pre-commit
pre-commit install

# Запуск хуков
pre-commit run --all-files
```

## 📋 Устранение неполадок

### Общие проблемы

**Ошибка компиляции с math.h:**
```bash
# Добавьте -lm в конец команды компиляции
gcc gradus_enhanced.c -o gradus -lm
```

**Ошибка с POSIX функциями:**
```bash
# Добавьте определения POSIX
CFLAGS="-D_POSIX_C_SOURCE=200809L"
```

**Ошибка на Windows:**
```bash
# Используйте MinGW или Visual Studio
# Для MinGW: x86_64-w64-mingw32-gcc
# Для MSVC: cl /Fe:gradus.exe gradus_enhanced.c
```

### Платформо-специфичные решения

**macOS:**
```bash
# Установка Xcode CLT
xcode-select --install

# Или через Homebrew
brew install gcc cmake
```

**Windows с MinGW:**
```bash
# Установка через MSYS2
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake

# Сборка
/mingw64/bin/gcc gradus_enhanced.c -o gradus.exe -lm
```

## 🚀 CI/CD

### GitHub Actions
Репозиторий включает полный CI/CD пайплайн:
- Сборка на Linux, macOS, Windows
- Тестирование с разными компиляторами
- Статический анализ
- Создание релизов
- Docker образы

### Локальный CI
```bash
# Запуск всех проверок
make test analyze format

# Или через Docker
docker-compose run --rm gradus-test
```

## 📚 Дополнительные ресурсы

- [CMake Documentation](https://cmake.org/documentation/)
- [GCC Manual](https://gcc.gnu.org/onlinedocs/)
- [Clang Documentation](https://clang.llvm.org/docs/)
- [Makefile Tutorial](https://makefiletutorial.com/)
- [Docker Best Practices](https://docs.docker.com/develop/dev-best-practices/)

## 🤝 Поддержка
    email:igor@makarovs.online
    subject:gradus-KIMI4Coding 
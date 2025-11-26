#define _POSIX_C_SOURCE 200809L  // Определение POSIX стандарта для использования современных функций

#include <ctype.h>   // Для функций работы с символами
#include <errno.h>   // Для обработки ошибок системного уровня
#include <math.h>    // Для математических функций (fabs, isinf, isnan)
#include <stdio.h>   // Для функций ввода/вывода
#include <stdlib.h>  // Для функций работы с памятью и процессом
#include <string.h>  // Для функций работы со строками
#include <unistd.h>  // Для функций работы с системой POSIX

/*
 * ((T)Scale, (T)Unit) Структура температур
 */
typedef enum {
    CELSIUS,      // Цельсий
    FAHRENHEIT,   // Фаренгейт
    KELVIN,       // Кельвин
    RANKINE,      // Ранкин
    REAUMUR,      // Реамюр
    DELISLE,      // Делисль
    NEWTON,       // Ньютон
    ROMER,        // Рёмер
    UNKNOWN       // Неизвестная шкала
} Temperature;

/*
 * Константы для температурных преобразований
 */
// Точка замерзания воды цельсия
#define WATER_FREEZING 0.0
// Точка кипения воды цельсия
#define WATER_BOILING 100.0

// Соотношение между шкалами Фаренгейта и Цельсия
// 9/5 для преобразования Цельсий ↔ Фаренгейт
#define FAHRENHEIT_RATIO (9.0 / 5.0)
// 5/9 для преобразования Фаренгейт ↔ Цельсий
#define CELSIUS_RATIO (5.0 / 9.0)

/*
 * Абсолютный ноль в различных шкалах
 */ 
// Абсолютный ноль в Цельсиях
#define ABSOLUTE_ZERO_CELSIUS -273.15
// Абсолютный ноль в Фаренгейтах
#define ABSOLUTE_ZERO_FAHRENHEIT -459.67
// Абсолютный ноль в Ранкинах
#define ABSOLUTE_ZERO_RANKINE 0.0
// Абсолютный ноль в Кельвинах
#define ABSOLUTE_ZERO_KELVIN 0.0

/*
 * Специфические константы для реже используемых шкал
 */
// Коэффициент для шкалы Реомюра
#define REAUMUR_RATIO (4.0 / 5.0)
// Коэффициент для шкалы Делиля
#define DELISLE_RATIO (3.0 / 2.0)
// Коэффициент для шкалы Ньютона
#define NEWTON_RATIO (33.0 / 100.0)
// Коэффициент для шкалы Рёмера
#define ROMER_RATIO (21.0 / 40.0)
// Смещение для шкалы Рёмера
#define ROMER_OFFSET 7.5
// Базовая точка для шкалы Делиля
#define DELISLE_BASE 100.0

/*
 * Help - функция вывода справки по использованию программы
 */
static void Usage(void) {
  fprintf(stderr, "Usage: gradus -s <source_scale> -t <target_scale> [temperature ...]\n");
  fprintf(stderr, "gradus -T (to display the temperature scales table)\n");
  fprintf(stderr, "Available scales: C (Celsius), F (Fahrenheit), K (Kelvin), R (Rankine), Re (Reaumur), De (Delisle), N (Newton), Ro (Romer)\n");
  exit(EXIT_FAILURE);
}

/*
 * Функции конвертации температурных шкал
 * Все функции используют Цельсий как базовую шкалу для преобразований
 */

/* Конвертирует Цельсий в Фаренгейт */
static double Celsius_to_Fahrenheit(double c) {
  return c * FAHRENHEIT_RATIO + 32.0;
}

/* Конвертирует Фаренгейт в Цельсий */
static double Fahrenheit_to_Celsius(double f) {
  return (f - 32.0) * CELSIUS_RATIO;
}

/* Конвертирует Цельсий в Кельвин (абсолютная термодинамическая шкала) */
static double Celsius_to_Kelvin(double c) { return c - ABSOLUTE_ZERO_CELSIUS; }

/* Конвертирует Кельвин в Цельсий */
static double Kelvin_to_Celsius(double k) { return k + ABSOLUTE_ZERO_CELSIUS; }

/* Конвертирует Цельсий в Ранкин (абсолютная шкала Фаренгейта) */
static double Celsius_to_Rankine(double c) {
  return (c - ABSOLUTE_ZERO_CELSIUS) * FAHRENHEIT_RATIO;
}

/* Конвертирует Ранкин в Цельсий */
static double Rankine_to_Celsius(double r) {
  return (r - ABSOLUTE_ZERO_FAHRENHEIT + 32.0) * CELSIUS_RATIO;
}

/* Конвертирует Цельсий в Реомюр (историческая шкала, вода замерзает при 0°, кипит при 80°) */
static double Celsius_to_Reaumur(double c) { 
  return c * REAUMUR_RATIO; 
}

/* Конвертирует Реомюр в Цельсий */
static double Reaumur_to_Celsius(double re) { 
  return re / REAUMUR_RATIO; 
}

/* Конвертирует Цельсий в Делиль (историческая шкала, убывающая температура) */
static double Celsius_to_Delisle(double c) {
  return (DELISLE_BASE - c) * DELISLE_RATIO;
}

/* Конвертирует Делиль в Цельсий */
static double Delisle_to_Celsius(double de) {
  return DELISLE_BASE - de / DELISLE_RATIO;
}

/* Конвертирует Цельсий в Ньютон (историческая шкала Исаака Ньютона) */
static double Celsius_to_Newton(double c) { 
  return c * NEWTON_RATIO; 
}

/* Конвертирует Ньютон в Цельсий */
static double Newton_to_Celsius(double n) { 
  return n / NEWTON_RATIO; 
}

/* Конвертирует Цельсий в Рёмер (датская шкала, вода замерзает при 7.5°) */
static double Celsius_to_Romer(double c) {
  return c * ROMER_RATIO + ROMER_OFFSET;
}

/* Конвертирует Рёмер в Цельсий */
static double Romer_to_Celsius(double ro) {
  return (ro - ROMER_OFFSET) / ROMER_RATIO;
}

/*
* Таблица градусов температур - выводит таблицу с температурами в разных шкалах
*/
void Print_temperature_table() {
  // Заголовок таблицы
  printf("┌───────┬───────┬───────┬───────┐\n");
  printf("│ %-5s │ %-5s │ %-5s │ %-5s │\n", "°C", "°F", "K", "°R");
  printf("├───────┼───────┼───────┼───────┤\n");

  // Данные таблицы - выводим температуры от -20°C до 100°C с шагом 10°
  for (double c = -20.0; c <= 100.0; c += 10.0) {
    double f = Celsius_to_Fahrenheit(c);
    double k = Celsius_to_Kelvin(c);
    double r = Celsius_to_Rankine(c);

    printf("│ %5d │ %5.1f │ %5.1f │ %5.1f │\n", (int)c, f, k, r);
  }

  // Нижняя граница таблицы
  printf("└───────┴───────┴───────┴───────┘\n");
}

// Проверка допустимости шкалы с учетом регистра
static int Is_valid_scale(const char *scale) {
    // Массив допустимых обозначений температурных шкал
    const char *valid_scales[] = 
        {"C", "c", "F", "f", "K", "k", "R", "r", 
         "Re", "re", "RE", "De", "de", "DE",
         "N", "n", "Ro", "ro", "RO", NULL};

    // Проходим по всем допустимым шкалам и сравниваем с входной
    for (int i = 0; valid_scales[i] != NULL; i++) {
        // сравниваем две строки, 0 если строки идентичны, < 0 - если str1 меньше str2 (в лексикографическом порядке)
        // > 0 - если str1 больше str2
        if (strcmp(scale, valid_scales[i]) == 0) { 
            return 1;  // Возвращаем 1 (true) если шкала найдена
        }
    }
    return 0;  // Возвращаем 0 (false) если шкала не найдена
}

// Дополнительная проверка входных данных - преобразует строку в число double
static double Parse(const char *s) {
  char *e = NULL;  // Указатель для проверки конца строки
  errno = 0;  // Сбрасываем код ошибки
  double v = strtod(s, &e);  // Преобразуем строку в double

  // Проверка на переполнение
  if (errno == ERANGE) {
    fprintf(stderr, "Number out of range: %s\n", s);
    exit(EXIT_FAILURE);
  }

  // Проверка на некорректный ввод
  if (errno != 0 || e == s || (e != NULL && *e != '\0')) {
    fprintf(stderr, "Invalid number: %s\n", s);
    exit(EXIT_FAILURE);
  }

  // Проверка на специальные значения
  if (isinf(v) || isnan(v)) {
    fprintf(stderr, "Special number not allowed: %s\n", s);
    exit(EXIT_FAILURE);
  }

  return v;  // Возвращаем преобразованное значение
}

// Конвертация строки в enum Temperature - определяет тип температурной шкалы
static Temperature Get_type(const char *scale_name) {
    // Сравниваем строку с каждой допустимой шкалой и возвращаем соответствующий enum
    if (strcmp(scale_name, "C") == 0 || strcmp(scale_name, "c") == 0) return CELSIUS;
    if (strcmp(scale_name, "F") == 0 || strcmp(scale_name, "f") == 0) return FAHRENHEIT;
    if (strcmp(scale_name, "K") == 0 || strcmp(scale_name, "k") == 0) return KELVIN;
    if (strcmp(scale_name, "R") == 0 || strcmp(scale_name, "r") == 0) return RANKINE;
    if (strcmp(scale_name, "Re") == 0 || strcmp(scale_name, "re") == 0 || strcmp(scale_name, "RE") == 0) return REAUMUR;
    if (strcmp(scale_name, "De") == 0 || strcmp(scale_name, "de") == 0 || strcmp(scale_name, "DE") == 0) return DELISLE;
    if (strcmp(scale_name, "N") == 0 || strcmp(scale_name, "n") == 0) return NEWTON;
    if (strcmp(scale_name, "Ro") == 0 || strcmp(scale_name, "ro") == 0 || strcmp(scale_name, "RO") == 0) return ROMER;
    return UNKNOWN;  // Возвращаем UNKNOWN если шкала не распознана
}

// Конвертирование температур между различными шкалами
static double Convert_temperature(double value, const char *source, const char *target) {

    Temperature source_type = Get_type(source);  // Определяем исходную шкалу
    Temperature target_type = Get_type(target);  // Определяем целевую шкалу

    double celsius;  // Переменная для хранения значения в Цельсиях

    // Проверка валидности шкал - если исходная шкала неизвестна, выводим ошибку
    if (source_type == UNKNOWN) {  // ИСПРАВЛЕНИЕ: было if (source_type = UNKNOWN) - ошибка присваивания вместо сравнения
        fprintf(stderr, "Unknown source scale: %s\n", source);
        Usage();  // Вызываем функцию справки и выходим
    } 

    // Проверка физических ограничений для Kelvin и Rankine - температура не может быть отрицательной
    if ((source_type == KELVIN || source_type == RANKINE) && value < 0) {
        fprintf(stderr, "Error: Temperature in %s scale cannot be negative (got %.2f)\n", source, value);
        exit(EXIT_FAILURE);
    }

    // Преобразуем исходную температуру в Цельсий (switch для source)
    switch (source_type) {
        case CELSIUS:  // Если исходная шкала - Цельсий
          celsius = value;  // Просто присваиваем значение
          break;
        case FAHRENHEIT:  // Если исходная шкала - Фаренгейт
          celsius = Fahrenheit_to_Celsius(value);  // Конвертируем в Цельсий
          break;
        case KELVIN:  // Если исходная шкала - Кельвин
          celsius = Kelvin_to_Celsius(value);  // Конвертируем в Цельсий
          break;
        case RANKINE:  // Если исходная шкала - Ранкин
          celsius = Rankine_to_Celsius(value);  // Конвертируем в Цельсий
          break;
        case REAUMUR:  // Если исходная шкала - Реомюр
          celsius = Reaumur_to_Celsius(value);  // Конвертируем в Цельсий
          break;
        case DELISLE:  // Если исходная шкала - Делиль
          celsius = Delisle_to_Celsius(value);  // Конвертируем в Цельсий
          break;
        case NEWTON:  // Если исходная шкала - Ньютон
          celsius = Newton_to_Celsius(value);  // Конвертируем в Цельсий
          break;
        case ROMER:  // Если исходная шкала - Рёмер
          celsius = Romer_to_Celsius(value);  // Конвертируем в Цельсий
          break;
        default:  // На случай если что-то пошло не так
          celsius = 0; 
          break;
    }

    // Преобразуем из Цельсия в целевую шкалу (switch для target)
    switch (target_type) {
        case CELSIUS:  // Если целевая шкала - Цельсий
          return celsius;  // Возвращаем значение в Цельсиях
        case FAHRENHEIT:  // Если целевая шкала - Фаренгейт
          return Celsius_to_Fahrenheit(celsius);  // Конвертируем из Цельсия в Фаренгейт
        case KELVIN:  // Если целевая шкала - Кельвин
          return Celsius_to_Kelvin(celsius);  // Конвертируем из Цельсия в Кельвин
        case RANKINE:  // Если целевая шкала - Ранкин
          return Celsius_to_Rankine(celsius);  // Конвертируем из Цельсия в Ранкин
        case REAUMUR:  // Если целевая шкала - Реомюр
          return Celsius_to_Reaumur(celsius);  // Конвертируем из Цельсия в Реомюр
        case DELISLE:  // Если целевая шкала - Делиль
          return Celsius_to_Delisle(celsius);  // Конвертируем из Цельсия в Делиль
        case NEWTON:  // Если целевая шкала - Ньютон
          return Celsius_to_Newton(celsius);  // Конвертируем из Цельсия в Ньютон
        case ROMER:  // Если целевая шкала - Рёмер
          return Celsius_to_Romer(celsius);  // Конвертируем из Цельсия в Рёмер
        default:  // Если целевая шкала неизвестна
          fprintf(stderr, "Unknown target scale: %s\n", target);  // Сообщаем об ошибке
          Usage();  // Вызываем справку и выходим
          return 0; // Эта строка никогда не достигнется, но нужна для компилятора
    }
}

// Главная функция программы
int main(int argc, char *argv[]) {

  char *source_scale = NULL;  // Указатель для хранения исходной шкалы
  char *target_scale = NULL;  // Указатель для хранения целевой шкалы
  int opt;  // Переменная для хранения опции командной строки

  // Обрабатываем флаги командной строки с помощью getopt
  while ((opt = getopt(argc, argv, "s:t:T")) != -1) {
    switch (opt) 
    {
      case 's':  // Опция -s (source scale)
        source_scale = optarg;  // Сохраняем исходную шкалу
        break;
      case 't':  // Опция -t (target scale)
        target_scale = optarg;  // Сохраняем целевую шкалу
        break;
      case 'T':  // Опция -T (table)
        Print_temperature_table();  // Выводим таблицу температур
        return 0;  // Завершаем программу
        break;
      default:  // Если опция не распознана
        Usage();  // Вызываем справку и выходим
    }
  }

  // Проверяем, что обе шкалы указаны
  if (!source_scale || !target_scale) {
    fprintf(stderr, "Source and target scales are required\n");
    Usage();
  }

  // Проверяем валидность исходной шкалы
  if (!Is_valid_scale(source_scale)) {
    fprintf(stderr, "Invalid source scale: %s\n", source_scale);
    Usage();
  }

  // Проверяем валидность целевой шкалы
  if (!Is_valid_scale(target_scale)) {
    fprintf(stderr, "Invalid target scale: %s\n", target_scale);
    Usage();
  }

  // Если есть аргументы командной строки после опций
  if (optind < argc) {
    // Обработка температур из аргументов командной строки
    for (int i = optind; i < argc; ++i) {
      double v = Parse(argv[i]);  // Преобразуем строку в число
      double r = Convert_temperature(v, source_scale, target_scale);  // Конвертируем температуру

      // Избегаем вывода -0.00
      if (fabs(r) < 0.005 && r != 0.0) {
        r = 0.0;
      }
      printf("%.2f\n", r);  // Выводим результат с 2 знаками после запятой
    }
  } else {
    // Обработка температур из stdin
    char *line = NULL;  // Указатель для хранения строки
    size_t n = 0;  // Размер буфера
    ssize_t l;  // Длина прочитанной строки

    // Читаем строки из стандартного ввода
    while ((l = getline(&line, &n, stdin)) != -1) {
      if (l && line[l - 1] == '\n')  // Убираем символ новой строки
        line[l - 1] = '\0';
      if (l > 1) { // Игнорируем пустые строки
        double v = Parse(line);  // Преобразуем строку в число
        double r = Convert_temperature(v, source_scale, target_scale);  // Конвертируем температуру

        // Избегаем вывода -0.00
        if (fabs(r) < 0.005 && r != 0.0) {
          r = 0.0;
        }
        printf("%.2f\n", r);  // Выводим результат с 2 знаками после запятой
      }
    }
    free(line); // Критически важно - освобождаем память, выделенную getline
  }

  return 0;  // Успешное завершение программы
}
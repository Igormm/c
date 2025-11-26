#define _POSIX_C_SOURCE 200809L  // Определение POSIX стандарта для использования современных функций

#include <ctype.h>   // Для функций работы с символами
#include <errno.h>   // Для обработки ошибок системного уровня
#include <math.h>    // Для математических функций (fabs, isinf, isnan)
#include <stdio.h>   // Для функций ввода/вывода
#include <stdlib.h>  // Для функций работы с памятью и процессом
#include <string.h>  // Для функций работы со строками
#include <unistd.h>  // Для функций работы с системой POSIX
#include <getopt.h>  // Для расширенной обработки опций командной строки

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
 * Глобальные переменные для настроек
 */
static char *input_separator = NULL;   // Сепаратор для входных данных
static char *output_separator = NULL;  // Сепаратор для выходных данных
static int array_mode = 0;             // Режим работы с массивами
static int table_output = 0;           // Режим вывода таблицы
static int graph_output = 0;           // Режим вывода графика

/*
 * Help - функция вывода справки по использованию программы
 */
static void Usage(void) {
  fprintf(stderr, "Usage: gradus [OPTIONS] -s <source_scale> -t <target_scale> [temperature ...]\n");
  fprintf(stderr, "\nOptions:\n");
  fprintf(stderr, "  -s, --source <scale>      Source temperature scale\n");
  fprintf(stderr, "  -t, --target <scale>      Target temperature scale\n");
  fprintf(stderr, "  -T, --table              Display temperature conversion table\n");
  fprintf(stderr, "  -G, --graph              Display temperature conversion graph\n");
  fprintf(stderr, "  -a, --array              Enable array processing mode\n");
  fprintf(stderr, "  --in-sep <separator>      Input separator (default: whitespace)\n");
  fprintf(stderr, "  --out-sep <separator>     Output separator (default: newline)\n");
  fprintf(stderr, "  -h, --help               Display this help message\n");
  fprintf(stderr, "\nAvailable scales: C (Celsius), F (Fahrenheit), K (Kelvin), R (Rankine)\n");
  fprintf(stderr, "                   Re (Reaumur), De (Delisle), N (Newton), Ro (Romer)\n");
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
  int i;
  double c;
  
  // Заголовок таблицы
  printf("╔═══════════════╦═══════════════╦═══════════════╦═══════════════╦═══════════════╗\n");
  printf("║ %-13s ║ %-13s ║ %-13s ║ %-13s ║ %-13s ║\n", "Celsius (°C)", "Fahrenheit (°F)", "Kelvin (K)", "Rankine (°R)", "Reaumur (°Re)");
  printf("╠═══════════════╬═══════════════╬═══════════════╬═══════════════╬═══════════════╣\n");

  // Данные таблицы - выводим температуры от -40°C до 100°C с шагом 10°
  for (i = -4, c = -40.0; i <= 10; i++, c += 10.0) {
    double f = Celsius_to_Fahrenheit(c);
    double k = Celsius_to_Kelvin(c);
    double r = Celsius_to_Rankine(c);
    double re = Celsius_to_Reaumur(c);

    printf("║ %13.1f ║ %13.1f ║ %13.1f ║ %13.1f ║ %13.1f ║\n", c, f, k, r, re);
  }

  // Нижняя граница таблицы
  printf("╚═══════════════╩═══════════════╩═══════════════╩═══════════════╩═══════════════╝\n");
}

/*
* График температур - выводит ASCII-график сравнения температур
*/
void Print_temperature_graph() {
  int i, j;
  double c;
  double min_temp = -20.0, max_temp = 100.0;
  int graph_width = 60;
  
  printf("\nТемпературный график по шкалам\n");
  printf("═══════════════════════════════════════════════════════════════════════════════\n\n");
  
  // Заголовок графика
  printf("%-15s | %-10s | %-10s | %-10s | %-10s |\n", "Temperature", "Celsius", "Fahrenheit", "Kelvin", "Rankine");
  printf("%-15s | %-10s | %-10s | %-10s | %-10s |\n", "(°C)", "(°C)", "(°F)", "(K)", "(°R)");
  printf("----------------|------------|------------|------------|------------|\n");
  
  // Данные для графика - выводим ключевые точки
  double points[] = {-20.0, -10.0, 0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0};
  int num_points = sizeof(points) / sizeof(points[0]);
  
  for (i = 0; i < num_points; i++) {
    c = points[i];
    double f = Celsius_to_Fahrenheit(c);
    double k = Celsius_to_Kelvin(c);
    double r = Celsius_to_Rankine(c);
    
    // Визуализация для Цельсия (базовая шкала)
    int c_pos = (int)((c - min_temp) / (max_temp - min_temp) * graph_width);
    
    printf("%7.1f°C      |", c);
    
    // Визуализация для каждой шкалы
    printf(" %7.1f    |", c);
    printf(" %7.1f    |", f);
    printf(" %7.1f    |", k);
    printf(" %7.1f    |", r);
    printf("\n");
  }
  
  printf("----------------|------------|------------|------------|------------|\n");
  printf("\nПримечания:\n");
  printf("• Точка замерзания воды: 0°C = 32°F = 273.15K = 491.67°R\n");
  printf("• Точка кипения воды: 100°C = 212°F = 373.15K = 671.67°R\n");
  printf("• Абсолютный ноль: -273.15°C = -459.67°F = 0K = 0°R\n\n");
}

// Проверка допустимости шкалы с учетом регистра
static int Is_valid_scale(const char *scale) {
    // Массив допустимых обозначений температурных шкал
    const char *valid_scales[] = 
        {"C", "c", "F", "f", "K", "k", "R", "r", 
         "Re", "re", "RE", "De", "de", "DE",
         "N", "n", "Ro", "ro", "RO", NULL};

    // Проходим по всем допустимым шкалам и сравниваем с входной
    int i = 0;
    while (valid_scales[i] != NULL) {
        if (strcmp(scale, valid_scales[i]) == 0) { 
            return 1;  // Возвращаем 1 (true) если шкала найдена
        }
        i++;
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
    if (source_type == UNKNOWN) {
        fprintf(stderr, "Unknown source scale: %s\n", source);
        Usage();
    }
    
    // Проверка валидности целевой шкалы
    if (target_type == UNKNOWN) {
        fprintf(stderr, "Unknown target scale: %s\n", target);
        Usage();
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

// Функция для обработки массива значений с учетом сепараторов
static void Process_array(const char *input, const char *source_scale, const char *target_scale) {
    char *input_copy = strdup(input);  // Копируем входную строку для безопасной обработки
    char *token;
    int first = 1;  // Флаг для первого элемента (чтобы не выводить сепаратор перед ним)
    
    // Определяем разделители
    const char *delims = " \t\n";  // По умолчанию пробелы, табы и переводы строк
    if (input_separator != NULL) {
        delims = input_separator;  // Используем пользовательский сепаратор если задан
    }
    
    // Разбиваем строку на токены
    token = strtok(input_copy, delims);
    while (token != NULL) {
        if (strlen(token) > 0) {  // Пропускаем пустые токены
            double value = Parse(token);  // Преобразуем строку в число
            double result = Convert_temperature(value, source_scale, target_scale);  // Конвертируем
            
            // Избегаем вывода -0.00
            if (fabs(result) < 0.005 && result != 0.0) {
                result = 0.0;
            }
            
            // Выводим результат с учетом сепаратора
            if (!first && output_separator != NULL) {
                printf("%s", output_separator);
            }
            printf("%.2f", result);
            first = 0;
        }
        token = strtok(NULL, delims);  // Получаем следующий токен
    }
    
    // Добавляем перевод строки в конце если не задан пользовательский сепаратор
    if (output_separator == NULL) {
        printf("\n");
    }
    
    free(input_copy);  // Освобождаем память
}

// Главная функция программы
int main(int argc, char *argv[]) {

  char *source_scale = NULL;  // Указатель для хранения исходной шкалы
  char *target_scale = NULL;  // Указатель для хранения целевой шкалы
  
  // Расширенные опции командной строки
  static struct option long_options[] = {
      {"source", required_argument, 0, 's'},
      {"target", required_argument, 0, 't'},
      {"table", no_argument, 0, 'T'},
      {"graph", no_argument, 0, 'G'},
      {"array", no_argument, 0, 'a'},
      {"in-sep", required_argument, 0, 'i'},
      {"out-sep", required_argument, 0, 'o'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
  };
  
  int opt;  // Переменная для хранения опции командной строки
  int option_index = 0;
  
  // Обрабатываем флаги командной строки с помощью getopt_long
  while ((opt = getopt_long(argc, argv, "s:t:TGa:h", long_options, &option_index)) != -1) {
    switch (opt) {
      case 's':  // Опция -s (source scale)
        source_scale = optarg;  // Сохраняем исходную шкалу
        break;
      case 't':  // Опция -t (target scale)
        target_scale = optarg;  // Сохраняем целевую шкалу
        break;
      case 'T':  // Опция -T (table)
        table_output = 1;  // Устанавливаем флаг вывода таблицы
        break;
      case 'G':  // Опция -G (graph)
        graph_output = 1;  // Устанавливаем флаг вывода графика
        break;
      case 'a':  // Опция -a (array mode)
        array_mode = 1;  // Устанавливаем режим массива
        break;
      case 'i':  // Опция --in-sep (input separator)
        input_separator = optarg;
        break;
      case 'o':  // Опция --out-sep (output separator)
        output_separator = optarg;
        break;
      case 'h':  // Опция -h (help)
        Usage();  // Вызываем справку
        break;
      default:  // Если опция не распознана
        Usage();  // Вызываем справку и выходим
    }
  }
  
  // Если запрошен вывод таблицы
  if (table_output) {
      Print_temperature_table();
      return 0;
  }
  
  // Если запрошен вывод графика
  if (graph_output) {
      Print_temperature_graph();
      return 0;
  }

  // Проверяем, что обе шкалы указаны (кроме случаев с таблицей/графиком)
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
    int i;
    int first = 1;  // Флаг для первого элемента
    
    // Обработка температур из аргументов командной строки
    for (i = optind; i < argc; ++i) {
      if (array_mode) {
          // Режим обработки массива - обрабатываем весь аргумент как массив значений
          Process_array(argv[i], source_scale, target_scale);
      } else {
          // Обычный режим - каждый аргумент отдельное значение
          double v = Parse(argv[i]);  // Преобразуем строку в число
          double r = Convert_temperature(v, source_scale, target_scale);  // Конвертируем температуру

          // Избегаем вывода -0.00
          if (fabs(r) < 0.005 && r != 0.0) {
            r = 0.0;
          }
          
          // Выводим результат с учетом сепаратора
          if (!first && output_separator != NULL) {
              printf("%s", output_separator);
          }
          printf("%.2f", r);
          first = 0;
      }
    }
    
    // Добавляем перевод строки если не задан пользовательский сепаратор
    if (!array_mode && output_separator != NULL && !first) {
        printf("\n");
    } else if (!array_mode && output_separator == NULL && !first) {
        printf("\n");
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
        if (array_mode) {
            // Режим обработки массива
            Process_array(line, source_scale, target_scale);
        } else {
            // Обычный режим
            double v = Parse(line);  // Преобразуем строку в число
            double r = Convert_temperature(v, source_scale, target_scale);  // Конвертируем температуру

            // Избегаем вывода -0.00
            if (fabs(r) < 0.005 && r != 0.0) {
              r = 0.0;
            }
            printf("%.2f\n", r);  // Выводим результат с 2 знаками после запятой
        }
      }
    }
    free(line); // Критически важно - освобождаем память, выделенную getline
  }

  return 0;  // Успешное завершение программы
}
#!/usr/bin/env python3
"""
Скрипт для автоматического тестирования сборки программы gradus
Поддерживает различные методы сборки и конфигурации
"""

import os
import sys
import subprocess
import tempfile
import shutil
from pathlib import Path
from typing import List, Dict, Tuple, Optional

class BuildTester:
    def __init__(self, source_dir: Path, build_dir: Path):
        self.source_dir = source_dir
        self.build_dir = build_dir
        self.results = []
        
    def run_command(self, cmd: List[str], cwd: Optional[Path] = None, 
                   capture_output: bool = True) -> Tuple[int, str, str]:
        """Выполнение команды и возврат результата"""
        try:
            result = subprocess.run(
                cmd, 
                cwd=cwd or self.source_dir,
                capture_output=capture_output,
                text=True,
                timeout=300  # 5 минут таймаут
            )
            return result.returncode, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            return -1, "", "Command timed out"
        except FileNotFoundError:
            return -1, "", f"Command not found: {cmd[0]}"
        except Exception as e:
            return -1, "", str(e)
    
    def check_dependencies(self) -> Dict[str, bool]:
        """Проверка доступности зависимостей"""
        deps = {
            'gcc': False,
            'clang': False,
            'make': False,
            'cmake': False,
            'docker': False,
        }
        
        for tool in deps:
            ret, _, _ = self.run_command(['which', tool], capture_output=True)
            deps[tool] = ret == 0
        
        return deps
    
    def test_gcc_build(self) -> Dict[str, any]:
        """Тестирование сборки с GCC"""
        print("🧪 Тестирование сборки с GCC...")
        
        ret, out, err = self.run_command([
            'gcc', '-Wall', '-Wextra', '-std=c99', '-o', 'gradus_gcc', 
            'gradus_enhanced.c', '-lm'
        ], cwd=self.build_dir)
        
        success = ret == 0 and os.path.exists(self.build_dir / 'gradus_gcc')
        
        if success:
            # Тестирование программы
            test_ret, test_out, test_err = self.run_command([
                './gradus_gcc', '-T'
            ], cwd=self.build_dir)
            functional = test_ret == 0
        else:
            functional = False
        
        return {
            'success': success,
            'functional': functional,
            'output': out + err,
            'test_output': test_out + test_err if success else ""
        }
    
    def test_clang_build(self) -> Dict[str, any]:
        """Тестирование сборки с Clang"""
        print("🧪 Тестирование сборки с Clang...")
        
        ret, out, err = self.run_command([
            'clang', '-Wall', '-Wextra', '-std=c99', '-o', 'gradus_clang',
            'gradus_enhanced.c', '-lm'
        ], cwd=self.build_dir)
        
        success = ret == 0 and os.path.exists(self.build_dir / 'gradus_clang')
        
        if success:
            # Тестирование программы
            test_ret, test_out, test_err = self.run_command([
                './gradus_clang', '-T'
            ], cwd=self.build_dir)
            functional = test_ret == 0
        else:
            functional = False
        
        return {
            'success': success,
            'functional': functional,
            'output': out + err,
            'test_output': test_out + test_err if success else ""
        }
    
    def test_makefile_build(self) -> Dict[str, any]:
        """Тестирование сборки через Makefile"""
        print("🧪 Тестирование сборки через Makefile...")
        
        # Копируем Makefile во временную директорию
        shutil.copy(self.source_dir / 'Makefile', self.build_dir / 'Makefile')
        
        ret, out, err = self.run_command(['make', 'clean'], cwd=self.build_dir)
        ret, out, err = self.run_command(['make'], cwd=self.build_dir)
        
        success = ret == 0 and os.path.exists(self.build_dir / 'gradus')
        
        if success:
            # Тестирование программы
            test_ret, test_out, test_err = self.run_command([
                './gradus', '-T'
            ], cwd=self.build_dir)
            functional = test_ret == 0
        else:
            functional = False
        
        return {
            'success': success,
            'functional': functional,
            'output': out + err,
            'test_output': test_out + test_err if success else ""
        }
    
    def test_cmake_build(self) -> Dict[str, any]:
        """Тестирование сборки через CMake"""
        print("🧪 Тестирование сборки через CMake...")
        
        cmake_build_dir = self.build_dir / 'cmake_build'
        cmake_build_dir.mkdir(exist_ok=True)
        
        # Конфигурация
        ret, out, err = self.run_command([
            'cmake', '..', '-DCMAKE_BUILD_TYPE=Release'
        ], cwd=cmake_build_dir)
        
        if ret != 0:
            return {
                'success': False,
                'functional': False,
                'output': out + err,
                'test_output': 'CMake configuration failed'
            }
        
        # Сборка
        ret, out, err = self.run_command([
            'cmake', '--build', '.', '-j4'
        ], cwd=cmake_build_dir)
        
        success = ret == 0 and os.path.exists(cmake_build_dir / 'gradus')
        
        if success:
            # Тестирование программы
            test_ret, test_out, test_err = self.run_command([
                './gradus', '-T'
            ], cwd=cmake_build_dir)
            functional = test_ret == 0
        else:
            functional = False
        
        return {
            'success': success,
            'functional': functional,
            'output': out + err,
            'test_output': test_out + test_err if success else ""
        }
    
    def test_build_script(self) -> Dict[str, any]:
        """Тестирование сборочного скрипта"""
        print("🧪 Тестирование сборочного скрипта...")
        
        ret, out, err = self.run_command([
            './build.sh', '--verbose'
        ], cwd=self.source_dir)
        
        success = ret == 0 and os.path.exists(self.source_dir / 'gradus')
        
        if success:
            # Тестирование программы
            test_ret, test_out, test_err = self.run_command([
                './gradus', '-T'
            ], cwd=self.source_dir)
            functional = test_ret == 0
        else:
            functional = False
        
        return {
            'success': success,
            'functional': functional,
            'output': out + err,
            'test_output': test_out + test_err if success else ""
        }
    
    def test_docker_build(self) -> Dict[str, any]:
        """Тестирование Docker сборки"""
        print("🧪 Тестирование Docker сборки...")
        
        if not shutil.which('docker'):
            return {
                'success': False,
                'functional': False,
                'output': 'Docker not found',
                'test_output': 'Docker not available'
            }
        
        ret, out, err = self.run_command([
            'docker', 'build', '-t', 'gradus-test', '.'
        ], cwd=self.source_dir)
        
        if ret != 0:
            return {
                'success': False,
                'functional': False,
                'output': out + err,
                'test_output': 'Docker build failed'
            }
        
        # Тестирование образа
        test_ret, test_out, test_err = self.run_command([
            'docker', 'run', '--rm', 'gradus-test', '-T'
        ])
        
        return {
            'success': True,
            'functional': test_ret == 0,
            'output': out + err,
            'test_output': test_out + test_err
        }
    
    def run_all_tests(self) -> Dict[str, any]:
        """Запуск всех тестов сборки"""
        print("🚀 Запуск тестов сборки программы gradus...")
        print("=" * 60)
        
        # Проверка зависимостей
        print("📋 Проверка зависимостей...")
        deps = self.check_dependencies()
        for tool, available in deps.items():
            status = "✓" if available else "✗"
            print(f"  {status} {tool}")
        
        print("\n" + "=" * 60)
        
        # Тесты сборки
        tests = {
            'GCC': self.test_gcc_build if deps['gcc'] else None,
            'Clang': self.test_clang_build if deps['clang'] else None,
            'Makefile': self.test_makefile_build if deps['make'] else None,
            'CMake': self.test_cmake_build if deps['cmake'] else None,
            'Build Script': self.test_build_script,
            'Docker': self.test_docker_build if deps['docker'] else None,
        }
        
        results = {}
        for name, test_func in tests.items():
            if test_func:
                print(f"\n{name}:")
                print("-" * 40)
                results[name] = test_func()
            else:
                results[name] = {
                    'success': False,
                    'functional': False,
                    'output': f'{name} not available',
                    'test_output': 'Skipped'
                }
        
        # Сводка результатов
        print("\n" + "=" * 60)
        print("📊 СВОДКА РЕЗУЛЬТАТОВ")
        print("=" * 60)
        
        total_tests = len(results)
        successful_builds = sum(1 for r in results.values() if r['success'])
        functional_programs = sum(1 for r in results.values() if r['functional'])
        
        print(f"Всего тестов: {total_tests}")
        print(f"Успешных сборок: {successful_builds}/{total_tests}")
        print(f"Рабочих программ: {functional_programs}/{total_tests}")
        
        print("\nДетальные результаты:")
        for name, result in results.items():
            build_status = "✅ УСПЕШНО" if result['success'] else "❌ ОШИБКА"
            func_status = "✅ РАБОТАЕТ" if result['functional'] else "❌ НЕ РАБОТАЕТ"
            print(f"  {name:15} | Сборка: {build_status} | Функциональность: {func_status}")
        
        # Рекомендации
        print("\n💡 РЕКОМЕНДАЦИИ:")
        if successful_builds == 0:
            print("  🔧 Не удалось собрать программу ни одним способом.")
            print("     Установите GCC: sudo apt-get install build-essential")
        elif functional_programs == 0:
            print("  🔧 Программы собираются, но не работают.")
            print("     Проверьте вывод ошибок выше.")
        elif functional_programs < successful_builds:
            print("  ⚠️  Некоторые сборки работают нестабильно.")
            print("     Рекомендуется использовать CMake или Makefile.")
        else:
            print("  ✅ Все методы сборки работают корректно!")
        
        return {
            'total_tests': total_tests,
            'successful_builds': successful_builds,
            'functional_programs': functional_programs,
            'results': results,
            'dependencies': deps
        }


def main():
    """Главная функция"""
    # Определение директорий
    source_dir = Path(__file__).parent.absolute()
    build_dir = source_dir / 'test_build'
    
    # Очистка предыдущих тестов
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(exist_ok=True)
    
    # Запуск тестов
    tester = BuildTester(source_dir, build_dir)
    results = tester.run_all_tests()
    
    # Возврат кода ошибки для CI/CD
    if results['successful_builds'] == 0:
        sys.exit(1)  # Критическая ошибка - ничего не собралось
    elif results['functional_programs'] == 0:
        sys.exit(2)  # Ошибка - программы не работают
    else:
        sys.exit(0)  # Успех


if __name__ == '__main__':
    main()
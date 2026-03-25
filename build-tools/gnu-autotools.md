install gnu autotool

Установка Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

Добавляем Homebrew в PATH (для терминала на базе Zsh):
echo 'export PATH="/opt/homebrew/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc

Установка GNU Autotools через Homebrew
brew install automake autoconf libtool

brew install make

Это установит:
automake (для Makefile.am) # Должен быть GNU Make (не BSD Make)
autoconf (для configure.ac)
libtool (для сборки библиотек)

make --version
automake --version
autoconf --version
libtool --version

---

m4 — это макропроцессор, который используется Autotools для генерации скриптов.

autotools/m4/ — это директория для кастомных макросов, которые расширяют функциональность configure.ac.

```
autotools/
├── m4/                   # Кастомные макросы (как выше)
│   ├── check_ant.m4      # Проверка Ant
│   ├── java_options.m4   # Настройки Java
│   └── ...               # Другие макросы
├── config.rpath          # Скрипт для работы с путями (для libtool)
└── install-sh            # Скрипт для кроссплатформенной установки
```

1. configure.ac (главный конфигурационный файл)
   Назначение: Проверка системы, настройка параметров сборки
```
AC_INIT([myapp], [1.0], [bugs@example.com])  # Имя, версия, контакты
AM_INIT_AUTOMAKE([foreign])                  # Инициализация Automake

# Проверка компиляторов
AC_PROG_CC                                   # Поиск C-компилятора
AC_PROG_CXX                                  # Поиск C++ компилятора
AC_PROG_JAVAC                                # Поиск Java-компилятора

# Проверка библиотек
PKG_CHECK_MODULES([GTK], [gtk+-3.0])        # Проверка GTK3

# Генерация Makefile'ов
AC_CONFIG_FILES([Makefile src/Makefile])     # Какие файлы создавать
AC_OUTPUT                                    # Финализация
```

2. Makefile.am (основные правила сборки)
   Назначение: Описание структуры проекта
```
SUBDIRS = src lib tests  # Поддиректории для сборки

# Глобальные флаги
AM_CFLAGS = -Wall -Wextra
AM_CXXFLAGS = -std=c++11

# Дополнительные файлы
dist_doc_DATA = README.md   # Устанавливаемые документы
EXTRA_DIST = config.h.in     # Файлы для дистрибуции
```

3. src/Makefile.am (сборка основной программы)
   Назначение: Компиляция исходников
```
bin_PROGRAMS = myapp        # Имя исполняемого файла

# Исходные файлы
myapp_SOURCES = main.c utils.c
myapp_HEADERS = utils.h

# Зависимости
myapp_LDADD = $(GTK_LIBS)   # Связывание с GTK
myapp_CPPFLAGS = $(GTK_CFLAGS) # Флаги компиляции
```

4. lib/Makefile.am (для библиотек)
   Назначение: Сборка статических/динамических библиотек
```
lib_LTLIBRARIES = libmylib.la        # Имя библиотеки
libmylib_la_SOURCES = lib.c lib.h    # Исходники
libmylib_la_LDFLAGS = -version-info 1:0:0  # Версия библиотеки
```

5. tests/Makefile.am (для тестов)
   Назначение: Сборка и запуск тестов
```
check_PROGRAMS = test_utils          # Тестовая программа
test_utils_SOURCES = test_utils.c    # Исходники теста
test_utils_LDADD = ../src/libmylib.la # Линковка с основной либой

TESTS = $(check_PROGRAMS)            # Что запускать при `make check`
```

6. config.h.in (шаблон конфига)
   Назначение: Настройки для разных платформ
```
#undef HAVE_GTK         // Будет определено configure
#undef VERSION          // Автоматически подставится
```

7. Генерация сборки (последовательность команд)
   autoreconf -ivf
   Генерирует скрипт configure на основе configure.ac

./configure
Проверяет систему и создает:

Makefile из Makefile.am

config.h из config.h.in

make
Запускает сборку по правилам из Makefile.am



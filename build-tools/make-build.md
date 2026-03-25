Формат правил:
```
цель: зависимости
    команда1
    команда2
----------------------
hello: hello.c
    gcc hello.c -o hello
```

Переменные

```
CC = gcc
CFLAGS = -Wall -O2
TARGET = program

$(TARGET): main.c
    $(CC) $(CFLAGS) main.c -o $(TARGET)
```

Автоматические переменные

```
%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@
```
- `$@` — имя цели
- `$<` — первая зависимость
- `$^` — все зависимости

Пример

```
# Настройки
JAVAC = javac
JAVA = java
SRC_DIR = src/main/java
BUILD_DIR = build
LIB_DIR = lib

# Списки файлов
JAVA_SRC = $(wildcard $(SRC_DIR)/**/*.java)
JAR_DEPS = $(LIB_DIR)/slf4j-api.jar $(LIB_DIR)/junit.jar

# Основные цели
all: myapp.jar

myapp.jar: $(JAVA_SRC) $(JAR_DEPS)
    $(JAVAC) -d $(BUILD_DIR) -cp "$(LIB_DIR)/*" $(JAVA_SRC)
    jar cvf $@ -C $(BUILD_DIR) .

# Загрузка зависимостей
$(LIB_DIR)/%.jar:
    mkdir -p $(LIB_DIR)
    wget -P $(LIB_DIR) "https://repo1.maven.org/maven2/$(subst .,/,$*)/$@"

# Очистка
clean:
    rm -rf $(BUILD_DIR) *.jar

# Запуск
run: myapp.jar
    $(JAVA) -jar $<
```



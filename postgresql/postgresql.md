# POSTGRESQL

## Меню

- [Коннект](#коннект)
- [Команды](#команды)

### Коннект

`psql -d <database_name> -h <hostname> -p <port_number> -U <username>`

Указывать -d <database_name> обязательно иначе будет пытаться подключиться к базе данных с именем пользователя

### Команды

| Команда              | Описание              |
|----------------------|-----------------------|
| \l \list \l+		       | all db                |
| \c <database_name> 	 | connect to db         |
| \dt \dt+ 			         | all tables in db      |
| \dv \dv+ 			         | all views in db       |
| \dn \dn+ 			         | all schema in db      |
| \df \df+ 			         | all functions in db   |
| \di \di+ 			         | all indexes in db     |
| \du				              | all users             |
| \q 				              | quit                  |
| \! <command> 		      | shell command         |
| \? 				              | help command          |
| \h <command> 		      | help command          |
| \o <filename> 		     | last response in file |
| \i <filename> 		     | script from file      |
| \echo <string> 		    | echo in console       |

Пример pg_hba.conf

# TYPE  DATABASE        USER            ADDRESS                 METHOD

# Локальные соединения (Unix-сокет)

local all all peer

# Локальные соединения с паролем

# Используйте, если хотите аутентифицироваться по паролю на локальном хосте

# local   all             all                                     md5

# Подключения по TCP/IP для всех баз и всех пользователей с использованием пароля

# (замените md5 на scram-sha-256 для более безопасного шифрования, если поддерживается)

host all all 127.0.0.1/32 md5
host all all             ::1/128 md5

# Подключения для определенного пользователя с паролем (например, "app_user")

host all app_user 192.168.1.0/24 md5

# Подключения без пароля (только в тестовой среде или в случае полного доверия)

# host    all             all             0.0.0.0/0               trust

# Подключения с использованием сертификатов (если настроено)

# hostssl all             all             0.0.0.0/0               cert
----------------------------
Рекомендации:
Для разработки:

local all all trust
host all all 127.0.0.1/32 trust
host all all             ::1/128 trust
Для production:

local all all md5
host all all 127.0.0.1/32 scram-sha-256
host all all             ::1/128 scram-sha-256
Для внешнего доступа:

host all all 0.0.0.0/0 md5
Настройте брандмауэр, чтобы ограничить доступ только для доверенных IP-адресов.

pg_hba.conf:
# Метод md5 требует указания пароля
local all postgres md5

	# Метод peer требует совпадения пользователя ОС и PostgreSQL
	local   all   postgres   peer

psql -U postgres -c "show hba_file;"

------------------------------------------------------
WARNING: Console code page (866) differs from Windows code page (1251)
8-bit characters might not work correctly. See psql reference
page "Notes for Windows users" for details.
ChatGPT сказал:
ChatGPT
Этот WARNING возникает из-за несовпадения кодировок консоли Windows и используемой кодировки PostgreSQL (или приложения
psql). Кодировка консоли (866) отличается от стандартной кодировки Windows (1251), что может вызывать проблемы с
отображением символов.

Решение проблемы:

1. Смените кодировку консоли на 1251:
   Введите следующую команду перед запуском psql:

cmd
Копировать код
chcp 1251
Эта команда установит кодовую страницу Windows-1251 в текущей консоли. После выполнения команды вы можете запустить
psql:

cmd
Копировать код
psql -U postgres
Примечание: Если вы хотите вернуть кодовую страницу обратно, используйте:

cmd
Копировать код
chcp 866

2. Убедитесь, что PostgreSQL использует правильную кодировку:
   Проверьте кодировку вашей базы данных. В psql выполните команду:

sql
Копировать код
\l
Посмотрите значение в колонке Encoding. Если база данных использует, например, UTF8, но вы работаете с Windows-1251, это
может вызывать дополнительные проблемы.

3. Добавьте настройки в конфигурацию .psqlrc:
   Если вы часто работаете с psql, можно настроить отображение клиентской кодировки:

Создайте (или отредактируйте) файл .psqlrc. В Windows он обычно находится по пути:
shell
Копировать код
%USERPROFILE%\.psqlrc
Добавьте строку:
plaintext
Копировать код
\encoding WIN1251

4. Используйте терминал с поддержкой UTF-8:
   Если вы хотите избежать проблем с кодировками вообще, используйте терминалы, поддерживающие UTF-8, например:

Windows Terminal (можно скачать из Microsoft Store).
WSL (Windows Subsystem for Linux).

5. Игнорируйте предупреждение:
   Если проблема не вызывает реальных ошибок в работе, вы можете игнорировать предупреждение. Оно информирует о
   возможных проблемах с отображением 8-битных символов, но не всегда они действительно возникают.

Попробуйте первый вариант, чтобы избавиться от предупреждения. Если вы работаете с текстами на кириллице в базе,
дополнительно уточните кодировку базы и консоли, чтобы избежать проблем с данными.


















-----------------------------------
maven spring build
[ERROR] COMPILATION ERROR : cannot find symbol

Причины:
IDE обрабатывает аннотации Lombok автоматически: IntelliJ IDEA может корректно обрабатывать аннотации Lombok благодаря
встроенной поддержке плагина, даже если Maven не настроен для обработки аннотаций.

Отсутствует обработка аннотаций в Maven: Maven компилирует проект через плагин maven-compiler-plugin, который может не
знать о необходимости обработки аннотаций Lombok.

Разная версия Java: IntelliJ может использовать одну версию JDK, а Maven — другую, что приводит к различиям в
компиляции.

File > Settings > Build, Execution, Deployment > Compiler > Annotation Processors -> "Enable annotation processing."
(но обычно intelij idea сама предлагает это включить)

добавить в pom.xml
<dependency>
<groupId>org.projectlombok</groupId>
<artifactId>lombok</artifactId>
<version>1.18.30</version>
<scope>provided</scope>
</dependency>

проверить @Data @Getter @Setter

Настройте Maven для обработки аннотаций Lombok
<build>
<plugins>
<plugin>
<groupId>org.apache.maven.plugins</groupId>
<artifactId>maven-compiler-plugin</artifactId>
<version>3.11.0</version>
<configuration>
<source>17</source> <!-- Версия Java -->
<target>17</target>
<annotationProcessorPaths>
<path>
<groupId>org.projectlombok</groupId>
<artifactId>lombok</artifactId>
<version>1.18.30</version>
</path>
</annotationProcessorPaths>
</configuration>
</plugin>
</plugins>
</build>

WARNING: /C:/Users/rmtj7/Desktop/web4back-spring/src/main/java/dev/ramil21/lab4back/model/RefreshToken.java:[36,21]
@Builder will ignore the initializing expression entirely. If you want the initializing expression to serve as default,
add @Builder.Default. If it is not supposed to be settable during building, make the field final.

Этот warning возникает, потому что Lombok аннотация @Builder игнорирует значения, инициализированные при объявлении
полей. Если вы хотите, чтобы такие значения использовались как значения по умолчанию при создании объекта через
@Builder, необходимо явно указать @Builder.Default.
Добавьте @Builder.Default:
Исправьте ваши классы, добавив аннотацию @Builder.Default для полей, которые имеют начальное значение. Например:

@Builder
public class RefreshToken {
@Builder.Default
private boolean active = true; // Значение по умолчанию будет учитываться при использовании Builder
}

Сделайте поле final, если оно не должно изменяться
@Builder
public class RefreshToken {
private final String token; // Обязательно задается при создании через Builder
private final Instant createdDate;
}

Почему это важно?
Без @Builder.Default, значения по умолчанию, заданные при объявлении полей, игнорируются. Это может привести к
неожиданным ошибкам в логике, если вы полагаетесь на эти значения.

зачем в postgresql нужны стандартные базы данных:
postgres
template0
template1

В PostgreSQL при установке создаются несколько стандартных баз данных: postgres, template0 и template1. Каждая из них
имеет своё назначение и играет важную роль в работе системы. Вот их описание:

1. База данных postgres
   Назначение:

Это универсальная база данных по умолчанию, которая предоставляется для выполнения административных задач.
Она используется, чтобы администратор мог подключаться к серверу PostgreSQL, если ни одна другая база данных не
существует или не настроена.
Когда используется:

Для администрирования (например, выполнения SQL-запросов на уровне сервера).
Для временного использования, если не требуется создавать собственную базу данных.
Можно ли удалить?

Удаление возможно, но не рекомендуется, так как это усложнит доступ к серверу без предварительно созданных баз данных.

2. База данных template1
   Назначение:

Это шаблонная база данных, которая используется для создания новых баз данных.
Когда вы создаёте новую базу данных, её структура и данные копируются из template1.
Особенности:

Вы можете изменять эту базу данных, например, добавлять свои функции, расширения или схемы. Эти изменения будут
автоматически применяться ко всем новым базам данных, созданным на основе template1.
Можно ли удалить или изменять?

Удалить её нельзя, но изменять можно. Если в ней произошли нежелательные изменения, можно восстановить её состояние,
скопировав данные из template0.

3. База данных template0
   Назначение:

Это базовый шаблон базы данных, который остаётся неизменным и используется как "чистый" источник для восстановления или
создания баз данных.
Она служит эталонной базой без пользовательских изменений.
Особенности:

База данных template0 всегда остаётся в своём первоначальном состоянии и недоступна для записи.
Используется, если необходимо создать новую базу данных, которая будет полностью "чистой", без изменений, внесённых в
template1.
Можно ли изменять?

Нет, база данных template0 защищена от изменений и недоступна для записи.
Как это работает на практике:
Создание новой базы данных:

По умолчанию PostgreSQL использует template1 как шаблон:
sql
Копировать код
CREATE DATABASE newdb;
Это эквивалентно:
sql
Копировать код
CREATE DATABASE newdb TEMPLATE template1;
Создание "чистой" базы данных:

Если требуется создать базу данных без пользовательских изменений, используют template0:
sql
Копировать код
CREATE DATABASE newdb TEMPLATE template0;
Резюме:
postgres — служебная база для административного доступа.
template1 — пользовательский шаблон для создания баз данных.
template0 — неизменяемый шаблон для создания "чистых" баз данных.
Эти базы данных важны для работы PostgreSQL и обеспечивают гибкость в управлении базами. Если у вас есть дополнительные
вопросы, как их использовать, напишите!



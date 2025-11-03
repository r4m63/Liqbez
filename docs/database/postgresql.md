# POSTGRESQL

## Меню

- [Коннект](#коннект)
- [Команды](#команды)

## Коннект

`psql -d <database_name> -h <hostname> -p <port_number> -U <username>`

Указывать -d <database_name> иначе будет пытаться подключиться к базе данных с именем пользователя

## Команды

| Команда                | Описание                  |
|------------------------|---------------------------|
| `\l` `\list` `\l+`		   | all databases             |
| `\c <database_name>` 	 | connect to database       |
| `\dt` `\dt+` 			       | all tables in database    |
| `\dv` `\dv+` 			       | all views in database     |
| `\dn` `\dn+` 			       | all schema in database    |
| `\df` `\df+` 			       | all functions in database |
| `\di` `\di+` 			       | all indexes in database   |
| `\du`				              | all users                 |
| `\q` 				              | quit                      |
| `\! <command>` 		      | shell command             |
| `\?` 				              | help command              |
| `\h <command>` 		      | help command              |
| `\o <filename>` 		     | last response in file     |
| `\i <filename>` 		     | execute script from file  |
| `\echo <string>` 		    | echo in console           |

## SQL

Реальный порядок выполнения:

1. FROM (+ JOIN)
2. WHERE
3. GROUP BY
4. HAVING
5. SELECT
6. ORDER BY
7. LIMIT / OFFSET

**DQL (выборка)**

| Команда               | Описание                                                                                                                                                                                                                                                                            |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `SELECT ... FROM ...` | SELECT: имена колонок (`SELECT id`), выражения (`SELECT price * count AS total`), агрегаты (`SELECT count(*)`) <br/> FROM: `SELECT u.id, o.id FROM users u`                                                                                                                         |
| `WHERE`               | Фильтрует строки до группировки: `WHERE status = 'open' AND total > 100;`, `WHERE email LIKE '%@gmail.com';`                                                                                                                                                                        |
| `GROUP BY`            | Объединяет строки в группы по указанным полям <br/> таблица: orders(id, user_id, total)<br/>`SELECT user_id, SUM(total) AS total_sum FROM orders GROUP BY user_id;` - берет все строки из orders, делит их на группы по user_id <br/> Результат: (user_id, total_sum)               |
| `HAVING`              | Фильтр для групп после GROUP BY <br/> таблица: orders(id, user_id, total)<br/>`SELECT user_id, SUM(total) AS total_sum FROM orders GROUP BY user_id HAVING SUM(total) > 300;` - берем все строки, собираем группы по user_id, считаем для каждой группы total_sum, фильтруем группы |
| `ORDER BY`            | Сортирует строки результата (ASC(возр)/DESC(убыв))                                                                                                                                                                                                                                  |
| `LIMIT / OFFSET`      | Постраничная выборка, LIMIT n - максимум n строк в результате, OFFSET m - пропустить m строк                                                                                                                                                                                        |

**JOIN**

| Команда               | Описание                                                                                                                                                                            |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `JOIN` = `INNER JOIN` | Берет только те пары строк, где условие ON выполняется (пересечение множеств) `... FROM users u JOIN orders o ON o.user_id = u.id;`                                                 |
| `LEFT JOIN`           | Берет все строки из левой таблицы. к ним прицепляет строки из правой по условию ON. Если совпадения нет - в колонках правой таблицы будут NULL.                                     |
| `RIGHT JOIN`          | Берет все строки из правой таблицы (orders) и подцепляет к ним левую. Если совпадения нет - в колонках левой будут NULL.                                                            |
| `FULL JOIN`           | Объединяет результат LEFT и RIGHT:берет все строки из обеих таблиц, где есть совпадение — склеивает, где нет — заполняет NULL.                                                      |
| `CROSS JOIN`          | Каждая строка левой таблицы соединяется с каждой строкой правой. Никакого условия соединения (`ON`) нет. `SELECT c.name AS color, s.name AS size FROM colors c CROSS JOIN sizes s;` |
| `SELF JOIN`           | Не отдельный тип, а приём: таблица джоинится сама с собой. `SELECT e.name, m.name AS manager_name FROM employees e LEFT JOIN employees m ON m.id = e.manager_id;`                   |

**DML (изменение данных)**

```sql
INSERT INTO users (name, email)
VALUES ('A', 'a@a.com'),
       ('B', 'b@b.com');

-- Вставка данных, полученных из запроса:
INSERT INTO archive_orders (id, user_id, total, created_at)
SELECT id, user_id, total, created_at
FROM orders
WHERE created_at < '2025-01-01';

UPDATE users
SET name  = 'Ivan Petrov',
    email = 'ivan.petrov@example.com'
WHERE id = 1;

DELETE
FROM users
WHERE id = 1;

DELETE
FROM users -- без where удалятся все данные

-- или быстрее:
         TRUNCATE TABLE users;

-- RETURNING - вернуть данные измененных строк

INSERT INTO orders (user_id, total)
VALUES (10, 123.45) RETURNING id;

DELETE
FROM users
WHERE id = 1 RETURNING *;

UPDATE users
SET name = 'Ivan Petrov'
WHERE id = 1 RETURNING *;

INSERT INTO users (name, email)
VALUES ('A', 'a@a.com') RETURNING id, name;
```

**DDL (структура)**

```sql
CREATE TABLE users
(
    id         bigserial PRIMARY KEY,
    email      text        NOT NULL UNIQUE,
    name       text,
    created_at timestamptz NOT NULL DEFAULT now()
);

CREATE VIEW active_users AS
SELECT id, email
FROM users
WHERE deleted_at IS NULL;

CREATE INDEX idx_users_email ON users (email);
CREATE UNIQUE INDEX idx_users_email_unique ON users (email);

CREATE SCHEMA logistics;
CREATE TABLE logistics.vehicles
(
);

CREATE SEQUENCE order_id_seq START 1 INCREMENT 1;
SELECT nextval('order_id_seq');

CREATE TYPE user_role AS ENUM ('admin', 'user', 'manager');


DROP TABLE users;
DROP VIEW active_users;
DROP INDEX idx_users_email;
DROP SCHEMA logistics;
DROP SEQUENCE order_id_seq;
DROP TYPE user_role;

DROP TABLE IF EXISTS users;

DROP TABLE users CASCADE; -- удалить объект и всё, что от него зависит (снесёт связанные объекты)

TRUNCATE TABLE logs; -- Удаляет все строки из таблицы, быстрее, чем DELETE FROM

TRUNCATE TABLE orders, order_items RESTART IDENTITY CASCADE; -- Несколько таблиц + каскад
```

```sql
ALTER TABLE users
    ADD COLUMN phone text;

ALTER TABLE users
DROP
COLUMN phone;

ALTER TABLE users
    RENAME COLUMN name TO full_name;

ALTER TABLE users
ALTER
COLUMN email TYPE varchar(255);

ALTER TABLE users
    ALTER COLUMN created_at SET DEFAULT now();
ALTER TABLE users
    ALTER COLUMN created_at DROP DEFAULT;

ALTER TABLE users
    ADD CONSTRAINT users_email_unique UNIQUE (email);
ALTER TABLE users
DROP
CONSTRAINT users_email_unique;
```

**TCL (транзакции)**

**Транзакция** — набор запросов, который либо выполняется весь, либо откатывается весь.
По умолчанию PostgreSQL работает в режиме **autocommit**: каждый отдельный `INSERT/UPDATE/...` = отдельная транзакция.

Начать явную транзакцию.

```sql
BEGIN;              -- или START TRANSACTION;

UPDATE accounts
SET balance = balance - 100
WHERE id = 1;

UPDATE accounts
SET balance = balance + 100
WHERE id = 2;
-- пока COMMIT/ROLLBACK не было, снаружи эти изменения не видны

COMMIT;
-- После COMMIT откатить уже нельзя.
-- ИЛИ вместо COMMIT;
ROLLBACK; -- всё, что было после BEGIN, отменяется
```

Savepoint

```sql
BEGIN;

UPDATE accounts
SET balance = balance - 100
WHERE id = 1;

SAVEPOINT sp1;

UPDATE accounts
SET balance = balance + 100
WHERE id = 2;

-- ошибка или передумали
ROLLBACK TO SAVEPOINT sp1;
-- RELEASE SAVEPOINT sp1; -- забыть сейвпоинт (необязательно)

-- здесь откатились только изменения после sp1
-- списание со счёта id=1 осталось, перевод на id=2 отменён

COMMIT;
```

**DCL (права)**

```sql
-- CONNECT - можно подключаться к БД
-- CREATE - создавать объекты (схемы)
-- TEMP - создавать временные таблицы

GRANT
CONNECT
ON DATABASE db_name TO user_name;

REVOKE CONNECT ON DATABASE db_name FROM user_name;
```

```sql
-- Для схем
-- USAGE  — видеть объекты внутри схемы
-- CREATE — создавать таблицы/типы/функции в схеме

GRANT
USAGE
ON
SCHEMA
shema_name TO user_name;
GRANT CREATE
ON SCHEMA shema_name TO user_name;

REVOKE CREATE ON SCHEMA shema_name FROM user_name;
```

```sql
-- Для таблиц

-- SELECT - читать строки
-- INSERT - вставлять строки
-- UPDATE - изменять строки
-- DELETE - удалять строки
-- TRUNCATE - делать TRUNCATE
-- REFERENCES - использовать таблицу как цель FK

GRANT SELECT, INSERT, UPDATE, DELETE
    ON TABLE table_name
    TO user_name;

REVOKE UPDATE, DELETE
    ON TABLE table_name
    FROM user_name;

-- Сразу для всех таблиц в схеме
GRANT
SELECT
ON ALL TABLES IN SCHEMA shema_name TO app_readonly;
```

```sql
-- Роли и членство в ролях
-- Роль = пользователь или группа пользователей

-- Создать роль/пользователя
CREATE ROLE user_name LOGIN PASSWORD 'secret';
CREATE ROLE group_name;

-- Сделать роль “группой” и добавить в неё пользователя
GRANT group_name TO user_name; -- app_user теперь член app_readonly
REVOKE group_name FROM user_name;


-- дать чтение всем
GRANT SELECT ON TABLE users TO PUBLIC;

-- забрать у всех
REVOKE ALL PRIVILEGES ON TABLE users FROM PUBLIC;
```

## Ограничения (constraints)

Можно задавать **на колонку** и **на таблицу**.

- `NOT NULL`
- `UNIQUE`
- `PRIMARY KEY` = `UNIQUE` + `NOT NULL`
- `FOREIGN KEY`
- `CHECK (условие)`
- `EXCLUDE` - сложные ограничения (для диапазонов, гео и т.п.)
- `DEFAULT`
- `DEFERRABLE [INITIALLY DEFERRED|IMMEDIATE]` - отложенная проверка FK/UNIQUE/EXCLUDE

## Типы данных

serial / identity

- serial - сахар вокруг integer + sequence + DEFAULT nextval(...).
- smallserial -> smallint
- serial -> integer
- bigserial -> bigint

Альтернативно:

```
CREATE SEQUENCE items_id_seq;
CREATE TABLE items (
  id integer NOT NULL DEFAULT nextval('items_id_seq'),
  PRIMARY KEY (id)
);
ALTER SEQUENCE items_id_seq OWNED BY items.id;
```

Современный способ - identity:

```
id bigint GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY
-- или
id bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY
```

---

Числовые

- `smallint` (2 байта)
- `integer` / `int` (4 байта)
- `bigint` (8 байт)
- `numeric(p,s)` / `decimal` - точная фикс. точка
- `real`, `double precision` - с плавающей точкой

Cтроки

- `text` - произвольная длина
- `varchar(n)` - до n символов
- `char(n)` - фиксированная длина, дополняется пробелами

Логика

- `boolean`

Дата/время

- `date`
- `time` [without time zone]
- `time with time zone` (timetz)
- `timestamp` [without time zone]
- `timestamp with time zone` (timestamptz) - чаще всего нужен именно он
- `interval` - промежутки времени

Прочее

- `uuid`
- `bytea` - бинарь
- `json`, `jsonb` - JSON (jsonb - структурированный/индексируемый)
- `ARRAY` - массивы: integer[], text[]
- `enum` - перечисления через CREATE TYPE ... AS ENUM

## FOREIGN KEY / REFERENCES

```sql
-- inline
user_id
bigint REFERENCES users(id)
  ON DELETE
CASCADE
  ON
UPDATE CASCADE;

-- table-level
CONSTRAINT fk_orders_user
  FOREIGN KEY (user_id)
  REFERENCES users(id)
  ON DELETE
CASCADE
  ON
UPDATE RESTRICT;
```

Режимы:

- `RESTRICT` - аналогично NO ACTION
- `CASCADE` - удалить/обновить дочерние строки
- `SET NULL` - установить NULL в FK
- `SET DEFAULT` - установить значение по умолчанию

## Индексы

Основное:

```
CREATE INDEX idx_users_email ON users (email);
CREATE UNIQUE INDEX idx_users_email_unique ON users (email);
DROP INDEX idx_users_email;
```

Виды:

- по умолчанию B-TREE - равенства/диапазоны, сортировка
- GIN - для jsonb, полнотекстового поиска, массивов
- GiST, SP-GiST, BRIN, HASH - специальные кейсы

Фишки:

частичный индекс:

```
CREATE INDEX idx_orders_open
ON orders (user_id)
WHERE status = 'open';
```

индекс по выражению:

```
CREATE INDEX idx_users_lower_email
ON users ((lower(email)));
```

CONCURRENTLY - без долгой блокировки записи:

```
CREATE INDEX CONCURRENTLY idx_users_phone ON users (phone);
```

## Триггеры и функции

Сначала функция, затем триггер

```
CREATE OR REPLACE FUNCTION set_updated_at()
RETURNS trigger AS $$
BEGIN
  NEW.updated_at := now();
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_set_updated_at
BEFORE UPDATE ON users
FOR EACH ROW
EXECUTE FUNCTION set_updated_at();
```

```
DROP TRIGGER trg_set_updated_at ON users;
DROP FUNCTION set_updated_at();
```

Ключевые моменты:

- BEFORE / AFTER
- INSERT, UPDATE, DELETE, TRUNCATE
- FOR EACH ROW - для каждой строки
- FOR EACH STATEMENT - один раз на запрос
- В триггер-функции доступны NEW и OLD

## Схемы

Схема = namespace внутри БД.
По умолчанию public.
Поиск по search_path.

```
CREATE SCHEMA logistics;
CREATE TABLE logistics.vehicles (...);

SET search_path TO logistics, public;

SELECT * FROM logistics.vehicles;
```

## Секвенсы (sequences)

Генератор чисел.

```
CREATE SEQUENCE order_id_seq
  START WITH 1
  INCREMENT BY 1
  NO MINVALUE
  NO MAXVALUE
  CACHE 1;

SELECT nextval('order_id_seq'); -- получить следующее значение
SELECT currval('order_id_seq'); -- последнее значение в текущей сессии

ALTER SEQUENCE order_id_seq RESTART WITH 1000;

-- связать с колонкой
ALTER SEQUENCE order_id_seq OWNED BY orders.id;
```

---


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

Разная версия Java: IntelliJ может использовать одну версию JDK, а Maven - другую, что приводит к различиям в
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
имеет свое назначение и играет важную роль в работе системы. Вот их описание:

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
Когда вы создаете новую базу данных, ее структура и данные копируются из template1.
Особенности:

Вы можете изменять эту базу данных, например, добавлять свои функции, расширения или схемы. Эти изменения будут
автоматически применяться ко всем новым базам данных, созданным на основе template1.
Можно ли удалить или изменять?

Удалить ее нельзя, но изменять можно. Если в ней произошли нежелательные изменения, можно восстановить ее состояние,
скопировав данные из template0.

3. База данных template0
   Назначение:

Это базовый шаблон базы данных, который остается неизменным и используется как "чистый" источник для восстановления или
создания баз данных.
Она служит эталонной базой без пользовательских изменений.
Особенности:

База данных template0 всегда остается в своем первоначальном состоянии и недоступна для записи.
Используется, если необходимо создать новую базу данных, которая будет полностью "чистой", без изменений, внесенных в
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
postgres - служебная база для административного доступа.
template1 - пользовательский шаблон для создания баз данных.
template0 - неизменяемый шаблон для создания "чистых" баз данных.
Эти базы данных важны для работы PostgreSQL и обеспечивают гибкость в управлении базами. Если у вас есть дополнительные
вопросы, как их использовать, напишите!



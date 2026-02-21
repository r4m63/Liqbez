# POSTGRESQL

<!-- toc -->

- [Коннект](#%D0%BA%D0%BE%D0%BD%D0%BD%D0%B5%D0%BA%D1%82)
- [Команды](#%D0%BA%D0%BE%D0%BC%D0%B0%D0%BD%D0%B4%D1%8B)
- [SQL](#sql)
    * [SQL DQL (Выборка данных)](#sql-dql-%D0%B2%D1%8B%D0%B1%D0%BE%D1%80%D0%BA%D0%B0-%D0%B4%D0%B0%D0%BD%D0%BD%D1%8B%D1%85)
    * [SQL DML (Изменение данных)](#sql-dml-%D0%B8%D0%B7%D0%BC%D0%B5%D0%BD%D0%B5%D0%BD%D0%B8%D0%B5-%D0%B4%D0%B0%D0%BD%D0%BD%D1%8B%D1%85)
    * [SQL DDL (Определение структуры)](#sql-ddl-%D0%BE%D0%BF%D1%80%D0%B5%D0%B4%D0%B5%D0%BB%D0%B5%D0%BD%D0%B8%D0%B5-%D1%81%D1%82%D1%80%D1%83%D0%BA%D1%82%D1%83%D1%80%D1%8B)
    * [SQL TCL (Управление транзакциями)](#sql-tcl-%D1%83%D0%BF%D1%80%D0%B0%D0%B2%D0%BB%D0%B5%D0%BD%D0%B8%D0%B5-%D1%82%D1%80%D0%B0%D0%BD%D0%B7%D0%B0%D0%BA%D1%86%D0%B8%D1%8F%D0%BC%D0%B8)
    * [SQL DCL (Управление правами)](#sql-dcl-%D1%83%D0%BF%D1%80%D0%B0%D0%B2%D0%BB%D0%B5%D0%BD%D0%B8%D0%B5-%D0%BF%D1%80%D0%B0%D0%B2%D0%B0%D0%BC%D0%B8)
    * [Ограничения (constraints)](#%D0%BE%D0%B3%D1%80%D0%B0%D0%BD%D0%B8%D1%87%D0%B5%D0%BD%D0%B8%D1%8F-constraints)
- [Типы данных](#%D1%82%D0%B8%D0%BF%D1%8B-%D0%B4%D0%B0%D0%BD%D0%BD%D1%8B%D1%85)
    * [FOREIGN KEY / REFERENCES](#foreign-key--references)
- [Индексы](#%D0%B8%D0%BD%D0%B4%D0%B5%D0%BA%D1%81%D1%8B)
- [Триггеры и функции](#%D1%82%D1%80%D0%B8%D0%B3%D0%B3%D0%B5%D1%80%D1%8B-%D0%B8-%D1%84%D1%83%D0%BD%D0%BA%D1%86%D0%B8%D0%B8)
- [Схемы](#%D1%81%D1%85%D0%B5%D0%BC%D1%8B)
- [Сиквенсы (sequences)](#%D1%81%D0%B8%D0%BA%D0%B2%D0%B5%D0%BD%D1%81%D1%8B-sequences)
- [Время в Postgresql](#%D0%B2%D1%80%D0%B5%D0%BC%D1%8F-%D0%B2-postgresql)
- [XID транзакции](#xid-%D1%82%D1%80%D0%B0%D0%BD%D0%B7%D0%B0%D0%BA%D1%86%D0%B8%D0%B8)
- [MVCC](#mvcc)
    + [INSERT реализация](#insert-%D1%80%D0%B5%D0%B0%D0%BB%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D1%8F)
    + [UPDATE реализация](#update-%D1%80%D0%B5%D0%B0%D0%BB%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D1%8F)
    + [DELETE реализация](#delete-%D1%80%D0%B5%D0%B0%D0%BB%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D1%8F)
- [ACID](#acid)
- [Конфликты](#%D0%BA%D0%BE%D0%BD%D1%84%D0%BB%D0%B8%D0%BA%D1%82%D1%8B)
- [Уровни изоляции](#%D1%83%D1%80%D0%BE%D0%B2%D0%BD%D0%B8-%D0%B8%D0%B7%D0%BE%D0%BB%D1%8F%D1%86%D0%B8%D0%B8)
- [Serializable Snapshot Isolation (SSI)](#serializable-snapshot-isolation-ssi)
- [VACUUM](#vacuum)

<!-- tocstop -->

# Коннект

`psql -d <database_name> -h <hostname> -p <port_number> -U <username>`

Указывать -d <database_name> иначе будет пытаться подключиться к базе данных с именем пользователя

# Команды

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

# SQL

Реальный порядок выполнения sql:

1. FROM (+ JOIN)
2. WHERE
3. GROUP BY
4. HAVING
5. SELECT
6. ORDER BY
7. LIMIT / OFFSET

## SQL DQL (Выборка данных)

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
| `FULL JOIN`           | Объединяет результат LEFT и RIGHT:берет все строки из обеих таблиц, где есть совпадение - склеивает, где нет - заполняет NULL.                                                      |
| `CROSS JOIN`          | Каждая строка левой таблицы соединяется с каждой строкой правой. Никакого условия соединения (`ON`) нет. `SELECT c.name AS color, s.name AS size FROM colors c CROSS JOIN sizes s;` |
| `SELF JOIN`           | Не отдельный тип, а приём: таблица джоинится сама с собой. `SELECT e.name, m.name AS manager_name FROM employees e LEFT JOIN employees m ON m.id = e.manager_id;`                   |

## SQL DML (Изменение данных)

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
VALUES (10, 123.45)
RETURNING id;

DELETE
FROM users
WHERE id = 1
RETURNING *;

UPDATE users
SET name = 'Ivan Petrov'
WHERE id = 1
RETURNING *;

INSERT INTO users (name, email)
VALUES ('A', 'a@a.com')
RETURNING id, name;
```

## SQL DDL (Определение структуры)

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
    DROP CONSTRAINT users_email_unique;
```

## SQL TCL (Управление транзакциями)

**Транзакция** - набор запросов, который либо выполняется весь, либо откатывается весь.
По умолчанию PostgreSQL работает в режиме **autocommit**: каждый отдельный `INSERT/UPDATE/...` = отдельная транзакция.

- Неявные (implicit) транзакции - СУБД сама оборачивает каждую команду в транзакцию.
- Явные (explicit) транзакции - самому говорить, где начало и конец транзакции (BEGIN/COMMIT) - для объединения
  нескольких команд в одну транзакцию

**Начать явную транзакцию**

```sql
BEGIN; -- или START TRANSACTION;

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

**Savepoint**

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

## SQL DCL (Управление правами)

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
-- USAGE  - видеть объекты внутри схемы
-- CREATE - создавать таблицы/типы/функции в схеме

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

# Типы данных

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
  ON
DELETE
    CASCADE
ON
UPDATE CASCADE;

-- table-level
CONSTRAINT fk_orders_user
  FOREIGN KEY (user_id)
  REFERENCES users(id)
  ON
DELETE
    CASCADE
ON
UPDATE RESTRICT;
```

Режимы:

- `RESTRICT` - аналогично NO ACTION
- `CASCADE` - удалить/обновить дочерние строки
- `SET NULL` - установить NULL в FK
- `SET DEFAULT` - установить значение по умолчанию

# Индексы

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

# Триггеры и функции

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

# Схемы

Схема = namespace внутри БД.
По умолчанию public.
Поиск по search_path.

```
CREATE SCHEMA logistics;
CREATE TABLE logistics.vehicles (...);

SET search_path TO logistics, public;

SELECT * FROM logistics.vehicles;
```

# Сиквенсы (sequences)

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

# Время в Postgresql

```sql
SELECT now(); -- timestamp with time zone, время начала транзакции
SELECT current_timestamp; -- то же самое, синоним now()
SELECT current_time; -- только время (time with time zone)
SELECT current_date;
-- только дата

-- Если нужно именно прямо сейчас, а не время начала транзакции
SELECT clock_timestamp(); -- реальное текущее время в момент вызова
```

# XID транзакции

Transaction ID (XID)

- Каждой модифицирующей транзакции (DML) Postgres выдаёт номер - XID.
- XID используется, чтобы понять:
    - кто создал строку,
    - кто удалил/обновил строку,
    - какие версии строк видимы в конкретной транзакции.

Каждая строка (tuple) в таблице имеет служебные поля (в заголовке), например:

- xmin - XID транзакции, которая создала эту версию строки.
- xmax - XID транзакции, которая удалила или заменила эту версию (если не 0).
- ctid - физический адрес версии ((page, offset)), при UPDATE у старой версии здесь ссылка на новую.

- Каждой транзакции, которая изменяет данные, Postgres выдаёт 32-битный номер - XID.
- Это число монотонно растёт (1, 2, 3, 4… до 2³²-1, потом wraparound).
- В каждой строке таблицы в заголовке есть:
    - xmin - XID транзакции, которая создала эту строку;
    - xmax - XID транзакции, которая удалила/изменила эту строку (если есть).

По этим полям + статусу транзакций Postgres решает, жива ли строка для конкретного запроса.

**Скрытые (системные) поля таблицы:**

- `xmin` - хранит xid транзакции, в рамках которой запись (копия записи) была создана
- `xmax` - хранит xid транзакции, в рамках которой запись была удалена
- `cmin` - идентификатор команды, создавшей запись
- `cmax` - идентификатор команды, удалившей запись

**Postgres не перезаписывает строку сразу, а:**

- при INSERT создаёт новую версию строки (xmin = XID вставки);
- при UPDATE:
    - помечает старую как удалённую (xmax = XID апдейта),
    - создаёт новую версию с xmin = XID апдейта;
- при DELETE просто ставит xmax = XID удаления.

Дальше, когда выполняется запрос, у него есть snapshot:

- нижняя граница XID, верхняя граница,
- список активных транзакций в момент снимка.

Postgres на основе этого snapshot и xmin/xmax решает:

- видим ли мы эту версию строки,
- или она ещё не зафиксирована / уже устарела.

То есть XID нужен, для того чтобы понять, Эта версия строки была уже закоммичена на момент моего запроса или нет?

Можем посмотреть:

`SELECT xmin, xmax, * FROM table_name;`

`SELECT pg_current_xact_id();` - Возвращает xid текущей транзакции

**Wraparound и VACUUM FREEZE:**

- XID 32-битный -> он зацикливается (wraparound) примерно после 2 млрд транзакций.
- Чтобы не забывать, какие старые строки давно закоммичены, Postgres:
    - периодически запускает VACUUM / autovacuum,
    - замораживает старые строки (xmin меняется на специальный FrozenXid),
    - тем самым защищается от wraparound-катастрофы.

Если не делать VACUUM, можно довести до ситуации, когда база начнёт паниковать и не даст выполнять новые транзакции,
пока не будет выгребен мусор.

# MVCC

Multi-Version Concurrency Control - многоверсионное управление конкурентным доступом.

**MVCC-идея:** Postgres не переписывает строку на месте, а создаёт новую версию, а старую помечает как устаревшую, но не
сразу удаляет.

Когда транзакция делает первый SELECT, она получает snapshot:

- список XID, которые сейчас ещё выполняются (in-progress),
- границы видимых XID.

По этому снимку и по xmin/xmax Postgres решает: Эта версия строки уже была закоммичена на момент моего снимка или ещё
нет?

> Вместо того чтобы перезаписывать строку и всех блокировать, БД хранит несколько версий строки и даёт разным
> транзакциям видеть «свою» версию данных.

### INSERT реализация

Транзакция T с XID = 100 делает: `INSERT INTO users(id, name) VALUES (1, 'Alice');`

- В нужной странице таблицы создаётся новый tuple(строка).
- В заголовке tuple:
    - xmin = 100 (XID текущей транзакции),
    - xmax = 0 (никто ещё не удалял),
    - ctid указывает на себя (позиция в этой же странице).
- Добавляется запись в индексы (например, по id).

Пока транзакция не закоммитит:

- другие транзакции не видят эту версию (если у них snapshot до коммита),
- сама транзакция, которая вставила, видит её всегда.

После COMMIT:

- статус XID=100 помечается как committed,
- новые транзакции, когда будут проверять видимость, увидят, что xmin=100 -> это уже закоммиченная версия -> строка
  видна.

### UPDATE реализация

UPDATE = DELETE старой версии + INSERT новой

Пусть есть строка: `id = 1, name = 'Alice', xmin = 50, xmax = 0`

Транзакция T с XID=100 делает: `UPDATE users SET name = 'Alice2' WHERE id = 1;`

- Находится старая версия строки (id=1, name='Alice').
- Старая версия не переписывается!
    - В её заголовок ставится: xmax = 100 (XID текущей транзакции).
- Это значит: «эта версия перестала быть актуальной начиная с транзакции 100».
    - Создаётся новый tuple (новая версия):
    - id = 1, name = 'Alice2'
    - xmin = 100
    - xmax = 0
    - ctid старой версии теперь указывает на новую, а у новой - на себя.
- Индексы:
    - В общем случае создаётся новый индексный entry на новую версию.
    - Старая запись в индексе остаётся, но при обходе будет отфильтрована, если версия невидима.
    - Есть оптимизация HOT (Heap-Only Tuple), когда можно обновить без новых записей в индексе, если ключи индекса не
      меняются и есть место в той же странице.

Что видят разные транзакции:

- Транзакции со snapshot до коммита XID=100:
- видят старую версию (xmin=50, xmax=100),
- xmax=100 для них ещё «не committed» -> версия считается живой.
- Транзакции со snapshot после коммита XID=100:
- не видят старую версию, потому что её xmax=100 теперь committed и «меньше/входит в диапазон»,
- видят новую версию (xmin=100).

Так обеспечивается:

- изолированность: каждый запрос видит согласованные данные на момент своего snapshot,
- без блокировки чтения (читаем старые версии).

### DELETE реализация

Транзакция T с XID=150 делает: `DELETE FROM users WHERE id = 1;`

- Находит текущую видимую версию строки.
- Не удаляет физически, а:
    - ставит xmax = 150 в заголовке tuple.
- Индексы не трогает прямо сейчас - запись в индексе всё ещё указывает на этот tuple.

Видимость:

- Транзакции со snapshot до коммита XID=150 всё ещё могут видеть строку (для них xmax=150 - ещё не committed).
- После коммита XID=150:
- новые транзакции при проверке видимости увидят, что xmax=150 - committed и строка «удалена» для них.

Физическое удаление версии (reclaim space) делает VACUUM:

- когда уверен, что ни одна активная транзакция уже не может видеть эту версию,
- он может:
- пометить tuple как «мертвый»,
- освободить место на странице,
- почистить ссылку в индексе (или пометить её как «invalid»).

**Проблемы, которые решает:**

1. Конкурентное чтение/запись без тотальной блокировки

- Проблема без MVCC: Если обновлять данные сразу то:
    - писатель блокирует строку,
    - читатели либо ждут, либо читают «грязные» данные.
- С MVCC:
    - Писатели создают новые версии,
    - длинные SELECT’ы продолжают читать старые версии, как будто ничего не менялось,
    - читатели не блокируют писателей и наоборот (чтение не блокирует запись, запись не блокирует чтение).

2. Стабильный снимок данных для длительных запросов

- Проблема: Длинный отчёт (SELECT) должен видеть консистентную картину, а в это время кто-то активно апдейтит таблицу.
- С MVCC:
    - В начале отчёта транзакция берёт snapshot,
    - весь запрос видит одну и ту же версию мира, даже если в реальности данные уже обновились,
    - нет «прыгучих» результатов и не-repeatable read (в более строгих уровнях изоляции).

3. Меньше явных блокировок и ожиданий

- MVCC сильно снижает:
    - конкуренцию за блокировки,
    - количество случаев, когда запросы стоят в очереди и ждут друг друга.
- По факту:
    - блокировки нужны в основном между писателями (оставшийся конфликт: два UPDATE одной и той же строки),
    - но чтения почти всегда идут без блокировок.

4. Реализация сильных уровней изоляции (до Serializable)

- На базе MVCC можно реализовать:
    - Read Committed, Repeatable Read,
    - Serializable (в PostgreSQL - через Serializable Snapshot Isolation + проверка конфликтов).
- Без MVCC пришлось бы намного жёстче блокировать данные -> меньше параллелизма.

**Минусы:**

- мусор (dead tuples) - старые версии строк -> нужен VACUUM/GC.
- Более сложная логика видимости (xmin/xmax, snapshot, XID, wraparound).
- Потенциальный раздутие таблиц/индексов лишним пространством, если плохо настроен vacuum.

# ACID

**О том - какие гарантии дает бд при работе с параллельными транзакциями**

- **A - Atomicity (атомарность)** - либо все операции внутри транзакции выполняются, либо ни одной не выполняется.


- **C - Consistency (согласованность)** - каждая транзакция переводит базу из одного консистентного состояния в другое.
  Консистентность определяется ограничениями (constraints), триггерами, приложением, бизнес логикой.

  > В контексте транзакций в бд, консистентность - это свойство системы, которое гарантирует, что бд всегда будет в
  > валидном состоянии после выполнения транзакции. То есть, после того как транзакция завершена, данные
  > должны соответствовать всем ограничениям и правилам базы данных (например, внешним ключам, уникальности).
  > Например: если транзакция перевела деньги с одного счёта на другой, консистентность гарантирует, что в системе не
  > будет денег, потерянных или появившихся из ниоткуда.

- **I - Isolation (изоляция)** - параллельные транзакции не должны мешать друг другу. Появляются аномали и уровнем
  изоляций их можно решать.


- **D - Durability (надёжность)** - после успешного коммита транзакции её изменения сохраняются навсегда, даже при сбое
  системы. Все изменения (и факт коммита) сначала пишутся в WAL (журнал). После падения - база replay’ит журнал,
  восстанавливая состояние до последнего коммита.

# Аномалии параллельных транзакций

- **Грязное чтение (dirty read)** - если одна транзакция видит измененные данные другой незавершенной транзакции<br/>
  Транзакция А изменила данные, но ещё не закоммитила.<br/>
  Транзакция B уже читает эти грязные данные.<br/>
  Потом А делает ROLLBACK -> B опиралась на то, чего как бы никогда не было.<br/>
  **Присутствует на уровне Read Uncommitted.**


- **Неповторяющееся чтение** - если один и тот же запрос в рамках транзакции возвращает разные результаты<br/>
  В начале транзакции B читает строку (баланс = 100).<br/>
  Параллельно A коммитит UPDATE (баланс = 200).<br/>
  B снова читает ту же строку в рамках той же транзакции и вдруг видит уже 200.<br/>
  То есть одна и та же выборка внутри одной транзакции даёт разные результаты.<br/>
  **Присутствует на уровне Read Committed.**


- **Фантомное чтение** - если один и тот же запрос в рамках транзакции возвращает разное число записей<br/>
  В транзакции B: SELECT * FROM orders WHERE status = 'NEW';<br/>
  Пока B работает, другая транзакция A добавляет новую подходящую строку и коммитит.<br/>
  B повторяет тот же SELECT и внезапно видит ещё одну строку - фантом.<br/>
  **Присутствует на уровне Repeatable Read.**

> **Изоляция отвечает: какие из этих феноменов допустимы на данном уровне, а какие - нет.**

# Уровни изоляции

PostgreSQL использует MVCC (Multi-Version Concurrency Control):

- Каждая транзакция видит снимок (snapshot) данных: только те версии строк, которые были действительны на момент
  начала транзакции (или запроса - зависит от уровня).
- Новые версии строк создаются, старые не затираются сразу -> можно читать старую картину мира, пока другие пишут
  новую.

Дальше включаются уровни изоляции:

- **READ UNCOMMITED** - НЕ ПОДДЕРЖИВАЕТСЯ В POSTGRESQL


- **READ COMMITTED**
    - Каждый запрос внутри транзакции видит актуальные на момент начала запроса коммитнутые данные.
    - Можно словить неповторяемые чтения и фантомы.
    - Зато меньше блокировок, больше параллелизма.


- **REPEATABLE READ**
    - Вся транзакция работает с одним снимком, сделанным при её старте.
    - Одну и ту же строку ты видишь одинаковой весь срок жизни транзакции.
    - Нет грязных и неповторяемых чтений, но возможны фантомы некоторых типов.
    - В PostgreSQL это ещё и SSI: при конфликтующих транзакциях одну может откатить.


- **SERIALIZABLE**
    - PostgreSQL пытается сделать так, будто все транзакции выполнялись строго по очереди, а не параллельно.
    - Максимальная изоляция, но может больше откатывать транзакции, если видит опасный конфликт.

**Выбор режима изоляции транзакции:** `BEGIN TRANSACTION ISOLATION LEVEL SERIALIZABLE;`

# VACUUM

Из-за MVCC: UPDATE и DELETE не переписывают строку, а оставляют старую версию и создают новую (или помечают как
удалённую).
В таблице накапливаются мертвые версии строк (dead tuples): старые версии после UPDATE, удалённые строки.

Пока есть активные транзакции, которые ещё могут видеть эти версии по своему snapshot, их нельзя просто выкинуть. Когда
уже никто не может их увидеть - это мусор, который надо убрать.

Вот это и делает VACUUM: он и есть сборщик мусора и сборщик старых XID.

**Алгоритм работы**

- Обходит строки таблицы
    - Смотрит на xmin/xmax версии.
    - По состоянию транзакций понимает: эту версию может увидеть кто-то ещё (живёт) или никто уже не увидит (мертва).
- Помечает мёртвые версии как свободное место
    - Перестаёт учитывать их как живые.
    - Помечает space на странице как доступный для новых вставок.
    - В индексах помечает соответствующие entries как «невалидные» (или чистит, зависит от версии и типа VACUUM).
- Обновляет visibility map / статистику
    - Visibility map помогает знать, какие страницы полностью состоят из видимых строк -> оптимизация index-only scans.
    - Статистика нужна планировщику запросов.
- Заморозка XID (VACUUM FREEZE)
    - Старые строки, которые уже очень давно закоммичены, получают специальный FrozenXid.
    - Это защищает от wraparound (переполнения 32-битных XID), чтобы через много лет база не «забыла», что эта строка
      давно “живёт”.

**Autovacuum vs ручной VACUUM**

- В PostgreSQL есть autovacuum launcher, который периодически запускает autovacuum worker’ы.
- Он сам решает, когда вакуумить таблицы, по порогам:
    - сколько было вставок/апдейтов/удалений,
    - возраст XID в таблице.
- Прод база почти всегда живёт на autovacuum, если его не отключали.

**Ручной VACUUM**

Можно вызвать самому

```sql
VACUUM
    table_name; -- мягкая уборка, без полной перестройки
VACUUM
    FREEZE table_name; -- агрессивная заморозка XID
VACUUM FULL
    table_name; -- полная перестройка таблицы с сжатием (блокирует)
```

> VACUUM FULL - это уже не просто уборка, а перепаковка таблицы:
> создаётся новый файл таблицы, копируются только живые строки,
> после чего старый файл удаляется -> реальное уменьшение файла на диске, но с блокировкой.



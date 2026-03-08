# Коннект

`psql -d <database_name> -h <hostname> -p <port_number> -U <username>`

Указывать -d <database_name> иначе будет пытаться подключиться к базе данных с именем пользователя

# Команды

| Команда              | Описание                  |
| -------------------- | ------------------------- |
| `\l` `\list` `\l+`   | all databases             |
| `\c <database_name>` | connect to database       |
| `\dt` `\dt+`         | all tables in database    |
| `\dv` `\dv+`         | all views in database     |
| `\dn` `\dn+`         | all schema in database    |
| `\df` `\df+`         | all functions in database |
| `\di` `\di+`         | all indexes in database   |
| `\du`                | all users                 |
| `\q`                 | quit                      |
| `\! <command>`       | shell command             |
| `\?`                 | help command              |
| `\h <command>`       | help command              |
| `\o <filename>`      | last response in file     |
| `\i <filename>`      | execute script from file  |
| `\echo <string>`     | echo in console           |

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
| --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `SELECT ... FROM ...` | SELECT: имена колонок (`SELECT id`), выражения (`SELECT price * count AS total`), агрегаты (`SELECT count(*)`) <br/> FROM: `SELECT u.id, o.id FROM users u`                                                                                                                         |
| `WHERE`               | Фильтрует строки до группировки: `WHERE status = 'open' AND total > 100;`, `WHERE email LIKE '%@gmail.com';`                                                                                                                                                                        |
| `GROUP BY`            | Объединяет строки в группы по указанным полям <br/> таблица: orders(id, user_id, total)<br/>`SELECT user_id, SUM(total) AS total_sum FROM orders GROUP BY user_id;` - берет все строки из orders, делит их на группы по user_id <br/> Результат: (user_id, total_sum)               |
| `HAVING`              | Фильтр для групп после GROUP BY <br/> таблица: orders(id, user_id, total)<br/>`SELECT user_id, SUM(total) AS total_sum FROM orders GROUP BY user_id HAVING SUM(total) > 300;` - берем все строки, собираем группы по user_id, считаем для каждой группы total_sum, фильтруем группы |
| `ORDER BY`            | Сортирует строки результата (ASC(возр)/DESC(убыв))                                                                                                                                                                                                                                  |
| `LIMIT / OFFSET`      | Постраничная выборка, LIMIT n - максимум n строк в результате, OFFSET m - пропустить m строк                                                                                                                                                                                        |

**JOIN**

| Команда               | Описание                                                                                                                                                                            |
| --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
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
SET name  = 'Ivan Petrov', email = 'ivan.petrov@example.com'
WHERE id = 1;

DELETE FROM users
WHERE id = 1;

DELETE FROM users -- без where удалятся все данные

-- или быстрее:
TRUNCATE TABLE users;

-- RETURNING - вернуть данные измененных строк

INSERT INTO orders (user_id, total)
VALUES (10, 123.45)
RETURNING id;

DELETE FROM users
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

---

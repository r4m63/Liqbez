# Масштабирование PostgreSQL

## Главная идея

Горизонтальное масштабирование PostgreSQL почти никогда не начинается с мысли: "добавим еще 5 нод".
Сначала нужно выжать максимум из одного узла, понять реальное узкое место и только потом масштабировать именно тот слой, который уперся в лимит.

Обычно порядок такой:

1. Найти bottleneck.
2. Оптимизировать single-node.
3. Выбрать подходящую схему масштабирования:
   - read replicas
   - partitioning
   - logical replication
   - separate OLTP / OLAP
   - sharding / distributed PostgreSQL
   - cache и async processing

PostgreSQL из коробки уже дает важную базу для этого:

- physical replication
- logical replication
- hot standby
- declarative partitioning
- `pg_stat_statements`
- `pg_stat_io`

---

## 1. Репликация в PostgreSQL

### 1.1. Physical replication

**Physical replication** - это репликация всего инстанса PostgreSQL на уровне WAL и файлового состояния.

Для streaming replication серверы работают в ролях:

- `primary` - основной сервер
- `standby` - резервный сервер, который получает WAL от primary и применяет изменения

Это основной механизм для:

- high availability
- read-only реплик
- failover-сценариев

Подвиды physical replication:

- **File-based log shipping** - WAL архивируется и копируется как файлы, затем применяется на standby
- **Streaming replication** - WAL идет потоком почти в реальном времени
- **Cascading replication** - standby может получать WAL не только от primary, но и от другой standby

### 1.2. Logical replication

**Logical replication** - это репликация логических изменений, обычно на уровне таблиц и строк, а не полного файлового состояния кластера.

Она нужна, когда требуется:

- реплицировать не весь сервер, а только часть данных
- гибко переносить данные между разными кластерами
- выделять отдельные домены или сервисы
- строить selective fan-out

### 1.3. По режиму подтверждения

Physical replication, особенно streaming replication, часто делят еще по режиму подтверждения commit:

| Режим          | Как работает                                           | Плюсы                  | Минусы                                           |
| -------------- | ------------------------------------------------------ | ---------------------- | ------------------------------------------------ |
| `asynchronous` | primary не ждет подтверждения standby на каждый commit | меньше задержка записи | при failover можно потерять последние транзакции |
| `synchronous`  | primary ждет подтверждения standby                     | выше надежность        | больше latency на запись                         |

### 1.4. Warm standby и hot standby

Это не отдельный способ передачи WAL, а режим использования standby.

| Режим          | Что это значит                                                           |
| -------------- | ------------------------------------------------------------------------ |
| `warm standby` | standby держится в актуальном состоянии, но для запросов не используется |
| `hot standby`  | на standby можно подключаться и выполнять read-only запросы              |

---

## 2. Кто управляет отказоустойчивостью

### 2.1. HA-менеджер

PostgreSQL сам умеет:

- быть `primary` и `standby`
- реплицировать WAL
- промоутить standby в новый primary

Но автоматическая координация кластера обычно делается внешним HA-слоем.

**HA-менеджер** - это внешний инструмент, который следит за узлами PostgreSQL и управляет их ролями при сбоях.

Что обычно делает HA-менеджер:

- следит, жив ли текущий primary
- решает, кто имеет право стать новым primary
- запускает `promotion` standby
- помогает избежать `split-brain`
- иногда bootstrap'ит узлы и управляет конфигом PostgreSQL

Типичные HA-менеджеры:

- Patroni
- repmgr
- pg_auto_failover

### 2.2. Pgpool-II

**Pgpool-II** - это прокси между клиентом и несколькими PostgreSQL-серверами.

Он может:

- давать единую точку входа
- отправлять `SELECT` на standby
- отправлять запись на primary
- участвовать в failover-сценариях
- делать балансировку read-трафика

Идея простая:

- клиент подключается не напрямую к PostgreSQL
- клиент подключается к `Pgpool-II`
- уже `Pgpool-II` решает, куда отправить запрос

### 2.3. HAProxy

**HAProxy** - это сетевой прокси / балансировщик.

Он:

- не управляет ролями PostgreSQL
- не назначает новый primary
- не координирует failover как HA-менеджер

Его задача:

- держать единый endpoint
- проверять backend'ы через health checks
- отправлять трафик только на живые узлы

### 2.4. Типичный production-стек

Если нужен и uptime, и read scaling, часто собирают такую схему:

- PostgreSQL streaming replication
- HA-менеджер, например Patroni
- HAProxy как единая точка входа
- PgBouncer для connection pooling

---

## 3. С чего начинается масштабирование

### 3.1. Шаг 0. Не масштабируй, пока не понял узкое место

Сначала измеряешь:

- топ медленных и самых частых запросов через `pg_stat_statements`
- I/O и WAL-нагрузку через встроенные статистики, включая `pg_stat_io`
- блокировки
- long transactions
- bloat
- autovacuum lag
- `p95` / `p99` latency
- `TPS` / `QPS`
- active connections
- cache hit ratio
- replication lag

Без этого очень легко масштабировать не то место.

### 3.2. Шаг 1. Сначала выжми максимум из single-node

До горизонтального scale-out обычно делают:

- тюнинг запросов и индексов
- удаление `N+1`
- удаление лишних `JOIN`
- нормальный `autovacuum` / `vacuum`
- connection pooling через `PgBouncer`
- partitioning для очень больших таблиц
- вынос тяжелой аналитики и отчетов из OLTP

Очень часто это дает больший эффект, чем раннее шардирование.

---

## 4. Основные варианты масштабирования

### 4.1. Read scaling: primary + read replicas

Это первый и самый частый шаг, если bottleneck - чтение.

Что делают:

- оставляют один primary для `write`
- добавляют physical streaming replicas
- read-only трафик уводят на standby
- приложение подключают либо через отдельный read endpoint, либо через прокси

Когда подходит:

- каталог
- лента
- история операций
- profile screens
- много `SELECT`, мало `UPDATE` / `INSERT`
- нужно разгрузить primary

Плюсы:

- простая и понятная топология
- хорошо снимает read-нагрузку
- строится на штатных возможностях PostgreSQL

Ограничения:

- writes горизонтально не масштабируются
- есть replication lag
- нужно учитывать stale reads в приложении
- в async failover возможна потеря последних неподтвержденных на standby транзакций

### 4.2. HA + read scaling

Если нужен не только performance, но и отказоустойчивость, обычно строят:

- PostgreSQL streaming replication
- HA-менеджер, например Patroni
- HAProxy для единой точки входа и health checks
- PgBouncer для pooling

Когда подходит:

- production с требованиями по uptime
- нужен один writer и понятная write topology
- основная проблема - connections, failover и read traffic
- distributed writes пока не нужны

Плюсы:

- проще сопровождать, чем distributed cluster
- хороший компромисс между надежностью и сложностью
- закрывает и HA, и read scaling

### 4.3. Partitioning внутри одного кластера

Если проблема в размере таблиц, вакууме, индексах и сканах, а не в числе write-нод, часто достаточно:

- declarative partitioning по дате
- partitioning по `tenant_id`
- partitioning по `region`
- `hash` / `range` / `list`
- partition pruning
- локальных индексов на партициях

Когда подходит:

- time-series
- ledger / event tables
- большие append-heavy таблицы
- retention и удаление старых разделов

Плюсы:

- снижает нагрузку внутри одного кластера
- упрощает retention
- часто заметно ускоряет запросы и обслуживание больших таблиц

Важно:

Это не горизонтальный scale-out между серверами, но часто сильно снимает нагрузку и позволяет надолго отложить шардирование.

### 4.4. Logical replication для selective scaling

Когда нужно реплицировать не весь инстанс, а отдельные таблицы или домены, используют:

- `publication / subscription`
- selective fan-out в отдельные кластеры
- миграции с минимальным downtime
- выделение hot tables
- выделение отдельных сервисов

Когда подходит:

- отделить reporting cluster
- вынести billing / reporting / search feeder
- миграция монолита к сервисам

Плюсы:

- гибче physical replication
- можно масштабировать или выделять отдельные части системы
- удобно для эволюции архитектуры

Ограничения:

- выше операционная сложность
- не решает автоматически проблему write-scaling всего primary

### 4.5. Sharding / distributed PostgreSQL

Если bottleneck уже в writes, общем throughput одного primary или объеме данных, нужен scale-out по данным:

- application-level sharding
- Citus / distributed PostgreSQL
- tenant-based sharding
- hash sharding
- geo sharding

Когда подходит:

- write-heavy OLTP
- multi-tenant SaaS
- объем данных больше, чем комфортно держать на одной ноде
- нужен рост throughput при добавлении узлов

Плюсы:

- позволяет масштабировать запись и общий объем данных
- дает реальный scale-out, когда одна нода больше не справляется

Минусы:

- сложнее schema design
- сложнее cross-shard `JOIN`
- сложнее cross-shard transactions
- сложнее rebalancing
- сложнее backups и migrations

Это уже отдельный уровень операционной сложности.

### 4.6. Separate OLTP / OLAP

Если primary страдает из-за отчетности и тяжелых аналитических запросов:

- OLTP оставляют на основном PostgreSQL
- аналитические данные отдают в отдельный кластер или warehouse
- BI и тяжелые `JOIN` / `AGGREGATION` убирают с transactional базы

Когда подходит:

- отчеты мешают транзакционной нагрузке
- аналитика сильно отличается по профилю от OLTP
- нужно разгрузить primary без усложнения write-path

Плюсы:

- OLTP и аналитика перестают мешать друг другу
- проще отдельно масштабировать отчетный контур

### 4.7. Cache + async processing

Если база не справляется из-за повторяющихся reads или тяжелой синхронной бизнес-логики, помогают:

- Redis / cache layer для hot reads
- очереди
- асинхронная обработка
- materialized projections
- read models
- idempotent consumers

Это не заменяет масштабирование PostgreSQL, но часто резко снижает давление на primary.

---

## 5. Как выбрать вариант по симптомам

| Симптом                                       | Что делать в первую очередь                                            |
| --------------------------------------------- | ---------------------------------------------------------------------- |
| CPU уходит в сложные `SELECT`                 | сначала индексы, планы, partitioning; потом read replicas              |
| слишком много connections                     | сначала `PgBouncer`                                                    |
| I/O и autovacuum упираются в огромные таблицы | partitioning, retention, schema redesign                               |
| уперлись в write throughput одного primary    | думать о sharding / distributed PostgreSQL, а не о новых read replicas |
| длинные отчеты мешают OLTP                    | отделять OLAP от OLTP                                                  |
| нужен uptime и быстрый failover               | streaming replication + Patroni + HAProxy + PgBouncer                  |
| нужно вынести часть домена в отдельный контур | logical replication / service decomposition                            |

---

## 6. Практическая шпаргалка

Если коротко:

- **Много чтения** -> read replicas
- **Нужен failover и отказоустойчивость** -> HA-менеджер + proxy + pooling
- **Огромные таблицы** -> partitioning
- **Нужно реплицировать только часть данных** -> logical replication
- **Уперлись в запись одного primary** -> sharding / distributed PostgreSQL
- **Отчеты мешают транзакциям** -> separate OLTP / OLAP
- **База страдает от повторяющихся reads и тяжелой синхронной логики** -> cache + async processing

Главное правило:

> Сначала понять, что именно уперлось в лимит, и только потом выбирать стратегию масштабирования.

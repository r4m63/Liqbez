# Архитектура Postgres


Когда клиент подключается к Postgres:
- запрос сначала принимает `postmaster` -- один главный управляющий процесс, он принимает подключения и запускает backend-процессы.
- postmaster форкает отдельный backend-процесс `postgres` -- рабочие серверные процессы которые реально выполняют SQL. Отдельный серверный процесс на каждое клиентское подключение.


`postmaster` — это родительский управляющий процесс.
- Postmaster сам не выполняет SQL пользователя.
- Он — диспетчер и supervisor.
- стартует экземпляр PostgreSQL
- выделяет и инициализирует shared memory
- запускает фоновые процессы
- слушает TCP/Unix socket
- принимает подключения клиентов
- создает backend-процессы
- следит за авариями дочерних процессов
- инициирует recovery после crash

`postgres`
- проходит аутентификацию
- читает параметры сессии
- получает SQL от клиента
- парсит, анализирует, планирует и выполняет запрос
- читает/изменяет shared buffers
- создает WAL-записи
- работает с блокировками, снапшотами, MVCC
- возвращает результат клиенту


---

- высокие накладные расходы на очень большое число соединений
- важность connection pooling (pgBouncer, pgpool-II)
- изоляция сбоев между сессиями лучше, чем в thread-based серверах
- память и ресурсы часто считаются на процесс / на сессию

----------------------------------------------------------------------------------------------------

Фоновые процессы
    writer
    wal writer
    checkpointer
    log writer
    archiver
    stats collector
В реальном PostgreSQL названия и состав немного точнее такие:
    background writer
    checkpointer
    wal writer
    autovacuum launcher + workers
    archiver
    logger (если включён)
    logical replication launcher / workers
    walsender / walreceiver (при репликации)
    startup process (на standby/при recovery)
    stats subsystem (в новых версиях уже не отдельный старый stats collector как раньше)

----------------------------------------------------------------------------------------------------


## Shared memory

Так как процессов много, Postgres использует общую память, чтобы они могли работать с единым состоянием сервера.
Shared memory делится на:

### Shared Buffers

Это главный кэш страниц таблиц и индексов в памяти.
В shared buffers лежат страницы данных по 8KB, считанные с диска или изменённые в памяти.

Когда backend хочет прочитать строку:
    по relation + block number ищется нужная страница в buffer pool
    если она уже есть — это buffer hit
    если нет — происходит read from disk и страница загружается в shared buffers
При записи
    Если строка меняется:
    модификация делается в shared buffers
    страница помечается как dirty
    на диск она может уйти позже
    но WAL должен быть записан раньше, чем dirty page попадет в data file
Это фундаментальный принцип write-ahead logging.

### WAL Buffers

Это буфер журнала предзаписи — Write-Ahead Log.
Когда транзакция меняет данные, PostgreSQL сначала формирует WAL records:
    INSERT
    UPDATE
    DELETE
    изменение страниц индексов
    DDL-операции
    информация для commit/abort и recovery
Эти записи сначала попадают в WAL buffers, а затем сбрасываются в WAL сегменты на диск.
Чтобы после падения можно было восстановить изменения, даже если страницы таблиц ещё не успели записаться в data files.

### Lock tables / lock manager structures

В shared memory есть структуры для блокировок:
    relation locks
    page/tuple locks (частично через инфу в tuple/header и transaction ids)
    advisory locks
    heavyweight locks
    lightweight locks
    spinlocks
В Postgres есть несколько уровней синхронизации:
    LWLock — внутренние легковесные блокировки структур shared memory
    spinlock — очень короткие low-level примитивы
    heavyweight lock — пользовательские/SQL-level locks (LOCK TABLE, relation lock и пр.)
    row-level locking — реализуется через метаданные tuple + xid + lock wait

### Transaction status / CLOG / pg_xact

Исторически CLOG = commit log. Сейчас чаще говорят про pg_xact, но смысл тот же:
где-то должна храниться информация о том, какая транзакция committed, aborted, in progress.
Когда tuple ссылается на xmin или xmax, backend должен понять:
    транзакция уже commit?
    aborted?
    ещё идёт?
Для этого используется статус транзакции.

----------------------------------------------------------------------------------------------------

## Data files

Это реальные файлы, в которых лежат:
    таблицы
    индексы
    TOAST-таблицы
    системные каталоги
PostgreSQL хранит данные по страницам (pages/blocks).
Обычно одна страница = 8 KB.
Таблица физически
Таблица — это не “один красивый объект”, а набор файлов под base/....
Внутри страниц лежат tuple с заголовками, где есть служебная информация, очень важная для MVCC:
    xmin — кто создал версию строки
    xmax — кто удалил/заменил
    флаги видимости
    ссылки HOT и т.д.

## WAL files

WAL — это журнал предзаписи.
Перед тем как грязная страница данных будет записана в data file,
соответствующая WAL-запись должна быть надежно записана на диск.
Это и есть write-ahead.

После сбоя сервер может:
перечитать WAL
“докрутить” committed-изменения
откатить незавершённые последствия crash

Что это даёт
crash safety
point-in-time recovery
streaming replication
logical decoding (частично через WAL)

## Log files

Обычные логи сервера:
    подключения
    ошибки
    медленные запросы
    autovacuum activity
    checkpoints
    deadlocks
    replication events
Это не WAL.
Надо четко различать:
WAL — журнал для восстановления и репликации
server logs — текстовые диагностические логи

## Archive files

Если включен archive_mode, завершённые WAL-сегменты можно отправлять в архив.
Это нужно для:
    PITR (point-in-time recovery)
    резервного восстановления
    некоторых схем DR

----------------------------------------------------------------------------------------------------


## ЖИЗНЕННЫЙ ЦИКЛ SQL ЗАПРОСА

Для каждого запроса backend `postgres` проходит примерно такие стадии:
- Parser
    - SQL превращается в внутреннее дерево разбора.
- Analyzer / Rewriter
    - Проверяются таблицы, колонки, типы, права доступа.
    - Также срабатывают rewrite rules, view expansion.
- Planner / Optimizer
    - Строится план выполнения:
    - Seq Scan / Index Scan / Bitmap Heap Scan
    - Nested Loop / Hash Join / Merge Join
    - Sort / Aggregate / Gather и т.д.
- Executor
    - План реально выполняется:
    - читаются страницы
    - ищутся tuple
    - ставятся блокировки
    - обновляются строки
    - пишется WAL


## ЖИЗНЕННЫЙ ЦИКЛ SELECT SQL ЗАПРОСА

Клиент отправляет SQL

Backend процесса клиента получает текст запроса.

Parse / analyze / plan

Postgres:
    разбирает SQL
    определяет relation users
    проверяет права
    решает, использовать индекс или seq scan

Executor начинает чтение
Допустим, найден Index Scan.

Backend ищет нужные страницы:
сначала в shared buffers
если нет — читает с диска

Проверка видимости tuple (MVCC)
Даже если нужная строка физически найдена, нужно проверить:
видна ли эта версия строки текущей транзакции?
не удалена ли она позже?
committed ли создатель строки?
То есть Postgres не просто “нашёл строку и отдал”.
Он обязательно применяет правила видимости MVCC.

Если tuple видим для snapshot текущей транзакции — строка идёт клиенту.


## ЖИЗНЕННЫЙ ЦИКЛ INSERT/UPDATE/DELETE SQL ЗАПРОСА

INSERT
Что делает backend
    находит/создает нужную страницу в heap
    формирует новый tuple
    записывает в tuple xmin = current_xid
    размещает tuple в page в shared buffers
    создает WAL record для этого изменения
    WAL попадает в WAL buffers
    при COMMIT WAL должен быть flushed на диск
    транзакция помечается committed
    клиент получает COMMIT OK
Data page может физически лечь на диск позже,
но commit уже считается успешным, если WAL flushed.

UPDATE
В PostgreSQL UPDATE обычно — это не изменение строки на месте, а создание новой версии строки.
То есть:
    старая версия tuple остаётся
    новая версия tuple создаётся
    старая получает xmax
    новая получает свой xmin
Это фундамент MVCC.
Почему так
Чтобы читатели не блокировали писателей, а писатели — читателей так жестко, как в СУБД с in-place update.
Следствие
Из-за UPDATE в Postgres появляются мертвые tuple, которые потом должен убрать VACUUM.

DELETE
DELETE тоже обычно не “стирает байты немедленно”.
Он:
    помечает tuple как удалённый определённой транзакцией (xmax)
    строка перестаёт быть видимой для будущих snapshot
    физически место потом освобождается вакуумом

----------------------------------------------------------------------------------------------------

## MVCC

MVCC = Multi-Version Concurrency Control

PostgreSQL хранит несколько версий одной и той же логической строки, чтобы:
читатели могли читать старую согласованную версию,
писатели могли писать новую версию,
снизить взаимные блокировки.


**Tuple** есть служебные поля:
- xmin — XID транзакции, создавшей эту версию
- xmax — XID транзакции, которая удалила/заменила эту версию
- другие служебные флаги


**Snapshot**
Когда начинается запрос или транзакция, Postgres строит snapshot:
- какие транзакции уже committed
- какие ещё в работе
- какие XID считать видимыми, И при чтении каждой строки проверяет её видимость относительно snapshot.

---

- Читатель и писатель не мешают друг другу так сильно
- SELECT может читать старую committed-версию строки
- UPDATE параллельно создает новую версию
- Но появляются “мертвые” версии
- Их никто не видит, но они занимают место.
- Поэтому нужен:
    - VACUUM
    - AUTOVACUUM
    - freeze старых XID


Из-за MVCC старые версии строк остаются в таблице.
Если их не убирать:
    таблицы раздуваются,
    индексы раздуваются,
    растёт I/O,
    тормозят запросы,
    возникает риск transaction ID wraparound.

**VACUUM**:
    помечает место, которое можно переиспользовать,
    очищает dead tuples,
    обновляет visibility map,
    помогает index-only scan,
    может freeze старые XID,

**Autovacuum**:
    Обычно VACUUM запускается автоматически через:
    autovacuum launcher,
    autovacuum workers,

Одна из самых частых причин проблем в PostgreSQL —
неправильно настроенный autovacuum или игнорирование bloat.

----------------------------------------------------------------------------------------------------

## WAL и durability

Сначала WAL, потом data pages.
То есть порядок такой:
    изменение сделано в памяти
    создана WAL-запись
    WAL flushed на диск
    commit подтвержден клиенту
    позже dirty page может попасть в data file


Что происходит при сбое
Допустим:
    commit уже был подтверждён
    data page ещё не успела записаться в файл таблицы
    сервер упал
При перезапуске PostgreSQL:
    читает checkpoint
    начинает recovery
    воспроизводит WAL после checkpoint
    восстанавливает изменения
Так обеспечивается консистентность.


fsync
    Говорит, что PostgreSQL должен реально обеспечивать сброс на стабильное хранилище через ОС.
synchronous_commit
    Определяет, насколько строго ждать flush WAL на commit.
    Если его ослабить, можно ускорить commit ценой риска потерять последние транзакции при аварии.
    Для production это тюнинг делается очень осторожно.


Checkpointer
Отвечает за checkpoint.
Что такое checkpoint
    Это момент, когда PostgreSQL гарантирует, что все изменённые страницы до определённой точки WAL записаны в data files.
    Checkpoint нужен, чтобы при crash recovery не проигрывать “слишком длинный” WAL-отрезок.
Что делает checkpointer
    заставляет dirty buffers сбрасываться на диск
    обновляет checkpoint record в WAL
    ограничивает длину recovery
Но есть цена
Слишком частые checkpoints:
    создают пики I/O
    увеличивают нагрузку на диск
    могут вызывать latency spikes


Background writer
Он не делает full checkpoint semantics.
Его задача — плавно заранее писать грязные страницы, чтобы backend-ам реже приходилось самим синхронно писать буферы.
То есть background writer:
    подчищает dirty buffers
    сглаживает I/O
    уменьшает write bursts
Простая разница
    background writer — “подметает постепенно”
    checkpointer — “фиксирует контрольную точку”


WAL writer
wal writer отвечает за более регулярный сброс WAL buffers в WAL files.
Но commit конкретной транзакции всё равно может инициировать нужный flush сам, если это требуется для durability.
То есть wal writer помогает:
    сглаживать запись WAL
    уменьшать пиковые задержки
    не держать WAL слишком долго только в памяти


Archiver
Если включено архивирование WAL:
    завершённый WAL-сегмент
    после закрытия / переключения
    копируется archiver-ом в архивное хранилище
Это нужно для:
    PITR
    резервного восстановления до точки времени
    стратегии disaster recovery


Logger / logs
Логгер пишет текстовые серверные логи.
Туда могут попадать:
    ошибки SQL
    deadlock detected
    duration запросов
    checkpoint completion
    autovacuum messages
    connection/disconnection
    replication state
Для эксплуатации это один из основных источников диагностики.


Статистика и planner
Чтобы оптимизатор выбирал правильный план, PostgreSQL хранит статистику:
    cardinality таблиц
    распределение значений
    null fraction
    most common values
    histogram bounds
    correlation
Эти данные обновляются через:
    ANALYZE
    autovacuum/analyze
Почему это важно
    Если статистика плохая или устарела, planner может выбрать ужасный план:
    seq scan вместо index scan
    nested loop вместо hash join
    неверный порядок join
Postgres оптимизирует не “магически”, а по статистическим оценкам стоимости.

----------------------------------------------------------------------------------------------------

Блокировки в PostgreSQL

Table-level locks
Row-level locks
Deadlocks
Advisory locks


Почему в Postgres много процессов, а не потоков
Это архитектурное решение.
Плюсы
    изоляция сессий
    один backend упал — не обязательно убил весь shared state напрямую
    исторически проще модель безопасности и надежности
Минусы
    каждый connection стоит памяти дороже
    контекстные переключения процессов тяжелее потоков
    большое число idle-коннектов вредно
Следствие
    Для production почти всегда нужен пул соединений.
Обычно:
    pgBouncer — самый популярный
    особенно важен для high-concurrency web workloads

----------------------------------------------------------------------------------------------------

Streaming replication
Основана на WAL.
Primary:
    генерирует WAL
    процесс walsender передаёт WAL standby
Standby:
    получает WAL через walreceiver
    записывает его
    startup process проигрывает WAL
Результат
    Standby становится физической копией primary почти в реальном времени.


Synchronous vs asynchronous replication
Asynchronous
    Primary не ждёт подтверждения standby.
    Быстрее, но есть риск потери последних подтвержденных на primary транзакций при катастрофе primary.
Synchronous
    Primary ждёт подтверждения от standby на commit.
    Надёжнее, но выше latency.


Logical replication
Передаются не сырые страницы/WAL на физическом уровне, а логические изменения:
insert/update/delete
по публикациям/подпискам
Полезно для:
    миграций
    selective replication
    интеграций
    zero-downtime upgrade сценариев


Recovery после сбоя
После аварийного завершения PostgreSQL делает recovery.
Механизм
читает последний checkpoint
проверяет состояние кластера
воспроизводит WAL начиная с checkpoint
возвращает committed-изменения
очищает последствия незавершённых транзакций
Почему это работает
Потому что WAL был записан раньше data pages.

----------------------------------------------------------------------------------------------------

Как устроены транзакции
Когда начинается транзакция:
    ей назначается XID
    строится snapshot
    операции работают с MVCC
    при commit статус транзакции фиксируется
    WAL commit record пишется и flush’ится
    после этого commit считается успешным
Isolation levels
В PostgreSQL есть:
    Read Committed
    Repeatable Read
    Serializable
Но реализуются они через MVCC и snapshot semantics, а не просто грубые блокировки.
Важно
    Repeatable Read в PostgreSQL довольно сильный и основан на snapshot isolation.
    Serializable реализован через SSI (Serializable Snapshot Isolation), а не через полный lock-based serial execution.

----------------------------------------------------------------------------------------------------

Индексы и доступ к данным
Postgres хранит индексы как отдельные relation files.
Типы индексов:
    B-tree
    Hash
    GIN
    GiST
    BRIN
    SP-GiST
Как используется индекс
    Индекс не заменяет таблицу полностью.
    Обычно он помогает быстро найти TID / tuple location, после чего executor идёт в heap.
Исключение — когда возможен index-only scan, если visibility map позволяет.

----------------------------------------------------------------------------------------------------

Память вне shared_buffers
    память Postgres — это не только shared_buffers.
    Есть ещё:
        work_mem — на сортировки, hash, aggregates, joins
        maintenance_work_mem — vacuum, create index, alter table
        temp_buffers
        локальная память backend-процесса
        stack
        OS page cache
    Если поставить очень большой work_mem, и одновременно пойдут десятки сложных запросов, память может “взорваться”, потому что это лимит на операцию, а не на инстанс целиком.
    Параметры
        shared_buffers
        work_mem
        maintenance_work_mem
        effective_cache_size
        wal_buffers
        max_wal_size
        checkpoint_timeout
        checkpoint_completion_target
        autovacuum_*
        max_connections


Что происходит при большом числе соединений
Так как каждое подключение = отдельный процесс, возникают проблемы:
много RAM на idle sessions
scheduler overhead
contention на shared structures
connection storms
проблемы при burst-трафике
Поэтому в production типичная схема такая:
приложение
pgBouncer
PostgreSQL с умеренным числом реальных backend-процессов



ДОП:
схему “путь запроса от клиента до диска” по шагам
отдельно очень глубокий разбор MVCC, VACUUM, xmin/xmax, visibility
разбор архитектуры PostgreSQL с точки зрения production-администрирования и тюнинга

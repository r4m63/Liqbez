# Архитектура PostgreSQL под капотом

## 1. Ментальная модель PostgreSQL

PostgreSQL это:

1. Мультипроцессный сервер (не thread-per-connection): на каждый клиентский коннект обычно отдельный backend-процесс `postgres`.
2. Общая разделяемая память для координации (`shared memory`).
3. Данные в файлах отношений (`relation files`) плюс журнал предзаписи WAL.
4. MVCC-движок: читатели не блокируют писателей, а видимость строк определяется снимком (`snapshot`), а не "текущим состоянием таблицы".

Ключевой принцип надежности:

1. Сначала WAL на диск.
2. Потом подтверждение транзакции клиенту.
3. Данные страниц таблиц могут дойти до диска позже (checkpoint/bgwriter).

## 2. Процессная архитектура

## Postmaster и backend-процессы

`postmaster` (главный процесс `postgres`) делает следующее:

1. Инициализирует shared memory и фоновые процессы.
2. Слушает сокет/порт.
3. На новый коннект форкает отдельный backend-процесс `postgres`.
4. Перезапускает упавшие служебные процессы при необходимости.

Backend-процесс на клиент:

1. Проходит pipeline: parse -> rewrite -> plan -> execute.
2. Работает с shared buffers, lock manager, WAL buffers.
3. Выполняет транзакции клиента до завершения сессии.

## Основные фоновые процессы

| Процесс                               | Роль                                                                                   |
| ------------------------------------- | -------------------------------------------------------------------------------------- |
| `checkpointer`                        | Делает checkpoint, управляет сбросом dirty-страниц и записью checkpoint record в WAL.  |
| `bgwriter`                            | Плавно выталкивает dirty-буферы из `shared_buffers`, чтобы снижать latency backend'ов. |
| `walwriter`                           | Периодически сбрасывает WAL buffers в WAL-сегменты на диск.                            |
| `autovacuum launcher` + workers       | Запускает `VACUUM/ANALYZE`, чистит мертвые tuple, обновляет статистику.                |
| `archiver`                            | При `archive_mode=on` отправляет завершенные WAL-сегменты во внешний архив.            |
| `logger` (`syslogger`)                | Пишет серверные логи в файлы при `logging_collector=on`.                               |
| `startup`                             | На старте/реплике выполняет recovery и WAL replay.                                     |
| `walreceiver`                         | На standby получает WAL от primary.                                                    |
| `walsender`                           | На primary стримит WAL подписчикам/репликам.                                           |
| `logical replication launcher/worker` | Логическая репликация (publication/subscription).                                      |
| `parallel workers`                    | Вспомогательные воркеры для parallel query.                                            |

Важно по терминологии:

1. В старых материалах есть `stats collector` как отдельный процесс. В современных PostgreSQL статистика в основном ведется через shared memory, отдельного процесса в прежнем виде нет.
2. `CLOG` историческое имя; сейчас это `pg_xact` (SLRU-хранилище статусов транзакций).

## 3. Shared memory: что реально лежит в RAM

Основные области:

1. `shared_buffers`: кэш страниц таблиц и индексов (8 KB pages).
2. `wal_buffers`: буфер для WAL-записей перед fsync/flush.
3. `Lock manager`: таблицы тяжеловесных блокировок, predicate locks, fast-path lock slots.
4. `ProcArray`: информация об активных транзакциях, нужная для snapshot/MVCC.
5. `SLRU`-буферы: кэш для `pg_xact`, `pg_subtrans`, `pg_multixact`, commit timestamp и т.д.
6. Фоновые структуры планировщика, статистики, replication slots и др.

Локальная память каждого backend:

1. `work_mem` для sort/hash/agg.
2. `temp_buffers` для временных таблиц.
3. `maintenance_work_mem` для VACUUM/CREATE INDEX/ALTER.
4. `MemoryContext`-иерархия для контролируемого выделения/освобождения памяти.

Практика:

1. `shared_buffers` это не весь кэш: есть еще page cache ОС.
2. Перераздача памяти между сотнями сессий неконтролируема: `work_mem` задается "на операцию", а не "на запрос целиком".

## 4. Физическое хранение данных

## Кластер и каталоги

`PGDATA` содержит:

1. `base/` таблицы/индексы пользовательских БД.
2. `global/` общекластерные каталоги.
3. `pg_wal/` WAL-сегменты.
4. `pg_xact/`, `pg_multixact/`, `pg_subtrans/` и др. служебные структуры.

## Отношения (relations)

Таблица и индекс физически это relation file (через `relfilenode`), обычно с сегментами по 1 GB.

Fork'и relation:

1. `main` основные страницы данных.
2. `fsm` Free Space Map (где есть свободное место).
3. `vm` Visibility Map (страницы, где tuple "все видимы"/"все frozen").
4. `init` для unlogged-таблиц (шаблон для реинициализации после краша).

## Страница (page) и tuple

Базовая единица I/O это страница 8 KB:

1. Заголовок страницы.
2. Массив line pointers.
3. Tuple-данные.

Heap tuple содержит служебные поля:

1. `xmin` XID создавшей транзакции.
2. `xmax` XID удалившей/обновившей транзакции.
3. `ctid` ссылка на актуальную версию (важно для HOT chain).
4. `infomask` флаги видимости/состояния.

## 5. Жизненный цикл SQL-запроса

## 5.1 Parse

SQL превращается в parse tree, выполняются базовые синтаксические/семантические проверки.

## 5.2 Rewrite

Правила (`RULE`), view expansion, security barrier преобразования.

## 5.3 Plan/Optimize

Планировщик выбирает plan tree по cost model:

1. Seq Scan, Index Scan, Bitmap Heap Scan.
2. Join-стратегии: Nested Loop, Hash Join, Merge Join.
3. Выбор зависит от статистики (`pg_statistic`), селективности, корреляции, `random_page_cost`, `effective_cache_size` и др.

## 5.4 Execute

Executor запускает узлы плана, читает/пишет страницы через buffer manager, соблюдает MVCC-видимость и блокировки.

## 6. MVCC и видимость данных

PostgreSQL не переписывает строку "на месте" при `UPDATE`:

1. Создается новая версия tuple.
2. Старая версия помечается `xmax`.
3. Читатели выбирают версию, видимую их snapshot.

Snapshot содержит:

1. `xmin` нижнюю границу видимости.
2. `xmax` верхнюю границу.
3. Список активных XID на момент старта statement/transaction.

Проверка видимости tuple:

1. Смотрим `xmin/xmax`.
2. Проверяем статус транзакции в `pg_xact` (committed/aborted/in-progress).
3. Учитываем уровень изоляции.

Уровни изоляции:

1. `READ COMMITTED`: новый snapshot на каждый statement.
2. `REPEATABLE READ`: один snapshot на всю транзакцию.
3. `SERIALIZABLE`: SSI с predicate locks и детекцией опасных структур.

## 7. WAL: гарантия durability

WAL (Write-Ahead Logging) пишет изменения в журнал раньше страниц таблиц.

При модификации:

1. Backend изменяет page в `shared_buffers`.
2. Формирует WAL record и кладет в `wal_buffers`.
3. COMMIT требует flush WAL до LSN коммита (в зависимости от `synchronous_commit`).
4. Клиент получает `COMMIT OK` только после подтвержденного условия durability.

Ключевые термины:

1. `LSN` позиция в WAL.
2. `full_page_writes`: при первом изменении страницы после checkpoint записывается полный образ страницы для защиты от torn pages.
3. `wal_level`: `minimal`/`replica`/`logical`.

## 8. Checkpoint и crash recovery

Checkpoint:

1. Фиксирует точку консистентности.
2. Принуждает нужные dirty-страницы к записи.
3. Позволяет recovery начинать replay с checkpoint, а не с начала времен.

После краша:

1. `startup` читает последний checkpoint.
2. Повторяет WAL (REDO) до конца журнала.
3. Незакоммиченные транзакции считаются откатанными логически (их данные невидимы по MVCC).

Итог: committed-транзакции, чей WAL был flushed, не теряются.

## 9. VACUUM, bloat и wraparound

Из-за MVCC старые версии tuple копятся как "мертвые" до очистки.

`VACUUM` делает:

1. Удаляет dead tuple и освобождает место для повторного использования.
2. Обновляет visibility map.
3. Продвигает `relfrozenxid` (защита от wraparound).

`ANALYZE` обновляет статистику планировщика.

Критичный риск:

1. XID 32-битный, есть wraparound.
2. Если таблицы долго не vacuum/freeze, можно получить аварийный anti-wraparound vacuum и деградацию сервиса.

Практика:

1. Следить за `age(datfrozenxid)` и `pg_stat_all_tables`.
2. Не отключать autovacuum глобально.

## 10. Блокировки и конкуренция

Типы блокировок:

1. Heavyweight locks (relation/table/transaction-level coordination).
2. Row-level locks (`FOR UPDATE`, `FOR KEY SHARE` и т.д.).
3. LWLocks для внутренних структур shared memory.
4. Spinlocks для очень коротких критических секций.
5. Predicate locks для SERIALIZABLE.

Deadlock:

1. PostgreSQL строит wait-for graph.
2. По `deadlock_timeout` запускает проверку.
3. Убивает одну из транзакций с ошибкой deadlock.

## 11. Индексы и методы доступа

Частые AM (access methods):

1. `btree` универсальный выбор, поддержка range/order.
2. `hash` узкоспециализированный equality.
3. `gin` inverted index (jsonb, array, full text).
4. `gist` геометрия, диапазоны, extensible operator classes.
5. `brin` очень большие таблицы с коррелированными данными.

Важно:

1. Индекс ускоряет чтение, но удорожает запись.
2. HOT update может избежать обновления индекса, если ключи индексов не изменились.
3. Неправильные fillfactor/индексная стратегия напрямую влияют на bloat.

## 12. Репликация и HA

## Физическая репликация

1. Primary пишет WAL.
2. Standby получает WAL (`walreceiver`) через `walsender`.
3. `startup` на standby применяет WAL replay.

Режимы:

1. Асинхронный: меньше latency, возможна потеря последних транзакций при аварии primary.
2. Синхронный: primary ждет подтверждение standby (`synchronous_standby_names`), выше latency, меньше риск потери.

## Логическая репликация

1. Публикации/подписки на уровне таблиц.
2. Репликация DML (не полный physical image).
3. Удобна для миграций/интеграций, но сложнее в конфликтных сценариях.

## PITR

Point-In-Time Recovery:

1. Base backup + архив WAL.
2. Восстановление на нужный момент времени/LSN/transaction.

## 13. Пулы соединений и масштабирование коннектов

Так как процесс на коннект дорогой:

1. `max_connections` нельзя бесконечно увеличивать.
2. Для high-load почти всегда нужен PgBouncer/pgpool-II (чаще PgBouncer).
3. Правильный режим pool'а (`transaction`/`session`) зависит от использования prepared statements и session state.

## 14. Observability: что мониторить в проде

Минимальный набор:

1. `pg_stat_activity`: активные запросы, wait_event, блокировки.
2. `pg_locks`: конфликтующие lock-и.
3. `pg_stat_statements`: топ SQL по времени/частоте/I/O.
4. `pg_stat_bgwriter` и checkpoint metrics.
5. WAL rate и lag реплик (`pg_stat_replication`, `pg_stat_wal_receiver`).
6. Autovacuum прогресс (`pg_stat_progress_vacuum`).
7. Bloat, размер relation/index, hit ratios.

Операционные симптомы:

1. Частые long checkpoints -> пики latency и I/O storms.
2. Рост replication lag -> риск долгого failover/RPO.
3. Долгие idle in transaction -> удержание xmin, раздувание таблиц, тормоза vacuum.

## 15. Сквозной путь операций

## INSERT/UPDATE/DELETE (упрощенно)

1. Backend получает statement.
2. Executor находит/меняет tuple в buffer cache.
3. Генерирует WAL record.
4. Ставит нужные lock-и.
5. На COMMIT пишет commit record в WAL и ждет flush (если требуется).
6. Dirty data pages могут записаться на диск позже.

## SELECT (упрощенно)

1. Планировщик выбирает scan/join strategy.
2. Executor читает tuple из cache/disk.
3. Для каждой tuple проверяет MVCC visibility.
4. Возвращает только версию, видимую текущему snapshot.

## 16. Соотнесение с вашей схемой

На вашей диаграмме ядро верное:

1. Клиент -> postmaster -> backend `postgres`.
2. Backend читает/пишет в разделяемую память.
3. WAL и файлы данных на диске разделены.
4. Есть фоновые процессы (`writer`, `wal writer`, `checkpointer`, `archiver`).

Что стоит уточнить для современных версий:

1. `writer` корректнее называть `bgwriter`.
2. `stats collector` как отдельный процесс исторически устарел.
3. `CLOG` сейчас обычно называют `pg_xact`/SLRU.
4. Кроме показанных процессов, важны `autovacuum`, `walsender/walreceiver`, `startup`, `logical replication workers`.

## 17. Частые ошибки в проде

1. Слишком высокий `max_connections` без пула -> context switching и OOM.
2. Игнорирование autovacuum -> bloat, wraparound, деградация плана.
3. Неверные checkpoint/WAL настройки -> шипы latency.
4. Отсутствие контроля long transactions -> vacuum не может чистить.
5. Непроверенные планы после изменения статистики/распределения данных.
6. Слепое использование `synchronous_commit=off` без понимания RPO.

## 18. Короткий итог

PostgreSQL это баланс четырех подсистем:

1. Процессная модель и shared memory.
2. MVCC + lock manager (корректность конкуренции).
3. WAL + checkpoint + recovery (надежность и crash safety).
4. Autovacuum + planner statistics (стабильная производительность во времени).

Если держать под контролем именно эти четыре блока, PostgreSQL предсказуемо масштабируется и устойчиво работает под высокой нагрузкой.

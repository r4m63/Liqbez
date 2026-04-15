

## 2. @Transactional — механика работы в Spring

### 2.1 Как работает под капотом

Spring использует **AOP-прокси** (JDK dynamic proxy или CGLIB) для перехвата вызовов методов, аннотированных `@Transactional`. Последовательность:

```
Вызов метода
    → TransactionInterceptor перехватывает
        → PlatformTransactionManager.getTransaction()
            → DataSource.getConnection()
            → connection.setAutoCommit(false)
        → Выполнение бизнес-логики
        → Если нет исключений: manager.commit()
        → Если RuntimeException/Error: manager.rollback()
        → connection.setAutoCommit(true), возврат в пул
```

### 2.2 Ключевые подводные камни

**Самовызов (self-invocation)** — главная ловушка. Вызов `@Transactional`-метода из того же класса обходит прокси, транзакция не создаётся:

```java
@Service
public class OrderService {

    public void processOrder(Order order) {
        // ⛔ Вызов идёт через this, а не через прокси
        // Транзакция НЕ будет создана
        this.saveOrder(order);
    }

    @Transactional
    public void saveOrder(Order order) {
        repository.save(order);
    }
}
```

**Решения:** вынести метод в другой бин; инжектить себя (`@Lazy private OrderService self;`); использовать `TransactionTemplate` программно; использовать AspectJ compile-time weaving вместо proxy-based AOP.

**Checked exceptions** — по умолчанию `@Transactional` откатывает только при `RuntimeException` и `Error`. Для checked exceptions нужно указать явно:

```java
@Transactional(rollbackFor = Exception.class)
public void riskyOperation() throws IOException { ... }
```

**readOnly = true** — это НЕ просто подсказка. В Hibernate отключается dirty-checking, что даёт реальный выигрыш на read-heavy операциях. PostgreSQL переключается в read-only snapshot mode. Spring может маршрутизировать на реплику через `AbstractRoutingDataSource`.

```java
@Transactional(readOnly = true)
public List<Order> findAllOrders() {
    return orderRepository.findAll();
}
```

**timeout** — задаёт максимальное время выполнения транзакции в секундах. Если превышен — откат:

```java
@Transactional(timeout = 5) // 5 секунд
public void longRunningOperation() { ... }
```

---

### 3.2 Уровни изоляции в SQL стандарте и Spring

```java
@Transactional(isolation = Isolation.READ_COMMITTED)
```

| Уровень            |
| ------------------ |
| `READ_UNCOMMITTED` |
| `READ_COMMITTED`   |
| `REPEATABLE_READ`  |
| `SERIALIZABLE`     |


**Isolation.DEFAULT** — использует уровень изоляции, настроенный в БД. PostgreSQL по умолчанию — `READ_COMMITTED`

---

## 4. Propagation — стратегии распространения транзакций

```java
@Transactional(propagation = Propagation.REQUIRED)
```

### 4.1 Полная таблица всех режимов

| Propagation          | Есть внешняя TX                         | Нет внешней TX     | Когда использовать                                                 |
| -------------------- | --------------------------------------- | ------------------ | ------------------------------------------------------------------ |
| `REQUIRED` (default) | Присоединяется                          | Создаёт новую      | Стандартный случай, бизнес-методы                                  |
| `REQUIRES_NEW`       | Приостанавливает внешнюю, создаёт новую | Создаёт новую      | Аудит-логи, которые должны сохраниться даже при откате основной TX |
| `NESTED`             | Создаёт savepoint                       | Создаёт новую      | Частичный откат внутри общей TX (не все провайдеры поддерживают)   |
| `SUPPORTS`           | Присоединяется                          | Без TX             | Read-only методы, где TX необязательна                             |
| `NOT_SUPPORTED`      | Приостанавливает внешнюю                | Без TX             | Тяжёлые вычисления, не требующие TX                                |
| `MANDATORY`          | Присоединяется                          | Бросает исключение | Методы, которые ДОЛЖНЫ вызываться в контексте TX                   |
| `NEVER`              | Бросает исключение                      | Без TX             | Методы, которые НЕЛЬЗЯ вызывать внутри TX                          |

### 4.2 Тонкости REQUIRES_NEW vs NESTED

**REQUIRES_NEW** — физически новое соединение из пула. Внешняя TX приостановлена (suspended). Внутренняя TX коммитится/откатывается независимо. Дедлок-риск: если обе TX блокируют одни и те же строки.

**NESTED** — тот же connection, используется SAVEPOINT. Откат вложенной TX откатывает только до savepoint. Коммит вложенной TX не окончателен — финальный коммит/откат зависит от внешней TX. JPA/Hibernate не поддерживает NESTED (только JDBC).

```java
@Service
public class OrderService {

    @Transactional
    public void placeOrder(Order order) {
        orderRepo.save(order);

        try {
            // Аудит сохранится даже при откате основной TX
            auditService.logAction("ORDER_PLACED", order.getId());
        } catch (Exception e) {
            // логируем, но не откатываем основную TX
        }
    }
}

@Service
public class AuditService {

    @Transactional(propagation = Propagation.REQUIRES_NEW)
    public void logAction(String action, Long entityId) {
        auditRepo.save(new AuditEntry(action, entityId));
    }
}
```

---

## 5. Optimistic vs Pessimistic Locking

### 5.1 Optimistic Locking (оптимистичная блокировка)

Философия: конфликты редки, поэтому не блокируем заранее, а проверяем при сохранении.

**Реализация через @Version в JPA/Hibernate:**

```java
@Entity
public class Account {
    @Id
    private Long id;

    @Version
    private Long version; // Hibernate автоматически инкрементирует

    private BigDecimal balance;
}
```

При каждом UPDATE Hibernate генерирует:

```sql
UPDATE account SET balance = ?, version = version + 1
WHERE id = ? AND version = ?
```

Если version не совпадает — `OptimisticLockException`.

**Когда использовать:** высокий read/write ratio; редкие конфликты; web-формы с длительным временем редактирования пользователем; микросервисная архитектура (нельзя держать DB lock между HTTP-запросами).

### 5.2 Pessimistic Locking (пессимистичная блокировка)

Философия: конфликты вероятны, блокируем строки на время работы.

```java
public interface AccountRepository extends JpaRepository<Account, Long> {

    // SELECT ... FOR UPDATE — эксклюзивная блокировка
    @Lock(LockModeType.PESSIMISTIC_WRITE)
    @Query("SELECT a FROM Account a WHERE a.id = :id")
    Account findByIdForUpdate(@Param("id") Long id);

    // SELECT ... FOR SHARE — разделяемая блокировка
    @Lock(LockModeType.PESSIMISTIC_READ)
    @Query("SELECT a FROM Account a WHERE a.id = :id")
    Account findByIdWithSharedLock(@Param("id") Long id);
}
```

**Таймаут блокировки** (критично для production):

```java
@QueryHints({
    @QueryHint(name = "javax.persistence.lock.timeout", value = "3000") // мс
})
@Lock(LockModeType.PESSIMISTIC_WRITE)
Account findByIdForUpdate(Long id);
```

**Когда использовать:** высокая конкуренция за одни и те же строки; критичные финансовые операции; короткие транзакции, где блокировка не создаёт проблем.

### 5.3 Сравнительная таблица

| Критерий                                 | Optimistic                | Pessimistic                           |
| ---------------------------------------- | ------------------------- | ------------------------------------- |
| Блокировка строк в БД                    | Нет                       | Да (FOR UPDATE / FOR SHARE)           |
| Момент проверки                          | При UPDATE/COMMIT         | При SELECT                            |
| Конфликт-стратегия                       | Retry (повтор)            | Wait (ожидание)                       |
| Дедлоки                                  | Невозможны                | Возможны                              |
| Пропускная способность (low contention)  | Высокая                   | Ниже из-за блокировок                 |
| Пропускная способность (high contention) | Деградирует (много retry) | Стабильная                            |
| Пригодность для распределённых систем    | Отлично                   | Плохо (lock не выходит за границу БД) |

### 5.4 Паттерн Optimistic Lock + Retry

```java
@Service
public class AccountService {

    @Retryable(
        retryFor = OptimisticLockException.class,
        maxAttempts = 3,
        backoff = @Backoff(delay = 100, multiplier = 2)
    )
    @Transactional
    public void transfer(Long fromId, Long toId, BigDecimal amount) {
        Account from = accountRepo.findById(fromId).orElseThrow();
        Account to = accountRepo.findById(toId).orElseThrow();
        from.debit(amount);
        to.credit(amount);
        // При конфликте версий — OptimisticLockException → retry
    }
}
```

---

## 6. Параллельные транзакции в Spring

### 6.1 Проблема: TransactionSynchronizationManager и ThreadLocal

Spring привязывает транзакцию к текущему потоку через `ThreadLocal`. Это означает, что `@Async` метод или `CompletableFuture.supplyAsync()` выполняются в другом потоке — без контекста транзакции вызывающего метода.

```java
@Transactional
public void processInParallel(List<Item> items) {
    // ⛔ НЕПРАВИЛЬНО: каждый поток создаст свою TX (или не создаст)
    items.parallelStream().forEach(item -> processItem(item));

    // ⛔ НЕПРАВИЛЬНО: CompletableFuture не наследует TX
    CompletableFuture.supplyAsync(() -> heavyComputation(item));
}
```

### 6.2 Правильные подходы

**Подход 1: Каждый поток — своя транзакция**

```java
@Service
public class BatchService {

    @Autowired
    private ItemProcessor itemProcessor; // Отдельный бин!

    public void processBatch(List<Item> items) {
        List<CompletableFuture<Void>> futures = items.stream()
            .map(item -> CompletableFuture.runAsync(
                () -> itemProcessor.processItem(item), // Свой бин → свой прокси → своя TX
                taskExecutor
            ))
            .toList();

        CompletableFuture.allOf(futures.toArray(new CompletableFuture[0])).join();
    }
}

@Service
public class ItemProcessor {

    @Transactional(propagation = Propagation.REQUIRES_NEW)
    public void processItem(Item item) {
        // Каждый вызов в своей TX
    }
}
```

**Подход 2: TransactionTemplate для ручного управления**

```java
@Service
public class BatchService {

    private final TransactionTemplate txTemplate;

    public void processBatch(List<Item> items) {
        ExecutorService executor = Executors.newFixedThreadPool(4);

        List<Future<?>> futures = items.stream()
            .map(item -> executor.submit(() ->
                txTemplate.execute(status -> {
                    // Программное создание TX в рабочем потоке
                    processItem(item);
                    return null;
                })
            ))
            .toList();

        // Собираем результаты, обрабатываем ошибки
        for (Future<?> f : futures) {
            try { f.get(); }
            catch (ExecutionException e) { /* обработка */ }
        }
    }
}
```

**Подход 3: Spring @Async + @Transactional**

```java
@Service
public class AsyncProcessor {

    // @Async создаёт новый поток, @Transactional создаёт TX в этом потоке
    @Async("taskExecutor")
    @Transactional
    public CompletableFuture<Result> processAsync(Item item) {
        Result result = doWork(item);
        return CompletableFuture.completedFuture(result);
    }
}
```

> Важно: `@Async` и `@Transactional` на одном методе работают, только если оба прокси правильно выстроены. Рекомендуется разнести на разные бины.

### 6.3 Virtual Threads (Java 21+) и транзакции

```java
@Bean
public TaskExecutor taskExecutor() {
    return new TaskExecutorAdapter(Executors.newVirtualThreadPerTaskExecutor());
}
```

Virtual threads работают с `ThreadLocal` — транзакционный контекст привязывается к virtual thread точно так же, как к platform thread. Но будьте осторожны: virtual threads + JDBC = pinning carrier thread при synchronized-блоках внутри JDBC-драйвера. PostgreSQL JDBC 42.7.0+ — resolved. MySQL Connector/J — в процессе.

---

## 7. Retry-механизмы

### 7.1 Spring Retry

```xml
<dependency>
    <groupId>org.springframework.retry</groupId>
    <artifactId>spring-retry</artifactId>
</dependency>
```

```java
@Configuration
@EnableRetry
public class RetryConfig {}

@Service
public class PaymentService {

    @Retryable(
        retryFor = {OptimisticLockException.class, TransientDataAccessException.class},
        noRetryFor = {IllegalArgumentException.class},
        maxAttempts = 3,
        backoff = @Backoff(
            delay = 200,       // начальная задержка мс
            multiplier = 2.0,  // экспоненциальный рост
            maxDelay = 5000    // максимальная задержка мс
        )
    )
    @Transactional
    public void processPayment(Payment payment) {
        // бизнес-логика
    }

    @Recover
    public void recoverPayment(OptimisticLockException e, Payment payment) {
        // Вызывается после исчерпания попыток
        alertService.notifyFailure(payment, e);
    }
}
```

### 7.2 Критическое правило: @Retryable ДОЛЖЕН быть НАД @Transactional

Retry должен оборачивать целую транзакцию, а не наоборот. Если retry внутри транзакции — после `OptimisticLockException` EntityManager становится невалидным, повторная попытка в той же TX бессмысленна.

Правильная архитектура:

```
@Retryable (бин-обёртка или тот же метод)
    └── @Transactional (новая чистая TX на каждую попытку)
            └── бизнес-логика
```

Варианты реализации:

```java
// Вариант 1: оба на одном методе
// Spring Retry прокси оборачивает Transactional прокси
@Retryable(retryFor = OptimisticLockException.class)
@Transactional
public void transfer(Long fromId, Long toId, BigDecimal amount) { ... }

// Вариант 2: явное разделение на два бина (надёжнее)
@Service
public class TransferFacade {
    @Retryable(retryFor = OptimisticLockException.class, maxAttempts = 3)
    public void transfer(Long fromId, Long toId, BigDecimal amount) {
        transferService.doTransfer(fromId, toId, amount);
    }
}

@Service
public class TransferService {
    @Transactional
    public void doTransfer(Long fromId, Long toId, BigDecimal amount) { ... }
}
```

### 7.3 RetryTemplate — программный подход

```java
@Bean
public RetryTemplate retryTemplate() {
    return RetryTemplate.builder()
        .maxAttempts(3)
        .exponentialBackoff(100, 2.0, 5000)
        .retryOn(TransientDataAccessException.class)
        .build();
}

// Использование
retryTemplate.execute(context -> {
    txTemplate.execute(status -> {
        processPayment(payment);
        return null;
    });
    return null;
});
```

### 7.4 Resilience4j Retry (альтернатива)

```java
@Retry(name = "paymentRetry", fallbackMethod = "fallback")
@Transactional
public void processPayment(Payment payment) { ... }
```

Конфигурация в `application.yml`:

```yaml
resilience4j:
  retry:
    instances:
      paymentRetry:
        maxAttempts: 3
        waitDuration: 200ms
        retryExceptions:
          - org.springframework.dao.OptimisticLockingFailureException
          - org.springframework.dao.TransientDataAccessException
```

---

## 8. XA и двухфазный коммит (2PC)

### 8.1 Архитектура XA (eXtended Architecture)

XA — стандарт Open Group (X/Open), определяющий интерфейс между **Transaction Manager (TM)** и **Resource Manager (RM)**. Позволяет координировать транзакции через несколько ресурсов (БД, очереди сообщений и т.д.).

**Компоненты:**

```
Application Program (AP)
    ↕
Transaction Manager (TM)     — координатор (Atomikos, Narayana, Bitronix)
    ↕            ↕
Resource Manager 1   Resource Manager 2   — участники (MySQL, PostgreSQL, JMS)
    (XAResource)         (XAResource)
```

**Java-интерфейсы:**
- `javax.transaction.xa.XAResource` — контракт между TM и RM
- `javax.transaction.UserTransaction` — API для AP
- `javax.transaction.TransactionManager` — внутренний API TM
- `javax.sql.XADataSource` — XA-совместимый DataSource

### 8.2 Протокол двухфазного коммита (2PC)

**Фаза 1: PREPARE (голосование)**

```
TM → RM1: PREPARE
TM → RM2: PREPARE

RM1: записывает все изменения в redo/undo log
     отвечает PREPARED (или ABORT)
RM2: аналогично
```

На этом этапе каждый RM гарантирует, что сможет выполнить COMMIT, если поступит команда. Данные записаны в лог, блокировки удерживаются.

**Фаза 2: COMMIT (решение)**

```
Если все RM ответили PREPARED:
    TM записывает COMMIT в свой лог (point of no return)
    TM → RM1: COMMIT
    TM → RM2: COMMIT

Если хотя бы один RM ответил ABORT:
    TM → все RM: ROLLBACK
```

**Критическая точка:** После записи решения COMMIT в лог TM — транзакция считается закоммиченной. Даже если TM упадёт, при восстановлении он прочитает лог и повторит COMMIT.

### 8.3 Проблемы 2PC

| Проблема                    | Описание                                                                                          | Последствия                                              |
| --------------------------- | ------------------------------------------------------------------------------------------------- | -------------------------------------------------------- |
| **Blocking**                | Если TM упал после PREPARE, но до COMMIT — все RM блокированы (держат locks) до восстановления TM | Недоступность ресурсов, возможный каскад timeout'ов      |
| **Single Point of Failure** | TM — единая точка отказа                                                                          | Потеря TM-лога = неопределённое состояние                |
| **Latency**                 | 2 синхронных раунда + fsync логов у TM и каждого RM                                               | На порядок медленнее локальных TX                        |
| **In-doubt transactions**   | RM в состоянии PREPARED не знает итога                                                            | Требуется ручное вмешательство или heuristic recovery    |
| **Heuristic exceptions**    | RM принимает решение (commit/rollback) без TM                                                     | HeuristicMixedException — данные могут быть inconsistent |

### 8.4 Оптимизации 2PC

**Read-Only Optimization:** Если RM в фазе PREPARE не делал записей — отвечает READ_ONLY и выходит из протокола. TM не посылает ему COMMIT.

**Last Resource Commit Optimization (LRCO):** Один (и только один) non-XA ресурс может участвовать в XA-транзакции. Он коммитится одной фазой в момент принятия решения. Снижает требования к ресурсам, но не гарантирует полную атомарность при сбое.

**One-Phase Commit:** Если только один RM участвует — TM пропускает PREPARE и сразу посылает COMMIT.

### 8.5 XA в Spring Boot — реализации

**Atomikos (наиболее популярный для embedded):**

```java
@Configuration
public class XAConfig {

    @Bean
    public UserTransactionManager atomikosTransactionManager() {
        UserTransactionManager utm = new UserTransactionManager();
        utm.setForceShutdown(false);
        return utm;
    }

    @Bean
    public UserTransactionImp userTransaction() throws SystemException {
        UserTransactionImp ut = new UserTransactionImp();
        ut.setTransactionTimeout(300);
        return ut;
    }

    @Bean
    public JtaTransactionManager transactionManager(
            UserTransactionManager atm, UserTransaction ut) {
        return new JtaTransactionManager(ut, atm);
    }

    @Bean
    public DataSource orderDataSource() {
        AtomikosDataSourceBean ds = new AtomikosDataSourceBean();
        ds.setUniqueResourceName("orderDB");
        ds.setXaDataSourceClassName("org.postgresql.xa.PGXADataSource");
        Properties props = new Properties();
        props.put("serverName", "localhost");
        props.put("databaseName", "orders");
        ds.setXaProperties(props);
        ds.setPoolSize(10);
        return ds;
    }

    @Bean
    public DataSource inventoryDataSource() {
        AtomikosDataSourceBean ds = new AtomikosDataSourceBean();
        ds.setUniqueResourceName("inventoryDB");
        ds.setXaDataSourceClassName("com.mysql.cj.jdbc.MysqlXADataSource");
        Properties props = new Properties();
        props.put("url", "jdbc:mysql://localhost:3306/inventory");
        ds.setXaProperties(props);
        ds.setPoolSize(10);
        return ds;
    }
}
```

**Narayana (JBoss / WildFly, production-grade):**

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-jta-narayana</artifactId>
</dependency>
```

```yaml
spring:
  jta:
    narayana:
      transaction-manager-id: "1"
      default-timeout: 300
      log-dir: ./narayana-logs
```

**Bitronix:** deprecated, не рекомендуется для новых проектов.

### 8.6 XA + JMS (транзакции БД + очередь сообщений)

```java
@Bean
public ActiveMQXAConnectionFactory xaConnectionFactory() {
    return new ActiveMQXAConnectionFactory("tcp://localhost:61616");
}

@Bean
public AtomikosConnectionFactoryBean atomikosConnectionFactory() {
    AtomikosConnectionFactoryBean bean = new AtomikosConnectionFactoryBean();
    bean.setUniqueResourceName("jmsXA");
    bean.setXaConnectionFactory(xaConnectionFactory());
    bean.setPoolSize(10);
    return bean;
}

@Service
public class OrderService {

    @Transactional // JTA транзакция покрывает и БД, и JMS
    public void placeOrder(Order order) {
        orderRepository.save(order);               // БД
        jmsTemplate.convertAndSend("orders", order); // JMS
        // Если JMS-send упадёт — откатится и БД
    }
}
```

---

## 9. Альтернативы 2PC для распределённых транзакций

### 9.1 Saga Pattern

Когда XA не подходит (микросервисы, разные БД, облачные managed-сервисы), используются Saga.

**Saga** — последовательность локальных транзакций, каждая из которых имеет компенсирующую транзакцию (compensating transaction).

**Два типа оркестрации:**

**Choreography (хореография)** — сервисы обмениваются событиями через брокер:

```
OrderService     → event: ORDER_CREATED
PaymentService   → слушает ORDER_CREATED → списывает деньги → event: PAYMENT_COMPLETED
InventoryService → слушает PAYMENT_COMPLETED → резервирует товар → event: INVENTORY_RESERVED
ShippingService  → слушает INVENTORY_RESERVED → создаёт доставку

При ошибке в InventoryService:
InventoryService → event: INVENTORY_FAILED
PaymentService   → слушает INVENTORY_FAILED → возврат денег (compensate)
OrderService     → слушает PAYMENT_REFUNDED → отмена заказа (compensate)
```

Плюсы: слабая связность, простота каждого шага. Минусы: сложно отслеживать общий прогресс, циклические зависимости, дебаг — ад.

**Orchestration (оркестрация)** — центральный координатор управляет сагой:

```java
public class OrderSagaOrchestrator {

    public void execute(Order order) {
        SagaExecution saga = SagaExecution.start();

        try {
            saga.step("payment",
                () -> paymentService.charge(order),      // action
                () -> paymentService.refund(order));      // compensate

            saga.step("inventory",
                () -> inventoryService.reserve(order),
                () -> inventoryService.release(order));

            saga.step("shipping",
                () -> shippingService.schedule(order),
                () -> shippingService.cancel(order));

        } catch (SagaStepException e) {
            saga.compensate(); // Откатывает все выполненные шаги в обратном порядке
        }
    }
}
```

Плюсы: чётко видна последовательность, проще тестировать. Минусы: оркестратор — центральная точка, больше связности.

**Frameworks:** Axon Framework, Eventuate Tram, MicroProfile LRA, Temporal.io, Apache Camel Saga.

### 9.2 Transactional Outbox Pattern

Проблема: как атомарно обновить БД и отправить событие?

```java
@Transactional
public void placeOrder(Order order) {
    orderRepository.save(order);

    // Сохраняем событие В ТУ ЖЕ БД в таблицу outbox
    outboxRepository.save(new OutboxEvent(
        "OrderCreated",
        objectMapper.writeValueAsString(order)
    ));
    // Одна локальная TX — атомарно!
}

// Отдельный процесс (Debezium CDC или polling)
// читает outbox, публикует в Kafka, удаляет запись
```

**Реализации:** Debezium (CDC на основе WAL/binlog), простой polling-scheduler, Spring Integration.

### 9.3 Event Sourcing + CQRS

Вместо хранения текущего состояния — храним последовательность событий:

```java
public class Account {
    private List<DomainEvent> changes = new ArrayList<>();

    public void debit(BigDecimal amount) {
        if (balance.compareTo(amount) < 0)
            throw new InsufficientFundsException();
        apply(new MoneyDebited(id, amount, Instant.now()));
    }

    private void apply(DomainEvent event) {
        // Мутируем состояние
        when(event);
        // Добавляем в uncommitted changes
        changes.add(event);
    }

    private void when(DomainEvent event) {
        if (event instanceof MoneyDebited e)
            this.balance = this.balance.subtract(e.amount());
    }
}
```

Преимущества: полный аудит-трейл, воспроизводимость, natural fit для распределённых систем (события — единица согласованности). Сложности: eventual consistency для read-моделей, snapshots для производительности, event schema evolution.

### 9.4 TCC (Try-Confirm-Cancel)

Двухфазный паттерн без координатора уровня БД:

```
Try:     каждый сервис резервирует ресурсы (но не коммитит)
Confirm: каждый сервис подтверждает резервацию
Cancel:  каждый сервис отменяет резервацию
```

```java
// В PaymentService:
public PaymentReservation tryCharge(Order order) {
    // Создаём hold на счёте, не списываем
    return new PaymentReservation(order.getAmount(), PENDING);
}

public void confirmCharge(PaymentReservation reservation) {
    reservation.setStatus(CONFIRMED);
    // Фактическое списание
}

public void cancelCharge(PaymentReservation reservation) {
    reservation.setStatus(CANCELLED);
    // Снятие hold
}
```

Отличие от Saga: TCC требует идемпотентности Confirm/Cancel, ресурсы «зарезервированы» после Try (в Saga шаги полностью коммитят).

---

## 10. Паттерн Idempotency (идемпотентность)

Фундаментальное требование для любого retry и любой распределённой системы.

```java
@Entity
public class IdempotencyKey {
    @Id
    private String key;          // UUID, генерируемый клиентом
    private String response;      // Сериализованный ответ
    private Instant createdAt;
}

@Transactional
public PaymentResult processPayment(String idempotencyKey, PaymentRequest req) {
    // Проверяем: уже обрабатывали?
    Optional<IdempotencyKey> existing = idempotencyRepo.findById(idempotencyKey);
    if (existing.isPresent()) {
        return deserialize(existing.get().getResponse());
    }

    // Первый вызов — обрабатываем
    PaymentResult result = executePayment(req);

    // Сохраняем ключ + результат (в той же TX!)
    idempotencyRepo.save(new IdempotencyKey(
        idempotencyKey, serialize(result), Instant.now()));

    return result;
}
```

---

## 11. Распределённые блокировки

Когда pessimistic lock в одной БД недостаточно — нужна распределённая блокировка.

### 11.1 Redis-based (Redisson / ShedLock)

```java
// Redisson
RLock lock = redissonClient.getLock("order:" + orderId);
try {
    if (lock.tryLock(5, 30, TimeUnit.SECONDS)) {
        // 5 сек ждём, 30 сек TTL
        processOrder(orderId);
    }
} finally {
    lock.unlock();
}
```

**Redlock-алгоритм** (Martin Kleppmann vs. Salvatore Sanfilippo — дискуссия):
Redisson реализует Redlock: блокировка на N/2+1 из N Redis-нод. Kleppmann доказал, что Redlock небезопасен при clock skew и GC-паузах. Для критичных данных лучше использовать fencing tokens.

### 11.2 ZooKeeper-based

```java
InterProcessMutex lock = new InterProcessMutex(curatorClient, "/locks/order/" + orderId);
try {
    if (lock.acquire(5, TimeUnit.SECONDS)) {
        processOrder(orderId);
    }
} finally {
    lock.release();
}
```

ZooKeeper обеспечивает более строгие гарантии, чем Redis (последовательная консистентность, ephemeral nodes). Но медленнее и требует отдельного кластера.

### 11.3 БД-based (ShedLock, advisoryLocks)

```java
// ShedLock — для scheduled tasks
@SchedulerLock(name = "reportGeneration", lockAtMostFor = "PT30M")
@Scheduled(cron = "0 0 * * * *")
public void generateReport() { ... }

// PostgreSQL Advisory Locks (лёгкие, без row-level overhead)
@Query(value = "SELECT pg_try_advisory_lock(:key)", nativeQuery = true)
boolean tryAdvisoryLock(@Param("key") long key);

@Query(value = "SELECT pg_advisory_unlock(:key)", nativeQuery = true)
boolean advisoryUnlock(@Param("key") long key);
```

---

## 12. Consistency Models — теоретический фундамент

### 12.1 CAP-теорема

В распределённой системе при сетевом разделении (Partition) нужно выбрать между Consistency и Availability:

- **CP-системы:** ZooKeeper, etcd, HBase — жертвуют доступностью ради консистентности.
- **AP-системы:** Cassandra, DynamoDB, CouchDB — жертвуют строгой консистентностью ради доступности.

В реальности выбор не бинарный — это спектр.

### 12.2 BASE vs ACID

| ACID        | BASE                  |
| ----------- | --------------------- |
| Atomicity   | Basically Available   |
| Consistency | Soft state            |
| Isolation   | Eventually consistent |
| Durability  |                       |

BASE — явный компромисс для распределённых систем: мы допускаем, что данные могут быть временно несогласованными, но придут к согласованности.

### 12.3 Модели согласованности (от строгой к слабой)

**Strict Consistency** → **Linearizability** → **Sequential Consistency** → **Causal Consistency** → **Eventual Consistency**

Для архитектора важно понимать: какой уровень согласованности реально нужен бизнесу? Банковский баланс — linearizability. Лента социальной сети — eventual consistency достаточна.

---

## 13. Spring Transaction Management — продвинутые темы

### 13.1 ChainedTransactionManager (deprecated, но встречается)

```java
// Коммитит несколько TM по очереди. НЕ атомарно!
// Если второй коммит упадёт — первый уже закоммичен
@Bean
public PlatformTransactionManager chainedTxManager(
        DataSourceTransactionManager ds1TxManager,
        DataSourceTransactionManager ds2TxManager) {
    return new ChainedTransactionManager(ds1TxManager, ds2TxManager);
}
```

Это best-effort, не настоящая распределённая транзакция.

### 13.2 TransactionTemplate — программное управление

```java
@Service
public class OrderService {

    private final TransactionTemplate txTemplate;

    public OrderService(PlatformTransactionManager txManager) {
        this.txTemplate = new TransactionTemplate(txManager);
        this.txTemplate.setIsolationLevel(
            TransactionDefinition.ISOLATION_REPEATABLE_READ);
        this.txTemplate.setTimeout(10);
    }

    public Order createOrder(OrderRequest req) {
        return txTemplate.execute(status -> {
            try {
                Order order = orderRepo.save(new Order(req));
                inventoryService.reserve(order);
                return order;
            } catch (InsufficientInventoryException e) {
                status.setRollbackOnly();
                throw e;
            }
        });
    }
}
```

Когда использовать: нужен тонкий контроль (setRollbackOnly, timeout per-call); в утилитных классах, где @Transactional не работает; в тестах.

### 13.3 TransactionSynchronization — хуки жизненного цикла

```java
@Transactional
public void placeOrder(Order order) {
    orderRepo.save(order);

    // Хук: выполнить ПОСЛЕ успешного коммита
    TransactionSynchronizationManager.registerSynchronization(
        new TransactionSynchronization() {
            @Override
            public void afterCommit() {
                // Безопасно отправить событие, т.к. данные уже в БД
                eventPublisher.publish(new OrderCreatedEvent(order.getId()));
            }
        }
    );
}

// Spring 4.2+: через @TransactionalEventListener
@TransactionalEventListener(phase = TransactionPhase.AFTER_COMMIT)
public void onOrderCreated(OrderCreatedEvent event) {
    notificationService.notify(event);
}
```

### 13.4 Несколько DataSource — маршрутизация

```java
public class ReadWriteRoutingDataSource extends AbstractRoutingDataSource {

    @Override
    protected Object determineCurrentLookupKey() {
        return TransactionSynchronizationManager.isCurrentTransactionReadOnly()
            ? "replica"
            : "primary";
    }
}

@Bean
public DataSource routingDataSource(
        DataSource primaryDataSource, DataSource replicaDataSource) {
    ReadWriteRoutingDataSource ds = new ReadWriteRoutingDataSource();
    ds.setTargetDataSources(Map.of(
        "primary", primaryDataSource,
        "replica", replicaDataSource
    ));
    ds.setDefaultTargetDataSource(primaryDataSource);
    return ds;
}
```

---

## 14. Тестирование транзакций

### 14.1 @Transactional в тестах

```java
@SpringBootTest
@Transactional // Каждый тест откатывается после завершения
class OrderServiceTest {

    @Test
    void shouldCreateOrder() {
        orderService.placeOrder(new OrderRequest(...));
        // Данные видны внутри теста (та же TX)
        assertThat(orderRepo.count()).isEqualTo(1);
        // После теста — автоматический ROLLBACK
    }
}
```

Осторожно: @Transactional в тестах скрывает баги с lazy loading (LazyInitializationException не воспроизведётся). Для интеграционных тестов лучше использовать `@Commit` или вручную чистить данные.

### 14.2 Testcontainers для реальных БД

```java
@Testcontainers
@SpringBootTest
class XATransactionTest {

    @Container
    static PostgreSQLContainer<?> postgres = new PostgreSQLContainer<>("postgres:16");

    @Container
    static MySQLContainer<?> mysql = new MySQLContainer<>("mysql:8");

    @Test
    void shouldCommitAcrossDatabases() {
        // Реальный XA-тест с двумя БД
    }
}
```

---

## 15. Production Checklist для Senior Architect

### 15.1 Выбор стратегии распределённых транзакций

```
Один сервис, одна БД?
    → Локальные TX + @Transactional

Один сервис, несколько БД/ресурсов?
    → XA/2PC (Atomikos/Narayana)
    → ИЛИ Outbox Pattern если допустима eventual consistency

Микросервисы, синхронная координация?
    → Saga (Orchestration) с Outbox
    → TCC для операций с резервацией

Микросервисы, асинхронная координация?
    → Saga (Choreography) + Event Sourcing
    → Outbox + CDC (Debezium)
```

### 15.2 Анти-паттерны

- **Распределённые транзакции через REST** — нет гарантий атомарности, retry небезопасны без идемпотентности.
- **Long-running XA-транзакции** — блокируют ресурсы, создают каскадные таймауты.
- **SERIALIZABLE «на всякий случай»** — убивает throughput; используйте минимально необходимый уровень.
- **Retry без idempotency** — дублирование операций.
- **Pessimistic locks через микросервисы** — блокировки не пересекают границы процессов.
- **Игнорирование порядка блокировок** — причина дедлоков. Всегда блокируйте ресурсы в детерминированном порядке (например, по ID).

### 15.3 Мониторинг

Что мониторить: количество и длительность TX; частота откатов; частота retry; время ожидания блокировок; in-doubt XA-транзакции; размер outbox-таблицы; consumer lag (при использовании Kafka).

Инструменты: Micrometer + Prometheus/Grafana; Spring Boot Actuator (`/actuator/metrics`); pg_stat_activity / SHOW ENGINE INNODB STATUS; Atomikos transaction logs.

---

## 16. Сводная таблица: когда что использовать

| Сценарий                              | Механизм                            | Trade-off                                  |
| ------------------------------------- | ----------------------------------- | ------------------------------------------ |
| CRUD одной сущности                   | `@Transactional` + Optimistic Lock  | Простота; retry при конфликтах             |
| Финансовая операция на одной БД       | `@Transactional` + Pessimistic Lock | Строгая консистентность; throughput ↓      |
| Два ресурса в одном сервисе           | XA / 2PC (Atomikos)                 | Атомарность; latency ↑↑                    |
| Микросервисы, сильная консистентность | Saga (Orchestration) + Outbox       | Сложность реализации; eventual consistency |
| Микросервисы, события                 | Choreography + Event Sourcing       | Максимальная decoupling; debugging ↓       |
| Scheduled tasks на кластере           | ShedLock / distributed lock         | Простота; single-execution guarantee       |
| High contention resource              | Distributed Lock (Redis/ZK)         | Координация; SPOF risk                     |


----
```
TransactionSynchronizationRegistry txRegistry
@Transactional
    public void importVehiclesTx(Long opId,
                                 List<VehicleImportItemDto> items,
                                 byte[] fileBytes,
                                 String finalKey,
                                 String safeName,
                                 String contentType,
                                 long size) {

        final AtomicInteger importedCount = new AtomicInteger(0);

        txRegistry.registerInterposedSynchronization(new Synchronization() {
            @Override
            public void beforeCompletion() {
                // 2 minio
                minioStorage.putObject(
                        finalKey,
                        fileBytes,
                        contentType,
                        Map.of("original-name", safeName)
                );
            }

            @Override
            public void afterCompletion(int status) {
                try {
                    if (status == Status.STATUS_COMMITTED) {
                        // success
                        opLogger.markSuccess(opId, importedCount.get(), finalKey, safeName, contentType, size);
                        wsHub.broadcastText("refresh");
                        return;
                    }

                    // rollback
                    minioStorage.removeObjectQuietly(finalKey);
                    opLogger.markFailure(opId, importedCount.get(), null, safeName, contentType, size);

                } catch (Throwable ignored) {
                }
            }
        });

        // 1 database
        for (VehicleImportItemDto item : items) {
            VehicleDto dto = VehicleImportItemDto.toEntity(item);

            vehicleService.checkUniqueVehicleName(dto.getName(), null);

            Coordinates coords = vehicleService.resolveCoordinatesForDto(dto);
            Vehicle v = VehicleDto.toEntity(dto, null);
            v.setCoordinates(coords);

            vehicleDao.save(v);
            importedCount.incrementAndGet();
        }
    }
```



-------

1. Главная рамка: что такое транзакции в Spring и где граница ответственности

Нужно сразу разделить 3 слоя:
	1.	бизнес-транзакция как логическая операция уровня домена,
	2.	техническая транзакция уровня БД / брокера / ресурса,
	3.	координация нескольких ресурсов — это уже мир JTA/XA/2PC.
Spring сам по себе дает транзакционную абстракцию и умеет работать как с локальными транзакциями (DataSourceTransactionManager, JpaTransactionManager), так и с глобальными/JTA-транзакциями (JtaTransactionManager). Spring Framework прямо описывает transaction management как единый abstraction layer над разными стратегиями.  ￼

Самая частая архитектурная ошибка: думать, что @Transactional автоматически решает распределенную согласованность между микросервисами. Нет. @Transactional в типичном Spring-приложении решает локальную транзакционность внутри одного процесса и обычно одного транзакционного менеджера. Для нескольких ресурсов или сервисов нужны отдельные паттерны: XA/2PC, Saga, Outbox/Inbox, idempotency, retry, компенсации.  ￼

⸻

2. Базовая модель ACID и почему этого недостаточно в distributed systems

Обычная локальная транзакция дает атомарность, согласованность, изоляцию и долговечность. Но как только у тебя в одной бизнес-операции участвуют две БД, БД + Kafka, БД + JMS, несколько сервисов, появляется вопрос: кто является координатором commit/rollback и как не получить “один ресурс закоммитился, другой нет”. Именно для этого и существует XA / two-phase commit. Jakarta Transactions прямо говорит, что transaction manager координирует двухфазный коммит между resource managers через XA-интерфейсы.  ￼

При этом в современных микросервисах XA используется намного реже, чем раньше, потому что цена у него высокая: сложность, latency, чувствительность к сбоям, проблемы масштабирования, ограничения облачных managed-сервисов и слабая поддержка многими не-XA системами. Поэтому архитектор должен знать XA глубоко, но еще важнее — понимать, когда его не использовать. Это уже инженерное решение, а не только знание API.  ￼

⸻

3. Spring transaction abstraction: что происходит под капотом

Spring держит транзакционный контекст на текущем потоке и связывает с ним ресурсы и синхронизации через TransactionSynchronizationManager. В его javadoc прямо сказано, что он управляет ресурсами и transaction synchronizations per thread. Это критично для понимания всего поведения Spring transactions.  ￼

Из этого следуют две важные истины:
(а) обычные imperative-транзакции в Spring завязаны на thread-bound context,
(б) транзакция не “перетекает сама” в другой поток через @Async, ExecutorService, CompletableFuture, параллельные стримы и т.д. Это уже другой execution context, и старый transaction context там не будет автоматически активен. Сам Spring описывает thread-bound synchronization model, а в testing docs отдельно указывает, что переход в новый поток ломает ожидаемое транзакционное поведение теста.  ￼

⸻

4. @Transactional: что это на самом деле

@Transactional — это метаданные, а не магия. Spring docs прямо говорят: сама аннотация только описывает transactional semantics, а реальное поведение появляется, когда включена соответствующая runtime infrastructure (@EnableTransactionManagement или XML-конфигурация).  ￼

Значения по умолчанию у @Transactional такие:
PROPAGATION_REQUIRED, ISOLATION_DEFAULT, read-write, rollback по RuntimeException и Error, но не по checked exception; timeout — дефолт underlying system. Это очень важно, потому что многие разработчики ошибочно думают, что любой exception откатит транзакцию. Нет, checked exceptions по умолчанию — не откатывают.  ￼

Также у @Transactional можно задавать propagation, isolation, timeout, readOnly, rollbackFor, noRollbackFor, а еще выбирать конкретный transaction manager через value/transactionManager. Isolation/timeout/readOnly применимы прежде всего для REQUIRED и REQUIRES_NEW.  ￼

⸻

5. Самая частая ловушка: proxy semantics и self-invocation

По умолчанию Spring обрабатывает @Transactional через AOP proxy. А значит, перехватываются только внешние вызовы через proxy. Если метод внутри класса вызвал другой метод этого же класса, то это self-invocation, и транзакционная аннотация на внутреннем методе может вообще не сработать. Spring docs формулируют это буквально: self-invocation не приводит к реальной транзакции в proxy mode.  ￼

Отсюда практическое правило архитектора:
не проектируй сервисы так, чтобы важная transactional semantics зависела от внутренних вызовов “this.method()”. Делай либо:
— разбиение на отдельные бины,
— либо programmatic transactions,
— либо AspectJ weaving, если действительно нужен interception для любых вызовов. Spring прямо указывает, что aspectj mode решает этот класс проблем, потому что там идет weaving байткода, а не proxy semantics.  ￼

⸻

6. Visibility методов и practical rules

В proxy mode транзакционные методы обычно должны быть public. Начиная с Spring 6 для class-based proxies могут работать и protected/package-private методы, но для interface-based proxies методы должны быть public и объявлены в интерфейсе. Spring отдельно рекомендует аннотировать методы concrete classes, а не полагаться на аннотации в интерфейсах.  ￼

Практически это значит:
для предсказуемости на enterprise-проекте делай transactional boundary на public method сервиса. Это минимизирует сюрпризы.  ￼

⸻

7. Propagation — ключ к пониманию вложенных вызовов

Propagation отвечает на вопрос: что делать, если transactional method вызвана внутри уже существующей транзакции. Spring подчеркивает разницу между logical transaction scopes и physical transaction. Это одно из самых важных мест во всей теме.  ￼

REQUIRED

Это дефолт. Если транзакции нет — создается. Если есть — текущий метод вступает в уже существующую физическую транзакцию. При этом внутренний scope может поставить transaction в rollback-only. Тогда внешний код может продолжать думать, что все ок, но при коммите получит UnexpectedRollbackException. Spring docs специально описывают этот сценарий.  ￼

Архитектурный смысл: REQUIRED хорош для “одного unit of work на запрос / use case”. Но если внутренний сервис должен жить независимо от внешнего outcome, это уже не REQUIRED.  ￼

REQUIRES_NEW

Всегда запускает новую независимую физическую транзакцию, не участвуя во внешней. Внутренняя транзакция может закоммититься или откатиться независимо, ее locks освобождаются сразу после завершения. Spring также предупреждает, что для этого нужен дополнительный connection, и можно легко уткнуться в exhaustion connection pool или даже дедлок.  ￼

Когда применять:
— аудит/лог операции, которая должна сохраниться даже если внешний use case упал,
— сохранение outbox/event record отдельно,
— ограниченно и осторожно.
Но злоупотребление REQUIRES_NEW — классический путь к скрытым проблемам производительности и неожиданной семантике консистентности.  ￼

NESTED

Это не новая физическая транзакция, а savepoint внутри существующей. Внутренний блок можно откатить частично, а внешняя транзакция продолжится. Spring docs указывавают, что это обычно маппится на JDBC savepoints и работает только с JDBC resource transactions.  ￼

Когда применять:
если нужна частичная rollback-логика в одной БД.
Когда не применять:
если люди думают, что NESTED — это “маленькая независимая транзакция”. Это не так.  ￼

Остальные propagation modes — коротко

SUPPORTS, MANDATORY, NOT_SUPPORTED, NEVER — это более узкие инструменты управления contract’ом вызова. На практике в enterprise чаще всего реально важны REQUIRED, REQUIRES_NEW, NESTED. Остальные нужны, когда ты явно формализуешь запрет/обязательность транзакционного контекста. Это вывод из модели Spring propagation semantics.  ￼

⸻

8. Isolation: что именно изолируется и от чего

Isolation отвечает на вопрос: какие аномалии конкурентного доступа разрешены. PostgreSQL docs очень удобно перечисляют аномалии: dirty read, non-repeatable read, phantom read, serialization anomaly.  ￼

Spring позволяет указывать isolation через @Transactional(isolation = ...), а также программно через TransactionTemplate / TransactionalOperator. Spring docs показывают, что isolation level — часть transaction definition.  ￼

Но критически важно: ISOLATION_DEFAULT означает “используй дефолт конкретной БД/ресурса”. Например, в PostgreSQL default — READ COMMITTED. Более того, PostgreSQL прямо пишет, что READ UNCOMMITTED у него фактически ведет себя как READ COMMITTED, а REPEATABLE READ у него сильнее стандарта и не допускает phantom reads.  ￼

Практическая интерпретация уровней

READ_UNCOMMITTED
Теоретически допускает dirty reads, но на PostgreSQL это фактически тот же READ COMMITTED. В реальной enterprise-практике почти никогда не нужен.  ￼

READ_COMMITTED
Обычно дефолт. Хорош для большинства CRUD-нагрузок. Но допускает non-repeatable reads и phantom-like эффекты в зависимости от БД. На PostgreSQL SELECT видит snapshot на начало statement, а не всей транзакции.  ￼

REPEATABLE_READ
Лучше защищает повторные чтения. На PostgreSQL он не допускает phantom reads, хотя стандарт это допускает. Это хороший выбор для многих сценариев “прочитал-решил-обновил”, но не абсолютная защита от всех write skew/serialization anomalies.  ￼

SERIALIZABLE
Самый строгий уровень. Гарантирует поведение, эквивалентное последовательному выполнению транзакций. Но платой будут abort/retry under contention и стоимость concurrency control. На high-load системах без продуманного retry это может стать проблемой. PostgreSQL прямо показывает, что serialization anomalies здесь запрещены.  ￼

⸻

9. Важный нюанс: isolation в Spring и участие во внешней транзакции

Spring отдельно предупреждает: если метод с REQUIRED входит в уже существующую транзакцию, то его локальные настройки isolation/timeout/readOnly обычно тихо игнорируются и наследуются от outer transaction. Можно включить validateExistingTransactions=true, чтобы конфликтующие настройки не игнорировались молча. Это очень важный production-level нюанс.  ￼

То есть:
@Transactional(isolation = SERIALIZABLE) на внутреннем методе не гарантирует, что реально будет SERIALIZABLE, если он просто присоединился к outer REQUIRED. Для этого нужен либо новый physical transaction (REQUIRES_NEW), либо другой boundary.  ￼

⸻

10. readOnly = true: что это значит и что это не значит

readOnly = true — это подсказка transaction infrastructure и persistence layer, а не абсолютный запрет на любые изменения. Spring javadoc по TransactionSynchronizationManager указывает, что read-only flag может использоваться, например, для настройки Hibernate Session в более “легкий” режим вроде manual flush.  ￼

То есть readOnly=true полезен для оптимизации, но не надо относиться к нему как к security/consistency механизму. Если нужно гарантировать “не писать” — это уже вопрос контрактов кода, ролей, SQL permissions и архитектурной дисциплины.  ￼

⸻

11. Programmatic transactions: когда они лучше аннотаций

Spring поддерживает не только декларативные, но и programmatic transactions через TransactionTemplate, а в reactive-мире — через TransactionalOperator. Там можно явно задать isolation, timeout и руками вызвать setRollbackOnly().  ￼

Когда programmatic approach лучше:
— сложная ветвящаяся логика,
— динамический выбор параметров транзакции,
— нужно точно контролировать границы маленьких критических секций,
— нужно обойти AOP/proxy-ограничения,
— нужно очень явно показать transactional flow в коде.  ￼

Для senior/architect это важный принцип:
аннотации — удобны, programmatic control — точнее.
На сложных местах точность важнее красоты.  ￼

⸻

12. Параллельные транзакции в Spring: как делать правильно

Если вопрос именно “как в Spring делать параллельные транзакции”, то ответ такой:
не пытайся делить одну imperative transaction между несколькими потоками. Spring transaction context привязан к текущему потоку. Новый поток — новый контекст.  ￼

Что делать правильно
	1.	Каждый параллельный worker должен иметь свой собственный transactional boundary.
	2.	Обычно это значит: отдельный @Transactional public method на другом bean, вызываемый worker’ом.
	3.	Если есть fan-out задач — каждая задача делает свою локальную транзакцию.
	4.	Глобальная согласованность между задачами уже достигается не shared transaction, а coordination pattern’ом: barrier, saga, state machine, outbox, idempotent merge.

Это уже не ограничение Spring, а фундамент распределенных систем.  ￼

Что нельзя делать

— считать, что @Async “унаследует” транзакцию,
— открывать долгую транзакцию и в ее рамках распараллеливать БД-операции,
— делать внешние network calls внутри долгой БД-транзакции без веской причины.

Это ведет к долгим lock’ам, contention, pool exhaustion и неочевидным rollback semantics. Эти риски логически следуют из thread-bound model Spring и resource-holding nature транзакций.  ￼

⸻

13. Optimistic vs Pessimistic locking

Hibernate user guide формулирует очень четко:
optimistic locking предполагает, что конфликтов мало, и проверка идет перед commit;
pessimistic locking предполагает, что конфликты вероятны, и данные надо блокировать после чтения до конца использования.  ￼

Optimistic locking

Обычно реализуется через @Version. При обновлении ORM проверяет, не изменил ли кто-то запись с момента чтения. Если изменил — получаешь optimistic lock conflict. Hibernate пишет, что этот подход хорошо масштабируется и особенно подходит для read-often / write-sometimes workloads.  ￼

Плюсы:
— высокая масштабируемость,
— меньше блокировок,
— отлично для web/request-response систем с короткими interaction windows.

Минусы:
— нужен retry/merge strategy,
— при высоком конфликте начинает “сыпаться” большим числом повторов.  ￼

Pessimistic locking

Обычно это SELECT ... FOR UPDATE или JPA LockModeType.PESSIMISTIC_WRITE / PESSIMISTIC_READ. Подходит там, где конфликт не просто возможен, а вероятен и дорог. Hibernate говорит, что pessimistic locking держит ресурс заблокированным после чтения до завершения использования.  ￼

Плюсы:
— меньше логических конфликтов “на commit”,
— полезно для inventory, allocation, уникальных слотов, денежных резервирований.

Минусы:
— lock contention,
— deadlocks,
— падает throughput.  ￼

Архитектурное правило выбора

Если конфликт редок и важен throughput — optimistic.
Если конфликт частый и ошибка недопустима — pessimistic.
Если ты не знаешь, что выбрать, обычно стартуют с optimistic + retry. Это инженерная эвристика на основе описанных моделей блокировок.  ￼

⸻

14. Retry: где он обязателен, а где опасен

Retry — это не “украшение”, а фундаментальный механизм для работы с:
— optimistic lock conflicts,
— serialization failures,
— transient network issues,
— deadlock victims,
— временные проблемы брокера/БД.
Но retry допустим только для идемпотентных или контролируемо повторяемых операций. Это уже архитектурное правило. Часто именно отсутствие продуманного retry превращает хорошую транзакционную модель в нестабильную систему.

В мире Spring сегодня есть два пути

1. Spring Retry
Проект spring-retry официально помечен как maintenance only и в README прямо сказано, что он superseded by Spring Framework 7 resilience features. Но он по-прежнему дает declarative retry через @EnableRetry, @Retryable, @Recover, а по умолчанию в declarative example retry делает до 3 попыток.  ￼

2. Retry в Spring Framework 7 core
Spring Framework 7 имеет свой org.springframework.core.retry.RetryTemplate. В его javadoc сказано, что по умолчанию retryable operation выполняется один раз и затем может быть повторена максимум 3 раза с fixed backoff 1 секунда.  ￼

Как мыслить о retry правильно

Retry должен быть:
— ограниченным,
— с backoff,
— желательно с jitter,
— с классификацией исключений: что retryable, что нет,
— с идемпотентностью или deduplication.

Нельзя бездумно ретраить:
— бизнес-ошибки валидации,
— несогласованные side effects,
— уже выполненные неидемпотентные внешние вызовы без idempotency key.

Это особенно критично в distributed flows.

⸻

15. Deadlocks, serialization failures и почему retry часто лучше, чем “сильнее lock”

При высоком уровне конкуренции часть ошибок — это нормальное явление, а не “сломалась БД”. Например:
— deadlock victim,
— optimistic locking conflict,
— serialization failure на SERIALIZABLE.

Для таких случаев обычно правильнее проектировать короткие транзакции + retry, чем пытаться сделать все еще более тяжелыми блокировками. PostgreSQL clearly distinguishes serialization anomalies as отдельный класс проблем на уровнях weaker than serializable. Hibernate отдельно показывает роль optimistic locking как механизма обнаружения конфликтов перед commit.  ￼

⸻

16. XA: что это вообще такое

XA — это стандартный интерфейсный мир для координации глобальной транзакции между несколькими resource managers через transaction manager. Jakarta Transactions описывает, что application server/enlistment logic сообщает transaction manager’у о participating resources, а тот уже проводит two-phase commit protocol между transaction manager и resource managers, как определено X/Open XA.  ￼

Проще:
есть Transaction Manager (TM),
есть несколько Resource Managers (RM) — например, две XA-capable БД,
и TM координирует их общий commit/rollback.  ￼

⸻

17. XA architecture — нормальная ментальная модель

У XA есть несколько сущностей:

Transaction Manager (TM)
Координатор. Решает begin / prepare / commit / rollback.  ￼

Resource Manager (RM)
Сама система хранения/очередь, умеющая XA: БД, JMS provider и т.д.  ￼

XAResource
Интерфейс между TM и RM. Jakarta Transactions прямо описывает XAResource.start, enlist/delist и группировку ресурсов через isSameRM.  ￼

Global transaction / branch
Одна глобальная транзакция может иметь несколько ветвей по разным RM. Если isSameRM показывает, что это один и тот же resource manager, TM может оптимизировать координацию.  ￼

⸻

18. 2PC — двухфазный коммит по шагам

Фаза 1: Prepare / Voting

TM спрашивает у всех участников: “сможешь ли ты закоммитить?”
Каждый RM делает все нужные проверки, подготавливает изменения, гарантирует durability prepared state и отвечает “yes” или “no”. Jakarta Transactions говорит, что перед start of two-phase commit вызывается beforeCompletion, а на commit time resource managers уведомляются prepare/commit/rollback согласно 2PC.  ￼

Фаза 2: Commit / Rollback

Если все сказали “yes” — TM шлет commit всем.
Если хоть кто-то сказал “no” — TM шлет rollback всем.  ￼

Почему 2PC дорогой

Потому что между prepare и final decision участники могут находиться в подвешенном состоянии, держать ресурсы, логи, блокировки и ждать решения координатора. При сбоях координатора возможны in-doubt / heuristic scenarios. Это фундаментальный trade-off 2PC. Narayana как раз и существует как production-grade transaction technology для такого класса задач.  ￼

⸻

19. One-phase commit optimization

Если в глобальной транзакции фактически участвует только один реальный RM, transaction manager может использовать упрощенный one-phase путь. Jakarta Transactions прямо упоминает “one-phase optimized case” рядом с prepare/completion. Это важная оптимизация.  ￼

⸻

20. Какие способы реализовать distributed transactions вообще существуют

Вот самая важная архитекторская классификация.

Подход A. Локальная транзакция одного ресурса

Обычный @Transactional + одна БД.
Самый надежный и дешевый путь. Всегда сначала пытайся свести use case к этому.  ￼

Подход B. XA / JTA / 2PC

Используется, когда нужно именно atomic commit между несколькими XA-capable ресурсами.
Например: две XA БД или XA БД + JMS. Jakarta EE platform требует поддержку multiple XA-capable resource adapters и full 2PC support в таком сценарии.  ￼

Подход C. Best-effort 1PC / chaining local transactions

Исторически встречается как “сделаем две локальные транзакции подряд и надеемся”.
Это не настоящий distributed atomic commit. Использовать можно только когда бизнес готов к рассогласованию и есть reconcile/compensation. Это уже архитектурная оценка риска.

Подход D. Outbox / Inbox

Записываешь изменение доменных данных и событие в одну локальную транзакцию в одной БД, а потом отдельный процесс публикует событие. Это сегодня один из самых практичных паттернов для микросервисов. Он не дает global atomicity “прямо сейчас”, но дает надежную eventual consistency.

Подход E. Saga / Process Manager

Разбиваешь бизнес-операцию на шаги с компенсациями. Подходит, когда XA невозможен или слишком дорог, а операция реально распределенная между сервисами.

Подход F. TCC / reservation-based protocols

Try-Confirm-Cancel и похожие схемы. Более сложные, но иногда лучше для финансовых/ресурсных сценариев.

Для senior-архитектора главный вопрос не “знаю ли я XA”, а “умею ли я выбрать между XA, Outbox и Saga”.

⸻

21. Когда XA оправдан

XA оправдан, когда одновременно выполняются условия:
— нужны строгие atomic guarantees между несколькими ресурсами,
— ресурсы XA-capable,
— latency/throughput penalty приемлемы,
— операционная команда готова жить с такой сложностью,
— failure semantics действительно требуют atomic all-or-nothing.  ￼

Типичный пример: legacy enterprise-system, monolith или integration-heavy system внутри одного controlled environment, где надо atomically сохранить в 2 XA systems.  ￼

⸻

22. Когда XA почти всегда плохая идея

— микросервисы через HTTP/gRPC,
— cloud-native managed services без полноценной XA поддержки,
— Kafka/Rabbit/NoSQL + REST side effects,
— high throughput / low latency event-driven systems,
— длительные бизнес-процессы с человеческими задержками.

В таких случаях лучше идти в Outbox + idempotency + Saga/compensation, а не тащить XA туда, где он по природе плохо ложится.

⸻

23. Spring + XA/JTA: как это выглядит концептуально

Spring поддерживает JTA через JtaTransactionManager. А thread-bound synchronization model у Spring при этом все равно остается, просто реальным координатором становится transaction manager глобального уровня. TransactionSynchronizationManager javadoc прямо перечисляет JtaTransactionManager как стандартный Spring transaction manager.  ￼

Исторически Spring Boot имел встроенные сценарии для JTA-менеджеров вроде Atomikos, Bitronix, Narayana. В старых Boot docs явно указаны Atomikos/Bitronix/Narayana starters. Для современного проектирования важнее понимать не конкретный starter, а сам принцип:
Spring может быть фронтом над JTA, а XA-ресурсы будут enlist’иться в глобальную транзакцию.  ￼

⸻

24. Narayana, Atomikos, Bitronix — что о них знать

Narayana — один из классических open-source transaction managers уровня enterprise; его docs описывают использование transaction technology для бизнес-приложений.  ￼

Bitronix исторически использовался, но сегодня чаще рассматривается как legacy path.
Atomikos тоже известен, но выбор конкретного JTA manager сегодня уже сильно зависит от экосистемы и поддержки.

Архитектору надо помнить:
выбор TM — это не просто dependency. Это операционная, failure-handling и observability платформа для глобальных транзакций.

⸻

25. Почему 2PC не решает “все распределенные транзакции”

2PC решает atomic commit между участниками XA-протокола. Но он не решает:
— долгие бизнес-процессы,
— человеческие задержки,
— внешние REST API,
— сервисы без XA,
— side effects вне транзакционного домена.

То есть 2PC — это решение для координации ресурсов, а не для всей problem space distributed business workflows.

⸻

26. @TransactionalEventListener: очень важный практический инструмент

Если надо выполнить действие только после успешного commit, в Spring есть @TransactionalEventListener. Документация говорит, что такой listener по умолчанию привязывается к commit phase транзакции.  ￼

Это невероятно полезно для:
— отправки доменных событий после commit,
— пост-коммит логики,
— безопасного разделения “изменил БД” и “реагируй на уже committed state”.

Но это не distributed transaction. Это просто правильная локальная post-commit orchestration.  ￼

⸻

27. Практика проектирования коротких транзакций

Senior-level правило:
транзакция должна быть как можно короче, а ее критическая секция — как можно уже.

Не надо:
— читать из БД,
— идти в REST,
— ждать брокер,
— делать тяжелые вычисления,
— потом возвращаться и commit.

Чем дольше транзакция живет, тем выше:
— lock contention,
— deadlocks,
— pool pressure,
— вероятность rollback,
— latency хвостов.

Это особенно важно при REQUIRES_NEW, потому что Spring docs отдельно предупреждают про дополнительные соединения и риск pool exhaustion.  ￼

⸻

28. Денежные операции, inventory, счетчики: какая стратегия обычно лучше

Денежные операции

Часто:
— локальная БД-транзакция,
— строгие инварианты,
— иногда pessimistic lock или SERIALIZABLE,
— обязательно idempotency key на внешних командах,
— distributed part через saga/outbox, а не XA, если это микросервисы.

Inventory / seats / slots

Часто:
— pessimistic locking для short critical reserve,
— либо atomic update with version / conditional update,
— timeout reservation model,
— retry при конфликте.

Счетчики / агрегаты

Часто:
— optimistic locking или atomic SQL update,
— careful retry.

Выбор зависит от конфликта, а не от вкуса команды.

⸻

29. Что нужно знать про checked exceptions и rollback rules

По умолчанию Spring откатывает по RuntimeException и Error, а checked exceptions — нет. Это официальная дефолтная семантика @Transactional.  ￼

Следствие:
если у тебя бизнес-ошибка оформлена как checked exception, а ты ожидаешь rollback — надо явно указать rollbackFor = ....
Это одна из самых дорогих по времени production-ошибок, потому что код визуально “падает”, а данные уже закоммичены.  ￼

⸻

30. Тестирование транзакционного поведения

Нельзя считать, что “если метод annotated, значит все ок”. Надо тестировать:
— commit path,
— rollback path,
— self-invocation edge cases,
— propagation behavior,
— conflict/retry cases,
— deadlock/lock timeout handling.

Spring testing docs отдельно напоминают, что transaction state привязан к thread-local, и переход в другой поток ломает test-managed transaction expectations. Это полезное напоминание и для production design.  ￼

⸻

31. Reactive transactions: отдельный мир

Spring docs показывают, что для reactive-style есть отдельный TransactionalOperator, а не просто перенос imperative semantics один в один. Это означает:
reactive transaction context — не то же самое, что обычный thread-bound imperative context; там важен контекст Reactor/Publisher pipeline. Spring отдельно документирует reactive transactional methods и TransactionalOperator.  ￼

Архитектурный вывод:
не смешивай в голове imperative @Transactional и reactive transactional model как будто это одно и то же.

⸻

32. Как я бы разложил все решение по уровням зрелости

Уровень 1. Junior/Middle

“Я знаю @Transactional, propagation и isolation.”

Уровень 2. Strong Middle/Senior

“Я понимаю proxy/self-invocation, thread-bound semantics, locking strategies, retries, deadlocks, rollback rules.”

Уровень 3. Senior+/Architect

“Я умею выбрать между local transaction, XA/2PC, outbox, saga, idempotency, pessimistic/optimistic control и понимаю стоимость каждой модели.”

Именно третий уровень реально нужен для распределенных систем.

⸻

33. Что важно запомнить как «архитекторские истины»
	1.	@Transactional — это не distributed consistency solution, а локальная transaction abstraction.  ￼
	2.	В Spring imperative transaction context обычно thread-bound. Параллелизм требует отдельных транзакционных границ.  ￼
	3.	REQUIRED — это joining existing tx; REQUIRES_NEW — новый physical tx; NESTED — savepoint, а не независимая транзакция.  ￼
	4.	Self-invocation ломает ожидания @Transactional в proxy mode.  ￼
	5.	Isolation — это договор с конкретной БД, а не абстрактная теория. Например, PostgreSQL имеет свои особенности: READ UNCOMMITTED = READ COMMITTED, а REPEATABLE READ сильнее стандарта по phantom reads.  ￼
	6.	Optimistic locking хорош для scale; pessimistic — для дорогих частых конфликтов.  ￼
	7.	Retry нужен почти всегда, но только там, где операция безопасно повторяема. Современный Spring уже двигается в сторону core resilience features, а Spring Retry переведен в maintenance only.  ￼
	8.	XA/2PC нужен редко, дорого стоит, но знать его нужно обязательно. Он применим только там, где реально есть XA-capable ресурсы и нужна строгая atomicity между ними.  ￼

⸻

34. Моя практическая рекомендация по порядку изучения

Изучай тему в таком порядке:
	1.	Локальные транзакции в Spring: @Transactional, rollback, propagation, isolation.
	2.	Thread-bound model, proxies, self-invocation, programmatic transactions.
	3.	Locking: optimistic/pessimistic, savepoints, deadlocks, retries.
	4.	Транзакции под высокой конкуренцией: short tx, connection pools, contention.
	5.	XA/JTA/2PC.
	6.	После этого — современные distributed patterns: outbox, saga, idempotency, compensation.

Это самый правильный путь, потому что без локальной модели distributed model обычно понимают неправильно.

⸻

35. Итог в одной фразе

Senior-level понимание distributed transactions в Spring — это не знание одной аннотации @Transactional, а умение проектировать границы согласованности между потоками, БД, сервисами и retry/locking-механизмами, понимая, когда нужен XA/2PC, а когда он только ухудшит систему.  ￼

Могу следующим сообщением сделать тебе еще вторую часть конспекта: уже в формате “шпаргалка + практические примеры кода на Spring Boot/JPA/JTA/XA/Retry/locks”.

# JPA

- **JPA (Java Persistence API)** - стандарт Java EE для объектно-реляционного отображения (ORM). Определяет набор
  аннотаций и интерфейсов (`jakarta.persistence.*`), которые позволяют описывать маппинг сущностей (POJO-классов)
  на таблицы базы данных и управлять ими через `EntityManager`.

## Конфигурирование

`pom.xml`:

```xml

<dependency>
    <groupId>jakarta.persistence</groupId>
    <artifactId>jakarta.persistence-api</artifactId>
    <version>3.1.0</version>
</dependency>

<dependency>
<groupId>org.hibernate.orm</groupId>
<artifactId>hibernate-core</artifactId>
<version>6.0.0.Final</version>
</dependency>

<dependency>
<groupId>org.postgresql</groupId>
<artifactId>postgresql</artifactId>
<version>42.3.1</version>
</dependency>
```

`src/main/resources/META-INF/persistence.xml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<persistence xmlns="https://jakarta.ee/xml/ns/persistence"
             xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
             xsi:schemaLocation="https://jakarta.ee/xml/ns/persistence
                                 https://jakarta.ee/xml/ns/persistence/persistence_3_1.xsd"
             version="3.1">

    <!-- Идентификатор persistence-unit -->
    <persistence-unit name="myPersistenceUnit" transaction-type="RESOURCE_LOCAL">
        <!-- Провайдер JPA (Hibernate) -->
        <provider>org.hibernate.jpa.HibernatePersistenceProvider</provider>

        <!-- Список управляемых сущностей -->
        <!-- Можно не указывать, тогда Hibernate сам отсканирует аннотированные классы -->
        <!--<class>com.example.domain.User</class>-->
        <!--<class>com.example.domain.Order</class>-->

        <!-- JDBC-соединение -->
        <properties>
            <!-- Драйвер -->
            <property name="jakarta.persistence.jdbc.driver" value="org.postgresql.Driver"/>
            <!-- URL -->
            <property name="jakarta.persistence.jdbc.url" value="jdbc:postgresql://localhost:5432/mydb"/>
            <!-- Пользователь/пароль -->
            <property name="jakarta.persistence.jdbc.user" value="myuser"/>
            <property name="jakarta.persistence.jdbc.password" value="mypassword"/>

            <!-- Dialect Hibernate -->
            <property name="hibernate.dialect" value="org.hibernate.dialect.PostgreSQLDialect"/>

            <!-- Формирование DDL (validate | update | create | create-drop) -->
            <property name="hibernate.hbm2ddl.auto" value="update"/>

            <!-- Логирование SQL -->
            <property name="hibernate.show_sql" value="true"/>
            <property name="hibernate.format_sql" value="true"/>

            <!-- Вторичный кэш (опционально) -->
            <property name="hibernate.cache.use_second_level_cache" value="true"/>
            <property name="hibernate.cache.region.factory_class"
                      value="org.hibernate.cache.jcache.JCacheRegionFactory"/>
            <property name="hibernate.javax.cache.provider"
                      value="org.ehcache.jsr107.EhcacheCachingProvider"/>
            <property name="hibernate.javax.cache.uri"
                      value="ehcache.xml"/>

            <!-- (Опционально) Пул соединений HikariCP -->
            <property name="hibernate.connection.provider_class"
                      value="com.zaxxer.hikari.hibernate.HikariConnectionProvider"/>
            <property name="hibernate.hikari.maximumPoolSize" value="10"/>
            <property name="hibernate.hikari.minimumIdle" value="2"/>
            <property name="hibernate.hikari.idleTimeout" value="30000"/>
        </properties>
    </persistence-unit>
</persistence>
```

## JPA Аннотации классов (Class-Level Annotations)

- `@Entity` Помечает класс как сущность, т.е. объект, который может быть сохранён в базе данных. Обязателен.

**Обязательные требования:**

- Класс должен быть public (не private или final).
- Должен иметь конструктор без аргументов (public или protected).
- Не должен быть final.
- Поля должны быть доступны через геттеры/сеттеры.

- `@Table` Описывает соответствующую таблицу в базе. Атрибуты:
    - `name` имя таблицы. Если не указано, JPA использует имя класса.
    - `schema` (_**опционально**_) схема БД.
    - `uniqueConstraints` (_**опционально**_) рантайме они ничего не делают (это инструкция для генерации констеинтов),
      но если констреинт создан уже в бд, то они ничего не делают
    - `indexes` (_**опционально**_) рантайме они ничего не делают (это инструкция для генерации схемы), но если схема
      создана уже в бд, то они ничего не делают | `name` - имя индекса в БД, `columnList` - колонки, по которым строится
      индекс (через запятую, если составной).

- `@Inheritance` (_**опционально**_) Стратегия наследования (для иерархий сущностей). Атрибуты:
    - `strategy` Значения:
        - `InheritanceType.SINGLE_TABLE` (по умолчанию).
        - `InheritanceType.JOINED`
        - `InheritanceType.TABLE_PER_CLASS`


- `@Cacheable` (_**опционально**_) Помечает сущность как кэшируемую вторичным уровнем (JPA-спецификация).


- `@Embeddable` для встраиваемых value-объектов (не сущность)

## JPA Аннотации полей (Field-Level Annotations)

`@Id` Единственная обязательная аннотация в любой сущности (указывает первичный ключ)

`@GeneratedValue`

`(GenerationType.AUTO0)` дать ORM провайдеру самому решить, что лучше для этой БД (На PostgreSQL Hibernate обычно
выберет
SEQUENCE)

`(GenerationType.IDENTITY)` id генерится только при реальном INSERT в БД, ORM не знает id на момент вставки

**Как это работает:**

- При INSERT Hibernate не передаёт id - колонка сама генерит значение.
- После вставки Hibernate делает:
- либо SELECT LAST_INSERT_ID() / SELECT @@IDENTITY,
- либо использует JDBC API getGeneratedKeys() и узнаёт сгенерированный id.

**Особенности:**

- id генерится только при реальном INSERT в БД.
- Hibernate не может сделать батч-вставку нормально с IDENTITY (у него нет id заранее).
- Нельзя “провернуть” вставку без доступа к БД.

**Плюсы:**

- Простейшая настройка, идеально для MySQL.
- Всё делает база, надёжно и понятно.

**Минусы:**

- Плохая поддержка batch insert.
- id появляется только после INSERT -> ORM сложнее оптимизировать работу.

`(GenerationType.SEQUENCE)` ORM заранее запрашивает пачку ID, раздать их объектам ещё до INSERT

**Как это работает:**

- Есть отдельный объект БД - sequence, который умеет говорить:
- nextval('user_seq') -> 1, 2, 3, 4..
- Hibernate может:
- заранее запросить пачку ID,
- раздать их объектам ещё до INSERT.

**Про allocationSize:**

- allocationSize = 50 значит: Hibernate один раз дернёт sequence (получит “блок”) и сам выдает 50 id в памяти.
- Это сильно уменьшает число обращений к sequence.
- Но важно: реальная последовательность в БД тоже должна быть согласована (шаги, границы), чтобы не было дыр/конфликтов.

**Плюсы:**

- Отлично работает с batch insert: id известно до вставки.
- Гибкая оптимизация (pooled, hi/lo).
- Правильный и рекомендуемый способ для PostgreSQL.

**Минусы:**

- Нужна поддержка sequence в БД.
- Нужно аккуратно настроить allocationSize, чтобы не встрять.

```sql
CREATE SEQUENCE user_seq START 1 INCREMENT 1;
CREATE TABLE users
(
    id   BIGINT PRIMARY KEY DEFAULT nextval('user_seq'),
    name TEXT
);
```

```java
@Entity
@SequenceGenerator(
    name = "user_seq_gen",
    sequenceName = "user_seq",
    allocationSize = 50
)
class User {
    @Id @GeneratedValue(strategy = GenerationType.SEQUENCE, generator = "user_seq_gen")
    private Long id;
}
```

`(GenerationType.TABLE)` самый костыльный способ, когда БД не умеет ни identity, ни sequence. Идея: симулируем
sequence через обычную таблицу

**Как это работает:**

- Перед вставкой Hibernate:
- делает SELECT next_val FROM id_generator WHERE id_name = 'user' FOR UPDATE;
- увеличивает next_val;
- использует старое значение как id или блок значений (с учётом allocationSize).
- Таблица id_generator играет роль псевдо-sequence.

**Плюсы:**

- Работает на любой нормальной SQL-БД.
- Не требует identity/sequence.

**Минусы:**

- Медленнее: каждый блок id -> отдельная запись в таблице, блокировка.
- Может быть узким местом по конкурентности.
- Сейчас почти не актуально, т.к. почти все БД умеют identity/sequence.

```sql
CREATE TABLE id_generator
(
    id_name  VARCHAR(50) PRIMARY KEY,
    next_val BIGINT
);
INSERT INTO id_generator (id_name, next_val)
VALUES ('user', 1);
```

```java
@Entity
@TableGenerator(
    name = "user_table_gen",
    table = "id_generator",
    pkColumnName = "id_name",
    valueColumnName = "next_val",
    pkColumnValue = "user",
    allocationSize = 50
)
class User {
    @Id @GeneratedValue(strategy = GenerationType.TABLE, generator = "user_table_gen")
    private Long id;
}
```

- `@Column` Основная аннотация для маппинга поля на столбец. Атрибуты:
    - `name` String Имя столбца в БД (если отличается от имени поля)
    - `nullable` boolean Может ли поле быть NULL (по умолчанию true)
    - `unique` boolean Должно ли поле быть уникальным (аналог UNIQUE в SQL)
    - `length` int Максимальная длина для строк (аналог VARCHAR(255))
    - `precision` int Общее количество цифр (для DECIMAL)
    - `scale` int Количество цифр после запятой (для DECIMAL)
    - `insertable` boolean Участвует ли поле в INSERT (по умолчанию true)
    - `updatable` boolean Участвует ли поле в UPDATE (по умолчанию true)
    - `columnDefinition` String Позволяет задать точный SQL-тип столбца
- `@Basic` Указывает, что поле - обычный примитив/строка/обёртка. Атрибуты:
    - `fetch` EAGER (по умолчанию) или LAZY (для больших типов, @Lob).
    - `optional` то же, что nullable (для примитивов всегда false).
- `@Lob` Помечает поле как CLOB или BLOB. Работает с String, Clob, byte[], Blob
- `@Temporal` Для java.util.Date или java.util.Calendar:
    - `TemporalType.DATE` (только дата).
    - `TemporalType.TIME` (только время).
    - `TemporalType.TIMESTAMP` (дата+время).
- `@Enumerated`Для enum-полей. По умолчанию ORDINAL, но рекомендуется:
    - `EnumType.STRING` - хранить строковое значение.
- `@Transient` Поле не будет маппиться на БД.

## JPA Связи

- `@JoinColumn` используется на стороне отношения, которая хранит внешний ключ (FK)
    - `name` Имя колонки в текущей таблице, которая хранит FK
    - `nullable, unique` ограничения в DDL.
    - `updatable/insertable` если столбец заполняется триггером/вьюхой и т.п.
    - `referencedColumnName` столбец в целевой таблице (обычно id).
    - `foreignKey = @ForeignKey(name = "...")` имя констрейнта.

---

`@ManyToOne/@OneToMany`

- `mappedBy`
- `cascade`
- `orphanRemoval`

FK (внешний ключ) хранит сторона @ManyToOne -- она владельческая.
Рекомендованная модель: двусторонняя связь: Child (владелец) <-> Parent (обратная, mappedBy).

бд:

```text
parent(id PK, ...)
child(id PK, parent_id FK -> parent.id NOT NULL, ...)
/* индекс нужен: */ INDEX(child.parent_id)
```

Базовая (рекомендованная) двусторонняя связь:

```java
// Parent - обратная сторона, коллекция детей
@Entity
class Parent {
  @Id @GeneratedValue Long id;

  @OneToMany(
      mappedBy = "parent",       // FK в Child
      cascade = CascadeType.ALL, // сохраняем/обновляем через Parent
      orphanRemoval = true,      // удаление ребёнка при исключении из коллекции
      fetch = FetchType.LAZY
  )
  private List<Child> children = new ArrayList<>();

  // Хелперы: всегда синхронизируют обе стороны!
  public void addChild(Child c) {
    children.add(c);
    c.setParent(this);
  }
  public void removeChild(Child c) {
    children.remove(c);
    c.setParent(null);
  }
}

// Child - владельческая сторона, FK здесь
@Entity
class Child {
  @Id @GeneratedValue Long id;

  @ManyToOne(fetch = FetchType.LAZY, optional = false)
  @JoinColumn(name = "parent_id", nullable = false, foreignKey = @ForeignKey(name = "fk_child_parent"))
  private Parent parent;
}

```

Нет лишней join-table; простая и быстрая схема.
orphanRemoval = true + хелперы = безопасное удаление «детей» из БД при удалении из коллекции.
LAZY минимизирует «жирные» запросы.
если бы было бы Односторонне (@OneToMany без mappedBy) -- JPA создаст join-table. Обычно не нужно.

ОШИБКИ:

Создалась лишняя join-table
Причина: односторонний @OneToMany без mappedBy.
Фикс: сделайте связь двусторонней (mappedBy = "parent" у Parent и FK в Child).
Либо (Hibernate-only) используйте @JoinColumn на @OneToMany.

Дети не сохраняются/обновляются вместе с родителем
Причина: нет каскадов PERSIST/MERGE.
Фикс: cascade = { PERSIST, MERGE } (или ALL, если вы уверены).
Лайфхак: если сохраняете через Parent, добавляйте детей только через addChild.

N+1 запросов
Фикс: используйте fetch join / @EntityGraph / @BatchSize.

equals/hashCode, toString и коллекции
Не включайте ленивые коллекции в toString - получите рекурсию/тонны SQL.
Для equals/hashCode: безопаснее по уникальному бизнес-ключу или по id (только после присвоения).
Set + неустойчивый equals до присвоения id = загадочные дубликаты. Проще начинать с List.

---

`@OneToOne`

- `name` имя FK-колонки у владельца.
- `fetch` ТИП: `FetchType`. Загрузка связанной сущности: `EAGER` (по умолчанию) или `LAZY`.
- `mappedBy` Имя поля во владельческой сущности (у кого FK). Используется на обратной стороне. Если задано, эта сторона
  не владеет FK.
- `cascade` ТИП: `CascadeType`. Массовые операции: `PERSIST`, `MERGE`, `REMOVE`, `REFRESH`, `DETACH`, `ALL`. Позволяет
  прокинуть операции из родителя на зависимую сущность.
- `optional` Если `false`, связь обязательна: в DDL для FK будет `NOT NULL`. Синхронизировать с
  `@JoinColumn(nullable = false)`.
- `orphanRemoval` Если `true`, при снятии связи (присвоении `null`) зависимая сущность удалится из БД автоматически.

Владелец - где FK (там ставим @JoinColumn).
Обратная сторона (inverse) - та, что через mappedBy ссылается на владеющую
Делайте LAZY.
Когда ставить orphanRemoval = true? Когда жизнь зависимой сущности полностью подчинена родителю.

```java
@Entity
class Order {
  @OneToOne(cascade = CascadeType.ALL, fetch = FetchType.LAZY)
  @JoinColumn(name = "invoice_id", unique = true, nullable = false)
  private Invoice invoice;
}

@Entity
class Invoice {
  @OneToOne(mappedBy = "invoice", fetch = FetchType.LAZY)
  private Order order;
}
```

---

`@ManyToMany`

- `name` имя FK-колонки у владельца.
- `fetch`

Используется редко, когда между сущностями нет своей таблицы-сущности.
Нужна join-table с двумя FKs. Одна сторона - владелец (без mappedBy).

бд:

```text
a(id PK, ...)
b(id PK, ...)
    a_b(
    a_id FK -> a.id  NOT NULL,
    b_id FK -> b.id  NOT NULL,
    PRIMARY KEY (a_id, b_id)
)
CREATE INDEX idx_a_b__a  ON a_b(a_id);
CREATE INDEX idx_a_b__b  ON a_b(b_id);
```

Базовая двусторонняя модель (рекомендуется):

```java
@Entity
class A {
  @Id @GeneratedValue Long id;

  // Владеющая сторона (нет mappedBy) - она пишет в a_b
  @ManyToMany
  @JoinTable(
    name = "a_b",
    joinColumns        = @JoinColumn(name = "a_id", foreignKey = @ForeignKey(name="fk_ab_a")),
    inverseJoinColumns = @JoinColumn(name = "b_id", foreignKey = @ForeignKey(name="fk_ab_b"))
  )
  private Set<B> bs = new HashSet<>();

  // хелперы держат обе стороны в синхроне
  public void addB(B b) {
    if (bs.add(b)) b.getAs().add(this);
  }
  public void removeB(B b) {
    if (bs.remove(b)) b.getAs().remove(this);
  }
}

@Entity
class B {
  @Id @GeneratedValue Long id;

  // Обратная сторона
  @ManyToMany(mappedBy = "bs")
  private Set<A> as = new HashSet<>();

  public Set<A> getAs() { return as; }
}
```

Нет лишних сущностей - только join-таблица.
Оба направления навигации есть.
Set предотвращает дубликаты на уровне коллекции (и всё равно держим UNIQUE в БД!).

Односторонняя модель (нормальный вариант, если обратная навигация не нужна):

```java
@Entity
class A {
  @Id @GeneratedValue Long id;

  @ManyToMany
  @JoinTable(
    name = "a_b",
    joinColumns        = @JoinColumn(name = "a_id"),
    inverseJoinColumns = @JoinColumn(name = "b_id")
  )
  private Set<B> bs = new HashSet<>();
}

@Entity
class B {
  @Id @GeneratedValue Long id;
  // нет обратной коллекции
}
```

Проще код, меньше синхронизации в памяти.
Всё равно нужна уникальность пары в БД, индексы и т.д.

Как только у связи появляются свои поля (роль, статус, позиция, даты, комментарий) - не @ManyToMany. Делайте
link-сущность:

```java
@Entity
class AB {
  @EmbeddedId
  private ABId id;

  @ManyToOne(fetch = FetchType.LAZY) @MapsId("aId")
  @JoinColumn(name = "a_id")
  private A a;

  @ManyToOne(fetch = FetchType.LAZY) @MapsId("bId")
  @JoinColumn(name = "b_id")
  private B b;

  // дополнительные поля связи
  private String role;
  private Integer position;
  private LocalDate since;
}

@Embeddable
class ABId implements Serializable {
  private Long aId;
  private Long bId;

  // equals/hashCode по обоим полям
}
```

Плюсы: можно хранить доп. атрибуты, уникальные ключи, порядок, статусы.
Минусы: на один класс больше, но это правильная модель.

---

PERSIST, MERGE - часто на aggregate-границах (родитель -> дети).
REMOVE - только если дочке нельзя жить без родителя (и вы реально хотите физическое удаление).
orphanRemoval = true - при parent.getChildren().remove(x) или child.setParent(null) запись удалится.
Ставьте LAZY везде, где это разумно (особенно @ManyToOne, @OneToOne).
Борьба с N+1: join fetch в JPQL, @EntityGraph, батч-фетч (@BatchSize).
Не ставьте много EAGER - тяжёлые запросы и неожиданные циклические загрузки.
equals/hashCode: не используйте ленивые коллекции; безопасно - по бизнес-ключу или по id (после присвоения).

Где физически FK? -> там владелец и @JoinColumn.
Нужна ли обратная навигация? -> mappedBy на обратной стороне.
Связь обязательна? -> optional=false + nullable=false.
Нужен каскад? -> минимум, только то, что реально нужно.
Ожидаются пачки чтения? -> LAZY + EntityGraph/join fetch.

---

# Persistence Unit

Persistence Unit (PU) - это конфигурация JPA:

- какая БД
- какие сущности
- какие настройки Hibernate
- имя юнита (name="myPu" в persistence.xml или spring.jpa.*)

Из Persistence Unit создаётся:

- EntityManagerFactory
- через него уже создается EntityManager

1 Persistence Unit = 1 БД

# Persistence Context

`@PersistenceContext` - аннотация Jakarta EE, которая говорит контейнеру инжектировать сюда EntityManager, связанный с
Persistence Unit + Это набор управляемых сущностей в памяти + их состояние.

PersistenceContext - это внутренняя структура, которую EntityManager использует,
чтобы хранить и отслеживать все Entity-объекты, находящиеся в состоянии Managed.

Он живёт ровно столько, сколько живёт EntityManager (обычно - одну транзакцию или HTTP-запрос).

- Следит, какие объекты уже загружены из БД.
- Проверяет, изменились ли поля (dirty checking).
- Знает, какие объекты нужно INSERT, UPDATE, DELETE при flush().
- Следит за связями (@OneToMany, @ManyToOne), кэшем и каскадами.
- Отслеживает идентичность - если ты дважды вызовешь em.find(Admin.class, 1L), ты получишь тот же самый объект, а не
  новую копию.

**Состояния сущностей в PersistenceContext**

| Состояние    | Где находится                                                      | Что делает JPA             |
| ------------ | ------------------------------------------------------------------ | -------------------------- |
| **New**      | вне контекста                                                      | объект ещё не сохранён     |
| **Managed**  | внутри PersistenceContext                                          | JPA отслеживает изменения  |
| **Detached** | вышел из контекста (например, после `em.clear()` или `em.close()`) | больше не отслеживается    |
| **Removed**  | помечен на удаление                                                | будет удалён при `flush()` |

# EntityManagerFactory

EntityManagerFactory - фабрика ентити менеджеров

Создаётся один раз на всё приложение при старте (на основе persistence.xml). Она живёт весь runtime
Это тяжёлый объект: он открывает JDBC-пулы, кэширует метаданные, маппинги сущностей, SQL-генерацию и т.п.
Не создаёт соединения к БД сам - это делает EntityManager, который из неё берётся.
Потокобезопасен и общий для всех потоков/запросов.

# EntityManager

Это основной интерфейс JPA, через который приложение общается с базой данных.

Он управляет жизненным циклом Entity сущностей и выполняет операции CRUD, JPQL-запросы, транзакции и
синхронизацию с БД.

На каждый HTTP-запрос (или транзакцию) контейнер создаёт новый экземпляр EntityManager, связанный с одним
PersistenceContext.

Жизненный цикл EntityManager обычно совпадает с жизненным циклом запроса или JTA-транзакции.
Он не потокобезопасен.
После коммита или rollback - автоматически закрывается контейнером.

В Jakarta EE/Spring, EntityManager создаётся контейнером (WildFly, Payara, TomEE...) на основе Persistence Unit,
описанной в persistence.xml

Когда контейнер поднимает приложение:

- читает persistence.xml
- создаёт EntityManagerFactory
- управляет EntityManager для каждого запроса или транзакции.

```java
@PersistenceContext(unitName = "studsPU")
private EntityManager em;
```

Контейнер внедряет управляемый EntityManager из пула.
Этот EntityManager связан с текущим контекстом транзакции (или с @RequestScoped контекстом в CDI).

| Тип                     | Когда используется          | Пример                                                                          |
| ----------------------- | --------------------------- | ------------------------------------------------------------------------------- |
| **Container-managed**   | Jakarta EE, Spring, Quarkus | `@PersistenceContext` - контейнер сам управляет транзакциями                    |
| **Application-managed** | Standalone-приложения       | `EntityManagerFactory emf = ...; EntityManager em = emf.createEntityManager();` |

Пример ручного использования (application-managed)

```java
EntityManagerFactory emf = Persistence.createEntityManagerFactory("studsPU");
EntityManager em = emf.createEntityManager();
em.getTransaction().begin();

Admin a = new Admin();
a.setLogin("root");
em.persist(a);

em.getTransaction().commit();
em.close();
emf.close();
```

## EntityManager CRUD-операции

| Метод                   | Описание                                                       |
| ----------------------- | -------------------------------------------------------------- |
| `persist(entity)`       | Добавить новый объект в контекст (INSERT при commit)           |
| `find(EntityClass, id)` | Найти по PK (SELECT)                                           |
| `merge(entity)`         | Обновить detached объект (UPDATE)                              |
| `remove(entity)`        | Удалить объект (DELETE)                                        |
| `contains(entity)`      | Проверить, управляется ли объект контекстом                    |
| `flush()`               | Синхронизировать контекст с БД (выполнить все накопленные SQL) |
| `clear()`               | Очистить контекст (все entity становятся detached)             |
| `detach(entity)`        | Отсоединить один объект от контекста                           |

Запросы (JPQL, Criteria, Native)

| Метод                                     | Описание                                     |
| ----------------------------------------- | -------------------------------------------- |
| `createQuery(String jpql, Class<T>)`      | Создать JPQL-запрос (объектный SQL)          |
| `createNamedQuery(String name, Class<T>)` | Использовать @NamedQuery из Entity           |
| `createNativeQuery(String sql, Class<T>)` | Выполнить нативный SQL и замаппить на Entity |
| `createNativeQuery(String sql)`           | Нативный SQL без маппинга                    |
| `createQuery(CriteriaQuery<T>)`           | Типобезопасные запросы через Criteria API    |

Транзакции (если не контейнер, а вручную)

| Метод               | Описание                                 |
| ------------------- | ---------------------------------------- |
| `getTransaction()`  | Возвращает объект `EntityTransaction`    |
| `joinTransaction()` | Присоединяет EM к текущей JTA-транзакции |

# EntityManager Lifecycle Callbacks

Это хуки жизненного цикла сущности - точки, в которые JPA/Hibernate сам зайдёт и вызовет метод, когда что-то происходит
с entity.

- `@PrePersist` перед INSERT, Вызывается, когда сущность переходит transient → persistent и готовится к вставке
- `@PostPersist` сразу после INSERT, Вызывается после того, как сущность уже вставлена в БД
- `@PreRemove` перед DELETE, Вызывается, когда сущность помечают на удаление
- `@PostRemove` после DELETE, Срабатывает после удаления
- `@PreUpdate` перед UPDATE, Срабатывает перед тем, как Hibernate собирается обновить сущность
- `@PostUpdate` после UPDATE, Срабатывает после успешного UPDATE в БД
- `@PostLoad` после загрузки из БД, Срабатывает каждый раз, когда ORM загружает сущность из БД

# Criteria API

Это объектная (типобезопасная) альтернатива JPQL.
Даёт возможность строить динамические запросы к базе данных не строками через типобезопасный Java-код в runtime.
Вместо строк `"SELECT v FROM Vehicle v WHERE v.type = :t"` создавать запрос с помощью Java-классов (CriteriaBuilder,
CriteriaQuery, Root и т.д.)

| Проблема с JPQL                                               | Как решает Criteria                     |
| ------------------------------------------------------------- | --------------------------------------- |
| Строки, ошибки видны только в runtime                         | Компиляция проверяет типы и имена полей |
| Сложно строить динамические фильтры (if … then add condition) | Легко добавлять условия программно      |
| Переименование полей ломает запросы                           | IDE-рефакторинг автоматически обновит   |
| Разные поля и условия по разным параметрам                    | Можно строить условия в цикле           |

Основные участники API

| Класс / интерфейс      | Назначение                                                                                         |
| ---------------------- | -------------------------------------------------------------------------------------------------- |
| **`CriteriaBuilder`**  | Точка входа. Создаёт запросы и выражения (`equal`, `like`, `and`, `or`, `sum`, и т.д.).            |
| **`CriteriaQuery<T>`** | Сам запрос. Хранит `select`, `from`, `where`, `orderBy`, `groupBy`, и т.д.                         |
| **`Root<T>`**          | Корневой объект выборки (`FROM Vehicle v`).                                                        |
| **`Predicate`**        | Логическое выражение (условие). Возвращается из `cb.equal()`, `cb.greaterThan()`, `cb.and()` и др. |
| **`Path<T>`**          | Абстракция пути к полю (`root.get("admin").get("login")`).                                         |
| **`Join<X,Y>`**        | JOIN-связи между сущностями (`root.join("admin")`).                                                |
| **`TypedQuery<T>`**    | Выполняет критерий-запрос (`em.createQuery(criteriaQuery)`).                                       |

Архитектура запроса Criteria API

```text
EntityManager
   │
   ├─> getCriteriaBuilder()
   │
   ▼
CriteriaBuilder  ->  CriteriaQuery<Vehicle>
                         │
                         ├── FROM -> Root<Vehicle>
                         ├── WHERE -> Predicate
                         ├── SELECT -> Projection
                         └── ORDER BY / GROUP BY
```

```text
Root<Vehicle> (алияс v)
 ├── Path: v.engine_power
 ├── Path: v.creation_time
 ├── Join admin (алияс a)
 │     └── Path: a.id
 └── Join coordinates (алияс c)
       ├── Path: c.x
       └── Path: c.y
```

- Root<T>
  Это корень запроса - то, что стоит в FROM ... в SQL
  Всегда создаётся через CriteriaQuery.from(EntityClass)
  это частный случай From<T, T> (т.е. Root наследуется от From)

- From<Z, X>
  Это источник строк в запросе (таблица или присоединённая таблица)
  Это “толстый” путь (path), который умеет делать JOIN и хранит уже сделанные JOIN-ы

- Path<X>
  Это любой путь к атрибуту: столбцу, embedded полю и даже к join’у как к узлу
  С Path вы строите предикаты: cb.equal(path, ...), cb.lessThan(path, ...) и т.д.

# JPA

- **JPA (Java Persistence API)** – стандарт Java EE для объектно-реляционного отображения (ORM). Определяет набор
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

    * Класс должен быть public (не private или final).
    * Должен иметь конструктор без аргументов (public или protected).
    * Не должен быть final.
    * Поля должны быть доступны через геттеры/сеттеры.


- `@Table` Описывает соответствующую таблицу в базе. Атрибуты:
    - `name` имя таблицы. Если не указано, JPA использует имя класса.
    - `schema` (_**опционально**_) схема БД.
    - `catalog` (_**опционально**_) каталог.
    - `uniqueConstraints` (_**опционально**_) уникальные ограничения (массив @UniqueConstraint).
    - `indexes` (_**опционально**_) индексы для ускорения запросов, `name` - имя индекса в БД, `columnList` - колонки,
      по которым строится индекс (через запятую, если составной).
      ```
      @Table(
          name = "orders",
          indexes = {
              @Index(name = "idx_order_customer", columnList = "customer_id"),
              @Index(name = "idx_order_status", columnList = "status")
          }
      )
      ```


- `@Inheritance` (_**опционально**_) Стратегия наследования (для иерархий сущностей). Атрибуты:
    - `strategy` Значения:
        - `InheritanceType.SINGLE_TABLE` (по умолчанию).
        - `InheritanceType.JOINED`
        - `InheritanceType.TABLE_PER_CLASS`


- `@Cacheable` (_**опционально**_) Помечает сущность как кэшируемую вторичным уровнем (JPA-спецификация).


- `@Embeddable` для встраиваемых value-объектов (не сущность)

## JPA Аннотации полей (Field-Level Annotations)

- `@Id` единственная обязательная аннотация в любой сущности (указывает первичный ключ).

  Помечает поле как первичный ключ (primary key) сущности.
  Обязательна для любой JPA-сущности. Должно быть непримитивным
  типом (Long, Integer, String, UUID и т.д.). Не должно быть final.

- `@GeneratedValue` Указывает стратегию автоматической генерации значений первичного ключа. Атрибуты:
    - `strategy` – (GenerationType) тип генерации (AUTO, IDENTITY, SEQUENCE, TABLE).
        - `GenerationType.AUTO` Hibernate сам выбирает стратегию, Обычно выбирает SEQUENCE или IDENTITY в зависимости от
          БД, Не рекомендуется для продакшна из-за непредсказуемости
        - `GenerationType.IDENTITY` Использует автоинкремент базы данных (SERIAL), Значение генерируется при вставке
          записи
        - `GenerationType.SEQUENCE` Использует sequence базы данных, требует создания sequence в БД
        - `GenerationType.TABLE` Использует отдельную таблицу для генерации ID
    - `generator` - (String) имя генератора (если используется @SequenceGenerator или @TableGenerator).

    ```
    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    ------------------------------------------------------
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    ------------------------------------------------------
    @Id
    @GeneratedValue(strategy = GenerationType.SEQUENCE, generator = "user_seq")
    @SequenceGenerator(name = "user_seq", sequenceName = "user_id_seq", allocationSize = 1)
    ------------------------------------------------------
    @Id
    @GeneratedValue(strategy = GenerationType.TABLE, generator = "id_gen")
    @TableGenerator(name = "id_gen", table = "id_generator", pkColumnName = "gen_name", valueColumnName = "gen_value")
    ------------------------------------------------------
    @Id
    @GeneratedValue(generator = "UUID")
    @GenericGenerator(name = "UUID", strategy = "org.hibernate.id.UUIDGenerator")
    ```

- `@SequenceGenerator` - Настраивает sequence для `GenerationType.SEQUENCE`
    ```
    @Id
    @GeneratedValue(strategy = GenerationType.SEQUENCE, generator = "user_seq")
    @SequenceGenerator(
        name = "employee_seq",      // Имя генератора (должно совпадать в @GeneratedValue)
        sequenceName = "emp_seq",   // Имя sequence в БД
        allocationSize = 50,        // Сколько значений зарезервировать за раз
        initialValue = 1000         // Начальное значение
    )
    ```

- `@TableGenerator` Настраивает таблицу для `GenerationType.TABLE`
    ```
    @Id
    @GeneratedValue(strategy = GenerationType.TABLE, generator = "id_gen")
    @TableGenerator(
        name = "id_gen",                // Имя генератора
        table = "ID_GENERATOR",         // Имя таблицы
        pkColumnName = "GEN_NAME",      // Колонка с именем генератора
        valueColumnName = "GEN_VALUE",  // Колонка со значением
        allocationSize = 10,            // Шаг увеличения
        initialValue = 100              // Начальное значение
    )
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
- `@Basic` Указывает, что поле — обычный примитив/строка/обёртка. Атрибуты:
    - `fetch` EAGER (по умолчанию) или LAZY (для больших типов, @Lob).
    - `optional` то же, что nullable (для примитивов всегда false).
- `@Lob` Помечает поле как CLOB или BLOB. Работает с String, Clob, byte[], Blob
- `@Temporal` Для java.util.Date или java.util.Calendar:
    - `TemporalType.DATE` (только дата).
    - `TemporalType.TIME` (только время).
    - `TemporalType.TIMESTAMP` (дата+время).
- `@Enumerated`Для enum-полей. По умолчанию ORDINAL, но рекомендуется:
    - `EnumType.STRING` — хранить строковое значение.
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
// Parent — обратная сторона, коллекция детей
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

// Child — владельческая сторона, FK здесь
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
Не включайте ленивые коллекции в toString — получите рекурсию/тонны SQL.
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

Владелец — где FK (там ставим @JoinColumn).
Обратная сторона (inverse) — та, что через mappedBy ссылается на владеющую
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
Нужна join-table с двумя FKs. Одна сторона — владелец (без mappedBy).

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

  // Владеющая сторона (нет mappedBy) — она пишет в a_b
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

Нет лишних сущностей — только join-таблица.
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

Как только у связи появляются свои поля (роль, статус, позиция, даты, комментарий) — не @ManyToMany. Делайте
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

PERSIST, MERGE — часто на aggregate-границах (родитель → дети).
REMOVE — только если дочке нельзя жить без родителя (и вы реально хотите физическое удаление).
orphanRemoval = true — при parent.getChildren().remove(x) или child.setParent(null) запись удалится.
Ставьте LAZY везде, где это разумно (особенно @ManyToOne, @OneToOne).
Борьба с N+1: join fetch в JPQL, @EntityGraph, батч-фетч (@BatchSize).
Не ставьте много EAGER — тяжёлые запросы и неожиданные циклические загрузки.
equals/hashCode: не используйте ленивые коллекции; безопасно — по бизнес-ключу или по id (после присвоения).

Где физически FK? → там владелец и @JoinColumn.
Нужна ли обратная навигация? → mappedBy на обратной стороне.
Связь обязательна? → optional=false + nullable=false.
Нужен каскад? → минимум, только то, что реально нужно.
Ожидаются пачки чтения? → LAZY + EntityGraph/join fetch.

---

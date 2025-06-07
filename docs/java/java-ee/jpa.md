# JPA

## Введение

### Что такое JPA и Hibernate

- **JPA (Java Persistence API)** – стандарт Java EE/Java SE для объектно-реляционного отображения (ORM). Определяет
  набор аннотаций и интерфейсов (`jakarta.persistence.*`), которые позволяют описывать маппинг сущностей (POJO-классов)
  на таблицы базы данных и управлять ими через `EntityManager`.
- **Hibernate** – одна из самых популярных реализаций JPA. Предоставляет:
    - Инструменты для генерации SQL-запросов.
    - Поддержку продвинутых функций (кэш второго уровня, собственный синтаксис `Criteria`, оптимизации загрузки и пр.).
    - Hibernate-specific расширения (аннотации из пакета `org.hibernate.annotations`).

## Подключение зависимостей

### Maven

```xml

<dependencies>
    <!-- JPA API -->
    <dependency>
        <groupId>jakarta.persistence</groupId>
        <artifactId>jakarta.persistence-api</artifactId>
        <version>3.1.0</version>
    </dependency>

    <!-- Hibernate Core (провайдер JPA) -->
    <dependency>
        <groupId>org.hibernate.orm</groupId>
        <artifactId>hibernate-core</artifactId>
        <version>6.0.0.Final</version>
    </dependency>

    <!-- JDBC-драйвер (например, для PostgreSQL) -->
    <dependency>
        <groupId>org.postgresql</groupId>
        <artifactId>postgresql</artifactId>
        <version>42.3.1</version>
    </dependency>

    <!-- (Опционально) HikariCP для пулов соединений -->
    <dependency>
        <groupId>com.zaxxer</groupId>
        <artifactId>HikariCP</artifactId>
        <version>5.0.1</version>
    </dependency>

    <!-- (Опционально) Логирование SQL -->
    <dependency>
        <groupId>org.slf4j</groupId>
        <artifactId>slf4j-simple</artifactId>
        <version>2.0.5</version>
    </dependency>
</dependencies>
```

## Конфигурирование Hibernate

Есть два основных подхода:

- Через persistence.xml (классический для JPA).
- Через hibernate.cfg.xml или Java-конфигурацию (при работе с «чистым» Hibernate).

### persistence.xml (JPA)

Расположите файл `META-INF/persistence.xml` в каталоге `src/main/resources`

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

### hibernate.cfg.xml (Native Hibernate)

Если вы не используете JPA API, а подключаетесь напрямую к Hibernate, необходим `hibernate.cfg.xml`

```xml
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE hibernate-configuration PUBLIC
        "-//Hibernate/Hibernate Configuration DTD 3.0//EN"
        "https://hibernate.org/dtd/hibernate-configuration-3.0.dtd">
<hibernate-configuration>
    <session-factory>
        <!-- JDBC -->
        <property name="hibernate.connection.driver_class">org.postgresql.Driver</property>
        <property name="hibernate.connection.url">jdbc:postgresql://localhost:5432/mydb</property>
        <property name="hibernate.connection.username">myuser</property>
        <property name="hibernate.connection.password">mypassword</property>

        <!-- Dialect -->
        <property name="hibernate.dialect">org.hibernate.dialect.PostgreSQLDialect</property>

        <!-- DDL -->
        <property name="hibernate.hbm2ddl.auto">update</property>

        <!-- Логирование -->
        <property name="hibernate.show_sql">true</property>
        <property name="hibernate.format_sql">true</property>

        <!-- Кэш второго уровня -->
        <property name="hibernate.cache.use_second_level_cache">true</property>
        <property name="hibernate.cache.region.factory_class">
            org.hibernate.cache.jcache.JCacheRegionFactory
        </property>
        <property name="hibernate.javax.cache.uri">ehcache.xml</property>

        <!-- Указываем пакеты/классы -->
        <mapping class="com.example.domain.User"/>
        <mapping class="com.example.domain.Order"/>
        <!-- Или сканируем пакет через аннотации: -->
        <!--<mapping package="com.example.domain"/>-->
    </session-factory>
</hibernate-configuration>
```

## JPA Аннотации классов (Class-Level Annotations)

- `@Entity` Помечает класс как сущность, т.е. объект, который может быть сохранён в базе данных. Обязателен.

  **Обязательные требования:**

    * Класс должен быть public или protected (не private или final).
    * Должен иметь конструктор без аргументов (public или protected).
    * Не должен быть final (Hibernate использует прокси для ленивой загрузки).
    * Поля должны быть доступны через геттеры/сеттеры (или public/protected).

- `@Table` Описывает соответствующую таблицу в базе. Атрибуты:
    - `name` – имя таблицы. Если не указано, JPA использует имя класса (User(class) → таблица user или User в
      зависимости от диалекта)
    - `schema` – схема БД (опционально).
    - `catalog` – каталог (если требуется).
    - `uniqueConstraints` – уникальные ограничения (массив @UniqueConstraint).
    - `indexes`индексы для ускорения запросов, `name` – имя индекса в БД, `columnList` – колонки, по которым строится
      индекс (через запятую, если составной).
      ```
      @Table(
          name = "orders",
          indexes = {
              @Index(name = "idx_order_customer", columnList = "customer_id"),
              @Index(name = "idx_order_status", columnList = "status")
          }
      )
      ```

- `@Inheritance` Стратегия наследования (для иерархий сущностей). Атрибуты:
    - `strategy` Значения:
        - `InheritanceType.SINGLE_TABLE` (по умолчанию).
        - `InheritanceType.JOINED`
        - `InheritanceType.TABLE_PER_CLASS`
- `@Cacheable` Помечает сущность как кэшируемую вторичным уровнем (JPA-спецификация).

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
    ---------
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    ---------
    @Id
    @GeneratedValue(strategy = GenerationType.SEQUENCE, generator = "user_seq")
    @SequenceGenerator(name = "user_seq", sequenceName = "user_id_seq", allocationSize = 1)
    ---------
    @Id
    @GeneratedValue(strategy = GenerationType.TABLE, generator = "id_gen")
    @TableGenerator(name = "id_gen", table = "id_generator", pkColumnName = "gen_name", valueColumnName = "gen_value")
    ---------
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

## JPA Связи (Associations)

- `@JoinColumn` используется на стороне отношения, которая хранит внешний ключ (FK). Чаще всего применяется совместно с
  `@ManyToOne`, `@OneToOne` или на «владельческой» стороне `@OneToMany`.

    * **Указывает имя столбца** в текущей таблице, где хранится внешний ключ.
    * Позволяет гибко настраивать колонки-связки: `nullable`, `unique`, `insertable`, `updatable` и т. д.
    * Если не указать, JPA сама сгенерирует имя: `<имя_поля>_<имя_первичного_ключа_целевой_сущности>`, например,
      `customer_id`.

| Атрибут                | Тип                            | По умолчанию                                   | Описание                                                                                                                                          |
|------------------------|--------------------------------|------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------|
| `name`                 | `String`                       | `""`                                           | Имя колонки в **текущей** таблице, которая хранит FK. Если не указан — JPA сгенерирует как `<имя_поля>_<pk_целевой_сущности>`.                    |
| `referencedColumnName` | `String`                       | `""`                                           | Имя колонки в **целевой** таблице (та, на которую ссылаются). По умолчанию — PK (обычно `id`).                                                    |
| `unique`               | `boolean`                      | `false`                                        | Если `true` — добавляется ограничение `UNIQUE` на уровне БД. Часто используется в `@OneToOne`, когда нужно гарантировать «один к одному» в схеме. |
| `nullable`             | `boolean`                      | `true`                                         | Если `false` — будет `NOT NULL` в DDL. Если связь обязательна, ставьте `nullable = false`.                                                        |
| `insertable`           | `boolean`                      | `true`                                         | Если `false`, колонка **не будет включаться** в `INSERT`. Обычно используют при «рассчитанных» или «чужих» FK, которые заполняются триггерами.    |
| `updatable`            | `boolean`                      | `true`                                         | Если `false`, колонка **не будет включаться** в `UPDATE`. Используется редко (например, когда внешний ключ не должен меняться после создания).    |
| `columnDefinition`     | `String`                       | `""`                                           | Позволяет явно задать SQL -фрагмент для создания колонки (тип, дефолтное значение, комментарий). Пример: `"BIGINT DEFAULT 0"` или `"UUID"`        |
| `table`                | `String`                       | `""`                                           | Имя таблицы, если сущность маппится на несколько таблиц (при использовании `@SecondaryTable`).                                                    |
| `foreignKey`           | `javax.persistence.ForeignKey` | `@ForeignKey(ConstraintMode.PROVIDER_DEFAULT)` | С  JPA 2.1 можно явно задать параметры FK-constraint: имя ограничения, поведение `ON DELETE`/`ON UPDATE`, режим `NO_CONSTRAINT`.                  |

```java
@Entity
@Table(name = "orders")
public class Order {
    // Один заказ связан с одним пользователем (владелец связи — Order)
    @ManyToOne(fetch = FetchType.LAZY, optional = false)
    @JoinColumn(
        name = "customer_id",           // имя колонки в таблице orders
        referencedColumnName = "id",    // ссылается на столбец id из таблицы customers
        nullable = false,               // NOT NULL
        foreignKey = @ForeignKey(        // явно задаём имя FK-constraint
            name = "FK_ORDER_CUSTOMER"
        )
    )
    private Customer customer;
}
```

1. Всегда указывайте `name`  
   Не полагайтесь на автоматически сгенерированные имена.

    ```java
    @JoinColumn(name = "customer_id")
    ```

2. Синхронизируйте `nullable = false` и `optional = false` (в @ManyToOne/@OneToOne)
   Если связь обязательна:

    ```
    @ManyToOne(optional = false)
    @JoinColumn(name = "order_id", nullable = false)
    private Order order;
    ```

3. `referencedColumnName` используют редко, только если PK целевой сущности не называется id.

   ```
   @JoinColumn(name = "cust_uuid", referencedColumnName = "uuid")
   private Customer customer;
   ```


4. `unique = true` уместно в @OneToOne, чтобы гарантировать на уровне БД, что один и тот же FK не встречается дважды.

   ```
   @OneToOne
   @JoinColumn(name = "passport_id", unique = true)
   private Passport passport;
   ```

5. `foreignKey = @ForeignKey(name = "...")` (JPA 2.1+)
   Позволяет задать имя ограничения в схеме:

    ```
    @JoinColumn(
        name = "product_id",
        foreignKey = @ForeignKey(name = "FK_PRODUCT_CATEGORY")
    )
    private Category category;
    ```

6. `insertable = false, updatable = false`
   Применяется, когда колонка заполняется средствами БД (триггеры) или отображает вычисляемый/чужой FK, и вы не хотите,
   чтобы JPA его меняла.

    ```
    @JoinColumn(name = "status_id", insertable = false, updatable = false)
    private Status status;
    ```

---

- `@OneToOne` описывает связь «один к одному» между двумя сущностями. Существует две стороны: Владеющая сторона (
  owning) — та, где хранится FK и ставится @JoinColumn. Обратная сторона (inverse) — та, что через mappedBy ссылается на
  владеющую; FK не создаётся.

| Атрибут         | Тип             | По умолчанию | Описание                                                                                                                                       |
|-----------------|-----------------|--------------|------------------------------------------------------------------------------------------------------------------------------------------------|
| `mappedBy`      | `String`        | `""`         | Имя свойства во «владельческой» сущности. Используется на **обратной** стороне. Если задано, эта сторона не владеет FK.                        |
| `cascade`       | `CascadeType[]` | `{}`         | Массовые операции: `PERSIST`, `MERGE`, `REMOVE`, `REFRESH`, `DETACH`, `ALL`. Позволяет «прокинуть» операции из родителя на зависимую сущность. |
| `fetch`         | `FetchType`     | `EAGER`      | Загрузка связанной сущности: `EAGER` (по умолчанию) или `LAZY`.                                                                                |
| `optional`      | `boolean`       | `true`       | Если `false`, связь обязательна: в DDL для FK будет `NOT NULL`. Синхронизируйте с `@JoinColumn(nullable = false)`.                             |
| `orphanRemoval` | `boolean`       | `false`      | Если `true`, при снятии связи (присвоении `null`) зависимая сущность удалится из БД автоматически.                                             |
| `targetEntity`  | `Class<?>`      | (от поля)    | Явно указывает класс связанной сущности. Обычно не нужен, поскольку JPA выводит тип из поля/геттер-а.                                          |

1. `mappedBy`

    * Указывается только на обратной стороне двунаправленной связи.
    * Значение — имя поля в «владельческой» сущности.
    * Если задан, JPA не создаёт колонку FK в этой таблице, т. к. она уже есть на стороне, где mappedBy отсутствует.
    * Пример (двунаправленная связь Order ↔ Invoice):

   ```java
   // Владеющая сторона (Order) — FK в таблице orders
   @Entity
   public class Order {
       @OneToOne(cascade = CascadeType.ALL)
       @JoinColumn(name = "invoice_id", nullable = false, unique = true)
       private Invoice invoice;
   }
   
   // Обратная сторона (Invoice) — нет FK-колонки в таблице invoices
   @Entity
   public class Invoice {
       @OneToOne(mappedBy = "invoice", fetch = FetchType.LAZY)
       private Order order;
   }
   ```

2. `cascade`

    * Определяет, какие операции из родительской сущности (где стоит @OneToOne) должны «придти» к зависимой (например,
      User ↔ UserProfile).
    * Типичные варианты:
        * CascadeType.PERSIST — при em.persist(parent) автоматически persist и дочернего объекта.
        * CascadeType.MERGE — при em.merge(parent) автоматически merge и дочернего.
        * CascadeType.REMOVE — при em.remove(parent) также удаляет дочерний объект.
        * CascadeType.ALL — все операции.
    * Когда применять:
        * Если зависимая сущность не имеет смысла без родителя, ставьте cascade = CascadeType.ALL.
        * Если нужно только сохранять и обновлять вместе, используйте PERSIST и MERGE.


3. `fetch`

    * По спецификации JPA, @OneToOne — EAGER (загружается при выборке родителя).
    * Рекомендуется явно указывать LAZY, чтобы избежать лишних JOIN и потенциальной «переобходной» ситуации (N+1).

   ```java
   @OneToOne(fetch = FetchType.LAZY)
   @JoinColumn(name = "profile_id")
   private UserProfile profile;
   ```

    * Особенность Hibernate:
        * Для полноценного LAZY без дополнительной загрузки нужно включить bytecode enhancement (Instrumentation) или
          использовать аннотацию @LazyToOne.

   ```java
   @OneToOne(fetch = FetchType.LAZY)
   @LazyToOne(LazyToOneOption.NO_PROXY)
   @JoinColumn(name = "profile_id")
   private UserProfile profile;
   ```

4. `optional`

    * Если optional = false, JPA добавит к FK NOT NULL (даже если вы не указали nullable = false в @JoinColumn).
    * Синхронизируйте:

   ```java
   @OneToOne(optional = false)
   @JoinColumn(name = "passport_id", nullable = false)
   private Passport passport;
   ```

5. `orphanRemoval`

    * Если true, при снятии ссылки parent.setChild(null) зависимая сущность будет автоматически удалена из БД.
    * Полезно, когда жизненный цикл зависимой сущности полностью соответствует родительской.

   ```java
   @OneToOne(cascade = CascadeType.ALL, orphanRemoval = true)
   @JoinColumn(name = "address_id")
   private Address address;
   При user.setAddress(null) объект Address удалится из таблицы addresses.
   ```

6. `targetEntity`

    * Обычно JPA выводит тип связанной сущности из поля (например, private UserProfile profile → UserProfile.class).
    * Используется только если поле имеет тип-интерфейс или generic:

   ```java
   @OneToOne(targetEntity = com.example.domain.Profile.class)
   @JoinColumn(name = "profile_id")
   private Object profileReference;
   ```

**Пример:**

Односторонняя связь (только владеющая сторона)

```java
@Entity
@Table(name = "users")
public class User {
    @OneToOne(
        fetch = FetchType.LAZY, 
        cascade = CascadeType.ALL, 
        optional = false, 
        orphanRemoval = true
    )
    @JoinColumn(
        name = "profile_id", 
        nullable = false, 
        unique = true, 
        foreignKey = @ForeignKey(name = "FK_USER_PROFILE")
    )
    private UserProfile profile;
}
```

* Что происходит:
    * В таблице `users` появится колонка `profile_id` с `NOT NULL` и `UNIQUE`.
    * При `em.persist(user)` вместе с `UserProfile` будет сохранена зависимая запись в `user_profiles`.
    * Если удалить связь (`user.setProfile(null)`), соответствующий `UserProfile` удалится из БД (
      `orphanRemoval = true`).

Двунаправленная связь (inverse + owning)

```java
@Entity
@Table(name = "orders")
public class Order {
    @Id
    @GeneratedValue
    private Long id;

    @OneToOne(
        fetch = FetchType.LAZY, 
        cascade = CascadeType.ALL
    )
    @JoinColumn(
        name = "invoice_id", 
        nullable = false, 
        unique = true, 
        foreignKey = @ForeignKey(name = "FK_ORDER_INVOICE")
    )
    private Invoice invoice;

    // Геттеры/сеттеры
}

@Entity
@Table(name = "invoices")
public class Invoice {
    @Id
    @GeneratedValue
    private Long id;

    @OneToOne(
        mappedBy = "invoice", 
        fetch = FetchType.LAZY
    )
    private Order order;
}
```

* Order (владеет): FK `invoice_id` создаётся в orders → `invoices.id`.
* Invoice (обратная): `mappedBy = "invoice"` → колонка не создаётся.
* В обоих классах `fetch = LAZY` (Hibernate потребует bytecode enhancement или `@LazyToOne`).

---

- `@ManyToOne` описывает отношение «многие к одному»: множество записей текущей сущности ссылаются на одну запись другой
  сущности. Сторона @ManyToOne всегда является «владельческой» (она хранит FK).

| Атрибут        | Тип             | По умолчанию | Описание                                                                                                                                                                  |
|----------------|-----------------|--------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `targetEntity` | `Class<?>`      | (от поля)    | Явно указывает класс связанной сущности. Обычно JPA сам понимает по типу поля/геттеру.                                                                                    |
| `cascade`      | `CascadeType[]` | `{}`         | Каскадируемые операции: `PERSIST`, `MERGE`, `REMOVE`, `REFRESH`, `DETACH`, `ALL`. Для `@ManyToOne` чаще всего не используют `REMOVE`, чтобы не удалить родителя случайно. |
| `fetch`        | `FetchType`     | `EAGER`      | Загрузка: `EAGER` (по умолчанию) или `LAZY`. Рекомендуется явно указывать `LAZY`.                                                                                         |
| `optional`     | `boolean`       | `true`       | Если `false`, FK-столбец будет `NOT NULL`. Синхронизируйте с `@JoinColumn(nullable = false)`.                                                                             |

```java
@Entity
@Table(name = "orders")
public class Order {
    @Id
    @GeneratedValue
    private Long id;

    @OneToMany(
        mappedBy = "order",
        cascade = CascadeType.ALL,
        orphanRemoval = true,
        fetch = FetchType.LAZY
    )
    private List<OrderItem> items = new ArrayList<>();
}

@Entity
@Table(name = "order_items")
public class OrderItem {
    @Id
    @GeneratedValue
    private Long id;

    @ManyToOne(fetch = FetchType.LAZY, optional = false)
    @JoinColumn(
        name = "order_id", 
        nullable = false, 
        foreignKey = @ForeignKey(name = "FK_ITEM_ORDER")
    )
    private Order order;
}
```

* OrderItem (владеющая сторона) хранит order_id.
* Order (обратная сторона) с mappedBy = "order" — FK не создаётся в orders.
* При удалении Order все OrderItem автоматически удалятся (CascadeType.ALL + orphanRemoval = true).

---

## Hibernate-specific аннотации для оптимизации запросов

- `@Fetch`
    - Пакет: `org.hibernate.annotations.Fetch`
    - Используется вместе с: `@OneToMany`, `@ManyToMany`, `@OneToOne`, `@ManyToOne` (хотя для ManyToOne/OneToOne чаще
      @Fetch(FetchMode.JOIN) не нужен).
    - Атрибуты:
        - value — FetchMode:
            - JOIN — явный JOIN в SQL при загрузке.
            - SELECT — традиционная «ленивая» выборка (может вызвать N+1).
            - SUBSELECT — Hibernate использует WHERE id IN (…) для пакетной загрузки.
    - Пример:
      ```java
      @OneToMany(mappedBy = "order")
      @Fetch(FetchMode.SUBSELECT)
      private List<OrderItem> items;
      ```
    - При загрузке списка заказов Hibernate сначала выберет все orders, а потом разом подгрузит order_items через
      IN (…).

- `@BatchSize`
    - Пакет: `org.hibernate.annotations.BatchSize`
    - Атрибуты:
        - size — int. Число записей, которые Hibernate будет загружать одним запросом при ленивой выборке коллекций или
          связанных сущностей.
    - Пример для коллекций:
       ```
       @OneToMany(mappedBy = "order")
       @BatchSize(size = 20)
       private List<OrderItem> items;
       ```
        - Если у вас есть 50 заказов и вы вызываете getItems() для каждого, Hibernate группирует их по 20, выполняя
          примерно 3 запроса вместо 50 (N+1).
    - Пример для сущностей:
       ```
       @Entity
       @BatchSize(size = 50)
       public class Product { … }
       ```
        - Когда лениво загружается Product через прокси, Hibernate сразу загрузит 50 записей по id IN (…) вместо одной.

- `@LazyToOne`
    - Пакет: `org.hibernate.annotations.LazyToOne` и `org.hibernate.annotations.LazyToOneOption`
    - Назначение: позволяет сделать @OneToOne или @ManyToOne действительно ленивой без bytecode enhancement (с помощью
      прокси-обёртки).
    - Опции:
        - NO_PROXY — Hibernate создаёт прокси на связанный объект.
        - PROXY — аналогично NO_PROXY.
        - FALSE — эквивалент EAGER.
    - Пример:
       ```java
       @OneToOne(fetch = FetchType.LAZY)
       @LazyToOne(LazyToOneOption.NO_PROXY)
       @JoinColumn(name = "profile_id")
       private UserProfile profile;
       ```


- `@FetchProfile`
    - Пакет: `org.hibernate.annotations.FetchProfile` и `org.hibernate.annotations.FetchMode`
    - Назначение: позволяет заранее описать профили «fetch-plan» (какие связи JOIN FETCH выполнять) и активировать их в
      коде.
    - Атрибуты:
        - name — имя профиля.
        - fetchOverrides — массив @FetchProfile.FetchOverride, в котором указываются entity, association, mode (JOIN) и
          т.п.
    - Пример:
      ```
      @FetchProfile(
         name = "order-with-items",
         fetchOverrides = {
            @FetchProfile.FetchOverride(
               entity = Order.class,
               association = "items",
               mode = FetchMode.JOIN
            )
         }
      )
      @Entity
      public class Order { … }
      ```
    - В коде:
       ```
       session.enableFetchProfile("order-with-items");
       Order o = session.get(Order.class, id);
       // При загрузке Order Hibernate выполнит JOIN с order_items.
       ```


- `@Formula`
    - Пакет: `org.hibernate.annotations.Formula`
    - Назначение: позволяет «привязать» вычисляемое поле: SQL-выражение, которое Hibernate вставляет в SELECT.
    - Атрибуты:
        - value — SQL-выражение, возвращающее одиночное значение.
    - Пример:
       ```
       @Entity
       public class Order {
       @Id
       private Long id;
       
           // Вычисляемое поле «количество товаров»:
           @Formula("(SELECT COUNT(*) FROM order_items oi WHERE oi.order_id = id)")
           private int itemCount;
       }
   ```
   - При выборке Order Hibernate подставит подзапрос и вернёт itemCount вместе с остальными колонками.

## Hibernate-specific Аннотации классов

- `@org.hibernate.annotations.Cache` Конфигурация L2-кэша конкретной сущности
    - `usage`
      @DynamicInsert / @DynamicUpdate
      Генерация SQL для INSERT/UPDATE только по непустым или изменившимся полям.

- `@SelectBeforeUpdate` Перед выполнением UPDATE выполняет SELECT, чтобы понять, нужно ли вообще обновлять строку.
- `@OptimisticLocking` Настройка оптимистической блокировки (VERSION или поля-считывателя).
- `@Subselect` Маппинг сущности на подзапрос.
- `@Formula` Виртуальное поле, выражение SQL, вычисляемое в запросе.
- `@FilterDef`, `@Filter` Фильтры, активируемые на уровне сессии.

## Создание JPA-сущностей: Best Practice

# Hibernate

## Основные принципы работы



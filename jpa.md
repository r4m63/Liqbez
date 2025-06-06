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

- `@JoinColumn` для явно указания столбца-связки (foreign key) в таблице, когда вы описываете отношение между
  сущностями. Чаще всего применяется вместе с `@ManyToOne`, `@OneToOne` или в обратной (владельческой) стороне
  `@OneToMany`/`@OneToOne`.

| Атрибут                | Тип                            | Значение по умолчанию                                    | Описание                                                                                                                                                                  |
|------------------------|--------------------------------|----------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `name`                 | `String`                       | `""`                                                     | Имя столбца во «владельческой» таблице, содержащего FK. Если не указано, JPA сгенерирует имя по умолчанию: `<имя_поля>_<имя_первичного_ключа>` (например, `customer_id`). |
| `referencedColumnName` | `String`                       | `""`                                                     | Имя столбца в «целевой» таблице (та, на которую ссылаются). По умолчанию — первичный ключ целевой сущности.                                                               |
| `unique`               | `boolean`                      | `false`                                                  | Если `true`, для этого столбца будет добавлено ограничение `UNIQUE`.                                                                                                      |
| `nullable`             | `boolean`                      | `true`                                                   | Определяет, может ли внешний ключ принимать значение `NULL`. Если `nullable = false`, будет `NOT NULL`.                                                                   |
| `insertable`           | `boolean`                      | `true`                                                   | Если `false`, столбец не будет включён в SQL-операцию `INSERT`.                                                                                                           |
| `updatable`            | `boolean`                      | `true`                                                   | Если `false`, столбец не будет включён в SQL-операцию `UPDATE`.                                                                                                           |
| `columnDefinition`     | `String`                       | `""`                                                     | Позволяет задать свой SQL-фрагмент для создания столбца (например, тип или дефолт).                                                                                       |
| `table`                | `String`                       | `""`                                                     | Имя таблицы, в которой находится этот столбец; чаще используется при маппинге на несколько таблиц (`@SecondaryTable`).                                                    |
| `foreignKey`           | `javax.persistence.ForeignKey` | `@ForeignKey( value = ConstraintMode.PROVIDER_DEFAULT )` | Позволяет задать дополнительные параметры внешнего ключа (имя ограничения, правило `ON DELETE`/`ON UPDATE` и т.д.).                                                       |

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

- `@OneToOne` Указывает, что текущая сущность в отношении «один к одному» находится в отношении с одной конкретной
  записью другой сущности. Может использоваться как двунаправленное, так и одностороннее отношение.

| Атрибут         | Тип             | Значение по умолчанию           | Описание                                                                                                                          |
|-----------------|-----------------|---------------------------------|-----------------------------------------------------------------------------------------------------------------------------------|
| `mappedBy`      | `String`        | `""`                            | Имя свойства в целевой (обратной) сущности, которое владеет связью. Применяется только на обратной стороне двунаправленной связи. |
| `cascade`       | `CascadeType[]` | `{}` (нет каскадов)             | Список операций каскадирования (`PERSIST, MERGE, REMOVE, REFRESH, DETACH, ALL`).                                                  |
| `fetch`         | `FetchType`     | `EAGER`                         | Тип загрузки: `EAGER` (по умолчанию) или `LAZY`.                                                                                  |
| `optional`      | `boolean`       | `true`                          | Если `false`, при попытке сохранить без связанной сущности возникнет ошибка (соответствует `NOT NULL` в FK).                      |
| `orphanRemoval` | `boolean`       | `false`                         | Если `true`, при разрыве связи «владелец → зависимость» зависимая сущность будет удалена из БД.                                   |
| `targetEntity`  | `Class<?>`      | текущий тип поля (по умолчанию) | Явно указывает класс связанной сущности (обычно не требуется, так как JPA умеет вывести из типа поля).                            |

> Примечание. Поля mappedBy и optional применяются только в тех случаях, когда нужно явно скорректировать поведение.

- `@ManyToOne`

---

## Hibernate Аннотации классов

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



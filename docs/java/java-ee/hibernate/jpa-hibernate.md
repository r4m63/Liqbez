@Enumerated
@OneToMany, @ManyToOne, @OneToOne, @ManyToMany
orphanRemoval = true – автоматическое удаление "осиротевших" записей.
@BatchSize (Hibernate) – загрузка коллекций пачками.
@Fetch(FetchMode.SUBSELECT) – загрузка подзапросом вместо N+1.
@OneToMany(mappedBy = "user")
@Fetch(FetchMode.SUBSELECT)
private List<Order> orders;
@Embeddable + @Embedded
@Embeddable
@Inheritance – стратегии наследования (SINGLE_TABLE, JOINED, TABLE_PER_CLASS).
@Entity
@Inheritance(strategy = InheritanceType.JOINED)

Всегда используй LAZY там, где возможно (иначе N+1 проблема).
Для @OneToMany по умолчанию LAZY, для @ManyToOne – EAGER (лучше явно указать LAZY).




-----

Best Practices для JPA-сущностей

Иммутабельность и неизменяемость
По возможности делай сущности неизменяемыми (immutable) через final поля и @Setter только где нужно.
Используй DTO для передачи данных, а не сущности напрямую.

Оптимальные Fetch-стратегии
Всегда LAZY, если не нужен EAGER.
Используй @EntityGraph для динамической загрузки связей.
JPQL/HQL с JOIN FETCH для явной загрузки.

@Query("SELECT u FROM User u JOIN FETCH u.orders WHERE u.id = :id")
User findByIdWithOrders(@Param("id") Long id);

Кэширование
@Cacheable (Spring) + Hibernate L2 Cache (@Cache).

@Entity
@Cacheable
@org.hibernate.annotations.Cache(usage = CacheConcurrencyStrategy.READ_WRITE)
public class Product { ... }

Оптимизация запросов
Избегай SELECT * – используй @NamedEntityGraph или проекции (DTO, интерфейсы).

Для массовых операций используй @Modifying + @Query.

@Modifying
@Query("UPDATE User u SET u.active = false WHERE u.lastLogin < :date")
void deactivateInactiveUsers(@Param("date") LocalDate date);

Валидация и constraints
@NotNull, @Size, @Email (из javax.validation).

@Column(nullable = false)
@NotNull
@Size(min = 3, max = 50)
private String username;

Аудит (кто и когда изменил)
@CreatedDate, @LastModifiedDate (Spring Data JPA).

@EntityListeners(AuditingEntityListener.class)
public class User {
@CreatedDate
private LocalDateTime createdAt;

    @LastModifiedDate
    private LocalDateTime updatedAt;

}

Best Practices
✅ Всегда используй LAZY для связей (@ManyToOne, @OneToMany), если не нужен EAGER.
✅ Избегай N+1 через JOIN FETCH, @EntityGraph, @BatchSize.
✅ Валидация через jakarta.validation (@NotBlank, @Size).
✅ Кэширование (@Cacheable) для часто читаемых данных.
✅ Аудит (@CreatedDate, @LastModifiedDate) для отслеживания изменений.
✅ Оптимистичная блокировка (@Version) для конкурентных обновлений.


----------

## Аннотации класса

Стандартные JPA-аннотации

| Аннотация                                     | Описание                                                        |
|-----------------------------------------------|-----------------------------------------------------------------|
| @SecondaryTable	                              | Позволяет маппить сущность на несколько таблиц.                 | 
| @Inheritance(strategy = InheritanceType.XXX)	 | Стратегия наследования (SINGLE_TABLE, JOINED, TABLE_PER_CLASS). | 
| @DiscriminatorColumn(name = "type")	          | Используется с SINGLE_TABLE для хранения типа сущности.         | 
| @DiscriminatorValue("value")	                 | Указывает значение дискриминатора для текущего класса.          | 
| @Embeddable	                                  | Объявляет класс как встраиваемый (не является сущностью).       | 
| @MappedSuperclass	                            | Класс-родитель, поля которого маппятся на таблицы наследников.  | 

Hibernate-специфичные аннотации

| Аннотация                                                       | Описание                                                  |
|-----------------------------------------------------------------|-----------------------------------------------------------|
| @DynamicInsert	                                                 | Генерирует SQL только для ненулевых полей.                | 
| @DynamicUpdate	                                                 | Обновляет только измененные поля (не весь объект).        | 
| @Immutable	                                                     | Запрещает изменения сущности (оптимизация для read-only). | 
| @Proxy(lazy = false)	                                           | Отключает ленивую загрузку прокси.                        | 
| @Polymorphism(type = PolymorphismType.EXPLICIT)	                | Указывает, включать ли класс в полиморфные запросы.       | 
| @Where(clause = "active = true")                                | 	Фильтр для всех запросов к сущности.                     | 
| @FilterDef(name = "activeFilter", parameters = @ParamDef(...))  | 	Динамический фильтр.                                     | 
| @Filter(name = "activeFilter", condition = "active = :active")	 | Применение фильтра.                                       | 
| @Subselect("SELECT ... FROM ...")	                              | Маппинг на SQL-представление (не таблицу).                | 
| @Synchronize({"table1", "table2"})                              | 	Указывает таблицы для проверки кэша.                     | 

Spring Data JPA и аудит

| Аннотация                                       | Описание                                             |
|-------------------------------------------------|------------------------------------------------------|
| @EntityListeners(AuditingEntityListener.class)	 | Включает аудит (@CreatedDate, @LastModifiedBy).      | 
| @CreatedBy, @LastModifiedBy                     | 	Автоматическое заполнение пользователя.             | 
| @CreatedDate, @LastModifiedDate	                | Автоматическое заполнение дат.                       | 
| @DomainEvents                                   | 	Публикация событий перед сохранением (Spring Data). | 
| @Transactional                                  | 	Управление транзакциями (Spring).                   | 

Кэширование

| Аннотация                                                              | Описание                                                       |
|------------------------------------------------------------------------|----------------------------------------------------------------|
| @Cacheable	                                                            | Включает кэширование (Spring).                                 | 
| @org.hibernate.annotations.Cache(usage = CacheConcurrencyStrategy.XXX) | 	Стратегия кэша (READ_ONLY, READ_WRITE, NONSTRICT_READ_WRITE). |

Валидация (Jakarta Validation)

| Аннотация  | 	Описание                               |
|------------|-----------------------------------------|
| @Validated | Включает валидацию для класса (Spring). |

## Аннотации полей

| Аннотация                                | Описание                                               |
|------------------------------------------|--------------------------------------------------------|
| @EntityListeners                         | Аудит (автоматическое заполнение createdAt/updatedAt). |
|                                          |                                                        |
| @Enumerated(EnumType.STRING)             | сохранение enum как строки                             |
|                                          |                                                        |
| @ManyToOne(fetch = LAZY)                 | всегда ленивая загрузка                                |
| @OneToMany(orphanRemoval = true          | автоматическое удаление "осиротевших" записей          |
| @BatchSize + @Fetch(FetchMode.SUBSELECT) | борьба с N+1                                           |
|                                          |                                                        |
| @Transient                               | поле не сохраняется в БД                               |
| @Version                                 | оптимистичная блокировка                               |
| @PrePersist, @PostLoad                   | хуки жизненного цикла                                  |
|                                          |                                                        |

# JPA specification

### **Best Practice:**

* Всегда указывайте name, если имя таблицы ≠ имени класса
* Используйте schema для многопользовательских БД
* Добавляйте индексы на часто используемые поля
* Уникальные constraints для бизнес-правил
    ```
    // Гарантирует, что email и phone уникальны:
    @Table(
        name = "users",
        uniqueConstraints = {
            @UniqueConstraint(name = "uk_user_email", columnNames = "email"),
            @UniqueConstraint(name = "uk_user_phone", columnNames = "phone")
        }
    )
    ```
* Избегайте избыточности с @Column(unique = true)
    ```
    // Избыточно (дублирование):
    @Table(uniqueConstraints = @UniqueConstraint(columnNames = "email"))
    public class User {
        @Column(unique = true)  // Лучше оставить только это
        private String email;
    }
    
    // Лучше:
    public class User {
        @Column(unique = true)  // Проще и понятнее
        private String email;
    }
    ```

---

## 1. Аннотация `@JoinColumn`

Аннотация `@JoinColumn` используется для явно указания столбца-связки (foreign key) в таблице, когда вы описываете
отношение между сущностями. Чаще всего применяется вместе с `@ManyToOne`, `@OneToOne` или в обратной (владельческой)
стороне `@OneToMany`/`@OneToOne`.

### 1.1 Основное назначение

- Указывает имя колонки в текущей таблице, которая хранит внешний ключ.
- Позволяет детально сконфигурировать, какие именно свойства колонки (nullable, unique, insertable, updatable и т.д.)
  будут применены.
- Используется на «владельческой» стороне отношения (та, что содержит внешний ключ).

### 1.2 Атрибуты `@JoinColumn`

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

> **Примечание.** В более старых версиях JPA (до 2.1) атрибут `foreignKey` отсутствует.

### 1.3 Какие параметры принимает и что они означают

1. **`name`**

- **Тип:** `String`
- **Описание:** конкретное имя колонки FK в текущей таблице.
- **Пример:**
  ```java
  @JoinColumn(name = "customer_id")
  ```
- **Если не указан:** JPA сделает `<имя_поля>_<pk_целевой_сущности>`. Например, если поле называется `customer` и у
  сущности `Customer` PK — `id`, то столбец будет `customer_id`.

2. **`referencedColumnName`**

- **Тип:** `String`
- **Описание:** явно указывает, на какой столбец целевой таблицы ссылается внешний ключ.
- **Пример (если у `Customer` не `id`, а `uuid` в качестве PK):**
  ```java
  @JoinColumn(name = "customer_uuid", referencedColumnName = "uuid")
  ```
- **Если не указан:** автоматически берётся PK (обычно `id`).

3. **`unique`**

- **Тип:** `boolean`
- **Описание:** если `true`, создаётся ограничение `UNIQUE` на уровне схемы. Используется редко, например, при
  `@OneToOne`, если хотим гарантировать «один к одному» на уровне БД.
- **Пример:**
  ```java
  @JoinColumn(name = "profile_id", unique = true)
  ```

4. **`nullable`**

- **Тип:** `boolean`
- **Описание:** если `false`, будет добавлено `NOT NULL` в DDL. По умолчанию `true` (разрешено `NULL`). Обычно для
  обязательных связей (`optional = false`) ставят `nullable = false`.
- **Пример:**
  ```java
  @JoinColumn(name = "customer_id", nullable = false)
  ```

5. **`insertable` / `updatable`**

- **Тип:** `boolean`
- **Описание:** управляют тем, включается ли колонка в SQL-запросы `INSERT` и/или `UPDATE`. Например, если хотите, чтобы
  колонка заполнялась только через БД (триггеры) или внешние механизмы, можно поставить `insertable = false`.
- **Пример:**
  ```java
  @JoinColumn(name = "order_type_fk", insertable = false, updatable = false)
  ```
- **Когда применяется:** очень редко. Чаще встречается в сценариях «внешние» или «рассчитанные» FK.

6. **`columnDefinition`**

- **Тип:** `String`
- **Описание:** дает возможность задать «сырой» SQL-фрагмент при создании столбца. Например, если нужен тип `UUID` или
  дефолтное значение.
- **Пример:**
  ```java
  @JoinColumn(name = "customer_id", columnDefinition = "BIGINT DEFAULT 0")
  ```

7. **`table`**

- **Тип:** `String`
- **Описание:** название таблицы, в которой находится этот столбец, если сущность маппится на несколько таблиц (
  используются `@SecondaryTable`).
- **Пример:**
  ```java
  @SecondaryTable(name = "customer_details")
  public class Customer {
      @Id
      private Long id;
      // ...
      @Column(table = "customer_details", name = "detail_info")
      private String detailInfo;
      // ...
      @OneToOne
      @JoinColumn(name = "passport_id", table = "customer_details")
      private Passport passport;
  }
  ```

8. **`foreignKey`** (JPA 2.1+)

- **Тип:** `javax.persistence.ForeignKey`
- **Описание:** дополнительные настройки внешнего ключа: имя ограничения, поведение `ON DELETE`/`ON UPDATE`, режим
  `ConstraintMode.NO_CONSTRAINT` (чтобы не создавать, если внешний ключ моделируется вручную).
- **Пример:**
  ```java
  @JoinColumn(
      name = "order_id",
      foreignKey = @ForeignKey(
          name = "FK_ORDER_CUSTOMER",
          value = ConstraintMode.CONSTRAINT
      )
  )
  private Order order;
  ```

### 1.4 Пример использования `@JoinColumn`

```java
@Entity
@Table(name = "orders")
public class Order {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

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

    // остальные поля
}

---

## `@ManyToOne`

```

@Entity
public class Comment {
@Id
@GeneratedValue
private Long id;

    @ManyToOne(fetch = FetchType.LAZY)  // Всегда LAZY для ManyToOne!
    @JoinColumn(name = "author_id")     // Столбец в таблице Comment
    private Author author;

}

```

`fetch = FetchType.LAZY` FetchType.LAZY (по умолчанию для @ManyToOne) или EAGER

`optional = false` Может ли связь быть null (по умолчанию true)

`cascade = CascadeType.ALL` Каскадные операции (сохранение/удаление)

`targetEntity = Author.class` Класс целевой сущности (если нужен дженерик)

---

```

@Entity
public class Comment {
@Id
@GeneratedValue
private Long id;

    @ManyToOne(fetch = FetchType.LAZY)  // Всегда LAZY!
    @JoinColumn(name = "user_id", nullable = false, foreignKey = @ForeignKey(name = "fk_comment_user"))
    private User author;

}

```

### Best Practices:

✅ Всегда используйте FetchType.LAZY
✅ Явно указывайте @JoinColumn с nullable = false для обязательной связи
✅ Добавляйте именованные foreign keys (@ForeignKey)
❌ Использование EAGER (риск N+1 проблемы)
❌ Отсутствие @JoinColumn (Hibernate сам создаст столбец с неочевидным именем)
✅ Всегда используйте LAZY (ленивую загрузку), если не нужен EAGER.
✅ Добавляйте @JoinColumn для явного указания имени столбца.
✅ Для обязательной связи указывайте optional = false.

## `@OneToMany`

```

@Entity
public class Author {
@Id
@GeneratedValue
private Long id;

    @OneToMany(mappedBy = "author", cascade = CascadeType.ALL, orphanRemoval = true)
    private List<Book> books = new ArrayList<>();

}

```

`mappedBy = "author"`    Указывает поле в дочерней сущности, которое управляет связью

`cascade = CascadeType.ALL`    Каскадные операции

`orphanRemoval = true`    Удалять ли "осиротевшие" записи (при удалении из коллекции)

`fetch = FetchType.LAZY`    FetchType.LAZY (по умолчанию для @OneToMany) или EAGER

### Best Practices:

✅ Всегда инициализируйте коллекции (new ArrayList<>()).
✅ Используйте orphanRemoval = true, если нужно удалять дочерние записи.
✅ Избегайте EAGER (может привести к N+1 проблеме).

---

```

@Entity
public class User {
@Id
@GeneratedValue
private Long id;

    @OneToMany(mappedBy = "user", cascade = CascadeType.ALL, orphanRemoval = true)
    private List<Order> orders = new ArrayList<>();  // Инициализация обязательна!

}

```

### Best Practices:

✅ Всегда используйте mappedBy на стороне владельца
✅ Инициализируйте коллекцию (new ArrayList<>())
✅ Для удаления дочерних элементов используйте orphanRemoval = true

### Ошибки:

❌ Отсутствие mappedBy (приводит к созданию лишней join-таблицы)
❌ Использование EAGER (загрузит все дочерние записи сразу)

## `@ManyToMany`

```

@Entity
public class Student {
@Id
@GeneratedValue
private Long id;

    @ManyToMany
    @JoinTable(
        name = "student_course",
        joinColumns = @JoinColumn(name = "student_id"),
        inverseJoinColumns = @JoinColumn(name = "course_id")
    )
    private Set<Course> courses = new HashSet<>();

}

@Entity
public class Course {
@Id
@GeneratedValue
private Long id;

    @ManyToMany(mappedBy = "courses")
    private Set<Student> students = new HashSet<>();

}

```

`mappedBy = "courses"`    Указывает, какая сторона управляет связью

`cascade = CascadeType.PERSIST`    Каскадные операции

`fetch = FetchType.LAZY`    FetchType.LAZY (по умолчанию) или EAGER

`targetEntity = Course.class`    Класс целевой сущности

### Best Practices:

✅ Используйте Set вместо List (избегает дубликатов).
✅ Одна сторона должна быть владельцем (с @JoinTable).
✅ Избегайте EAGER (может загрузить всю БД).

---

```

@Entity
public class Student {
@Id
@GeneratedValue
private Long id;

    @ManyToMany
    @JoinTable(
        name = "student_course",
        joinColumns = @JoinColumn(name = "student_id"),
        inverseJoinColumns = @JoinColumn(name = "course_id"),
        foreignKey = @ForeignKey(name = "fk_student_course_student"),
        inverseForeignKey = @ForeignKey(name = "fk_student_course_course")
    )
    private Set<Course> courses = new HashSet<>();  // Set вместо List!

}

@Entity
public class Course {
@Id
@GeneratedValue
private Long id;

    @ManyToMany(mappedBy = "courses")
    private Set<Student> students = new HashSet<>();

}

```

### Best Practices:

✅ Используйте Set вместо List для избежания дубликатов
✅ Одна сторона должна быть владельцем (с @JoinTable)
✅ Добавляйте именованные foreign keys

### Ошибки:

❌ Отсутствие mappedBy на одной из сторон (создаст две join-таблицы)
❌ Использование List без дополнительных мер против дубликатов

## Other INFO

Каскадные операции (cascade)
Определяют, какие операции применяются к связанным сущностям:

Тип каскада Описание
CascadeType.ALL - Все операции (сохранить, обновить, удалить)
CascadeType.PERSIST - Сохранить связанную сущность
CascadeType.MERGE - Обновить связанную сущность
CascadeType.REMOVE - Удалить связанную сущность
CascadeType.REFRESH - Обновить из БД
CascadeType.DETACH - Отсоединить от контекста

Правила каскадирования:

```

// Сохранит/обновит/удалит все заказы при сохранении пользователя
@OneToMany(mappedBy = "user", cascade = {CascadeType.PERSIST, CascadeType.MERGE, CascadeType.REMOVE})
private List<Order> orders;

```

```

@OneToMany(mappedBy = "author", cascade = {CascadeType.PERSIST, CascadeType.MERGE})
private List<Book> books;

```

Best Practices:
Для @ManyToOne обычно не используют каскады
Для @OneToMany часто используют CascadeType.ALL + orphanRemoval
Для @ManyToMany каскады используют осторожно
Опасные сценарии:
⚠️ CascadeType.ALL на @ManyToMany может случайно удалить связанные сущности
⚠️ Отсутствие orphanRemoval при удалении из коллекции оставляет "висячие" записи

# Java Bean Validation

Валидационные аннотации из Jakarta Bean Validation (JSR-380) могут применяться к разным объектам (Entity, DTO,
Form-объектам)

В Spring REST-контроллерах (DTO) - При получении HTTP-запроса Spring автоматически проверяет DTO перед вызовом метода
контроллера

- Клиент отправляет JSON в /users.
- Spring преобразует JSON в UserDto.
- До вызова метода createUser() запускается валидация.
- Если есть ошибки — возвращается 400 Bad Request с описанием ошибок.

---

`@NotBlank String name` Проверяет, что строка не null и не пустая (после обрезки пробелов)

`@NotEmpty List<String>` Проверяет, что строка/коллекция не null и не пустая

`@NotNull Long id` Проверяет, что поле не null

`@Size(min = 3, max = 50) String username` Проверяет длину строки/коллекции (min/max)

`@Pattern(regexp = "^[A-Za-z0-9]+$")` Проверяет соответствие регулярному выражению

`@Min(18) int age` Минимальное значение (для чисел)

`@Max(100) int age` Максимальное значение (для чисел)

`@Positive BigDecimal price` Число должно быть > 0

`@PositiveOrZero int quantity` Число ≥ 0

`@Negative BigDecimal balance` Число должно быть < 0

`@NegativeOrZero int delta` Число ≤ 0

`@Digits(integer=3, fraction=2)` Проверяет количество цифр (целая и дробная часть)

`@Past LocalDate birthDate` Дата должна быть в прошлом

`@PastOrPresent LocalDateTime createdAt` Дата в прошлом или сегодня

`@Future LocalDate expiryDate` Дата должна быть в будущем

`@FutureOrPresent LocalDate appointmentDate` Дата в будущем или сегодня

`@Email String email` Проверяет, что строка — валидный email

`@URL String website` Проверяет, что строка — валидный URL

`@CreditCardNumber String cardNumber` Проверяет номер кредитной карты (Luhn algorithm)

`@Length(min = 5, max = 20)` Аналог @Size (из Hibernate Validator)

`@NotEmpty List<String> tags` Коллекция/массив не должны быть пустыми

`@Size(min = 1, max = 10) List<Product> products` Проверяет размер коллекции/массива

# Где лучше задавать ограничения: в JPA-модели или в PostgreSQL?

| Способ              | Плюсы                            | Минусы                       | Когда использовать              |
|---------------------|----------------------------------|------------------------------|---------------------------------|
| Только в JPA        | Простота, переносимость между БД | Нет гарантии на уровне БД    | Простые проекты, прототипы      |
| Только в PostgreSQL | Максимальная надёжность          | Сложно поддерживать миграции | Критичные к данным системы      |
| Комбинированный     | Баланс надёжности и гибкости     | Дублирование кода            | Большинство production-проектов |

# Hibernate

## N+1 проблема в hibernate

Возникает, когда у родительской сущности есть дочерние сущность. Приложение делает 1 запрос, чтобы взять родительскую
сущность, а потом, чтобы подтянуть все дочерние сущность - выполняется по 1 запросу для каждой дочерней сущности.
То есть сначала делает 1 select, чтобы взять всех родительских сущностей, а потом для каждой родительской сущности он
будет делать отдельный select (с фильтром where на родительскую сущность).

### Решение 1. Использовать LAZY (FetchType.LAZY)

Это решит проблему - только если нам не надо использовать дочерние сущности и мы не будем их вызывать. Иначе - просто
отложено позже выполнятся n+1 запросов для всех дочерних сущностей по каждому отдельному select.

### Решение 2. Использовать Join Fetch

Надо сделать чтобы Hibernate под капотом сделал sql запрос с join внутри.
То есть надо сделать кастомный sql запрос с Join внутри и вызывать его - тогда будет выполняться 1 запрос.

```java
@Repository
@ALLArgsConstructor
public class CustomClientRepository {

  @PersistenceContext
  private final EntityManager entityManager;

  public List<Client> findAllClientWithPayments) {
    String jpql = "select c from Client c join fetch c.payments";
    return entityManager.createQuery(jpql, Client.class)
           .getResultList();
  }
｝
```

Так же можно и переопределить стандартный метод репозитория

```java
public interface ClientRepository extends JpaRepository<Client, Integer>

  @Query("select c from Client c join fetch c.payments")
  @Override 
  List<Client> findAll();
}
```

Или просто сделать свой метод

```java
public interface ClientRepository extends JpaRepository<Client, Integer>

  @Query("select c from Client c join fetch c.payments")
  List<Client> findAllWithPaments();
}
```

### Решение 3. Использовать Entity Graph

### Решение 4. Использовать Batch Size

Hibernate тогда не будет делать join, а будет так же подгружать лениво и будет объединять пачками - батчами с указанным
размером (компромисс между join и загрузкой лениво).

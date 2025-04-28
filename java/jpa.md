@Entity – отмечает класс как JPA-сущность.
@Id – указывает на первичный ключ.
@GeneratedValue – автоматическая генерация ID.
@GeneratedValue(strategy = GenerationType.IDENTITY)
@Id
@GeneratedValue(strategy = GenerationType.IDENTITY)  // Для автоинкремента (MySQL, PostgreSQL)
// @GeneratedValue(strategy = GenerationType.SEQUENCE)  // Для последовательностей (Oracle)
// @GeneratedValue(strategy = GenerationType.UUID)  // UUID
private Long id;
@Table – кастомизирует имя таблицы и схему.
@Column – кастомизация колонки.
@Enumerated – сохранение enum.
@OneToMany, @ManyToOne, @OneToOne, @ManyToMany
Всегда используй LAZY там, где возможно (иначе N+1 проблема).
Для @OneToMany по умолчанию LAZY, для @ManyToOne – EAGER (лучше явно указать LAZY).
orphanRemoval = true – автоматическое удаление "осиротевших" записей.
@BatchSize (Hibernate) – загрузка коллекций пачками.
@OneToMany(mappedBy = "user")
@BatchSize(size = 20)
private List<Order> orders;
@Fetch(FetchMode.SUBSELECT) – загрузка подзапросом вместо N+1.
@OneToMany(mappedBy = "user")
@Fetch(FetchMode.SUBSELECT)
private List<Order> orders;
@Embeddable + @Embedded
@Embeddable
public class Address {
private String city;
private String street;
}
@Entity
public class User {
@Embedded
private Address address;
}
@Inheritance – стратегии наследования (SINGLE_TABLE, JOINED, TABLE_PER_CLASS).
@Entity
@Inheritance(strategy = InheritanceType.JOINED)
public abstract class BaseEntity { ... }

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

# Подробное описание работы

## `@Entity`

Помечает класс как сущность, т.е. объект, который может быть сохранён в базе данных.

**Обязательные требования:**

* Класс должен быть public или protected (не private или final).
* Должен иметь конструктор без аргументов (public или protected).
* Не должен быть final (Hibernate использует прокси для ленивой загрузки).
* Поля должны быть доступны через геттеры/сеттеры (или public/protected).

## `@Table`

Используется вместе с `@Entity` для тонкой настройки маппинга сущности на таблицу базы данных.

---

`name` Сущность маппится на таблицу app_users. Если не указано, JPA использует имя класса (User(class) → таблица user
или User в зависимости от диалекта).

```
@Table(name = "app_users")
```

---

`schema` Таблица hr.employees. Если БД использует схемы.

```
@Table(name = "employees", schema = "hr")
```

---

`indexes` индексы для ускорения запросов, `name` – имя индекса в БД, `columnList` – колонки, по которым строится
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

---

`uniqueConstraints` уникальные ограничения. Позволяет задать составные уникальные ключи (например, UNIQUE(email,
phone)).

```
@Table(
    name = "users",
    uniqueConstraints = {
        @UniqueConstraint(name = "uk_user_email", columnNames = "email"),
        @UniqueConstraint(name = "uk_user_phone", columnNames = "phone")
    }
)
```

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

## `@Id`

Помечает поле как первичный ключ (primary key) сущности.
Обязательна для любой JPA-сущности. Должно быть непримитивным
типом (Long, Integer, String, UUID и т.д.). Не должно быть final.

## `@GeneratedValue`

Указывает стратегию автоматической генерации значений первичного ключа.

---

`strategy` Type - GenerationType

- `GenerationType.AUTO` Hibernate сам выбирает стратегию, Обычно выбирает SEQUENCE или IDENTITY в зависимости от БД, Не
  рекомендуется для продакшна из-за непредсказуемости
- `GenerationType.IDENTITY` Использует автоинкремент базы данных (SERIAL), Значение генерируется при вставке записи
- `GenerationType.SEQUENCE` Использует sequence базы данных, требует создания sequence в БД
- `GenerationType.TABLE` Использует отдельную таблицу для генерации ID

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

---

`generator` Type - String, Имя генератора (для SEQUENCE или TABLE)

```
@GeneratedValue(generator = "seq_gen")
```

---

`@SequenceGenerator` Настраивает sequence для `GenerationType.SEQUENCE`

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

---

`@TableGenerator` Настраивает таблицу для `GenerationType.TABLE`

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

## `@Column`

Позволяет настраивать маппинг поля класса на столбец таблицы в базе данных. Если её не указывать, Hibernate использует
значения по умолчанию (имя поля = имя столбца)

---

`name` String Имя столбца в БД (если отличается от имени поля)

---

`nullable` boolean Может ли поле быть NULL (по умолчанию true)

---

`unique` boolean Должно ли поле быть уникальным (аналог UNIQUE в SQL)

---

`length` int Максимальная длина для строк (аналог VARCHAR(255))

---

`precision` int Общее количество цифр (для DECIMAL)

---

`scale` int Количество цифр после запятой (для DECIMAL)

---

`insertable` boolean Участвует ли поле в INSERT (по умолчанию true)

---

`updatable` boolean Участвует ли поле в UPDATE (по умолчанию true)

---

`columnDefinition` String Позволяет задать точный SQL-тип столбца

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

### Ошибки:

❌ Использование EAGER (риск N+1 проблемы)
❌ Отсутствие @JoinColumn (Hibernate сам создаст столбец с неочевидным именем)

### Best Practices:

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

### Как избежать N+1 проблемы?

Проблема: при LAZY-загрузке Hibernate делает отдельный запрос для каждой дочерней сущности.

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



















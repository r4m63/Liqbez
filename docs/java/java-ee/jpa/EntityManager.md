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

на каждый HTTP-запрос (или транзакцию) контейнер создаёт новый экземпляр EntityManager, связанный с одним
PersistenceContext

Жизненный цикл EntityManager обычно совпадает с жизненным циклом запроса или JTA-транзакции.
Он не потокобезопасен.
После коммита или rollback — автоматически закрывается контейнером.

В Jakarta EE/Spring, EntityManager создаётся контейнером (WildFly, Payara, TomEE...) на основе Persistence Unit,
описанной в persistence.xml

Когда контейнер поднимает приложение:

- читает persistence.xml
- создаёт EntityManagerFactory
- управляет EntityManager для каждого запроса или транзакции.

@PersistenceContext(unitName = "studsPU")
private EntityManager em;

Контейнер внедряет управляемый EntityManager из пула.
Этот EntityManager связан с текущим контекстом транзакции (или с @RequestScoped контекстом в CDI).

| Тип                     | Когда используется          | Пример                                                                          |
|-------------------------|-----------------------------|---------------------------------------------------------------------------------|
| **Container-managed**   | Jakarta EE, Spring, Quarkus | `@PersistenceContext` — контейнер сам управляет транзакциями                    |
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

EntityManager:

CRUD-операции

| Метод                   | Описание                                                       |
|-------------------------|----------------------------------------------------------------|
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
|-------------------------------------------|----------------------------------------------|
| `createQuery(String jpql, Class<T>)`      | Создать JPQL-запрос (объектный SQL)          |
| `createNamedQuery(String name, Class<T>)` | Использовать @NamedQuery из Entity           |
| `createNativeQuery(String sql, Class<T>)` | Выполнить нативный SQL и замаппить на Entity |
| `createNativeQuery(String sql)`           | Нативный SQL без маппинга                    |
| `createQuery(CriteriaQuery<T>)`           | Типобезопасные запросы через Criteria API    |

Транзакции (если не контейнер, а вручную)

| Метод               | Описание                                 |
|---------------------|------------------------------------------|
| `getTransaction()`  | Возвращает объект `EntityTransaction`    |
| `joinTransaction()` | Присоединяет EM к текущей JTA-транзакции |

# PersistenceContext

@PersistenceContext - аннотация Jakarta EE, которая говорит контейнеру инжектировать сюда EntityManager, связанный с
persistence unit

PersistenceContext — это внутренняя структура, которую EntityManager использует,
чтобы хранить и отслеживать все Entity-объекты, находящиеся в состоянии Managed.

Он живёт ровно столько, сколько живёт EntityManager (обычно — одну транзакцию или HTTP-запрос).

Следит, какие объекты уже загружены из БД.
Проверяет, изменились ли поля (dirty checking).
Знает, какие объекты нужно INSERT, UPDATE, DELETE при flush().
Следит за связями (@OneToMany, @ManyToOne), кэшем и каскадами.
Отслеживает идентичность — если ты дважды вызовешь em.find(Admin.class, 1L), ты получишь тот же самый объект, а не новую
копию.

Состояния сущностей в PersistenceContextСостояния сущностей в PersistenceContext

| Состояние    | Где находится                                                      | Что делает JPA             |
|--------------|--------------------------------------------------------------------|----------------------------|
| **New**      | вне контекста                                                      | объект ещё не сохранён     |
| **Managed**  | внутри PersistenceContext                                          | JPA отслеживает изменения  |
| **Detached** | вышел из контекста (например, после `em.clear()` или `em.close()`) | больше не отслеживается    |
| **Removed**  | помечен на удаление                                                | будет удалён при `flush()` |

# Criteria API

это объектная (типобезопасная) альтернатива JPQL
даёт возможность строить динамические запросы к базе данных не строками, а через типобезопасный Java-код
Вместо строк "SELECT v FROM Vehicle v WHERE v.type = :t" ты создаёшь запрос с помощью Java-классов (CriteriaBuilder,
CriteriaQuery, Root и т.д.)

| Проблема с JPQL                                               | Как решает Criteria                     |
|---------------------------------------------------------------|-----------------------------------------|
| Строки, ошибки видны только в runtime                         | Компиляция проверяет типы и имена полей |
| Сложно строить динамические фильтры (if … then add condition) | Легко добавлять условия программно      |
| Переименование полей ломает запросы                           | IDE-рефакторинг автоматически обновит   |
| Разные поля и условия по разным параметрам                    | Можно строить условия в цикле           |

Основные участники API

| Класс / интерфейс      | Назначение                                                                                         |
|------------------------|----------------------------------------------------------------------------------------------------|
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
CriteriaBuilder  →  CriteriaQuery<Vehicle>
                         │
                         ├── FROM → Root<Vehicle>
                         ├── WHERE → Predicate
                         ├── SELECT → Projection
                         └── ORDER BY / GROUP BY
```


Root<T> 
Это корень запроса — то, что стоит в FROM ... в SQL
Всегда создаётся через CriteriaQuery.from(EntityClass)
это частный случай From<T, T> (т.е. Root наследуется от From)

From<Z, X>
Это источник строк в запросе (таблица или присоединённая таблица)
Это “толстый” путь (path), который умеет делать JOIN и хранит уже сделанные JOIN-ы

Path<X>
Это любой путь к атрибуту: столбцу, embedded полю и даже к join’у как к узлу
С Path вы строите предикаты: cb.equal(path, ...), cb.lessThan(path, ...) и т.д.

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




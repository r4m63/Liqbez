у hibernate есть 2 API
свой собственный и JPA

можно писать как persistence.xml так и hibernate.cfg.xml

у Hibernate действительно есть две API: собственная (native API) и JPA (Java Persistence API). Они используются для взаимодействия с базой данных, но отличаются своей спецификой.

1. Собственная API Hibernate (Hibernate Native API)
   Это оригинальный API, разработанный исключительно для Hibernate.
   Предоставляет больше возможностей и гибкости, чем JPA, включая такие функции, как:
   Кэширование второго уровня (Second Level Cache).
   Собственные запросы HQL (Hibernate Query Language).
   Более тонкий контроль за поведением сессии, транзакциями и работой с объектами.
   Используется через класс Session из org.hibernate.Session, который предоставляет методы для выполнения операций CRUD и запросов.
   Пример:

Session session = sessionFactory.openSession();
Transaction transaction = session.beginTransaction();

Employee emp = new Employee();
emp.setName("John Doe");
emp.setSalary(50000);
session.save(emp);

transaction.commit();
session.close();
2. JPA API (Java Persistence API)
   Это стандартная спецификация Java для работы с ORM (Object-Relational Mapping), поддерживаемая многими провайдерами (например, Hibernate, EclipseLink, OpenJPA).
   Абстрагируется от конкретного провайдера (например, Hibernate), что упрощает миграцию между разными реализациями JPA.
   Использует интерфейсы EntityManager, EntityTransaction и аннотации, такие как @Entity, @Table, @Column для описания моделей.
   Пример:


EntityManagerFactory emf = Persistence.createEntityManagerFactory("my-persistence-unit");
EntityManager em = emf.createEntityManager();

em.getTransaction().begin();

Employee emp = new Employee();
emp.setName("Jane Doe");
emp.setSalary(60000);
em.persist(emp);

em.getTransaction().commit();
em.close();
Основные различия:
Характеристика	            Hibernate API	                        JPA API
Спецификация	            Проприетарный API Hibernate.	        Стандартная спецификация Java.
Гибкость	                Более мощный и гибкий.	                Более ограниченный функционал.
Зависимость от Hibernate	Привязан к Hibernate.	                Может использоваться с любым JPA-провайдером.
Контроль за поведением	    Полный контроль через Session.	        Абстрагированный EntityManager.
Кэширование	                Встроенная поддержка второго уровня.	Требуется настройка через провайдер.

Вы можете использовать JPA для большей совместимости и стандартного подхода, а также подключать функции Hibernate API, когда нужно больше гибкости.

















1. Использование Hibernate API:
   С Hibernate вам действительно нужно явно создать SessionFactory и управлять его жизненным циклом. Ваш пример HibernateUtil показывает, как это делается:

Инициализация: Используется Configuration().configure() для чтения файла hibernate.cfg.xml и создания SessionFactory.

Закрытие ресурса: Поскольку SessionFactory занимает ресурсы (например, пул подключений), вы вручную закрываете его в @PreDestroy.

CDI: Вы используете аннотацию @Produces, чтобы предоставить SessionFactory для инъекции, что удобно в приложении с CDI (Contexts and Dependency Injection).

2. Использование JPA API:
   Если вы используете JPA, и ваш JPA-провайдер (например, Hibernate) интегрирован в ваше приложение, процесс сильно упрощается:

Инициализация: JPA берет на себя создание EntityManagerFactory и управление его жизненным циклом. Вы не инициализируете или закрываете ничего вручную.

Инъекция EntityManager: С помощью аннотации @PersistenceContext, контейнер сам предоставляет и управляет EntityManager. Вам даже не нужно заботиться о создании или закрытии этого объекта.

4. Основные различия:
   Hibernate API	                                                        JPA API
   Требует явной инициализации SessionFactory.	                            В контейнерах Jakarta EE можно использовать @PersistenceContext.
   Полный контроль над SessionFactory и Session.	                        Контейнер управляет EntityManagerFactory и EntityManager.
   Более сложное управление ресурсами.	                                    Простота за счет стандартизированного подхода.
   Используется только в standalone-приложениях или при явной настройке.	Удобен для Jakarta EE или Spring-приложений.

5. Рекомендация:
   Если вы используете контейнер (например, Jakarta EE или Spring): Используйте JPA с @PersistenceContext, это проще и менее подвержено ошибкам.

Если вы пишете standalone-приложение: Используйте JPA, но инициализируйте EntityManagerFactory вручную через Persistence.createEntityManagerFactory().

Hibernate API стоит использовать только если вам нужны специфичные возможности Hibernate или вы полностью исключаете переносимость на другие ORM.









Краткое сравнение подходов
Особенность	                JPA (persistence.xml)	                Hibernate API (hibernate.cfg.xml)
Стандарт	                Jakarta EE JPA	                        Hibernate-specific
Конфигурация	            persistence.xml	                        hibernate.cfg.xml
Инъекция	                @PersistenceContext (в контейнере)	    Ручное управление через SessionFactory
Переносимость	            Высокая (работает с любым ORM)	        Низкая (только Hibernate)
Сложность настройки	        Низкая (особенно в контейнерах)	        Выше (ручное управление)



---

# Hibernate

ORM (Object Realtional Mapping) - преобразование объектно-ориентированной модели в реляционную и наоборот

Hibernate - реализация ORM

Основные компоненты Hibernate

SessionFactory и Session

SessionFactory:

SessionFactory — это тяжелый объект, который создается один раз за время жизни приложения. Он настраивается с использованием конфигурационного файла (например, hibernate.cfg.xml) и отвечает за создание объектов Session.

В большинстве случаев SessionFactory инициализируется при старте приложения и используется для создания сессий в дальнейшем.

Session:

Session — это легковесный объект, который представляет собой одно соединение с базой данных. Он используется для выполнения операций с данными, таких как создание, чтение, обновление и удаление (CRUD).

Session не является потокобезопасным, поэтому для каждого потока (или запроса) следует открывать свою собственную сессию.

Сессия должна быть открыта перед выполнением операций с базой данных и закрыта после завершения работы, чтобы освободить ресурсы.

```java
SessionFactory sessionFactory = new Configuration().configure().buildSessionFactory();

try (Session session = sessionFactory.openSession()) {
    Transaction transaction = session.beginTransaction();
    
    // Выполнение операций
    MyEntity entity = new MyEntity("Example Name");
    session.save(entity);
    
    // Получение сущности
    MyEntity retrievedEntity = session.get(MyEntity.class, entity.getId());
    System.out.println("Retrieved Entity: " + retrievedEntity.getName());
    
    // Обновление сущности
    retrievedEntity.setName("Updated Name");
    session.update(retrievedEntity);
    
    // Удаление сущности
    session.delete(retrievedEntity);
    
    // Завершение транзакции
    transaction.commit();
} catch (Exception e) {
    e.printStackTrace();
    // Обработка ошибок
} finally {
    sessionFactory.close(); // Закрытие SessionFactory, если это необходимо
}
```

SessionFactory:

Это фабрика для создания объектов Session. Она является тяжелым объектом, который обычно создается один раз за время жизни приложения. SessionFactory настраивается с использованием конфигурационного файла (обычно hibernate.cfg.xml или с помощью аннотаций).

Session:

Это основной интерфейс для взаимодействия с Hibernate. Он представляет собой одно соединение с базой данных и используется для выполнения операций CRUD (Create, Read, Update, Delete). Session является временным и не является потокобезопасным.

Transaction:

Этот интерфейс используется для управления транзакциями. Все операции с базой данных должны быть обернуты в транзакцию для обеспечения целостности данных.

Query:

Интерфейс для выполнения запросов к базе данных. Hibernate поддерживает HQL (Hibernate Query Language) и Criteria API для создания запросов.

Entity:

Классы, которые представляют таблицы в базе данных. Каждый экземпляр класса соответствует одной записи в таблице.

2. Основные этапы работы Hibernate

   2.1. Настройка Hibernate

   Перед тем как использовать Hibernate, необходимо настроить его. Это включает в себя создание файла конфигурации (обычно hibernate.cfg.xml), где указываются параметры подключения к базе данных, диалект SQL и другие настройки.

   Пример конфигурационного файла:

    ```xml
    <hibernate-configuration>
        <session-factory>
            <property name="hibernate.dialect">org.hibernate.dialect.MySQLDialect</property>
            <property name="hibernate.connection.driver_class">com.mysql.cj.jdbc.Driver</property>
            <property name="hibernate.connection.url">jdbc:mysql://localhost:3306/mydb</property>
            <property name="hibernate.connection.username">user</property>
            <property name="hibernate.connection.password">password</property>
            <property name="hibernate.hbm2ddl.auto">update</property>
        </session-factory>
    </hibernate-configuration>
    ```

   2.2. Создание SessionFactory

   После настройки Hibernate, необходимо создать объект SessionFactory:

    ```java
    import org.hibernate.Session;
    import org.hibernate.SessionFactory;
    import org.hibernate.Transaction;
    import org.hibernate.cfg.Configuration;

    import javax.persistence.*;

    @Entity
    @Table(name = "my_entity")
    class MyEntity {
        @Id
        @GeneratedValue(strategy = GenerationType.IDENTITY)
        private Long id;

        @Column(name = "name")
        private String name;

        // Конструкторы, геттеры и сеттеры
        public MyEntity() {}

        public MyEntity(String name) {
            this.name = name;
        }

        public Long getId() {
            return id;
        }

        public void setId(Long id) {
            this.id = id;
        }

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }
    }

    public class HibernateExample {
        public static void main(String[] args) {
            // Создание SessionFactory
            SessionFactory sessionFactory = new Configuration().configure().buildSessionFactory();

            // Открытие сессии
            Session session = sessionFactory.openSession();
            Transaction transaction = null;

            try {
                // Начало транзакции
                transaction = session.beginTransaction();

                // Создание новой сущности
                MyEntity entity = new MyEntity("Example Name");
                session.save(entity); // Сохранение сущности

                // Получение сущности
                MyEntity retrievedEntity = session.get(MyEntity.class, entity.getId());
                System.out.println("Retrieved Entity: " + retrievedEntity.getName());

                // Обновление сущности
                retrievedEntity.setName("Updated Name");
                session.update(retrievedEntity); // Обновление сущности

                // Удаление сущности
                session.delete(retrievedEntity); // Удаление сущности

                // Завершение транзакции
                transaction.commit();
            } catch (Exception e) {
                if (transaction != null) {
                    transaction.rollback(); // Откат транзакции в случае ошибки
                }
                e.printStackTrace();
            } finally {
                session.close(); // Закрытие сессии
                sessionFactory.close(); // Закрытие SessionFactory
            }
        }
    }
    ```



3. Кэширование в Hibernate

   Hibernate поддерживает кэширование на двух уровнях:

   Первичный кэш (Session Cache):

   Это кэш, который хранит объекты в пределах одной сессии. Когда вы загружаете объект, он помещается в первичный кэш, и при последующих запросах к тому же объекту в рамках той же сессии Hibernate будет возвращать его из кэша, а не из базы данных.

   Вторичный кэш (Second Level Cache):

   Это кэш, который может использоваться между сессиями. Он может быть настроен для использования различных кэш-провайдеров, таких как EHCache, Infinispan и других. Вторичный кэш позволяет уменьшить количество обращений к базе данных, сохраняя часто запрашиваемые данные в памяти.

4. Механизм отображения (Mapping)

   Hibernate использует механизмы отображения для связы зки объектов Java с таблицами базы данных. Это может быть сделано с помощью XML-файлов или аннотаций. Основные типы отображения включают:

   @Entity: Обозначает класс как сущность, которая будет отображаться на таблицу в базе данных.

   @Table: Указывает имя таблицы, с которой будет связана сущность.

   @Id: Указывает первичный ключ сущности.

   @GeneratedValue: Определяет стратегию генерации значений для первичного ключа.

   @Column: Позволяет настроить отображение полей класса на столбцы таблицы.

5. Обработка ассоциаций

   Hibernate поддерживает различные типы ассоциаций между сущностями:

   One-to-One: Одна сущность связана с одной другой сущностью.

   One-to-Many: Одна сущность связана с множеством других сущностей.

   Many-to-One: Множество сущностей связано с одной сущностью.

   Many-to-Many: Множество сущностей связано с множеством других сущностей.

   Пример ассоциации One-to-Many:

    ```java
    @Entity
    public class Parent {
        @Id
        @GeneratedValue(strategy = GenerationType.IDENTITY)
        private Long id;

        @OneToMany(mappedBy = "parent")
        private List<Child> children;

        // Геттеры и сеттеры
    }

    @Entity
    public class Child {
        @Id
        @GeneratedValue(strategy = GenerationType.IDENTITY)
        private Long id;

        @ManyToOne
        @JoinColumn(name = "parent_id")
        private Parent parent;

        // Геттеры и сеттеры
    }
    ```




Общая архитектура

Конфигурация Hibernate:

Обычно вы создаете конфигурацию Hibernate в отдельном классе или файле конфигурации (например, hibernate.cfg.xml или с помощью Java-конфигурации). Если вы используете Spring, вы можете настроить SessionFactory с помощью аннотаций.

DAO (Data Access Object):

DAO-слой отвечает за взаимодействие с базой данных. В DAO-классе вы можете использовать Session для выполнения операций с базой данных.

Сервисный слой:

Сервисный слой может использовать DAO для выполнения бизнес-логики. Это помогает отделить бизнес-логику от доступа к данным.

Пример с использованием Spring

Если вы используете Spring, вот пример того, как можно организовать инициализацию Hibernate и использование сессий в DAO-классе.

1. Конфигурация Spring и Hibernate

Создайте файл конфигурации Spring (например, applicationContext.xml):

```xml
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xsi:schemaLocation="http://www.springframework.org/schema/beans
                           http://www.springframework.org/schema/beans/spring-beans.xsd">

    <bean id="dataSource" class="org.apache.commons.dbcp.BasicDataSource">
        <property name="driverClassName" value="com.mysql.cj.jdbc.Driver"/>
        <property name="url" value="jdbc:mysql://localhost:3306/mydb"/>
        <property name="username" value="your_username"/>
        <property name="password" value="your_password"/>
    </bean>

    <bean id="sessionFactory" class="org.springframework.orm.hibernate5.LocalSessionFactoryBean">
        <property name="dataSource" ref="dataSource"/>
        <property name="packagesToScan" value="com.example.model"/> <!-- Пакет с вашими сущностями -->
        <property name="hibernateProperties">
            <props>
                <prop key="hibernate.dialect">org.hibernate.dialect.MySQLDialect</prop>
                <prop key="hibernate.hbm2ddl.auto">update</prop>
            </props>
        </property>
    </bean>

    <bean id="transactionManager" class="org.springframework.orm.hibernate5.HibernateTransactionManager">
        <property name="sessionFactory" ref="sessionFactory"/>
    </bean>

    <tx:annotation-driven transaction-manager="transactionManager"/>
</beans>
```

2. DAO-класс

Создайте DAO-класс, который будет использовать Session для выполнения операций с базой данных:

```java
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Repository;
import org.springframework.transaction.annotation.Transactional;

@Repository
public class MyEntityDAO {

    @Autowired
    private SessionFactory sessionFactory;

    // Используем аннотацию @Transactional для управления транзакциями
    @Transactional
    public void save(MyEntity entity) {
        Session session = sessionFactory.getCurrentSession();
        session.save(entity);
    }

    @Transactional(readOnly = true)
    public MyEntity findById(Long id) {
        Session session = sessionFactory.getCurrentSession();
        return session.get(MyEntity.class, id);
    }

    @Transactional
    public void update(MyEntity entity) {
        Session session = sessionFactory.getCurrentSession();
        session.update(entity);
    }

    @Transactional
    public void delete(MyEntity entity) {
        Session session = sessionFactory.getCurrentSession();
        session.delete(entity);
    }
}
```

3. Сервисный класс

Создайте сервисный класс, который будет использовать DAO для выполнения бизнес-логики:

```java
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class MyEntityService {

    @Autowired
    private MyEntityDAO myEntityDAO;

    @Transactional
    public void createEntity(String name) {
        MyEntity entity = new MyEntity(name);
        myEntityDAO.save(entity);
    }

    @Transactional(readOnly = true)
    public MyEntity getEntity(Long id) {
        return myEntityDAO.findById(id);
    }

    @Transactional
    public void updateEntity(MyEntity entity) {
        myEntityDAO.update(entity);
    }

    @Transactional
    public void deleteEntity(MyEntity entity) {
        myEntityDAO.delete(entity);
    }
}
```

Таким образом, в enterprise-приложениях рекомендуется использовать паттерны проектирования и фреймворки, такие как Spring, для управления сессиями Hibernate и инициализации `SessionFactory`. Это позволяет организовать код более структурированно, отделяя бизнес-логику от доступа к данным и обеспечивая удобное управление транзакциями. Использование аннотаций, таких как `@Transactional`, упрощает работу с транзакциями и сессиями, позволяя вам сосредоточиться на бизнес-логике, а не на управлении ресурсами.


**Сессии**

```java
Session session = sessionFactory.openSession();
Transaction transaction = null;
    
    try{
        transaction = session.beginTransaction();
        
        /**
         * Here we make some work.
         * */
        
        transaction.commit();
    }catch(Exception e){
        if(transaction !=null){
            transaction.rollback();
            e.printStackTrace();
        }
        e.printStackTrace();
    }finally{
        session.close();
    }
```

В интерфейсе Session определены 23 метода, которые мы можем использовать:


Transaction beginTransaction()
Начинает транзакцию и возвращает объект Transaction.

void cancelQuery()
Отменяет выполнение текущего запроса.

void clear()
Полностью очищает сессию

Connection close()
Заканчивает сессию, освобождает JDBC-соединение и выполняет очистку.

Criteria createCriteria(String entityName)
Создание нового экземпляра Criteria для объекта с указанным именем.

Criteria createCriteria(Class persistentClass)
Создание нового экземпляра Criteria для указанного класса.

Serializable getIdentifier(Object object)
Возвращает идентификатор данной сущности, как сущности, связанной с данной сессией.

void update(String entityName, Object object)
Обновляет экземпляр с идентификатором, указанном в аргументе.

void update(Object object)
Обновляет экземпляр с идентификатором, указанном в аргументе.

void saveOrUpdate(Object object)
Сохраняет или обновляет указанный экземпляр.

Serializable save(Object object)
Сохраняет экземпляр, предварительно назначив сгенерированный идентификатор.

boolean isOpen()
Проверяет открыта ли сессия.

boolean isDirty()
Проверят, есть ли в данной сессии какие-либо изменения, которые должны быть синхронизованы с базой данных (далее – БД).

boolean isConnected()
Проверяет, подключена ли сессия в данный момент.

Transaction getTransaction()
Получает связанную с этой сессией транзакцию.

void refresh(Object object)
Обновляет состояние экземпляра из БД.

SessionFactory getSessionFactory()
Возвращает фабрику сессий (SessionFactory), которая создала данную сессию.

Session get(String entityName, Serializable id)
Возвращает сохранённый экземпляр с указанными именем сущности и идентификатором. Если таких сохранённых экземпляров нет – возвращает null.

void delete(String entityName, Object object)
Удаляет сохранённый экземпляр из БД.

void delete(Object object)
Удаляет сохранённый экземпляр из БД.

SQLQuery createSQLQuery(String queryString)
Создаёт новый экземпляр SQL-запроса (SQLQuery) для данной SQL-строки.

Query createQuery(String queryString)
Создаёт новый экземпляр запроса (Query) для данной HQL-строки.

Query createFilter(Object collection, String queryString)
Создаёт новый экземпляр запроса (Query) для данной коллекции и фильтра-строки.

---

Ключевая функция Hibernate заключается в том, что мы можем взять занчения из нашего Java-класса и созранить их в таблице базы данных (далее – БД). С помощью конфигурационных файлов мы указываем Hibernate как извлечь данные из класса и соединить с определённым столбцами в таблице БД.

Если мы хотим, чтобы экземпляры (объекты) Java-класса в будущем созранялся в таблице БД, то мы называем их “сохранямые классы” (persistent class). Для того, чтобы сделать работу с Hibernate аксимально удобной и эффективной, мы следует использовать программную молель Простых Старых Java Объектов (Plain Old Java Object – POJO).

Короче Persistent classes - Постоянные Классы - просто классы @Entity сущности которые мапятся на бд

как мапить java класс на таблицу бд

1) xml
2) аннотации

Пример:

```xml
-Developer.hbm.xml
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE hibernate-mapping PUBLIC
        "-//Hibernate/Hibernate Mapping DTD//EN"
        "http://www.hibernate.org/dtd/hibernate-mapping-3.0.dtd">
<hibernate-mapping>
    <class name="net.proselyte.hibernate.example.model.Developer" table="HIBERNATE_DEVELOPERS">
        <meta attribute="class-description">
            This class contains developer's details.
        </meta>
        <id name="id" type="int" column="ID">
            <generator class="native"/>
        </id>
        <property name="firstName" column="FIRST_NAME" type="string"/>
        <property name="lastName" column="LAST_NAME" type="string"/>
        <property name="specialty" column="SPECIALTY" type="string"/>
        <property name="experience" column="EXPERIENCE" type="int"/>
    </class>
</hibernate-mapping>
```

```xml
- hibernate.cfg.xml
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE hibernate-configuration SYSTEM
"http://www.hibernate.org/dtd/hibernate-configuration-3.0.dtd">

<hibernate-configuration>
<session-factory>
<property name="hibernate.dialect">
org.hibernate.dialect.MySQLDialect
</property>
<property name="hibernate.connection.driver_class">
com.mysql.jdbc.Driver
</property>

<!-- Assume ИМЯ ВАШЕЙ БД is the database name -->
<property name="hibernate.connection.url">
jdbc:mysql://localhost/ИМЯ_ВАШЕЙ_БАЗЫ_ДАННЫХ
</property>
<property name="hibernate.connection.username">
ВАШЕ ИМЯ ПОЛЬЗОВАТЕЛЯ
</property>
<property name="hibernate.connection.password">
ВАШ ПАРОЛЬ
</property>

<!-- List of XML mapping files -->
<mapping resource="Developer.hbm.xml"/>

</session-factory>
</hibernate-configuration>
```

```java
public class Developer {
    private int id;
    private String firstName;
    private String lastName;
    private String specialty;
    private int experience;

    /**
     * Default Constructor
     */
    public Developer() {
    }

    /**
     * Plain constructor
     */
    public Developer(int id, String firstName, String lastName, String specialty, int experience) {
        this.id = id;
        this.firstName = firstName;
        this.lastName = lastName;
        this.specialty = specialty;
        this.experience = experience;
    }


    /**
     * Getters and Setters
     */
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getFirstName() {
        return firstName;
    }

    public void setFirstName(String firstName) {
        this.firstName = firstName;
    }

    public String getLastName() {
        return lastName;
    }

    public void setLastName(String lastName) {
        this.lastName = lastName;
    }

    public String getSpecialty() {
        return specialty;
    }

    public void setSpecialty(String specialty) {
        this.specialty = specialty;
    }

    public int getExperience() {
        return experience;
    }

    public void setExperience(int experience) {
        this.experience = experience;
    }

    /**
     * toString method (optional)
     */
    @Override
    public String toString() {
        return "Developer{" +
                "id=" + id +
                ", firstName='" + firstName + '\'' +
                ", lastName='" + lastName + '\'' +
                ", specialty='" + specialty + '\'' +
                ", experience=" + experience +
                '}';
    }
}


public class DeveloperRunner {
    private static SessionFactory sessionFactory;

    public static void main(String[] args) {
        sessionFactory = new Configuration().configure().buildSessionFactory();

        DeveloperRunner developerRunner = new DeveloperRunner();

        System.out.println("Adding developer's records to the DB");
        /**
         *  Adding developer's records to the database (DB)
         */
        developerRunner.addDeveloper("Proselyte", "Developer", "Java Developer", 2);
        developerRunner.addDeveloper("Some", "Developer", "C++ Developer", 2);
        developerRunner.addDeveloper("Peter", "UI", "UI Developer", 4);

        System.out.println("List of developers");
        /**
         * List developers
         */
        List developers = developerRunner.listDevelopers();
        for (Developer developer : developers) {
            System.out.println(developer);
        }
        System.out.println("===================================");
        System.out.println("Removing Some Developer and updating Proselyte");
        /**
         * Update and Remove developers
         */
        developerRunner.updateDeveloper(10, 3);
        developerRunner.removeDeveloper(11);

        System.out.println("Final list of developers");
        /**
         * List developers
         */
        developers = developerRunner.listDevelopers();
        for (Developer developer : developers) {
            System.out.println(developer);
        }
        System.out.println("===================================");

    }

    public void addDeveloper(String firstName, String lastName, String specialty, int experience) {
        Session session = sessionFactory.openSession();
        Transaction transaction = null;

        transaction = session.beginTransaction();
        Developer developer = new Developer(firstName, lastName, specialty, experience);
        session.save(developer);
        transaction.commit();
        session.close();
    }

    public List listDevelopers() {
        Session session = this.sessionFactory.openSession();
        Transaction transaction = null;

        transaction = session.beginTransaction();
        List developers = session.createQuery("FROM Developer").list();

        transaction.commit();
        session.close();
        return developers;
    }

    public void updateDeveloper(int developerId, int experience) {
        Session session = this.sessionFactory.openSession();
        Transaction transaction = null;

        transaction = session.beginTransaction();
        Developer developer = (Developer) session.get(Developer.class, developerId);
        developer.setExperience(experience);
        session.update(developer);
        transaction.commit();
        session.close();
    }

    public void removeDeveloper(int developerId) {
        Session session = this.sessionFactory.openSession();
        Transaction transaction = null;

        transaction = session.beginTransaction();
        Developer developer = (Developer) session.get(Developer.class, developerId);
        session.delete(developer);
        transaction.commit();
        session.close();
    }

}

```

Виды Mapping

Связи в ORM деляся на 3 гурппы:

- Связывание коллекций
- Ассоциативное связывание
- Связывание компоннетов

Рассмотрим каждую из них:

Связывание коллекций:

Если среди значений класса есть коллекции (collections) каких-либо значений, мы можем связать (map) их с помощью любого интерфейса коллекций, доступных в Java.

В Hibernate мы можем оперировать следующими коллекциями:

- java.util.List
- java.util.Collection
- java.util.Set
- java.util.SortedSet
- java.util.Map
- java.util.SortedMap

Ассоциативное связывание:

Связывание ассоциаций  – это связывание (mapping) классов и отношений между таблицами в БД. Сущействует 4 типа таких зависимостей:

- Many-to-One
- One-to-One
- One-to-Many
- Many-to-One

Связывание компонентов:

Возможна ситуация, при которой наш Java – класс имеет ссылку на другой класс, как одну из переменных. Если класс, на который мы ссылаемся не имеет своего собственного жизненного цикла и полностью зависит от жизненного цикла класса, который на него ссылается, то класс, на который ссылаются называется классом Компонентом (Component Class).


АННОТАЦИИ

Обязательными аннотациями являются следующие:



@Entity

Эта аннотация указывает Hibernate, что данный класс является сущностью (entity bean). Такой класс должен иметь конструктор по-умолчанию (пустой конструктор).

@Table

С помощью этой аннотации мы говорим Hibernate,  с какой именно таблицей необходимо связать (map) данный класс. Аннотация @Table имеет различные аттрибуты, с помощью которых мы можем указать имя таблицы, каталог, БД и уникальность столбцов в таблец БД.

@Id

С помощью аннотации @Id мы указываем первичный ключ (Primary Key) данного класса.

@GeneratedValue

Эта аннотация используется вместе с аннотацией @Id и определяет такие паметры, как strategy и generator.

@Column

Аннотация @Column определяет к какому столбцу в таблице БД относится конкретное поле класса (аттрибут класса).

Наиболее часто используемые аттрибуты аннотации @Column такие:

name
Указывает имя столбца в таблице

unique
Определяет, должно ли быть данноезначение уникальным

nullable
Определяет, может ли данное поле быть NULL, или нет.

length
Указывает, какой размер столбца (например колчиство символов, при использовании String).


HQL - HIBERNATE QUERY LANGUAGE

HQL (Hibernate Query Language) – это объекто-ориентированный (далее – ОО) язык запросов, который крайне похож на SQL.

Отличие между HQL и SQL состоит в том, что SQL работает таблицами в базе данных (далее – БД) и их столбацами, а HQL – с сохраняемыми объектами (Persistent Objects) и их полями (аттрибутами класса).

Hibernate трнаслирует HQL – запросы в понятные для БД SQL – запросы, которые и выполняют необходимые нам действия в БД.

Мы также имеем возможность испольщовать обычные SQL – запросы в Hibernate используя Native SQL, но использование HQL является более предпочтительным.

Давайте рассмотрим основные ключевые слова языка HQL:

FROM

Если мы хотим загрузить в память наши сохраняемые объекты, то мы будем использовать ключевое слово FROM. Вот пример его использования:

```java
Query query = session.createQuery("FROM Developer");
List developers = query.list();
```

Developer – это POJO – класс Developer.java, который ассоцииргван с таблицей в шагей БД.

INSERT
Мы используем ключевое слово INSERT, в том случае, если хотим добавить запись в таблицу нашей БД.

Вот пример использования этого ключевого слова:

```java
 Query query = 
           session.createQuery("INSERT INTO Developer (firstName, lastName, specialty, experience)"); 
```

UPDATE
Ключевое слово UPDATE используется для обновления одного или нескольких полей объекта. Вот так это выглядит на практике:

```java
   Query query = 
      session.createQuery(UPDATE Developer SET experience =: experience WHERE id =: developerId);
   query.setParameter("expericence", 3);
```


DELETE
Это ключевоеслово используется для удаления одного или нескольких объектов. Пример использования:

```java
   Query query = session.createQuery("DELETE FROM Developer WHERE id = :developerId");
   query.setParameter("developerId", 1);
```


SELECT
Если мы хотим получить запись из таблицы нашей БД, то мы должны использоваьб ключевое слово SELECT. Пример использования:

```java
   Query query = session.createQuery("SELECT D.lastName FROM Developer D");
   List developers = query.list();
```


AS
В предыдущем примере мы использовали запись формы Developer D. С использованием опционального ключевого слова AS это будет выглядеть так:

```java
   Query query = session.createQuery("FROM Developer AS D");
   List developers = query.list();
```


WHERE
В том случае, если мы хотим получить объекты, которые соответствуют опредлённым параметрам, то мы должны использовать ключевое слово WHERE. На практике это выглядит следующим образом:

```java
   Query query = session.createQuery("FROM Developer D WHERE D.id = 1");
   List developer = query.list();
```


ORDER BY
Жля того, чтобы отсортировать список обхектов, полученных в результате запроса, мы должны применить ключевое слово ORDER BY. Нам необходимо укаать параметр, по которому список будет отсортирован и тип сортировки – по возрастанию (ASC) или по убыванию (DESC). В виде кода это выгллядит так:

```java
   Query query = 
        session.createQuery("FROM Developer D WHERE experience > 3 ORDER BY D.experience DESC");
```


GROUP BY
С помощью ключевого слова GROUP BY мы можем группировать данные, полученные из БД по какому-либо признаку. Вот простой пример применения данного ключевого слова:

```java
   Query query = session.createQuery("SELECT MAX(D.experience), D.lastName, D.specialty FROM Developer D GROUP BY D.lastName");
   List developers = query.list();
```

Методы аггрегации
Язык запросов Hibernate (HQL) пожжерживает различные методы аггрегации, которые доступны и в SQL. HQL поддреживает слудующие методы:

min(имя свойства)

Мнимальное значение данного свойства.

max(имя свойства)

Максимальное занчение данного свойтсва.

sum(имя свойства)

Сумма всех занчений данного свойтсва.

avg(имя свойства)

Среднее арифметическое всех значений данного свойства

count(имя свойства)

Какое количество раз данное свойство встречается в результате.

В данной статье мы ознакомились с основами языка запросов Hibernate (HQL).


КЭШИРОВАНИЕ - 3 уровня




---


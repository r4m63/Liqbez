# Spring Framework

## Фреймворки внутри

- [Spring Core](./spring-core.md)
- [Spring Web MVC](./spring-mvc.md)
- [Spring Data](./spring-data.md)
- [Spring Security](./spring-security.md)
- [Spring Boot](./spring-boot.md)
- Spring WebFlux
- Spring Cloud


## Идейные отличия от Java EE

**Кто хозяин: сервер vs приложение**

Java EE:

- IoC - часть контейнера приложения (WildFly, Payara, WebLogic, WebSphere, TomEE).
- Деплой war/ear внутрь уже готового окружения.
- DI даётся спецификациями:
    - CDI (@Inject, @Named, @ApplicationScoped...)
    - @EJB, @Resource.
- Контейнер создаёт и управляет бинами как часть сервера.

Spring:

- IoC-контейнер - часть самого приложения.
- Собирается jar (Spring Boot) и просто запускается: java -jar app.jar.
- Внутри этого процесса поднимается ApplicationContext, и уже он создаёт бины.
- Никакого внешнего сервер-приложений не нужно (Tomcat/Jetty - просто встроенная библиотека).

**Модель IoC/DI**

Java EE (CDI):

- DI стандартизован (jakarta.inject, jakarta.enterprise.cdi).
- Основные фишки:
    - @Inject для полей/конструкторов/сеттеров.
    - @Qualifier для выбора между реализациями.
    - @Scope (@RequestScoped, @SessionScoped, @ApplicationScoped...).
    - Интерсепторы (@Interceptor, @AroundInvoke), декораторы, events.
- Контейнер CDI накрывает только то, что видит как CDI-бины (архив, beans.xml, discovery mode и т.п.).

Spring:

- Свой IoC/DI: ApplicationContext, BeanFactory.
- Основные фишки:
    - @Component/@Service/@Repository/@Controller, @Bean.
    - @Autowired, @Qualifier, @Primary.
    - Богатая конфигурация через Java-код (@Configuration), аннотации, properties.
    - Свой AOP (@Transactional, @Async, @Cacheable и т.п. поверх прокси).

Разница в подходе:

- Java EE: спека задаёт минимум, остальное vendor.
- Spring: всё под одной крышей Spring, максимум фич в одном контейнере.

**Связь с окружением**

Java EE:

- Контейнер управляет:
    - транзакциями (JTA),
    - пулом коннекшенов,
    - JMS, JPA, security, JNDI-ресурсами.
    - DI завязан на ресурсы сервера:
    - @PersistenceContext,
    - @Resource(lookup="jdbc/..."),
    - @EJB и т.д.

Spring:

- Сам поднимает:
    - DataSource (HikariCP и т.п.),
    - EntityManagerFactory,
    - транзакционный менеджер,
    - JMS-templates, Kafka-listeners и т.п.
- Всё конфигуришь внутри Spring-контекста, независимо от серверной платформы.

- Java EE IoC = впаян в контейнер приложения и его ресурсы.
- Spring IoC = самодостаточный мир, который можно запустить где угодно (jar, Docker, k8s, обычный Tomcat).

В Java EE: Сервер уже всё умеет, я просто описываю бины и даю аннотации @Inject, @EJB, @Resource - контейнер сам
разрулит.

В Spring: Я сам собираешь всё приложение из бинов, Spring-контейнер - это конструктор, куда я стаскиваю всё, что нужно,
и контролирую поведение.

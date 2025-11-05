# Spring Framework

Свой IoC и CDI
использует инфраструктурные решения Java / Jakarta EE

Идейные отличия от Java EE

- Концепция Java EE - разделение обязанностей между контейнером и компонентом; Концепция Spring - IoC / CDI.
- Контейнер в Java EE включает в себя приложение; приложение в Spring включает в себя контейнер.
- Java EE - спецификация; Spring - фреймворк.

---
РАСПИСАТЬ
РАСПИСАТЬ
РАСПИСАТЬ
РАСПИСАТЬ
1. Кто хозяин: сервер vs приложение

Java EE / Jakarta EE
• IoC — часть контейнера приложения (WildFly, Payara, WebLogic, WebSphere, TomEE и т.п.).
• Ты деплоишь war/ear “внутрь” уже готового окружения.
• DI даётся спецификациями:
• CDI (@Inject, @Named, @ApplicationScoped …)
• @EJB, @Resource и т.п.
• Контейнер создаёт и управляет твоими бинами как часть сервера.

Spring
• IoC-контейнер — часть самого приложения.
• Ты собираешь jar (Spring Boot) и просто запускаешь: java -jar app.jar.
• Внутри этого процесса поднимается ApplicationContext, и уже он создаёт бины.
• Никакого внешнего “сервер-приложений” не нужно (Tomcat/Jetty — просто встроенная библиотека).

👉 Подход:
• Java EE: “у нас есть мощный сервер, в него закидываем приложения”.
• Spring: “у нас есть автономное приложение, внутри него живёт собственный контейнер”.

⸻

2. Модель IoC/DI

Java EE (CDI)
• DI стандартизован (jakarta.inject, jakarta.enterprise.cdi).
• Основные фишки:
• @Inject для полей/конструкторов/сеттеров.
• @Qualifier для выбора между реализациями.
• @Scope (@RequestScoped, @SessionScoped, @ApplicationScoped, …).
• Интерсепторы (@Interceptor, @AroundInvoke), декораторы, events.
• Контейнер CDI “накрывает” только то, что видит как CDI-бины (архив, beans.xml, discovery mode и т.п.).

Spring
• Свой IoC/DI: ApplicationContext, BeanFactory.
• Основные фишки:
• @Component/@Service/@Repository/@Controller, @Bean.
• @Autowired, @Qualifier, @Primary.
• Богатая конфигурация через Java-код (@Configuration), аннотации, properties.
• Свой AOP (@Transactional, @Async, @Cacheable и т.п. поверх прокси).

👉 Разница в подходе:
• Java EE: “спека задаёт минимум, остальное vendor”.
• Spring: “всё под одной крышей Spring, максимум фич в одном контейнере”.

⸻

3. Связь с окружением

Java EE
• Контейнер управляет:
• транзакциями (JTA),
• пулом коннекшенов,
• JMS, JPA, security, JNDI-ресурсами.
• DI завязан на ресурсы сервера:
• @PersistenceContext,
• @Resource(lookup="jdbc/..."),
• @EJB и т.д.

Spring
• Сам поднимает:
• DataSource (HikariCP и т.п.),
• EntityManagerFactory,
• транзакционный менеджер,
• JMS-templates, Kafka-listeners и т.п.
• Всё конфигуришь внутри Spring-контекста, независимо от “серверной платформы”.

👉 Java EE IoC = “впаян” в контейнер приложения и его ресурсы.
👉 Spring IoC = “самодостаточный мир”, который ты можешь запустить где угодно (jar, Docker, k8s, обычный Tomcat).

⸻

4. Исторически по ощущениям разработчика
   • В Java EE:
   “Сервер уже всё умеет, я просто описываю бины и даю аннотации @Inject, @EJB, @Resource — контейнер сам разрулит”.
   • В Spring:
   “Я сам собираю всё приложение из бинов, Spring-контейнер — это конструктор, куда я стаскиваю всё, что нужно, и
   контролирую поведение”.

⸻

Если грубо в одну фразу:

Java EE IoC – DI как часть “большого стандартизованного сервера приложений”.
Spring IoC – DI как часть самого приложения, с огромной экосистемой вокруг одного контейнера.
---

Фреймворки внутри

- Spring Core
- Spring Web MVC
- Spring WebFlux
- Spring Data
- Spring Security
- Spring Cloud
- Spring Boot





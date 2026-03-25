# Spring Core

## Бины в Spring

**Scope (жизненный цикл)**

- singleton (по умолчанию) - Один объект на весь ApplicationContext. 99% обычных бинов.
- prototype - Новый объект при каждом getBean() / каждом инжекте (лениво). Удобно для одноразовых штук.
- request (web) - Один объект на HTTP-запрос.
- session (web) - Один объект на HTTP-сессию.
- application (web) - Один объект на ServletContext (на всё веб-приложение).
- websocket (websocket-приложения) - Один объект на WebSocket-сессию.
- Кастомные scope

**Роли (для читаемости)**

Все они по сути `@Component`, но с разным смыслом:
Это не разные типы объектов для контейнера, а просто удобные 'метки', но их тоже часто называют 'типами бинов'

- `@Component` - базовый обычный бин.
- `@Service` - сервисный слой (бизнес-логика).
- `@Repository` - DAO/репозиторий (доступ к БД, перевод исключений).
- `@Controller` - MVC-контроллер (возвращает view/модель).
- `@RestController` - REST-контроллер (JSON/XML).
- `@Configuration` - класс с @Bean-методами (конфигурация).

**По способу объявления**

- Component-scan бины: `@Component`, `@Service`, `@Repository`, `@Controller` и т.п.
- Java-config бины: методы с `@Bean` в `@Configuration` классах.

## IoC, DI и Context

IoC-контейнер в Spring - это обычный Java-объект(ы), который живёт в памяти JVM и управляет всеми твоими бинами

IoC-контейнер = BeanFactory

`ApplicationContext ctx = new AnnotationConfigApplicationContext(AppConfig.class);` - тут `ctx` и есть IoC-контейнер

Контекст = настроенный и запущенный IoC-контейнер + окружение приложения

`BeanFactory` - базовый минимальный интерфейс IoC-контейнера. Он знает как создать бин и как внедрить зависимости, но
почти ничего не делает сам по себе. Всё магическое поведение (аннотации, автосвязывание, прокси, транзакции)
возможно только если вы вручную подключили соответствующие пост-процессоры.

Минимальный IoC-контейнер:

- Создание и DI бинов.
- Lazy-инициализация.
- Мало инфраструктуры.
- Хранит метаданные бинов (BeanDefinition) и создаёт экземпляры.
- Делает DI (конструктор/сеттер/поле) - если есть зарегистрированный обработчик.
- Управляет скоупами и разрушением бинов.

Важные подинтерфейсы/реализации:

- ListableBeanFactory - перечисление бинов по типу/имени.
- HierarchicalBeanFactory - поддержка родительской фабрики.
- AutowireCapableBeanFactory - программное автосвязывание, пост-процессоры.
- DefaultListableBeanFactory - главная рабочая лошадь (конкретная реализация, которой пользуется Spring под капотом).

Обычно с чистым BeanFactory напрямую не взаимодействуют. Но полезно понимать, что любой ApplicationContext содержит
внутри DefaultListableBeanFactory.

---

`ApplicationContext` расширяет BeanFactory и добавляет платформенные сервисы. Это - стандартный выбор в реальных
приложениях (включая Spring Boot).
Это и есть контейнер Spring, который держит бины, их зависимости и окружение, наследуется от BeanFactory

Это объект, который:

- хранит описание всех бинов (метаданные),
- создаёт и хранит экземпляры бинов,
- управляет их жизненным циклом и зависимостями,
- даёт инфраструктурные сервисы (события, i18n, доступ к ресурсам, профили и т.д.).

Он всегда содержит внутри BeanFactory, но поверх этого предоставляет:

- Авто-подключение инфраструктуры: сам находит и регистрирует все BeanFactoryPostProcessor и BeanPostProcessor из
  контекста (поэтому заводится @Autowired, @Configuration, AOP, транзакции и т.д. без вашего вмешательства).
- События: публикация/подписка (ApplicationEventPublisher, @EventListener).
- Ресурсы: Resource/ResourcePatternResolver (classpath:, file:, classpath*:).
- i18n: MessageSource (строки локализуются через messages.properties).
- Профили и окружение: Environment, PropertySources, @Profile.
- Pre-instantiate singletons: по refresh() заранее создаёт все singleton-бины (если не помечены @Lazy). Это даёт быстрый
  fail-fast при старте.

ApplicationContext – расширение BeanFactory:

- Управление ресурсами (Resource).
- События (ApplicationEventPublisher).
- Сообщения / i18n.
- Автоматическая регистрация многих BeanPostProcessor.
- Интеграция с Spring AOP, транзакциями и т.д.

В реальных приложениях почти всегда работаешь с ApplicationContext.

Частые реализации

- AnnotationConfigApplicationContext - Java/аннотации, non-web.
- ClassPathXmlApplicationContext - XML (наследие, актуально для старых проектов).
- Web: WebApplicationContext - контекст, привязанный к ServletContext (скоупы request, session, application, websocket).
- Spring Boot: под капотом - AnnotationConfigServletWebServerApplicationContext (или близкие), вы их обычно не трогаете
  напрямую. SpringApplication.run(...) возвращает ConfigurableApplicationContext.

Когда нужен бин - он запрашивается у ApplicationContext;
До Spring Framework 4.3 нужно было явно указывать аннотацию @Autowired;
По умолчанию - внедрение через конструктор;
Используя @Autowired - можно внедрять через setter;
Также поддерживаются аннотации из JSR-330 - @Inject, @Named.

Два способа конфигурации:

- xml - устаревший вариант;
- annotations - при помощи сканирования;

При сканировании выполняется поиск бинов, помеченных `@Component` и `@Configuration`.
В `@Configuration` классах можно объявить методы-провайдеры бинов, пометив их аннотацией `@Bean`.
`@ComponentScan` - для указания пакетов, в которых нужно выполнить сканирование.

Какие контексты бывают

По сути, это разные реализации ApplicationContext под разные задачи.

Обычные (non-web)

- AnnotationConfigApplicationContext - Java/аннотационный конфиг без веба: консольные утилиты, standalone-приложения.
- ClassPathXmlApplicationContext, FileSystemXmlApplicationContext - Старый стиль через XML.

Их задача – просто поднять IoC-контейнер, без HTTP, сессий и т.п.

Веб-контекст

- WebApplicationContext - Подтип ApplicationContext, привязан к ServletContext (Java EE/Servlet container).
  Умеет:
    - регистрировать веб-скоупы (request, session, application, websocket),
    - работать с сервлет-окружением,
    - часто используется вместе с DispatcherServlet.

В классической Spring MVC-конфигурации обычно есть:

- Root WebApplicationContext - Контекст уровня приложения (репозитории, сервисы, безопасность и т.д.).
- Контекст DispatcherServlet
    - Отдельный WebApplicationContext для веб-слоя (контроллеры, view-resolvers и т.п.).
    - Может иметь root-контекст как parent.

## Environment

Spring Core предоставляет абстракцию Environment

через Environment можно получить доступ:
к переменным окружения;
к JVM переменным;
С абстракцией Environment тесно связано понятие профиля запуска приложения;
можно наполнить ApplicationContext бинами в зависимости от профиля запуска.

можно добавлять дополнительные источники параметров конфигурации - @PropertySource;
источники параметров просматриваются в следующем порядке:

- пользовательские источники;
- ServletConfig (только для web-контекста);
- ServletContext (<context-param> в web.xml);
- JNDI;
- параметры запуска JVM;
- переменные окружения.










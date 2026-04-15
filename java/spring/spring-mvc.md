# Spring Web MVC

Spring MVC — это слой обработки HTTP поверх Servlet API:

```
HTTP request
  → Servlet Filter chain
  → DispatcherServlet
  → HandlerMapping
  → HandlerAdapter
  → Controller method
  → ReturnValueHandler
  → ViewResolver или HttpMessageConverter
  → HTTP response
```

| Аннотация                                                                       | Где используется | Что делает                                                      |
| ------------------------------------------------------------------------------- | ---------------- | --------------------------------------------------------------- |
| `@Controller`                                                                   | класс            | MVC-контроллер для HTML/view-сценариев                          |
| `@RestController`                                                               | класс            | `@Controller` + `@ResponseBody` для REST/JSON                   |
| `@RequestMapping`                                                               | класс/метод      | общий mapping по path/method/headers/params/consumes/produces   |
| `@GetMapping`, `@PostMapping`, `@PutMapping`, `@PatchMapping`, `@DeleteMapping` | метод            | shortcut'ы для HTTP-методов                                     |
| `@PathVariable`                                                                 | параметр метода  | берет значение из path template: `/users/{id}`                  |
| `@RequestParam`                                                                 | параметр метода  | берет query/form parameter                                      |
| `@RequestBody`                                                                  | параметр метода  | читает HTTP body через `HttpMessageConverter`                   |
| `@ModelAttribute`                                                               | параметр/метод   | bind form/query object или предварительно кладет данные в model |
| `@Valid`                                                                        | параметр/поле    | запускает Jakarta Bean Validation                               |
| `@Validated`                                                                    | параметр/тип     | Spring-вариант validation, полезен для групп                    |
| `@ResponseBody`                                                                 | метод/класс      | пишет return value прямо в HTTP body                            |
| `@ResponseStatus`                                                               | метод/exception  | задает HTTP status                                              |
| `@ExceptionHandler`                                                             | метод            | обрабатывает exception локально или в advice                    |
| `@ControllerAdvice`                                                             | класс            | глобальные MVC-советы: exceptions/model/binders                 |
| `@RestControllerAdvice`                                                         | класс            | `@ControllerAdvice` + `@ResponseBody`                           |
| `@InitBinder`                                                                   | метод            | настраивает `WebDataBinder`                                     |
| `@CrossOrigin`                                                                  | класс/метод      | включает CORS для выбранных endpoints                           |

| Задача                                    | Обычно использовать                                                            |
| ----------------------------------------- | ------------------------------------------------------------------------------ |
| HTML-страницы                             | `@Controller` + `Model` + Thymeleaf                                            |
| JSON API                                  | `@RestController` + DTO + `@RequestBody`/`ResponseEntity`                      |
| Ошибки REST API                           | `@RestControllerAdvice` + `ProblemDetail` или единый DTO ошибки                |
| Валидация body/form                       | `@Valid` рядом с `@RequestBody` или `@ModelAttribute`                          |
| Валидация `@PathVariable`/`@RequestParam` | constraint-аннотации на параметрах, в Spring 6.1+ встроенная method validation |
| Сквозная безопасность                     | Spring Security filter chain                                                   |
| Логирование/метрики вокруг MVC handler    | `HandlerInterceptor`                                                           |
| Кодировка/CORS/security на уровне servlet | `Filter`                                                                       |

## Введение в Spring Web MVC

**Spring Web MVC** — это модуль Spring Framework для построения веб-приложений поверх Jakarta Servlet API.

Он реализует паттерн **Model–View–Controller (MVC)** для server-side rendering и одновременно служит зрелым REST-фреймворком. Spring MVC предоставляет:
- централизованный диспетчер входящих запросов (`DispatcherServlet`),
- механизм маппинга запросов на методы Java-классов,
- инфраструктуру для привязки данных, валидации, обработки ошибок,
- рендеринг HTML-представлений,
- запись HTTP body через `HttpMessageConverter` (JSON, XML, text, binary, files).


- Обработка HTTP-запросов и формирование HTTP-ответов.
- Построение REST API (JSON/XML).
- Построение серверных приложений с HTML-представлениями (Thymeleaf, JSP).
- Привязка HTTP-данных (query params, path variables, body) к Java-объектам.
- Валидация входящих данных.
- Централизованная обработка ошибок.


- **`spring-web`** — базовый модуль: HTTP-абстракции, `RestClient`, `RestTemplate`, базовые фильтры, multipart-инфраструктура.
  Не содержит MVC-инфраструктуру.
- **`spring-webmvc`** — полный MVC-стек: `DispatcherServlet`, контроллеры, view resolvers, message converters и т.д.
  Зависит от `spring-web`.

На Spring Boot 4 обычно подключают `spring-boot-starter-webmvc`, который тянет оба модуля и servlet container.
В Boot 3.x использовался `spring-boot-starter-web`; это привычное имя для проектов на Boot 3.x.


|             | Spring Web MVC                    | Spring WebFlux                            |
| ----------- | --------------------------------- | ----------------------------------------- |
| Стек        | Servlet (blocking)                | Reactive (non-blocking)                   |
| Основа      | `jakarta.servlet`                 | Reactor (`Mono`/`Flux`)                   |
| Сервер      | Tomcat, Jetty, Undertow (servlet) | Netty, Tomcat, Jetty (reactor)            |
| Модель      | Один поток на запрос              | Event-loop, небольшое число потоков       |
| Когда       | Большинство приложений            | Высоконагруженные I/O-интенсивные сервисы |
| Контроллеры | Обычные методы                    | Те же аннотации + reactive return types   |

Spring MVC — **blocking**, поток блокируется пока сервис выполняется.
WebFlux — **non-blocking**, поток освобождается сразу и уведомляется при готовности результата.


**MVC** — паттерн разделения ответственности:

```
Model  — данные/состояние, передаваемые в представление
View   — представление (обычно HTML)
Controller — принимает запрос, вызывает сервис, передает данные в View
```

В Spring MVC:
- **Model** — объект `Model`/`ModelMap`, атрибуты, передаваемые в шаблон
- **View** — Thymeleaf/JSP/FreeMarker-шаблон для HTML
- **Controller** — Java-класс с аннотацией `@Controller`/`@RestController`
- **REST body** — не классический `View`, а результат работы `HandlerMethodReturnValueHandler` + `HttpMessageConverter`

Бизнес-логика в Spring-приложении обычно живет не в `Model`, а в domain/service layer.

----------------------------------------------------------------------------------------------------

## Общая архитектура Spring Web MVC

Spring MVC работает поверх **Jakarta Servlet API** (`jakarta.servlet.*`).

Servlet stack — это синхронная, блокирующая модель:
- каждый запрос обрабатывается одним потоком из пула,
- поток занят до тех пор, пока не сформирован ответ,
- для конкурентности используется пул потоков (`ThreadPoolExecutor` в Tomcat).


**Servlet Container** (Tomcat, Jetty, Undertow):
- принимает TCP/HTTP-соединение,
- парсирует HTTP-запрос,
- создает `HttpServletRequest` и `HttpServletResponse`,
- вызывает зарегистрированный `Servlet` (в нашем случае — `DispatcherServlet`),
- управляет пулом потоков.

Spring MVC только управляет обработкой внутри одного запроса — всё, что ниже, делает Servlet Container.

`DispatcherServlet` — **Front Controller** Spring MVC.

Это обычный `HttpServlet`, но он:
- зарегистрирован как точка входа для всех запросов (`/`),
- внутри делегирует обработку другим компонентам: `HandlerMapping`, `HandlerAdapter`, `ViewResolver` и т.д.,
- координирует весь lifecycle запроса внутри Spring MVC.


```
Клиент
  │
  ▼
Servlet Container (Tomcat)
  │  создает HttpServletRequest / HttpServletResponse
  ▼
Filters (Servlet API уровень)
  │  CharacterEncodingFilter, Spring Security, CORS...
  ▼
DispatcherServlet
  │
  ├─► HandlerMapping → ищет контроллер/метод
  │
  ├─► HandlerInterceptor.preHandle
  │
  ├─► HandlerAdapter → подготавливает и вызывает метод
  │       └─► ArgumentResolvers (binding аргументов)
  │       └─► Validation
  │       └─► Controller Method
  │       └─► ReturnValueHandlers / HttpMessageConverter для @ResponseBody
  │
  ├─► HandlerInterceptor.postHandle
  │
  ├─► ViewResolver → View render для HTML
  │
  └─► HandlerInterceptor.afterCompletion

Клиент получает HTTP-ответ
```

| Компонент              | Роль                                            |
| ---------------------- | ----------------------------------------------- |
| `DispatcherServlet`    | Front Controller, координатор всего             |
| `HandlerMapping`       | Находит handler (контроллер + метод) по запросу |
| `HandlerAdapter`       | Вызывает найденный handler                      |
| `HandlerInterceptor`   | Перехватчики до/после вызова контроллера        |
| `ArgumentResolver`     | Подставляет аргументы в метод контроллера       |
| `HttpMessageConverter` | JSON/XML ↔ Java object                          |
| `ViewResolver`         | Имя view → реальный шаблон                      |
| `View`                 | Рендерит HTML                                   |
| `ExceptionResolver`    | Обрабатывает исключения                         |

----------------------------------------------------------------------------------------------------

## Lifecycle HTTP-запроса

1. Клиент (браузер, Postman, другой сервис) отправляет HTTP-запрос.
2. Запрос содержит: method, URL, headers, cookies, body, query params.
3. Servlet Container принимает TCP-соединение, парсирует HTTP.
4. Создаются `HttpServletRequest` и `HttpServletResponse`.

Центральный метод `DispatcherServlet.doDispatch()`:

```java
// Упрощенно, внутри doDispatch():
HandlerExecutionChain chain = getHandler(request);         // 1. найти handler
HandlerAdapter adapter = getHandlerAdapter(chain.getHandler()); // 2. найти adapter
chain.applyPreHandle(request, response);                   // 3. preHandle interceptors
ModelAndView mv = adapter.handle(request, response, chain.getHandler()); // 4. вызвать контроллер
chain.applyPostHandle(request, response, mv);              // 5. postHandle interceptors
processDispatchResult(request, response, chain, mv, ex);   // 6. рендер / error handling
chain.triggerAfterCompletion(request, response, ex);       // 7. afterCompletion
```

В реальном коде вокруг этого есть обработка multipart, async, exceptions и cleanup. Псевдокод выше нужен только для понимания очередности.

Поиск обработчика

`HandlerMapping` анализирует запрос:
- URL pattern (`/users/{id}`)
- HTTP method (`GET`, `POST`, ...)
- headers / params / content-type

Возвращает `HandlerExecutionChain` = handler + список interceptor'ов.

Подготовка аргументов метода

`HandlerAdapter` (реализация `RequestMappingHandlerAdapter`):
- анализирует сигнатуру метода контроллера,
- для каждого параметра вызывает подходящий `HandlerMethodArgumentResolver`,
- результаты: `@PathVariable` из URI, `@RequestParam` из query, `@RequestBody` из JSON-тела и т.д.

Вызов контроллера

После подготовки аргументов — reflection-вызов метода контроллера.
Контроллер возвращает результат.

Обработка результата

`HandlerMethodReturnValueHandler` обрабатывает return value:
- если `@ResponseBody` / `@RestController` → `HttpMessageConverter` сериализует объект в тело ответа,
- если String / `ModelAndView` → ViewResolver ищет шаблон,
- если `ResponseEntity` → устанавливает статус, заголовки, тело.

Формирование HTTP-ответа

- Устанавливается HTTP status code.
- Устанавливаются response headers (`Content-Type`, `Cache-Control`, ...).
- Записывается response body (HTML, JSON, файл...).
- Servlet Container отправляет ответ клиенту.

----------------------------------------------------------------------------------------------------

## DispatcherServlet

`DispatcherServlet` — это `HttpServlet`, который является **точкой входа** всего Spring MVC.

Он расширяет цепочку:
```
Servlet → GenericServlet → HttpServlet → HttpServletBean → FrameworkServlet → DispatcherServlet
```

**Front Controller** — паттерн, при котором один объект принимает все входящие запросы и делегирует их конкретным обработчикам.

Преимущества:
- единое место для сквозных задач (логирование, безопасность, i18n),
- централизованная маршрутизация,
- простота добавления общего поведения.

Как он маршрутизирует запросы

`DispatcherServlet` использует список `HandlerMapping`:
1. Перебирает все зарегистрированные `HandlerMapping`.
2. Первый, который вернул handler — используется.
3. По умолчанию используется `RequestMappingHandlerMapping` (на основе `@RequestMapping`).

Как он взаимодействует с другими компонентами

Все компоненты — Spring-бины в `WebApplicationContext`:
- `HandlerMapping` (может быть несколько)
- `HandlerAdapter` (может быть несколько)
- `ViewResolver` (может быть несколько, с приоритетом)
- `HandlerExceptionResolver`
- `MultipartResolver`
- `LocaleResolver`

`DispatcherServlet` инициализирует их при старте. Если не найден кастомный — использует дефолтные из `DispatcherServlet.properties`.

 Что происходит внутри DispatcherServlet

При первом запуске (инициализация):
1. Создает `WebApplicationContext`.
2. Находит и инициализирует все MVC-компоненты.

При каждом запросе (`doDispatch`):
1. Определяет multipart или нет.
2. Ищет handler chain.
3. Обрабатывает запрос.
4. Обрабатывает исключения, если они возникли.
5. Публикует событие `RequestHandledEvent`.

----------------------------------------------------------------------------------------------------

## Handler Infrastructure

HandlerMapping

Интерфейс: `org.springframework.web.servlet.HandlerMapping`

Основные реализации:
- **`RequestMappingHandlerMapping`** — главная, работает с `@RequestMapping`/`@GetMapping` и т.д.
- `BeanNameUrlHandlerMapping` — маппит URL на имя бина.
- `RouterFunctionMapping` — для функционального стиля WebFlux/MVC.

`RequestMappingHandlerMapping` при старте сканирует все `@Controller`-бины и строит карту:
`{method, URL, params, headers, consumes, produces}` → `HandlerMethod`.

HandlerAdapter

Интерфейс: `org.springframework.web.servlet.HandlerAdapter`

Задача: знать, как вызвать конкретный тип handler.

Основные реализации:
- **`RequestMappingHandlerAdapter`** — вызывает `@RequestMapping`-методы контроллеров.
- `SimpleControllerHandlerAdapter` — вызывает старый `Controller`-интерфейс.
- `HandlerFunctionAdapter` — для функциональных handler'ов.

`RequestMappingHandlerAdapter` содержит список `HandlerMethodArgumentResolver` и `HandlerMethodReturnValueHandler`.

Handler execution chain

`HandlerExecutionChain` = handler + упорядоченный список interceptor'ов.

```java
public class HandlerExecutionChain {
    private final Object handler;          // метод контроллера
    private List<HandlerInterceptor> interceptorList;
}
```

Порядок выполнения:
1. `preHandle` всех interceptor'ов (по порядку)
2. сам handler
3. `postHandle` всех interceptor'ов (в обратном порядке)
4. `afterCompletion` всех interceptor'ов, чей `preHandle` успешно прошел (в обратном порядке)

HandlerInterceptor

Интерфейс: `org.springframework.web.servlet.HandlerInterceptor`

```java
public interface HandlerInterceptor {
    // до контроллера: если вернуть false — цепочка прерывается
    default boolean preHandle(HttpServletRequest req, HttpServletResponse res, Object handler) throws Exception {
        return true;
    }
    // после контроллера, до рендеринга view; при @ResponseBody ModelAndView обычно null
    default void postHandle(HttpServletRequest req, HttpServletResponse res, Object handler, ModelAndView mv) {}
    // после завершения запроса для успешно пройденных interceptor'ов
    default void afterCompletion(HttpServletRequest req, HttpServletResponse res, Object handler, Exception ex) {}
}
```

Типичные применения:
- логирование времени выполнения,
- проверка авторизации (хотя лучше Spring Security),
- установка Locale,
- метрики и трейсинг,
- очистка ThreadLocal.

HandlerExceptionResolver

Интерфейс: `org.springframework.web.servlet.HandlerExceptionResolver`

Когда в цепочке выбросилось исключение — `DispatcherServlet` передает его в список `HandlerExceptionResolver` (по приоритету).

Основные реализации:
- **`ExceptionHandlerExceptionResolver`** — обрабатывает `@ExceptionHandler`.
- `ResponseStatusExceptionResolver` — обрабатывает `@ResponseStatus`.
- `DefaultHandlerExceptionResolver` — обрабатывает стандартные Spring-исключения.

----------------------------------------------------------------------------------------------------

## Контроллеры

Контроллер — это bean, помеченный `@Controller` или `@RestController`, методы которого обрабатывают
HTTP-запросы. Он принимает HTTP-данные через параметры метода, вызывает сервисный слой, возвращает
результат (view name, DTO, ResponseEntity ...).

`@Controller`:
- Помечает класс как Spring MVC controller.
- Включает Component Scan.
- Методы по умолчанию возвращают имя view.

```java
@Controller
@RequestMapping("/users")
public class UserController {
    @GetMapping("/{id}")
    public String getUser(@PathVariable Long id, Model model) {
        model.addAttribute("user", userService.findById(id));
        return "users/detail";
    }
}
```

`@RestController`:
- `@RestController` = `@Controller` + `@ResponseBody`.
- `@ResponseBody` на уровне класса означает: все методы возвращают тело ответа напрямую.
- Никакого ViewResolver — только HttpMessageConverter.

```java
@RestController
@RequestMapping("/api/users")
public class UserRestController {
    @GetMapping("/{id}")
    public UserDto getUser(@PathVariable Long id) {
        return userService.findById(id);  // сериализуется в JSON
    }
}
```

**Разница между ними:**

|                      | `@Controller`                            | `@RestController`               |
| -------------------- | ---------------------------------------- | ------------------------------- |
| Метаннотация         | `@Component`                             | `@Controller` + `@ResponseBody` |
| Возврат по умолчанию | имя view                                 | тело HTTP-ответа                |
| Для чего             | HTML-приложения                          | REST API                        |
| ViewResolver         | используется                             | не используется                 |
| HttpMessageConverter | используется для `@ResponseBody`-методов | используется для всех методов   |

**Аннотации раздела:**

| Аннотация         | Где ставится | Что означает                                                                                                                               |
| ----------------- | ------------ | ------------------------------------------------------------------------------------------------------------------------------------------ |
| `@Controller`     | класс        | Регистрирует класс как Spring bean и MVC-controller. Методы обычно возвращают имя view, `ModelAndView`, `View` или пишут response вручную. |
| `@RestController` | класс        | Составная аннотация: `@Controller` + `@ResponseBody`. Все return values по умолчанию идут в HTTP body.                                     |
| `@ResponseBody`   | метод/класс  | Отключает трактовку return value как имени view и передает значение в `HttpMessageConverter`.                                              |
| `@RequestMapping` | класс/метод  | Задает общий URL и условия выбора handler'а. На классе обычно задает базовый path.                                                         |

`@Controller` и `@RestController` — это stereotype-аннотации: они участвуют в component scan и делают класс Spring bean'ом.

Структура controller-класса

```java
@RestController
@RequestMapping("/api/products")
@RequiredArgsConstructor
public class ProductController {
    @GetMapping
    public List<ProductDto> getAll() { ... }

    @GetMapping("/{id}")
    public ProductDto getById(@PathVariable Long id) { ... }

    @PostMapping
    @ResponseStatus(HttpStatus.CREATED)
    public ProductDto create(@RequestBody @Valid CreateProductRequest req) { ... }

    @PutMapping("/{id}")
    public ProductDto update(@PathVariable Long id, @RequestBody @Valid UpdateProductRequest req) { ... }

    @DeleteMapping("/{id}")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void delete(@PathVariable Long id) { ... }
}
```

**Хорошие практики проектирования контроллеров:**

- **Тонкий контроллер**: только HTTP-слой, вся логика — в service.
- **Не использовать Entity напрямую**: работать только с DTO.
- **Явные status codes**: через `@ResponseStatus` или `ResponseEntity`.
- **Один контроллер на один ресурс**: `UserController`, `OrderController`.
- **Разделять page-контроллеры и API-контроллеры** при гибридных приложениях.


`@RequestMapping` - Базовая аннотация маппинга. Может стоять на классе и/или методе.
На классе — задает базовый путь. На методе — уточняет маппинг.

```java
@RequestMapping(
    value = "/users",         // URL pattern
    method = RequestMethod.GET, // HTTP method
    params = "active=true",   // query param
    headers = "X-API=v2",     // header
    consumes = "application/json",  // Content-Type входящего запроса
    produces = "application/json"   // Accept клиента
)
```


Специализированные аннотации - Shortcutы над `@RequestMapping`. Все принимают те же параметры
(`params`, `headers`, `consumes`, `produces`):

```java
@GetMapping("/path")
@PostMapping("/path")
@PutMapping("/path")
@DeleteMapping("/path")
@PatchMapping("/path")
```

Аннотации и атрибуты mapping

| Аннотация/атрибут | Где                        | Значение                                                                                                          |
| ----------------- | -------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| `@RequestMapping` | класс/метод                | Универсальный mapping. Используется, когда нужны нестандартные условия или общий base path.                       |
| `@GetMapping`     | метод                      | `GET`, безопасное чтение ресурса.                                                                                 |
| `@PostMapping`    | метод                      | `POST`, создание ресурса или command/action.                                                                      |
| `@PutMapping`     | метод                      | `PUT`, полная замена ресурса.                                                                                     |
| `@PatchMapping`   | метод                      | `PATCH`, частичное изменение ресурса.                                                                             |
| `@DeleteMapping`  | метод                      | `DELETE`, удаление ресурса.                                                                                       |
| `value` / `path`  | атрибут                    | URL pattern: `"/users/{id}"`. `value` и `path` — aliases.                                                         |
| `method`          | атрибут `@RequestMapping`  | HTTP method: `RequestMethod.GET`. В shortcut-аннотациях уже задан.                                                |
| `params`          | атрибут                    | Дополнительное условие по query/form params: `"active=true"`, `"!debug"`.                                         |
| `headers`         | атрибут                    | Дополнительное условие по headers: `"X-Version=2"`, `"!X-Legacy"`.                                                |
| `consumes`        | атрибут                    | Какой `Content-Type` endpoint принимает. Несовпадение обычно приводит к `415 Unsupported Media Type`.             |
| `produces`        | атрибут                    | Какой response media type endpoint умеет отдавать. Несовпадение с `Accept` может привести к `406 Not Acceptable`. |
| `version`         | атрибут Spring Framework 7 | Условие по API version, если включена MVC API versioning.                                                         |

Mapping по URL Поддерживает паттерны:

```java
@GetMapping("/users/{id}")          // path variable
@GetMapping("/files/**")            // wildcard (любой суффикс)
@GetMapping("/users/{id}/orders")   // вложенный ресурс
@GetMapping("/v{version}/users")    // переменная в сегменте
```

Паттерны используют `PathPatternParser` по умолчанию в современных Spring Boot приложениях.
Старый `AntPathMatcher` еще встречается в legacy-проектах, но новые приложения лучше проектировать
под `PathPatternParser`.

Mapping по HTTP method

```java
@RequestMapping(value = "/users", method = RequestMethod.GET)
// или просто
@GetMapping("/users")
```

Если метод не указан — маппинг работает для всех HTTP-методов.

Mapping по params

```java
@GetMapping(value = "/users", params = "active=true")
@GetMapping(value = "/users", params = "!active")       // параметра нет
@GetMapping(value = "/users", params = {"active", "role=admin"})
```

Mapping по headers

```java
@GetMapping(value = "/users", headers = "X-Version=2")
@GetMapping(value = "/users", headers = "!X-Version")
```

consumes и produces

```java
// consumes — Content-Type входящего запроса (что мы принимаем)
@PostMapping(value = "/users", consumes = MediaType.APPLICATION_JSON_VALUE)

// produces — Accept из запроса (что мы отдаем)
@GetMapping(value = "/users", produces = MediaType.APPLICATION_JSON_VALUE)

// Content negotiation — несколько вариантов
@GetMapping(value = "/users", produces = {
    MediaType.APPLICATION_JSON_VALUE,
    MediaType.APPLICATION_XML_VALUE
})
```

API versioning в Spring Framework 7

Spring Framework 7 добавил встроенную поддержку API versioning в Spring MVC. Сначала нужно настроить, откуда брать версию:

```java
@Configuration
public class WebConfig implements WebMvcConfigurer {
    @Override
    public void configureApiVersioning(ApiVersionConfigurer configurer) {
        configurer.useRequestHeader("API-Version");
    }
}
```

После этого mapping может учитывать version:

```java
@GetMapping(path = "/accounts/{id}", version = "1.0")
public AccountDto getV1(@PathVariable Long id) { ... }

@GetMapping(path = "/accounts/{id}", version = "2.0")
public AccountDto getV2(@PathVariable Long id) { ... }

@GetMapping(path = "/accounts/{id}", version = "2.0+")
public AccountDto getV2AndNewer(@PathVariable Long id) { ... }
```

Версию можно резолвить из header, query parameter, path segment или media type parameter.
Если запрошенная версия не поддерживается, Spring выбрасывает `InvalidApiVersionException`,
что обычно приводит к `400 Bad Request`.

----------------------------------------------------------------------------------------------------

## Параметры методов контроллера

`@PathVariable`

Извлекает значение из URL-шаблона.

```java
@GetMapping("/users/{id}")
public UserDto getUser(@PathVariable Long id) { ... }

@GetMapping("/users/{userId}/orders/{orderId}")
public OrderDto getOrder(@PathVariable Long userId, @PathVariable Long orderId) { ... }

// Если имя переменной отличается от имени параметра:
@GetMapping("/users/{user-id}")
public UserDto get(@PathVariable("user-id") Long userId) { ... }

// Опциональный:
@GetMapping({"/users/{id}", "/users"})
public UserDto get(@PathVariable(required = false) Long id) { ... }
```

`@RequestParam`

Извлекает query-параметр или form-параметр.

```java
// GET /users?role=admin&page=2
@GetMapping("/users")
public List<UserDto> getUsers(
    @RequestParam String role,
    @RequestParam(defaultValue = "0") int page,
    @RequestParam(required = false) String name
) { ... }

// Несколько значений одного параметра:
// GET /filter?tag=java&tag=spring
@GetMapping("/filter")
public List<Item> filter(@RequestParam List<String> tag) { ... }
```

`@RequestBody`

Читает тело HTTP-запроса и десериализует через `HttpMessageConverter`.

```java
@PostMapping("/users")
public UserDto create(@RequestBody CreateUserRequest request) { ... }

// С валидацией:
@PostMapping("/users")
public UserDto create(@RequestBody @Valid CreateUserRequest request) { ... }
```

`Content-Type: application/json` → Jackson десериализует JSON в Java-объект.

`@ModelAttribute`

Привязывает данные формы (или query params) к Java-объекту.

```java
// GET /search?name=Alice&age=25
@GetMapping("/search")
public String search(@ModelAttribute SearchForm form, Model model) { ... }

// POST /register с form data
@PostMapping("/register")
public String register(@ModelAttribute @Valid RegisterForm form, BindingResult result) { ... }
```

Также может обозначать метод, заполняющий модель перед каждым запросом в контроллере:

```java
@ModelAttribute("categories")
public List<Category> populateCategories() {
    return categoryService.findAll(); // добавляется в model автоматически
}
```

`@RequestHeader`

Извлекает значение HTTP-заголовка.

```java
@GetMapping("/profile")
public UserDto getProfile(
    @RequestHeader("Authorization") String authHeader,
    @RequestHeader(value = "X-Request-Id", required = false) String requestId
) { ... }
```

`@CookieValue`

Извлекает значение cookie.

```java
@GetMapping("/dashboard")
public String dashboard(@CookieValue("sessionId") String sessionId) { ... }
```

`@RequestPart`

Для multipart-запросов, когда часть — это JSON или файл.

```java
@PostMapping("/upload")
public void upload(
    @RequestPart("file") MultipartFile file,
    @RequestPart("meta") @Valid FileMeta meta
) { ... }
```

`HttpServletRequest / HttpServletResponse`

Прямой доступ к servlet-объектам:

```java
@GetMapping("/info")
public String info(HttpServletRequest request, HttpServletResponse response) {
    String ip = request.getRemoteAddr();
    response.setHeader("X-Custom", "value");
    return "info";
}
```

`HttpSession`

```java
@GetMapping("/cart")
public CartDto getCart(HttpSession session) {
    return (CartDto) session.getAttribute("cart");
}
```

`Principal`

Текущий аутентифицированный пользователь (интегрируется со Spring Security):

```java
@GetMapping("/me")
public UserDto getCurrentUser(Principal principal) {
    return userService.findByUsername(principal.getName());
}
```

`Locale`

```java
@GetMapping("/greeting")
public String greeting(Locale locale, Model model) {
    model.addAttribute("msg", messageSource.getMessage("hello", null, locale));
    return "greeting";
}
```

`Model`

```java
@GetMapping("/users/{id}")
public String profile(@PathVariable Long id, Model model) {
    model.addAttribute("user", userService.findById(id));
    model.addAttribute("roles", roleService.findAll());
    return "users/profile";
}
```


`@SessionAttribute` читает уже существующий session attribute. Это не то же самое, что `@SessionAttributes`, который сохраняет model attributes в session для controller workflow.

```java
@GetMapping("/checkout")
public String checkout(@SessionAttribute("cart") Cart cart, Model model) {
    model.addAttribute("cart", cart);
    return "checkout";
}
```

`@RequestAttribute` читает attribute, который ранее положил filter, interceptor или servlet container:

```java
@GetMapping("/trace")
public String trace(@RequestAttribute("traceId") String traceId) { ... }
```


`@MatrixVariable` извлекает path parameters внутри URI segment:

```http
GET /cars;color=red;year=2024
```

```java
@GetMapping("/cars")
public List<CarDto> cars(@MatrixVariable String color,
                         @MatrixVariable int year) { ... }
```

На практике matrix variables встречаются редко и могут требовать явной настройки path matching / URL handling.

**Аннотации параметров контроллера**

| Аннотация / тип параметра                    | Источник данных                      | Типичный сценарий                                      |
| -------------------------------------------- | ------------------------------------ | ------------------------------------------------------ |
| `@PathVariable`                              | path template: `/users/{id}`         | ID ресурса, slug, nested resource ids                  |
| `@RequestParam`                              | query string или form data           | фильтры, пагинация, сортировка, простые form fields    |
| `@RequestBody`                               | HTTP body                            | JSON/XML DTO в REST API                                |
| `@ModelAttribute` на параметре               | query/form parameters + model        | HTML-формы, search/filter objects                      |
| `@ModelAttribute` на методе                  | выполняется перед handler method     | общие данные для model, например справочники           |
| `@RequestHeader`                             | HTTP headers                         | auth header, request id, feature flags                 |
| `@CookieValue`                               | cookies                              | user preferences, legacy session identifiers           |
| `@RequestPart`                               | part в `multipart/form-data`         | файл + JSON metadata в одном запросе                   |
| `@SessionAttribute`                          | existing HTTP session attribute      | чтение конкретного значения из session                 |
| `@RequestAttribute`                          | request attribute                    | данные, положенные filter/interceptor/servlet          |
| `@MatrixVariable`                            | matrix params в path segment         | редкие API с segment parameters                        |
| `HttpServletRequest` / `HttpServletResponse` | servlet API                          | низкоуровневый доступ, когда абстракций MVC не хватает |
| `HttpSession`                                | server-side session                  | cart, wizard flow, временное состояние                 |
| `Principal` / `Authentication`               | security context                     | текущий пользователь                                   |
| `Locale`                                     | `LocaleResolver` / `Accept-Language` | i18n                                                   |
| `Model` / `ModelMap` / `Map`                 | MVC model                            | данные для HTML-шаблона                                |

Правило выбора простое: path identity — `@PathVariable`, optional filters — `@RequestParam`, JSON body — `@RequestBody`, HTML form object — `@ModelAttribute`.

----------------------------------------------------------------------------------------------------

## Data Binding

**Data Binding** — автоматическое преобразование HTTP-данных из URL/query/form fields в Java-объекты.
Spring MVC выполняет binding через `WebDataBinder`.
JSON/XML body для `@RequestBody` читается прежде всего через `HttpMessageConverter`;
После десериализации может запускаться validation.

Привязка request-параметров к Java-объекту

```java
// GET /search?name=Alice&minAge=18&maxAge=30
public class SearchCriteria {
    private String name;
    private int minAge;
    private int maxAge;
    // getters/setters
}

@GetMapping("/search")
public List<UserDto> search(@ModelAttribute SearchCriteria criteria) { ... }
// Spring сам свяжет query params с полями SearchCriteria
```

WebDataBinder

`WebDataBinder` выполняет:
- type conversion (String → int, String → Date, ...),
- binding полей,
- валидацию.

Можно кастомизировать через `@InitBinder` в контроллере или `@ControllerAdvice`:

```java
@InitBinder
public void initBinder(WebDataBinder binder) {
    binder.setDisallowedFields("id", "createdAt"); // запрещаем биндить эти поля
    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy");
    binder.registerCustomEditor(Date.class, new CustomDateEditor(dateFormat, false));
}
```

Аннотация `@InitBinder` помечает метод, который настраивает `WebDataBinder` перед binding'ом аргументов контроллера. Частые задачи:
- запретить опасные поля (`id`, `role`, `createdAt`) для защиты от mass assignment,
- зарегистрировать legacy `PropertyEditor`,
- подключить кастомный validator для конкретной формы.

Типовые преобразования данных

Встроенные конверторы и formatter'ы:
- `String` → `int`, `long`, `double`, `boolean`
- `String` → `LocalDate`, `LocalDateTime`, `OffsetDateTime` при стандартных ISO-форматах или через `@DateTimeFormat`
- `String` → `enum`
- `String` → `UUID`

Custom converters и formatters

**Converter** — конвертирует один тип в другой:

```java
@Component
public class StringToStatusConverter implements Converter<String, Status> {
    @Override
    public Status convert(String source) {
        return Status.valueOf(source.toUpperCase());
    }
}
```

Аннотации и API раздела:

| Аннотация / API                  | Где                     | Что делает                                                        |
| -------------------------------- | ----------------------- | ----------------------------------------------------------------- |
| `@InitBinder`                    | метод controller/advice | Настраивает `WebDataBinder` для binding'а request data.           |
| `@DateTimeFormat`                | поле/параметр           | Задает формат parsing/printing для date/time типов в MVC binding. |
| `@NumberFormat`                  | поле/параметр           | Задает формат чисел в MVC binding.                                |
| `Converter<S,T>`                 | bean/config             | Одностороннее преобразование типа.                                |
| `Formatter<T>`                   | bean/config             | String ↔ object с учетом `Locale`.                                |
| `WebMvcConfigurer#addFormatters` | config                  | Регистрирует converters/formatters глобально для MVC.             |

**Formatter** — конвертирует String ↔ Object с учетом Locale:

```java
@Component
public class MoneyFormatter implements Formatter<Money> {
    @Override
    public Money parse(String text, Locale locale) { ... }
    @Override
    public String print(Money money, Locale locale) { ... }
}
```

Регистрация:

```java
@Configuration
public class WebConfig implements WebMvcConfigurer {
    @Override
    public void addFormatters(FormatterRegistry registry) {
        registry.addConverter(new StringToStatusConverter());
    }
}
```

Ошибки binding

Если binding не удался (например, буква вместо числа для `int`-поля):
- создается `BindingResult` с ошибками типа `FieldError`,
- если `BindingResult` объявлен сразу после bindable-объекта — контроллер сам решает, что делать с ошибками,
- если подходящего `BindingResult` нет — Spring выбрасывает exception; конкретный тип зависит от аргумента: `MethodArgumentNotValidException`, `BindException`, `MethodArgumentTypeMismatchException`, `HandlerMethodValidationException` и др.

----------------------------------------------------------------------------------------------------

## Валидация данных

Валидация — проверка входных данных на корректность на границе приложения (контроллер).
Цель: не пропустить некорректные данные в сервисный слой и БД.

@Valid

Стандартная аннотация из Jakarta Bean Validation (`jakarta.validation.Valid`). Запускает валидацию объекта:

```java
@PostMapping("/users")
public UserDto create(@RequestBody @Valid CreateUserRequest request) { ... }
```

Важно: в Boot 4 / Boot 3 validation не считается частью web starter'а.
Для Bean Validation обычно добавляют отдельный starter:

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-validation</artifactId>
</dependency>
```

@Validated

Spring-аннотация. На параметрах и типах DTO полезна прежде всего для **групп валидации**:

```java
@PutMapping("/users/{id}")
public UserDto update(@PathVariable Long id,
                      @RequestBody @Validated(Update.class) UpdateUserRequest req) { ... }
```

В Spring Framework 6.1+ / 7.x MVC умеет встроенную method validation для controller methods.
Поэтому для новых контроллеров обычно не ставят `@Validated` на класс только ради
`@RequestParam` / `@PathVariable`; достаточно constraint-аннотаций на параметрах:

```java
@RestController
public class UserController {
    @GetMapping("/users/{id}")
    public UserDto get(@PathVariable @Min(1) Long id) { ... }
}
```

Если поставить `@Validated` на класс контроллера, включается старый AOP-based механизм method
validation через proxy. Это может быть нужно в legacy-сценариях, но для современного MVC чаще лишнее.

BindingResult

Содержит ошибки валидации и binding. Должен идти сразу после валидируемого объекта:

```java
@PostMapping("/register")
public String register(@Valid @ModelAttribute RegisterForm form, BindingResult result) {
    if (result.hasErrors()) {
        return "register"; // вернуть форму с ошибками
    }
    userService.register(form);
    return "redirect:/login";
}
```

Bean Validation

Аннотации из `jakarta.validation`:

```java
public class CreateUserRequest {
    @NotBlank
    @Size(min = 3, max = 50)
    private String name;

    @Email
    @NotBlank
    private String email;

    @Min(18) @Max(120)
    private int age;

    @NotNull
    @Valid  // вложенная валидация
    private AddressDto address;

    @Pattern(regexp = "^\\+?[0-9]{10,15}$")
    private String phone;
}
```

Часто используемые аннотации:

| Аннотация           | Назначение                                |
| ------------------- | ----------------------------------------- |
| `@NotNull`          | не null                                   |
| `@NotBlank`         | не null, не пустая строка (включает trim) |
| `@NotEmpty`         | не null, не пустая коллекция/строка       |
| `@Size(min, max)`   | длина строки или размер коллекции         |
| `@Min(value)`       | минимальное числовое значение             |
| `@Max(value)`       | максимальное числовое значение            |
| `@Email`            | формат email                              |
| `@Pattern(regexp)`  | regex                                     |
| `@Positive`         | > 0                                       |
| `@PositiveOrZero`   | >= 0                                      |
| `@Past` / `@Future` | дата в прошлом/будущем                    |
| `@Valid`            | вложенная валидация                       |

Аннотации раздела:

| Аннотация                       | Где ставится                     | Что делает                                                                         |
| ------------------------------- | -------------------------------- | ---------------------------------------------------------------------------------- |
| `@Valid`                        | параметр, поле, generic element  | Запускает стандартную Bean Validation и cascade validation для вложенных объектов. |
| `@Validated`                    | параметр, тип, метод             | Spring-аннотация, добавляет поддержку validation groups.                           |
| `@NotNull`                      | поле/параметр                    | Значение не `null`; пустая строка допустима.                                       |
| `@NotBlank`                     | `String`                         | Не `null` и после trim есть хотя бы один символ.                                   |
| `@NotEmpty`                     | `String`, collection, map, array | Не `null` и размер больше 0.                                                       |
| `@Size`                         | `String`, collection, map, array | Проверяет длину/размер.                                                            |
| `@Min` / `@Max`                 | числа                            | Числовые границы.                                                                  |
| `@Positive` / `@PositiveOrZero` | числа                            | Положительное или неотрицательное значение.                                        |
| `@Email`                        | `String`                         | Проверка email-формата.                                                            |
| `@Pattern`                      | `String`                         | Проверка регулярным выражением.                                                    |
| `@Past` / `@Future`             | date/time                        | Дата/время в прошлом или будущем.                                                  |

**Обработка ошибок валидации**

При использовании `@RequestBody @Valid` без `BindingResult`:
- выбрасывается `MethodArgumentNotValidException`,
- обрабатывается через `@ExceptionHandler` или `@ControllerAdvice`.

При validation constraints прямо на параметрах метода (`@PathVariable @Min(1)`, `@RequestParam @NotBlank`) в Spring 6.1+ обычно выбрасывается `HandlerMethodValidationException`.

```java
@RestControllerAdvice
public class GlobalExceptionHandler {
    @ExceptionHandler(MethodArgumentNotValidException.class)
    @ResponseStatus(HttpStatus.BAD_REQUEST)
    public ErrorResponse handleValidation(MethodArgumentNotValidException ex) {
        List<String> errors = ex.getBindingResult().getFieldErrors().stream()
            .map(e -> e.getField() + ": " + e.getDefaultMessage())
            .toList();
        return new ErrorResponse("Validation failed", errors);
    }

    @ExceptionHandler(HandlerMethodValidationException.class)
    @ResponseStatus(HttpStatus.BAD_REQUEST)
    public ErrorResponse handleMethodValidation(HandlerMethodValidationException ex) {
        return new ErrorResponse("Validation failed", List.of(ex.getMessage()));
    }
}
```

В современных REST API также можно возвращать стандартный `ProblemDetail` (`application/problem+json`) вместо собственного DTO ошибки.

**Validation в HTML-формах**

```java
@PostMapping("/register")
public String register(@Valid @ModelAttribute RegisterForm form,
                       BindingResult result,
                       Model model) {
    if (result.hasErrors()) {
        return "register"; // показываем форму снова с ошибками
    }
    userService.register(form);
    return "redirect:/dashboard";
}
```

В Thymeleaf:

```html
<input type="text" th:field="*{name}" th:errorclass="error"/>
<span th:errors="*{name}" class="error-msg"></span>
```

**Validation в REST API**

```java
@PostMapping("/users")
public ResponseEntity<UserDto> create(@RequestBody @Valid CreateUserRequest req) {
    return ResponseEntity.status(201).body(userService.create(req));
}
```

При ошибке — `400 Bad Request` с телом ошибки через `@ControllerAdvice`.

----------------------------------------------------------------------------------------------------

## Возвращаемые значения контроллеров

**String как имя view**

```java
@GetMapping("/home")
public String home() {
    return "home"; // ViewResolver ищет templates/home.html
}
```

**Model + view name**

```java
@GetMapping("/users/{id}")
public String user(@PathVariable Long id, Model model) {
    model.addAttribute("user", userService.findById(id));
    return "users/detail";
}
```

**ModelAndView**

```java
@GetMapping("/users/{id}")
public ModelAndView user(@PathVariable Long id) {
    ModelAndView mav = new ModelAndView("users/detail");
    mav.addObject("user", userService.findById(id));
    return mav;
}
```

Старый стиль, но всё ещё работает. Удобно когда view и модель определяются условно.

**View**

```java
@GetMapping("/special")
public View special() {
    return new RedirectView("/home");
}
```

**void**

Если response пишется вручную через `HttpServletResponse`:

```java
@GetMapping("/download")
public void download(HttpServletResponse response) throws IOException {
    response.setContentType("application/pdf");
    response.getOutputStream().write(fileBytes);
}
```

Если `void`-метод не записал response сам, Spring может попытаться определить view name из request path. Поэтому для REST лучше явно возвращать `ResponseEntity<Void>` или ставить `@ResponseStatus`.

**DTO / object**

Только при `@ResponseBody` или `@RestController`:

```java
@GetMapping("/users/{id}")
public UserDto getUser(@PathVariable Long id) {
    return userService.findById(id); // сериализуется в JSON
}
```

**@ResponseBody**

Указывает, что return value должен писаться прямо в тело ответа:

```java
@Controller
public class UserController {
    @GetMapping("/api/users/{id}")
    @ResponseBody
    public UserDto getUser(@PathVariable Long id) { ... }
}
```

**ResponseEntity<T>**

Полный контроль над ответом: статус, заголовки, тело:

```java
@GetMapping("/users/{id}")
public ResponseEntity<UserDto> getUser(@PathVariable Long id) {
    return userService.findById(id)
        .map(user -> ResponseEntity.ok(user))
        .orElse(ResponseEntity.notFound().build());
}

@PostMapping("/users")
public ResponseEntity<UserDto> create(@RequestBody @Valid CreateUserRequest req) {
    UserDto created = userService.create(req);
    URI location = URI.create("/api/users/" + created.getId());
    return ResponseEntity.created(location).body(created);
}

// Кастомные заголовки:
return ResponseEntity.ok()
    .header("X-Custom-Header", "value")
    .body(dto);
```

**HttpEntity<T>**

Как `ResponseEntity`, но без статус-кода (только заголовки + тело):

```java
@GetMapping("/info")
public HttpEntity<InfoDto> info() {
    HttpHeaders headers = new HttpHeaders();
    headers.add("X-Info", "value");
    return new HttpEntity<>(new InfoDto(), headers);
}
```

**redirect**

```java
// Простой redirect:
@PostMapping("/login")
public String login() {
    return "redirect:/dashboard";
}

// Через RedirectView:
@PostMapping("/submit")
public View submit() {
    return new RedirectView("/success");
}

// Через ResponseEntity:
@GetMapping("/old-path")
public ResponseEntity<Void> redirect() {
    return ResponseEntity.status(HttpStatus.MOVED_PERMANENTLY)
        .location(URI.create("/new-path"))
        .build();
}
```

**forward**

```java
@GetMapping("/old")
public String forward() {
    return "forward:/new";  // внутренний forward, URL не меняется
}
```

**Возврат файлов и ресурсов**

```java
// через Resource:
@GetMapping("/files/{filename}")
public ResponseEntity<Resource> downloadFile(@PathVariable String filename) {
    Resource resource = new FileSystemResource(Paths.get(uploadDir, filename));
    return ResponseEntity.ok()
        .header(HttpHeaders.CONTENT_DISPOSITION, "attachment; filename=\"" + filename + "\"")
        .contentType(MediaType.APPLICATION_OCTET_STREAM)
        .body(resource);
}
```

**Async return types**

```java
// Callable — выполняется в другом потоке, освобождает servlet-поток:
@GetMapping("/async")
public Callable<String> async() {
    return () -> heavyService.process();
}

// DeferredResult — результат устанавливается асинхронно извне:
@GetMapping("/deferred")
public DeferredResult<String> deferred() {
    DeferredResult<String> result = new DeferredResult<>(5000L); // timeout 5s
    executor.submit(() -> result.setResult(heavyService.process()));
    return result;
}

// StreamingResponseBody — потоковая отдача данных:
@GetMapping("/stream")
public StreamingResponseBody stream() {
    return outputStream -> {
        for (int i = 0; i < 100; i++) {
            outputStream.write(("line " + i + "\n").getBytes());
            outputStream.flush();
        }
    };
}
```

**Аннотации и типы return value**

| Return value / аннотация               | Что делает                                                                                                 |
| -------------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| `String` в `@Controller`               | Имя view: `"users/list"`, `"redirect:/users"`, `"forward:/internal"`.                                      |
| `String` в `@RestController`           | Тело ответа как text/plain или другой negotiated media type.                                               |
| `@ResponseBody`                        | Передает return value в `HttpMessageConverter`, а не в `ViewResolver`.                                     |
| `@ResponseStatus`                      | Фиксирует status code для успешного метода или exception class.                                            |
| `ResponseEntity<T>`                    | Полный контроль: status, headers, body. Лучший выбор для REST, когда status/headers зависят от результата. |
| `HttpEntity<T>`                        | Headers + body без явного status.                                                                          |
| `ModelAndView`                         | View name/object + model в одном return value.                                                             |
| `ProblemDetail`                        | Стандартное тело ошибки для REST, обычно `application/problem+json`.                                       |
| `Callable<T>` / `DeferredResult<T>`    | Servlet async processing; результат будет обработан обычными return value handlers.                        |
| `StreamingResponseBody` / `SseEmitter` | Потоковая отдача или Server-Sent Events.                                                                   |

----------------------------------------------------------------------------------------------------

## Model в Spring MVC

`Model` — контейнер атрибутов, которые контроллер передает во view (шаблон).

Это Map `String → Object`: ключ — имя атрибута в шаблоне, значение — Java-объект.

**Передача данных из controller во view:**

```java
@GetMapping("/dashboard")
public String dashboard(Model model) {
    model.addAttribute("user", currentUser());
    model.addAttribute("stats", statsService.getStats());
    return "dashboard";
}
```

В Thymeleaf:

```html
<span th:text="${user.name}"></span>
<span th:text="${stats.total}"></span>
```

**Model, Map, ModelMap**

Три взаимозаменяемых варианта:

```java
// 1. Model (интерфейс Spring)
public String page(Model model) {
    model.addAttribute("key", value);
    return "view";
}

// 2. Map<String, Object>
public String page(Map<String, Object> model) {
    model.put("key", value);
    return "view";
}

// 3. ModelMap (extends LinkedHashMap)
public String page(ModelMap modelMap) {
    modelMap.addAttribute("key", value);
    return "view";
}
```

Все три работают одинаково — Spring их поддерживает.

**Когда использовать Model**

Только для классического MVC с HTML-шаблонизатором.
В REST-контроллерах (`@RestController`) — `Model` не нужен, данные передаются через return value.

**Добавление атрибутов в model**

```java
// Через параметр метода:
model.addAttribute("name", value);
model.addAllAttributes(map);

// Через @ModelAttribute-метод в контроллере (выполняется перед каждым методом):
@ModelAttribute("categories")
public List<Category> categories() {
    return categoryService.findAll();
}
```

----------------------------------------------------------------------------------------------------

## View и ViewResolver


`View` — интерфейс Spring MVC, представляющий объект, который рендерит response. В HTML-сценариях это обычно шаблон, но `View` может быть и redirect/internal resource/custom renderer.

```java
public interface View {
    void render(Map<String, ?> model, HttpServletRequest request, HttpServletResponse response) throws Exception;
}
```

View получает атрибуты модели и рендерит HTML (или другой формат) в response.

**Что такое ViewResolver**

`ViewResolver` — интерфейс, преобразующий строковое имя view в объект `View`.

```java
public interface ViewResolver {
    View resolveViewName(String viewName, Locale locale) throws Exception;
}
```

Spring перебирает зарегистрированные `ViewResolver` по приоритету до первого успешного.

**Разрешение имени представления**

```java
return "users/profile";
// ViewResolver: "users/profile" → templates/users/profile.html
// Thymeleaf: prefix = "classpath:/templates/", suffix = ".html"
```

Для redirect/forward:
```java
return "redirect:/login";   // специальный redirect-префикс, обычно превращается в RedirectView
return "forward:/internal"; // внутренний forward
```

**Рендеринг HTML**

1. Контроллер возвращает view name.
2. `ViewResolver` находит шаблон.
3. `View.render()` берет model attributes и рендерит шаблон в HTML.
4. HTML записывается в `HttpServletResponse`.

**Основные шаблонизаторы**

| Шаблонизатор      | ViewResolver                   | Описание                                   |
| ----------------- | ------------------------------ | ------------------------------------------ |
| **Thymeleaf**     | `ThymeleafViewResolver`        | Современный, популярный, HTML5-совместимый |
| **FreeMarker**    | `FreeMarkerViewResolver`       | Мощный, гибкий                             |
| **Mustache**      | `MustacheViewResolver`         | Логически бедный, простой                  |
| **JSP**           | `InternalResourceViewResolver` | Классика, устаревший подход                |
| **Groovy Markup** | `GroovyMarkupViewResolver`     | DSL на Groovy                              |

**Thymeleaf в Spring MVC**

Зависимость: `spring-boot-starter-thymeleaf`

Auto-configuration:
- `ThymeleafViewResolver` (приоритет 1)
- шаблоны: `classpath:/templates/`
- суффикс: `.html`

Пример шаблона:

```html
<!DOCTYPE html>
<html xmlns:th="http://www.thymeleaf.org">
<body>
    <h1 th:text="${user.name}">Default Name</h1>
    <ul>
        <li th:each="item : ${items}" th:text="${item.title}"></li>
    </ul>
    <form th:action="@{/users/save}" th:object="${form}" method="post">
        <input type="text" th:field="*{name}"/>
        <span th:errors="*{name}"></span>
    </form>
</body>
</html>
```

**JSP и другие view technologies**

JSP — старый подход (встроен в Tomcat/Jetty).
Не работает с embedded Jetty в fat-jar. Требует `war`-деплой или `spring-boot-starter-tomcat`.

```properties
spring.mvc.view.prefix=/WEB-INF/jsp/
spring.mvc.view.suffix=.jsp
```

----------------------------------------------------------------------------------------------------

## REST в Spring MVC

Spring MVC — это не только HTML. Он является полноценным REST-фреймворком.
Исторически создавался для HTML, но REST-поддержка добавлялась постепенно и сейчас это основной сценарий.

`@RestController`

```java
@RestController
@RequestMapping("/api/v1/orders")
public class OrderController {
    // все методы возвращают тело ответа напрямую
}
```

**JSON-ответы**

По умолчанию Jackson сериализует Java-объекты в JSON.
`Content-Type: application/json` устанавливается автоматически.

```java
@GetMapping
public List<OrderDto> getAll() {
    return orderService.findAll(); // → JSON array
}
```

**DTO как формат обмена**

DTO (Data Transfer Object) — отдельные классы для входа и выхода API:

```java
// Request DTO (входящие данные)
public record CreateOrderRequest(
    @NotNull Long userId,
    @NotEmpty List<@Valid OrderItemRequest> items
) {}

// Response DTO (исходящие данные)
public record OrderDto(
    Long id,
    String status,
    BigDecimal total,
    List<OrderItemDto> items
) {}
```

Никогда не возвращайте Entity напрямую из контроллера.

**ResponseEntity**

```java
@GetMapping("/{id}")
public ResponseEntity<OrderDto> getById(@PathVariable Long id) {
    return orderService.findById(id)
        .map(ResponseEntity::ok)
        .orElseThrow(() -> new OrderNotFoundException(id));
}
```

**Status codes**

| Ситуация                       | Статус                    |
| ------------------------------ | ------------------------- |
| Успешный GET                   | 200 OK                    |
| Успешный POST (создание)       | 201 Created               |
| Успешный DELETE / PUT без тела | 204 No Content            |
| Ресурс не найден               | 404 Not Found             |
| Ошибка валидации               | 400 Bad Request           |
| Ошибка авторизации             | 401 Unauthorized          |
| Нет прав                       | 403 Forbidden             |
| Конфликт                       | 409 Conflict              |
| Внутренняя ошибка              | 500 Internal Server Error |

```java
@ResponseStatus(HttpStatus.CREATED)
@PostMapping
public OrderDto create(@RequestBody @Valid CreateOrderRequest req) { ... }
```

**Headers**

```java
@GetMapping("/{id}")
public ResponseEntity<OrderDto> get(@PathVariable Long id) {
    return ResponseEntity.ok()
        .header("X-Request-Id", UUID.randomUUID().toString())
        .body(orderService.findById(id));
}
```

**Content negotiation**

Spring выбирает формат ответа на основе заголовка `Accept`:

- `Accept: application/json` → JSON
- `Accept: application/xml` → XML (если добавлен `jackson-dataformat-xml`)
- `Accept: */*` → первый доступный converter

```java
@GetMapping(value = "/users", produces = {
    MediaType.APPLICATION_JSON_VALUE,
    MediaType.APPLICATION_XML_VALUE
})
public List<UserDto> getUsers() { ... }
```

**Аннотации REST-раздела**

| Аннотация / тип                                               | Где                  | Что делает                                                                          |
| ------------------------------------------------------------- | -------------------- | ----------------------------------------------------------------------------------- |
| `@RestController`                                             | класс                | Делает все методы body-oriented: return value пишется через `HttpMessageConverter`. |
| `@RequestBody`                                                | параметр             | Десериализует body запроса в DTO.                                                   |
| `@ResponseBody`                                               | метод/класс          | Сериализует return value в body ответа.                                             |
| `@ResponseStatus`                                             | метод/exception      | Задает HTTP status, когда он статичен.                                              |
| `ResponseEntity<T>`                                           | return value         | Status + headers + body, когда ответ зависит от результата.                         |
| `ProblemDetail`                                               | return value         | Стандартный формат REST-ошибки.                                                     |
| `@JsonProperty`, `@JsonIgnore`, `@JsonInclude`, `@JsonFormat` | DTO fields/accessors | Jackson-аннотации для управления JSON-контрактом.                                   |

----------------------------------------------------------------------------------------------------

## 15. HttpMessageConverter

### 15.1. Что это такое

`HttpMessageConverter<T>` — интерфейс для преобразования HTTP message body ↔ Java object.

```java
public interface HttpMessageConverter<T> {
    boolean canRead(Class<?> clazz, MediaType mediaType);
    boolean canWrite(Class<?> clazz, MediaType mediaType);
    T read(Class<? extends T> clazz, HttpInputMessage inputMessage) throws IOException;
    void write(T t, MediaType contentType, HttpOutputMessage outputMessage) throws IOException;
}
```

### 15.2. Преобразование JSON ↔ Java object

`MappingJackson2HttpMessageConverter`:
- `@RequestBody CreateUserRequest request` → Jackson читает JSON в Java object
- Java object из `@ResponseBody` / `@RestController` → Jackson пишет JSON в response body

Условие: `Content-Type: application/json` для запроса, `Accept: application/json` для ответа.

Если параметр метода имеет тип `String`, чаще используется `StringHttpMessageConverter`, а не Jackson.

### 15.3. @RequestBody и converters

```java
@PostMapping("/users")
public UserDto create(@RequestBody CreateUserRequest request) {
    // Spring:
    // 1. Смотрит Content-Type: application/json
    // 2. Ищет converter, который canRead(CreateUserRequest.class, application/json)
    // 3. MappingJackson2HttpMessageConverter.read() → объект
}
```

### 15.4. @ResponseBody и converters

```java
@GetMapping("/users/{id}")
@ResponseBody
public UserDto getUser(@PathVariable Long id) {
    // Spring:
    // 1. Смотрит Accept: application/json
    // 2. Ищет converter, который canWrite(UserDto.class, application/json)
    // 3. MappingJackson2HttpMessageConverter.write() → JSON в body
}
```

### 15.5. Jackson и JSON serialization

Jackson — дефолтная библиотека в Spring Boot.

Кастомизация через `ObjectMapper`:

```java
@Bean
public Jackson2ObjectMapperBuilderCustomizer customizer() {
    return builder -> builder
        .featuresToDisable(SerializationFeature.WRITE_DATES_AS_TIMESTAMPS)
        .featuresToEnable(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES)
        .propertyNamingStrategy(PropertyNamingStrategies.SNAKE_CASE);
}
```

Аннотации на DTO:

```java
@JsonProperty("user_name")       // имя поля в JSON
@JsonIgnore                       // исключить поле
@JsonInclude(NON_NULL)           // не включать null-поля
@JsonFormat(pattern = "dd.MM.yyyy") // формат даты
```

### 15.6. XML / text / binary converters

Встроенные конверторы:

| Converter                                 | MediaType                                         |
| ----------------------------------------- | ------------------------------------------------- |
| `StringHttpMessageConverter`              | `text/plain`, `text/*`                            |
| `MappingJackson2HttpMessageConverter`     | `application/json`                                |
| `MappingJackson2XmlHttpMessageConverter`  | `application/xml` (если `jackson-dataformat-xml`) |
| `ByteArrayHttpMessageConverter`           | `application/octet-stream`                        |
| `ResourceHttpMessageConverter`            | `*/*` (для `Resource`)                            |
| `FormHttpMessageConverter`                | `application/x-www-form-urlencoded`               |
| `AllEncompassingFormHttpMessageConverter` | form data + multipart parts                       |

---

## 16. Работа с HTML-формами

### 16.1. GET- и POST-формы

```html
<!-- GET-форма: данные в URL query string -->
<form th:action="@{/search}" method="get">
    <input type="text" name="query"/>
    <button type="submit">Search</button>
</form>

<!-- POST-форма: данные в body как application/x-www-form-urlencoded -->
<form th:action="@{/register}" th:object="${form}" method="post">
    <input type="text" th:field="*{name}"/>
    <input type="email" th:field="*{email}"/>
    <button type="submit">Register</button>
</form>
```

### 16.2. @ModelAttribute

Используется для передачи формы в контроллер:

```java
@PostMapping("/register")
public String register(@ModelAttribute RegisterForm form) { ... }
```

Также используется для подготовки пустого объекта формы для GET-запроса:

```java
@GetMapping("/register")
public String showForm(Model model) {
    model.addAttribute("form", new RegisterForm()); // передаем пустую форму
    return "register";
}
```

### 16.3. Binding полей формы

Thymeleaf `th:field` автоматически:
- генерирует `id`, `name` атрибуты,
- вставляет текущее значение из объекта модели,
- работает с `th:object` для привязки к форме.

```html
<form th:object="${form}">
    <input th:field="*{email}"/>  <!-- name="email", id="email", value="${form.email}" -->
</form>
```

### 16.4. Validation формы

```java
@PostMapping("/register")
public String register(@Valid @ModelAttribute("form") RegisterForm form,
                       BindingResult result) {
    if (result.hasErrors()) {
        return "register"; // вернуть форму с ошибками
    }
    userService.register(form);
    return "redirect:/login";
}
```

**Важно**: `BindingResult` должен идти сразу после валидируемого объекта!

### 16.5. Повторный показ формы с ошибками

При `result.hasErrors()` → `return "register"`:
- Spring оставляет заполненный form-объект в модели,
- шаблон отображает ранее введенные данные,
- Thymeleaf показывает ошибки через `th:errors`.

```html
<div th:if="${#fields.hasErrors('name')}" class="error">
    <span th:errors="*{name}">Name error</span>
</div>
```

### 16.6. Связка Controller + Model + Thymeleaf form

```java
// Контроллер
@Controller
@RequestMapping("/register")
public class RegisterController {

    @GetMapping
    public String showForm(Model model) {
        model.addAttribute("form", new RegisterForm());
        return "register";
    }

    @PostMapping
    public String submit(@Valid @ModelAttribute("form") RegisterForm form,
                         BindingResult result) {
        if (result.hasErrors()) return "register";
        userService.register(form);
        return "redirect:/login?registered";
    }
}
```

```html
<!-- register.html -->
<form th:action="@{/register}" th:object="${form}" method="post">
    <input type="text" th:field="*{name}" placeholder="Name"/>
    <span th:errors="*{name}" class="error"></span>

    <input type="email" th:field="*{email}" placeholder="Email"/>
    <span th:errors="*{email}" class="error"></span>

    <button type="submit">Register</button>
</form>
```

---

## 17. Обработка исключений

### 17.1. Ошибки в Spring MVC

Исключения могут возникнуть:
- при binding/validation,
- в контроллере,
- в сервисном слое,
- при сериализации ответа.

`DispatcherServlet` перехватывает исключения и передает в `HandlerExceptionResolver`.

### 17.2. @ExceptionHandler

Локальный обработчик ошибок в контроллере:

```java
@Controller
public class UserController {

    @GetMapping("/users/{id}")
    public UserDto getUser(@PathVariable Long id) {
        return userService.findById(id); // throws UserNotFoundException
    }

    @ExceptionHandler(UserNotFoundException.class)
    @ResponseStatus(HttpStatus.NOT_FOUND)
    public ErrorResponse handleNotFound(UserNotFoundException ex) {
        return new ErrorResponse(ex.getMessage());
    }
}
```

Работает только для методов этого контроллера.

Аннотация `@ExceptionHandler` указывает, какие exception-типы обрабатывает метод. Метод может возвращать:
- view name / `ModelAndView` для HTML,
- DTO / `ProblemDetail` для REST,
- `ResponseEntity<?>`, если нужно управлять status и headers.

### 17.3. @ControllerAdvice

Глобальный обработчик для всех контроллеров:

```java
@ControllerAdvice
public class GlobalExceptionHandler {

    @ExceptionHandler(UserNotFoundException.class)
    @ResponseStatus(HttpStatus.NOT_FOUND)
    public String handleNotFound(UserNotFoundException ex, Model model) {
        model.addAttribute("error", ex.getMessage());
        return "error/404";
    }

    @ExceptionHandler(Exception.class)
    @ResponseStatus(HttpStatus.INTERNAL_SERVER_ERROR)
    public String handleGeneral(Exception ex) {
        return "error/500";
    }
}
```

Можно ограничить область: `@ControllerAdvice("com.example.controller")` или `@ControllerAdvice(assignableTypes = UserController.class)`.

### 17.4. @RestControllerAdvice

`@RestControllerAdvice` = `@ControllerAdvice` + `@ResponseBody`.
Все методы возвращают JSON, а не view name.

```java
@RestControllerAdvice
public class GlobalRestExceptionHandler {

    @ExceptionHandler(UserNotFoundException.class)
    @ResponseStatus(HttpStatus.NOT_FOUND)
    public ErrorResponse handleNotFound(UserNotFoundException ex) {
        return new ErrorResponse("NOT_FOUND", ex.getMessage());
    }

    @ExceptionHandler(MethodArgumentNotValidException.class)
    @ResponseStatus(HttpStatus.BAD_REQUEST)
    public ErrorResponse handleValidation(MethodArgumentNotValidException ex) {
        List<String> details = ex.getBindingResult().getFieldErrors().stream()
            .map(e -> e.getField() + ": " + e.getDefaultMessage())
            .toList();
        return new ErrorResponse("VALIDATION_ERROR", details);
    }

    @ExceptionHandler(Exception.class)
    @ResponseStatus(HttpStatus.INTERNAL_SERVER_ERROR)
    public ErrorResponse handleGeneral(Exception ex) {
        return new ErrorResponse("INTERNAL_ERROR", "Unexpected error occurred");
    }
}
```

### 17.4.1. Аннотации и классы для ошибок

| Аннотация / класс                | Где используется          | Что делает                                                                              |
| -------------------------------- | ------------------------- | --------------------------------------------------------------------------------------- |
| `@ExceptionHandler`              | метод                     | Обрабатывает исключения указанного типа.                                                |
| `@ControllerAdvice`              | класс                     | Глобальный advice для MVC: exceptions, global model attributes, binders.                |
| `@RestControllerAdvice`          | класс                     | `@ControllerAdvice` + `@ResponseBody`, удобно для JSON API.                             |
| `@ResponseStatus`                | метод или exception class | Задает HTTP status. Для сложных REST-ошибок лучше `ResponseEntity` или `ProblemDetail`. |
| `ResponseStatusException`        | throw из кода             | Быстро пробросить HTTP status + reason без своего exception class.                      |
| `ProblemDetail`                  | return value/body         | Стандартный формат HTTP API errors (`application/problem+json`).                        |
| `ResponseEntityExceptionHandler` | base class для advice     | Удобная база для переопределения стандартных Spring MVC exception handlers.             |

Пример с `ProblemDetail`:

```java
@RestControllerAdvice
public class ApiExceptionHandler {
    @ExceptionHandler(UserNotFoundException.class)
    public ProblemDetail handleNotFound(UserNotFoundException ex) {
        ProblemDetail problem = ProblemDetail.forStatus(HttpStatus.NOT_FOUND);
        problem.setTitle("User not found");
        problem.setDetail(ex.getMessage());
        problem.setProperty("code", "USER_NOT_FOUND");
        return problem;
    }
}
```

### 17.5. Глобальная обработка исключений

Рекомендуемый подход для REST API:
1. Создать иерархию кастомных исключений: `BusinessException`, `NotFoundException`, `ConflictException`.
2. Создать единый `@RestControllerAdvice`.
3. Определить единый формат ошибок: свой DTO или стандартный `ProblemDetail`.

```java
public record ErrorResponse(
    String code,
    String message,
    List<String> details,
    Instant timestamp
) {}
```

### 17.6. Возврат ошибок в HTML

```java
@ControllerAdvice
public class HtmlExceptionHandler {
    @ExceptionHandler(AccessDeniedException.class)
    public ModelAndView handleAccessDenied(AccessDeniedException ex) {
        ModelAndView mav = new ModelAndView("error/403");
        mav.addObject("message", ex.getMessage());
        return mav;
    }
}
```

### 17.7. Возврат ошибок в JSON

```java
@RestControllerAdvice
@RequestMapping(produces = MediaType.APPLICATION_JSON_VALUE)
public class JsonExceptionHandler {

    @ExceptionHandler(NotFoundException.class)
    public ResponseEntity<ErrorResponse> handleNotFound(NotFoundException ex) {
        return ResponseEntity.status(404)
            .body(new ErrorResponse("NOT_FOUND", ex.getMessage()));
    }
}
```

---

## 18. Interceptors

### 18.1. Что такое interceptor

`HandlerInterceptor` — компонент Spring MVC, который встраивается в цепочку обработки запроса.

В отличие от Servlet Filter (уровень контейнера) — interceptor живет внутри Spring MVC и имеет доступ к handler и ModelAndView.

### 18.2. preHandle

Выполняется **до** вызова контроллера.

```java
@Override
public boolean preHandle(HttpServletRequest request,
                          HttpServletResponse response,
                          Object handler) throws Exception {
    log.info("Request: {} {}", request.getMethod(), request.getRequestURI());
    // Вернуть false — прервать обработку (ответ уже должен быть записан в response)
    return true;
}
```

### 18.3. postHandle

Выполняется **после** контроллера, но **до** рендеринга view.

Не вызывается, если контроллер выбросил исключение до нормального return value.
При `@ResponseBody` / `@RestController` метод может быть вызван, но `ModelAndView` обычно будет `null`, потому что ответ формируется через `HttpMessageConverter`, а не через view.

```java
@Override
public void postHandle(HttpServletRequest request,
                        HttpServletResponse response,
                        Object handler,
                        ModelAndView modelAndView) throws Exception {
    if (modelAndView != null) {
        modelAndView.addObject("globalData", globalService.getData());
    }
}
```

### 18.4. afterCompletion

Выполняется после завершения запроса для interceptor'ов, чей `preHandle` успешно вернул `true`.
По смыслу похож на `finally`: сюда попадают и успешные запросы, и запросы с exception после прохождения interceptor'а.

```java
@Override
public void afterCompletion(HttpServletRequest request,
                              HttpServletResponse response,
                              Object handler,
                              Exception ex) throws Exception {
    long duration = System.currentTimeMillis() - startTime;
    log.info("Request completed in {}ms, exception: {}", duration, ex);
    // очистка ThreadLocal и т.д.
}
```

### 18.5. Для чего используются interceptors

- Логирование времени выполнения запроса.
- Проверка токена/сессии (light-weight auth, без Spring Security).
- Установка Locale.
- Передача глобальных данных во view (через `postHandle`).
- Аудит-логирование.
- Rate limiting (легковесный).
- MDC/трейсинг — добавление correlation ID в ThreadLocal.

### 18.6. Отличие interceptor от filter

|                       | Servlet Filter           | HandlerInterceptor               |
| --------------------- | ------------------------ | -------------------------------- |
| Уровень               | Servlet Container        | Spring MVC                       |
| Когда                 | вокруг DispatcherServlet | внутри MVC-обработки             |
| Доступ к handler      | нет                      | да (знает метод контроллера)     |
| Доступ к ModelAndView | нет                      | да (в postHandle)                |
| Spring-бины           | через ApplicationContext | да, сам является Spring-бином    |
| Применение            | CORS, encoding, security | логирование, auth, i18n, метрики |

Регистрация interceptor:

```java
@Configuration
public class WebConfig implements WebMvcConfigurer {
    @Override
    public void addInterceptors(InterceptorRegistry registry) {
        registry.addInterceptor(new LoggingInterceptor())
                .addPathPatterns("/api/**")
                .excludePathPatterns("/api/public/**");
    }
}
```

---

## 19. Filters и место Spring MVC в servlet stack

### 19.1. Что такое servlet filter

`Filter` — компонент Jakarta Servlet API (`jakarta.servlet.Filter`), выполняющийся вокруг сервлетов.

```java
@Component
public class RequestLoggingFilter implements Filter {
    @Override
    public void doFilter(ServletRequest request,
                          ServletResponse response,
                          FilterChain chain) throws IOException, ServletException {
        // до сервлета
        chain.doFilter(request, response); // вызов следующего filter / сервлета
        // после сервлета
    }
}
```

### 19.2. Разница между Filter и Interceptor

|                            | Filter                                                                                                              | Interceptor                          |
| -------------------------- | ------------------------------------------------------------------------------------------------------------------- | ------------------------------------ |
| Спецификация               | Servlet API                                                                                                         | Spring MVC                           |
| Уровень                    | До/вокруг DispatcherServlet                                                                                         | Внутри MVC (после DispatcherServlet) |
| Доступ к Spring Context    | да, если filter зарегистрирован как Spring bean (`@Component`, `FilterRegistrationBean`, security chain); иначе нет | да, является Spring-бином            |
| Знает handler              | нет                                                                                                                 | да                                   |
| Может читать/изменять тело | да (оборачивая)                                                                                                     | нет напрямую                         |
| Используется для           | CORS, GZIP, security, encoding                                                                                      | логирование, auth, locale, метрики   |

### 19.3. Где заканчивается servlet container и начинается Spring MVC

```
TCP Connection
    └─► Servlet Container (Tomcat)
            └─► Filter Chain (Servlet API)
                    └─► DispatcherServlet (Spring MVC начинается здесь)
                            └─► HandlerInterceptors
                            └─► Controller
```

### 19.4. Примеры использования filters

| Filter                        | Назначение                                        |
| ----------------------------- | ------------------------------------------------- |
| `CharacterEncodingFilter`     | Устанавливает кодировку UTF-8                     |
| `CorsFilter`                  | CORS-заголовки                                    |
| `OncePerRequestFilter`        | Базовый класс для filter, выполняющегося один раз |
| Spring Security Filter Chain  | Вся цепочка безопасности                          |
| `CommonsRequestLoggingFilter` | Логирование запросов                              |
| `ShallowEtagHeaderFilter`     | ETag для кеширования                              |

---

## 20. Работа с файлами

### 20.1. Загрузка файлов

Spring MVC обрабатывает `multipart/form-data` через `MultipartResolver`.

Boot автоконфигурирует `StandardServletMultipartResolver`.

Настройки (application.properties):

```properties
spring.servlet.multipart.enabled=true
spring.servlet.multipart.max-file-size=10MB
spring.servlet.multipart.max-request-size=20MB
```

### 20.2. MultipartFile

```java
@PostMapping("/upload")
public ResponseEntity<String> upload(@RequestParam("file") MultipartFile file) {
    if (file.isEmpty()) {
        return ResponseEntity.badRequest().body("Empty file");
    }
    String filename = StringUtils.cleanPath(file.getOriginalFilename());
    Path targetPath = uploadDir.resolve(filename);
    Files.copy(file.getInputStream(), targetPath, StandardCopyOption.REPLACE_EXISTING);
    return ResponseEntity.ok("Uploaded: " + filename);
}
```

В production нельзя доверять `getOriginalFilename()` напрямую: проверяйте path traversal (`..`), whitelist расширений/media types, лимиты размера и права доступа. Часто безопаснее генерировать свое имя файла и хранить оригинальное имя отдельно как metadata.

Несколько файлов:

```java
@PostMapping("/upload-multiple")
public List<String> uploadMultiple(@RequestParam("files") List<MultipartFile> files) {
    return files.stream()
        .map(f -> storageService.store(f))
        .toList();
}
```

### 20.3. multipart/form-data

HTML-форма для загрузки файла:

```html
<form th:action="@{/upload}" method="post" enctype="multipart/form-data">
    <input type="file" name="file"/>
    <button type="submit">Upload</button>
</form>
```

`enctype="multipart/form-data"` — обязательно!

### 20.4. Отдача файлов клиенту

```java
@GetMapping("/files/{filename:.+}")
public ResponseEntity<Resource> download(@PathVariable String filename) throws IOException {
    Path filePath = uploadDir.resolve(filename).normalize();
    Resource resource = new UrlResource(filePath.toUri());

    if (!resource.exists()) {
        return ResponseEntity.notFound().build();
    }

    String contentType = Files.probeContentType(filePath);
    if (contentType == null) {
        contentType = MediaType.APPLICATION_OCTET_STREAM_VALUE;
    }

    return ResponseEntity.ok()
        .contentType(MediaType.parseMediaType(contentType))
        .header(HttpHeaders.CONTENT_DISPOSITION,
                "attachment; filename=\"" + resource.getFilename() + "\"")
        .body(resource);
}
```

Для скачивания пользовательских файлов обязательно проверяйте, что `normalize()` не вывел путь за пределы upload-директории:

```java
Path root = uploadDir.toAbsolutePath().normalize();
Path filePath = root.resolve(filename).normalize();
if (!filePath.startsWith(root)) {
    throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "Invalid filename");
}
```

### 20.5. Resource

`Resource` — Spring-абстракция над файлами и потоками данных.

Реализации:
- `FileSystemResource` — файл на диске,
- `ClassPathResource` — файл из classpath,
- `UrlResource` — файл по URL,
- `ByteArrayResource` — данные в памяти.

### 20.6. Content-Disposition

Заголовок `Content-Disposition` контролирует поведение браузера:

```java
// Скачать файл (браузер предложит сохранить):
.header(HttpHeaders.CONTENT_DISPOSITION, "attachment; filename=\"report.pdf\"")

// Открыть в браузере (inline):
.header(HttpHeaders.CONTENT_DISPOSITION, "inline; filename=\"image.png\"")
```

---

## 21. Redirect и Forward

### 21.1. Что такое redirect

**Redirect** — сервер отвечает `3xx` с заголовком `Location`. Браузер делает **новый** HTTP-запрос по новому URL.

- URL в браузере меняется.
- Создается новый request/response.
- Данные из первого request не доступны (только flash attributes).
- Два HTTP-запроса.

### 21.2. Что такое forward

**Forward** — сервер **внутри себя** передает обработку другому handler'у.

- URL в браузере не меняется.
- Тот же request/response.
- Данные request доступны в target.
- Один HTTP-запрос.

### 21.3. Разница между redirect и forward

|                                 | Redirect              | Forward                        |
| ------------------------------- | --------------------- | ------------------------------ |
| HTTP-запросов                   | 2                     | 1                              |
| URL меняется                    | да                    | нет                            |
| Request-объект                  | новый                 | тот же                         |
| Данные из оригинального request | нет                   | да                             |
| Используется для                | POST → redirect → GET | внутренняя передача управления |

### 21.4. PRG pattern (Post-Redirect-Get)

Паттерн для предотвращения двойной отправки формы:

```
1. POST /register — обработка формы
2. redirect:/login  — 302 Found, Location: /login
3. GET /login  — браузер делает новый запрос
```

Без redirect: при обновлении страницы браузер предлагает повторить POST.
С redirect: обновление страницы повторяет GET (безопасно).

```java
@PostMapping("/register")
public String register(@Valid @ModelAttribute RegisterForm form, BindingResult result) {
    if (result.hasErrors()) return "register";
    userService.register(form);
    return "redirect:/login?registered"; // PRG
}
```

### 21.5. Когда использовать каждый вариант

- **Redirect**: после успешного POST (PRG), перенаправление на другую страницу, смена URL.
- **Forward**: когда нужно передать управление другому контроллеру с тем же request (редко).

---

## 22. Асинхронная обработка в Spring MVC

### 22.1. Асинхронность в servlet stack

Spring MVC работает на Servlet API, где один запрос = один поток.
Servlet 3.0+ поддерживает **async processing**: поток можно освободить, пока идет долгая операция, а затем вернуть результат.

Важно: Spring MVC async освобождает servlet-поток, но не делает блокирующий код неблокирующим. Если внутри `Callable` выполняется JDBC-запрос или blocking HTTP call, он всё равно занимает поток executor'а. Для настоящего non-blocking I/O нужен WebFlux/reactive stack или неблокирующие клиенты.

Для production обычно настраивают:
- `AsyncTaskExecutor`,
- timeout,
- propagation `SecurityContext`/MDC,
- обработку timeout/error callbacks.

### 22.2. Callable

Контроллер возвращает `Callable<T>`. Spring выполняет его в отдельном потоке (из TaskExecutor), освобождая servlet-поток.

```java
@GetMapping("/heavy")
public Callable<String> heavyOperation() {
    return () -> {
        Thread.sleep(5000); // симуляция долгой работы
        return "result";    // выполняется в другом потоке
    };
}
```

### 22.3. DeferredResult

Результат устанавливается извне (из другого потока, по событию, из MQ):

```java
@GetMapping("/subscribe")
public DeferredResult<String> subscribe() {
    DeferredResult<String> result = new DeferredResult<>(10_000L); // timeout 10s

    // Кто-то установит результат позже:
    eventBus.subscribe(event -> result.setResult(event.getData()));

    result.onTimeout(() -> result.setResult("timeout"));
    return result;
}
```

### 22.4. WebAsyncTask

Как `Callable`, но с возможностью задать timeout и executor:

```java
@GetMapping("/task")
public WebAsyncTask<String> asyncTask() {
    return new WebAsyncTask<>(5000L, () -> heavyService.process());
}
```

### 22.5. StreamingResponseBody

Потоковая отдача больших данных без буферизации:

```java
@GetMapping("/export")
public StreamingResponseBody export() {
    return outputStream -> {
        try (PrintWriter writer = new PrintWriter(outputStream)) {
            for (int i = 0; i < 1_000_000; i++) {
                writer.println("line " + i);
                writer.flush();
            }
        }
    };
}
```

### 22.6. SseEmitter

Server-Sent Events — сервер push'ит события клиенту через постоянное HTTP-соединение:

```java
@GetMapping(value = "/events", produces = MediaType.TEXT_EVENT_STREAM_VALUE)
public SseEmitter streamEvents() {
    SseEmitter emitter = new SseEmitter(30_000L);

    executor.submit(() -> {
        try {
            for (int i = 0; i < 10; i++) {
                emitter.send(SseEmitter.event()
                    .name("update")
                    .data("event " + i));
                Thread.sleep(1000);
            }
            emitter.complete();
        } catch (Exception e) {
            emitter.completeWithError(e);
        }
    });

    return emitter;
}
```

---

## 23. Статические ресурсы

### 23.1. Что считается static content

CSS, JavaScript, изображения, шрифты, favicon.ico — файлы, которые отдаются как есть, без обработки.

### 23.2. CSS / JS / images

Spring Boot по умолчанию раздает статику из:
- `classpath:/static/`
- `classpath:/public/`
- `classpath:/resources/`
- `classpath:/META-INF/resources/`
- root of `ServletContext` при war deployment

```
src/main/resources/static/
    css/
        main.css     → /css/main.css
    js/
        app.js       → /js/app.js
    images/
        logo.png     → /images/logo.png
```

### 23.3. Настройка resource handling

```java
@Configuration
public class WebConfig implements WebMvcConfigurer {
    @Override
    public void addResourceHandlers(ResourceHandlerRegistry registry) {
        registry.addResourceHandler("/files/**")
                .addResourceLocations("file:/opt/uploads/")
                .setCacheControl(CacheControl.maxAge(7, TimeUnit.DAYS));
    }
}
```

### 23.4. Раздача статических файлов

```properties
# Изменить базовый путь (по умолчанию /**)
spring.mvc.static-path-pattern=/static/**

# Изменить источник:
spring.web.resources.static-locations=classpath:/static/,classpath:/public/
```

В Thymeleaf:

```html
<link rel="stylesheet" th:href="@{/css/main.css}"/>
<script th:src="@{/js/app.js}"></script>
<img th:src="@{/images/logo.png}" alt="Logo"/>
```

---

## 24. Локализация и интернационализация

### 24.1. Locale

`Locale` — объект, представляющий язык и регион пользователя (например, `ru_RU`, `en_US`).

Spring MVC определяет Locale через `LocaleResolver`.

### 24.2. LocaleResolver

`LocaleResolver` — интерфейс, определяющий текущий `Locale` запроса.

Реализации:
- **`AcceptHeaderLocaleResolver`** — из `Accept-Language` header (дефолт).
- `CookieLocaleResolver` — из cookie.
- `SessionLocaleResolver` — из HTTP-сессии.
- `FixedLocaleResolver` — фиксированный Locale.

```java
@Bean
public LocaleResolver localeResolver() {
    SessionLocaleResolver resolver = new SessionLocaleResolver();
    resolver.setDefaultLocale(Locale.ENGLISH);
    return resolver;
}

// Смена языка:
@Bean
public LocaleChangeInterceptor localeChangeInterceptor() {
    LocaleChangeInterceptor interceptor = new LocaleChangeInterceptor();
    interceptor.setParamName("lang"); // GET /page?lang=ru
    return interceptor;
}
```

### 24.3. MessageSource

`MessageSource` — интерфейс для получения локализованных сообщений.

```properties
# messages.properties (дефолт):
user.greeting=Hello, {0}!

# messages_ru.properties:
user.greeting=Привет, {0}!
```

```java
@Autowired
private MessageSource messageSource;

public String greet(String name, Locale locale) {
    return messageSource.getMessage("user.greeting", new Object[]{name}, locale);
}
```

Настройка:

```properties
spring.messages.basename=messages
spring.messages.encoding=UTF-8
```

### 24.4. i18n в Spring MVC

```java
@GetMapping("/greeting")
public String greeting(Locale locale, Model model) {
    String msg = messageSource.getMessage("greeting", null, locale);
    model.addAttribute("message", msg);
    return "greeting";
}
```

### 24.5. Локализованные сообщения в представлениях

В Thymeleaf:

```html
<!-- Простое сообщение: -->
<p th:text="#{user.greeting}"></p>

<!-- С параметром: -->
<p th:text="#{user.greeting(${user.name})}"></p>

<!-- Сообщения ошибок валидации тоже из MessageSource: -->
<span th:errors="*{email}"></span>
```

---

## 25. Session и Flash Attributes

### 25.1. Работа с HttpSession

```java
@GetMapping("/cart/add/{id}")
public String addToCart(@PathVariable Long id, HttpSession session) {
    Cart cart = (Cart) session.getAttribute("cart");
    if (cart == null) cart = new Cart();
    cart.add(productService.findById(id));
    session.setAttribute("cart", cart);
    return "redirect:/cart";
}
```

### 25.2. Хранение состояния между запросами

HTTP — stateless протокол. Для хранения состояния между запросами:
- **Session** — хранится на сервере, ID в cookie (`JSESSIONID`).
- **Cookie** — хранится в браузере.
- **JWT/Token** — в header или cookie, stateless.
- **Flash attributes** — одноразовые данные для redirect.

### 25.3. @SessionAttributes

Хранит атрибуты модели в сессии для использования между запросами в рамках одного контроллера:

```java
@Controller
@SessionAttributes("wizard")   // сохраняем объект "wizard" в сессии
public class WizardController {

    @GetMapping("/step1")
    public String step1(Model model) {
        model.addAttribute("wizard", new WizardData()); // создаем и кладем в сессию
        return "wizard/step1";
    }

    @PostMapping("/step2")
    public String step2(@ModelAttribute WizardData wizard) { // берем из сессии
        wizard.setStep2Data(...);
        return "wizard/step2";
    }

    @PostMapping("/finish")
    public String finish(@ModelAttribute WizardData wizard, SessionStatus status) {
        wizardService.complete(wizard);
        status.setComplete(); // убираем из сессии
        return "redirect:/done";
    }
}
```

### 25.4. Flash attributes

**Flash attributes** — данные, которые живут ровно один redirect (один запрос после redirect).

Используются для передачи сообщений об успехе/ошибке после POST-redirect-GET:

```java
@PostMapping("/order/create")
public String createOrder(RedirectAttributes redirectAttributes) {
    orderService.create(...);
    redirectAttributes.addFlashAttribute("successMessage", "Order created!");
    return "redirect:/orders";
}

@GetMapping("/orders")
public String orders(Model model) {
    // "successMessage" автоматически появится в модели, если был установлен через flash
    return "orders";
}
```

В Thymeleaf:

```html
<div th:if="${successMessage}" class="alert alert-success">
    <span th:text="${successMessage}"></span>
</div>
```

### 25.5. RedirectAttributes

Интерфейс для передачи данных через redirect:

```java
// Flash attribute (в сессии, исчезает после одного запроса):
redirectAttributes.addFlashAttribute("message", "Saved!");

// Query param (добавляется к URL):
redirectAttributes.addAttribute("id", createdId); // redirect:/items?id=42
```

Аннотации и типы раздела:

| Аннотация / тип      | Где используется    | Что делает                                                                                       |
| -------------------- | ------------------- | ------------------------------------------------------------------------------------------------ |
| `@SessionAttributes` | класс `@Controller` | Сохраняет выбранные model attributes в HTTP session в рамках controller workflow.                |
| `@ModelAttribute`    | метод/параметр      | Создает/читает model attribute; вместе с `@SessionAttributes` может доставать объект из session. |
| `SessionStatus`      | параметр метода     | Позволяет вызвать `setComplete()` и очистить session attributes текущего controller workflow.    |
| `RedirectAttributes` | параметр метода     | Добавляет query params (`addAttribute`) или flash attributes (`addFlashAttribute`) для redirect. |
| `HttpSession`        | параметр метода     | Низкоуровневый прямой доступ к server-side session.                                              |

`@SessionAttributes` не стоит использовать как общий storage пользователя. Это инструмент для небольших MVC-flow, например wizard form. Для security/session management используйте Spring Security и явные сервисы.

---

## 26. Конфигурирование Spring MVC

### 26.1. @EnableWebMvc

Включает полную MVC-конфигурацию Spring MVC в Java Config:

```java
@Configuration
@EnableWebMvc
public class WebConfig { ... }
```

**В Spring Boot**: `@EnableWebMvc` говорит приложению: “я сам полностью управляю MVC-конфигурацией”. Поэтому Boot MVC auto-configuration больше не добавляет свои обычные настройки поверх. В Boot-приложениях почти всегда используйте `WebMvcConfigurer` без `@EnableWebMvc`.

Аннотации раздела:

| Аннотация        | Где ставится | Что делает                                                                          |
| ---------------- | ------------ | ----------------------------------------------------------------------------------- |
| `@Configuration` | класс        | Объявляет Java config class со Spring beans/customizers.                            |
| `@EnableWebMvc`  | config class | Импортирует MVC-конфигурацию Spring Framework напрямую; в Boot обычно не нужна.     |
| `@Bean`          | метод        | Регистрирует объект как Spring bean, например `LocaleResolver` или `MessageSource`. |

### 26.2. Java Config

```java
@Configuration
public class WebConfig implements WebMvcConfigurer {
    // переопределяем только нужное
}
```

### 26.3. Настройка view resolvers

```java
@Override
public void configureViewResolvers(ViewResolverRegistry registry) {
    registry.jsp("/WEB-INF/jsp/", ".jsp");
    // или:
    registry.viewResolver(new InternalResourceViewResolver("/WEB-INF/views/", ".html"));
}
```

### 26.4. Настройка converters

```java
@Override
public void configureMessageConverters(List<HttpMessageConverter<?>> converters) {
    // полностью заменяет список
    converters.add(new MappingJackson2HttpMessageConverter());
}

@Override
public void extendMessageConverters(List<HttpMessageConverter<?>> converters) {
    // расширяет список (предпочтительнее)
    converters.add(0, new CustomConverter());
}
```

### 26.5. Настройка interceptors

```java
@Override
public void addInterceptors(InterceptorRegistry registry) {
    registry.addInterceptor(new LoggingInterceptor());
    registry.addInterceptor(new AuthInterceptor())
            .addPathPatterns("/admin/**")
            .excludePathPatterns("/admin/login");
}
```

### 26.6. Настройка resource handlers

```java
@Override
public void addResourceHandlers(ResourceHandlerRegistry registry) {
    registry.addResourceHandler("/static/**")
            .addResourceLocations("classpath:/static/")
            .setCacheControl(CacheControl.maxAge(365, TimeUnit.DAYS).cachePublic());
}
```

### 26.7. WebMvcConfigurer

Интерфейс с default-методами. Реализуйте только то, что нужно:

```java
@Configuration
public class WebConfig implements WebMvcConfigurer {

    @Override
    public void addCorsMappings(CorsRegistry registry) {
        registry.addMapping("/api/**")
                .allowedOrigins("https://example.com")
                .allowedMethods("GET", "POST", "PUT", "DELETE");
    }

    @Override
    public void addFormatters(FormatterRegistry registry) {
        registry.addConverter(new StringToEnumConverter());
    }

    @Override
    public void configureDefaultServletHandling(DefaultServletHandlerConfigurer configurer) {
        configurer.enable(); // передать статику в default servlet
    }
}
```

---

## 27. Spring MVC и Spring Boot

### 27.1. Как Boot автоконфигурирует MVC

Spring Boot через `@EnableAutoConfiguration` подключает `WebMvcAutoConfiguration`, если `spring-webmvc` в classpath.

`WebMvcAutoConfiguration`:
- регистрирует `DispatcherServlet`,
- конфигурирует `Jackson` / `Gson`,
- настраивает resource handlers,
- регистрирует default ViewResolver'ы,
- настраивает content negotiation,
- настраивает message converters.

### 27.2. spring-boot-starter-webmvc

В Spring Boot 4 servlet MVC starter называется `spring-boot-starter-webmvc`:

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-webmvc</artifactId>
</dependency>
```

В Spring Boot 3.x обычно используется старое имя:

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-web</artifactId>
</dependency>
```

Транзитивно подтягивает:
- `spring-webmvc`
- `spring-web`
- embedded Tomcat (`spring-boot-starter-tomcat`)
- Jackson (`jackson-databind`)
- `slf4j`

Bean Validation подключается отдельно:

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-validation</artifactId>
</dependency>
```

### 27.3. Что Boot настраивает автоматически

| Компонент                    | По умолчанию                                                     |
| ---------------------------- | ---------------------------------------------------------------- |
| `DispatcherServlet`          | маппинг на `/`                                                   |
| `Jackson ObjectMapper`       | настроен разумно                                                 |
| `ContentNegotiationStrategy` | по `Accept` header                                               |
| Static resources             | `/static/`, `/public/`, `/resources/`                            |
| `MessageSource`              | `messages.properties`                                            |
| Error handling               | `/error` → `BasicErrorController`                                |
| Multipart                    | включен, лимиты настраиваются через `spring.servlet.multipart.*` |

### 27.4. Когда нужна ручная конфигурация

- Нужны кастомные interceptors.
- Нужен кастомный `ObjectMapper`.
- Нужны кастомные CORS настройки.
- Нужен нестандартный resource handler.
- Нужен нестандартный ViewResolver.

```java
@Configuration
public class WebConfig implements WebMvcConfigurer {
    // Без @EnableWebMvc — Boot auto-configuration остается активной,
    // мы лишь расширяем её.
}
```

### 27.5. MVC в Boot-приложении

```java
@SpringBootApplication
public class Application {
    public static void main(String[] args) {
        SpringApplication.run(Application.class, args);
    }
}
```

Boot сам запускает embedded Tomcat, регистрирует `DispatcherServlet`, поднимает ApplicationContext.
Порт по умолчанию: `8080`. Конфигурация: `application.properties`.

```properties
server.port=8080
server.servlet.context-path=/app
spring.mvc.servlet.path=/mvc  # путь DispatcherServlet
```

Аннотация `@SpringBootApplication` объединяет:
- `@SpringBootConfiguration`,
- `@EnableAutoConfiguration`,
- `@ComponentScan`.

Именно `@EnableAutoConfiguration` включает `WebMvcAutoConfiguration`, если в classpath есть Spring MVC и servlet stack.

---

## 28. Практические стили использования Spring MVC

### 28.1. Server-side rendered приложение

Сервер рендерит HTML через шаблонизатор.

Стек:
- `@Controller` (не `@RestController`)
- Thymeleaf или FreeMarker
- `Model` для передачи данных
- Redirect после POST (PRG)
- Flash attributes для сообщений

### 28.2. REST API приложение

Сервер отдает JSON, фронтенд — отдельный (React, Vue, etc.).

Стек:
- `@RestController`
- DTO в/из JSON
- `ResponseEntity` для контроля статусов
- `@ControllerAdvice` для ошибок
- OpenAPI/Swagger для документации

### 28.3. Гибридное приложение

И HTML-страницы, и JSON API в одном приложении.

Стек:
- Разные контроллеры: `UserPageController` и `UserApiController`
- Разные URL: `/users/**` и `/api/users/**`
- Shared сервисный слой

### 28.4. MVC для Thymeleaf

```java
@Controller
@RequestMapping("/users")
public class UserPageController {

    @GetMapping
    public String list(Model model, @RequestParam(defaultValue = "0") int page) {
        model.addAttribute("users", userService.findAll(PageRequest.of(page, 20)));
        return "users/list";
    }

    @GetMapping("/new")
    public String newForm(Model model) {
        model.addAttribute("form", new UserForm());
        return "users/form";
    }

    @PostMapping
    public String create(@Valid @ModelAttribute UserForm form,
                         BindingResult result,
                         RedirectAttributes flash) {
        if (result.hasErrors()) return "users/form";
        userService.create(form);
        flash.addFlashAttribute("success", "User created");
        return "redirect:/users";
    }
}
```

### 28.5. MVC для JSON API

```java
@RestController
@RequestMapping("/api/v1/users")
@RequiredArgsConstructor
public class UserApiController {

    private final UserService userService;

    @GetMapping
    public PagedResponse<UserDto> getAll(Pageable pageable) {
        return userService.findAll(pageable);
    }

    @GetMapping("/{id}")
    public UserDto getById(@PathVariable Long id) {
        return userService.findById(id);
    }

    @PostMapping
    @ResponseStatus(HttpStatus.CREATED)
    public UserDto create(@RequestBody @Valid CreateUserRequest request) {
        return userService.create(request);
    }

    @PutMapping("/{id}")
    public UserDto update(@PathVariable Long id,
                          @RequestBody @Valid UpdateUserRequest request) {
        return userService.update(id, request);
    }

    @DeleteMapping("/{id}")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void delete(@PathVariable Long id) {
        userService.delete(id);
    }
}
```

---

## 29. Лучшие практики

### 29.1. Тонкие контроллеры

Контроллер = только HTTP-слой. Никакой бизнес-логики:

```java
// Плохо:
@PostMapping("/orders")
public OrderDto create(@RequestBody CreateOrderRequest req) {
    // проверка инвентаря, расчет цены, отправка email — НЕ ЗДЕСЬ
    if (inventoryService.getStock(req.getProductId()) < req.getQuantity()) {
        throw new IllegalStateException("Not enough stock");
    }
    ...
}

// Хорошо:
@PostMapping("/orders")
public ResponseEntity<OrderDto> create(@RequestBody @Valid CreateOrderRequest req) {
    return ResponseEntity.status(201).body(orderService.create(req));
}
```

### 29.2. Вынос бизнес-логики в сервисы

- Сервис управляет транзакциями (`@Transactional`).
- Сервис содержит бизнес-правила.
- Сервис оркестрирует вызовы нескольких репозиториев.
- Контроллер не работает с репозиторием напрямую.

### 29.3. Использование DTO

```java
// НЕ возвращайте Entity:
@GetMapping("/{id}")
public User getUser(@PathVariable Long id) { ... } // плохо

// Возвращайте DTO:
@GetMapping("/{id}")
public UserDto getUser(@PathVariable Long id) { ... } // хорошо
```

Отдельные DTO для запроса и ответа: `CreateUserRequest`, `UpdateUserRequest`, `UserDto`, `UserSummaryDto`.

### 29.4. Единый формат ошибок

```java
public record ErrorResponse(
    String code,
    String message,
    List<FieldError> errors,
    Instant timestamp
) {
    public record FieldError(String field, String message) {}
}
```

Один `@RestControllerAdvice` для всего приложения.

### 29.5. Валидация на границе приложения

Синтаксическая/контрактная валидация (`@NotBlank`, `@Email`, `@Size`) — на границе приложения: controller DTO.
Бизнес-правила (например, уникальность email, доступность товара, лимиты пользователя) — в сервисе.

Сервисный слой не должен слепо полагаться на то, что его всегда вызывает только HTTP-контроллер: сервисы часто переиспользуются из jobs, message listeners и tests.

### 29.6. Разделение page controllers и api controllers

```
com.example.web.pages     → @Controller с Thymeleaf (URL: /*)
com.example.web.api       → @RestController с JSON (URL: /api/*)
com.example.service       → shared сервисный слой
```

---

## 30. Частые ошибки и подводные камни

### 30.1. Путаница между @Controller и @RestController

```java
// Ошибка: @Controller + возврат объекта без @ResponseBody
@Controller
public class UserController {
    @GetMapping("/users/{id}")
    public UserDto getUser(@PathVariable Long id) {
        return userService.findById(id); // Spring ищет view "UserDto", получает ошибку!
    }
}

// Правильно:
@RestController  // или добавить @ResponseBody на метод
public class UserController { ... }
```

### 30.2. Неправильный возврат String

```java
// @RestController: String как тело ответа
@RestController
public class Controller {
    @GetMapping("/view")
    public String test() {
        return "hello"; // вернет строку "hello" как тело, НЕ view!
    }
}

// @Controller: String как имя view
@Controller
public class Controller {
    @GetMapping("/page")
    public String page() {
        return "home"; // ViewResolver ищет templates/home.html
    }
}
```

### 30.3. Ошибки binding и validation

```java
// Ошибка: BindingResult не сразу после валидируемого объекта
@PostMapping("/register")
public String register(BindingResult result, @Valid RegisterForm form) { ... } // WRONG!

// Правильно:
@PostMapping("/register")
public String register(@Valid RegisterForm form, BindingResult result) { ... } // OK
```

```java
// Ошибка: не проверять BindingResult (и форма молча проходит)
@PostMapping("/register")
public String register(@Valid @ModelAttribute RegisterForm form, BindingResult result) {
    userService.register(form); // вызовется даже при ошибках валидации!
    return "redirect:/login";
}

// Правильно:
if (result.hasErrors()) return "register";
```

### 30.4. Смешивание view-логики и бизнес-логики

```java
// Плохо: бизнес-логика в контроллере:
@GetMapping("/dashboard")
public String dashboard(Model model) {
    List<Order> orders = orderRepository.findAll(); // напрямую репозиторий
    BigDecimal total = orders.stream().map(Order::getTotal).reduce(BigDecimal.ZERO, BigDecimal::add);
    model.addAttribute("total", total);
    return "dashboard";
}

// Хорошо:
@GetMapping("/dashboard")
public String dashboard(Model model) {
    model.addAttribute("stats", dashboardService.getStats());
    return "dashboard";
}
```

### 30.5. Непонимание роли DispatcherServlet

- `DispatcherServlet` — это один конкретный servlet, не магия.
- У него есть свой `WebApplicationContext` (дочерний от root).
- Можно зарегистрировать несколько `DispatcherServlet` с разными URL (редко нужно).
- `@EnableWebMvc` в Boot берет MVC-конфигурацию под ручное управление — часто ошибка, если вы хотели только добавить interceptor/converter.

### 30.6. Путаница между Filter, Interceptor и Controller Advice

|                      | Filter                         | Interceptor           | ControllerAdvice     |
| -------------------- | ------------------------------ | --------------------- | -------------------- |
| Уровень              | Servlet                        | Spring MVC            | Spring MVC           |
| Когда                | до/после всего MVC             | до/после controller   | только при exception |
| Знает Spring Context | если зарегистрирован Spring'ом | да                    | да                   |
| Для чего             | encoding, CORS, security       | logging, auth, locale | exception handling   |

---

## 31. Итоговая схема Spring Web MVC

### 31.1. Основные компоненты

```
┌─────────────────────────────────────────────────────────────┐
│                    Spring Web MVC                            │
│                                                              │
│  DispatcherServlet                                           │
│       │                                                      │
│       ├─ HandlerMapping ──────► HandlerExecutionChain        │
│       │       └─ RequestMappingHandlerMapping                │
│       │                                                      │
│       ├─ HandlerAdapter ──────► ArgumentResolvers            │
│       │       └─ RequestMappingHandlerAdapter   ReturnValueHandlers │
│       │                                                      │
│       ├─ HandlerInterceptors (pre/post/afterCompletion)      │
│       │                                                      │
│       ├─ HttpMessageConverter (JSON/XML ↔ Java)              │
│       │       └─ MappingJackson2HttpMessageConverter         │
│       │                                                      │
│       ├─ ViewResolver ─────────► View                        │
│       │       └─ ThymeleafViewResolver                       │
│       │                                                      │
│       └─ HandlerExceptionResolver                            │
│               └─ ExceptionHandlerExceptionResolver           │
└─────────────────────────────────────────────────────────────┘
```

### 31.2. Полный путь HTTP-запроса

```
1.  Клиент → HTTP-запрос
2.  Servlet Container принимает, создает HttpServletRequest/Response
3.  Filters (CharacterEncodingFilter, Spring Security, ...)
4.  DispatcherServlet.doDispatch()
5.    HandlerMapping → HandlerExecutionChain (handler + interceptors)
6.    HandlerInterceptor.preHandle() [все, по порядку]
7.    HandlerAdapter → подготовка аргументов (ArgumentResolvers)
8.      @PathVariable, @RequestParam, @RequestBody, @ModelAttribute, ...
9.      Валидация (`@Valid`, constraint-аннотации, method validation)
10.   Controller метод — вызов Java-метода
11.   Controller → Service → Repository → DB
12.   Возврат результата (DTO, ResponseEntity, String, ...)
13.   Обработка return value внутри HandlerAdapter:
        → если @ResponseBody/REST: HttpMessageConverter → JSON/XML
        → если ResponseEntity: status + headers + body
        → если view name: готовится ModelAndView
14.   HandlerInterceptor.postHandle() [после успешного handler return, в обратном порядке; ModelAndView может быть null]
15.   Если нужен view render: ViewResolver → View → HTML
16.   Если redirect: 3xx + Location header
17.   Формирование HTTP response (status, headers, body)
18.   HandlerInterceptor.afterCompletion() [для interceptor'ов, чей preHandle успешно прошел]
19.   Servlet Container → HTTP-ответ → Клиент

При exception на любом шаге:
      HandlerExceptionResolver → @ExceptionHandler / @ControllerAdvice
```

### 31.3. Варианты реализации контроллеров

```
Вариант 1: HTML-приложение (Thymeleaf)
  @Controller → Model + view name → ViewResolver → HTML

Вариант 2: REST JSON API
  @RestController → DTO → HttpMessageConverter → JSON

Вариант 3: Гибридный
  @Controller + @RestController в разных пакетах

Вариант 4: ResponseEntity (полный контроль)
  return ResponseEntity.status(201).header(...).body(dto)

Вариант 5: Асинхронный
  return Callable / DeferredResult / StreamingResponseBody
```

### 31.4. Когда использовать MVC-view, а когда REST JSON

| Критерий        | MVC (HTML)             | REST (JSON)                  |
| --------------- | ---------------------- | ---------------------------- |
| Фронтенд        | На сервере (Thymeleaf) | Отдельный (React/Vue/Mobile) |
| SEO             | Важен                  | Не важен                     |
| Интерактивность | Умеренная              | Высокая                      |
| API для других  | Не нужно               | Нужно                        |
| Архитектура     | Монолит                | Микросервисы / SPA           |
| Кто рендерит    | Сервер                 | Браузер                      |

В большинстве современных проектов: **REST JSON API** + отдельный фронтенд.
Для admin-панелей и simple CRUD: **MVC с Thymeleaf** удобнее и проще.

---

## 32. Тестирование Spring MVC

### 32.1. Что тестировать

Spring MVC обычно тестируют на нескольких уровнях:

| Уровень          | Инструмент                                        | Что проверяет                                         |
| ---------------- | ------------------------------------------------- | ----------------------------------------------------- |
| Controller slice | `@WebMvcTest` + `MockMvc`                         | mapping, binding, validation, status codes, JSON/view |
| Full context     | `@SpringBootTest` + `@AutoConfigureMockMvc`       | MVC вместе с реальной конфигурацией приложения        |
| HTTP black-box   | `TestRestTemplate`, `WebTestClient`, Rest Assured | приложение через настоящий HTTP server                |

Для большинства контроллеров лучший старт — `@WebMvcTest`: быстро, изолированно, без БД и полного application context.

Обычно для тестов подключают общий test starter:

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-test</artifactId>
    <scope>test</scope>
</dependency>
```

### 32.2. @WebMvcTest + MockMvc

```java
@WebMvcTest(UserApiController.class)
class UserApiControllerTest {

    @Autowired
    MockMvc mockMvc;

    @MockitoBean
    UserService userService;

    @Test
    void returnsUser() throws Exception {
        given(userService.findById(1L))
            .willReturn(new UserDto(1L, "Alice"));

        mockMvc.perform(get("/api/v1/users/{id}", 1L)
                .accept(MediaType.APPLICATION_JSON))
            .andExpect(status().isOk())
            .andExpect(content().contentTypeCompatibleWith(MediaType.APPLICATION_JSON))
            .andExpect(jsonPath("$.id").value(1))
            .andExpect(jsonPath("$.name").value("Alice"));
    }
}
```

### 32.3. Проверка validation errors

```java
@Test
void rejectsInvalidBody() throws Exception {
    mockMvc.perform(post("/api/v1/users")
            .contentType(MediaType.APPLICATION_JSON)
            .content("""
                {"name":"","email":"bad-email"}
                """))
        .andExpect(status().isBadRequest());
}
```

### 32.4. Аннотации тестов

| Аннотация               | Где ставится      | Что делает                                                                                                                                   |
| ----------------------- | ----------------- | -------------------------------------------------------------------------------------------------------------------------------------------- |
| `@WebMvcTest`           | test class        | Поднимает MVC slice: controllers, MVC infrastructure, Jackson, validation, controller advice. Не поднимает весь сервисный/репозиторный слой. |
| `@SpringBootTest`       | test class        | Поднимает полный application context. Медленнее, но ближе к реальному приложению.                                                            |
| `@AutoConfigureMockMvc` | test class        | Добавляет `MockMvc` к полному Boot context.                                                                                                  |
| `@MockitoBean`          | test class/field  | Регистрирует Mockito mock в Spring context. Современная замена старому Boot `@MockBean`.                                                     |
| `@Autowired`            | field/constructor | Внедряет `MockMvc`, `ObjectMapper` и другие beans в тест.                                                                                    |

### 32.5. Частые проверки MockMvc

```java
mockMvc.perform(get("/api/users")
        .param("page", "0")
        .header("X-Request-Id", "test")
        .accept(MediaType.APPLICATION_JSON))
    .andExpect(status().isOk())
    .andExpect(header().exists("X-Request-Id"))
    .andExpect(jsonPath("$.items").isArray());
```

Для HTML-контроллеров проверяют view и model:

```java
mockMvc.perform(get("/users"))
    .andExpect(status().isOk())
    .andExpect(view().name("users/list"))
    .andExpect(model().attributeExists("users"));
```

---

## 33. CORS в Spring MVC

### 33.1. Что такое CORS

CORS (Cross-Origin Resource Sharing) — браузерный механизм, который решает, может ли frontend с одного origin обращаться к API на другом origin.

Origin = scheme + host + port:

```
https://app.example.com
http://localhost:3000
```

CORS не заменяет authentication/authorization. Это только политика браузера.

### 33.2. @CrossOrigin

Для одного контроллера или метода:

```java
@RestController
@RequestMapping("/api/users")
@CrossOrigin(origins = "https://app.example.com")
public class UserApiController {

    @GetMapping
    public List<UserDto> getUsers() { ... }
}
```

На методе:

```java
@GetMapping("/{id}")
@CrossOrigin(origins = "http://localhost:3000", methods = RequestMethod.GET)
public UserDto getById(@PathVariable Long id) { ... }
```

### 33.3. Глобальная настройка CORS

```java
@Configuration
public class WebConfig implements WebMvcConfigurer {
    @Override
    public void addCorsMappings(CorsRegistry registry) {
        registry.addMapping("/api/**")
                .allowedOrigins("https://app.example.com")
                .allowedMethods("GET", "POST", "PUT", "PATCH", "DELETE")
                .allowedHeaders("Authorization", "Content-Type")
                .allowCredentials(true)
                .maxAge(3600);
    }
}
```

Если включен Spring Security, CORS часто нужно включить и в security-конфигурации, потому что preflight `OPTIONS` проходит через security filter chain.

### 33.4. Аннотации и настройки CORS

| Аннотация / настройка              | Где                    | Что делает                                                                                                         |
| ---------------------------------- | ---------------------- | ------------------------------------------------------------------------------------------------------------------ |
| `@CrossOrigin`                     | класс/метод controller | Локально включает CORS для выбранных endpoints.                                                                    |
| `WebMvcConfigurer#addCorsMappings` | config class           | Глобальные CORS rules для MVC.                                                                                     |
| `allowedOrigins`                   | CORS config            | Явный список origins.                                                                                              |
| `allowedOriginPatterns`            | CORS config            | Pattern-based origins, полезно для subdomains.                                                                     |
| `allowedMethods`                   | CORS config            | Какие HTTP methods разрешены.                                                                                      |
| `allowedHeaders`                   | CORS config            | Какие request headers разрешены.                                                                                   |
| `exposedHeaders`                   | CORS config            | Какие response headers браузер может читать из JS.                                                                 |
| `allowCredentials`                 | CORS config            | Разрешает cookies/authorization credentials. С credentials нельзя использовать wildcard `*` как конкретный origin. |
| `maxAge`                           | CORS config            | Сколько браузер может кешировать preflight response.                                                               |

### 33.5. Preflight request

Для “непростых” запросов браузер сначала отправляет:

```http
OPTIONS /api/users
Origin: https://app.example.com
Access-Control-Request-Method: POST
Access-Control-Request-Headers: Content-Type, Authorization
```

Сервер должен ответить CORS-заголовками. Только после этого браузер отправит настоящий `POST`.

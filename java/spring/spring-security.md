# Spring Security

- [WEB Авторизации](../../web/auth.md)




----------------------------------------------------------------------------------------------------

## Аутентификация

Аутентификация почти всегда проходит по одной и той же схеме:

```text
1. Filter извлекает credentials из запроса
2. Создает Authentication object (еще не authenticated)
3. Передает его в AuthenticationManager
4. AuthenticationManager -> AuthenticationProvider
5. Provider проверяет пользователя и credentials
6. Возвращается новый authenticated Authentication
7. Он кладется в SecurityContext
8. Запрос идет дальше как от уже аутентифицированного пользователя
```


```text
POST /login
  username=alice
  password=secret

UsernamePasswordAuthenticationFilter
  -> создает UsernamePasswordAuthenticationToken(unauthenticated)
  -> AuthenticationManager.authenticate(...)
      -> ProviderManager
          -> DaoAuthenticationProvider
              -> UserDetailsService.loadUserByUsername("alice")
              -> PasswordEncoder.matches(raw, encoded)
              -> вернуть authenticated token
  -> SecurityContextHolder.getContext().setAuthentication(authenticatedToken)
```


### Basic Auth

```text
Authorization: Basic base64(username:password)

BasicAuthenticationFilter
  -> извлекает header
  -> декодирует username/password
  -> создает Authentication
  -> AuthenticationManager
  -> ProviderManager / DaoAuthenticationProvider
  -> SecurityContext
```

### JWT Bearer Token

```text
Authorization: Bearer <jwt>

BearerTokenAuthenticationFilter
  -> достает token из header
  -> AuthenticationManager / AuthenticationProvider
  -> проверка подписи, exp, aud, iss, claims
  -> создается JwtAuthenticationToken
  -> SecurityContext
```

При JWT обычно нет server-side session, и аутентификация выполняется **на каждом запросе заново** по токену.

----------------------------------------------------------------------------------------------------

## Авторизация

После аутентификации Spring Security проверяет: имеет ли текущий пользователь право на этот URL / HTTP method / method call

В современном servlet stack это обычно делает `AuthorizationFilter`.

```text
AuthorizationFilter
  -> берет текущий Authentication из SecurityContextHolder
  -> передает его в AuthorizationManager
  -> AuthorizationManager решает grant / deny
  -> если grant -> запрос идет дальше
  -> если deny -> AccessDeniedException
```

------------------------------------------------------------------------------------------------------------------------






## Архитектура Spring Security

```text
HTTP request
    > Servlet Container
    > DelegatingFilterProxy
    > FilterChainProxy
    > SecurityFilterChain
    > Security Filters
        > SecurityContext (load)
        > HeaderWriterFilter
        > CorsFilter / CsrfFilter
        > LogoutFilter
        > Authentication filter
        > AuthenticationManager
        > AuthenticationProvider
        > UserDetailsService / external auth source
        > AnonymousAuthenticationFilter
        > ExceptionTranslationFilter
        > AuthorizationFilter
        > SecurityContext (save/clear)
    > DispatcherServlet
    > Controller
    > Method Security
    > Service
    > HTTP response
```

------------------------------------------------------------------------------------------------------------------------
`Servlet container`
1. **Servlet container** знает только про обычные servlet filters.
2. Spring подставляет туда `DelegatingFilterProxy`.
3. `DelegatingFilterProxy` передает работу Spring-bean'у.
4. Этим bean'ом обычно является `FilterChainProxy`.
5. `FilterChainProxy` выбирает подходящую `SecurityFilterChain`.
6. Уже внутри нее выполняются security filters.

Servlet container не знает про Spring Security напрямую
DelegatingFilterProxy - мост между servlet-миром и Spring context
FilterChainProxy - главный координатор security-фильтров

1. Загрузить SecurityContext
2. Защитить запрос от типовых угроз
3. Выполнить аутентификацию
4. Выполнить авторизацию
5. Передать запрос дальше в приложение
6. Сохранить/очистить SecurityContext
------------------------------------------------------------------------------------------------------------------------
`DelegatingFilterProxy` - Это bridge между servlet container и Spring ApplicationContext.
Это мост между Servlet Container и Spring Context.
Servlet container знает только обычный servlet filter, а DelegatingFilterProxy уже делегирует
выполнение Spring-bean’у, обычно springSecurityFilterChain.

Servlet container умеет регистрировать только `Filter`, но не знает, как искать Spring beans.
`DelegatingFilterProxy` решает эту проблему: он сам зарегистрирован как servlet filter, но делегирует
работу Spring-bean'у.

```text
Tomcat/Jetty
  -> DelegatingFilterProxy
      -> bean springSecurityFilterChain
          -> FilterChainProxy
```
------------------------------------------------------------------------------------------------------------------------
`FilterChainProxy` - Центральный filter Spring Security. Это **главный координационный объект** всей web security.
- принять запрос,
- найти подходящую `SecurityFilterChain`,
- выполнить filters в правильном порядке,
- очистить контекст после завершения запроса,
- применить `HttpFirewall`.

Это центральный security-filter Spring Security.
Он управляет всеми security filters и является стартовой точкой servlet security architecture.
Spring docs прямо говорят, что security filters обычно регистрируются именно через FilterChainProxy.
------------------------------------------------------------------------------------------------------------------------
`SecurityFilterChain` - правило вида: если request подходит под matcher -> применить нужный набор security filters
```java
@Bean
SecurityFilterChain apiChain(HttpSecurity http) { ... }      // /api/**
@Bean
SecurityFilterChain webChain(HttpSecurity http) { ... }      // все остальное
```
- `FilterChainProxy` выбирает **первую** подходящую цепочку,
- цепочки могут быть разными для API, admin, actuator, статических ресурсов,
- у каждой цепочки свой matcher и свой набор правил.

FilterChainProxy выбирает подходящую SecurityFilterChain для конкретного запроса по matcher’ам.
То есть у тебя может быть одна цепочка для /api/**, другая для /admin/**, третья для всего остального.
Это и есть архитектурная модель Spring Security web layer.
------------------------------------------------------------------------------------------------------------------------
`HttpSecurity` — DSL-билдер, через который настраивается `SecurityFilterChain`. Через него добавляется:
- правила авторизации,
- form login,
- basic auth,
- JWT resource server,
- logout,
- session management,
- CSRF,
- CORS,
- custom filters.

```text
HttpSecurity -> строит SecurityFilterChain
SecurityFilterChain -> используется FilterChainProxy
FilterChainProxy -> реально исполняет защиту на запросе
```
------------------------------------------------------------------------------------------------------------------------
После выбора `SecurityFilterChain` начинают работать `Security filters`.

1. Загрузка `SecurityContext`

Spring Security сначала работает с `SecurityContext` — контекстом безопасности текущего запроса.
По умолчанию `SecurityContextHolder` хранит его в ThreadLocal, а `FilterChainProxy` гарантирует
корректную очистку после обработки запроса.  ￼

1. `Authentication filter`

Один из authentication filters извлекает учетные данные из запроса.
Это может быть:
- username/password,
- bearer token,
- basic auth header,
- session/cookie,
- OAuth2 login response.

Конкретный фильтр зависит от configured authentication mechanism.

1. `AuthenticationManager`

Фильтр передает запрос на аутентификацию в `AuthenticationManager`.
Это главный orchestrator authentication flow.  ￼

1. `AuthenticationProvider`

Обычно `AuthenticationManager` реализован как `ProviderManager`, который делегирует проверку одному
или нескольким `AuthenticationProvider`. Spring docs прямо говорят, что default
implementation — `ProviderManager`, который опрашивает список `AuthenticationProvider`.  ￼

1. Проверка пользователя

AuthenticationProvider уже делает реальную проверку:
- через UserDetailsService,
- password encoder,
- JWT verification,
- LDAP,
- OAuth2/OpenID Connect provider,
- другой внешний источник identity.

Для username/password Spring обычно строит `AuthenticationManager` с `DaoAuthenticationProvider`.  ￼

1. Сохранение результата в `SecurityContextHolder`

Если аутентификация успешна, в `SecurityContextHolder` сохраняется `Authentication` текущего пользователя.
После этого текущий principal доступен дальше по цепочке и в том же потоке.

------------------------------------------------------------------------------------------------------------------------
После аутентификации идет авторизация, проверка: можно ли этому пользователю выполнять данный запрос.

1. `AuthorizationFilter`

Spring docs указывают, что AuthorizationFilter по умолчанию находится последним в security filter chain.
Это означает, что перед authorization уже успевают выполниться authentication filters и exploit protections.  ￼

Именно здесь проверяется:
- authenticated ли пользователь,
- есть ли у него нужная role/authority,
- разрешен ли доступ к URL.

1. Переход в Spring MVC

Если запрос прошел security filter chain:
- он идет дальше в DispatcherServlet,
- потом уже в HandlerMapping,
- HandlerAdapter,
- controller method.

То есть Spring Security стоит перед Spring MVC в servlet request pipeline.

------------------------------------------------------------------------------------------------------------------------

`Method Security` — второй уровень защиты

После web-level authorization может включаться Method Security.

Это уже не filter chain, а защита вызовов методов:
- `@PreAuthorize`
- `@PostAuthorize`
- `@Secured`
- `@RolesAllowed`

Spring Security умеет защищать не только URL, но и вызовы методов.

Подключается обычно так:

```java
@Configuration
@EnableMethodSecurity
public class MethodSecurityConfig {
}
```

После этого работают аннотации:

- `@PreAuthorize`
- `@PostAuthorize`
- `@PreFilter`
- `@PostFilter`
- `@Secured`
- `@RolesAllowed`

Пример:

```java
@PreAuthorize("hasRole('ADMIN') or #id == authentication.name")
public UserDto getUser(String id) { ... }
```

Под капотом это работает через Spring AOP / method interceptors:

```text
Method invocation
  -> Security interceptor
  -> AuthorizationManager / expression evaluation
  -> grant / deny
  -> вызов метода или exception
```

Это важно, потому что URL-защиты недостаточно: сервис может вызываться не только из одного контроллера.

------------------------------------------------------------------------------------------------------------------------








## Абстракции Spring Security

`Authentication` — одна из центральных сущностей Spring Security. Она бывает в двух состояниях:
1. **до успешной аутентификации** - содержит credentials, но еще не trusted
2. **после успешной аутентификации** - содержит principal, authorities и признак authenticated

```text
Authentication
  principal      -> кто пользователь
  credentials    -> пароль / токен / secret
  authorities    -> роли и права
  details        -> детали запроса
  authenticated  -> успешно ли подтверждена личность
```

**Примеры реализаций:**
- `UsernamePasswordAuthenticationToken`
- `authenticated UsernamePasswordAuthenticationToken`
- `BearerTokenAuthenticationToken`
- `JwtAuthenticationProvider`
- `JwtAuthenticationToken`
- `AnonymousAuthenticationToken`
- `RememberMeAuthenticationToken`
------------------------------------------------------------------------------------------------------------------------
`SecurityContext` - хранит текущий `Authentication`.
Внутри него лежит:
- текущий Authentication
- а внутри Authentication:
- principal
- credentials
- authorities
- details
- флаг authenticated

`SecurityContextHolder` — это holder / доступ к текущему security context, это статическая точка доступа:

```java
Authentication auth = SecurityContextHolder.getContext().getAuthentication();
```

```text
SecurityContextHolder
    > SecurityContext
        > Authentication
            > principal
            > authorities
            > credentials
```

В servlet-приложении контекст обычно живет **на время запроса** и привязан к текущему потоку выполнения.
------------------------------------------------------------------------------------------------------------------------
`AuthenticationManager` Это фасад. Он не обязан сам знать, как проверять все виды логина.
Обычно он делегирует работу в набор `AuthenticationProvider`.

```java
Authentication authenticate(Authentication authentication);
```

Современная абстракция авторизации.
Она отвечает на вопрос: можно ли текущему Authentication получить доступ к этому ресурсу/методу?
Используется:
- в web security через `AuthorizationFilter`,
- в method security через interceptor'ы и annotations.
------------------------------------------------------------------------------------------------------------------------
`ProviderManager` - Самая частая реализация `AuthenticationManager`.
Если provider умеет работать с данным типом `Authentication`, он пытается аутентифицировать.
Если получилось — возвращает authenticated object.
Если нет — управление идет дальше или выбрасывается исключение.

```text
ProviderManager
  -> Provider 1 supports this token? yes/no
  -> Provider 2 supports this token? yes/no
  -> Provider 3 supports this token? yes/no
```
------------------------------------------------------------------------------------------------------------------------
`AuthenticationProvider` - Это конкретная стратегия проверки credentials.
- `DaoAuthenticationProvider` — username/password через `UserDetailsService`
- JWT/bearer provider — проверка токена
- LDAP provider — проверка через LDAP
- pre-auth provider — когда пользователя уже аутентифицировал внешний proxy/SSO
------------------------------------------------------------------------------------------------------------------------
`DaoAuthenticationProvider` — самая типовая реализация для login/password. Его шаги:
1. получает username из входного token,
2. вызывает `UserDetailsService`,
3. получает `UserDetails`,
4. через `PasswordEncoder` сравнивает пароль,
5. проверяет флаги account locked/disabled/expired,
6. создает authenticated `Authentication`.
------------------------------------------------------------------------------------------------------------------------
`UserDetailsService` - Используется чаще всего в username/password сценариях. Задача:
- найти пользователя в БД,
- вернуть его логин, хеш пароля, роли, флаги blocked/expired/locked.

```java
UserDetails loadUserByUsername(String username)
```

Важно:
`UserDetailsService` **не проверяет пароль сам**. Он только загружает данные пользователя.
Проверка пароля делается в `AuthenticationProvider` через `PasswordEncoder`.
------------------------------------------------------------------------------------------------------------------------
`PasswordEncoder` - Отвечает за безопасную работу с паролями. Основные правила:
- пароли не хранятся в открытом виде,
- сравнение идет через хеш,
- современный дефолт — bcrypt/argon2/pbkdf2/scrypt,
- `NoOpPasswordEncoder` — только для демо и тестов.
------------------------------------------------------------------------------------------------------------------------
`AuthorizationManager` - Это стратегия принятия решения об авторизации. Она может учитывать:
- роли (`ROLE_ADMIN`)
- authorities (`user:read`, `order:write`)
- HTTP method
- URL path
- request matcher
- expression/rule
- объект вызова метода

```java
http.authorizeHttpRequests(auth -> auth
    .requestMatchers("/admin/**").hasRole("ADMIN")
    .requestMatchers(HttpMethod.GET, "/orders/**").hasAuthority("order:read")
    .anyRequest().authenticated()
);
```
----------------------------------------------------------------------------------------------------








## Модули Spring Security

| Модуль                 | Артефакт                                      | Что внутри                                                                                                                                |
| ---------------------- | --------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- |
| Core                   | `spring-security-core`                        | базовые интерфейсы: `Authentication`, `GrantedAuthority`, `SecurityContext`, `AuthenticationManager`, `UserDetails`, method security base |
| Web                    | `spring-security-web`                         | servlet filters, `FilterChainProxy`, session/security context web infrastructure, CSRF, headers, request cache                            |
| Config                 | `spring-security-config`                      | Java DSL (`HttpSecurity`), конфигурация `SecurityFilterChain`, `@EnableWebSecurity`, `@EnableMethodSecurity`                              |
| OAuth2 Core            | `spring-security-oauth2-core`                 | общие OAuth2 / OIDC модели и контракты                                                                                                    |
| OAuth2 Client          | `spring-security-oauth2-client`               | login через Google/GitHub/Keycloak, OAuth2 client flows                                                                                   |
| OAuth2 Resource Server | обычно через oauth2 modules + Boot autoconfig | JWT / opaque token validation для API                                                                                                     |
| OAuth2 JOSE            | `spring-security-oauth2-jose`                 | JWS/JWE/JWT support, Nimbus integration                                                                                                   |
| LDAP                   | `spring-security-ldap`                        | интеграция с LDAP/Active Directory                                                                                                        |
| ACL                    | `spring-security-acl`                         | объектно-уровневые ACL-права, используется редко                                                                                          |
| Test                   | `spring-security-test`                        | `@WithMockUser`, `SecurityMockMvcRequestPostProcessors`, helpers для тестов                                                               |

------------------------------------------------------------------------------------------------------------------------








## Обработка security-исключений

`ExceptionTranslationFilter`

Это не "фильтр аутентификации" и не "фильтр авторизации".
Это **bridge между Java exceptions и HTTP response**.

Он ловит:

- `AuthenticationException`
- `AccessDeniedException`

И переводит их в корректное HTTP-поведение.

Если пользователь не аутентифицирован:

Когда возникает `AuthenticationException`:

```text
ExceptionTranslationFilter
  -> очищает SecurityContext
  -> при необходимости сохраняет исходный запрос
  -> вызывает AuthenticationEntryPoint
```

`AuthenticationEntryPoint` решает, как запросить credentials:

- redirect на login page,
- вернуть `401 Unauthorized`,
- добавить `WWW-Authenticate`,
- отдать JSON-ошибку.

Если пользователь аутентифицирован, но доступа нет:

Когда возникает `AccessDeniedException`:

```text
ExceptionTranslationFilter
  -> вызывает AccessDeniedHandler
  -> обычно это 403 Forbidden
```


| Компонент                    | Что делает                                    |
| ---------------------------- | --------------------------------------------- |
| `ExceptionTranslationFilter` | переводит security exceptions в HTTP response |
| `AuthenticationEntryPoint`   | запускает процесс логина / возвращает 401     |
| `AccessDeniedHandler`        | обрабатывает 403                              |
| `RequestCache`               | может сохранить исходный запрос до логина     |

----------------------------------------------------------------------------------------------------








## `SecurityContext` в течение запроса

SecurityContext — это объект, который хранит информацию о текущей безопасности запроса/потока выполнения.


Жизненный цикл контекста

```text
Начало запроса
  -> создать/загрузить SecurityContext
  -> положить в SecurityContextHolder
  -> filters и приложение используют его
Конец запроса
  -> сохранить при необходимости
  -> очистить SecurityContextHolder
```

Почему это важно

Если контекст не очищать:

- один пользователь может "утечь" в другой запрос,
- особенно опасно при thread pools,
- возможны очень неприятные баги безопасности.

Поэтому `FilterChainProxy` и related filters не только ставят `Authentication`, но и **обязаны правильно очищать контекст**.

Где хранится context

Зависит от режима:

| Режим             | Где обычно хранится              |
| ----------------- | -------------------------------- |
| Stateful web app  | session + request/thread context |
| Stateless JWT API | только на время текущего запроса |
| Test              | часто вручную через test support |

----------------------------------------------------------------------------------------------------








## Stateful и Stateless архитектура

**Stateful.** Типичный сценарий:

- form login,
- после логина есть session,
- `SecurityContext` может восстанавливаться между запросами,
- удобно для server-side web приложений.

Схема:

```text
Login request
  -> authenticate
  -> сохранить security state в session

Следующий request
  -> достать context из session
  -> пользователь уже считается вошедшим
```

**Stateless.** Типичный сценарий:

- REST API,
- JWT/Bearer token,
- session не используется как источник auth-state,
- каждый запрос сам приносит токен.

Схема:

```text
Каждый request
  -> Bearer token
  -> token validation
  -> создать Authentication
  -> положить в SecurityContext только на время этого запроса
  -> после ответа очистить
```

| Характеристика                                      | Stateful        | Stateless                  |
| --------------------------------------------------- | --------------- | -------------------------- |
| Где хранится auth state                             | session         | в токене / снаружи сервера |
| Нужна ли повторная аутентификация на каждом запросе | нет             | да                         |
| Типичный UI                                         | server-side web | SPA / mobile / public API  |
| Масштабирование                                     | сложнее         | проще                      |

----------------------------------------------------------------------------------------------------








## Фильтры Spring Security

| Filter                                 | Роль                                                        |
| -------------------------------------- | ----------------------------------------------------------- |
| `SecurityContextHolderFilter`          | подготавливает/управляет `SecurityContextHolder` на запросе |
| `HeaderWriterFilter`                   | security headers                                            |
| `CorsFilter`                           | CORS                                                        |
| `CsrfFilter`                           | CSRF-защита                                                 |
| `LogoutFilter`                         | logout processing                                           |
| `UsernamePasswordAuthenticationFilter` | form login / username-password authentication               |
| `BasicAuthenticationFilter`            | HTTP Basic                                                  |
| `BearerTokenAuthenticationFilter`      | OAuth2 Resource Server / Bearer token                       |
| `AnonymousAuthenticationFilter`        | создает anonymous authentication, если пользователя нет     |
| `ExceptionTranslationFilter`           | переводит security exceptions в HTTP response               |
| `AuthorizationFilter`                  | проверка доступа                                            |

Важно: порядок filters имеет значение. Например:
- authentication должна произойти **до** authorization,
- exception handling должен стоять так, чтобы поймать нужные исключения,
- anonymous auth обычно ставится после реальной попытки аутентификации.

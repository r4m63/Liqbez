# Spring Security

- [WEB Авторизации](../../web/auth.md)




## Архитектура Spring Security



----------------------------------------------------------------------------------------------------


BASIC AUTH - Spring

Процесс предоставления доступа пользователю:
1) Идентификация - поиск пользователя в системе по (ид, логину, почте и тд - однозначно идентифицировать)
2) Аутентификация - подтверждение подлиности по (секретной информации, паролю, коду, OTP, одноразовый пароль, последние цифры номера телефона)
	1+2 пункты в современных приложениях происходит одновременно, но внтури приложения все происходит раздельно
3) Авторизация - Определение прав доступа, разграничение прав доступа для ресурсов.


Как Spring Security интегрируется в приложения - через систему ФИЛЬТРОВ - прежде чем попасть в сервлет
Пользователь отправляет запрос на получение доступа к ресурсу.




Абстракции в Spring Security:

	Authentication - Представляет текущего аутентифицированного пользователя.
		Содержит:
		Principal (основная информация о пользователе, например, имя).
		Credentials (пароль или токен).
		Authorities (роли и права).
	AuthenticationManager - Главный компонент, который обрабатывает аутентификацию.
		Проверяет данные пользователя с помощью AuthenticationProvider.
	AuthenticationProvider - Проверяет данные для аутентификации.
		Например, DaoAuthenticationProvider для проверки логина/пароля.
	SecurityContext - Хранит информацию о текущей аутентификации.
		Связан с текущей сессией.
	SecurityContextHolder - Статический хранилище, где SecurityContext доступен во время выполнения.

	FilterChainProxy - Центральный класс, управляющий цепочкой фильтров.
		Каждый запрос проходит через набор фильтров, например:
		UsernamePasswordAuthenticationFilter — обрабатывает логин/пароль.
		BasicAuthenticationFilter — проверяет Basic Auth.



Подробная логика работы Spring Security:

	Входящий запрос:
		Каждый запрос обрабатывается FilterChainProxy.
	Аутентификация:
		Если пользователь не аутентифицирован, запускается фильтр (например, BasicAuthenticationFilter или
		UsernamePasswordAuthenticationFilter).
		Данные передаются в AuthenticationManager.
	Авторизация:
		После успешной аутентификации проверяются права пользователя на запрашиваемый ресурс (через
		AccessDecisionManager).
	Ответ клиенту:
		Если запрос разрешен, он передается дальше в приложение.
		Если нет, возвращается ошибка (например, 403 Forbidden).



------------------------------------------------------------------------------------------------------------------------

Form-based Authentication
HTTP Basic Authentication
JWT
OAuth2



Интеграция Spring Security с PostgreSQL
    UserDetailsService


------------------------------------------------------------------------------------------------------------------------





## Архитектура Spring Security

Spring Security — это не "одна аннотация для логина", а целая инфраструктура безопасности, которая:

- перехватывает HTTP-запросы **до входа в контроллер**,
- определяет, кто пользователь,
- проверяет, можно ли ему выполнять действие,
- хранит текущую аутентификацию в контексте,
- обрабатывает ошибки безопасности,
- умеет защищать не только web-слой, но и **method/service layer**.


```text
Запрос
  -> Servlet Container
  -> DelegatingFilterProxy
  -> FilterChainProxy
  -> SecurityFilterChain
  -> Security Filters
  -> DispatcherServlet
  -> Controller
```

То есть Spring Security в servlet-приложении живет **прежде всего как цепочка фильтров**.

```text
Client
  -> Servlet Container
      -> DelegatingFilterProxy
          -> FilterChainProxy
              -> SecurityFilterChain (первая подошедшая)
                  -> SecurityContextHolderFilter
                  -> CSRF / CORS / headers filters
                  -> Authentication filters
                  -> AnonymousAuthenticationFilter
                  -> ExceptionTranslationFilter
                  -> AuthorizationFilter
                      -> DispatcherServlet
                          -> Controller
                              -> Service
```
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
`FilterChainProxy` - Центральный filter Spring Security. Это **главный координационный объект** всей web security.
- принять запрос,
- найти подходящую `SecurityFilterChain`,
- выполнить filters в правильном порядке,
- очистить контекст после завершения запроса,
- применить `HttpFirewall`.
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
`DelegatingFilterProxy` - Это bridge между servlet container и Spring ApplicationContext.

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

| Абстракция               | Что означает                                                            |
| ------------------------ | ----------------------------------------------------------------------- |
| `Authentication`         | текущая попытка аутентификации или уже аутентифицированный пользователь |
| `Principal`              | "кто это" — пользователь, client, subject                               |
| `Credentials`            | чем подтверждает себя пользователь: пароль, токен, code, secret         |
| `GrantedAuthority`       | права/роли пользователя                                                 |
| `SecurityContext`        | контейнер для текущего `Authentication`                                 |
| `SecurityContextHolder`  | глобальная точка доступа к текущему `SecurityContext`                   |
| `AuthenticationManager`  | фасад для аутентификации                                                |
| `AuthenticationProvider` | конкретная стратегия проверки credentials                               |
| `UserDetailsService`     | загрузка пользователя по username/login/email                           |
| `PasswordEncoder`        | сравнение/хеширование пароля                                            |
| `AuthorizationManager`   | проверка: можно ли выполнить действие                                   |

------------------------------------------------------------------------------------------------------------------------
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
- `AnonymousAuthenticationToken`
- `RememberMeAuthenticationToken`
- `JwtAuthenticationToken`
- `BearerTokenAuthenticationToken`
------------------------------------------------------------------------------------------------------------------------
`SecurityContext` - хранит текущий `Authentication`.

`SecurityContextHolder` — это статическая точка доступа:

```java
Authentication auth = SecurityContextHolder.getContext().getAuthentication();
```

```text
SecurityContextHolder
  -> SecurityContext
      -> Authentication
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

Основные модули Spring Security

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

## Как проходит HTTP-запрос под капотом


```text
1. Приходит HTTP request
2. Запрос попадает в DelegatingFilterProxy
3. DelegatingFilterProxy вызывает FilterChainProxy
4. FilterChainProxy выбирает SecurityFilterChain
5. SecurityContext загружается/инициализируется
6. Срабатывают security filters
7. Если нужно — выполняется аутентификация
8. Выполняется авторизация
9. Если доступ разрешен -> запрос идет в DispatcherServlet / controller
10. После завершения запроса контекст очищается/сохраняется
```


```text
Client
  -> Request
      -> DelegatingFilterProxy
          -> FilterChainProxy
              -> SecurityContextHolderFilter
              -> CorsFilter / CsrfFilter / HeaderWriterFilter
              -> LogoutFilter
              -> Authentication Filters
              -> AnonymousAuthenticationFilter
              -> ExceptionTranslationFilter
              -> AuthorizationFilter
              -> DispatcherServlet
                  -> Controller
                  -> Service
                  -> Response
```

----------------------------------------------------------------------------------------------------

## Как работает аутентификация под капотом

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

## Как работает авторизация под капотом

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

----------------------------------------------------------------------------------------------------
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

## Обработка security-исключений

----------------------------------------------------------------------------------------------------
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
----------------------------------------------------------------------------------------------------
`AuthenticationEntryPoint` решает, как запросить credentials:

- redirect на login page,
- вернуть `401 Unauthorized`,
- добавить `WWW-Authenticate`,
- отдать JSON-ошибку.

----------------------------------------------------------------------------------------------------
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

### 11.1. Жизненный цикл контекста

```text
Начало запроса
  -> создать/загрузить SecurityContext
  -> положить в SecurityContextHolder
  -> filters и приложение используют его
Конец запроса
  -> сохранить при необходимости
  -> очистить SecurityContextHolder
```

### Почему это важно

Если контекст не очищать:

- один пользователь может "утечь" в другой запрос,
- особенно опасно при thread pools,
- возможны очень неприятные баги безопасности.

Поэтому `FilterChainProxy` и related filters не только ставят `Authentication`, но и **обязаны правильно очищать контекст**.

### Где хранится context

Зависит от режима:

| Режим             | Где обычно хранится              |
| ----------------- | -------------------------------- |
| Stateful web app  | session + request/thread context |
| Stateless JWT API | только на время текущего запроса |
| Test              | часто вручную через test support |

----------------------------------------------------------------------------------------------------

## Stateful и Stateless архитектура

### Stateful

Типичный сценарий:

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

### Stateless

Типичный сценарий:

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

### Разница

| Характеристика                                      | Stateful        | Stateless                  |
| --------------------------------------------------- | --------------- | -------------------------- |
| Где хранится auth state                             | session         | в токене / снаружи сервера |
| Нужна ли повторная аутентификация на каждом запросе | нет             | да                         |
| Типичный UI                                         | server-side web | SPA / mobile / public API  |
| Масштабирование                                     | сложнее         | проще                      |

----------------------------------------------------------------------------------------------------

## 13. Method Security

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

----------------------------------------------------------------------------------------------------

## Часто встречающиеся filters

Это не полный список, а именно те filters, которые чаще всего встречаются в реальных приложениях.

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

Важно: порядок filters имеет значение.
Например:

- authentication должна произойти **до** authorization,
- exception handling должен стоять так, чтобы поймать нужные исключения,
- anonymous auth обычно ставится после реальной попытки аутентификации.

----------------------------------------------------------------------------------------------------

## Как Spring Security строит решение по шагам


```text
1. Приходит request
2. DelegatingFilterProxy передает его в FilterChainProxy
3. FilterChainProxy выбирает первую подходящую SecurityFilterChain
4. SecurityContext подготавливается
5. Применяются security filters защиты (headers, CORS, CSRF, logout и т.д.)
6. Authentication filter пытается извлечь credentials
7. AuthenticationManager делегирует в AuthenticationProvider
8. При успехе authenticated Authentication кладется в SecurityContext
9. AuthorizationFilter вызывает AuthorizationManager
10. Если доступ есть -> request идет в controller/service
11. Если нет -> exception
12. ExceptionTranslationFilter превращает exception в 401/403/redirect
13. После завершения запроса контекст очищается/сохраняется
```


Spring Security — это **pipeline безопасности**, где:

- filters отвечают за перехват запроса,
- `AuthenticationManager` + `AuthenticationProvider` отвечают за проверку личности,
- `SecurityContextHolder` хранит текущего пользователя,
- `AuthorizationManager` отвечает за доступ,
- `ExceptionTranslationFilter` превращает security-ошибки в HTTP-ответ.

----------------------------------------------------------------------------------------------------

## Самые важные классы, которые нужно знать наизусть

| Класс / интерфейс            | Зачем нужен                                    |
| ---------------------------- | ---------------------------------------------- |
| `SecurityFilterChain`        | конфигурация набора security filters           |
| `HttpSecurity`               | DSL для построения security chain              |
| `FilterChainProxy`           | главный координатор фильтров                   |
| `Authentication`             | текущий пользователь / попытка логина          |
| `SecurityContext`            | контейнер для `Authentication`                 |
| `SecurityContextHolder`      | доступ к текущему контексту                    |
| `AuthenticationManager`      | фасад аутентификации                           |
| `ProviderManager`            | стандартная реализация `AuthenticationManager` |
| `AuthenticationProvider`     | конкретная стратегия проверки                  |
| `UserDetailsService`         | загрузка пользователя                          |
| `PasswordEncoder`            | проверка пароля                                |
| `AuthorizationManager`       | принятие решения о доступе                     |
| `ExceptionTranslationFilter` | превращает exceptions в 401/403/redirect       |
| `AuthenticationEntryPoint`   | запускает login / возвращает 401               |
| `AccessDeniedHandler`        | обрабатывает 403                               |

----------------------------------------------------------------------------------------------------

### Короткая мнемосхема

```text
Request
  -> FilterChainProxy
  -> Authentication
  -> SecurityContext
  -> Authorization
  -> Controller
```

```text
Кто ты?
  -> Authentication

Где это хранится?
  -> SecurityContext

Можно ли тебе сюда?
  -> AuthorizationManager

Кто все это запускает?
  -> Security filters / FilterChainProxy
```

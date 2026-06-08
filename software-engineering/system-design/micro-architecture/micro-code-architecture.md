# Micro Architecture

Как устроен код внутри одного deployable unit.

- Layered Architecture
- Clean Architecture
- Onion Architecture
- Hexagonal Architecture
- Vertical Slice Architecture
- Feature-Based Architecture
- Modular Monolith
- Component-Based Architecture
- Screaming Architecture

Как организован UI/presentation слой.

- MVC
- MVP
- MVVM
- MVU / Elm Architecture
- MVI
- Flux
- Redux
- VIPER
- PAC

Способы организации бизнес-логики

- Transaction Script
- Domain Model
- Table Module
- Service Layer
- Anemic Domain Model
- Rich Domain Model
- DDD Tactical Patterns


> Важное уточнение: Clean, Onion и Hexagonal — близкие
> это не три радикально разные штуки.
> Они все из одного семейства:
> business/application core внутри
> infrastructure/framework/database снаружи
> dependencies point inward
> Разница лишь в акценте
> Это разные формулировки одного семейства идей: business/application core должен быть отделён от UI, БД, framework и infrastructure.

## Layered Architecture

Главная идея: Разделить код по слоям ответственности.

То есть layered architecture говорит: Положи код в правильный слой.
Но она не всегда жёстко говорит: Application core не должен зависеть от инфраструктуры.

Классическая схема:

```text
Controller -> Service -> Repository -> Database

Controller  отвечает за HTTP/API
            читает HTTP request
            парсит JSON/form
            валидирует формат
            вызывает service
            мапит ошибку в HTTP response
            возвращает JSON/HTML
Service     отвечает за бизнес-логику
            бизнес-сценарии
            координацию операций
            проверки правил
            вызов repository
            вызов внешних сервисов
Repository  отвечает за работу с БД
            читает из БД
            пишет в БД
            мапит row/entity в объект приложения
```

- легко стартовать
- мало абстракций
- подходит для CRUD
- понятна большинству backend-разработчиков
- хорошо ложится на web framework

Проблема в том, что со временем Service часто превращается в огромный мешок и внутри оказывается
всё: бизнес-логика, SQL, Redis, JWT, HTTP ошибки, валидация, аудит, работа с cookies, работа с Kafka

Формально всё ещё есть слои: Controller -> Service -> Repository, но фактически Service знает слишком много.

## Hexagonal Architecture

Главная идея: Бизнес/application core не знает про HTTP, Postgres, Redis, Kafka, HTML,
JWT-библиотеки и прочие детали. Core знает только интерфейсы

Гексагональная архитектура, или Ports & Adapters, говорит:
В центре находится бизнес/application core. Всё внешнее подключается через адаптеры.

```text
              HTTP Handler
                   |
                   v
              Use Case
                   |
        ┌──────────┴──────────┐
        v                     v
  UserRepository          TokenSigner
        ^                     ^
        |                     |
Postgres Adapter        JWT Adapter


Inbound Adapters -> Application Core -> Output Ports <- Output Adapters
```


Главная идея hexagonal architecture: Не просто controller -> service -> repository, а
внешний мир не должен протекать в бизнес-логику.

То есть core не должен знать про:
- HTTP
- Go templates
- PostgreSQL
- Redis
- Kafka
- JWT library
- pgx
- net/http
- html/template
- конкретный framework

Core должен знать только про свои интерфейсы:
- UserRepository
- SessionStore
- TokenSigner
- PasswordHasher
- AuditRecorder
- Clock
- RandomGenerator


**Ключевые термины:**

Core - domain, application use cases, business rules

Port — это интерфейс, который описывает потребность core. Порт описывает не технологию, а потребность use case.

Adapter — это реализация порта через конкретную технологию.
Inbound adapter - то, что вызывает use case
Outbound adapter - то, что вызывается из use case через порт

## Clean Architecture

Clean Architecture — это более формализованная версия той же идеи: зависимости всегда направлены внутрь.

Главное правило: Внутренние слои не знают про внешние.

```text
Entities
    ↑
Use Cases
    ↑
Interface Adapters
    ↑
Frameworks & Drivers


Или снаружи внутрь:


Frameworks / DB / Web
        ↓
Interface Adapters
        ↓
Use Cases
        ↓
Entities

```



--------------


ПРОМПТ:


Объясни мне как software engineer разницу между Layered Architecture, Hexagonal Architecture / Ports and Adapters, Clean Architecture, Onion Architecture и DDD.

Мне нужен не поверхностный ответ, а инженерное сравнение:

1. Что означает каждый подход.
2. Кто его автор или основной популяризатор.
3. Где он впервые или канонически описан: книга, статья, блог, год.
4. Какие проблемы он решает.
5. Чем он отличается от других подходов.
6. Где подходы пересекаются и почему их часто путают.
7. В чём практическая разница в коде.
8. Как это выглядит на примере backend-проекта.
9. Какие есть плюсы, минусы и риски overengineering.
10. Что лучше использовать для SSO/backend-системы с OIDC, admin panel, login API, PostgreSQL, Redis и Go.

Важно:
- Не выдавай Clean, Onion и Hexagonal как полностью независимые “магические” архитектуры, если они реально близки.
- Укажи, что Layered Architecture, Clean Architecture, Onion Architecture и Hexagonal Architecture частично пересекаются.
- Отдельно объясни, что DDD — это не просто структура папок, а подход к моделированию бизнес-домена.
- Приведи конкретные примеры: controller/service/repository, use case, port, adapter, repository interface, Postgres adapter, Redis adapter.
- Покажи пример структуры проекта на Go.
- Объясни, когда достаточно обычной layered architecture, а когда стоит использовать hexagonal/clean/onion.
- Дай рекомендации без фанатизма и без лишней церемонии.

Обязательно приведи источники:
- Alistair Cockburn — Hexagonal Architecture / Ports and Adapters
- Martin Fowler — Patterns of Enterprise Application Architecture и Presentation-Domain-Data Layering
- Robert C. Martin — Clean Architecture
- Jeffrey Palermo — Onion Architecture
- Eric Evans — Domain-Driven Design

Формат ответа:
- Сначала короткая карта различий.
- Потом подробное объяснение каждого подхода.
- Потом сравнительная таблица.
- Потом практический пример на backend/SSO.
- Потом список источников для изучения.
- Пиши на русском языке.

Дополнительно: критикуй каждый подход. Объясни, где авторы и практики спорят между собой. Не делай вид, что это строгие научные стандарты. Отделяй канонические определения от распространённых интерпретаций в индустрии.

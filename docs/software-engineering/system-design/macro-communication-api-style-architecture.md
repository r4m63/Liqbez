# Communication & API Style (Macro-Architecture): Архитектура клиент-серверного взаимодействия веб приложений

## REST API

**REST API** - это архитектурный стиль клиент-серверного взаимодействия, который определяет, по каким правилам
приложения должны обмениваться данными между собой.

- Stateless. Сервер не хранит состояние клиента. Каждый запрос содержит всю необходимую информацию.
- Все данные представлены в виде ресурсов endpoint (/users, /products).
- Использование стандартных HTTP-методов (GET, POST, PUT, DELETE, etc).
- Ответы сервера могут быть кэшированы для повышения производительности.
- Слоистая система. Клиент не знает, взаимодействует ли он напрямую с сервером или через прокси.
- Формат данных json (xml устаревшее)

### Структура REST API endpoints ресурсов

Основные правила:

1. Использовать существительные (не глаголы)

   Плохо: `/getUsers`, `/createOrder`, `/deleteProduct`

   Хорошо: GET `/users`, POST `/orders`, DELETE `/products/{id}`

2. Называть ресурсы во множественном числе (Единообразие и явное указание на коллекцию)

   GET `/users` (все пользователи)

   GET `/users/1` (конкретный пользователь)

3. Использовать нижний регистр и дефисы (kebab-case)

   Плохо: `/UserOrders`, `/productCategories`

   Хорошо: `/user-orders`, `/product-categories`

4. Избегать пробелов и спецсимволов

   Вместо пробелов использовать `-` или `_`:
   GET `/product-categories` (лучше, чем `/product%20categories`)

Иерархия ресурсов:

1. Вложенность для связанных сущностей. Если ресурс принадлежит другому ресурсу, использовать иерархию:

   `/users/{userId}/orders` - Заказы пользователя

   `/users/{userId}/orders/{orderId}` - Конкретный заказ

2. Глубина вложенности ≤ 2–3 уровней

   Плохо: `/users/1/orders/5/products/3/reviews`

   Лучше: `/reviews?productId=3&userId=1` (с фильтрацией)

3. Для фильтрации, сортировки, пагинации - использовать query-параметры
   GET `/users?role=admin&status=active`
   GET `/products?sort=-price,created_at` - `-` = DESC
   GET `/articles?page=2&limit=10`

4. Версионирование API

   `/api/v1/users`

   `/api/v2/users`

### Документирование

Swagger - документация

### Тестирование

## GraphQL

GraphQL — это спецификация языка запросов и система выполнения, которая может работать поверх разных транспортных
протоколов (HTTP, WebSocket)

- GraphQL сам по себе stateless (как и REST), но может использоваться в stateful-сценариях:
    - Stateless-часть (HTTP): Обычные запросы (Queries) и мутации (Mutations) работают через HTTP (как REST).
    - Stateful-часть (WebSocket): Подписки (Subscriptions) требуют постоянного соединения (WebSocket), которое
      поддерживает состояние.
- Один URL (/graphql)
- Клиент каждый раз определяет что получать (получает только запрошенные поля)

### Работа GraphQL под капотом

Клиент:

- Отправляет GraphQL-запрос (Query/Mutation) через HTTP POST:

```
POST /graphql HTTP/1.1
Content-Type: application/json

{
  "query": "query { user(id: 1) { name email } }"
}
```

Сервер:

- Парсит запрос.
- Валидирует его против схемы.
- Выполняет через резолверы (функции для получения данных).
- Возвращает JSON-ответ:

```
{ "data": { "user": { "name": "Alice", "email": "example@example.com" } } }
```

Подписки (WebSocket):

- Для Subscriptions используется WebSocket (протокол graphql-ws)
- Сервер поддерживает состояние соединения и сам отправляет данные при событиях.

```
// Клиент подключается к WebSocket и отправляет:
{
  "type": "subscribe",
  "query": "subscription { newMessage { text } }"
}
```

**Как GraphQL делает выборку данных?**

Механизм резолверов (Resolvers): Каждое поле в GraphQL-запросе обрабатывается резолвером — функцией, которая знает, где
взять данные.

```
# Схема
type Query {
  user(id: ID!): User
}

type User {
  id: ID!
  name: String!
  email: String!
  posts: [Post!]!
}

// Резолверы
const resolvers = {
Query: {
user: (parent, args, context) => {
return db.users.find(user => user.id === args.id); // Данные из БД
}
},
User: {
posts: (user) => {
return db.posts.filter(post => post.authorId === user.id); // Связанные данные
}
}
        };
```

**Процесс выполнения запроса:**

- Парсинг: Сервер разбирает GraphQL-запрос в AST (Abstract Syntax Tree).
- Валидация: Проверяет, что запрос соответствует схеме.
- Исполнение:
    - Для каждого поля запускается соответствующий резолвер.
    - Резолверы могут получать данные из:
    - БД (SQL, MongoDB).
    - Других API (REST, gRPC).
    - Кэша (Redis).
- Сборка ответа: Данные объединяются в JSON-структуру, запрошенную клиентом.

### Принцип работы запросов

**Гибкие запросы данных (Queries)**

- Клиент точно указывает, какие данные ему нужны.
- Можно запрашивать связанные данные за один запрос (например, пользователя и его посты).
- Избегает проблем over-fetching (получение лишних данных) и under-fetching (недостаточность данных).
    ```
    query {
      user(id: "1") {
        name
        email
        posts {
          title
          comments {
            text
          }
        }
      }
    }
    ```

**Изменение данных (Mutations)**

- Аналог POST/PUT/PATCH/DELETE в REST.
- Позволяет изменять данные на сервере и сразу получать обновленные поля в ответе.
  ```
  mutation {
    createPost(title: "New Post", content: "Hello!") {
      id
      title
    }
  }
  ```

**Реал-тайм обновления (Subscriptions)**

- Сервер может отправлять данные клиенту без запроса (например, уведомления, чаты).
- Работает через WebSocket (graphql-ws или subscriptions-transport-ws).
  ```
  subscription {
    newMessage(roomId: "123") {
      id
      text
      sender {
        name
      }
    }
  }
  ```

**Интроспекция API**

- Клиент может автоматически получать схему API (типы, поля, документацию).
- Используется для генерации документации (например, GraphiQL).
  ```
  query {
    __schema {
      types {
        name
        fields {
          name
          type {
            name
          }
        }
      }
    }
  }
  ```

### Инициация запроса с сервера

Сервер может инициировать запрос к клиенту, но только через подписки (Subscriptions).

- Клиент подключается к серверу через WebSocket.
- Сервер пассивно ожидает событий (например, новое сообщение в чате).
- Когда событие происходит, сервер автоматически отправляет данные всем подписанным клиентам.

### Ограничения GraphQL

- Нет встроенного кэширования: В отличие от REST (где кэшируются URL), GraphQL требует ручной настройки кэша (например,
  через Apollo Client).
- Сложность запросов: Клиент может отправить слишком сложный запрос (например, с глубокой вложенностью), что нагрузит
  сервер.
- Проблема N+1: Если запрашиваются связанные данные, сервер может сделать много запросов к БД.

### Инструменты для работы с GraphQL

Серверные

- Apollo Server (Node.js)
- GraphQL Yoga (упрощенный сервер)
- Hasura (GraphQL поверх PostgreSQL)

Клиентские

- Apollo Client (React/Vue)
- Relay (оптимизирован для Facebook)
- URQL (легковесная альтернатива)

## RPC (Remote Procedure Call)

## gRPC

## SOAP (Simple Object Access Protocol)

SOAP - это протокол обмена структурированными сообщениями. Формат данных - Soap-XML.
Может использоваться с любым протоколом прикладного уровня (SMTP, FTP, HTTP).
Для описания SOAP сервисов используется WSDL (Web Services Description Language) - язык описания веб-сервисов и доступа
к ним, основанный на XML.

STATEFUL

```
<binding type="bookPortType" name="bookBind">
  <soap:binding style="document" transport="http://schemas.xmlsoap.org/soap/http"/>
  <operation name="getBook">
    <soap:operation soapAction="getBook"/> 
      <input>
        <soap:body use="literal"/>
      </input>
      <output>
        <soap:body use="iteral"/>
      </output>
  </operation>
</binding>
<service name="Hello Service">
  <port binding="bookBind" name="bookPort">
    <soap:address location="http://localhost/bookservice"/>
  </port>
</service>
```

## tRPC

## WebSocket

## SSE (Server-Sent Events)

## Webhook

## MQTT

## AMQP

## CORBA
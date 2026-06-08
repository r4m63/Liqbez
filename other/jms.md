# JMS, брокеры сообщений, BPMS и планировщики задач

Cheatsheet для экзамена по дисциплине **"Java: бизнес-логика программных систем"**.

Фокус: что это, зачем нужно, как устроено, где применять, какие плюсы и минусы, что сказать на экзамене.

---

## Карта вопросов

Исходные вопросы сгруппированы так:

| Группа | Какие вопросы сюда входят |
|---|---|
| **JMS / Jakarta Messaging** | распределенная обработка, ресурсы и сообщения, архитектура, методы отправки, очереди, модели доставки сообщений |
| **RabbitMQ** | что это, архитектура, плюсы/минусы, где используется |
| **Kafka** | все про Kafka: архитектура, особенности, преимущества/недостатки, области применения |
| **ActiveMQ / Apache MQ** | ActiveMQ, Apache MQ, архитектура, плюсы/минусы, особенности, где используется |
| **Протоколы очередей** | AMQP, MQTT, STOMP, Kafka protocol, OpenWire, JMS как API |
| **Асинхронная обработка** | когда хорошо и плохо использовать асинхронные сообщения и фоновые задачи |
| **Планировщики задач** | cron, Spring scheduling, Jakarta EE timers, Quartz, системы периодических задач |
| **BPMS** | что такое BPMS, компоненты, жизненный цикл бизнес-процесса, плюсы/минусы |
| **Camunda** | Camunda BPMN, Cockpit, транзакции, состав ПО, стадии разработки с BPMS |

Важно: формулировка **"Java mail service"** в этом списке, скорее всего, означает **Java Message Service / JMS**, а не JavaMail.

---

# 1. JMS / Jakarta Messaging

## Что такое JMS

**JMS** исторически означает **Java Message Service**. В современных Jakarta EE спецификациях это называется **Jakarta Messaging**.

JMS - это **стандартный Java API для работы с системами обмена сообщениями**.

Главная мысль:

```text
JMS - это не брокер и не сетевой протокол.
JMS - это Java API, через который приложение работает с брокером сообщений.
```

Примеры JMS-провайдеров:

- Apache ActiveMQ Classic;
- Apache ActiveMQ Artemis;
- IBM MQ;
- OpenMQ;
- WebLogic JMS;
- WildFly / JBoss messaging.

JMS нужен, чтобы Java-приложение могло:

- отправлять сообщения;
- получать сообщения;
- работать с очередями и топиками;
- использовать транзакции;
- делать асинхронную обработку;
- развязывать producer и consumer по времени.

## Зачем нужен JMS

Без JMS сервисы часто вызывают друг друга синхронно:

```text
OrderService -> PaymentService
OrderService ждет ответ
```

С JMS можно сделать иначе:

```text
OrderService -> Queue: payment.requests
PaymentWorker <- Queue: payment.requests
```

Плюс:

- отправитель не обязан ждать моментальной обработки;
- получатель может быть временно недоступен;
- нагрузку можно буферизовать;
- обработчики можно масштабировать;
- бизнес-операции можно выполнять асинхронно.

## Архитектура JMS

Типовая архитектура:

```text
Producer
  -> JMS API
  -> JMS Provider / Broker
  -> Destination
  -> Consumer
```

Компоненты:

| Компонент | Что делает |
|---|---|
| **JMS Client** | Java-приложение, которое отправляет или получает сообщения |
| **JMS Provider** | реализация JMS: ActiveMQ, Artemis, IBM MQ |
| **Broker** | сервер сообщений, хранит и доставляет сообщения |
| **Destination** | адрес доставки: `Queue` или `Topic` |
| **Producer** | отправляет сообщения |
| **Consumer** | получает сообщения |
| **Message** | объект с данными, headers и properties |

## Основные ресурсы JMS

### `ConnectionFactory`

**ConnectionFactory** - фабрика подключений к JMS-провайдеру.

Через нее создается `Connection` или `JMSContext`.

В Jakarta EE обычно берется через JNDI или injection:

```java
@Resource(lookup = "jms/MyConnectionFactory")
private ConnectionFactory connectionFactory;
```

В Spring обычно настраивается как bean.

### `Destination`

**Destination** - место, куда отправляется сообщение.

Есть два основных типа:

| Тип | Модель |
|---|---|
| `Queue` | point-to-point |
| `Topic` | publish/subscribe |

### `Queue`

**Queue** - очередь.

Модель:

```text
Producer -> Queue -> один Consumer из группы
```

Свойства:

- сообщение обычно обрабатывается одним consumer'ом;
- если consumer недоступен, сообщение может ждать в очереди;
- подходит для фоновых задач и команд;
- хорошо масштабируется через несколько consumer'ов.

Пример:

```text
payment.requests
email.to-send
report.generation
```

### `Topic`

**Topic** - топик для рассылки события нескольким подписчикам.

Модель:

```text
Producer -> Topic -> Consumer A
                  -> Consumer B
                  -> Consumer C
```

Свойства:

- одно сообщение может получить несколько подписчиков;
- подходит для событий;
- подписчики логически независимы;
- durable subscription позволяет получать сообщения даже после временного отключения подписчика.

Пример:

```text
order.created
user.registered
payment.completed
```

## `Connection`, `Session`, `Producer`, `Consumer`

| Объект | Роль |
|---|---|
| `Connection` | физическое или логическое соединение с провайдером |
| `Session` | контекст работы с сообщениями, транзакциями и acknowledge |
| `MessageProducer` | отправитель |
| `MessageConsumer` | получатель |
| `JMSContext` | упрощенный API JMS 2.0+, объединяет connection/session |

Классическая схема:

```text
ConnectionFactory
  -> Connection
  -> Session
  -> MessageProducer / MessageConsumer
```

Современная схема:

```text
ConnectionFactory
  -> JMSContext
  -> Producer / Consumer
```

## Сообщение JMS

JMS-сообщение состоит из:

```text
Headers
Properties
Body
```

### Headers

Headers задаются спецификацией JMS.

Частые поля:

| Header | Значение |
|---|---|
| `JMSMessageID` | уникальный id сообщения |
| `JMSDestination` | куда отправлено сообщение |
| `JMSCorrelationID` | связь запроса и ответа |
| `JMSReplyTo` | куда отправить ответ |
| `JMSDeliveryMode` | persistent или non-persistent |
| `JMSPriority` | приоритет |
| `JMSExpiration` | срок жизни сообщения |
| `JMSTimestamp` | время отправки |
| `JMSType` | логический тип сообщения |

### Properties

Properties - пользовательские метаданные.

Например:

```text
tenantId = "acme"
traceId = "abc-123"
eventType = "OrderCreated"
```

Они полезны для:

- фильтрации сообщений;
- маршрутизации;
- трассировки;
- передачи технического контекста.

### Body

Body - полезная нагрузка.

Типы JMS-сообщений:

| Тип | Для чего |
|---|---|
| `TextMessage` | строка, часто JSON/XML |
| `ObjectMessage` | Java-объект, сейчас используют осторожно |
| `BytesMessage` | бинарные данные |
| `MapMessage` | набор key-value |
| `StreamMessage` | последовательность примитивов |
| `Message` | сообщение без body, только headers/properties |

Практически чаще всего используют `TextMessage` с JSON или `BytesMessage` с protobuf/avro.

## Методы отправки сообщений JMS

Классический API:

```java
MessageProducer producer = session.createProducer(queue);
TextMessage message = session.createTextMessage("{\"orderId\": 10}");
producer.send(message);
```

Упрощенный API:

```java
try (JMSContext context = connectionFactory.createContext()) {
    context.createProducer()
           .send(queue, "{\"orderId\": 10}");
}
```

Что можно настроить при отправке:

| Настройка | Что означает |
|---|---|
| `DeliveryMode.PERSISTENT` | сообщение нужно сохранить надежно |
| `DeliveryMode.NON_PERSISTENT` | быстрее, но надежность ниже |
| Priority | приоритет сообщения |
| TTL | срок жизни сообщения |
| `JMSCorrelationID` | связь между request и response |
| `JMSReplyTo` | адрес для ответа |

### Отправка в очередь

```text
Producer -> Queue -> Consumer
```

Используется для:

- фоновых задач;
- обработки команд;
- email/SMS-рассылки;
- генерации отчетов;
- интеграции с внешней системой.

Особенность: одно сообщение получает один обработчик.

### Отправка в topic

```text
Producer -> Topic -> несколько Subscribers
```

Используется для:

- доменных событий;
- уведомления нескольких систем;
- event-driven архитектуры.

Особенность: одно событие могут обработать разные независимые подписчики.

## Получение сообщений JMS

Есть два режима.

### Синхронное получение

Consumer явно вызывает `receive`.

```java
Message message = consumer.receive(5000);
```

Подходит:

- для простых worker'ов;
- для тестов;
- когда поток сам контролирует цикл обработки.

Минус: поток блокируется.

### Асинхронное получение

Consumer регистрирует listener.

```java
consumer.setMessageListener(message -> {
    // обработка
});
```

В Jakarta EE часто используют **Message-Driven Bean**:

```java
@MessageDriven(activationConfig = {
    @ActivationConfigProperty(
        propertyName = "destinationLookup",
        propertyValue = "jms/paymentQueue"
    )
})
public class PaymentListener implements MessageListener {
    public void onMessage(Message message) {
        // обработка сообщения
    }
}
```

В Spring часто используют:

```java
@JmsListener(destination = "payment.requests")
public void handle(String payload) {
    // обработка сообщения
}
```

## Модели доставки сообщений в JMS

### Point-to-Point

Это модель очереди.

```text
Producer -> Queue -> один Consumer
```

Гарантия: каждое сообщение должно быть обработано одним получателем.

Хорошо для:

- задач;
- команд;
- work queue;
- асинхронного выполнения бизнес-операций.

### Publish / Subscribe

Это модель топика.

```text
Publisher -> Topic -> Subscriber A
                  -> Subscriber B
```

Гарантия: каждый активный подписчик получает копию сообщения.

Хорошо для:

- событий;
- уведомлений;
- интеграции нескольких подсистем.

### Durable subscription

Durable subscription - долговременная подписка на topic.

Если subscriber отключился, провайдер может сохранить для него сообщения и доставить позже.

Нужно, когда:

- подписчик не должен терять события;
- обработчик может перезапускаться;
- события важны для бизнес-сценария.

### Non-durable subscription

Если subscriber не подключен, сообщения для него не сохраняются.

Подходит для:

- неважных уведомлений;
- live updates;
- мониторинга;
- временных подписчиков.

## Надежность доставки

Типовые варианты, которые обсуждают на экзамене:

| Семантика | Что означает |
|---|---|
| At-most-once | не больше одного раза, но сообщение можно потерять |
| At-least-once | минимум один раз, возможны дубли |
| Exactly-once | ровно один раз на уровне бизнес-эффекта, дорого и сложно |

Для JMS на практике чаще проектируют под **at-least-once**:

- сообщение может быть доставлено повторно;
- обработчик должен быть идемпотентным;
- нужен учет обработанных message id или business id;
- ошибки отправляют в DLQ.

## Acknowledge modes

Если session не транзакционная, acknowledge определяет, когда сообщение считается обработанным.

| Mode | Смысл |
|---|---|
| `AUTO_ACKNOWLEDGE` | провайдер подтверждает автоматически после успешного receive/listener |
| `CLIENT_ACKNOWLEDGE` | приложение явно вызывает `acknowledge()` |
| `DUPS_OK_ACKNOWLEDGE` | ленивое подтверждение, возможны дубли |

Если session транзакционная:

- `commit()` подтверждает обработку;
- `rollback()` возвращает сообщение для повторной доставки.

## Транзакции JMS

JMS session может быть транзакционной.

Пример:

```text
receive message
process business logic
send another message
commit session
```

Если ошибка:

```text
rollback session
message can be redelivered
```

В Jakarta EE можно связать JMS с JTA-транзакцией:

```text
DB update + JMS send в одной глобальной транзакции
```

Но в современных микросервисах часто избегают распределенных транзакций и используют:

- transactional outbox;
- saga;
- идемпотентные consumer'ы;
- retry и DLQ.

## Распределенная обработка в JMS

Распределенная обработка означает, что сообщения обрабатываются несколькими процессами, узлами или сервисами.

Пример:

```text
Queue: report.jobs
  -> worker-1
  -> worker-2
  -> worker-3
```

Плюсы:

- горизонтальное масштабирование;
- балансировка нагрузки;
- устойчивость к падению одного worker'а;
- буферизация пиков.

Риски:

- повторная доставка;
- гонки за общие ресурсы;
- нарушение порядка;
- сложнее отлаживать;
- нужна идемпотентность.

## Что сказать на экзамене

JMS - это стандартный Java/Jakarta API для обмена сообщениями. Он не является брокером и не задает конкретный сетевой протокол. Приложение работает с абстракциями `ConnectionFactory`, `Destination`, `Queue`, `Topic`, `MessageProducer`, `MessageConsumer`, а реальную доставку выполняет JMS-провайдер. JMS поддерживает очереди, publish/subscribe, транзакции, подтверждения доставки и асинхронную обработку.

---

# 2. RabbitMQ

## Что такое RabbitMQ

**RabbitMQ** - брокер сообщений.

Он принимает сообщения от publishers, маршрутизирует их через exchanges и кладет в queues, откуда их читают consumers.

Главная модель RabbitMQ:

```text
Publisher -> Exchange -> Binding -> Queue -> Consumer
```

RabbitMQ часто используют, когда нужна классическая очередь задач или гибкая маршрутизация сообщений.

## Архитектура RabbitMQ

| Компонент | Роль |
|---|---|
| Producer / Publisher | отправляет сообщение |
| Exchange | принимает сообщение и решает, куда его направить |
| Binding | правило связи exchange и queue |
| Queue | хранит сообщения до чтения consumer'ом |
| Consumer | получает и обрабатывает сообщения |
| Virtual host | логическая изоляция ресурсов |
| Connection | TCP-соединение клиента с broker'ом |
| Channel | легковесный логический канал внутри connection |

Схема:

```text
Publisher
  -> Exchange
      -> Binding by routing key
          -> Queue
              -> Consumer
```

## Exchange types

| Exchange | Как маршрутизирует |
|---|---|
| `direct` | по точному routing key |
| `topic` | по шаблону routing key |
| `fanout` | во все связанные queues |
| `headers` | по headers сообщения |

Примеры:

```text
direct: routing key = payment.created
topic:  routing key = order.* или order.# 
fanout: всем подписчикам
```

## Где используется RabbitMQ

RabbitMQ хорошо подходит для:

- очередей фоновых задач;
- email/SMS/notification jobs;
- RPC через сообщения;
- интеграции микросервисов;
- routing-heavy систем;
- delayed/retry processing;
- задач, где сообщение должно быть обработано одним worker'ом.

Примеры:

```text
order-service -> RabbitMQ -> payment-worker
user-service  -> RabbitMQ -> email-worker
api-service   -> RabbitMQ -> report-worker
```

## Преимущества RabbitMQ

- простая ментальная модель очереди;
- гибкая маршрутизация через exchanges;
- зрелая экосистема;
- поддержка AMQP, MQTT, STOMP через плагины;
- acknowledgements;
- dead letter exchanges;
- retries;
- management UI;
- удобно использовать для task queue.

## Недостатки RabbitMQ

- не предназначен для долговременного хранения огромного event log;
- повторное чтение старых сообщений не является основной моделью;
- порядок сообщений может нарушаться при requeue, priority queue, нескольких consumer'ах;
- высокая нагрузка требует аккуратной настройки;
- для event streaming Kafka обычно подходит лучше;
- кластеризация и high availability сложнее, чем кажется на старте.

## RabbitMQ vs JMS

RabbitMQ - конкретный брокер.

JMS - Java API.

RabbitMQ может быть доступен из Java, но его нативная модель - AMQP exchanges/queues/bindings, а не JMS. Для JMS чаще используют ActiveMQ/Artemis/IBM MQ.

## Что сказать на экзамене

RabbitMQ - брокер сообщений, основанный вокруг AMQP-модели: publisher отправляет сообщение в exchange, exchange по bindings и routing key направляет его в queue, consumer читает из queue. RabbitMQ удобен для очередей задач, фоновой обработки и гибкой маршрутизации. Его слабое место - он не является распределенным журналом событий как Kafka.

---

# 3. Kafka

## Что такое Kafka

**Apache Kafka** - распределенная платформа для хранения и обработки потоков событий.

Kafka ближе не к классической очереди, а к **распределенному append-only log**.

Главная идея:

```text
Producer -> Topic -> Partition -> Offset -> Consumer Group
```

Сообщения в Kafka обычно не удаляются сразу после чтения. Они хранятся по retention policy, а consumer сам хранит позицию чтения - offset.

## Архитектура Kafka

| Компонент | Роль |
|---|---|
| Broker | сервер Kafka |
| Topic | логический поток событий |
| Partition | часть topic, единица параллелизма и порядка |
| Offset | позиция записи внутри partition |
| Producer | пишет события |
| Consumer | читает события |
| Consumer group | группа consumer'ов, совместно читающих topic |
| Controller / KRaft quorum | управляет metadata кластера |
| Replica | копия partition на broker'е |
| Leader replica | принимает запись и обычно обслуживает чтение |
| Follower replica | копирует данные у leader |

Схема:

```text
Kafka Cluster
  broker-1
  broker-2
  broker-3

Topic: orders
  partition-0: leader broker-1, replicas broker-2/broker-3
  partition-1: leader broker-2, replicas broker-1/broker-3
```

## Topic и partition

Topic - имя потока событий.

Partition - часть topic.

Важно:

```text
Kafka гарантирует порядок только внутри одной partition.
```

Если нужно сохранить порядок событий по заказу:

```text
key = orderId
```

Тогда события одного заказа попадут в одну partition.

## Producer

Producer отправляет record в topic.

Record обычно содержит:

| Поле | Смысл |
|---|---|
| key | ключ, часто влияет на выбор partition |
| value | payload |
| headers | метаданные |
| timestamp | время события или записи |

Настройки надежности:

| Настройка | Смысл |
|---|---|
| `acks=0` | не ждать подтверждения |
| `acks=1` | ждать подтверждения leader |
| `acks=all` | ждать подтверждения от ISR |
| `retries` | повторы при ошибках |
| `enable.idempotence=true` | защита от дублей при retry на producer side |

## Consumer group

Consumer group - группа обработчиков с одним group id.

Правило:

```text
Одна partition внутри одной consumer group назначается только одному consumer'у.
```

Пример:

```text
Topic orders, 3 partitions
Consumer group payment-service:
  consumer-1 -> partition-0
  consumer-2 -> partition-1
  consumer-3 -> partition-2
```

Если consumer'ов больше, чем partition'ов, лишние будут простаивать.

## Offset

Offset - позиция сообщения внутри partition.

Consumer хранит offset, чтобы знать, откуда продолжать чтение.

Можно:

- читать новые события;
- перечитывать старые;
- перематывать offset;
- запускать разные consumer groups независимо.

## Репликация и отказоустойчивость

У partition есть leader и followers.

```text
Producer -> Leader partition -> Followers replicate
```

**ISR** - in-sync replicas, реплики, которые достаточно синхронны с leader.

При сбое leader Kafka выбирает нового leader из актуальных реплик.

В современной Kafka для metadata используется **KRaft**. В старых материалах часто встречается ZooKeeper.

## Особенности Kafka

- высокая пропускная способность;
- хранение событий по retention;
- повторное чтение событий;
- независимые consumer groups;
- порядок внутри partition;
- горизонтальное масштабирование через partitions;
- интеграция с stream processing;
- удобна для event sourcing, CDC, логов, аналитики.

## Преимущества Kafka

- хорошо держит большой поток событий;
- можно хранить историю и перечитывать;
- один topic могут читать много систем;
- высокая производительность;
- масштабирование через partition;
- сильная экосистема: Kafka Streams, Connect, Schema Registry, ksqlDB, Flink/Spark integrations;
- хорошо подходит для event-driven архитектуры.

## Недостатки Kafka

- сложнее RabbitMQ для простых очередей;
- не лучший выбор для маленькой task queue;
- порядок только внутри partition;
- нужно проектировать ключи сообщений;
- требуется операционная экспертиза;
- consumer должен сам корректно управлять offset;
- сложные retry/DLQ сценарии требуют отдельного дизайна;
- не заменяет BPMS и workflow engine.

## Где используется Kafka

Kafka хорошо подходит для:

- event-driven микросервисов;
- аудита событий;
- логов и метрик;
- clickstream;
- CDC из базы данных;
- интеграции между системами;
- streaming analytics;
- event sourcing;
- передачи событий нескольким независимым потребителям.

Kafka хуже подходит для:

- простой очереди "взял задачу и удалил";
- request/response с быстрым ответом;
- сложной маршрутизации по правилам;
- бизнес-процессов с human tasks и BPMN;
- маленьких систем, где сложность не окупается.

## Kafka vs классическая очередь

| Критерий | Kafka | Классическая очередь |
|---|---|---|
| Основная модель | distributed log | queue |
| После чтения | сообщение остается до retention | обычно удаляется/acknowledged |
| Повторное чтение | нормальная практика | обычно неудобно |
| Масштабирование | partitions | consumers/queues |
| Порядок | внутри partition | зависит от брокера и consumers |
| Лучший сценарий | поток событий | фоновые задачи |

## Что сказать на экзамене

Kafka - это распределенный журнал событий. Producer пишет records в topic, topic делится на partitions, внутри partition есть offset и сохраняется порядок. Consumer'ы читают данные группами, каждая consumer group имеет свои offsets. Kafka подходит для больших потоков событий, интеграции микросервисов, аналитики и CDC, но избыточна для простой очереди задач.

---

# 4. ActiveMQ / Apache MQ

## Что такое ActiveMQ

**Apache ActiveMQ** - семейство брокеров сообщений от Apache.

Обычно важно различать:

| Продукт | Что это |
|---|---|
| ActiveMQ Classic | старый, зрелый JMS-брокер |
| ActiveMQ Artemis | более современный брокер, основан на HornetQ, используется как развитие ActiveMQ |

Если в билете написано **Apache MQ**, обычно имеют в виду **Apache ActiveMQ**.

## Архитектура ActiveMQ / Artemis

Типовая схема:

```text
Java App
  -> JMS API
  -> ActiveMQ / Artemis Broker
  -> Queue / Topic
  -> Consumer
```

Компоненты:

| Компонент | Роль |
|---|---|
| Broker | принимает, хранит и доставляет сообщения |
| Queue | point-to-point доставка |
| Topic | publish/subscribe доставка |
| Address | адрес маршрутизации в Artemis |
| Routing type | `anycast` или `multicast` в Artemis |
| Journal | хранилище сообщений |
| Connector / Acceptor | сетевое подключение клиентов |

В Artemis:

| Routing type | Аналог |
|---|---|
| `anycast` | очередь, один consumer |
| `multicast` | topic, несколько subscribers |

## Поддерживаемые протоколы

ActiveMQ/Artemis могут поддерживать:

- JMS;
- AMQP;
- OpenWire;
- STOMP;
- MQTT;
- core protocol Artemis.

Это делает их удобными для enterprise-интеграции.

## Где используется ActiveMQ

ActiveMQ хорошо подходит для:

- Java EE / Jakarta EE систем;
- JMS-интеграции;
- legacy enterprise систем;
- очередей задач;
- интеграции приложений через queue/topic;
- систем, где нужен JMS API и транзакции.

## Преимущества ActiveMQ

- хорошая интеграция с Java/JMS;
- поддержка queue и topic;
- транзакции и acknowledge;
- разные протоколы;
- зрелая enterprise-модель;
- удобен для Jakarta EE;
- Artemis быстрее и современнее Classic.

## Недостатки ActiveMQ

- для event streaming Kafka обычно сильнее;
- Classic может быть legacy-выбором;
- кластеризация и HA требуют аккуратной настройки;
- под очень большие event logs не лучший выбор;
- меньше популярен в новых cloud-native архитектурах, чем Kafka/RabbitMQ.

## Что сказать на экзамене

ActiveMQ - это брокер сообщений, часто используемый как JMS-провайдер. Он поддерживает очереди, топики, транзакции и разные протоколы. ActiveMQ удобен для Java enterprise систем, где нужен стандарт JMS. ActiveMQ Artemis - более современная ветка, в которой есть address model и routing types `anycast`/`multicast`.

---

# 5. RabbitMQ vs Kafka vs ActiveMQ

| Критерий | RabbitMQ | Kafka | ActiveMQ / Artemis |
|---|---|---|---|
| Главная модель | message broker | distributed log | JMS/enterprise broker |
| Лучше всего | task queue, routing | event streaming | JMS, enterprise messaging |
| Хранение истории | не основная модель | основная модель | возможно, но не как Kafka |
| Повторное чтение | неудобно | удобно | ограниченно |
| Маршрутизация | сильная | простая по topic/partition | queue/topic/address |
| Java/JMS | не основная модель | не JMS | сильная сторона |
| Типичный use case | jobs, notifications | events, analytics, CDC | Java EE messaging |

Коротко:

- **RabbitMQ** - когда нужна очередь задач и гибкая маршрутизация.
- **Kafka** - когда нужен поток событий, хранение и повторное чтение.
- **ActiveMQ** - когда нужен JMS-провайдер для Java enterprise.

---

# 6. Протоколы взаимодействия с очередями сообщений

## Важное различие

**JMS - это API, а не сетевой протокол.**

Протоколы описывают, как клиент и брокер общаются по сети.

API описывает, как программист пишет код.

```text
Java code -> JMS API -> provider implementation -> network protocol -> broker
```

## AMQP

**AMQP** - Advanced Message Queuing Protocol.

Есть две важные версии:

| Версия | Где встречается |
|---|---|
| AMQP 0-9-1 | основная модель RabbitMQ |
| AMQP 1.0 | enterprise-интеграции, Azure Service Bus, Artemis |

AMQP 0-9-1 использует:

- exchanges;
- queues;
- bindings;
- routing keys;
- acknowledgements.

Плюсы:

- открытый протокол;
- хорош для брокеров сообщений;
- поддерживает надежную доставку;
- гибкая маршрутизация.

Минусы:

- сложнее простого HTTP;
- версии 0-9-1 и 1.0 сильно отличаются;
- не является моделью distributed log как Kafka.

Где используется:

- RabbitMQ;
- ActiveMQ Artemis;
- enterprise messaging.

## MQTT

**MQTT** - легкий publish/subscribe протокол.

Хорош для:

- IoT;
- слабых устройств;
- нестабильной сети;
- телеметрии.

Плюсы:

- легкий;
- мало overhead;
- удобен для устройств;
- есть QoS levels.

Минусы:

- не для сложной enterprise-маршрутизации;
- не для больших event streaming сценариев;
- payload и схемы нужно проектировать отдельно.

## STOMP

**STOMP** - Simple Text Oriented Messaging Protocol.

Плюсы:

- простой текстовый протокол;
- легко отлаживать;
- можно использовать через WebSocket;
- понятен разным языкам.

Минусы:

- меньше возможностей, чем AMQP;
- не лучший выбор для высокой нагрузки;
- часто используется как простой фасад, а не как основа всей messaging-платформы.

Где используется:

- WebSocket messaging;
- Spring WebSocket;
- ActiveMQ/RabbitMQ plugins.

## Kafka protocol

Kafka использует собственный бинарный протокол.

Он заточен под:

- high throughput;
- batching;
- запись в partitions;
- чтение по offset;
- consumer groups.

Плюсы:

- высокая производительность;
- хорошо подходит для log-based модели;
- эффективный протокол для Kafka-кластера.

Минусы:

- специфичен для Kafka;
- не универсальный messaging-протокол;
- требует Kafka-клиентов или совместимых прокси.

## OpenWire

**OpenWire** - протокол ActiveMQ Classic.

Плюсы:

- хорошо интегрирован с ActiveMQ Classic;
- поддерживает JMS-сценарии.

Минусы:

- специфичен для ActiveMQ;
- в новых системах чаще выбирают AMQP/Kafka/RabbitMQ-native подходы.

## HTTP и WebSocket

Иногда сообщения передают через HTTP/WebSocket:

- webhooks;
- callbacks;
- browser notifications;
- simple event push.

Плюсы:

- просто интегрировать;
- легко проходит через инфраструктуру;
- удобно для внешних API.

Минусы:

- HTTP сам по себе не брокер;
- нет очереди без дополнительной системы;
- retry, ordering, durability нужно реализовывать отдельно.

## Сравнение протоколов

| Протокол | Лучше всего | Не лучший выбор |
|---|---|---|
| JMS | Java API к брокеру | межъязыковой сетевой стандарт |
| AMQP | enterprise queues, routing | distributed log |
| MQTT | IoT, телеметрия | сложная бизнес-интеграция |
| STOMP | простые клиенты, WebSocket | high throughput |
| Kafka protocol | Kafka event streaming | универсальные очереди |
| OpenWire | ActiveMQ Classic | новые независимые системы |
| HTTP/WebSocket | webhooks, browser push | надежная очередь без брокера |

## Что сказать на экзамене

Протокол очередей сообщений определяет сетевое взаимодействие клиента и брокера. AMQP хорош для брокеров и маршрутизации, MQTT - для IoT, STOMP - для простых текстовых клиентов и WebSocket, Kafka protocol - для Kafka и чтения по offset. JMS при этом не протокол, а Java API, который скрывает конкретную реализацию провайдера.

---

# 7. Асинхронная обработка сообщений

## Что такое асинхронная обработка

Асинхронная обработка означает, что вызывающий код не ждет полного выполнения работы прямо сейчас.

Синхронно:

```text
Client -> API -> PaymentService -> ответ
```

Асинхронно:

```text
Client -> API -> Queue/Event Log -> Worker -> результат позже
```

## Когда хорошо использовать

Асинхронная обработка подходит, когда:

- операция долгая;
- результат не нужен мгновенно;
- нужно выдержать пики нагрузки;
- нужно развязать сервисы;
- есть фоновые задачи;
- нужно отправлять уведомления;
- нужно интегрироваться с медленной внешней системой;
- есть несколько независимых реакций на событие;
- можно жить с eventual consistency.

Примеры:

| Сценарий | Почему async подходит |
|---|---|
| Отправка email | пользователь не должен ждать SMTP |
| Генерация отчета | долго и ресурсоемко |
| Обработка изображений | CPU-heavy задача |
| Интеграция с платежным провайдером | внешняя система может тормозить |
| Аудит событий | не должен ломать основной запрос |
| Notification fanout | много независимых получателей |

## Когда плохо использовать

Асинхронная обработка не подходит или опасна, когда:

- результат нужен прямо сейчас;
- пользователь должен сразу получить точный ответ;
- операция простая и быстрая;
- порядок операций критичен, а обеспечить его сложно;
- бизнес не допускает eventual consistency;
- команда не готова к retry, DLQ, tracing и идемпотентности;
- система маленькая, а брокер добавляет лишнюю сложность.

Примеры:

| Сценарий | Почему async может быть плох |
|---|---|
| Проверка пароля при логине | нужен немедленный ответ |
| Списание денег без строгого контроля | опасны дубли и гонки |
| Простое чтение справочника | брокер не нужен |
| UI-команда с мгновенным результатом | пользователь ждет ответ |

## Главные риски async

| Риск | Что делать |
|---|---|
| Дубли сообщений | идемпотентные обработчики |
| Потеря сообщений | persistent delivery, replication, monitoring |
| Нарушение порядка | ключи partition, single consumer, ordering design |
| Скрытые ошибки | DLQ, alerts, dashboards |
| Eventual consistency | явно проектировать статусы |
| Сложная отладка | correlation id, trace id, logs |
| Повторные side effects | idempotency key, unique constraint |

## Обязательные паттерны

### Idempotent consumer

Consumer должен корректно переживать повторную доставку.

Пример:

```text
Если paymentId уже обработан, повторно деньги не списывать.
```

Технически:

- хранить processed message id;
- использовать unique constraint;
- проверять business status;
- делать операции повторяемыми без вреда.

### Retry

Retry нужен для временных ошибок:

- сеть;
- timeout;
- временная недоступность внешней системы.

Но retry опасен без лимитов.

Нужны:

- max attempts;
- backoff;
- отдельная retry queue/topic;
- различение temporary/permanent errors.

### DLQ

**Dead Letter Queue** - очередь для сообщений, которые не удалось обработать.

DLQ нужна, чтобы:

- не терять проблемные сообщения;
- не блокировать основную очередь;
- разбирать ошибки вручную или отдельным процессом.

### Transactional outbox

Outbox решает проблему:

```text
DB commit успешен, но message publish упал.
```

Идея:

```text
1. В одной DB transaction сохранить business data и outbox event.
2. Отдельный publisher читает outbox и отправляет событие в брокер.
3. После успешной отправки помечает событие опубликованным.
```

### Saga

Saga - паттерн для длинных бизнес-транзакций между сервисами.

Вместо одной распределенной ACID-транзакции:

- каждый сервис делает локальную транзакцию;
- публикует событие;
- при ошибке выполняются компенсирующие действия.

## Что сказать на экзамене

Асинхронная обработка хороша для долгих задач, фоновой работы, интеграции сервисов и сглаживания нагрузки. Она плохо подходит, когда нужен немедленный точный ответ или строгая согласованность прямо сейчас. Главные условия успешного async-дизайна: идемпотентность, retry, DLQ, correlation id, мониторинг и понятная модель eventual consistency.

---

# 8. Планировщики задач

## Что такое планировщик задач

**Планировщик задач** запускает работу по времени или расписанию.

Примеры:

- каждый день в 03:00 пересчитать статистику;
- каждые 5 минут проверить статусы платежей;
- раз в месяц создать счета;
- через 10 минут повторить неудачную задачу.

Планировщик отвечает на вопрос:

```text
Когда запустить задачу?
```

Очередь сообщений отвечает на другой вопрос:

```text
Как надежно передать работу обработчику?
```

## Cron

**cron** - классическая Unix/Linux система запуска периодических задач.

Архитектура:

```text
crond daemon
  -> читает crontab
  -> в нужное время запускает command
```

Где конфигурируется:

| Место | Для чего |
|---|---|
| `crontab -e` | задачи пользователя |
| `/etc/crontab` | системный crontab |
| `/etc/cron.d/*` | отдельные системные файлы |
| `/etc/cron.hourly` | задачи каждый час |
| `/etc/cron.daily` | задачи каждый день |
| systemd timers | современная альтернатива на Linux |

### Cron expression

Классический cron:

```text
* * * * *
| | | | |
| | | | +-- day of week
| | | +---- month
| | +------ day of month
| +-------- hour
+---------- minute
```

Примеры:

| Expression | Значение |
|---|---|
| `* * * * *` | каждую минуту |
| `*/5 * * * *` | каждые 5 минут |
| `0 3 * * *` | каждый день в 03:00 |
| `0 9 * * 1-5` | по будням в 09:00 |
| `0 0 1 * *` | первого числа каждого месяца |

В Spring и Quartz часто используется 6 или 7 полей, где есть seconds:

```text
second minute hour day-of-month month day-of-week
```

Пример Spring:

```text
0 */5 * * * *
```

Это каждые 5 минут, в нулевую секунду.

## Cron: плюсы и минусы

Плюсы:

- прост;
- встроен в Unix/Linux;
- не требует Java-приложения;
- хорошо подходит для shell-скриптов и системных задач.

Минусы:

- плохо управлять из приложения;
- слабая observability;
- нет нормального retry из коробки;
- нет distributed lock;
- сложно версионировать бизнес-логику;
- неудобен для сложных зависимостей.

## Spring scheduling

Spring поддерживает планирование через `@Scheduled`.

Минимальный пример:

```java
@EnableScheduling
@Configuration
class SchedulingConfig {
}

@Component
class ReportJob {
    @Scheduled(cron = "0 0 3 * * *")
    public void run() {
        // daily report
    }
}
```

Варианты:

```java
@Scheduled(fixedRate = 5000)
@Scheduled(fixedDelay = 5000)
@Scheduled(cron = "0 */5 * * * *")
```

Разница:

| Вариант | Смысл |
|---|---|
| `fixedRate` | запускать каждые N ms от старта предыдущего запуска |
| `fixedDelay` | ждать N ms после завершения предыдущего запуска |
| `cron` | запуск по cron expression |

Плюсы:

- просто;
- удобно в Spring Boot;
- рядом с бизнес-кодом;
- легко использовать dependency injection.

Минусы:

- по умолчанию может быть один поток, если не настроить scheduler;
- в нескольких инстансах задача может запуститься несколько раз;
- для кластера нужен lock: ShedLock, DB lock, Quartz cluster или внешний scheduler;
- нет полноценного job management UI из коробки.

## Jakarta EE timers

В Jakarta EE есть **Timer Service**.

Пример:

```java
@Singleton
public class BillingTimer {
    @Schedule(hour = "3", minute = "0", persistent = true)
    public void runBilling() {
        // monthly billing logic
    }
}
```

Особенности:

- работает внутри application server;
- интегрируется с EJB/CDI окружением;
- может быть persistent;
- подходит для enterprise Java приложений.

Плюсы:

- стандарт Jakarta EE;
- интеграция с контейнером;
- можно использовать транзакции и container services.

Минусы:

- привязка к Jakarta EE серверу;
- менее популярен в Spring Boot проектах;
- меньше гибкости, чем Quartz.

## Quartz

**Quartz** - Java-библиотека для планирования задач.

Главные компоненты:

| Компонент | Роль |
|---|---|
| `Scheduler` | управляет задачами |
| `Job` | код, который выполняется |
| `JobDetail` | описание job и ее data |
| `Trigger` | когда запускать job |
| `JobStore` | где хранить расписания и состояние |
| `ThreadPool` | потоки выполнения |
| Listener | реакции на события scheduler/job/trigger |

Схема:

```text
Scheduler
  -> Trigger fires
  -> JobDetail
  -> Job.execute()
```

### Triggers

| Trigger | Для чего |
|---|---|
| `SimpleTrigger` | один запуск или повтор с интервалом |
| `CronTrigger` | запуск по cron expression |
| Calendar-based trigger | более сложные календарные правила |

### JobStore

| JobStore | Смысл |
|---|---|
| RAMJobStore | хранение в памяти, теряется при restart |
| JDBC JobStore | хранение в базе, можно переживать restart |

Quartz может работать в cluster mode, если несколько приложений используют одну БД и корректно настроены.

### Misfire

**Misfire** - ситуация, когда задача должна была запуститься, но scheduler не смог сделать это вовремя.

Причины:

- приложение было выключено;
- не хватило потоков;
- scheduler был перегружен.

Для misfire задают policy:

- выполнить сразу;
- пропустить;
- пересчитать следующий запуск.

## Cron vs Spring vs Jakarta EE vs Quartz

| Критерий | Cron | Spring `@Scheduled` | Jakarta EE Timer | Quartz |
|---|---|---|---|---|
| Где работает | ОС | Spring app | Jakarta EE server | Java app |
| Простота | высокая | высокая | средняя | средняя |
| Dependency injection | нет | да | да | да |
| Persistent jobs | ограниченно | нет из коробки | да | да через JDBC |
| Cluster mode | нет | нет из коробки | зависит от сервера | да |
| UI | нет | нет | зависит от сервера | обычно отдельно |
| Лучший use case | системные скрипты | простые app jobs | enterprise Java | сложные расписания |

## Что сказать на экзамене

Планировщик задач запускает код по времени. Cron работает на уровне ОС и удобен для системных команд. Spring `@Scheduled` прост для задач внутри Spring-приложения, но требует отдельного решения для кластера. Jakarta EE Timer Service подходит для enterprise приложений на Jakarta EE. Quartz - более мощная Java-библиотека с jobs, triggers, persistent storage, misfire handling и cluster mode.

---

# 9. BPMS

## Что такое BPMS

**BPMS** - Business Process Management System.

Это система для моделирования, исполнения, мониторинга и улучшения бизнес-процессов.

Главная идея:

```text
Бизнес-процесс описывается явно,
движок исполняет его,
а люди и системы выполняют отдельные шаги.
```

Пример бизнес-процесса:

```text
Заявка на кредит
  -> проверка данных
  -> скоринг
  -> ручное согласование
  -> решение
  -> уведомление клиента
```

## Зачем нужен BPMS

BPMS нужен, когда процесс:

- длинный;
- многошаговый;
- включает людей;
- включает несколько систем;
- должен быть виден бизнесу;
- меняется со временем;
- требует аудита;
- имеет ветвления, таймеры, компенсации и статусы.

Если процесс простой и полностью технический, BPMS может быть лишним.

## Ключевые компоненты BPMS

| Компонент | Роль |
|---|---|
| Process Modeler | моделирование процесса |
| Process Engine | исполнение процесса |
| Tasklist | пользовательские задачи |
| Rules / DMN | бизнес-правила и решения |
| Forms | формы для human tasks |
| Connectors / Workers | интеграция с сервисами |
| Monitoring | контроль процесса |
| Audit log | история выполнения |
| Admin tools | управление инцидентами и версиями |

## Жизненный цикл бизнес-процесса в BPMS

### 1. Анализ

На этом этапе выясняют:

- цель процесса;
- участников;
- входы и выходы;
- события старта;
- возможные ветвления;
- SLA;
- ошибки и исключения.

Результат: понятное описание процесса.

### 2. Моделирование

Процесс описывают в нотации, чаще всего BPMN.

Определяют:

- start event;
- tasks;
- gateways;
- events;
- end events;
- lanes/pools;
- timers;
- error paths.

Результат: BPMN-модель.

### 3. Реализация

Модель связывают с кодом:

- service tasks вызывают workers/delegates;
- user tasks получают forms;
- business rules выносят в DMN;
- интеграции подключают через REST, messaging, connectors.

Результат: исполняемый процесс.

### 4. Развертывание

Процесс публикуют в BPMS engine.

Важно:

- версионирование моделей;
- миграция активных экземпляров;
- права доступа;
- настройки окружения.

### 5. Исполнение

BPMS создает process instance.

Дальше:

- engine двигает токены по BPMN;
- service tasks выполняются кодом;
- user tasks ждут человека;
- timers ждут времени;
- errors создают инциденты или идут по error boundary events.

### 6. Мониторинг

Смотрят:

- сколько процессов запущено;
- где они зависли;
- какие ошибки возникли;
- какие SLA нарушены;
- какие задачи ждут пользователей.

### 7. Оптимизация

На основе данных процесс улучшают:

- убирают узкие места;
- меняют правила;
- автоматизируют ручные шаги;
- сокращают время выполнения.

## Преимущества BPMS

- процесс виден явно;
- бизнес и разработчики говорят на одной модели;
- проще сопровождать длинные workflow;
- есть аудит;
- есть мониторинг;
- удобно работать с human tasks;
- можно менять процесс без полного переписывания приложения.

## Недостатки BPMS

- повышает сложность архитектуры;
- требует дисциплины моделирования;
- не подходит для каждой простой операции;
- можно получить "бизнес-логику, размазанную по диаграммам";
- нужны знания BPMN/DMN;
- эксплуатация engine тоже стоит денег и времени.

## Когда BPMS подходит

BPMS подходит для:

- кредитных заявок;
- страховых кейсов;
- согласований;
- onboarding;
- KYC/AML;
- документооборота;
- заказов с множеством статусов;
- процессов с участием людей.

BPMS не нужен для:

- простого CRUD;
- одной короткой транзакции;
- низкоуровневой технической очереди;
- простого scheduled job;
- высокочастотного event streaming.

## Что сказать на экзамене

BPMS - это система управления бизнес-процессами. Она позволяет моделировать процесс, исполнять его через process engine, назначать пользовательские задачи, интегрироваться с сервисами, мониторить выполнение и улучшать процесс. BPMS полезна для длинных, изменяемых, многошаговых процессов с участием людей и систем. Минус - дополнительная сложность и необходимость правильно разделять код и процессную модель.

---

# 10. Camunda

## Что такое Camunda

**Camunda** - платформа для автоматизации бизнес-процессов.

Она позволяет:

- моделировать процессы в BPMN;
- исполнять process instances;
- назначать user tasks;
- вызывать service tasks;
- использовать DMN decision tables;
- мониторить процессы;
- разбирать инциденты.

## Camunda 7 vs Camunda 8

| Критерий | Camunda 7 | Camunda 8 |
|---|---|---|
| Engine | embedded/process engine на Java | Zeebe distributed workflow engine |
| Типичная интеграция | Java delegates, external tasks, REST | job workers, gRPC/REST, cloud-native |
| Транзакции | связаны с ACID transaction в engine DB | event-sourced log, распределенная модель |
| Хранилище engine | relational DB | Zeebe log + exporters |
| Операционный инструмент | Cockpit | Operate |
| Пользовательские задачи | Tasklist | Tasklist |
| Моделирование | Modeler | Web/Desktop Modeler |

Коротко:

- **Camunda 7** ближе к Java enterprise engine с relational DB.
- **Camunda 8** ближе к cloud-native orchestration engine на Zeebe.

## BPMN: из чего состоит

Основные элементы BPMN:

| Элемент | Смысл |
|---|---|
| Start Event | начало процесса |
| End Event | конец процесса |
| User Task | задача человека |
| Service Task | автоматическая задача |
| Script Task | выполнение скрипта |
| Business Rule Task | вызов DMN/правил |
| Exclusive Gateway | выбор одного пути |
| Parallel Gateway | параллельные ветки |
| Event-based Gateway | ожидание одного из событий |
| Timer Event | ожидание времени |
| Message Event | ожидание/отправка сообщения |
| Error Boundary Event | обработка ошибки |
| Subprocess | вложенный процесс |
| Call Activity | вызов другого процесса |

Минимальная схема:

```text
Start
  -> User Task: заполнить заявку
  -> Service Task: проверить данные
  -> Gateway: решение
      -> approved -> End
      -> rejected -> End
```

## Состав ПО Camunda

Типовые компоненты:

| Компонент | Роль |
|---|---|
| Modeler | создание BPMN/DMN/forms |
| Process Engine / Zeebe | исполнение процессов |
| Tasklist | работа пользователей с задачами |
| Cockpit / Operate | мониторинг и операции |
| Admin / Identity | пользователи, группы, права |
| Optimize | аналитика процессов |
| Workers / Delegates | выполнение service tasks |

## Зачем нужен Cockpit

**Cockpit** - инструмент Camunda 7 для технического мониторинга процессов.

Он нужен, чтобы:

- видеть deployed process definitions;
- смотреть running process instances;
- находить, где процесс остановился;
- анализировать incidents;
- повторять failed jobs;
- смотреть переменные процесса;
- выполнять migration process instances;
- помогать сопровождать production.

Важно:

```text
Cockpit не моделирует процесс.
Он помогает наблюдать и управлять выполнением процесса.
```

В Camunda 8 похожую операционную роль выполняет **Operate**.

## Создание и управление бизнес-процессами в Camunda

Типичный цикл:

1. Нарисовать BPMN в Modeler.
2. Добавить technical ids: process id, task ids, message names.
3. Для service tasks указать delegate/job type.
4. Для user tasks назначить candidate groups/users.
5. Добавить forms или внешние UI.
6. Добавить DMN, если нужны бизнес-правила.
7. Задеплоить модель.
8. Запустить process instance.
9. Обрабатывать jobs/tasks.
10. Мониторить выполнение через Cockpit/Operate.
11. Исправлять incidents.
12. Версионировать модель.

## Изменение бизнес-процессов

Изменение процесса требует аккуратности:

- новая версия BPMN применяется к новым process instances;
- старые instances могут продолжить выполнение по старой версии;
- для активных instances может понадобиться migration;
- нельзя бездумно удалять activity, на которой уже стоят процессы;
- нужно учитывать переменные и compatibility.

## Транзакции в Camunda 7

Camunda 7 использует relational DB и выполняет процесс между transaction boundaries.

Ключевое понятие:

```text
Wait state - точка сохранения состояния процесса.
```

Wait states:

- user task;
- receive task;
- message catch event;
- timer event;
- external task;
- async continuation.

Что происходит:

```text
engine выполняет шаги процесса
  -> доходит до wait state
  -> сохраняет состояние в DB
  -> commit transaction
```

Если ошибка до commit:

- изменения процесса откатываются;
- process instance возвращается к последнему сохраненному состоянию.

### Async before / async after

`asyncBefore` и `asyncAfter` создают transaction boundary.

Нужно, когда:

- service task может упасть;
- нужно retry через job executor;
- нельзя держать длинную transaction;
- нужно отделить технический шаг от предыдущего состояния.

Пример:

```text
User Task
  -> asyncBefore Service Task
  -> Service Task выполняется job executor'ом
```

### Job Executor

Job Executor выполняет async jobs:

- async continuations;
- timers;
- retries;
- failed jobs.

При ошибке job может повторяться. После исчерпания retries возникает incident.

## Транзакции в Camunda 8

Camunda 8 использует Zeebe.

Модель другая:

- Zeebe - распределенный workflow engine;
- состояние пишется в append-only log;
- workers забирают jobs и завершают их;
- процесс движется событиями;
- нет той же модели embedded ACID transaction, как в Camunda 7.

Упрощенная схема:

```text
Zeebe creates job
  -> Worker activates job
  -> Worker executes business code
  -> Worker completes/fails job
  -> Zeebe records next state
```

Практический вывод:

- бизнес-код worker'а должен быть идемпотентным;
- внешние side effects нужно проектировать осторожно;
- retry и timeout задаются для jobs;
- consistency чаще строится через process orchestration и compensations.

## Camunda и разработка приложений с BPMS

Стадии разработки:

| Стадия | Что делают |
|---|---|
| Анализ | описывают бизнес-процесс и участников |
| BPMN-моделирование | рисуют процесс |
| Техническая детализация | задают ids, variables, workers, forms |
| Реализация | пишут delegates/workers/services |
| Интеграция | подключают REST, messaging, DB, external systems |
| Тестирование | проверяют happy path, errors, timers, compensation |
| Деплой | публикуют BPMN/DMN/forms и приложение |
| Мониторинг | смотрят instances, incidents, SLA |
| Версионирование | выпускают новые версии процесса |

## Camunda: преимущества

- BPMN делает процесс видимым;
- удобно для long-running processes;
- есть user tasks;
- есть monitoring tools;
- есть DMN;
- есть retry/incidents;
- процесс можно версионировать;
- бизнес-логика процесса отделяется от технического кода.

## Camunda: недостатки

- BPMN легко усложнить;
- не вся логика должна быть на диаграмме;
- нужно понимать transaction boundaries;
- нужны тесты процессов;
- при неправильном дизайне получается тяжелая поддержка;
- Camunda не заменяет брокер сообщений, БД или обычный код.

## Что сказать на экзамене

Camunda - BPMS/workflow-платформа для моделирования и исполнения бизнес-процессов в BPMN. В Camunda 7 процесс исполняется Java engine'ом с relational DB, а транзакционные границы обычно совпадают с wait states или async continuations. Cockpit нужен для технического мониторинга process instances, incidents, переменных и jobs. В Camunda 8 используется Zeebe, где процесс исполняется распределенно через log и job workers.

---

# 11. Экзаменационные мини-ответы

## Распределенная обработка в JMS

Распределенная обработка в JMS - это обработка сообщений несколькими consumer'ами или узлами. Producer кладет сообщение в queue/topic, а consumers обрабатывают его независимо. Это дает масштабирование и отказоустойчивость, но требует идемпотентности, контроля повторной доставки и аккуратной работы с порядком сообщений.

## Ресурсы и сообщения в JMS

Основные ресурсы JMS: `ConnectionFactory`, `Connection`, `Session`, `Destination`, `Queue`, `Topic`, `MessageProducer`, `MessageConsumer`, `JMSContext`. Сообщение состоит из headers, properties и body. Типы сообщений: `TextMessage`, `BytesMessage`, `ObjectMessage`, `MapMessage`, `StreamMessage`, `Message`.

## Архитектура JMS

JMS-клиент через JMS API обращается к JMS-провайдеру. Провайдер управляет очередями и топиками, хранит сообщения и доставляет их consumer'ам. Архитектурно JMS разделяет приложение и конкретный брокер, поэтому Java-код работает со стандартными интерфейсами.

## Методы отправки сообщений JMS

Сообщения отправляют через `MessageProducer.send()` или через `JMSContext.createProducer().send()`. Можно отправлять в `Queue` для point-to-point модели или в `Topic` для publish/subscribe. При отправке настраивают delivery mode, priority, TTL, correlation id и reply-to.

## Модели доставки сообщений в JMS

Главные модели: point-to-point через queue и publish/subscribe через topic. В queue одно сообщение получает один consumer. В topic одно сообщение получают подписчики. Подписка может быть durable, если сообщения нужно сохранять для временно отключенного подписчика.

## RabbitMQ

RabbitMQ - брокер сообщений с моделью publisher -> exchange -> binding -> queue -> consumer. Он хорош для очередей задач, routing-heavy сценариев и фоновой обработки. Плюсы: гибкая маршрутизация, acknowledgements, DLQ, management UI. Минусы: не distributed log, не лучший вариант для длительного хранения и повторного чтения больших потоков событий.

## Kafka

Kafka - распределенный append-only log для событий. Producer пишет records в topic, topic делится на partitions, consumer groups читают сообщения по offsets. Kafka хороша для event streaming, CDC, аналитики, логов и интеграции микросервисов. Минусы: сложность эксплуатации, порядок только внутри partition, избыточность для простой очереди задач.

## ActiveMQ

ActiveMQ - Apache-брокер сообщений и JMS-провайдер. Поддерживает queue/topic, транзакции, acknowledge и разные протоколы. ActiveMQ Classic - старый зрелый брокер, ActiveMQ Artemis - более современный. Подходит для Java enterprise и Jakarta EE систем.

## Протоколы очередей сообщений

AMQP используют для брокеров и маршрутизации, MQTT - для IoT, STOMP - для простых текстовых клиентов и WebSocket, Kafka protocol - для Kafka, OpenWire - для ActiveMQ Classic. JMS не является протоколом, это Java API.

## Когда использовать async

Async стоит использовать для долгих операций, фоновых задач, интеграции сервисов, пиков нагрузки и событийной архитектуры. Не стоит использовать, если нужен немедленный точный ответ, строгая согласованность в одном запросе или задача слишком простая. Async требует retry, DLQ, идемпотентности и мониторинга.

## Планировщики задач

Планировщики запускают задачи по времени. Cron работает на уровне ОС. Spring `@Scheduled` удобен внутри Spring-приложения. Jakarta EE Timer Service подходит для enterprise Java. Quartz нужен для сложных расписаний, persistent jobs, misfire handling и cluster mode.

## BPMS

BPMS управляет жизненным циклом бизнес-процесса: моделирование, исполнение, user tasks, интеграции, мониторинг, аудит и оптимизация. BPMS полезна для длинных процессов с людьми и несколькими системами. Минус - дополнительная сложность.

## Camunda

Camunda - BPMS/workflow engine с BPMN, DMN, Tasklist и операционными инструментами. Cockpit в Camunda 7 нужен для мониторинга и управления process instances, incidents и jobs. В Camunda 7 транзакционные границы связаны с wait states и async continuations. В Camunda 8 используется Zeebe и job workers.

---

# 12. Быстрое сравнение

## Очередь, topic, log

| Модель | Смысл | Пример технологии |
|---|---|---|
| Queue | одно сообщение одному worker'у | JMS Queue, RabbitMQ Queue |
| Topic pub/sub | одно событие многим подписчикам | JMS Topic |
| Distributed log | события хранятся и читаются по offset | Kafka |

## Что выбрать

| Задача | Выбор |
|---|---|
| Фоновая задача | RabbitMQ, JMS Queue, ActiveMQ |
| Java EE messaging | JMS + ActiveMQ/Artemis/IBM MQ |
| Поток событий для многих систем | Kafka |
| CDC и аналитика | Kafka |
| IoT telemetry | MQTT |
| WebSocket notifications | STOMP over WebSocket |
| Сложный бизнес-процесс с людьми | BPMS/Camunda |
| Простая задача раз в день | cron или Spring `@Scheduled` |
| Сложное persistent расписание | Quartz |

## Главные ошибки на экзамене

| Ошибка | Правильно |
|---|---|
| "JMS - это брокер" | JMS - это Java API |
| "Kafka - обычная очередь" | Kafka - distributed log |
| "RabbitMQ и Kafka одинаковые" | RabbitMQ - broker/routing, Kafka - event log |
| "Async всегда лучше" | async усложняет consistency и debugging |
| "Cron и Quartz одно и то же" | cron - ОС, Quartz - Java scheduler |
| "Cockpit нужен для рисования BPMN" | BPMN рисуют в Modeler, Cockpit нужен для мониторинга |
| "Camunda 7 и 8 одинаковые" | Camunda 7 DB/Java engine, Camunda 8 Zeebe/distributed engine |

---

# 13. Источники для сверки

- Jakarta Messaging 3.1 Specification: <https://jakarta.ee/specifications/messaging/3.1/>
- RabbitMQ AMQP Concepts: <https://www.rabbitmq.com/tutorials/amqp-concepts>
- RabbitMQ Queues: <https://www.rabbitmq.com/docs/4.2/queues>
- Apache Kafka KRaft: <https://kafka.apache.org/40/operations/kraft/>
- ActiveMQ Artemis Core Architecture: <https://activemq.apache.org/components/artemis/documentation/latest/architecture.html>
- Spring Task Execution and Scheduling: <https://docs.spring.io/spring-framework/reference/integration/scheduling.html>
- Quartz Scheduler Documentation: <https://www.quartz-scheduler.org/documentation/>
- Camunda 7 Transaction Handling: <https://docs.camunda.io/docs/8.6/components/best-practices/development/understanding-transaction-handling-c7/>
- Camunda 8 Zeebe Technical Concepts: <https://docs.camunda.io/docs/components/zeebe/technical-concepts/technical-concepts-overview/>

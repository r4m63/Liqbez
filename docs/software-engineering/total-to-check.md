# Software Engineering: ключевые разделы и их этапы

Ниже — поэтапное содержание для трёх основных разделов Software Engineering.

---

## 1. Requirements Engineering

1. **Инициация сбора требований**  
   1.1. Определение целей и контекста проекта  
   1.2. Идентификация стейкхолдеров  
   1.3. Сбор исходных документов и бизнес-описаний

2. **Выявление требований (Elicitation)**  
   2.1. Интервью с заказчиком и пользователями  
   2.2. Анкеты, опросы, фокус-группы  
   2.3. Наблюдение за текущими процессами (shadowing)  
   2.4. Workshop / JAD-сессии

3. **Анализ и категоризация**  
   3.1. Разделение на функциональные (FR) и нефункциональные (NFR) требования  
   3.2. Приоритизация (MoSCoW, Kano)  
   3.3. Уточнение бизнес-правил и ограничений

4. **Моделирование требований**  
   4.1. Use Case-диаграммы (UML)  
   4.2. User Story Mapping / Story Board  
   4.3. BPMN / Activity Diagrams

5. **Документирование в SRS**  
   5.1. Введение и Scope  
   5.2. FR: описание сценариев с уникальными ID  
   5.3. NFR: производительность, безопасность, масштабируемость  
   5.4. Интерфейсы: UI, API, Hardware, Network  
   5.5. Бизнес-правила и допущения

6. **Валидация и согласование**  
   6.1. Review-сессии с заказчиком  
   6.2. Прототипы / Mock-up  
   6.3. Финальное утверждение SRS

7. **Управление требованиями**  
   7.1. Версионирование SRS  
   7.2. Обработка Change Requests  
   7.3. Traceability Matrix (связь требований → дизайн → тесты)

---

## 2. System Design

1. **Подготовка и входные данные**  
   1.1. Утверждённые FR/NFR из SRS  
   1.2. Технические ограничения и предпосылки  
   1.3. KPIs: RPS/QPS, latency, SLA

2. **High-Level Architecture**  
   2.1. Выбор стиля: монолит ↔ микросервисы ↔ SOA  
   2.2. Слоистая архитектура (Presentation, Business, Data)  
   2.3. C4 Model: Context & Container Diagrams  
   2.4. Разграничение Bounded Contexts (DDD)

3. **Компоненты и интеграция**  
   3.1. API Gateway, Load Balancer  
   3.2. Базы данных: SQL vs NoSQL, репликация, шардирование  
   3.3. Кэш (Redis/Memcached), CDN  
   3.4. Messaging (Kafka, RabbitMQ)

4. **Нефункциональные решения**  
   4.1. Масштабируемость: горизонтальная vs вертикальная  
   4.2. Отказоустойчивость: Health Checks, Failover, Replicas  
   4.3. Безопасность: TLS, OAuth2/JWT, WAF  
   4.4. Производительность: кеширование, CDN, тонкие клиенты

5. **Detailed Design**  
   5.1. UML-диаграммы классов и последовательностей  
   5.2. ER-диаграмма базы данных, индексы  
   5.3. Проектирование API (REST/GraphQL/gRPC)  
   5.4. Стратегии кеширования и обработки ошибок (retry, circuit breaker)

6. **Инфраструктура и деплой**  
   6.1. Docker-контейнеризация, Dockerfile  
   6.2. Оркестрация: Kubernetes / Docker Swarm  
   6.3. CI/CD pipeline: build → test → deploy → rollback  
   6.4. Мониторинг и логирование (Prometheus, ELK, Grafana)

7. **Верификация дизайна**  
   7.1. PoC / Spike solutions для критичных компонентов  
   7.2. Архитектурные ревью (ADR)  
   7.3. Нагрузочное и security-тестирование

---

## 3. Project Management

1. **Инициация проекта**  
   1.1. Project Charter (цели, обоснование)  
   1.2. Определение стейкхолдеров  
   1.3. Ключевые риски и допущения

2. **Планирование**  
   2.1. Scope Definition & WBS  
   2.2. Оценка ресурсов и сроков (PERT, Delphi, Expert Judgment)  
   2.3. График: Gantt Chart, Critical Path Method  
   2.4. Бюджет и план закупок  
   2.5. Risk Management Plan  
   2.6. Communication Plan

3. **Исполнение**  
   3.1. Team Formation & Roles (PO, PM, Dev Team)  
   3.2. Управление ресурсами и задачами (Task Tracking)  
   3.3. Code Review & QA практики  
   3.4. Управление изменениями (Change Requests)

4. **Мониторинг & контроль**  
   4.1. Earned Value Management (EV, PV, AC, SPI, CPI)  
   4.2. Status Reporting (Dashboards, Status Reports)  
   4.3. Управление рисками (Risk Register обновления)  
   4.4. Quality Control (тестирование, инспекции)

5. **Закрытие проекта**  
   5.1. Acceptance Testing и передача продукта  
   5.2. Lessons Learned / Post-mortem  
   5.3. Архивация документации  
   5.4. Закрытие контрактов и освобождение ресурсов

---

> **Примечание:** каждый из пунктов можно развить в отдельный Markdown-файл-шпаргалку, если нужна более детальная
> проработка.


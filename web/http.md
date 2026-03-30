# HTTP

## Способы передачи данных c клиента на сервер:

- Query (Get-параметры)
    ```
    /book?author=name
    ```
- Body запроса (Json, XML)
    ```
    {"author"="name", "title"="book1"}
    ```
- Заголовки
- Cookies

## Способы передачи данных c сервера на клиент:

- Body ответа (Json, XML)
- Status Code
- Заголовки
- Cookies

## HTTP Body Types

Часто используемые:

| Тип                                                           | Где и зачем применяется                                                                                                            |
| ------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| **`application/json`**                                        | Основной формат обмена данными в REST-, GraphQL- и большинстве webhook-API. Читается нативно во фронтенд-фреймворках и на сервере. |
| **`text/html`**                                               | “Язык” веб-страниц. Каждый браузер по умолчанию ожидает HTML при переходе по URL.                                                  |
| **`application/x-www-form-urlencoded`**                       | Данные простых HTML-форм (`key=value&…`) — поиск, логин, фильтры. Разбирается любым бэкенд-фреймворком без доп. настроек.          |
| **`multipart/form-data`**                                     | Загрузка файлов через `<form>` (аватары, документы). Содержит текстовые поля и двоичные части в одном запросе.                     |
| **`text/css`**                                                | Таблицы стилей. Без него не будет оформления страниц; кешируется агрессивно, поэтому критичен для производительности.              |
| **`application/javascript`** *(или устар. `text/javascript`)* | Скрипты клиентской логики: React, Vue, аналитика. На него завязаны SPA и интерактивность.                                          |
| **`image/png`, `image/jpeg`, `image/webp`**                   | Основные форматы растровых картинок (лого, фото, иконки). Их суммарный вес — львиная доля трафика сайтов.                          |

[Полный список](./http-body-types.txt)

## Коды ответов http

| Код | Фраза                 | Для чего нужен                                        |
| --- | --------------------- | ----------------------------------------------------- |
| 100 | Continue              | Клиент может продолжать тело запроса                  |
| 101 | Switching Protocols   | Сервер переходит на другой протокол (WebSocket и др.) |
| 102 | Processing *(WebDAV)* | Долгая операция, ещё выполняется                      |
| 103 | Early Hints           | Заголовки-подсказки (`Link`) до окончательного ответа |

| Код | Фраза                         | Назначение                                 |
| --- | ----------------------------- | ------------------------------------------ |
| 200 | OK                            | Запрос успешен                             |
| 201 | Created                       | Создан новый ресурс                        |
| 202 | Accepted                      | Принят в обработку, результата пока нет    |
| 203 | Non-Authoritative Information | Прокси изменил исходный ответ              |
| 204 | No Content                    | Тело отсутствует, но всё ОК                |
| 205 | Reset Content                 | Очистить формы на клиенте                  |
| 206 | Partial Content               | Фрагмент (Range)                           |
| 207 | Multi-Status *(WebDAV)*       | Множественные результаты по коллекции      |
| 208 | Already Reported *(WebDAV)*   | Узел уже был отражён ранее                 |
| 226 | IM Used                       | Ответ содержит delta-обновление (RFC 3229) |

| Код | Фраза              | Что делает                                           |
| --- | ------------------ | ---------------------------------------------------- |
| 300 | Multiple Choices   | Несколько вариантов                                  |
| 301 | Moved Permanently  | Постоянный редирект                                  |
| 302 | Found              | Временный редирект (исторически «Moved Temporarily») |
| 303 | See Other          | Перейти по другому URI (POST→GET)                    |
| 304 | Not Modified       | Кэш актуален (`ETag`/`Last-Modified`)                |
| 305 | Use Proxy          | Депрецировано, требовался прокси                     |
| 306 | (Unused)           | Зарезервировано                                      |
| 307 | Temporary Redirect | Временный, сохраняет метод                           |
| 308 | Permanent Redirect | Постоянный, сохраняет метод                          |

| Код | Фраза                                     | Когда возникает                              |
| --- | ----------------------------------------- | -------------------------------------------- |
| 400 | Bad Request                               | Синтаксическая ошибка                        |
| 401 | Unauthorized                              | Требуется аутентификация                     |
| 402 | Payment Required                          | Зарезервирован (часто для квот/платежей)     |
| 403 | Forbidden                                 | Доступ запрещён                              |
| 404 | Not Found                                 | Ресурс не найден                             |
| 405 | Method Not Allowed                        | Метод не поддерживается                      |
| 406 | Not Acceptable                            | Нет представления, подходящего под `Accept`  |
| 407 | Proxy Authentication Required             | Нужна auth к прокси                          |
| 408 | Request Timeout                           | Клиент слишком медленный / idle              |
| 409 | Conflict                                  | Конфликт состояния ресурса                   |
| 410 | Gone                                      | Ресурс удалён без возврата                   |
| 411 | Length Required                           | Нет `Content-Length`, а он обязателен        |
| 412 | Precondition Failed                       | Нарушены `If-*` условия                      |
| 413 | Payload Too Large                         | Тело превышает лимит                         |
| 414 | URI Too Long                              | URL непропорционально велик                  |
| 415 | Unsupported Media Type                    | Неподдерживаемый `Content-Type`              |
| 416 | Range Not Satisfiable                     | Диапазон вне ресурса                         |
| 417 | Expectation Failed                        | `Expect: 100-continue` не выполнено          |
| 418 | I'm a teapot Δ                            | Пасхалка «RFC 2324»                          |
| 421 | Misdirected Request                       | Соединение к другому хосту                   |
| 422 | Unprocessable Content *(WebDAV/RFC 9110)* | Семантически неверный JSON/XML               |
| 423 | Locked *(WebDAV)*                         | Ресурс заблокирован                          |
| 424 | Failed Dependency *(WebDAV)*              | Предыдущее действие провалилось              |
| 425 | Too Early                                 | Повторная отправка небезопасна (HTTP Replay) |
| 426 | Upgrade Required                          | Перейдите на другой протокол                 |
| 428 | Precondition Required                     | Требуются `If-Match`/`If-Unmodified-Since`   |
| 429 | Too Many Requests                         | Лимит запросов (rate-limit)                  |
| 431 | Request Header Fields Too Large           | Слишком большие заголовки                    |
| 451 | Unavailable For Legal Reasons             | Блокировка по закону (DMCA, суд)             |

| Код | Фраза                           | Когда применяется                           |
| --- | ------------------------------- | ------------------------------------------- |
| 500 | Internal Server Error           | Общее «что-то сломалось»                    |
| 501 | Not Implemented                 | Метод не поддержан                          |
| 502 | Bad Gateway                     | Промежуточный сервер получил неверный ответ |
| 503 | Service Unavailable             | Сервис временно недоступен (обслуживание)   |
| 504 | Gateway Timeout                 | Апстрим не ответил вовремя                  |
| 505 | HTTP Version Not Supported      | Версия протокола не поддерживается          |
| 506 | Variant Also Negotiates         | Ошибка content-negotiation                  |
| 507 | Insufficient Storage *(WebDAV)* | Недостаточно места                          |
| 508 | Loop Detected *(WebDAV)*        | В WebDAV-дереве рекурсия                    |
| 510 | Not Extended                    | Требуются доп. расширения метода            |
| 511 | Network Authentication Required | Портал авторизации (Wi-Fi captive)          |

## URI vs URL

| Аспект            | URI                                                                        | URL                                                                            |
| ----------------- | -------------------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| **Цель**          | Однозначно идентифицировать ресурс в абстрактном пространстве имён.        | Указать сетевой механизм доступа + расположение ресурса.                       |
| **Схема**         | Любая, в т. ч. нестандартная (`urn:`, `tel:`, `data:`).                    | Сетевые протоколы (`http:`, `https:`, `ftp:`, `ws:` …).                        |
| **Локатор?**      | Может, но не обязан. Пример без локации — URN.                             | Всегда содержит информацию «где» и «как».                                      |
| **Компоненты**    | `scheme:[//authority]path[?query][#fragment]` — но authority необязателен. | Обязательны `scheme://authority/path` (query и fragment опциональны).          |
| **Примеры**       | `urn:isbn:9780306406157` (книга) <br>`tel:+49-30-1234567` (номер телефона) | `ftp://ftp.kernel.org/pub/linux/README` <br>`https://github.com/user/repo.git` |
| **Использование** | XML-Namespaces, RDF, MIME-types, SIP-идентификаторы.                       | Ссылки в HTML, API-эндпоинты, загрузка файлов.                                 |
| **Стандарт**      | RFC 3986 (общие правила), RFC 8141 (URN).                                  | Тот же RFC 3986 + уточнения в отдельных RFC для схем (`http`, `ftp` и т. д.).  |

## HTTP Кэширование (заголовки ответа - Cache-Control, Expires, ETag)


## Типы body

Content-Type:

Самые популярные:
application/json
multipart/form-data
application/x-www-form-urlencoded
text/plain

-------------------
application/json - Обычный JSON
application/ld+json - JSON-LD (Linked Data)
application/schema+json - JSON Schema
application/geo+json - GeoJSON (для геоданных)
application/csp-report - JSON-отчеты о Content Security Policy

application/xml - Обычный XML
application/rss+xml - RSS-ленты
application/atom+xml - Atom-фид
application/mathml+xml - MathML (математические формулы)
application/svg+xml - SVG (графика в формате XML)
application/xslt+xml - XSLT (трансформация XML)
application/xhtml+xml - XHTML

text/plain - Обычный текст
text/csv - Табличные данные (CSV)
text/tab-separated-values - Табличные данные с табуляцией
text/html - HTML-страницы
text/css - CSS-файлы
text/javascript - JS-код (редко используется)
text/markdown - Markdown

multipart/form-data - Чаще всего используется для загрузки файлов
multipart/mixed - Несколько частей разных типов (например, JSON + файл)
multipart/alternative - Альтернативные версии одного контента (HTML + текст)
multipart/related - Несколько взаимосвязанных частей (например, HTML + изображения)
multipart/signed - Подписанный контент (с цифровой подписью)
multipart/encrypted - Зашифрованные данные

application/x-www-form-urlencoded - Данные в виде key=value&key2=value2

application/octet-stream - Бинарные данные (неизвестный формат)
application/pdf - PDF-документы
application/zip - ZIP-архивы
application/gzip - GZIP-архивы
application/x-bzip2 - BZIP2-архивы
application/x-tar - TAR-архивы
application/x-7z-compressed - 7Z-архивы
application/x-rar-compressed - RAR-архивы
application/vnd.android.package-archive - APK-файл (Android)
application/msword - Документы Word (.doc)
application/vnd.openxmlformats-officedocument.wordprocessingml.document - DOCX
application/vnd.ms-excel - Файлы Excel (.xls)
application/vnd.openxmlformats-officedocument.spreadsheetml.sheet - XLSX
application/vnd.ms-powerpoint - Презентации PowerPoint (.ppt)
application/vnd.openxmlformats-officedocument.presentationml.presentation - PPTX

image/png - PNG
image/jpeg - JPEG
image/gif - GIF
image/webp - WebP
image/svg+xml - SVG
image/bmp - BMP
image/tiff - TIFF
image/x-icon - ICO (фавиконка)

audio/mpeg - MP3
audio/wav - WAV
audio/ogg - OGG
audio/flac - FLAC
audio/aac - AAC

video/mp4 - MP4
video/webm - WebM
video/ogg - Ogg Video
video/x-msvideo - AVI
video/x-matroska - MKV

application/graphql - GraphQL-запросы
application/x-ndjson - JSON-поток (Newline Delimited JSON)
application/vnd.api+json - JSON:API (стандарт для REST API)
application/problem+json - Ошибки в формате JSON
application/problem+xml - Ошибки в формате XML
application/x-pem-file - PEM-файл (ключи SSL)
application/x-font-ttf - TTF-шрифты
application/x-font-woff - WOFF-шрифты
application/x-font-woff2 - WOFF2-шрифты
application/x-httpd-php - PHP-код
application/vnd.apple.installer+xml - macOS Installer

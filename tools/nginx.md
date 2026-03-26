# Nginx

## Установка

```bash
sudo dnf install -y nginx
sudo systemctl enable --now nginx
sudo systemctl status nginx
```

## Конфиги

`/etc/nginx/sites-available` -- доступные конфиги

`/etc/nginx/sites-enabled/` -- активированные конфиги

или:

`/etc/nginx/conf.d` -- для rockylinux

Пример `/etc/nginx/nginx.conf`
```bash
user nginx;                         # От какого Unix-пользователя запускать worker-процессы
worker_processes auto;              # Кол-во worker'ов; auto = по числу CPU
pid /run/nginx.pid;                 # PID-файл master-процесса
error_log /var/log/nginx/error.log warn;  # Глобальный лог ошибок и минимальный уровень

# Подключение динамических модулей, если дистрибутив их так ставит
include /usr/share/nginx/modules/*.conf;

events {
    worker_connections 1024;        # Сколько соединений может держать один worker
    multi_accept on;                # Принимать сразу несколько новых соединений
    use epoll;                      # Метод обработки соединений на Linux
}

http {
    include       /etc/nginx/mime.types;    # MIME-типы файлов
    default_type  application/octet-stream; # Тип по умолчанию, если не распознан

    # Формат access log
    log_format main
        '$remote_addr - $remote_user [$time_local] "$request" '
        '$status $body_bytes_sent "$http_referer" '
        '"$http_user_agent" "$http_x_forwarded_for"';

    access_log /var/log/nginx/access.log main;   # Основной access log
    error_log  /var/log/nginx/error.log warn;    # Лог ошибок на уровне http

    sendfile on;                     # Быстрая отдача файлов через sendfile()
    tcp_nopush on;                   # Оптимизация отправки больших ответов
    tcp_nodelay on;                  # Уменьшение задержек для keepalive/малых пакетов
    keepalive_timeout 65;            # Время удержания keepalive-соединения
    keepalive_requests 1000;         # Сколько запросов разрешить в одном keepalive-соединении
    types_hash_max_size 4096;        # Размер hash-таблицы MIME-типов

    server_tokens off;               # Не светить версию nginx в ответах/ошибках
    charset utf-8;                   # Кодировка по умолчанию, если нужна

    client_max_body_size 20m;        # Максимальный размер тела запроса (upload)
    client_body_buffer_size 128k;    # Буфер для тела запроса
    client_header_buffer_size 1k;    # Буфер под request headers
    large_client_header_buffers 4 16k; # Крупные заголовки/длинные cookie

    reset_timedout_connection on;    # Жестче закрывать таймаутные соединения
    send_timeout 60s;                # Таймаут отправки ответа клиенту
    client_body_timeout 60s;         # Таймаут чтения body
    client_header_timeout 60s;       # Таймаут чтения заголовков

    # Gzip-сжатие
    gzip on;                         # Включить gzip
    gzip_comp_level 5;               # Уровень сжатия
    gzip_min_length 1024;            # Не сжимать слишком маленькие ответы
    gzip_proxied any;                # Сжимать и для proxy-ответов
    gzip_vary on;                    # Добавлять Vary: Accept-Encoding
    gzip_types
        text/plain
        text/css
        application/json
        application/javascript
        text/xml
        application/xml
        application/xml+rss
        text/javascript
        image/svg+xml;               # Какие типы сжимать

    # Если nginx стоит за reverse proxy / балансировщиком
    # set_real_ip_from 10.0.0.0/8;   # Доверенная сеть прокси
    # real_ip_header X-Forwarded-For;# Откуда брать реальный IP клиента
    # real_ip_recursive on;          # Корректно обрабатывать цепочку прокси

    # Rate limiting zones
    limit_req_zone $binary_remote_addr zone=req_limit_per_ip:10m rate=10r/s; # Лимит запросов
    limit_conn_zone $binary_remote_addr zone=conn_limit_per_ip:10m;           # Лимит соединений

    # Cache path пример
    # proxy_cache_path /var/cache/nginx levels=1:2 keys_zone=mycache:10m
    #                  max_size=1g inactive=60m use_temp_path=off;

    # Upstream-группа backend'ов
    upstream app_backend {
        server 127.0.0.1:3000 max_fails=3 fail_timeout=30s;  # Первый backend
        # server 127.0.0.1:3001 backup;                      # Резервный backend
        keepalive 32;                                        # Keepalive к backend'ам
    }

    # Подключить все отдельные сайты
    include /etc/nginx/conf.d/*.conf;
}
```

Пример `/etc/nginx/conf.d/example.com`
```bash
# /etc/nginx/conf.d/example.com.conf

server {
    listen 80;                               # Слушать IPv4 на 80 порту
    listen [::]:80;                          # Слушать IPv6 на 80 порту
    server_name example.com www.example.com; # Имена хоста

    access_log /var/log/nginx/example.com.access.log main; # Лог доступа сайта
    error_log  /var/log/nginx/example.com.error.log warn;  # Лог ошибок сайта

    return 301 https://example.com$request_uri; # Перенаправить HTTP -> HTTPS
}

server {
    listen 443 ssl http2;                    # HTTPS + HTTP/2
    listen [::]:443 ssl http2;               # HTTPS + HTTP/2 по IPv6
    server_name example.com www.example.com; # Домен и алиасы

    root /var/www/example.com/public;        # Корень сайта
    index index.html index.htm index.php;    # Индексные файлы

    access_log /var/log/nginx/example.com.access.log main; # Access log
    error_log  /var/log/nginx/example.com.error.log warn;  # Error log

    # TLS / SSL
    ssl_certificate     /etc/letsencrypt/live/example.com/fullchain.pem; # Сертификат
    ssl_certificate_key /etc/letsencrypt/live/example.com/privkey.pem;   # Приватный ключ
    ssl_session_timeout 1d;                  # Время жизни SSL-сессии
    ssl_session_cache shared:SSL:10m;        # Кэш SSL-сессий
    ssl_session_tickets off;                 # Обычно выключают для безопасности
    ssl_protocols TLSv1.2 TLSv1.3;           # Разрешенные версии TLS
    ssl_prefer_server_ciphers off;           # Для современных настроек обычно off

    # HSTS включай только когда точно уверен в HTTPS
    add_header Strict-Transport-Security "max-age=31536000; includeSubDomains" always;

    # Базовые security headers
    add_header X-Frame-Options "SAMEORIGIN" always;          # Защита от clickjacking
    add_header X-Content-Type-Options "nosniff" always;      # Не угадывать MIME
    add_header Referrer-Policy "strict-origin-when-cross-origin" always;

    # Размер загрузок
    client_max_body_size 20m;                # Ограничение upload

    # Если сайт статический
    location / {
        try_files $uri $uri/ /index.html;    # Сначала файл/каталог, иначе index.html
    }

    # Кэширование статических файлов
    location ~* \.(css|js|jpg|jpeg|png|gif|ico|svg|webp|woff|woff2)$ {
        expires 30d;                         # Кэш на 30 дней
        access_log off;                      # Не логировать каждый статический файл
        add_header Cache-Control "public, max-age=2592000, immutable";
    }

    # Скрыть чувствительные файлы
    location ~ /\.(?!well-known).* {
        deny all;                            # Запретить доступ к dotfiles
    }

    # Пример ACME challenge под certbot
    location ^~ /.well-known/acme-challenge/ {
        root /var/www/letsencrypt;           # Где лежат challenge-файлы
        default_type "text/plain";
    }

    # Пример proxy на backend-приложение
    location /app/ {
        proxy_pass http://app_backend/;      # Передать запрос в upstream
        proxy_http_version 1.1;              # HTTP/1.1 к backend
        proxy_set_header Host $host;         # Передать исходный Host
        proxy_set_header X-Real-IP $remote_addr; # Реальный IP клиента
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for; # Цепочка proxy IP
        proxy_set_header X-Forwarded-Proto $scheme; # http/https
        proxy_set_header Connection "";      # Для keepalive/proxy
        proxy_read_timeout 60s;              # Таймаут чтения от backend
        proxy_connect_timeout 5s;            # Таймаут коннекта к backend
        proxy_send_timeout 60s;              # Таймаут отправки в backend

        # limit_req zone=req_limit_per_ip burst=20 nodelay; # Пример rate limit
        # proxy_cache mycache;                              # Пример кэша proxy
        # proxy_cache_valid 200 10m;                       # Кэш 200-ответов
    }

    # Кастомные страницы ошибок
    error_page 404 /404.html;                # Страница 404
    location = /404.html {
        internal;                            # Нельзя открыть напрямую извне
    }

    error_page 500 502 503 504 /50x.html;    # Страница ошибок backend/сервера
    location = /50x.html {
        internal;
    }
}
```

для debian-based:
```bash
sudo ln -s /etc/nginx/sites-available/example.com /etc/nginx/sites-enabled/example.com
sudo nginx -t
sudo systemctl reload nginx
```

## Сертификаты

```bash
sudo dnf install -y epel-release
sudo dnf install -y certbot python3-certbot-nginx
sudo certbot --nginx -d example.com
sudo systemctl enable --now certbot-renew.timer
```

## Load Balancer

```bash
http {
    upstream api_backend {
        server 127.0.0.1:8001 max_fails=3 fail_timeout=30s; # backend 1
        server 127.0.0.1:8002 max_fails=3 fail_timeout=30s; # backend 2
        keepalive 32;                                       # keepalive к backend
    }

    server {
        listen 80;
        server_name api.example.com;

        location / {
            proxy_pass http://api_backend;                  # отправить в upstream
            proxy_http_version 1.1;                         # HTTP/1.1 к backend
            proxy_set_header Host $host;                    # передать Host
            proxy_set_header X-Real-IP $remote_addr;        # IP клиента
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for; # цепочка IP
            proxy_set_header X-Forwarded-Proto $scheme;     # http/https
            proxy_set_header Connection "";                 # нормально для keepalive
            proxy_connect_timeout 5s;                       # таймаут соединения к backend
            proxy_read_timeout 60s;                         # таймаут чтения ответа
            proxy_send_timeout 60s;                         # таймаут отправки запроса
        }
    }
}
```

round robin -- По умолчанию

```bash
upstream api_backend {
    server 10.0.0.11:8080;
    server 10.0.0.12:8080;
}
```

least_conn -- Подходит, если запросы сильно отличаются по длительности.

```bash
upstream api_backend {
    least_conn;             # меньше активных соединений -> выше шанс получить запрос
    server 10.0.0.11:8080;
    server 10.0.0.12:8080;
}
```

ip_hash -- Подходит, если нужна “липкость” по IP клиента.

```bash
upstream api_backend {
    ip_hash;                # один клиентский IP старается ходить в один backend
    server 10.0.0.11:8080;
    server 10.0.0.12:8080;
}
```

hash -- Подходит, если маршрутизация должна зависеть от ключа.

```bash
upstream api_backend {
    hash $request_uri consistent;       # одинаковый URI -> один и тот же backend
    server 10.0.0.11:8080;
    server 10.0.0.12:8080;
}
```

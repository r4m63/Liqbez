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
user  www-data;
worker_processes  auto;
pid /run/nginx.pid;

# Модули из пакета nginx-common (Ubuntu)
include /etc/nginx/modules-enabled/*.conf;

events {
    worker_connections  4096;
    multi_accept        on;
}

http {
    ##
    # Базовые настройки
    ##
    sendfile            on;
    tcp_nopush          on;
    tcp_nodelay         on;
    keepalive_timeout   65s;
    types_hash_max_size 2048;
    server_tokens       off;        # не светим версию nginx
    client_max_body_size 50m;       # увеличьте при необходимости загрузок

    include             /etc/nginx/mime.types;
    default_type        application/octet-stream;

    ##
    # Логи (удобный JSON-формат)
    ##
    log_format main_json escape=json
      '{ "time":"$time_iso8601",'
      ' "remote_addr":"$remote_addr",'
      ' "request":"$request",'
      ' "status":$status,'
      ' "body_bytes_sent":$body_bytes_sent,'
      ' "request_time":$request_time,'
      ' "upstream_response_time":"$upstream_response_time",'
      ' "method":"$request_method",'
      ' "host":"$host",'
      ' "uri":"$uri",'
      ' "referer":"$http_referer",'
      ' "agent":"$http_user_agent" }';

    access_log  /var/log/nginx/access.log  main_json;
    error_log   /var/log/nginx/error.log   warn;

    ##
    # Сжатие
    ##
    gzip on;
    gzip_comp_level 5;
    gzip_min_length 1024;
    gzip_vary on;
    gzip_proxied any;
    gzip_types
      text/plain text/css text/xml application/xml application/json
      application/javascript application/x-javascript application/rss+xml
      image/svg+xml;

    ##
    # Прокси-дефолты + WebSocket upgrade
    ##
    map $http_upgrade $connection_upgrade {
        default upgrade;
        ''      close;
    }
    proxy_http_version          1.1;
    proxy_set_header            Host              $host;
    proxy_set_header            X-Real-IP         $remote_addr;
    proxy_set_header            X-Forwarded-For   $proxy_add_x_forwarded_for;
    proxy_set_header            X-Forwarded-Proto $scheme;
    proxy_set_header            Upgrade           $http_upgrade;
    proxy_set_header            Connection        $connection_upgrade;
    proxy_connect_timeout       5s;
    proxy_send_timeout          60s;
    proxy_read_timeout          60s;

    ##
    # Пример лимитов (можно применить на /api)
    ##
    limit_req_zone $binary_remote_addr zone=api_ratelimit:10m rate=10r/s;

    ##
    # Включаем конфиги сайтов/доп.конфигов
    ##
    include /etc/nginx/conf.d/*.conf;
    include /etc/nginx/sites-enabled/*;
}

```

Пример `/etc/nginx/conf.d/example.com`
```bash
server {
    	listen 80;
        server_name example.com;
        location / {
                proxy_pass http://127.0.0.1:1337;
                proxy_set_header Host $host;
                proxy_set_header X-Real-IP $remote_addr;
                proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
                proxy_set_header X-Forwarded-Proto $scheme;
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





----

worker_processes auto;

events {
        worker_connections 1024;
}

http {
        client_max_body_size 10G;
        types_hash_max_size 2048;
        types_hash_bucket_size 128;
        include       mime.types;
        default_type  application/octet-stream;

server {
        listen 80;
        server_name aze-umma.ru;
        location / {
                proxy_pass http://127.0.0.1:45001;
                proxy_set_header Host $host;
                proxy_set_header X-Real-IP $remote_addr;
                proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
                proxy_set_header X-Forwarded-Proto $scheme;
        }
}


server {
        server_name cloud.ramil21.ru;
        location / {
                proxy_pass http://127.0.0.1:21103;
                proxy_set_header Host $host;
                proxy_set_header X-Real-IP $remote_addr;
                proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
                proxy_set_header X-Forwarded-Proto $scheme;
        }

        listen 443 ssl; # managed by Certbot
        ssl_certificate /etc/letsencrypt/live/cloud.ramil21.ru/fullchain.pem; # managed by Certbot
        ssl_certificate_key /etc/letsencrypt/live/cloud.ramil21.ru/privkey.pem; # managed by Certbot
        include /etc/letsencrypt/options-ssl-nginx.conf; # managed by Certbot
        ssl_dhparam /etc/letsencrypt/ssl-dhparams.pem; # managed by Certbot
}

# Убираем лишний default сервер
server {
        listen 80 default_server;
        server_name _;
        return 403;
}


server {
    if ($host = cloud.ramil21.ru) {
        return 301 https://$host$request_uri;
    } # managed by Certbot


        listen 80;
        server_name cloud.ramil21.ru;
    return 404; # managed by Certbot

}


}

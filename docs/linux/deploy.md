




# Деплой сервисов в linux

## Nextcloud

### Docker deploy

#### docker-compose.yml

```
services:
  nextcloud:
    image: nextcloud:28.0
    container_name: nextcloud
    restart: unless-stopped
    ports:
      - "21103:80"
    volumes:
      - /mnt/vortex/nextcloud/config:/var/www/html/config
      - /mnt/vortex/nextcloud/data:/var/www/html/data
      - /mnt/vortex/nextcloud/apps:/var/www/html/custom_apps
      - /mnt/vortex/nextcloud/themes:/var/www/html/themes
    environment:
      - POSTGRES_HOST=nextcloud_db
      - POSTGRES_DB=nextcloud
      - POSTGRES_USER=nextcloud
      - POSTGRES_PASSWORD=210605
      - NEXTCLOUD_ADMIN_USER=ramil
      - NEXTCLOUD_ADMIN_PASSWORD=210605
    depends_on:
      - nextcloud_db
    networks:
      - nextcloud_network

  nextcloud_db:
    image: postgres:16.3
    container_name: nextcloud_db
    restart: unless-stopped
    volumes:
      - /mnt/vortex/nextcloud/postgres:/var/lib/postgresql/data
    environment:
      - POSTGRES_DB=nextcloud
      - POSTGRES_USER=nextcloud
      - POSTGRES_PASSWORD=210605
    networks:
      - nextcloud_network

networks:
  nextcloud_network:
    name: nextcloud_network
```

Можно указать в "127.0.0.1:8080:80" чтобы иметь доступ только в localhost

```
ports:
    - "127.0.0.1:8080:80"
```

Можно указать Trusted domains сразу через env

```
environment:
    - NEXTCLOUD_TRUSTED_DOMAINS=ваш-домен.com
    - NEXTCLOUD_TRUSTED_DOMAINS=ваш_домен.com,localhost,192.168.xxx.xxx
```

Запуск контейнера:

```
docker compose up -d --force-recreate
```

Добавить доверенные доменны для деплоя:

Добавить в `vim /mnt/vortex/nextcloud/config/config.php`

```
'trusted_domains' =>
  array (
    0 => 'cloud.ramil21.ru',
    1 => 'localhost',
    2 => '45.93.5.140',
  ),
```

После изменений в volumes - `docker restart <container_name>`

#### SSL сертификаты

Добавить в `vim /mnt/vortex/nextcloud/config/config.php`

```
'overwrite.cli.url' => 'https://example-domain.ru',
'overwriteprotocol' => 'https',
```

Добавить nginx конфиг стандартный сначала без ssl сертификатов

```
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
```

`sudo dnf install certbot python3-certbot-nginx` - для fedora dnf install certbot

`sudo pacman -S certbot certbot-nginx` - для archlinux

Выдать сертификат

`sudo certbot --nginx`

`sudo certbot --nginx -d cloud.domain-example.ru`

Проверка

`openssl s_client -connect cloud.ramil21.ru:443`

`sudo ss -tulnp | grep 9090`

После выдачи ssl сертификата на сайт конфиг поменяется на

```
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
server {
    if ($host = cloud.ramil21.ru) {
        return 301 https://$host$request_uri;
    } # managed by Certbot
        listen 80;
        server_name cloud.ramil21.ru;
    return 404; # managed by Certbot
}
```

### Ошибка загрузки больших данных - ограничение nginx body

```
http {
    client_max_body_size 10G;
    types_hash_max_size 2048;
    types_hash_bucket_size 128;
    ...
```

## Gitlab

## Wireguard

### Docker deploy

#### docker-compose.yml

Задать хэшированный пароль

```
docker run --rm -it ghcr.io/wg-easy/wg-easy wgpw '210605'
```

```
docker run --detach \
--name wg-easy \
--env LANG=ru \
--env WG_HOST=45.93.5.140 \
--env PASSWORD_HASH='$2a$12$K/hAkEbJhkE2HFtpVQ.Ly.YofSCJCmkCg6Vuu/YbFt653h1X1VQTS' \
--env PORT=51821 \
--env WG_PORT=51820 \
--volume ~/.wg-easy:/etc/wireguard \
--publish 51820:51820/udp \
--publish 51821:51821/tcp \
--cap-add NET_ADMIN \
--cap-add SYS_MODULE \
--sysctl 'net.ipv4.conf.all.src_valid_mark=1' \
--sysctl 'net.ipv4.ip_forward=1' \
--restart unless-stopped \
ghcr.io/wg-easy/wg-easy
```

Включить IP forwarding на хосте напрямую:

```
sudo sysctl -w net.ipv4.ip_forward=1
sudo sysctl -w net.ipv6.conf.all.forwarding=1
```

Либо добавить в `/etc/sysctl.conf`:

```
net.ipv4.ip_forward=1
net.ipv6.conf.all.forwarding=1
```

Чтобы получить доступ к админке роутера, на клиенте в конфиг вручную надо добавить в AllowedIPs:

```
AllowedIPs = 0.0.0.0/0, ::/0, 192.168.0.0/24
```

НО, иногда не работает, и придется прокидывать порт через ssh -L

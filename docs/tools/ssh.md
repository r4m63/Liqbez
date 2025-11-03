# SSH

## Конфиги в ubuntu

## Создание ключей

Сгенерировать ключ (если ещё нет)

```bash
ssh-keygen -t ed25519 -C "your_comment"
# путь по умолчанию: ~/.ssh/id_ed25519
```

Он создаст:
`~/.ssh/id_ed25519` - приватный
`~/.ssh/id_ed25519.pub` - публичный

Скопировать публичный ключ на сервер

Вариант 1:

`ssh-copy-id -i ~/.ssh/id_ed25519.pub user@server`

Вариант 2:

`cat ~/.ssh/id_ed25519.pub` - скопировать

На сервере:

```bash
mkdir -p ~/.ssh
chmod 700 ~/.ssh
echo "СЮДА_ВСТАВИТЬ_СТРОКУ_КЛЮЧА" >> ~/.ssh/authorized_keys
chmod 600 ~/.ssh/authorized_keys
```

## Алиас на подключение

в `~/.ssh/config`:

```text
Host prod
    HostName 203.0.113.10 
    User user_name 
    Port 22 
    IdentityFile ~/.ssh/id_ed25519
```

`chmod 600 ~/.ssh/config`

## Файлы в ~/.ssh

- config - Конфиг SSH-клиента. Тут задаются алиасы для удобного подключения например ssh server_name.
- id_ed25519 - Приватный SSH-ключ (тип Ed25519). Хранить только у себя, никому не показывать. Права обычно 600.
- id_ed25519.pub - Публичный ключ к вышеуказанному приватному. Его как раз копируют на сервер в ~/.ssh/authorized_keys.
- known_hosts - Кэш хост-ключей серверов, к которым уже подключался.
- known_hosts.old - Бэкап предыдущей версии known_hosts
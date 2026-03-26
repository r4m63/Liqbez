# SSH

## Создание ключей

Сгенерировать на локальной машине ключ

```bash
ssh-keygen -t ed25519 -f ~/.ssh/name_key
```

Отправить ключ на сервер. Вариант 1:

`ssh-copy-id -i ~/.ssh/id_ed25519.pub user@server`

Отправить ключ на сервер. Вариант 2:

`cat ~/.ssh/id_ed25519.pub` - скопировать

На сервере выполнить:

```bash
mkdir -p ~/.ssh
chmod 700 ~/.ssh
echo "СЮДА_ВСТАВИТЬ_СТРОКУ_КЛЮЧА" >> ~/.ssh/authorized_keys
chmod 600 ~/.ssh/authorized_keys
```

---

Конфиг для алиасов на локальной машине.

В `~/.ssh/config`:

```text
Host *
  ServerAliveInterval 60
  ServerAliveCountMax 3
  TCPKeepAlive yes
  AddKeysToAgent yes
  UseKeychain yes

Host alias_name
  HostName 1.2.3.4
  Port 2222
  User user_name
  IdentityFile ~/.ssh/name_key
  IdentitiesOnly yes
```

`chmod 600 ~/.ssh/config`

UseKeychain - агент работает на macos, для других:

```
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/pg_key
ssh-add -l
```

либо в .bashrc написать скрипт

## Файлы в ~/.ssh

- config - Конфиг SSH-клиента. Тут задаются алиасы для удобного подключения например ssh server_name.
- id_ed25519 - Приватный SSH-ключ (тип Ed25519). Хранить только у себя, никому не показывать. Права обычно 600.
- id_ed25519.pub - Публичный ключ к вышеуказанному приватному. Его как раз копируют на сервер в ~/.ssh/authorized_keys.
- known_hosts - Кэш хост-ключей серверов, к которым уже подключался.
- known_hosts.old - Бэкап предыдущей версии known_hosts


## Конфигурирование ssh на сервере

SSH-доступ: кого пускаем и как

```bash
# /etc/ssh/sshd_config (минимум)
PasswordAuthentication no
PubkeyAuthentication yes
AllowGroups ssh-users
ClientAliveInterval 300
ClientAliveCountMax 2

# SFTP-только и chroot для группы sftp-alpha:
Match Group sftp-alpha
    ChrootDirectory /srv/sftp/%u
    ForceCommand internal-sftp
    X11Forwarding no
    AllowTcpForwarding no
```

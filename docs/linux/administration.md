# motd

статический MOTD
cat /etc/motd

(Все исполняемые скрипты из этой директории выполняются по порядку)
ls -l /etc/update-motd.d/

run-parts /etc/update-motd.d/
bash /etc/update-motd.d/99-harbor

sudo chmod -x /etc/update-motd.d/*
sudo chmod +x /etc/update-motd.d/99-justheader-harbor

# входы и юзеры

`who` - Кто в системе прямо сейчас
`w` - Кто в системе прямо сейчас подробнее с ip и idle
`users` - Быстрый список логинов
`last -a` - История успешных логинов (и ребутов)
`last -a | grep -E "sshd|pts/"` - только ssh сессии
`lastlog` - кто логинился последний раз
`lastlog -u ramil` - по конкретному юзеру

Неудачные попытки входа

```
sudo grep -i "failed password" /var/log/auth.log | tail -n 50
sudo grep -i "invalid user" /var/log/auth.log | tail -n 50
```

Список активных SSH-сессий из логов

```
sudo journalctl -u ssh -S today | egrep "Accepted|session opened|session closed" | tail -n 200
```

Процессы конкретного пользователя

```
ps -u ramil -o pid,tty,stat,lstart,etime,cmd --sort=-lstart | head -n 30
```

# Fail2ban

`sudo fail2ban-client status` - Какие jail’ы активны
`sudo fail2ban-client status sshd` - Статус конкретного jail (обычно sshd)

логи:

```
sudo tail -n 200 /var/log/fail2ban.log
sudo journalctl -u fail2ban -n 200 --no-pager
```

Снять бан с IP
`sudo fail2ban-client set sshd unbanip 101.91.192.9`

Посмотреть, какие IP сейчас в nft-наборе (реально забанены)

```
sudo nft list set inet f2b-table addr-set-sshd
sudo fail2ban-client status sshd | sed -n '/Banned IP list/p' - что делает команда?
```

# Создание ключей под мой sshd конфиг:

На локальной машине
`ssh-keygen -t ed25519 -a 64 -f ~/.ssh/id_ed25519_harbor -C "ramil@harbor"`

приватный ключ: ~/.ssh/id_ed25519_harbor (никому не показывать)
публичный ключ: ~/.ssh/id_ed25519_harbor.pub (его можно копировать)

на сервер:
`ssh-copy-id -i ~/.ssh/id_ed25519_harbor.pub ramil@harbor`
Он сам добавит ключ в:
/home/ramil/.ssh/authorized_keys

либо

`cat ~/.ssh/id_ed25519_harbor.pub`

На сервере:

```
mkdir -p ~/.ssh
chmod 700 ~/.ssh
nano ~/.ssh/authorized_keys
```

Вставить одной строкой публичный ключ, сохранить, затем:

`chmod 600 ~/.ssh/authorized_keys`

Настроить на локальной машине удобный вход (config)

`vim ~/.ssh/config`

```
Host harbor
  HostName harbor
  User ramil
  IdentityFile ~/.ssh/id_ed25519_harbor
  IdentitiesOnly yes
```

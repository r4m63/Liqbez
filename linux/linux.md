# Администрирование

```bash
# Показать всех пользователей из /etc/passwd
cat /etc/passwd

# Более читаемый формат
getent passwd

# Только имена пользователей
getent passwd | cut -d: -f1

# Пользователи, которые могут войти в систему (с валидной оболочкой)
getent passwd | grep -v "/nologin\|/false"

# Пользователи с UID < 1000 (обычно системные)
getent passwd | awk -F: '$3 < 1000 {print $1}'

# Пользователи с UID >= 1000
getent passwd | awk -F: '$3 >= 1000 {print $1}'

# Просмотр групп

# Показать все группы из /etc/group
cat /etc/group

# Более читаемый формат
getent group

Supported databases:
ahosts ahostsv4 ahostsv6 aliases ethers group gshadow hosts initgroups
netgroup networks passwd protocols rpc services shadow


# Только названия групп
getent group | cut -d: -f1

# Показать группы пользователя username
groups username

# Или так
id username

#

# Подробная информация
id

# Или
whoami

# Заменить username на имя пользователя
id username
finger username  # если установлен finger

# Последние входы в систему
last

# Только успешные входы
last | grep -v "still logged in\|crash"

# Текущие пользователи
who

# Более подробно
w

# Пользователи с правами sudo
getent group sudo

# Или посмотреть /etc/sudoers
sudo cat /etc/sudoers | grep -v "^#"

getent passwd | grep username

getent group | grep groupname

```

Домашние каталоги

```bash
# /etc/adduser.conf
DIR_MODE=0750
UMASK=027
# На уже существующие HOME:
chmod 750 /home/*  # или 700 для максимальной изоляции
```

Общие каталоги проектов

```bash
mkdir -p /srv/projects/alpha
chgrp proj-alpha /srv/projects/alpha
chmod 2770 /srv/projects/alpha             # setgid на каталоге
# По умолчанию новые файлы наследуют группу и права:
setfacl -d -m g:proj-alpha:rwx /srv/projects/alpha
setfacl -m  g:proj-alpha:rwx /srv/projects/alpha
# read-only доступ аналитику:
usermod -aG analysts andrew
setfacl -m g:analysts:rx /srv/projects/alpha
```

Sudo — только по нужным командам

```bash
visudo -f /etc/sudoers.d/ops

# пример: дежурные могут рестартовать сервисы и читать логи journalctl
User_Alias OPS = %ops
Cmnd_Alias SVC = /usr/bin/systemctl restart nginx, /usr/bin/systemctl restart myapp
Cmnd_Alias LOG = /usr/bin/journalctl *
OPS ALL=(root) SVC
OPS ALL=(root) LOG
# без NOPASSWD по умолчанию; включайте точечно, если надо:
# OPS ALL=(root) NOPASSWD: SVC
```

Ограничение запуска программ

```bash
mkdir -p /opt/whitelist/bin
chgrp devs /opt/whitelist/bin
chmod 750 /opt/whitelist/bin
ln -s /usr/bin/git /opt/whitelist/bin/
# Для пользователей с «белым списком» задайте PATH через /etc/profile.d/whitelist.sh:
echo 'if id -nG | grep -qw devs; then export PATH="/opt/whitelist/bin"; fi' >/etc/profile.d/whitelist.sh
```

Все будет делать CI/CD, доступ по ssh на сервер нужен только

Администраторы (1-2 человека):
Задачи: Аварийное восстановление, обновление ОС, настройка сети, мониторинг
Доступ: Полный sudo доступ
Группы: sudo, adm, ssh-users

DevOps/Инженеры развертывания:
Задачи: Настройка CI/CD, мониторинг деплоев, отладка pipeline
Доступ: Ограниченный sudo (только для сервисов приложения)
Группы: ci-admins, docker, ssh-users

Разработчики (в АВАРИЙНЫХ ситуациях):
Задачи: Отладка production-проблем, анализ логов
Доступ: Только чтение логов, без модификации кода
Группы: developers-readonly, ssh-users

Нужен CI/CD пользователь и Docker/Kubernetes

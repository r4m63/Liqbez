
Файловая система:
```txt
/
/bin			исполняемые файлы системы + файлы необходимые для работы и загрузки системы
/boot			файлы необходимые для загрузки системы + ядро + grub
/dev			файлы устройств
/etc			конфигурационные файлы
/home			домашние директории пользователей
/lib			библиотеки для работы системных утилит и приложений
/lib64		    64 битные библиотеки для работы системных утилит и приложений
/media		    стандартное место для монтирования внешних устройств хранения данных
/mnt			временное монтированные файловые системы
/opt			для установки дополнительных программного обеспечения, которое не входит в стандартные пакеты
/proc			предоставляет информацию о состоянии системы и работающих процессах
/root			домашний каталог root
/run			для хранения временных файлов, которые необходимы для работы системы и приложений
/sbin			системные исполняемые файлы, которые необходимы для администрирования и управления системой
/sys			интерфейс для взаимодействия с ядром и различными устройствами в системе
/tmp			хранение временных файлов, которые создаются различными приложениями и службами
/usr			хранение пользовательских программ и данных
/var			предназначена для хранения данных, которые могут изменяться во время работы системы
```

```txt
r — read        u — user owner
w — write       g — group
x — execute     o — others
                a — all (u+g+o)

- rwx r-x r--
| ||| ||| |||
| ||| ||| ||└─ others
| ||| └└└─── group
| └└└─────── user
└─────────── тип файла

Первый символ       3 тройки
- — обычный файл    rwx — права владельца
d — папка           r-x — права группы
l — ссылка          r-- — права остальных

Каждое право — это число
r = 4
w = 2
x = 1

7 = 4+2+1 = rwx
6 = 4+2 = rw-
5 = 4+1 = r-x
4 = 4 = r--
3 = 2+1 = -wx
2 = 2 = -w-
1 = 1 = --x
0 = ---

chmod 755 file.txt
chmod [кому][операция][права] file.txt
chmod o+x file.txt
chmod u+rw file.txt
chmod a=r file.txt
chmod u=rwx file.txt
chmod u+x,g-w,o=r file.txt
```



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

# Подробная информация
id

# Или
whoami

# Заменить username на имя пользователя
id username

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


# Диски

SATA и SCSI диски обозначаются как `sdX`, где X — это буква, представляющая диск. `sda` — первый SATA/SCSI диск, `sdb` — второй и так далее.

NVMe диски обозначаются как `nvmeXnY`, где X — номер контроллера, а Y — номер диска. `nvme0n1` — первый NVMe контроллер и первый диск на этом контроллере

```bash
lsblk		# показать все диски и разделы

sudo mkdir /mnt/usb
sudo mount /dev/sda1 /mnt/usb
sudo umount /mnt/usb

# Автоматическое монтирование:
blkid
sudo vim /etc/fstab
UUID=мой-uuid /mnt/usb ext4 defaults 0 2
sudo mount -a

расширить LVM
df -h /
sudo lvextend -r -l +100%FREE /dev/mapper/fedora-root


lsblk -f
sudo mkdir -p /mnt/example_name
sudo mount /dev/sdb1 /mnt/example_name
mount | grep example_name

sudo umount /mnt/vortex
sudo umount /dev/sdb1
```


Чтобы диск автоматически монтировался при каждом перезагрузке, нужно добавить его в файл `/etc/fstab`

`sudo blkid /dev/sdb1`

Пример вывода:

`/dev/sdb1: UUID="3e1c1a0c-57ef-47af-88a7-2195058f324b" TYPE="ext4"`

`sudo vim /etc/fstab`

Добавить строку для раздела:

`UUID=3e1c1a0c-57ef-47af-88a7-2195058f324b /mnt/vortex ext4 defaults,noatime 0 2`

- UUID — уникальный идентификатор раздела.
- /mnt/vortex — точка монтирования.
- ext4 — тип файловой системы.
- defaults — типичные параметры монтирования.
- noatime — отключение обновления времени доступа, ускоряет I/O операции.
- 0 2 — параметры для утилиты fsck (проверка файловой системы), где 0 — не проверять, 2 — проверка после корневого
  раздела.

Сохранить изменения

`sudo mount -a`

`sudo chown ramil:ramil /mnt/vortex`

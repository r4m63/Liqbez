# Cocpit

```bash
sudo pacman -Syu
sudo pacman -S cockpit
sudo systemctl enable --now cockpit.socket
```

`pacman -S networkmanager && systemctl enable --now NetworkManager`

- `sudo pacman -S cockpit-files`
- `sudo pacman -S cockpit-machines`
- `sudo pacman -S cockpit-podman`
- `sudo pacman -S cockpit-packagekit`
- `sudo pacman -S cockpit-storaged`

`systemctl restart cockpit.socket`

---

# Nginx

```
sudo pacman -Syu
sudo pacman -S nginx
sudo systemctl start nginx
sudo systemctl enable nginx
systemctl status nginx
```

# редактируем конфиги → затем

sudo nginx -t # убедиться, что «syntax is ok»
sudo systemctl reload nginx # мягкая перезагрузка без простоя

sudo nginx -t Проверяет синтаксис текущего конфигурационного файла (/etc/nginx/nginx.conf) и всех файлов, подключённых
через include.
sudo nginx -t -q То же, но выводит сообщения только об ошибках (quiet-режим).
sudo nginx -t -c /путь/к/файлу.conf Проверяет указанный альтернативный конфиг вместо дефолтного.
sudo nginx -T Dump-режим: выводит итоговый объединённый конфиг и сразу проверяет его синтаксис (удобно для отладки
include).

---

# Docker

```bash
sudo pacman -Syu
sudo pacman -S docker docker-compose
sudo systemctl enable --now docker.service
sudo usermod -aG docker ramil
newgrp docker
docker info
docker run --rm hello-world
```

---

# Монтирование диска

1 Найдите нужный раздел
bash
Копировать
Редактировать
lsblk -f # удобный обзор: имя-устройства, файловая система, метка, UUID, точка монтирования

# или

sudo fdisk -l # классический вывод с геометрией дисков
Предположим, что подключать надо раздел /dev/sdb1 (ext4).

2 (Опционально) Создайте файловую систему
Пропустите, если раздел уже отформатирован.

bash
Копировать
Редактировать
sudo mkfs.ext4 /dev/sdb1

# для XFS   sudo mkfs.xfs /dev/sdb1

# для NTFS  sudo mkfs.ntfs -f /dev/sdb1 # пакет ntfs-3g

3 Создайте точку монтирования
bash
Копировать
Редактировать
sudo mkdir -p /mnt/vortex
4 Временное монтирование (на текущую сессию)
bash
Копировать
Редактировать
sudo mount /dev/sdb1 /mnt/vortex

# проверить

mount | grep vortex # или df -h /mnt/vortex
Чтобы отмонтировать:

bash
Копировать
Редактировать
sudo umount /mnt/vortex # или sudo umount /dev/sdb1
5 Постоянное монтирование через /etc/fstab
Узнайте UUID раздела:

bash
Копировать
Редактировать
sudo blkid /dev/sdb1

# пример вывода:

# /dev/sdb1: UUID="3e1c1a0c-57ef-47af-88a7-2195058f324b" TYPE="ext4"

Откройте /etc/fstab любым редактором:

bash
Копировать
Редактировать
sudo nano /etc/fstab
Добавьте строку (пример для ext4):

text
Копировать
Редактировать
UUID=3e1c1a0c-57ef-47af-88a7-2195058f324b /mnt/vortex ext4 defaults,noatime 0 2
defaults — типичные опции (rw, suid, dev, exec, auto, nouser, async).

noatime — отключает обновление таймстампа доступа, чуть ускоряет I/O.

Последние две цифры: 0 — не делать dump, 2 — проверять fsck после / (которая имеет 1).

Для других ФС:

xfs — xfs defaults 0 0 (XFS не нуждается в fsck).

ntfs — ntfs-3g defaults,uid=1000,gid=1000 0 0 (uid/gid нужны, чтобы обычный пользователь мог писать).

Проверьте синтаксис и смонтируйте всё, не перезагружаясь:

bash
Копировать
Редактировать
sudo mount -a
Если ошибок нет, диск автоподнимется при каждой загрузке.

6 Права доступа (если нужен обычный пользователь)
bash
Копировать
Редактировать

# Сделать владельцем каталога текущего пользователя (ramil)

sudo chown ramil:ramil /mnt/vortex

# или открыть запись группе storage

sudo chgrp storage /mnt/vortex && sudo chmod 770 /mnt/vortex
Для NTFS/EXFAT вместо chown применяют опции uid= и gid= в fstab.

Краткий чек-лист
lsblk -f → выясняем устройство и UUID.

mkdir /mnt/vortex.

mount /dev/sdXn /mnt/vortex → проверяем.

Прописываем строку в /etc/fstab, проверяем mount -a.

Настраиваем владельца/права при необходимости.

После этого раздел всегда будет доступен в /mnt/vortex/.


---

# SSH

pacman -Syu
pacman -S openssh
systemctl start sshd
systemctl enable sshd
systemctl status sshd

Файл конфигурации: /etc/ssh/sshd_config

---

# добавить юзера в арчлинукс и дать ему sudo

pacman -S sudo
useradd -m -G wheel -s /bin/bash user1
passwd user1

EDITOR=vim visudo

Найдите строку:

Редактировать

# %wheel ALL=(ALL:ALL) ALL

и раскомментируйте её (уберите #):

Редактировать
%wheel ALL=(ALL:ALL) ALL

su - user1 # войти от имени нового пользователя
sudo pacman -Syu # тест: обновление системы с sudo

---

# диски

1 Найдите нужный раздел
lsblk -f # удобный обзор: имя-устройства, файловая система, метка, UUID, точка монтирования

# или

sudo fdisk -l # классический вывод с геометрией дисков

Создайте точку монтирования
sudo mkdir -p /mnt/vortex

4 Временное монтирование (на текущую сессию)
sudo mount /dev/sdb1 /mnt/vortex

# проверить

mount | grep vortex # или df -h /mnt/vortex
Чтобы отмонтировать:
sudo umount /mnt/vortex # или sudo umount /dev/sdb1

5 Постоянное монтирование через /etc/fstab
Узнайте UUID раздела:

sudo blkid /dev/sdb1

# пример вывода:

# /dev/sdb1: UUID="3e1c1a0c-57ef-47af-88a7-2195058f324b" TYPE="ext4"

Откройте /etc/fstab любым редактором:

sudo nano /etc/fstab
Добавьте строку (пример для ext4):

UUID=3e1c1a0c-57ef-47af-88a7-2195058f324b /mnt/vortex ext4 defaults,noatime 0 2
defaults — типичные опции (rw, suid, dev, exec, auto, nouser, async).

noatime — отключает обновление таймстампа доступа, чуть ускоряет I/O.

Последние две цифры: 0 — не делать dump, 2 — проверять fsck после / (которая имеет 1).

Для других ФС:

xfs — xfs defaults 0 0 (XFS не нуждается в fsck).

ntfs — ntfs-3g defaults,uid=1000,gid=1000 0 0 (uid/gid нужны, чтобы обычный пользователь мог писать).

Проверьте синтаксис и смонтируйте всё, не перезагружаясь:

sudo mount -a
Если ошибок нет, диск автоподнимется при каждой загрузке.

6 Права доступа (если нужен обычный пользователь)

# Сделать владельцем каталога текущего пользователя (ramil)

sudo chown ramil:ramil /mnt/vortex

# или открыть запись группе storage

sudo chgrp storage /mnt/vortex && sudo chmod 770 /mnt/vortex
Для NTFS/EXFAT вместо chown применяют опции uid= и gid= в fstab.

---

# Certbot

sudo pacman -Syu
sudo pacman -S certbot certbot-nginx

sudo certbot --nginx -d example.com -d www.example.com

автоматическое продление
sudo systemctl enable --now certbot-renew.timer

---

sudo pacman -S lm_sensors
sudo sensors-detect
на все ответить yes

sensorssnvg
watch -n1 sensors

sudo pacman -S btop
sudo pacman -S s-tui
sudo pacman -S nvtop
sudo pacman -S glances
sudo pacman -S iotop iftop nload

====

sudo pacman -S cockpit cockpit-pcp
sudo systemctl enable --now cockpit.socket

---

# Docker

docker compose ls

docker compose -p myproj ps
docker compose -p myproj ps --services # только сервисы
docker compose -p myproj ps -a # включая остановленные

docker compose -p mailcowdockerized down --volumes




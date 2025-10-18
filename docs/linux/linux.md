# Cockpit

```bash
sudo apt update
sudo apt install -y cockpit
sudo systemctl enable --now cockpit.socket
```

```bash
sudo apt install -y cockpit-storaged
sudo apt install -y cockpit-networkmanager
sudo apt install -y cockpit-packagekit packagekit packagekit-tools
sudo systemctl enable --now packagekit
sudo apt install -y cockpit-pcp pcp
sudo systemctl enable --now pmcd pmlogger
sudo apt install -y cockpit-machines qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils
sudo systemctl enable --now libvirtd
sudo usermod -aG libvirt,kvm $USER
sudo apt install -y cockpit-podman podman
systemctl --user enable --now podman.socket
loginctl enable-linger $USER
sudo apt install -y cockpit-sosreport
sudo systemctl restart cockpit.socket
```

# Docker

```bash
sudo apt update
sudo apt install -y apt-transport-https ca-certificates curl software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
sudo apt update
sudo apt install -y docker-ce docker-ce-cli containerd.io
sudo systemctl enable --now docker
sudo systemctl status docker
sudo usermod -aG docker $USER
newgrp docker
docker --version
docker run --rm hello-world
sudo curl -L "https://github.com/docker/compose/releases/download/$(curl -s https://api.github.com/repos/docker/compose/releases/latest | jq -r .tag_name)/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
sudo chmod +x /usr/local/bin/docker-compose
docker-compose --version
```

# Монтирование дисков

```bash
lsblk -f
sudo mkdir -p /mnt/example_name
sudo mount /dev/sdb1 /mnt/example_name
mount | grep example_name
```

Отмонтировать диск

```bash
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


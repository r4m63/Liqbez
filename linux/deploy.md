# Деплой сервисов в linux системах

# Nextcloud

# Gitlab

# Wireguard

## Docker deploy

Задать хэшированный пароль

`docker run --rm -it ghcr.io/wg-easy/wg-easy wgpw '210605'`

`docker run --detach \
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
ghcr.io/wg-easy/wg-easy`

Включить IP forwarding на хосте напрямую:

`sudo sysctl -w net.ipv4.ip_forward=1`

`sudo sysctl -w net.ipv6.conf.all.forwarding=1`

Либо добавить в /etc/sysctl.conf:

`net.ipv4.ip_forward=1`

`net.ipv6.conf.all.forwarding=1`

Чтобы получить доступ к админке роутера, на клиенте в конфиг вручную надо добавить в AllowedIPs:

`AllowedIPs = 0.0.0.0/0, ::/0, 192.168.0.0/24`

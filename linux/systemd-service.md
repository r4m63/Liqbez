```
/etc/systemd/system
sudo vim my.service
```

```bash
[Unit]
# Описание
Description=Описание сервиса
Documentation=man:nginx(8)      # ссылки на man/URL/документацию unit’а

# Порядок старта
After=network.target            # стартовать после указанных unit’ов
Before=nginx.service            # стартовать раньше указанных unit’ов

# Зависимости
Requires=postgresql.service     # сильная зависимость; если зависимость не поднимется, сервис тоже
Wants=network-online.target     # мягкая зависимость; systemd попробует поднять, но failure не обязателен для отказа
BindsTo=dev-sda1.device         # сильная привязка, часто к устройствам/монтированиям
PartOf=myapp.target             # если основной unit перезапускается/останавливается, этот следует за ним
Conflicts=shutdown.target       # взаимоисключающие unit’ы

# Условия запуска
ConditionPathExists=/etc/myapp.conf
ConditionFileNotEmpty=/etc/myapp.conf
ConditionDirectoryNotEmpty=/var/lib/myapp
AssertPathExists=/data

# Ограничение частоты старта
StartLimitIntervalSec=60        # Ограничивает число неудачных запусков за интервал.
StartLimitBurst=3

[Service]
Type=simple     # Варианты:
                # simple — default для многих случаев; ExecStart запущен, service считается стартовавшим почти сразу
                # exec — похож на simple, но старт считается успешным после успешного execve()
                # forking — старый daemon mode, когда процесс форкается в фон
                # oneshot — выполнить команду и выйти
                # notify — сервис сам сообщает systemd, что готов
                # notify-reload — notify + отдельная логика reload
                # dbus — сервис считается готовым, когда зарегистрировал имя на D-Bus
                # idle — откладывает выполнение до разгрузки очереди jobs
                # Для современных приложений обычно используют simple, exec, notify, реже forking.
                # Как выбирать:
	            # - обычный foreground-процесс → Type=simple
	            # - legacy daemon, который сам уходит в фон → Type=forking
	            # - init-скрипт/разовая команда → Type=oneshot
	            # - приложение с sd_notify → Type=notify

# Команды запуска/остановки
ExecStart=/usr/bin/myapp --config /etc/myapp.conf   # главная команда запуска
ExecStartPre=/usr/bin/test -f /etc/myapp.conf       # перед стартом
ExecStartPost=/usr/bin/logger started               # после старта
ExecReload=/bin/kill -HUP $MAINPID                  # что делать при systemctl reload
ExecStop=/usr/bin/myapp --stop                      # как останавливать
ExecStopPost=/usr/bin/rm -f /run/myapp.pid          # после остановки

# Перезапуск
Restart=on-failure
                        # Варианты:
                        # no
                        # on-failure
                        # always
                        # on-abnormal
                        # on-watchdog
                        # on-success
                        # on-abort
RestartSec=5
RestartSteps=...
RestartMaxDelaySec=...

# Таймауты
TimeoutStartSec=30              # сколько ждать старта
TimeoutStopSec=20               # сколько ждать остановки
TimeoutAbortSec=15
RuntimeMaxSec=1h                # максимум времени жизни сервиса
RuntimeRandomizedExtraSec=30
WatchdogSec=30                  # интервал watchdog для сервисов, умеющих общаться с systemd

# Пользователь, группа, umask
User=myuser
Group=mygroup
SupplementaryGroups=docker      # дополнительные группы
UMask=0027                      # маска прав для создаваемых файлов

# Рабочий каталог и окружение
WorkingDirectory=/opt/myapp                 # cwd процесса
Environment=APP_ENV=prod                    # переменные окружения
Environment=PORT=8080
EnvironmentFile=/etc/myapp.env              # читать env из файла
PassEnvironment=HTTP_PROXY HTTPS_PROXY      # пробросить переменные из manager environment
UnsetEnvironment=DEBUG                      # удалить переменные из окружения процесса

# PID-файл
PIDFile=/run/myapp.pid      # Обычно нужно только для Type=forking, когда основной PID надо узнать по pidfile.

# stdout/stderr и журнал
StandardOutput=journal
StandardError=journal
SyslogIdentifier=myapp
LogLevelMax=info

# Завершение процессов
KillMode=control-group      # как убивать процессы сервиса
KillSignal=SIGTERM          # основной сигнал остановки
FinalKillSignal=SIGKILL     # если не завершился вовремя
SendSIGKILL=yes             # слать ли SIGKILL в конце
SendSIGHUP=no

# Минимизация привилегий
NoNewPrivileges=yes
PrivateTmp=yes
PrivateDevices=yes
ProtectSystem=full
ProtectHome=yes
ProtectKernelTunables=yes
ProtectKernelModules=yes
ProtectControlGroups=yes
RestrictSUIDSGID=yes
LockPersonality=yes
MemoryDenyWriteExecute=yes
RestrictRealtime=yes

# Фильтрация системных вызовов и capability
CapabilityBoundingSet=CAP_NET_BIND_SERVICE
AmbientCapabilities=CAP_NET_BIND_SERVICE
SystemCallFilter=@system-service
SystemCallArchitectures=native
RestrictAddressFamilies=AF_INET AF_INET6 AF_UNIX

# Доступ к ФС
ReadOnlyPaths=/etc
ReadWritePaths=/var/lib/myapp /var/log/myapp
InaccessiblePaths=/home
TemporaryFileSystem=/tmp:size=100M
BindPaths=/srv/data:/data

# Контроль ресурсов
MemoryMax=500M
MemoryHigh=300M
CPUQuota=50%
CPUWeight=200
TasksMax=512
IOWeight=200

[Install]
WantedBy=multi-user.target      # при enable создается зависимость от target
                                #   multi-user.target — обычный серверный multi-user режим
                                #   graphical.target — графический режим
                                #   default.target — target по умолчанию через symlink/alias логики systemd
Also=myapp-worker.service       # вместе включать/выключать дополнительные unit’ы
Alias=myapp.service             # альтернативное имя unit’а
RequiredBy=some.target          # более сильная версия
```

```
sudo systemctl daemon-reload
sudo systemctl enable --now myapp.service
```

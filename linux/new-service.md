cd /etc/systemd/system

sudo vim aze-umma.ru.service

[Unit]
Description=Gunicorn instance to serve aze-umma.ru
After=network.target

[Service]
User=ramil
Environment=PORT=4000
ExecStart=/bin/bash -c 'cd /var/www/aze-umma.ru && npm start'
Restart=on-failure
[Install]
WantedBy=multi-user.target


sudo systemctl start aze-umma.ru
sudo systemctl enable aze-umma.ru


server {
    listen 80;
    server_name site1.example.com;  # Замените на домен

    location / {
        proxy_pass http:localhost:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}







server {
        listen 80;
        server_name gitlab.ramil21.ru;

        return 301 https://$host$request_uri;  # Перенаправление на HTTPS
    }

    server {
        listen 443 ssl;
        server_name gitlab.ramil21.ru;

        ssl_certificate /etc/letsencrypt/live/gitlab.ramil21.ru/fullchain.pem;
        ssl_certificate_key /etc/letsencrypt/live/gitlab.ramil21.ru/privkey.pem;

        location / {
            proxy_pass http://gitlab:80;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
        }
    }
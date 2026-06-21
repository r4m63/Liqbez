ADD			Add local or remote files and directories.
ARG			Use build-time variables.
CMD			Specify default commands.
COPY			Copy files and directories.
ENTRYPOINT		Specify default executable.
ENV			Set environment variables.
EXPOSE		Describe which ports your application is listening on.
FROM			Create a new build stage from a base image.
HEALTHCHECK	Check a container's health on startup.
LABEL			Add metadata to an image.
MAINTAINER		Specify the author of an image.
ONBUILD		Specify instructions for when the image is used in a build.
RUN			Execute build commands.
SHELL			Set the default shell of an image.
STOPSIGNAL		Specify the system call signal for exiting a container.
USER			Set user and group ID.
VOLUME		Create volume mounts.
WORKDIR		Change working directory.


----------------------------------------------------------------------------
FROM python:3.13-rc-bookworm

# рабочий каталог внутри контейнера, все следующие команды будут выполняться в этой директории 'cd /app'
# Это должен быть каталог, существующий в контейнере.
# Это должен быть каталог, куда будет скопирован код вашего приложения.
# Хорошей практикой является использование единообразной структуры каталогов во всех образах Docker.
WORKDIR /app

# копирует каталог, содержащий Dockerfile и его содержимое в контейнер в /app 
COPY . /app/


# RUN используется для выполнения команды в контейнере
RUN pip3 install --upgrade pip && pip install --no-cache-dir -r requirements.txt


# открывает порт из контейнера в хост-машину
# но он не сопоставляет эти порты автоматически с хост-машиной при запуске контейнера
# все равно нужно запустить контейнер docker run -p 8080:5000
EXPOSE 5000

# указывает команду по умолчанию для запуска при запуске контейнера
CMD ["python", "-m", "venv", "venv", "&&", "python", "main.py"]
----------------------------------------------------------------------------







# название образа - flask и взять Dockerfile из . текущей директории, так мы сбилдили образ
docker build --tag flask .	

# -p порты 8080 - на хост-машине, 5000 - в контейнере
# --name - имя контейнера, flas-test - имя образа 
docker run -p 8080:5000 --name flask-test-container flask-test







------------------------------------

# build the flask container
docker build -t prakhar1989/foodtrucks-web .

# create the network
docker network create foodtrucks-net

# start the ES container
docker run -d --name es --net foodtrucks-net -p 9200:9200 -p 9300:9300 -e "discovery.type=single-node" docker.elastic.co/elasticsearch/elasticsearch:6.3.2

# start the flask app container
docker run -d --net foodtrucks-net -p 5000:5000 --name foodtrucks-web prakhar1989/foodtrucks-web







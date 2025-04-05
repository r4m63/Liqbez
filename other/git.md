# GIT

Git - распределенная система контроля версий, потому что в ней не требуется центральный сервер, у каждого разработчика
локальная версия репозитория. В реальных проектах используют сервер для удобства обмена данными, сервер не выполняет
никакой умной работы, это только центральный пункт обмена данными.

git-bash-prompt
oh-my-zsh

## Команды

### Настройка Git

`git config`

- `--system`
- `--global`
- `--local`
- `--list`

`git config user.email "email@example.com"`

`git config user.name "Name"`

### Создание репозитория

`git init`

### Просмотр состояния и изменений

`git status` Отображает состояние рабочей директории и индекса (staging area), показывая: Отслеживаемые и
неотслеживаемые файлы, Изменения готовые к коммиту, Состояние ветки, Конфликты слияния (если есть)

- `git status -v` или `--verbose` Показать дополнительные подробности (повторный флаг увеличивает детализацию)
- `git status -s` или `--short` Краткий формат вывода
- `git status -b` или `--branch` Показать информацию о ветке и ее связи с удаленным репозиторием
- `git status --long` Полный формат вывода (по умолчанию)

`git diff` Показывает различия между: Рабочей директорией и индексом, Индексом и последним коммитом, Двумя коммитами,
Двумя ветками, Двумя файлами

- `git diff` Сравнение рабочей директории с индексом
- `git diff --staged` или `--cached` Сравнение индекса с последним коммитом
- `git diff HEAD` Сравнение рабочей директории с последним коммитом
- `git diff <commit>` Сравнение рабочей директории с указанным коммитом
- `git diff <commit1> <commit2>` Сравнение двух коммитов
- `git diff <branch1> <branch2>` Сравнение двух веток

### Добавление и фиксация изменений

`git add <file>` добавляет изменения из рабочего каталога в индекс

- `git add .` все файлы в текущей директории
- `git add -A` все изменения (добавления, удаления, перемещения)
- `git add -u` только отслеживаемые файлы

`git commit` фиксирует все подготовленные изменения в истории Git

- `git commit -m "Сообщение"` сообщение коммита
- `git commit -am "Сообщение"` -a заменяет (add .)
- `git commit --amend` позволяет изменить последний коммит (можно исправить сообщение или добавить забытые файлы)
- `--no-edit` не открывать редактор для сообщения коммита
- `--date=now` перезаписать дату
- `--author="Name <email>"` изменить автора
- `--reset-author` сбросить автора на текущего

### Работа с ветками

`git branch` – показать список веток.

`git branch <name>` – создать новую ветку.

`git branch -d <name>` – удалить ветку.

`git checkout <branch>` – переключиться на ветку.

`git checkout -b <new-branch>` – создать и переключиться на новую ветку.

`git switch <ветка>` – переключиться на ветку (альтернатива checkout).

`git merge <branch>` – слить изменения из указанной ветки.

`git rebase <branch>` – применить изменения поверх другой ветки.

`git cherry-pick <коммит>` – применить один коммит в текущую ветку.

### История и логи

`git log`

- `git log --oneline`
- `git log --graph`
-
`git log --graph --abbrev-commit --decorate --format=format:'%C(bold blue)%h%C(reset) - %C(bold green)(%ar)%C(reset) %C(white)%s%C(reset) %C(dim white)- %an%C(reset)%C(auto)%d%C(reset)' --all`

`git show <commit>`

`git blame <file>`

### Отмена изменений

`git revert <commit>` создаёт новый коммит откат, который отменяет изменения из указанного коммита, не удаляет коммит из
истории, а вносит противоположные изменения, чтобы “перекрыть” его эффект

`git reset`

`git reset <commit>`

- `git reset --soft <commit>`
- `git reset --mixed <commit>`
- `git reset --hard <commit>`
- `git reset --soft @~2`
- `git reset` HEAD

`git restore <file>`

`git clean` -dxf

### Удаленные репозитории

`git clone <url>`

`git remote`                   

`git remote -v`                

`git remote add <name> <url>`  

`git fetch <remote>`           

`git pull <remote> <branch>`   

`git push <remote> <branch>`   

`git push -u <remote> <branch>`

### Теги

`git tag`                         

`git tag <name>`                  

`git tag -a <name> -m "Сообщение"`

`git push <remote> --tags`

### Сохранение временных изменений

`git stash`              

`git stash pop`          

`git stash list`         

`git stash apply <stash>`

### Дополнительные

`git submodule add <url>`                

`git submodule update --init --recursive`

`git help <команда>` – получить справку по команде.

`git reflog` – показать историю всех изменений, включая удаленные коммиты.

`git bisect` – поиск коммита, вызвавшего ошибку.

`git gc` – очистка репозитория (удаление старых объектов).

`git fsck` – проверка целостности репозитория.

## Области GIT

1. Working directory

   Файлы, которые лежат в файловой системе

2. Index (в .git/index)

   Содержит изменения, которые будут включены в следующий коммит.

   Контролирует, какие изменения попадут в следующий коммит

3. Repository (в .git) - база данных всех версий проекта

   Содержит полную историю коммитов, веток, тегов

## Содержимое .git

## .github

## .gitattribute

## Сброс кэша

Если `.gitignore` не применяется - значит есть файлы которые git уже отслеживает.
Нужно удалить файлы из индекса

```bash
git rm -r --cached .
git add .
git commit -m "Restore cached + .gitignore"
git push
```

## Как правильно писать коммиты

## Как правильно создавать ветки


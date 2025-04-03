# GIT

Git - распределенная система контроля версий, потому что в ней не требуется центральный сервер, у каждого разработчика
локальная версия репозитория. В реальных проектах используют сервер для удобства обмена данными, сервер не выполняет
никакой умной работы, это только центральный пункт обмена данными.

git-bash-prompt
oh-my-zsh

## Команды

`git config`
- `--system`
- `--global`
- `--local`
- `--list`
`git config user.name "Name"`
`git config user.email "email@example.com"`

### Создание и клонирование репозитория

`git init`
`git clone <url>`

### Просмотр состояния и изменений

`git status`
- `-s` / `--short` 
- `-b` / `--branch`

`git diff`
- `--staged` – сравнивает индекс с последним коммитом.
- `--name-only` – показывает только измененные файлы.
- `--color-words` – подсвечивает изменения в тексте.
- `HEAD` – сравнить с последним коммитом.

### Добавление и фиксация изменений

`git add <file>`
`git commit -m "Сообщение"`
`git commit --amend`

### Отмена изменений

`git revert <commit>`       
`git reset`                 
`git reset --soft <commit>`
`git reset --mixed <commit>`
`git reset --hard <commit>`
`git reset -- soft @~2`
`git restore <file>`
`git reset` HEAD
`git clean` -dxf

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
`git log --oneline`
`git log --graph`  
`git show <commit>`
`git blame <file>`

### Удаленные репозитории

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
`git cherry-pick <коммит>` – применить один коммит в текущую ветку.

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

# Сброс кэша

Если `.gitignore` не применяется - значит есть файлы которые git уже отслеживает.
Нужно удалить файлы из индекса

```bash
git rm -r --cached .
git add .
git commit -m "Сброс кэша, обновление .gitignore"
git push
```



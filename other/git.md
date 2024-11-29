Если .gitignore не применяется - значит есть файлы которые git уже отслеживает.
Нужно удалить файлы из индекса


git rm -r --cached .
git add .
git commit -m "Обновление .gitignore и удаление отслеживаемых файлов"
git push


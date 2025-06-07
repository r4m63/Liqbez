# Git Flow

Git Flow — это одновременно модель ветвления (branching model) и набор утилит (расширение) для Git, которые
автоматизируют эту модель. Разберём оба аспекта подробнее.

## Модель ветвления Git Flow

**Основные ветки**

- master (или main)
  Содержит только продукционный (production-ready) код. Каждый коммит в master помечается тегом с версией (например,
  v1.2.0).
- develop
  Интеграционная ветка для разработки. Все новые фичи и исправления сначала вливаются сюда. С неё потом создаются ветки
  выпуска (release) и в неё же вливаются горячие исправления (hotfix).

**Вспомогательные ветки**

- feature/…
    - Для разработки новых функций.
    - Название: feature/<имя>, например, feature/login.
    - Создаются от develop и по завершении вливаются обратно в develop.
- release/…
    - Для подготовки к новому релизу: финальное тестирование, правка документации, сборка пакета.
    - Название: release/<версия>, например, release/1.2.0.
    - Создаётся от develop; после проверки вливается в master (и тегируется), а затем изменения (например, правки багов)
    - вливаются обратно в develop.
- hotfix/…
    - Для оперативного исправления критических ошибок на продакшене.
    - Название: hotfix/<версия>, например, hotfix/1.2.1.
    - Создаётся от master; после исправления вливается и в master (и тегируется) и в develop (чтобы изменения не
      затерялись).

## Жизненный цикл

```git
# 1. Инициализация проекта
git checkout master
git checkout -b develop

# 2. Новая фича
git checkout develop
git checkout -b feature/user-auth
# …работа…
git checkout develop
git merge --no-ff feature/user-auth
git branch -d feature/user-auth

# 3. Подготовка релиза
git checkout develop
git checkout -b release/1.2.0
# …финальные доработки…
git checkout master
git merge --no-ff release/1.2.0
git tag -a v1.2.0 -m "Release 1.2.0"
git checkout develop
git merge --no-ff release/1.2.0
git branch -d release/1.2.0

# 4. Горячий фикс
git checkout master
git checkout -b hotfix/1.2.1
# …исправление…
git checkout master
git merge --no-ff hotfix/1.2.1
git tag -a v1.2.1 -m "Hotfix 1.2.1"
git checkout develop
git merge --no-ff hotfix/1.2.1
git branch -d hotfix/1.2.1
```

## Gir Flow CLI

Чтобы не вводить все команды вручную, есть удобный Git Flow AVH Edition (git-flow), устанавливаемый через пакетный
менеджер

```bash
brew install git-flow-avh
apt-get install git-flow
```

1. Инициализация в репозитории
   ```
   git flow init
   ```
   — задаёт имена веток (master, develop) и шаблоны для префиксов (feature/, release/, hotfix/).

2. Фичи
   ```
   git flow feature start user-auth
   …работа…
   git flow feature finish user-auth
   ```
   — автоматически создаёт ветку от develop, а при завершении — вольёт в develop и удалит.

3. Релизы
   ```
   git flow release start 1.2.0
   …финалка…
   git flow release finish 1.2.0
   ```
   — вольёт в master (и создаст тег 1.2.0), затем вольёт в develop и удалит ветку.

4. Hotfix
   ```
   git flow hotfix start 1.2.1
   …фикс…
   git flow hotfix finish 1.2.1
   ```
   — аналогично релизу, но качается от master.

5. Поддержка других типов
   Есть команды `git flow support start <name>`, `git flow bugfix start <name>` и т. д., но чаще используются только три
   основных.

---

- Git Flow
    - Две главные ветки: main (продакшен) и develop (разработка).
    - Вспомогательные ветки: feature/*, release/*, hotfix/*.
    - Хорошо подходит для средних и больших команд с чётким циклом релизов.
- GitHub Flow
    - Одна главная ветка (main), все изменения делаются через короткоживущие feature-ветки.
    - После прохождения CI и ревью фичи сразу вливаются в main и деплоятся.
    - Идеален для непрерывного деплоя и малого либо продуктового стартапа.
- GitLab Flow
    - Комбинация Git Flow и GitHub Flow + ветки для разных окружений (staging, production).
    - Возможны варианты: feature-driven, environment-driven, release-driven.
    - Гибок для разных стратегий CI/CD и контроля окружений.
- Trunk-Based Development
    - Все изменения мерджатся в одну «trunk»-ветку (main) очень часто (несколько раз в день).
    - Для незавершённых фич используют Feature Flags, ветки живут не дольше дня.
    - Минимизирует конфликты, требует высокой дисциплины и автоматизации тестов.
- OneFlow
    - Упрощённый Git Flow: только main и develop, без отдельных release-веток.
    - Релизы тегируются прямо в develop, хотфиксы создаются от main.
    - Менее громоздок, чем классический Git Flow, но сохраняет разделение продакшн/разработка.
- Gitflow-Lite (Simplified Flow)
    - Только main + короткие feature-ветки, релизы — через теги в main.
    - Хотфиксы делают прямо в main или в отдельных ветках, быстро вливаясь обратно.
    - Подходит для небольших команд с умеренным объёмом работ.











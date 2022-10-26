# Приватный репозиторий Клуба креативного программирования 2022-2023

## Как участвовать в разработке

1. **⚠️⚠️⚠️ НЕ КОММИТИТЬ В МАСТЕР ⚠️⚠️⚠️**.
2. Для каждого нового проекта создавать новую ветку с соответствующим названием, и открывать pull-request, указав @vasthecat или @mchernigin в качестве reviewer, и себя в качестве assignees.
3. Сообщения к коммитам и комментарии пишем на русском языке (и без кракозябр (на utf-8)).
4. Можно оставлять в коде пометки в формате `// TODO(mchernigin): Дописать этот README` или `// NOTE(mchernigin): Я не понимаю, что тут происходит`.

## Сборка

### Windows

1. Запустить `winregen.bat`, нажав на него дважды из проводника;
2. Запустить сгенерированное решение Visual Studio (`.sln`) в папке `Build`;
3. Теперь можно собирать проекты внутри Visual Studio, указав нужный в качестве стартового;
4. При добавлении новых проектов перейти к пункту 1.

### Не Windows

1. Запустить `./maker.sh build` из терминала, находясь в корневой папке проекта;
2. Теперь можно запускать проекты с помощью команды `./Build/Projects/{project-name}/{project-name}`;
3. При любых изменениях файлов повторить пункт 1: и при добавлении новых проектов, и при изменении старых;
4. Для сборки релизной версии, в пункте 1 запускать `./maker.sh release`.

> На Linux и MacOS запускать проекты надо из корневой папки именно командой `./Build/Projects/{project-name}/{project-name}`, иначе пути к ассетам будут отличаться от Windows.

## Добавление новых проектов

Для того, чтобы добавить новый проект, достаточно скопировать папку `example-project` внутри `./Projects` и переименовать её. Внутри каждого проекта будет компилироваться только файл `main.cpp`.

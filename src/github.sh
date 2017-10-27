#!/usr/bin/env bash

# Получаем корневую дирректорию
readonly root=$(cd "$(dirname "$0")" && pwd)

# Добавляем репозиторий проекта на github
git remote add github https://github.com/anyks/aming.git

# Создаем github ветку
git branch github
git checkout github

# Выполняем нормальную сборку проекта
cd $root && make dev
# Выполняем сборку утилиты очистки
g++ ${root}/tools/clear.cpp -o ${root}/bin/clear

# Выполняем пулл проекта
#git pull github master:github

# Процедура удаления комментариев в C++ исходниках
clearComments(){
	# Получаем список файлов
	files=$(find $root -name '*.'$1)
	# Переходим по всему списков cpp файлов
	for file in $files
	do
		# Выполняем парсинг адреса
		dir="${file%/*}"
		fname=${file##*/}
		ext=${file##*.}
		# Выполняем удаление комментов
		${root}/bin/clear $file $dir/aming_$fname
		# Выполняем удаление предыдущего файла
		rm $file
	done
}
# Выполняем очистку комментариев
clearComments "cpp"
clearComments "h"

# Добавляем все файлы в репозиторий
git add $root

# Удаляем файлы из репозитория
git rm --cached ${root}/github.sh
git rm --cached ${root}/Makefile
git rm ${root}/tools/aming_clear.cpp

# Комитим изменения
git commit -am "First commit"

# Выполняем заливку в репозиторий
git push github github:master

# Добавляем файлы обратно
git add ${root}/github.sh
git add ${root}/Makefile

# Переключаемся обратно в основную ветку
git checkout master

# Удаляем ветку
git branch -D github


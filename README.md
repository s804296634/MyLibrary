Git 全局设置

git config --global user.name "s804296634"
git config --global user.email "804296634@qq.com"

创建新的版本库

git clone http://192.168.1.7/gitlab/embedded/nicholas/test.git
cd test
touch README.md
git add README.md
git commit -m "add README"
git push -u origin master


已存在的文件夹

cd existing_folder
git init
git remote add origin http://192.168.1.7/gitlab/embedded/nicholas/test.git
git add .
git commit -m "Initial commit"
git push -u origin master

已存在的版本库

cd existing_repo
git remote rename origin old-origin
git remote add origin http://192.168.1.7/gitlab/embedded/nicholas/test.git
git push -u origin --all
git push -u origin --tags

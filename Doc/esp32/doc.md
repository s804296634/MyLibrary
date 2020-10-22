### 访问本地网盘
smb://192.168.1.3/tech2/

simon
beacon+

### esp32环境搭建

以下命令和重新安装 git clone 的子模组。
git submodule update --init --recursive


### IDF路径

gedit ~/.bashrc
末尾加上  export IDF_PATH=/home/xt/esp/esp-idf

source ~/.bashrc  #更新配置

. $HOME/esp/esp-idf/export.sh  # 可将此行代码 写入 /etc/profile 中

### 串口号查询

ls /dev/ttyUSB*


### 串口权限

sudo usermod -a -G dialout user_name 
user_name 改成当前登陆的用户名

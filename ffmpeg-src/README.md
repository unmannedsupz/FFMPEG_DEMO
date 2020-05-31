# 编译ffmpeg

# 1.安装yasm:  
wget http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz  
tar xvzf yasm-1.3.0.tar.gz  
cd yasm-1.3.0  
./configure  
make && make install    
  
# 2.安装nasm(2.13以上版本)  
wget https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/nasm-2.14.02.tar.bz2  
tar xvf nasm-2.14.02.tar.bz2   
cd nasm-2.14.02  
./configure    
make && make install   
  
# 3.安装其他依赖  
yum install cmake -y  
  
# 4.编译x264(只编译静态库)  
x264下载地址：  
http://ftp.videolan.org/pub/videolan/x264/snapshots/  
tar xvf x264-snapshot-20191024-2245-stable.tar.bz2   
cd x264-snapshot-20191024-2245-stable  
./configure --enable-static --prefix=../x264 --enable-pic   
make -j16  
make install  
  
# 5.编译x265(只编译静态库)  
x265下载地址:  
http://ftp.videolan.org/pub/videolan/x265/  
tar xvf x265_3.2.tar.gz   
cd x265_3.2/build/linux/  
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="../../../x265" -DENABLE_SHARED:bool=off ../../source  
make -j16  
make install  
  
# 6.编译ffmpeg  
tar xvf ffmpeg-4.1.3.tar.bz2  
cd ffmpeg-4.1.3  
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:../x265/lib/pkgconfig  
./configure --enable-shared --enable-nonfree --enable-gpl --enable-pthreads --enable-libx264 --enable-libx265 --prefix=../ffmpeg --extra-cflags="-I../x264/include -I../x265/include" --extra-ldflags="-L../x264/lib/ -L../x265/lib/ -ldl -lm -lpthread -lstdc++"  
make -j32  
make install  

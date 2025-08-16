# Croplines

Croplines是一个用于制作曲谱同步视频的小工具，它通过GUI图形界面与用户进行交互，以高效地裁剪乐谱。

## 使用方法

Croplines目前已经实现了裁剪、可缩放和可移动的画布显示图片，以及项目保存的功能。

通过使用鼠标滚轮，可以对图片进行放大或缩小操作。同时，通过鼠标拖拽，可以移动图片，以便更方便地观察乐谱的局部细节，避免裁剪到线谱或乐谱标记。

要进行裁剪操作，请使用鼠标右键单击。程序将绘制一条直线来分割裁剪区域。如果需要删除裁剪横线并撤销裁剪操作，请同时按下"D"键并使用鼠标右键单击横线。

## 快捷键

键盘上下键实现翻页

## 编译

### OpenCV 编译选项

```
Commandline options:
-DBUILD_SHARED_LIBS:BOOL="0" -DOPENCV_GAPI_GSTREAMER:BOOL="0" -DBUILD_opencv_calib3d:BOOL="1" -DBUILD_opencv_videoio:BOOL="0" -DBUILD_opencv_java:BOOL="0" -DWITH_FFMPEG:BOOL="0" -DWITH_OPENMP:BOOL="1" -DBUILD_opencv_ts:BOOL="0" -DBUILD_TESTS:BOOL="0" -DOPENCL_LIBRARY:STRING="" -DWITH_EIGEN:BOOL="0" -DBUILD_opencv_python_tests:BOOL="0" -DLAPACK_LIBRARIES:STRING="" -DBUILD_opencv_gapi:BOOL="0" -DBUILD_opencv_python_bindings_generator:BOOL="0" -DBUILD_opencv_video:BOOL="0" -DWITH_LAPACK:BOOL="0" -DVIDEOIO_ENABLE_PLUGINS:BOOL="0" -DBUILD_JAVA:BOOL="0" -DBUILD_opencv_java_bindings_generator:BOOL="0" -DWITH_AVIF:BOOL="0" -DWITH_GSTREAMER:BOOL="0" -DWITH_DIRECTML:BOOL="0" -DWITH_OPENEXR:BOOL="0" -DBUILD_opencv_python3:BOOL="0" 


Cache file:
BUILD_SHARED_LIBS:BOOL=0
OPENCV_GAPI_GSTREAMER:BOOL=0
BUILD_opencv_calib3d:BOOL=1
BUILD_opencv_videoio:BOOL=0
BUILD_opencv_java:BOOL=0
WITH_FFMPEG:BOOL=0
WITH_OPENMP:BOOL=1
BUILD_opencv_ts:BOOL=0
BUILD_TESTS:BOOL=0
OPENCL_LIBRARY:STRING=
WITH_EIGEN:BOOL=0
BUILD_opencv_python_tests:BOOL=0
LAPACK_LIBRARIES:STRING=
BUILD_opencv_gapi:BOOL=0
BUILD_opencv_python_bindings_generator:BOOL=0
BUILD_opencv_video:BOOL=0
WITH_LAPACK:BOOL=0
VIDEOIO_ENABLE_PLUGINS:BOOL=0
BUILD_JAVA:BOOL=0
BUILD_opencv_java_bindings_generator:BOOL=0
WITH_AVIF:BOOL=0
WITH_GSTREAMER:BOOL=0
WITH_DIRECTML:BOOL=0
WITH_OPENEXR:BOOL=0
BUILD_opencv_python3:BOOL=0
```

建议使用 MSYS2 环境。安装 wxWidgets 与 Cereal 库。
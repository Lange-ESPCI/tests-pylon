# Notes

# Compilation OpenCV

## Bibliothèques pour les images

`libjpeg8-dev libtiff5-dev libjasper-dev libpng12-dev`

## Bibliothèques pour la vidéo

`libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libxvidcore-dev libx264-dev`

## Bibliothèques graphiques

`libgtk-3-dev`

## Bibliothèques d'optimisations

`libatlas-base-dev gfortran`

## Génération du makefile

https://docs.opencv.org/4.5.2/db/d05/tutorial_config_reference.html

 ```bash
 cmake -D CMAKE_BUILD_TYPE=RELEASE\
 -D CMAKE_INSTALL_PREFIX=/usr/local\
 -D OPENCV_EXTRA_MODULES_PATH=../opencv_contrib-master/modules\
 -D BUILD_EXAMPLES=ON\
 -D WITH_FFMPEG=1\
 -D WITH_CUDA=ON\
 -D CUDA_GENERATION=Pascal\
 -D ENABLE_FAST_MATH=1\
 -D WITH_CUBLAS=1\
 ../opencv-master/
 ```
Structure des fichiers :

```
.
├── opencv_build
├── opencv_contrib-master
└── opencv-master
```

Pour la version de CUDA voir architecture GPU.

## Compilation

`make -j 4`

C'est long et c'est chiant.

## Installation

`sudo make install`

/*
Copyright (C) 2010 Arne Bernin
This code is licensed to you under the terms of the GNU GPL, version 2 or version 3;
see:
http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
http://www.gnu.org/licenses/gpl-3.0.txt
*/
/*
Modified Code
Copyright (C) 2013 Jay Rambhia
This code is licensed to you under the terms of the GNU GPL, version 2 or version 3;
see:
http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
http://www.gnu.org/licenses/gpl-3.0.txt
*/



#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libfreenect.h>
#include <pthread.h>
#define CV_NO_BACKWARD_COMPATIBILITY
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <unistd.h>                                 //para usar sleep y usleep

  //librerias para usar HRTF

#include <iostream>
#include <cstdlib>
#include "yse.hpp"
#ifdef WINDOWS
#include <conio.h>
#else
#include "wombat/wincompat.h"
#endif

YSE::sound sound;   //objeto sonido



#define FREENECTOPENCV_WINDOW_D "Depthimage"
#define FREENECTOPENCV_WINDOW_N "Normalimage"
#define FREENECTOPENCV_RGB_DEPTH 3
#define FREENECTOPENCV_DEPTH_DEPTH 1
#define FREENECTOPENCV_RGB_WIDTH 640
#define FREENECTOPENCV_RGB_HEIGHT 480
#define FREENECTOPENCV_DEPTH_WIDTH 640
#define FREENECTOPENCV_DEPTH_HEIGHT 480

using namespace cv;
using namespace std;

Mat depthimg, rgbimg, tempimg, canny_temp, canny_img, origin;

pthread_mutex_t mutex_depth = PTHREAD_MUTEX_INITIALIZER;  // mutex para los datos de profundidad
pthread_t cv_thread;



// funcion que transforma el dato de profundidad en milimetros
int millimeters (int value)
{
    float mt;
    int mm;
    mt=0.1236*tan(value/2842.5+1.1863);
    mm=mt*1000;
    return mm;
}

//setea posicion del emisor del sonido

void setPosition(float x, float z) {

  YSE::Vec pos;
  x=-x/100;
  z=z/500;
  pos.set(x, 0, z);
  sound.pos(pos);

}

//calcula la distancia del eje X
int calculateXDistance (int pixelX, int depthValue)
{
    int xDistance;
    int pixel = pixelX -320;
    xDistance= pixel*depthValue/575;
    return xDistance;
}

//filtra si suena o no
int filter(int depthValue, int distanceX)
{
    int flag;
    if(depthValue>2500)
    {
        flag=0;
    }
    else if(abs(distanceX)>1000)
    {
        flag=0;
    }
    else
    {
        flag=1;
    }
    return flag;
}


// llamado para adquirr datos de la kinec, esto lo hace libfreenect
void depth_cb(freenect_device *dev, void *depth, uint32_t timestamp)
{
    Mat depth8;
    Mat mydepth = Mat( FREENECTOPENCV_DEPTH_WIDTH,FREENECTOPENCV_DEPTH_HEIGHT, CV_16UC1, depth);

    // mutex para profundidad
    pthread_mutex_lock( &mutex_depth );
    memcpy(origin.data, mydepth.data, 640*960);
    mydepth.convertTo(depth8, CV_8UC1, 255.0/2048.0);
    memcpy(depthimg.data, depth8.data, 640*480);
    // unlock mutex
    pthread_mutex_unlock( &mutex_depth );
 
}
 

//thread para procesar los datos con open cv

void *cv_threadfunc (void *ptr) {
    depthimg = Mat(FREENECTOPENCV_DEPTH_HEIGHT, FREENECTOPENCV_DEPTH_WIDTH, CV_8UC1);
    tempimg = Mat(FREENECTOPENCV_RGB_HEIGHT, FREENECTOPENCV_RGB_WIDTH, CV_8UC3);
    canny_img = Mat(FREENECTOPENCV_RGB_HEIGHT, FREENECTOPENCV_RGB_WIDTH, CV_8UC1);
    canny_temp = Mat(FREENECTOPENCV_DEPTH_HEIGHT, FREENECTOPENCV_DEPTH_WIDTH, CV_8UC3);
    origin = Mat(FREENECTOPENCV_DEPTH_HEIGHT, FREENECTOPENCV_DEPTH_WIDTH, CV_16UC1);

    double minval;
    double maxval;
    Point minLoc;
    Point maxLoc;

    int pto1;
    int pto2;
    int depthValue;
    int contador=0;
    int distanceX;
    int decision;

    // use image polling
    while (1)
    {
        //lock depth mutex
        pthread_mutex_lock( &mutex_depth );

        Canny(depthimg, canny_temp, 50.0, 200.0, 3);
        cvtColor(depthimg,tempimg,CV_GRAY2BGR);
       // erode(depthimg, depthimg, Mat(), Point(-1,-1),2,1,1);
       // dilate(depthimg, depthimg, Mat(), Point(-1,-1),2,1,1);

        // Busca los pixeles mas grande y mas pequeño
        minMaxLoc(depthimg,&minval,&maxval,&minLoc,&maxLoc);

        //punto minimo posicion x e y
        pto1=minLoc.x;
        pto2=minLoc.y;

        // puntos para creacion de los rectangulos
        Point pt1 = Point(minLoc.x -50,minLoc.y-50);
        Point pt2 = Point(minLoc.x +50,minLoc.y+50);

        cout << "punto1=" << pto1;
        cout << "punto2=" << pto2 << endl;
        // ver el origin por q entrega puras weas y revisar el depthing
       // cout << "valor" << (int)origin.at<unsigned short>(pto2,pto1) << "  valor minimo="<<minval <<endl;

        cout << "valor" << (int)origin.at<unsigned short>(pto2,pto1) << "  valor minimo="<<minval <<endl;
        cout << "x "<< pto1 << "    y "<< pto2<< endl;

        rectangle(tempimg,pt1,pt2,0,2,8,0); //rectangulo para seguir el punto mas cercano

        depthValue=millimeters((int)origin.at<unsigned short>(pto2,pto1));
        distanceX = calculateXDistance(pto1,depthValue);
        cout << "lo que entrega" << depthValue<< "  distancia en x"<< distanceX << endl;

        decision=filter(depthValue,distanceX);

        // filtrado por interes (if else  con los flags)
        if(decision == 1)
        {
            setPosition(distanceX,depthValue);
        sound.play();
        }

        if(decision==0)
        {
            sound.stop();
            //YSE::System.update();
        }
        usleep(100000);
        YSE::System.update();
        contador = contador+1;
        //YSE::System.sleep (100);

        imshow(FREENECTOPENCV_WINDOW_D,tempimg);
        imshow("original depth", origin);
        //imshow("original depth", depthimg);
        //imshow("Depth Canny", canny_temp);
        //unlock mutex for depth image
        pthread_mutex_unlock( &mutex_depth );

        //lock mutex for rgb image
        //pthread_mutex_lock( &mutex_rgb );

        //cvtColor(rgbimg,tempimg,CV_BGR2RGB);
        //cvtColor(tempimg, canny_img, CV_BGR2GRAY);

        //imshow(FREENECTOPENCV_WINDOW_N, tempimg);

        //Canny(canny_img, canny_img, 50.0, 200.0, 3);

        //imshow("Canny Image", canny_img);
        //unlock mutex
        //pthread_mutex_unlock( &mutex_rgb );

        // wait for quit key
        if(cvWaitKey(15) == 27)
        {
            YSE::System.close();
            break;
        }


    }
    pthread_exit(NULL);

    return NULL;

}
 
int main(int argc, char **argv)
{
 
    freenect_context *f_ctx;
    freenect_device *f_dev;
 
    int res = 0;
    int die = 0;
    printf("Kinect camera test\n");
 
    if (freenect_init(&f_ctx, NULL) < 0)
    {
        printf("freenect_init() failed\n");
        return 1;
    }
 
    if (freenect_open_device(f_ctx, &f_dev, 0) < 0)
    {
        printf("Could not open device\n");
        return 1;
    }

    YSE::System.init();             // inicia YSE sound engine
    //carga y validacion del sonido
    sound.create("drone.ogg", NULL, true);

    // false on validation means the sound could not be loaded
    if (!sound.valid())
    {
      std::cout << "sound 'drone.ogg' not found" << std::endl;
      std::cin.get();
      goto exit;
    }

    //cambiar el formato de los datos a milimetros directamente .... no funciona
    //freenect_frame_mode frame=freenect_find_depth_mode( FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_MM);
    //freenect_set_depth_mode(f_dev,frame);

    freenect_set_depth_callback(f_dev, depth_cb);
 
    // create opencv display thread
    res = pthread_create(&cv_thread, NULL, cv_threadfunc, NULL);
    if (res)
    {
        printf("pthread_create failed\n");
        return 1;
    }
    printf("init done\n");
 
    freenect_start_depth(f_dev);

    while(!die && freenect_process_events(f_ctx) >= 0 );

exit:
  // don't forget this...
  YSE::System.close();
  return 0;
}

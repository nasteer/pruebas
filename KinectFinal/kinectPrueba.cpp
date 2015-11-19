
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
#define FREENECTOPENCV_DEPTH_DEPTH 1
#define FREENECTOPENCV_DEPTH_WIDTH 640
#define FREENECTOPENCV_DEPTH_HEIGHT 480

using namespace cv;
using namespace std;

Mat depthimg, tempimg, origin;

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
    x=x/100;
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

//calcula la distancia del eje Y
int calculateYDistance (int pixelY, int depthValue)
{
    int yDistance;
    int pixel = pixelY -320;
    yDistance= pixel*depthValue/575;
    return yDistance;
}



//filtra si suena o no dependiendo del espacio de interes
int filter(int depthValue, int distanceX)
{
    int flag;
    if(depthValue>1500)
    {
        flag=0;
    }
    else if(abs(distanceX)>600)
    {
        flag=0;
    }
    else
    {
        flag=1;
    }
    return flag;
}


// llamado para adquirir datos de la kinect, esto lo hace libfreenect
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
    tempimg = Mat(FREENECTOPENCV_DEPTH_HEIGHT, FREENECTOPENCV_DEPTH_WIDTH, CV_8UC3);
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
        //bloqueo mutex profundidad
        pthread_mutex_lock( &mutex_depth );

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


        rectangle(tempimg,pt1,pt2,0,2,8,0); //rectangulo para seguir el punto mas cercano

        depthValue=millimeters((int)origin.at<unsigned short>(pto2,pto1));
        distanceX = calculateXDistance(pto1,depthValue);

        decision=filter(depthValue,distanceX);

        // filtrado por interes
        if(decision == 1)
        {
            setPosition(distanceX,depthValue);
        sound.play();
        }

        if(decision==0)
        {
            sound.stop();
        }
        usleep(100000);
        YSE::System.update();
        contador = contador+1;

        imshow(FREENECTOPENCV_WINDOW_D,tempimg);
        imshow("original depth", origin);
        //desbloqueo mutex profundidad
        pthread_mutex_unlock( &mutex_depth );


        switch(cvWaitKey(15)) {
          case 27: // esc
            YSE::System.close();
            break;
          case 119:// w
            sound.volume(sound.volume() + 0.1);
            break;
          case 115:// s
            sound.volume(sound.volume() - 0.1);
            break;
          case 113:// q
            sound.volume(0.5);
            break;
          case 49: // 1
            sound.volume(0.0);
            break;
          case 110: // n
            sound.speed (sound.speed () - 0.01);
            cout << "velocidad=" << sound.speed()<< endl;
            break;
          case 109: // m
            sound.speed (sound.speed () + 0.01);
            cout << "velocidad=" << sound.speed()<< endl;
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
    int get;
    printf("Kinect camera test\n");
 
    if (freenect_init(&f_ctx, NULL) < 0)
    {
        printf("freenect_init() failed\n");
        return 1;
    }
 
    if (freenect_open_device(f_ctx, &f_dev, 0) < 0)
    {
        printf("No se puede abrir el dispositivo\n");
        return 1;
    }


    //carga y validacion del sonido
    cin>>get;

    switch(get) {
        case 1:
            YSE::System.init();                         // inicia YSE sound engine
            sound.create("grave(150).ogg", NULL, true);
            sound.volume(0.5);
            break;
        case 2:
            YSE::System.init();
            sound.create("grave(180).ogg", NULL, true);
            sound.volume(0.5);
            break;
        case 3:
            YSE::System.init();
            sound.create("grave(200).ogg", NULL, true);
            sound.volume(0.5);
            break;
        case 4:
            YSE::System.init();
            sound.create("grave(300).ogg", NULL, true);
            sound.volume(0.5);
            break;
        case 5:
            YSE::System.init();
            sound.create("agudo(800).ogg", NULL, true);
            sound.volume(0.5);
            break;
    }


    // Falso en la validacion, significa que no se cargo el sonido
    if (!sound.valid())
    {
        std::cout << "sonido no encontrado" << std::endl;
        std::cin.get();
        goto exit;
    }

    freenect_set_depth_callback(f_dev, depth_cb);
 
    // crea el hilo de OpenCV
    res = pthread_create(&cv_thread, NULL, cv_threadfunc, NULL);
    if (res)
    {
        printf("fallo la creacion del hilo\n");
        return 1;
    }
    printf("Inicio\n");
 
    freenect_start_depth(f_dev);

    while(!die && freenect_process_events(f_ctx) >= 0 );

exit:
    YSE::System.close();
    return 0;
}

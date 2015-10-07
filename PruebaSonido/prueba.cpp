#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include "yse.hpp"
#include <unistd.h>
/*
#ifdef WINDOWS
#include <conio.h>
#else
*/
#ifdef YSE_WINDOWS
#include <conio.h>
#else
#include "wombat/wincompat.h"
#endif

YSE::sound sound1;


void setPosition(float hola, float z) {
  // YSE has a very flexible vector class built in
  YSE::Vec pos;
  //pos.zero();   YSE::Listener.pos(pos);
  pos.set(hola, 0, z);
  sound1.pos(pos);

}


int main() {
  // initialize audio system
  YSE::System.init();

  // load a sound in memory
  sound1.create("dron2.ogg", NULL, true);

  // false on validation means the sound could not be loaded
  if (!sound1.valid()) {
    std::cout << "sound 'drone.ogg' not found" << std::endl;
    std::cin.get();
    goto exit;
  }

  std::cout << "Press spacebar to toggle sound playing." << std::endl;
  std::cout << "Or e to exit."                           << std::endl;


 sound1.play();

 //YSE::Vec pos(5, 0, 5);
 //float x = 1.0;
 //sound1.pos(pos);
 setPosition(0.001,1);
 //sound1.pos(YSE::Vec(x,0,0));
  while (true) {


    /* the sleep function can be used to make sure the update function doesn't run at full
       speed. In a simple demo it does not make sense to update that much. In real software
       this should probably not be used. Just call YSE::System.update() in your main program loop.
    */
    YSE::System.sleep (100);

    /* The update function activates all changes you made to sounds and channels since the last call.
       This function locks the audio processing thread for a short time. Calling it more than 50 times
       a second is really overkill, so call it once in your main program loop, not after changing every setting.
    */
    YSE::System.update(   );

  }

exit:
  // don't forget this...
  YSE::System.close();
  return 0;
}

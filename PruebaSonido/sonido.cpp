#include <iostream>
//#include <unistd.h>
//#include <stdlib.h>
//#include <termios.h>
//#include <unistd.h>
//#include <fcnt1.h>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
//#include <ncurses.h>
#include "yse.hpp"
#ifdef YSE_WINDOWS
#include <conio.h>
#else
#include "wombat/wincompat.h"
#endif 



// crea el objeto del sonido
YSE::sound sound1;

enum direction {
	ADELANTE,
	ATRAS,
	IZQUIERDA,
	DERECHA,
};

void moverFuente(direction d);
void reset();

/*int kbhit(void)
{
   int ch1 = getch();
 
   if (ch1 != ERR) {
       ungetch(ch1);
       return 1;
   } else {
       return 0;
   }
}
*/

int contador =0;


int main() {
 	//initscr();
 
  	//cbreak();
   	//noecho();
   	//nodelay(stdscr, TRUE);
 
   	//scrollok(stdscr, TRUE);

	// inicializa el sistema de audio  YSE
	YSE::System().init();

	// carga el sonido, lo comienza a reproducion y lo situa en la posicion inicial con reset
	sound1.create("Heal8.ogg", NULL, true).play();
	reset();

	std::cout << "Posicion inicial (x,y,z)" << std::endl;
	std::cout << "Sound 1 : 0 / 0 / 5" << std::endl;
	std::cout << std::endl;
	std::cout << "Usar WASD para mover la fuente del sonido." << std::endl;
	std::cout << "Presionar r para volver a posicion inicial" << std::endl;
	std::cout << "Presionar e para salir" << std::endl;

	while (true) {
		/*if (kbhit()) {
			//char ch = getchar();
			char ch = getch();
			switch (ch) {
			case 'a': moverFuente(IZQUIERDA); break;
			case 'd': moverFuente(DERECHA); break;
			case 'w': moverFuente(ADELANTE); break;
			case 's': moverFuente(ATRAS); break;
			case 'r': reset(); break;
			case 'e': goto exit;
			}
		} */

		for (int i=0; i<= 100; i++){
		contador= contador +1;
			if (i==99){
				moverFuente(DERECHA);
			}
		}




		YSE::System().sleep(100);
		//YSE::Vec pos = sound1.getPosition();
		//pos.x = sin(std::clock() / static_cast<Flt>(CLOCKS_PER_SEC)) * 10;
		//sound1.setPosition(pos);
		YSE::System().update();
	}

exit:
	YSE::System().close();
	return 0;
}

void moverFuente(direction d) {
		YSE::sound * s;
		s = &sound1;
		YSE::Vec pos = s->getPosition();
		switch (d) {
			case ADELANTE: pos.z += 0.5f; s->setPosition(pos); break;
			case ATRAS: pos.z -= 0.5f; s->setPosition(pos); break;
			case DERECHA: pos.x -= 0.5f; s->setPosition(pos); break;
			case IZQUIERDA: pos.x += 0.5f; s->setPosition(pos); break;
		}
}

void reset() {
	YSE::Vec pos;
	pos.set(5, 0, 5);  sound1.setPosition(pos);
}

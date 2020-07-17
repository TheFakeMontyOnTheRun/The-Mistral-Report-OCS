#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>


#ifndef __DJGPP__
const long UCLOCKS_PER_SEC = 1000;

long timeEllapsed = 0;

long uclock() {
    timeEllapsed += (1000 / 60);
    return timeEllapsed;
}

#endif

#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

void initHW();

void shutdownHW();



void initHW() {
    initFileReader("base.pfs");
    graphicsInit();
}

void shutdownHW() {
    graphicsShutdown();
}
#ifndef ANDROID

int main(int argc, char **argv) {


    puts(
            "The Mistral Report - Invisible Affairs, 2018-2019 - by the Brotherhood "
            "of 13h");

    //srand(time(NULL));
    initHW();
    enterState(kMainMenu);


#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while (isRunning) {
        mainLoop();
    }
#endif
    unloadStateCallback();
    shutdownHW();

    return 0;
}

#endif

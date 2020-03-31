#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "LoadBitmap.h"
#include "Enums.h"
#include "Engine.h"
#include "FixP.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"

extern size_t biggestOption;

int32_t Interrogation_initStateCallback(int32_t tag, void *data) {

    currentBackgroundBitmap = loadBitmap("pattern.img");

    timeUntilNextState = 10000 - 1;

    currentPresentationState = kWaitingForInput;
    return 0;
}

void Interrogation_initialPaintCallback() {
    drawRepeatBitmap(0, 32, 320, 200, currentBackgroundBitmap);

    fill(255, 8, 8, 128, 0, TRUE);
    fill(0, 128, 256, 8, 0, TRUE);

    fill(0, 0, 256, 128, 255, FALSE);

    drawRect(0, 0, 256, 128, 0);
    fill(0, 0, 256, 8, 0, FALSE);

    fill(8, 144, 120, 8, 0, FALSE);
    fill(142, 144, 80, 8, 0, FALSE);
    fill(236, 144, 64, 8, 0, FALSE);

    drawTextAt(3, 19, "Interrogation", 255);
    drawTextAt(20, 19, "Emotions", 255);
    drawTextAt(32, 19, "Stress", 255);

    fill(8 + 8, 144 + 8, 120, 48, 0, TRUE);
    fill(142 + 8, 144 + 8, 80, 48, 0, TRUE);
    fill(236 + 8, 144 + 8, 64, 48, 0, TRUE);

    fill(8, 152, 120, 40, 255, FALSE);
    fill(142, 152, 80, 40, 255, FALSE);
    fill(236, 152, 64, 40, 255, FALSE);

    drawRect(8, 152, 120, 40, 0);
    drawRect(142, 152, 80, 40, 0);

    drawRect(236, 152, 64, 40, 0);
}

void Interrogation_repaintCallback() {

}

int32_t Interrogation_tickCallback(int32_t tag, void *data) {

    long delta = *((long *) data);

    timeUntilNextState -= delta;


    if (timeUntilNextState <= 0) {

        switch (currentPresentationState) {
            case kAppearing:
                timeUntilNextState = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
                currentPresentationState = kWaitingForInput;
                break;
            case kWaitingForInput:


                break;
            case kConfirmInputBlink1:
            case kConfirmInputBlink2:
            case kConfirmInputBlink3:
            case kConfirmInputBlink4:
            case kConfirmInputBlink5:
            case kConfirmInputBlink6:
                timeUntilNextState = MENU_ITEM_TIME_TO_BLINK_MS;
                currentPresentationState =
                        (enum EPresentationState) ((int) currentPresentationState + 1);
                break;
            case kFade:
                return nextNavigationSelection;
        }
    }


    return -1;
}

void Interrogation_unloadStateCallback() {
    releaseBitmap(currentBackgroundBitmap);
}

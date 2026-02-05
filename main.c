//
// Created by lidongyooo on 2026/2/5.
//

#include "platform.h"

#ifdef PLATFORM_IOS

#include "FridaGum-IOS-17.6.2.h"

#else

#include "FridaGum-Android-17.6.2.h"

#endif


int main () {
    gum_init();
    printf("xxx %p", main);
}
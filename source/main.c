#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <io/pad.h>
#include <sysutil/sysutil.h>

static int rodando = 1;

static void evento_sistema(u64 status, u64 param, void *userdata) {
    if (status == SYSUTIL_EXIT_GAME) {
        rodando = 0;
    }
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    ioPadInit(7);
    sysUtilRegisterCallback(0, evento_sistema, NULL);

    SDL_Surface *tela = SDL_SetVideoMode(1280, 720, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
    SDL_Surface *pou = IMG_Load("/dev_hdd0/game/POUPS3001/USRDIR/corpo.png");
    SDL_Surface *comida = IMG_Load("/dev_hdd0/game/POUPS3001/USRDIR/comida.png");

    padInfo padinfo;
    padData paddata;
    int comendo = 0;
    int timer_comida = 0;

    while (rodando) {
        sysUtilCheckCallback();
        ioPadGetInfo(&padinfo);

        if (padinfo.status[0]) {
            ioPadGetData(0, &paddata);
            if (paddata.BTN_CROSS && timer_comida == 0) {
                comendo = 1;
                timer_comida = 40;
            }
            if (paddata.BTN_START) {
                rodando = 0;
            }
        }

        SDL_FillRect(tela, NULL, SDL_MapRGB(tela->format, 135, 206, 235));

        if (pou) {
            SDL_Rect pos_pou = { 640 - (pou->w / 2), 360 - (pou->h / 2), 0, 0 };
            if (timer_comida > 0) pos_pou.y -= 25;
            SDL_BlitSurface(pou, NULL, tela, &pos_pou);
        }

        if (comida && timer_comida > 0) {
            SDL_Rect pos_comida = { 640 - (comida->w / 2), 380, 0, 0 };
            SDL_BlitSurface(comida, NULL, tela, &pos_comida);
            timer_comida--;
        }

        SDL_Flip(tela);
        SDL_Delay(16);
    }

    if (pou) SDL_FreeSurface(pou);
    if (comida) SDL_FreeSurface(comida);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

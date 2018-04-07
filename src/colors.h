/*! \file colors.h
 * \brief Arquivo de cabeçalho colors.h
 * \author Marcos Paulo Massao Iseki <mm.iseki@gmail.com>
 * \author Rodrigo Albuquerque de Oliveira Siqueira <rodriados@gmail.com>
 * \author Thiago Yasutaka Ikeda <thiagoyasutaka@gmail.com>
 * \date 23 Oct 2014
 *
 * Contém a declaração da biblioteca externa utilizada e das definições
 * de pré-compilação que alteram o aspecto das mensagens impressas no
 * terminal de comando.
 */
#pragma once

#include <cstdio>

#define SPACE "    "

#define foreground(color) FORE##color
#define background(color) BACK##color
#define style(style_) style_

#define FOREBLACK printf("\033[30m") 
#define FORERED printf("\033[31m") 
#define FOREGREEN printf("\033[32m") 
#define FOREYELLOW printf("\033[33m") 
#define FOREBLUE printf("\033[34m") 
#define FOREMAGENTA printf("\033[35m") 
#define FORECYAN printf("\033[36m") 
#define FOREWHITE printf("\033[37m") 
#define FORENORMAL printf("\033[39m") 

#define BACKBLACK printf("\033[40m") 
#define BACKRED printf("\033[41m") 
#define BACKGREEN printf("\033[42m") 
#define BACKYELLOW printf("\033[43m") 
#define BACKBLUE printf("\033[44m") 
#define BACKMAGENTA printf("\033[45m") 
#define BACKCYAN printf("\033[46m") 
#define BACKWHITE printf("\033[47m") 
#define BACKNORMAL printf("\033[49m")

#define BRIGHT printf("\033[1m")
#define DIM printf("\033[2m")
#define NORMAL printf("\033[22m")
#define RESETALL printf("\033[0m")
#define UNDERLINE printf("\033[4m")
#define BLINKSLOW printf("\033[5m")
#define BLINKRAPID printf("\033[6m")
#define ITALIC printf("\033[3m")
#define NEGATIVE printf("\033[7m")
#ifndef GIROUETTE_DRIVER_H
#define GIROUETTE_DRIVER_H


void init_windvane_link(void);
void receive_windvane_usart(void);
// renvoie l'angle de direction du vent absolu (ie : par rapport au nord)
int getGirouette(int ancien, int nord);

#endif
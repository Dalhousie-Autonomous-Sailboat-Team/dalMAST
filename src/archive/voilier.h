#ifndef VOILIER_H
#define VOILIER_H

#define angle_mort 500  //450 //"camembert interdit au près"

char directionNeutre(void);
char voileNeutre(void);
char voilierInit(void);
char tourneD(int val,char droite);
char borderVoile(int val);
void tourneDroite(int angle_bat_cap, char droite); 
void Diriger(int angleBateau,int angleCap, int angleVent); 

#endif
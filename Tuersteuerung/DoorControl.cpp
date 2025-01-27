/*
 * Praktikum MRT2 
 * ART1 Türsteuerung
 * Institut für Automatisierungstechnik
 * letztes Update Mai 2018
 * Autor: M.Herhold
 * Version: r2
 */

#include "DoorInterface.h"
#include "DoorControl.h"
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;


int bw1, bw2, nta, ntz, elo, x1, elg, lsh, lsv, bm, x2, y11, y2, y3;
unsigned char eingang;
int betriebsmodus = 0;
int position;
bool ersterDurchlauf = true;

DoorControl::DoorControl() : door_if(false, true)
{
	// constructor
	// ... maybe, insert your sensor and actor initialization here?
}

DoorControl::~DoorControl()
{
	// destructor
	door_if.quit_doorcontrol_flag = true;
}

void DoorControl::run()
{
	// ... insert your main loop here ...
	// example:
	std::string test;
	std::string msg;		// temporary variable to construct message
	unsigned max_count = 999000;	// loop this often
	unsigned delay_ms = 20;		// Milliseconds to wait on one loop
	
	for(unsigned i=0; !door_if.quit_doorcontrol_flag && i < max_count; i++){
		DoorControl::Sensoren_lesen();
		DoorControl::getBetriebsmodus();

		switch(betriebsmodus){
		case 0: DoorControl::Aus();
		case 1: DoorControl::Reparatur();
		case 2: DoorControl::Handbetrieb();
		case 3: DoorControl::Automatik();
		//default: DoorControl::Aus();
		}

		if(x2 == 1) test = " xyz";
		else test = " zyx";

		//construct counter message
		msg = "press 'q', or wait ";
		msg += std::to_string((int)((max_count-i) / (1000/delay_ms) + 1));
		msg += " seconds to quit";
		msg += test;
		door_if.DebugString(msg);

		// wait some time
		usleep(delay_ms * 1000);

	}
}
void DoorControl::Sensoren_lesen(){
	int temp;
	unsigned int port = 0;
	door_if.DIO_Read(port, &eingang);
	temp = eingang & 1;
	if(temp == 1) bw1 = 1; else bw1 = 0;
	temp = eingang & 2;
	if(temp == 2) bw2 = 1; else bw2 = 0;
	temp = eingang & 4;
	if(temp == 4) nta = 1; else nta = 0;
	temp = eingang & 8;
	if(temp == 8) ntz = 1; else ntz = 0;
	temp = eingang & 16;
	if(temp == 16) elo = 1; else elo = 0;
	temp = eingang & 32;
	if(temp == 32) x1 = 1; else x1 = 0;
	temp = eingang & 64;
	if(temp == 64) elg = 1; else elg = 0;
	temp = eingang & 128;
	if(temp == 128) lsh = 1; else lsh = 0;

	port = 1;
	door_if.DIO_Read(port, &eingang);

	temp = eingang & 1;
	if(temp == 1) lsv = 1; else lsv = 0;
	temp = eingang & 2;
	if(temp == 2) bm = 1; else bm = 0;
	temp = eingang & 4;
	if(temp == 4) x2 = 1; else x2 = 0;

}

void DoorControl::outputY(){
	if (y11 == 0 && y2 == 0 && y3 == 0){				//Wenn alle int 0 dann output 0 (alles aus)
			door_if.DIO_Write(2,0);
		}
	else if (y11 == 0 && y2 == 0 && y3 == 1){				//Wenn nur int c 1 dann output 4 (Lampe blinkt sonst alles aus)
				door_if.DIO_Write(2,4);
		}
	else if (y11 == 1 && y2 == 0 && y3 == 1){				//Wenn nur int b 0 dann output 5 (Motor öffnen läuft Lampe blibkt)
			door_if.DIO_Write(2,5);
		}
	else if (y11 == 0 && y2 == 1 && y3 == 1){				//Wenn nur int a 0 dann output 6 (Motor schließen läuft Lampe blinkt)
			door_if.DIO_Write(2,6);
		}
}

void DoorControl::getBetriebsmodus() {

	// 0. aus
	if (!bw1 && !bw2) {
		betriebsmodus = 0;
	}
	// 1. Reparatur
	if (!bw1 && bw2) {
		betriebsmodus = 1;
	}
	// 2. Handbetrieb
	if (bw1 && !bw2) {
		betriebsmodus = 2;
	}
	// 3. Automatik
	if (bw1 && bw2) {
		betriebsmodus = 3;
	}
	return;
}

void DoorControl::getPosition() {

	// 1. Tür offen.
	if (elo) {
		position = 1;
	}
	// 2. Tür weder offen noch geschlossen.
	else if (!elo && !elg) {
		position = 2;
	}
	// 3. Tür geschlossen
	else if (elg) {
		position = 3;
	}
	else position = 0;
	return;
}

void DoorControl::Aus(){
	//ersterDurchlauf  = true;
	y11 = 0;
	y2 = 0;
	y3 = 0;
	DoorControl::outputY();
	return;
}
void DoorControl::Handbetrieb(){
	//ersterDurchlauf = true;
	y11 = 0;
	y2 = 0;
	y3 = 0;
	DoorControl::outputY();
	while(nta == 1 && elo ==0 && ntz == 0 && betriebsmodus == 2){ //wenn nta gedrückt aber tür noch nihct koplett offen(elo = 0)
		y11 = 1;				//Moter zum aufmachen läuft
		y2 = 0;					//Moter zum zumachen ist aus
		y3 = 1;					//Lampe leuchtet

		DoorControl::outputY();
		DoorControl::Sensoren_lesen();
		DoorControl::getBetriebsmodus();
		}
	while(ntz == 1 && elg == 0 && nta == 0 && betriebsmodus == 2){ //wenn nta gedrückt aber tür noch nihct koplett offen(elo = 0)
		y11 = 0;					//Moter zum aufmachen ist aus
		y2 = 1;					//Moter zum zumachen läuft
		y3 = 1;					//Lampe leuchtet

		DoorControl::outputY();
		DoorControl::Sensoren_lesen();
		DoorControl::getBetriebsmodus();
	}
	return;
}
void DoorControl::Reparatur(){
	y11 = 0;
	y2 = 0;
	y3 = 0;
	DoorControl::outputY();
	while(!elo && (lsv || lsh || bm || nta) && betriebsmodus == 1){ // Tür vollständig öffnen
		y11 = 1;
		y2 = 0;
		y3 = 1;
		DoorControl::outputY();
		DoorControl::Sensoren_lesen();
		DoorControl::getBetriebsmodus();
		//sleep(3); //TODO: resetbarer Timer
	}
	while(betriebsmodus == 1 && ntz && !elg){ // Tür schließen
		y11 = 0;
		y2 = 1;
		y3 = 1;
		DoorControl::outputY();
		DoorControl::Sensoren_lesen();
		DoorControl::getBetriebsmodus();
	}
return;
}
void DoorControl::Automatik(){
	y11 = 0;
	y2 = 0;
	y3 = 0;
	DoorControl::outputY();
	if(ersterDurchlauf){ //TODO: funktioniert semi
		ersterDurchlauf = false;
		y11 = 0;
		y2 = 0;
		y3 = 1;
		DoorControl::outputY();
		sleep(5);
		y3 = 0;
		DoorControl::outputY();
	}
	DoorControl::Sensoren_lesen();
	DoorControl::getPosition();
	DoorControl::getBetriebsmodus();
	while(!elo && (lsv || lsh || bm || nta) && betriebsmodus == 3){ // Tür vollständig öffnen
		while(!elo){
			y11 = 1;
			y2 = 0;
			y3 = 1;
			DoorControl::outputY();
			DoorControl::Sensoren_lesen();
			DoorControl::getBetriebsmodus();
		}
		y11 = 0;
		y2 = 0;
		y3 = 0;
		DoorControl::outputY();
		sleep(3); //TODO: resetbarer Timer
	}
	if(elo){  // Tür schließen
		while((!elg && !lsv && !lsh && !bm && !nta && betriebsmodus == 3) || ntz){ //TODO: ntz macht noch nicht so wirklich, was er soll
			y11 = 0;
			y2 = 1;
			y3 = 1;
			DoorControl::outputY();
			DoorControl::Sensoren_lesen();
			DoorControl::getBetriebsmodus();
		}
		y11 = 0;
		y2 = 0;
		y3 = 0;
		DoorControl::outputY();
	}
	/*
	while(position == 1 && betriebsmodus == 3){ //tür offen

		/*
		auto startTime = chrono::system_clock::now();
		auto neueZeit = chrono::system_clock::now();
		chrono::duration<double> elapsedTime = neueZeit - startTime;
		auto numSeconds = elapsedTime.count();
		while(numSeconds < wartezeit){
			DoorControl::Sensoren_lesen();
			if(lsh || lsv || bm || nta){
				startTime = chrono::system_clock::now();
			}
			neueZeit = chrono::system_clock::now();
			numSeconds = elapsedTime.count();
		}

		//sleep(3);
	}
	while(position == !3  && lsh==0 && lsv==0 && bm==0 && nta==0 && betriebsmodus == 3){ // Tuer schließen un auf Sensoren achten
	y11=0;				//moter zum öffnen aus
	y2=1;				//Motor zum zumachen an
	y3=1;				//Lampe an
	DoorControl::outputY();
	DoorControl::Sensoren_lesen();
	DoorControl::getBetriebsmodus();
	DoorControl::getPosition();
	/*
	if(nta == 1|| lsh == 1 || lsv == 1 || bm == 1){		//falls input dann
		ist = 4;										//öffnen
	}

	}*/

	return;
}

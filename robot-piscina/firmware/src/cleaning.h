// cleaning.h — spazzola laterale e pompa skimmer (DRV8833 #2).
#pragma once

void cleaningBegin();
void brushSet(bool on);
void pumpSet(bool on);
void cleaningStop();   // ferma spazzola e pompa

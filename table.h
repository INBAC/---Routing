#pragma once

typedef struct{
char destIP[16];
int destPort;
char hopIP[16];
int hopPort;
int metric;
}tableEntry;

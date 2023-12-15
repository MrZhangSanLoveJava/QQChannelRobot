#ifndef __QQSERVER_H
#define __QQSERVER_H

#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <json-c/json.h>

#define URL "https://bots.qq.com/app/getAppAccessToken"
#define USER_URL "https://api.sgroup.qq.com"

#define SIZE 10

void run();


#endif


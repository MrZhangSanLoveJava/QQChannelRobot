#include <qqServer.h>

CURL* curl = NULL;
char proof[64];       // 凭证信息
char proof_time[5];         // 到期时间

typedef struct DataStruct{
	char* memroy;
	size_t size;
}Data;

typedef struct GetChannelData{
	char id[128];       // 频道ID
	char name[128];     // 频道名称
	char icon[128];     // 频道头像
	char owner_id[128]; // 创建人用户ID
	char owner[2];	    // 当前人是否是创建人
	char member_cont[10];    // 成员数
	char max_members[10];    // 最大成员数
	char description[128];	// 描述
	char joined_at[128];    // 加入时间
}channeData;

typedef struct GetSubchannelData{
	char id[64];              // 频道ID
	char guild_id[64];        // 父频道ID
	char name[128];
}subchannel;

// 回调函数
size_t get_token_back(char* buff, size_t size, size_t nmend, void* userp){
	size_t bytes = size * nmend;
	Data* data = (Data*)userp;
	data->memroy = realloc(data->memroy, data->size + bytes + 1);
	if (data->memroy == NULL){
		printf("Not enough memory\n");
		return -2;
	}
	memcpy(&(data->memroy[data->size]), buff, bytes);
	data->size += bytes;
	data->memroy[data->size] = 0;
	return bytes;
}

// 初始化init
char curl_init(){
	
	curl = curl_easy_init();
	if (!curl){
		printf("初始化失败");
		return -1;
	}
	return 0;
}

// 解析凭证 也没用
void analysis_proof(const char* data){
	int i, j = 0, flag = 0;
	for (i = 0; data[i] != '\0'; i++){
		if (data[i] == ':' && data[i + 1] == '\"' && flag == 0){
			for (j = 0, i += 2; data[i] != '\"'; j++, i++)
				proof[j] = data[i];
			proof[j] = '\0';
			flag = 1;
		}
		if (data[i] == ':' && data[i + 1] == '\"' && flag){
			for (j = 0, i += 2; data[i] != '\"'; j++, i++)
				proof_time[j] = data[i];
			proof_time[j] = '\0';
		}
	}
	printf("proof=%s\n", proof);
	printf("time=%s\n", proof_time);
}

// 解析频道中的子频道信息
// TODO:明天记得写完
int analysis_subchannel_data(const char* data, subchannel* channel[]){
	struct json_objetc* json = NULL, *temp = NULL, *getData = NULL;
	int i, len;    // 遍历变量
	subchannel* myPoint = NULL;

	json = json_tokener_parse(data);     // 解析为json数据
	len = json_object_array_length(json);
	printf("4:%s\n", json_object_get_string(temp));
	for (i = 0; i < len; i++){
		myPoint = (subchannel*)malloc((sizeof(subchannel)));
		temp = json_object_array_get_idx(json, i);      // 获取里面的内容
		json_object_object_get_ex(temp, "id", &(getData));
		strcpy(myPoint->id, json_object_get_string(getData));
		json_object_object_get_ex(temp, "guild_id", &(getData));
		strcpy(myPoint->guild_id, json_object_get_string(getData));
		json_object_object_get_ex(temp, "name", &(getData));
		strcpy(myPoint->name, json_object_get_string(getData));
		channel[i] = myPoint;
	}
	return len;       // 返回一下长度
}

/* 
   请求URL封装
	url:请求的url
	flag:1是post
		 0是get
	reqD:请求的内容
*/
char* request(const char* url, const char flag, const char* reqD, struct crl_slist* list){
	// 请求最终的字符串
	char buf_url[128];
	int res;
	Data back_data;      // 接收内容
	back_data.memroy = malloc(1);
	back_data.size = 0; 
	sprintf(buf_url, "%s%s", USER_URL, url);          
	curl_easy_setopt(curl, CURLOPT_URL, buf_url);     // 请求的url
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list); // 请求的头
	curl_easy_setopt(curl, CURLOPT_POST, flag);       // POST还是GET请求
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, reqD);  // 发送的内容
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_token_back);    // 使用回调函数来接收数据
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&back_data);
	// 执行
	res = curl_easy_perform(curl);
	if (res != CURLM_OK){
		printf("error:%d\n", res);
		return "error";
	}
	return back_data.memroy;
}

// 解析所在频道
channeData* analysis_channel(const char* data){
	channeData* channel = (channeData*)malloc(sizeof(channeData));      // 直接开辟内存
	// 直接用json-c解决
	struct json_objetc* json = NULL, *temp = NULL, *content = NULL;
	json = json_tokener_parse(data);
	// 获取ID
	temp = json_object_array_get_idx(json, 0);

	// 获取内容 很简单
	json_object_object_get_ex(temp, "id", &content);
	strcpy(channel->id, json_object_get_string(content));

	json_object_object_get_ex(temp, "name", &content);
	strcpy(channel->name, json_object_get_string(content));

	json_object_object_get_ex(temp, "icon", &content);
	strcpy(channel->icon, json_object_get_string(content));

	json_object_object_get_ex(temp, "owner_id", &content);
	strcpy(channel->owner_id, json_object_get_string(content));

	json_object_object_get_ex(temp, "owner", &content);
	strcpy(channel->owner, json_object_get_string(content));

	json_object_object_get_ex(temp, "joined_at", &content);
	strcpy(channel->joined_at, json_object_get_string(content));

	json_object_object_get_ex(temp, "member_count", &content);
	strcpy(channel->member_cont, json_object_get_string(content));

	json_object_object_get_ex(temp, "max_members", &content);
	strcpy(channel->max_members, json_object_get_string(content));

	json_object_object_get_ex(temp, "description", &content);
	strcpy(channel->description, json_object_get_string(content));
	
	// TAG:可以删除
	printf("id=%s\nname=%s\nicon=%s\nowner_id=%s\nowner=%s\njoined_at=%s\nmember_count=%s\nmax_members=%s\ndescription=%s\n", channel->id,channel->name, channel->icon, channel->owner_id, channel->owner,channel->joined_at, channel->member_cont, channel->max_members,channel->description);
	return channel;
}

// 获取凭证 一点用都没有，但是不舍得删除
void get_access_token(const char* appid, const char* client){
	int res;
	struct curl_slist* headers = NULL;
	char buf[128];
	Data back_data;
	back_data.memroy = malloc(1);
	back_data.size = 0;
	
	// 拼接请求内容
	sprintf(buf, "{\"appId\":\"%s\", \"clientSecret\":\"%s\"}", appid, client);

	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);    // 给协议头
	curl_easy_setopt(curl, CURLOPT_URL, URL);      // 设置URL
	curl_easy_setopt(curl, CURLOPT_POST, 1);         // 发送post请求
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);  // 发送json
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_token_back);    // 使用回调函数来接收数据
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&back_data);
	// 执行
	res = curl_easy_perform(curl);
	if (res != CURLM_OK){
		printf("error:%d\n", res);
	}
	curl_slist_free_all(headers);
	printf("back_data=%s\n", back_data.memroy);
	analysis_proof(back_data.memroy);
}

// 获取机器人加入的频道
channeData* get_roobt_id(const char* appid, const char* token){
	char url[124];
	char buf[1024];
	Data back_data;
	back_data.memroy = malloc(1);
	back_data.size = 0;
	int ret;
	struct curl_slist* heade = NULL;
	channeData* idData;        // 读取的信息的地址

	sprintf(url, "/users/@me/guilds");
	// 拼接请求头
	sprintf(buf, "Contene-Type: application/json");
	heade = curl_slist_append(heade, buf);
	sprintf(buf, "Authorization: Bot %s.%s", appid, token);
	heade = curl_slist_append(heade, buf);

	back_data.memroy = request(url, 0, NULL, heade);   // 发送请求
	printf("data_id:%s\n", back_data.memroy);   // 输出请求的结果
	curl_slist_free_all(heade);
	idData = analysis_channel(back_data.memroy);        // 解析json字符串
	// 释放
	free(back_data.memroy);
	return idData;
}

// 获取子频道id
int get_subchannel_id(const char* appid, const char* token, const char* father, subchannel* getsubchannel[]){
	Data back_data;
	back_data.memroy = malloc(1);
	back_data.size = 0;
	struct curl_slist* head = NULL;
	char buf[1024];
	char user_fun_url[1024];
	int i, len;

	// 拼接请求头
	sprintf(buf, "Contene-Type: application/json");
	head = curl_slist_append(head, buf);
	sprintf(buf, "Authorization: Bot %s.%s", appid, token);
	head = curl_slist_append(head, buf);

	sprintf(user_fun_url, "/guilds/%s/channels", father);	  	// 拼接url
	
	back_data.memroy = request(user_fun_url, 0, NULL, head);     // 发送请求

	printf("subchannel_id=%s\n", back_data.memroy);

	len = analysis_subchannel_data(back_data.memroy, getsubchannel);       // 解析json

	return len;
}

// 发送私信
void send_private(){
	
}

// 向子频道发送信息
void send_channel_data(const char* appid, const char* token, const char* data, const char* name){
	Data back_data;
	back_data.memroy = malloc(1);
	back_data.size = 0;
	struct curl_slist* head = NULL;
	char buf[1024];
	char user_fun_url[256];
	char send_data[128];
	channeData* channeldata;
	subchannel* getsubchannel[SIZE];
	int i, len;

	// 获取机器人所在频道的id
	channeldata = get_roobt_id(appid, token);

	len = get_subchannel_id(appid, token, channeldata->id, getsubchannel);       // 获取子频道

	for (i = 0; i < len; i++){
		if (strcmp(name, getsubchannel[i]->name) == 0){
			// 拼接url
			sprintf(user_fun_url, "/channels/%s/messages", getsubchannel[i]->id);	  
			printf("user_fun_url=%s\n", user_fun_url);
		}
	}
	
	// 拼接请求头
	sprintf(buf, "Contene-Type: application/json");
	head = curl_slist_append(head, buf);
	sprintf(buf, "Authorization: Bot %s.%s", appid, token);
	head = curl_slist_append(head, buf);

	sprintf(send_data, "{\"content\": \"%s\", \"msg_id\":\"\"}", data);              		  // 拼接发送的数据
	
	printf("%s\n", send_data);
	back_data.memroy = request(user_fun_url, 1, send_data, head);     // 发送请求
	printf("%s\n", back_data.memroy);

	for (i = 0; i < len; i++){
		// printf("id=%s\nguild_id=%s\nname=%s\n", getsubchannel[i]->id, getsubchannel[i]->guild_id, getsubchannel[i]->name);
		free(getsubchannel[i]);      // 释放
	}
	free(channeldata);      // 用完就释放
}

// 执行
void run(){
	if (curl_init() == 0){
		// get_access_token("102060772", "jZRI90riYOF6yqic");
		send_channel_data("102060772", "ozl7CU61aNjYOwEg8RUOqW1MuKWspFMp", "hello", "聊天室");
	}
	curl_easy_cleanup(curl);
}

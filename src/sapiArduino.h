#ifndef sapiArduino_h 
#define sapiArduino_h

#define LOGIN_OK 0
#define LOGIN_WRONG 2
#define WiFi_OFF 1
#define GENERIC_ERROR 3

//session
typedef struct{
  String key;
  String jsonid;
} Session;

typedef struct{
	String name;
	String type;
	const char* content;
	long length;	
	String date;
} FileInfo;

int doLogin(const char* username, const char* password, Session* login);

int uploadBuffer(Session login, const char* buffer, long length);

String buildMultipart(String boundary, FileInfo info);

String Timer();

void SaveData(String date,String weight);
#endif

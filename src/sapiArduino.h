#ifndef sapiArduino_h 
#define sapiArduino_h

enum {
	LOGIN_OK,
	AUTENTICATION_FAILED,
	WiFi_NOT_CONNECTED,
	GENERIC_ERROR,
	JSESSIONID_MISS,
	AUTENTICATION_KEY_NOT_FOUND,
} ResponseCod;



//session
typedef struct{
  String key;
  String jsonid;
} Session;

typedef struct{
	String name;
	String type;
	String content;
	long length;	
	String date;
} FileInfo;

int doLogin(const char* username, const char* password, Session* login);

int uploadBuffer(Session login, const char* buffer, long length);

int uploadFile(Session login, FileInfo file);

int resumableUploadFile(Session login, FileInfo file);

String sendMetadata(Session login, FileInfo file, String Id);

int saveFile(Session log, FileInfo file, String Id);

void storageId(String Id);

String readId();

#endif

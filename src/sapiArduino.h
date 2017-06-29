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
}session;

int doLogin(const char* username, const char* password, session* login);
#endif

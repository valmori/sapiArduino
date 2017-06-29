#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>

static String createCred(const char* id, const char* pass);
static String getJsessionid(String line);
static String getValidationkey(String line);
int getStatusCode(String line);

//do the login at onemediahub.com, get the jsessionid, validationkey and returns a number code 0, if the login is happened successfully, 1 failed to stabiliz connetion with the host
//2 username or password is wrong, 3 anexatly error in response from server
int doLogin(const char* username, const char* password, session* login){
  WiFiClientSecure client;
  const char* host = "onemediahub.com";
  const int httpsPort = 443;
  if(!client.connect(host, httpsPort)){
  	return WiFi_OFF;
  }
  String user = createCred(username, password);
  String url = "/sapi/login?action=login";
  String request = "POST " + url + " HTTP/1.1\r\n" + 
               "Host: " + host + "\r\n" +                     
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + user.length() + "\r\n" +
               "Connection: close\r\n" +
               "\r\n"+
               
               user;
  client.print(request);
  String line= "";
  String control = "p";
  while (client.connected()||(control!=line)) {
    control = line;
    line += client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  client.read();
  login->jsonid = getJsessionid(line);
  login->key = getValidationkey(line);
  return getStatusCode(line);
}

//create the login=username&password=account-infomation
static String createCred(const char* id, const char* pass){
  String login="login=";
  String e="&";
  String other="password=";
  String postData;
  postData = postData + login + id + e + other + pass;
  return postData; 
}

static String getJsessionid(String line){
	String token = "\"jsessionid\":\"";
	int index = line.indexOf("\"jsessionid\":\"" );
	index += token.length();
	int endindex = line.indexOf("\"", index);
	return line.substring(index,endindex);
}

static String getValidationkey(String line){
	String token = "\"validationkey\":\"";
	int index = line.indexOf("\"validationkey\":\"" );
	index += token.length();
	int endindex = line.indexOf("\"", index);
	return line.substring(index,endindex); 
}

int getStatusCode(String line){
	String token = "HTTP/1.1 ";
	token = line.substring(token.length(),(token.length()+3)); 
	switch(token.toInt()){
		case 200:
			return LOGIN_OK;
		break;
		case 401:
			return LOGIN_WRONG;
		break
		default:
			return GENERIC_ERROR;
		break;
	}
}

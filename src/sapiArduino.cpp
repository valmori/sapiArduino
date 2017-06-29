#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <string.h>
#include <sapiArduino.h>



static String createCred(const char* id, const char* pass);
static String getJsessionid(String line);
static String getValidationkey(String line);
static int getStatusCode(String line);
static String buildMultipart(String boundary, FileInfo info);

//do the login at onemediahub.com, get the jSessionid, validationkey and returns a number code 0, if the login is happened successfully, 1 failed to stabiliz connetion with the host
//2 username or password is wrong, 3 anexatly error in response from server
int doLogin(const char* username, const char* password, Session* login){
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

int uploadBuffer(Session login, const char* buffer, long length){	
	FileInfo file = {"NoName.txt", "text/plain", buffer, length};	
	return uploadFile(login, file);
}

int uploadFile(Session login, FileInfo file){
	const char* host = "onemediahub.com";
	const int httpsPort = 443;
	WiFiClientSecure client;
	if(!client.connect(host, httpsPort)){
		return WiFi_OFF;
	}	
	String url = "/sapi/upload?action=save&validationkey=" + login.key;
	String boundary = "46w9f0apovnw23951faydgi";
	String body = buildMultipart(boundary,file); 
 	String request = String("POST ") + url + " HTTP/1.1\r\n" + //rfc http 1.1
               "Host: " + host + "\r\n" +    
               "Cookie: JSESSIONID=" + login.jsonid + "\r\n" + 
               "Content-Type: multipart/form-data;boundary=" + boundary + "\r\n" +
               "Content-Length: " + body.length() + "\r\n" +
               "Connection: close" + "\r\n" +
               "\r\n"+
               
               body;
    client.print(request);
    String line= "";
    String control = "p";
    while ((control!=line)) {
	    control = line;
	    line += client.readStringUntil('\n');
	    if (line == "\r") {
	      break;
	    }
	}
    client.read();
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
		break;
		default:
			return GENERIC_ERROR;
		break;
	}	
}


String buildMultipart(String boundary, FileInfo info){
	String multipart;
	multipart += "--" + boundary + "\r\n" +
				"Content-Disposition: form-data; name=\"data\"\r\n\r\n" +	
				"{" +
				    "\"data\":{" + 
						"\"name\": \"" + info.name + "\"," +
						"\"creationdate\": "+ info.date + "," +
						"\"modificationdate\": "+ info.date + "," +
						"\"contenttype\":\"text/plain\"," +
						"\"size\":" + info.length + "," + 
						"\"folderid\":-1" +
				    "}" +
				"}\r\n" +
              
              "--" + boundary + "\r\n" +
              "Content-Disposition: form-data; name=\"file\"; filename=\"" + info.name + "\"\r\n" +
			  "Content-Type: \"" + info.type + "\"\r\n\r\n" +
			  info.content + "\r\n" +
              "--" + boundary + "--";
  return multipart;
}


String Timer(){
	
   dateTime=NTPch.getNTPtime(1.0, 1);
   NTPch.printDateTime(dateTime);
   byte actualHour = dateTime.hour;
   byte actualMinute = dateTime.minute;
   byte actualsecond = dateTime.second;
   int actualyear = dateTime.year;
   byte actualMonth = dateTime.month;
   byte actualday =dateTime.day;
   String date="";
   date = date + String(actualHour)+ String(actualMinute) + String(actualsecond) + String(actualyear) + String(actualMonth) + String(actualday);
   return date;

}

void SaveData(String date,String weight){
  int addr=0,addd;
  int i=0;
  EEPROM.commit();
  for (addr=0;addr<date.length();addr++) {
    EEPROM.write(addr,date[addr]);  
    Serial.print(date[addr]);
  }
  for(addd=addr;addd<weight.length()+addr;addd++){
    EEPROM.write(addd,weight[i]);    
    Serial.print(weight[i]);
    i++;
  }
  Serial.println();
  EEPROM.end();
}

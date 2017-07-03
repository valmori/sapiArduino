#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <string.h>
#include <sapiArduino.h>
#include <EEPROM.h>


static String createCred(const char* id, const char* pass);
static String getJsessionid(String line);
static String getValidationkey(String line);
static int getStatusCode(String line);
static String buildMultipart(String boundary, FileInfo info);
static String createMetadata(FileInfo info, String Id);
static String getId(String line);
static String createGetBody(String Id);
static String getUrl(line);
String sendMetadata(Session login, FileInfo file, String Id);
int saveFile(Session log, FileInfo file, String Id);

//do the login at onemediahub.com, get the jSessionid, validationkey and returns a number code 0, if the login is happened successfully, 1 failed to stabiliz connetion with the host
//2 username or password is wrong, 3 anexatly error in response from server
int doLogin(const char* username, const char* password, Session* login){
	WiFiClientSecure client;
	const char* host = "onemediahub.com";
	const int httpsPort = 443;
	if(!client.connect(host, httpsPort)){
		return WiFi_NOT_CONNECTED;
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
  if(login->jsonid == NULL){
  	return JSESSIONID_MISS;
  }
  if(login->key == NULL){
  	return AUTENTICATION_KEY_NOT_FOUND;
  }
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
		return WiFi_NOT_CONNECTED;
	}	
	String url = "/sapi/upload?action=save&validationkey=" + login.key;
	String boundary = "46w9f0apovnw23951faydgi";
	String body = buildMultipart(boundary,file); 
	//perché non fare una funzione per la request?
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

int resumableUploadFile(Session login, FileInfo file){  
	int statusCode;
	String Id = sendMetadata(login, file, "");
	if (Id != NULL){
		statusCode = saveFile(login, file, Id);
	}
	return statusCode;
}

int dowloadWithId(String Id, FileInfo* file, Session login){
	WiFiClientSecure client;
	const char* host = "onemediahub.com";
	const int httpsPort = 443;
	if(!client.connect(host, httpsPort)){
		return WiFi_NOT_CONNECTED;
	}
	String body = createGetBody(Id);
	String url = "/sapi/media?action=get&origin=omh&validationkey=" + login.key;
	String request = "POST " + url + " HTTP/1.1\r\n" + 
	           "Host: " + host + "\r\n" +                     
	           "Cookie: JSESSIONID=" + login.jsonid + "\r\n" +
	           "Content-Type: application/json \r\n" + 
	           "Content-Length: " + body.length() + "\r\n" +
	           "Connection: close\r\n" +
	           "\r\n"+
	           
	           body;
	client.print(request);
	Serial.println(request);
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
  Serial.println("download: ");
  Serial.println(line);
  return getUrl(line);	
}

String fileGet(String url){
	WiFiClientSecure client;
	String request;
	const char* host = "onemediahub.com";
	const int httpsPort = 443;
	if(!client.connect(host, httpsPort)){
		return WiFi_NOT_CONNECTED;
	}
	request += String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
	client.print(request);
	Serial.println(request);
	//<.--------------------------------------------------------------------------------------------------
	//controllare la risposta come gestirla, cosa contiene, ecc..
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

String getUrl(line){
	String token = "\"url\":\"";
	int index = line.indexOf("\"url\":\"" );
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
			return AUTENTICATION_FAILED;
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
						"\"name\":\"" + info.name + "\"," +
						"\"creationdate\":\""+ info.date + "\"," +
						"\"modificationdate\":\""+ info.date + "\"," +
						"\"contenttype\":\"" + info.type + "\"," +
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

String sendMetadata(Session login, FileInfo file, String Id){ 
	WiFiClientSecure client;
	const char* host = "onemediahub.com";
	const int httpsPort = 443;
	if(!client.connect(host, httpsPort)){
		return "no log";
	}
	String url = "/sapi/upload/file?action=save-metadata&validationkey=" + login.key;
	String body = createMetadata(file, Id);
	String request = String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +    
               "Cookie: JSESSIONID=" + login.jsonid + "\r\n" + 
               "Content-Type: application/json\r\n" +
               "Content-Length: " + body.length() + "\r\n" +
               "Connection: close" + "\r\n" +
               "\r\n"+
               
               body;
    client.print(request);
    Serial.println(request);
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
	Serial.println(line);
    return getId(line);
}

String createMetadata(FileInfo info, String Id){
	String metadata;
	metadata += "{";
	metadata +=     "\"data\":{";
	metadata +=         "\"name\":\"" + info.name + "\"," +
						Id +
						"\"creationdate\":\""+ info.date + "\"," +
						"\"modificationdate\":\""+ info.date + "\"," +
						"\"contenttype\":\"" + info.type + "\"," +
						"\"size\":" + info.length + "," + 
						"\"folderid\":-1" +
				    "}" +
				"}";
	return metadata;
}

String getId(String line){
	String token = "\"id\":\"";
	int index = line.indexOf("\"id\":\"" );
	index += token.length();
	int endindex = line.indexOf("\"", index);
	return line.substring(index,endindex); 
}

int saveFile(Session log, FileInfo file, String Id){
	WiFiClientSecure client;
	const char* host = "onemediahub.com";
	const int httpsPort = 443;
	if(!client.connect(host, httpsPort)){
		return WiFi_NOT_CONNECTED;
	}
	String url = "/sapi/upload/file?action=save&validationkey=" + log.key;
	String request = String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
			   "x-funambol-id: " + Id + "\r\n" +
			   "x-funambol-file-size:" + file.content.length() + "\r\n" +
               "Cookie: JSESSIONID=" + log.jsonid + "\r\n" + 
               "Content-Type: text/plain\r\n" +
               "Content-Length: " + file.content.length() + "\r\n" +
               "Connection: close" + "\r\n" +
               "\r\n"+
               
               file.content;
    client.print(request);
    Serial.print(request);
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
    Serial.print(line);
    if(line.indexOf("\"MED-1000\"" )>0){
    	return 1000;
	}
    return 0;
}

String createGetBody(String Id){
	String request;
	request += 	    "{";
	request +=			"\"data\":{";
	request +=		    "\"ids\":[\"" + 
					    	Id +
 					    "\"]," +
 					    "\"fields\":[" +
 					    	"\"url\"" +
						 "]" + 
					  "}"+
					"}";
	return request;
}

void storageId(String Id){
	for(int a = 0; a < Id.length(); a++){
		EEPROM.write(a, Id[a]);
	}
	EEPROM.commit();
}

String readId(){
	char Id[20];
	int a = 0;
	Id[a] = EEPROM.read(a);
	while(int(Id[a]) != 0){
		a++;
		Id[a] = EEPROM.read(a);
	}
	return Id;
} 

#include <Adafruit_CC3000.h>
#include <SPI.h>
//things you need to change
#define     WLAN_SSID              "your wifi name"      // cannot be longer than 32 characters!
#define     WLAN_PASS              "your wifi password"
#define     WLAN_SECURITY          WLAN_SEC_WPA2  // Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define     LOG_TOKEN              "token from logentries.com"
//end of things you need to change

#define     ADAFRUIT_CC3000_IRQ    3    // MUST be an interrupt pin!
#define     ADAFRUIT_CC3000_VBAT   5    // VBAT and CS can be any two pins
#define     ADAFRUIT_CC3000_CS     10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, 
                                         ADAFRUIT_CC3000_IRQ, 
                                         ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2);
Adafruit_CC3000_Client  logger;


char LOG_HOST[20] = "data.logentries.com";
uint32_t log_ip = 0;
uint32_t t;
const unsigned long connectTimeout  = 15L * 1000L;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(5000);
  
  if (!cc3000.begin()) {
    Serial.println("Unable to initialize the CC3000");
    
    while(1) { }
  }
  Serial.println("CC3000 initialized");
  
  if (!cc3000.deleteProfiles()) {
    Serial.println("CC3000 Failed to delete old profiles");
    while(1){ }
  }  
  Serial.println("CC3000 deleted old profiles");    
  
  uint16_t firmware = checkFirmwareVersion();
  if (firmware < 0x113) {
    Serial.println(F("Wrong firmware version!"));
  }   
  
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println("Failed to connect to AP");
    while(1){}
  }  
  Serial.println("CC3000 connected to AP");
  
  while (!cc3000.checkDHCP()) {
    delay(200);
  }
  Serial.println("CC3000 DHCP acquired");  
   
  while (! displayConnectionDetails()) {}  

  log_ip = 0;
  while (log_ip == 0) {
    if (!cc3000.getHostByName(LOG_HOST, &log_ip))  {
      Serial.println("Couldn't resolve LogEntries endpoint");
      while(1){}
    }
    delay(500);
 }
  Serial.print("Resolved LogEntries IP -- ");
  cc3000.printIPdotsRev(log_ip);

  Serial.println("");    
  Serial.println("Initialization complete");
  Log("Initialization complete");
  Serial.println("---------");
}


void Log(char logEntry[100]){
  t = millis();
  if (!logger.connected()){ 
    Serial.println("Connecting logger");
    do {
      logger = cc3000.connectTCP(log_ip, 80);
    } while ((!logger.connected()) && ((millis() - t) < connectTimeout));
  }
  Serial.println("logger connected");
  
  if (logger.connected()) {
    Serial.println("logging");
    logger.fastrprint(LOG_TOKEN);
    Serial.print(LOG_TOKEN);
    logger.fastrprint(" ");
    logger.fastrprintln(logEntry);
    Serial.println(logEntry);
    logEntry[0] = 0;
  }
}

void loop(){
  Log("In the loop");
  delay(10000);
}

bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to display IP information. \r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

uint16_t checkFirmwareVersion(void)
{
  uint8_t major, minor;
  uint16_t version;
  
#ifndef CC3000_TINY_DRIVER  
  if(!cc3000.getFirmwareVersion(&major, &minor))
  {
    Serial.println(F("Unable to retrieve the firmware version!\r\n"));
    version = 0;
  }
  else
  {
    Serial.print(F("Firmware V. : "));
    Serial.print(major); Serial.print(F(".")); Serial.println(minor);
    version = major; version <<= 8; version |= minor;
  }
#endif
  return version;
}

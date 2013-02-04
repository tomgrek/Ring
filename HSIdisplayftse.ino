#include <SPI.h>
#include <Ethernet.h>
#include "HT1632.h"
#include "TimerOne.h"

byte mac[] = {  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x06 };
char serverName[] = "www.google.com";
String textToScroll = "";
String nextText;
String tempString = ""; //else setting nexttext will skip some if too slow
String tempString2 = ""; //from detected l_cur" or c : " onwards - so we can detect the next " and hence get the number between "" regardless of its no. of digits
int lengthTextToScroll = 0;
int currentX = 32;

int x = 0; int y = 0; //location of HSI price and change

int whereAmI = 0;

EthernetClient client;
HT1632LEDMatrix matrix = HT1632LEDMatrix(2,3,4); //DATA,WR,CS

String returnedHTML;
String uparrow = ""; String downarrow = "";

void setup() {
  if (Ethernet.begin(mac) == 0) {
    nextText = "Failed to config ethernet";
    while(true);
  }
  matrix.begin(HT1632_COMMON_8NMOS);
  matrix.fillScreen();
  matrix.setTextSize(1);
  matrix.setTextColor(1);
  Timer1.initialize(100000);
  uparrow += char(24);
  downarrow += char(25);
  textToScroll = "Hello";
  lengthTextToScroll = (-6*textToScroll.length())+2;
  nextText = textToScroll;
  delay(2000);
  Timer1.attachInterrupt(scroller);
  tryAgain:
  delay(3000);
  if (client.connect(serverName, 80)) {
    if (whereAmI == 0)
      { client.println("GET /finance/info?infotype=infoquoteall&q=UKX HTTP/1.0"); }
     else
      { client.println("GET /finance/info?infotype=infoquoteall&q=HSI HTTP/1.0"); }
    client.println();
  } 
  else {
    nextText = "Internet Failed";
    goto tryAgain;
  }

  delay(500);
}

int charsReceived;
char c;

void loop()
{
  //delay(2000); //WAS 2000
  returnedHTML = "";
  if (client.connected()) { //ONLY DO IF CONNECTED - UNTESTED
  charsReceived = 0;
  while (client.available()) {
    c = client.read();
    charsReceived++;
    Serial.print(c);
    returnedHTML += c;
    if (charsReceived > 860) break;
  }
  } //END ONLY DO IF CONNECTED
  if (!returnedHTML.equals("")) { //ONLY IF WE GOT DATA
  if (!client.connected()) { client.stop(); }
  x = returnedHTML.indexOf("l\" : \"");
  y = returnedHTML.indexOf("c\" : \"");
if ((x!=0)&&(x!=NULL)) { 
  if (whereAmI == 0) 
    {  tempString = "^FTSE:" + returnedHTML.substring(x+6,x+14);  } 
  else
    { tempString = "^HSI:" + returnedHTML.substring(x+6,x+15); } 
  tempString += " ";
  if ((y!=0)&&(y!=NULL)) {
  tempString2 = returnedHTML.substring(y+6,returnedHTML.length());
  if (tempString2.startsWith("-"))
  {
  tempString += downarrow;
  tempString += tempString2.substring(1,tempString2.indexOf("\"")); //0 is first char? or 1.
  } //stock gone down
  else
  {
    tempString += uparrow;
    tempString += tempString2.substring(0,tempString2.indexOf("\""));
  }
  nextText = tempString;
  } } //end x and y are not 0 or null.
  
  } //END ONLY IF WE GOT DATA
  
  client.stop(); // NEW AND UNTESTED...
 if (!returnedHTML.equals("")) { delay(17000);  if (whereAmI == 0) whereAmI++; else whereAmI--; } else { nextText = "Problem."; delay(2000); }
  
  tryAgain2:
  if (client.connect(serverName, 80)) {
    if (whereAmI == 1) { client.println("GET /finance/info?infotype=infoquoteall&q=HSI HTTP/1.0"); }
    else
      { client.println("GET /finance/info?infotype=infoquoteall&q=HSI HTTP/1.0"); }
    client.println();
    delay(500);
  } 
  else {
    nextText = "Internet Failed";
    delay(2000);
    goto tryAgain2;
  }
}

void scroller()
{
  matrix.clearScreen();
  matrix.setCursor(currentX,0);
  matrix.print(textToScroll);
  matrix.writeScreen();
  currentX--;
  if ((currentX) < lengthTextToScroll)
    {
      currentX = 32;
      textToScroll = nextText;
      lengthTextToScroll = (-6*textToScroll.length())+2;
    }
}


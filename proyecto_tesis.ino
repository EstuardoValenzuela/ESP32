
#include "SPI.h"
#include "TFT_eSPI.h"
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "icons.h"
#include <WiFi.h>
#include <Preferences.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define BUZZER_PIN 32 // ESP32 GIOP21 pin connected to Buzzer's pin

#define KEYBOARD_BG         tft.color565(153, 217, 234)
#define ACTIONB_BG            tft.color565(195, 195, 195)
#define SPACE_BG            tft.color565(143, 167, 248)
#define DEL_BG              tft.color565(248, 7, 7)
#define  TEXT_COLORM        tft.color565(36, 150, 149)
#define  BTN_COLORM         tft.color565(141, 217, 143)

#define TFT_GREY 0x5AEB

//========================== USER INTERFACE VARIABLES =====================
String text;
uint8_t optionToSave = 0;
bool typeKeyboard = true;
bool transKeyboard = false;
int start_ypoint  = 120;
String mainWifiSSID, mainWifiPSW, mainNameDevice, mainFirebaseKey, mainFirebaseURL;
bool changeSSID, changeSSIDPSW;
bool keyboard_stat = false; //Triger to print the keyboard

int countPRESSED = 0;

TFT_eSPI tft = TFT_eSPI();
Preferences preferences;

//Firebase configuration
    //Define Firebase Data object
    FirebaseData fbdo;

    FirebaseAuth auth;
    FirebaseConfig config;

    unsigned long sendDataPrevMillis = 0;
    int count = 0;
    bool signupOK = false;

void printwrap(String msg, int x, int y)
{

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setFreeFont(FF1);
   uint16_t auxCount = 0;
   Serial.print("Length = "+msg.length());
    int maxchars = 20;
    if (msg.length() > maxchars or msg.length()-1 < maxchars or msg.length() < maxchars)
    {
      tft.fillRect(5, 10, 230, 15, TFT_BLACK);
      String line1 = msg;
      auxCount = line1.length()-maxchars;
      line1.remove(maxchars, auxCount);
      tft.drawString(line1, x, y);
    }
    if (msg.length() > maxchars*2 or msg.length()-1 < maxchars*2)
    {
      tft.fillRect(5, 25, 230, 15, TFT_BLACK);
      String line2 = msg;
      line2.remove(0, maxchars);
      line2.remove(maxchars, line2.length());
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawString(line2, x, y+15);
    }
   if (msg.length() > maxchars*3 or msg.length()-1 < maxchars*3)
    {
      tft.fillRect(5, 40, 230, 15, TFT_BLACK);
      String line3 = msg;
      line3.remove(0, maxchars*2);
      line3.remove(maxchars, line3.length());
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawString(line3, x, y+30);
    }
   if (msg.length() < maxchars*4 or msg.length()-1 < maxchars*4)
    {
      tft.fillRect(5, 55, 230, 15, TFT_BLACK);
      String line3 = msg;
      line3.remove(0, maxchars*3);
       line3.remove(maxchars, line3.length());
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawString(line3, x, y+45);
    }
    
}

void printKeys(const char *msg, int x, int y){ //Problem detected!!!!!!
    tft.fillRect(0, 0+start_ypoint, 240, 96, KEYBOARD_BG);
    tft.setTextColor(TFT_BLACK, KEYBOARD_BG);
        //Horizontal Lines
    tft.drawLine(0,  0+y, 240,  0+y, TFT_BLACK);
    tft.drawLine(0, 96+y, 240, 96+y, TFT_BLACK);
    tft.drawLine(0, 24+y, 240, 24+y, TFT_BLACK);
    tft.drawLine(0, 47+y, 240, 47+y, TFT_BLACK);
    tft.drawLine(0, 72+y, 240, 72+y, TFT_BLACK);

    //Vertical Lines
    tft.drawLine(0,   0+y, 0,    96+y, TFT_BLACK);    
    tft.drawLine(23,  0+y, 23,   96+y, TFT_BLACK);    
    tft.drawLine(47,  0+y, 47,   96+y, TFT_BLACK);    
    tft.drawLine(71,  0+y, 71,   96+y, TFT_BLACK);    
    tft.drawLine(95,  0+y, 95,   96+y, TFT_BLACK);    
    tft.drawLine(119, 0+y, 119,  96+y, TFT_BLACK);    
    tft.drawLine(143, 0+y, 143,  96+y, TFT_BLACK);    
    tft.drawLine(167, 0+y, 167,  96+y, TFT_BLACK);    
    tft.drawLine(191, 0+y, 191,  96+y, TFT_BLACK);    
    tft.drawLine(215, 0+y, 215,  96+y, TFT_BLACK);    
    tft.drawLine(240, 0+y, 240,  96+y, TFT_BLACK);   
    for ( int i = 0; i < 11; i++)
    {
      String keyPrint = String(msg[i]);
       tft.drawString(keyPrint, (i*x)-17,  6+y);
    }
    for (int i = 10; i < 21; i++)
    {
       String keyPrint = String(msg[i]);
       tft.drawString(keyPrint, ((i-10)*x)-17,  28+y);
    }
    for (int i = 20; i < 31; i++)
    {
       String keyPrint = String(msg[i]);
       tft.drawString(keyPrint, ((i-20)*x)-17,  53+y);
    }
    
    for (int i = 31; i < 41; i++)
    {
       String keyPrint = String(msg[i]);
       tft.drawString(keyPrint, ((i-30)*x)-17,  77+y);
    }

    detectPress(msg);
  }

void draw_keyboard(uint16_t y, bool flag){
    if (flag == true){
            tft.fillScreen(TFT_BLACK);
            //Text Area
            tft.fillRect(0, 0, 240, 80, TFT_GREY);
            tft.fillRect(5, 5, 230, 70, TFT_BLACK);



         //   tft.setTextColor(TFT_BLACK, KEYBOARD_BG);
            tft.fillRoundRect(190, y-35,  50, 30, 4, TFT_RED); //DEL
            tft.fillRoundRect(0,   100+y,  65, 30, 4, ACTIONB_BG); //SHIFT
            tft.fillRoundRect(180,  100+y,  60, 30, 4, ACTIONB_BG); //Special characters
            tft.fillRoundRect(73, 100+y,  100, 30, 4, SPACE_BG); //Space
            tft.fillRoundRect(50,  140+y,  70, 30, 4, TFT_GREEN); //OK
            tft.fillRoundRect(140, 140+y,  70, 30, 4, TFT_BLUE); //Back
            
            tft.setFreeFont(FF1);
            tft.setTextColor(TFT_BLACK, ACTIONB_BG);
            tft.drawString("abc", 15, 108+start_ypoint);
            tft.drawString("?#!", 195, 110+y);
            tft.setTextColor(TFT_BLACK, SPACE_BG);
            tft.drawString("_____", 95, 110+y);
            tft.setTextColor(TFT_BLACK, DEL_BG);
            tft.drawString("DEL",   200, y-28);
            tft.setTextColor(TFT_WHITE, TFT_GREEN);
            tft.drawString("OK",   80-4, 147+y);
            tft.setTextColor(TFT_WHITE, TFT_BLUE);
            tft.drawString("Back",   155, 147+y);
            dataSavePrint();
    }
}
void setup(){
   delay(500);

    Serial.begin(115200);

    tft.begin();
    tft.setRotation(0);
    uint16_t calData[5] = { 297, 3583, 426, 3470, 6 };
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    tft.setTouch(calData);
    tft.fillScreen(TFT_BLACK);

    getData();
    draw_menu();
  }

void loop(){}

void draw_menu(){
    if (keyboard_stat == true)
    {
        
    }
    else{

    tft.fillScreen(TFT_BLACK);

    draw_icon(icon_BWifi, 65, 70, 2, 35);
    draw_icon(icon_BFirebase, 65, 49, 14, 160);
    //=================== BUTTONS ======================================
     tft.fillRoundRect(85, 5, 140, 40, 4, BTN_COLORM); //SSID
     tft.fillRoundRect(85, 50, 140, 40, 4, BTN_COLORM); //PASSWORD
     tft.fillRoundRect(85, 95, 140, 40, 4, BTN_COLORM); //NAME DEVICE
     tft.fillRoundRect(85, 150, 140, 40, 4, BTN_COLORM);  //URL FB
     tft.fillRoundRect(85, 195, 140, 40, 4, BTN_COLORM);  //KEY KB

     
     tft.fillRoundRect(20, 240, 90, 30, 4, TFT_GREEN);
     tft.fillRoundRect(130, 240, 90, 30, 4, TFT_BLUE);
     
      tft.setTextColor(TEXT_COLORM, BTN_COLORM);
     tft.setFreeFont(FF1);
     tft.drawString("Set SSID", 105, 18);
     tft.drawString("Set", 139, 55);
     tft.drawString("Password", 114, 68);
     tft.drawString("Set Name", 108, 107 );

     tft.drawString("Set URL", 110, 163);
     tft.drawString("Set Key", 110, 206);
     
     tft.setTextColor(TFT_BLACK, TFT_GREEN);
     tft.drawString("Save", 40, 247);
     tft.setTextColor(TFT_BLACK, TFT_BLUE);
     tft.drawString("Back", 150, 247);

     tft.setTextColor(TFT_GREEN, TFT_BLACK);
     tft.setFreeFont(FF1);
     tft.drawString("WiFi:", 3, 285);
     tft.drawString("Firebase:", 3, 300);


      system_diagnostic();

      while (true)
      {
        //  Serial.println("Waiting to press...");
          uint16_t x = 0, y = 0; // To store the touch coordinates

          // Pressed will be set true is there is a valid touch on the screen
          bool pressed = tft.getTouch(&x, &y);
          if (pressed) { 
              onBuzzer();
              Serial.println("User press the screen");
              tft.fillCircle(x, y, 2, TFT_WHITE);
              if(85 < x  && x < 225 && 5< y && y < 45 ){
                    effect_press("Set SSID", 105, 18, BTN_COLORM);
                    optionToSave = 1;
                    draw_keyboard(start_ypoint, true);
                    keyboard_stat = true;
                    printKeys(" 1234567890qwertyuiopasdfghjkl:zxcvbnm,.-", 24, start_ypoint);
                    break;
                  }
              if(85 < x  && x < 225 && 50< y && y < 90 ){
                    tft.setTextColor(TFT_RED, BTN_COLORM);
                    tft.drawString("Set", 139, 55);
                    tft.drawString("Password", 114, 68);
                    delay(200);
                    tft.setTextColor(TFT_BLACK, BTN_COLORM);
                    tft.drawString("Set", 139, 55);
                    tft.drawString("Password", 114, 68);
                    optionToSave = 2;
                    draw_keyboard(start_ypoint, true);
                    keyboard_stat = true;
                    printKeys(" 1234567890qwertyuiopasdfghjkl:zxcvbnm,.-", 24, start_ypoint);
                    break;
                  }
              if(85 < x  && x < 225 && 95< y && y < 135 ){
                    optionToSave = 3;
                    effect_press("Set Name", 108, 107, BTN_COLORM);
                    draw_keyboard(start_ypoint, true);
                    keyboard_stat = true;
                    printKeys(" 1234567890qwertyuiopasdfghjkl:zxcvbnm,.-", 24, start_ypoint);
                    break;
                  }
              if(85 < x  && x < 225 && 150< y && y < 190 ){
                    effect_press("Set URL", 110, 163, BTN_COLORM);
                    optionToSave = 4;
                    draw_keyboard(start_ypoint, true);
                    keyboard_stat = true;
                    printKeys(" 1234567890qwertyuiopasdfghjkl:zxcvbnm,.-", 24, start_ypoint);
                    break;
                  }
              if(85 < x  && x < 225 && 195< y && y < 235 ){
                    effect_press("Set Key", 110, 206, BTN_COLORM);
                    optionToSave = 5;
                    draw_keyboard(start_ypoint, true);
                    keyboard_stat = true;
                    printKeys(" 1234567890qwertyuiopasdfghjkl:zxcvbnm,.-", 24, start_ypoint);
                    break;
                  }
              if(20 < x  && x < 110 && 240< y && y < 270 ){
//                      saveData();
                      connectWifi();
                      loginFirebase(mainFirebaseURL, mainFirebaseKey);
                     // loginFirebase("https://small-talk-fmek.firebaseio.com/", "AIzaSyBqCO97KXcNagdqSoKuaEijeXQbCv-kU54");
                  }
              if(130< x  && x < 220 &&  240< y && y < 270 ){
                    // keyboard_stat = false;
                    // print_menu = false;
                    // print_text = false;
                    // template_load();
                    // plot_data_status = true;
                    // come_to_home = true;
                    
                    // break;
                  }
                delay(500);
        } 
      }
    }

}

void draw_icon(const uint16_t* icon, uint16_t w, uint16_t h, uint16_t x, uint16_t y){
  int row, col, buffidx=0;
  for (row=0; row<w; row++) { // For each scanline...
    for (col=0; col<h; col++) { // For each pixel...
      //To read from Flash Memory, pgm_read_XXX is required.
      //Since image is stored as uint16_t, pgm_read_word is used as it uses 16bit address
      tft.drawPixel(col+x, row+y, pgm_read_word(icon + buffidx));
      buffidx++;
    } // end pixel
  
  }
}
void system_diagnostic(){


     tft.fillRect(60, 285, 250, 15, TFT_BLACK);
     tft.fillRect(97, 285, 265, 15, TFT_BLACK);

      if (WiFi.status() == WL_CONNECTED )
      {
          String ip_host = String(WiFi.localIP()[0])+"."+String(WiFi.localIP()[1])+"."+String(WiFi.localIP()[2])+"."+String(WiFi.localIP()[3]);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setFreeFont(FF1);
          tft.drawString(ip_host, 60, 285);
      }
      else{
          tft.setTextColor(TFT_RED, TFT_BLACK);
          tft.setFreeFont(FF1);
          tft.drawString("Not connected", 60, 285);
      }

 //     if(signupOK == true){
  /*    tft.setTextColor(TFT_GREEN, TFT_BLACK); 
        tft.setFreeFont(FF1);
        tft.drawString("Login OK", 100, 300);*/
 //     }
   //   else{
        tft.setTextColor(TFT_RED, TFT_BLACK); 
        tft.setFreeFont(FF1);
        tft.drawString("Login Fail", 100, 300);
   //   }


}

void onBuzzer(){
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);

}

void dataSavePrint(){
    switch (optionToSave)
    {
    case 1:
        printwrap(mainWifiSSID, 8, 10);
        text = mainWifiSSID;
        break;
    case 2:
        printwrap(mainWifiPSW, 8, 10);
        break;
    case 3:
        printwrap(mainNameDevice, 8, 10);    
        text = mainNameDevice;
        break;
    case 4:
        printwrap(mainFirebaseURL, 8, 10);
        text = mainFirebaseURL;
        break;
    case 5:
        printwrap(mainFirebaseKey, 8, 10);
        text = mainFirebaseKey; //DEVELOPMENT ONLY!
        break;

    default:
        break;
    }
        
}

void prepareDataToSave(String textToSave){
    switch (optionToSave)
    {
    case 1:
        if (mainWifiSSID != textToSave)
        {
            mainWifiSSID = textToSave;
            changeSSID = true;
            saveData("ssid", mainWifiSSID);
        }
        break;
    case 2:
        if (mainWifiPSW != textToSave)
        {
            changeSSIDPSW = true;
            mainWifiPSW = textToSave;
            saveData("password", mainWifiPSW);
        }
        break;
    case 3:
        if (mainNameDevice != textToSave)
        {
            mainNameDevice = textToSave;   
            saveData("device", mainNameDevice);
        }
        break;
    case 4:
        if (mainFirebaseURL != textToSave)
        {
            mainFirebaseURL = textToSave;   
            saveData("api_url", mainFirebaseURL);
        }
        break;
    case 5:
        if (mainFirebaseKey != textToSave)
        {
           mainFirebaseKey = textToSave;
           saveData("api_key", mainFirebaseKey);
        }
        break;

    default:
        break;
    }
        
}

void saveData(String param, String data){
    const char* dataSave = data.c_str();
    const char* paramSave = param.c_str();
    preferences.begin("settings", false);
    preferences.putString(paramSave, data); 
    Serial.println("Settings Saved!");
    preferences.end();
}
void getData(){
  preferences.begin("settings", false);
  mainWifiSSID = preferences.getString("ssid", ""); 
    delay(400);
  mainWifiPSW = preferences.getString("password", "");
    delay(400);
  mainNameDevice = preferences.getString("device", "");
    delay(400);
  mainFirebaseURL = preferences.getString("api_url", "");
    delay(400);
  mainFirebaseKey = preferences.getString("api_key", "");
    delay(400);

  Serial.println("SSID: "+mainWifiSSID+" Password:"+mainWifiPSW+" Device name:"+mainNameDevice+" Firebase URL:"+mainFirebaseURL+ " FirebaseKey:"+mainFirebaseKey);
  Serial.println("SSID: "+String(mainWifiSSID.length())+" Password:"+String(mainWifiPSW.length())+" Device name:"+String(mainNameDevice.length())+" Firebase URL:"+String(mainFirebaseURL.length())+ " FirebaseKey:"+String(mainFirebaseKey.length()));
  preferences.end();
}

void connectWifi(){
    if (changeSSID == true or changeSSIDPSW == true)
    {
        changeSSID = false;
        changeSSIDPSW = false;
        WiFi.disconnect();
    }
    
    tft.fillRect(60, 285, 250, 15, TFT_BLACK);
    
    if (WiFi.status() == WL_CONNECTED){
    //    tft.fillRect(60, 285, 250, 15, TFT_BLACK); 
        tft.setTextColor(TFT_GREEN, TFT_BLACK); 
        tft.drawString(String(WiFi.localIP()[0])+"."+String(WiFi.localIP()[1])+"."+String(WiFi.localIP()[2])+"."+String(WiFi.localIP()[3]), 60, 285);
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        if (mainWifiSSID.length() > 0)
         {
            WiFi.begin(mainWifiSSID.c_str(), mainWifiPSW.c_str());
            int get_timeout = 0;
                //check wi-fi is connected to wi-fi network
                while (true) {
                        if (WiFi.status() == WL_CONNECTED)
                        {
                        tft.fillRect(60, 285, 250, 15, TFT_BLACK); 
                        tft.setTextColor(TFT_GREEN, TFT_BLACK); 
                        tft.drawString(String(WiFi.localIP()[0])+"."+String(WiFi.localIP()[1])+"."+String(WiFi.localIP()[2])+"."+String(WiFi.localIP()[3]), 60, 285);
                        break;
                        }
                        tft.setTextColor(TFT_GREEN, TFT_BLACK); 
                        tft.drawString(".", 60+(get_timeout*10), 285);
                        delay(1000);
                        if (get_timeout == 15){
                            keyboard_stat = false;
                            tft.fillRect(60, 285, 250, 15, TFT_BLACK); 
                            tft.setTextColor(TFT_RED, TFT_BLACK);
                            tft.setFreeFont(FF1);
                            tft.drawString("Not connected", 60, 285);
                        break;
                        }
                        get_timeout++;
                }
         }
         if (mainWifiSSID.length() == 0)
         {
            tft.fillRect(60, 285, 250, 15, TFT_BLACK); 
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.setFreeFont(FF1);
            tft.drawString("Not configured", 60, 285);
         }
         
    }
    

    
}
void loginFirebase(String apiURL, String apiKey){
    //Authenticate to Firebase
    /* Assign the api key (required) */
      config.api_key = apiKey.c_str();
      /* Assign the RTDB URL (required) */
      config.database_url = apiURL.c_str();
      Firebase.reconnectWiFi(true);
      /* Sign up */
      if (Firebase.signUp(&config, &auth, "", "")){
    //  if (Firebase.ready()){
        Serial.println("ok");
        /* Assign the callback function for the long running token generation task */
        // config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
        
        // Firebase.begin(&config, &auth);
        // Firebase.reconnectWiFi(true);
        signupOK = true;
        tft.fillRect(97, 300, 265, 15, TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK); 
        tft.setFreeFont(FF1);
        tft.drawString("Login Ok", 100, 300);
      }
      else{
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
        signupOK = false;
        tft.fillRect(97, 300, 265, 15, TFT_WHITE);
        tft.setTextColor(TFT_RED, TFT_BLACK); 
        tft.setFreeFont(FF1);
        tft.drawString("Login Fail", 100, 300);
      }
        /* Assign the callback function for the long running token generation task */
        config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
        Firebase.begin(&config, &auth);
}
void detectPress(const char *abc){
    uint8_t showKeyboard = 0;
    while (true)
    {
      if (keyboard_stat == false){ break;}  
       else{
            uint16_t x = 0, y = 0; // To store the touch coordinates

            // Pressed will be set true is there is a valid touch on the screen
            bool pressed = tft.getTouch(&x, &y);
            if (pressed && 0+start_ypoint<y && y<96+start_ypoint) { 
                uint16_t char_pass = 0;
                for (uint8_t iy = 0; iy < 4; iy++)
                {
                    for (uint8_t ix = 0; ix <10; ix++)
                    {
                        
                        tft.fillCircle(x, y, 2, TFT_WHITE);
                    //     Serial.println("Range x1: "+String(23*ix)+" x2:"+String(23 +(23*ix))+" y1:" + String(iy*24)+" y2:"+String(24+(24*iy))); 
                        if(24*ix< x && x < 24 +(24*ix)  &&  (iy*24)+start_ypoint  < y && y < 24+(24*iy)+start_ypoint){
                            Serial.println("PRESSED: "+String(abc[char_pass+1]));
                            effect_press(String(abc[char_pass+1]), (24*ix)+7,  (24*iy)+5+start_ypoint, KEYBOARD_BG);
                             text += String(abc[char_pass+1]);
                            printwrap(text,8, 10);
                            onBuzzer();
                        }
                    char_pass++;
              
                    }
        
                 }
            }
            if (pressed)
            {
                                 //=============================Function Buttons
            if(100 < x && x < 190 && 100+start_ypoint < y && y < start_ypoint+130){ //Space key
                    text += " ";
                    effect_press("_____", 95, 110+start_ypoint, SPACE_BG);
                    printwrap(text,8, 10);
                }

            if(0 < x && x < 65 && 100+start_ypoint < y && y < start_ypoint+130){ //ABC key
                    if (typeKeyboard == true)
                    {
                       showKeyboard = 1;
                       typeKeyboard = false;
                       break;
                    }
                    if (typeKeyboard == false)
                    {
                       typeKeyboard = true;
                       showKeyboard = 0;
                       break;
                    }   
                    
                }

            if(180 < x && x < 240 && 100+start_ypoint < y && y < start_ypoint+130){ //Special Char key
                    if (transKeyboard == true)
                    {
                       transKeyboard = false;
                       effect_press("?#!", 195, 110+start_ypoint, ACTIONB_BG);
                       delay(400);
                       tft.setTextColor(TFT_BLACK, ACTIONB_BG);
                       tft.drawString("abc", 15, 108+start_ypoint);
                       printKeys(" 1234567890qwertyuiopasdfghjkl:zxcvbnm,.-", 24, start_ypoint);
                       break;
                    }
                    if (transKeyboard == false)
                    {
                       transKeyboard = true;
                       tft.setTextColor(TFT_BLACK, ACTIONB_BG);
                       effect_press("?#!", 195, 110+start_ypoint, ACTIONB_BG);
                       delay(400);
                       printKeys(" 1234567890|!#$%&/()=?+{}:_[]<>;:,.-", 24, start_ypoint);
                       break;
                    }   
                    
                }
            
            if(190 < x && x < 240 && start_ypoint-35 < y && y < start_ypoint-5){ //Del key
                    int len;
                    len = text.length();
                    text.remove(len-1,1);
                    effect_press("DEL",   200, start_ypoint-28, DEL_BG);
                    printwrap(text,8, 10);
                    }


            if(50 < x && x < 120 && 140+start_ypoint < y && y < start_ypoint+170){ //Ok key
                prepareDataToSave(text);
                keyboard_stat = false;
                text = "";
                draw_menu();
                break;
                }

            if(140 < x && x < 210 && 140+start_ypoint < y && y < start_ypoint+170){ //Back key
                text = ""; //Clear global variable
                keyboard_stat = false;
                draw_menu();
                break;
                }
            }
            

        }
    }
    
    if (showKeyboard == 0)
    {
        tft.setTextColor(TFT_BLACK, ACTIONB_BG);
        tft.fillRoundRect(0,   100+start_ypoint,  65, 30, 4, ACTIONB_BG); 
        tft.drawString("abc", 15, 108+start_ypoint);
        countPRESSED++;
        Serial.println("Count "+String(countPRESSED));
        printKeys(" 1234567890qwertyuiopasdfghjkl:zxcvbnm,.-", 24, start_ypoint);
                    
    }
    if (showKeyboard == 1)
    {
        countPRESSED++;
        tft.setTextColor(TFT_BLACK, ACTIONB_BG);
        tft.fillRoundRect(0,   100+start_ypoint,  65, 30, 4, ACTIONB_BG); //SHIFT
        tft.drawString("ABC", 15, 108+start_ypoint);
        Serial.println("Count "+String(countPRESSED));
        printKeys(" 1234567890QWERTYUIOPASDFGHJKL@ZXCVBNM,.-", 24, start_ypoint);

    }


}
void effect_press(String key_press, int x, int y, int color){
            tft.setTextColor(TFT_RED, color);
            tft.drawString(key_press, x, y);
            delay(200);
            tft.setTextColor(TFT_BLACK, color);
            tft.drawString(key_press, x, y);
}

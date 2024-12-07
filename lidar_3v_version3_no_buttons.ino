// guy eakin 2024-Dec-7
// adapted from https://www.diyengineers.com/2022/06/02/lidar-how-to-use-with-arduino/
// display adapted from https://github.com/adafruit/Adafruit_SSD1306/blob/master/examples/ssd1306_128x32_i2c/ssd1306_128x32_i2c.ino
//
// Lidar device that allows user to define a range between two points.  If the LIDAR detects
// an object between these two points, a 3.3v pulse is delivered as output through a 2.5 mm stereo jack
//  currently set up to run on an Arduino micro


// libraries that need to be downloaded to the arduino IDE
    #include <Wire.h>        // Instantiate the Wire library
    #include <TFLI2C.h>      // TFLuna-I2C Library v.0.1.1
    #include <Adafruit_GFX.h>  // for display
    #include <Adafruit_SSD1306.h> // for display

//display
    #define SCREEN_WIDTH 128 // OLED display width, in pixels
    #define SCREEN_HEIGHT 32 // OLED display height, in pixels
    // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
    #define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
    #define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//lidar functions
    TFLI2C tflI2C;
    int16_t  tfDist;    // distance in centimeters
    int16_t  tfAddr = TFL_DEF_ADR;  // Use this default I2C address
    int16_t  tfFlux;  //flux or strength of lidar signal
    int16_t  tfTemp;    // temperature in Celsius

//button functions  --- these are unused currently, but allow for manual buttons on the device to set the trigger distances and activate a laser
    const int buttonPin_Near = 5;   // the number of the pushbutton pin
    const int buttonPin_Far = 7;   // the number of the pushbutton pin
    const int ledPin = 13;    // the number of the LED pin
    int buttonState_Near = 0;
    int buttonState_Far = 0;

// default settings for set points
    int triggerDistNear = 50;  //set a threshold distance (cm) defining the near point of the triggering range 
    int triggerDistFar = 100;  //set a threshold distance (cm) defining the far point 
    char message = "";  //general alert message

//output function - delivers 5v to outputpin
    int armingPin = A0; // if pin is low, then this will prevent triggering function used for testing
    int isArmed = 0;     // status variable
    int outputPin = 6;   

// radio functions  D0-D4 corresponds to pin ID on the RF receiver 
  int signalPinD0 = 8;       
  int signalPinD1 = 9;
  int signalPinD2 = 10;
  int signalPinD3 = 11;
  int isRadio = 0; // variable used to display if input signal comes from the radio rather than a button on the circuit
  
// targeting laser output pin - used to visably mark where the LIDAR infrared beam is being projected
  int laserPin = 12;


// Notes
// I2C Pins are SCL/SDA on Arduino Uno, but SCL=3 and SDA=2 on Micro
// Pinout conversion for different Arduinos
//              UNO   Micro     Nano    NanoEvery
// SignalPinD0    8       8             8
//          D1    9       9             9
//          D2    10      10            10
//          D3    11      11            11
//      Output    3       6             6
// ArminingPin    A0      A0            A0
//         SCL    SCL     3       A5    D19
//         SDA    SDA     2       A4    D18
//        Near    2       5             5
//         Far    4       7             7
//    LaserPin            12            12
//         LED    13      13            13 
//               
//               
//                
void setup(){
    Serial.begin(115200);  // Initalize serial port
    Serial.println("booting");
    Wire.begin();           // Initalize Wire library
    // initialize the LED pin as an output:
    pinMode(ledPin, OUTPUT);

    // initialize the pushbutton pin as an input:
    pinMode(buttonPin_Near, INPUT);
    pinMode(buttonPin_Far, INPUT);

    // set armed Pin as input
      pinMode(armingPin, INPUT_PULLUP);
    // initialize output pin to deliver 5v
      pinMode(outputPin, OUTPUT);

    //display
      // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
      if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
      }
      // Show initial display buffer contents on the screen --
      // the library initializes this with an Adafruit splash screen.
      display.display();
      delay(250); // Pause 

      // Clear the buffer
      display.clearDisplay();

      // Draw a single pixel in white
      display.drawPixel(10, 10, SSD1306_WHITE);

      // Show the display buffer on the screen. You MUST call display() after
      // drawing commands to make them visible on screen!
      display.display();
      delay(250);

      // radio receiver setup
      pinMode(signalPinD0, INPUT);   // this button will be the A button on the transmitter - used to manually trigger shutter
      pinMode(signalPinD1, INPUT);   //   B button - set near 
      pinMode(signalPinD2, INPUT);   //   C button - set far
      pinMode(signalPinD3, INPUT);   //   D button - turn on targetting laser
      //pinMode(outputPinD3_A, OUTPUT);      

      //targetting laser output
      pinMode(laserPin, OUTPUT);
}

 
void loop(){
    char *message = "";
    // get physical button state  (Unused)
    //buttonState_Near = digitalRead(buttonPin_Near);
    //buttonState_Far = digitalRead(buttonPin_Far);

    // this is counterintuitive but input_pullup is declared on armingPin to reduce noise
    // so the low state is actually the armed state.
    if (digitalRead(armingPin) == 1){
      isArmed = 0;
    }
    else {isArmed = 1;}

    // get radio button state
    int PinD0State_D = digitalRead(signalPinD0); // laser function
    int PinD1State_C = digitalRead(signalPinD1);  // sets far distance for lidar
    int PinD2State_B = digitalRead(signalPinD2);  // sets near distance for lidar
    int PinD3State_A = digitalRead(signalPinD3);  // triggers camera shutter
    isRadio = 0; // variable to record if an input signal originates from the radio receiver

    //if(tflI2C.getData(tfDist, tfAddr)){   
    if(tflI2C.getData(tfDist, tfFlux, tfTemp, tfAddr)){ 
        // trigger output (ie, shutter release) in the case of radio signal "A" (PinD3State_A = High)
        if (PinD3State_A == 1){
            char *message = "Radio trigger";
            isRadio = 1;
            displayConstructor(tfDist, triggerDistNear, triggerDistFar, true, message, isArmed, isRadio, buttonState_Near, buttonState_Far);
            triggerOutput(200);
        }
        // turn on laser in case of Radio Signal D
        if (PinD0State_D == 1){
            char *message = "Laser on";
            isRadio = 1;
            displayConstructor(tfDist, triggerDistNear, triggerDistFar, true, message, isArmed, isRadio, buttonState_Near, buttonState_Far);
            digitalWrite(laserPin, HIGH);
            delay(50);
            digitalWrite(laserPin, LOW);
        }

        // retrieve LIDAR button data
        // check if the pushbutton is pressed OR if a radio signal is received.
        // If it is, the buttonState_Near is set to HIGH:
        // as long as signal strength is sufficient (ie, tfFlux is >= some number)
        if ((((buttonState_Near == HIGH) && (tfFlux >= 200)) || ((PinD2State_B == HIGH) && (tfFlux >= 200)) )) {
          // turn LED on:
          digitalWrite(ledPin, HIGH);   // turns LED on Arduino on
          triggerDistNear = tfDist;
          if (PinD2State_B == HIGH){
            char *message = "Radio near";
            isRadio = 1; 
            }
          //else {char *message = "set near ";} //needed for physical buttons
          displayConstructor(tfDist, triggerDistNear, triggerDistFar, false, message, isArmed, isRadio, buttonState_Near, buttonState_Far);
          delay(50);
        } 
        else {
          // turn Arduino LED off, and reset the display
          digitalWrite(ledPin, LOW);
          char *message = "";
          displayConstructor(tfDist, triggerDistNear, triggerDistFar, false, message, isArmed, isRadio, buttonState_Near, buttonState_Far);
        }

        // check if the pushbutton is pressed or radio signal received. If it is, the buttonState_Far is HIGH:
        // as long as signal strength is greater than a value 
        if ((( ( (buttonState_Far == HIGH) && (tfFlux >= 200)) || (PinD1State_C == HIGH) && (tfFlux >= 200)) )) {
          // turn LED on:
          digitalWrite(ledPin, HIGH);
          triggerDistFar = tfDist;
          if (PinD1State_C == HIGH){
            char *message = "Radio far";
            isRadio = 1; 
            }
        //  else {char *message = "set far  ";}
          displayConstructor(tfDist, triggerDistNear, triggerDistFar, false, message, isArmed, isRadio, buttonState_Near, buttonState_Far);
        } 
        else {
          // turn LED off:
          digitalWrite(ledPin, LOW);
          char *message = "";
          displayConstructor(tfDist, triggerDistNear, triggerDistFar, false, message, isArmed, isRadio, buttonState_Near, buttonState_Far);
        }

        // if the far distance is closer than near distance that is silly and we need to swap the values
        if (triggerDistFar < triggerDistNear) {
          char *message = "swap near/far";
          displayConstructor(tfDist, triggerDistNear, triggerDistFar, false, message, isArmed, isRadio, buttonState_Near, buttonState_Far);
          int var = triggerDistFar;
          triggerDistFar = triggerDistNear;
          triggerDistNear = var;
        }


        // check if object is between trigger distances
        if (((tfDist >= triggerDistNear)) && (tfDist <= (triggerDistFar))){
          
            // push relevant data to serial monitor/plotter
            char *message = "in range ";
            displayConstructor(tfDist, triggerDistNear, triggerDistFar, true, message, isArmed, isRadio, buttonState_Near, buttonState_Far);

            // take output pin High (trigger output)
            if (isArmed){
              
              char *message = "triggered";
              displayConstructor(tfDist, triggerDistNear, triggerDistFar, true, message, isArmed, isRadio, buttonState_Near, buttonState_Far);
              triggerOutput(200);

              //flash LED excitedly
              flashExcitedly(20,6);
            }

        }          
    }
    else {    // most likely the lidar threw an error
        tfDist = -1;
        displayConstructor(tfDist, triggerDistNear, triggerDistFar, false, message = "lidar error", isArmed, isRadio, buttonState_Near, buttonState_Far);
    }
    delay(10);


}

// when the system triggers the shutter, this delivers the 5v pulse for a fixed time, then turns off the pulse
void triggerOutput(int outputDuration){
    digitalWrite(outputPin, HIGH);
    delay(outputDuration);
    digitalWrite(outputPin, LOW);
}

void displayConstructor(int tfDist, int triggerDistNear, int triggerDistFar, bool isTriggered, char *message, int isArmed, int isRadio, int buttonState_Near,int buttonState_Far){
  myserialplotter(tfDist, triggerDistNear, triggerDistFar, true, message, buttonState_Near, buttonState_Far);
  int point = vizData(tfDist, triggerDistNear,triggerDistFar);
  Serial.print("Armed: ");Serial.println(isArmed);
  oledDisplay(tfDist,triggerDistNear, triggerDistFar, isTriggered, message, point, isArmed, isRadio);
}

void myserialplotter(int tfDist, int triggerDistNear, int triggerDistFar, bool isTriggered, char *message ,int buttonState_Near,int buttonState_Far){
    
    //when triggered condition is true, provide a indicator in the serial monitor
    //using a variable set to the average of the near and far distances 
    int var = 0;
    if (isTriggered){
      var = 0.5 * (triggerDistNear + triggerDistFar);
    }

    Serial.print("Triggered:");Serial.print(var);
    Serial.print(",");    
    Serial.print("Dist:");Serial.print(tfDist);
    Serial.print(",");
    Serial.print("Near:");Serial.print(triggerDistNear);
    Serial.print(",");
    Serial.print("Far:");Serial.print(triggerDistFar);
    Serial.print(",");
    Serial.print("buttons(nf): ");Serial.print(buttonState_Near);Serial.print(buttonState_Far);
    Serial.print(",");
    Serial.print("Msg:");Serial.print(message);
    Serial.print("\t\t\t,");
}



char vizData(int tfDist, int triggerDistNear, int triggerDistFar){
    // represent lidar distances as ascii image for the display.  
    //  a marker (*) representing an object in the beam path is plotted on the this graph
    //  Near "=-==|==========|==-=" Far
    //            |-- zone --|
    //         |-- zone +/- 50% --|
    //  this function returns the index number of where a marker should be placed in this string
    


    int lengthZone = triggerDistFar - triggerDistNear;
    int far = triggerDistFar + (.5 * lengthZone);
    int near = triggerDistNear - (.5 * lengthZone);
    
    int point = 0;
    if (tfDist > far){point = 19;}
    else if (tfDist < near){point = 0;}
    else if ((tfDist <= far) && (tfDist >= near)){
      float unitlength = (far-near)/16; //calculate length in cm of each of the 16 ascii chars in the "zone + 50%"
      point = float(round( (tfDist-near)/(unitlength)) + 1);  // place marker at representative distance in zone + 50%
    }
    
    //Serial.print("vizA:"); Serial.print(point);Serial.print("");   //this works
    return point;
}  

void oledDisplay(int tfDist, int triggerDistNear, int triggerDistFar, bool isTriggered, char *message, int point, int Isarmed, int isRadio ){
    // build the Oled display data

    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    
    // display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text

    display.clearDisplay();
    display.setCursor(0, 0);     // Start at top-left corner
    display.cp437(true);
    if (isArmed != 0){display.write(0x2A);} // draw an asterix in top left if system is armed 
    else {display.write(0x2D);} // draw an hyphen for unarmed
    display.setCursor(8, 0);     
    display.print(message); // print any system messages
    display.setCursor(90, 0);
    display.print(tfDist); display.print(" cm");  // distance to target as determined by LIDAR

    display.setCursor(0, 11);
    char vizArray[21] = "=-==|==========|==-="; //20 chars + null
    vizArray[point] = byte(42);  // replace a point in the visualization string with an * to show target position
    display.println(vizArray);
    
    display.setCursor(20, 22);  
    display.print(triggerDistNear);
    display.setCursor(80, 22); 
    display.print(triggerDistFar); 

    display.drawRect(25,10,68,9,SSD1306_WHITE );

    if (isRadio == 1){
      // draw a little radio if a radio signal is received
      display.drawRect(107,23,6,8,SSD1306_WHITE );
      display.drawRect(111,18,2,5, SSD1306_WHITE);
      display.drawPixel(114,25, SSD1306_WHITE);
      display.drawLine(108,24,110,24, SSD1306_WHITE);
    }

    if (message == "in range " || message == "triggered" || message =="Radio trigger"){
    // why not draw a camera when the trigger is fired?
    display.drawRect(50,24,18,8,SSD1306_WHITE ); //camera body
    display.drawCircle(58,28,3,SSD1306_WHITE ); // lens
    display.drawRect(58,22,4,2,SSD1306_WHITE ); // flash
    display.drawRect(63,25,4,2,SSD1306_WHITE ); //viewfinder
    }

    if (message == "triggered"){
    //flash 
    display.drawLine(56,22,53,21, SSD1306_WHITE);
    display.drawLine(63,22,66,21, SSD1306_WHITE);
    }
    display.display();   
}   


void flashExcitedly(int delayTime, int flashTimes){
    int var = 0;
    while (var < flashTimes){    
        digitalWrite(ledPin, HIGH);
        delay(delayTime);
        digitalWrite(ledPin, LOW);
        var++;
    }
}

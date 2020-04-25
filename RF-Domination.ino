/*
Wireless Domination by Mark Allen PamojaIT.com - Edit this at your own risk
If you edit this sketch, please acknoweldge me.
Written using the tutorial from ForceTronics - http://forcetronic.blogspot.com/search/label/wireless%20sensor%20network
*/

/**
 * Modifications made by James Delph
 */

#include <SPI.h>              //Call SPI library so you can communicate with the nRF24L01+
#include <nRF24L01.h>         //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <RF24.h>             //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


const uint8_t PIN_CE = 9;      //This pin is used to set the nRF24 to standby (0) or active mode (1)
const uint8_t PIN_CSN = 10;    //This pin is used for SPI comm chip select
RF24 wirelessSPI(PIN_CE, PIN_CSN); // Declare object from nRF24 library (Create your wireless SPI)
const uint64_t READ_ADDRESS = 0xB00B1E50C3LL;  // Create read pipe address for the network
const uint64_t W_ADDRESS = 0xB00B1E50C3LL;  // Set the write pipe to be the same as the read pipe
const uint8_t rFChan = 89;    //Set channel default (chan 84 is 2.484GHz to 2.489GHz)
const uint8_t rDelay = 7;     //this is based on 250us increments, 0 is 250us so 7 is 2 ms
const uint8_t rNum = 5;       //number of retries that will be attempted
LiquidCrystal_I2C lcd(0x3f,20,4);  // set the LCD address to 0x27 or 0x3F
static uint32_t counter;

/* ====================================================
 * Set change the following for the node number (1st time of 5)
  ===================================================
*/
const uint8_t NODE_NUMBER = 1;


//Create a structure to hold button data and channel data
struct PayLoad {
  uint8_t node;
  uint8_t sensor;
};

PayLoad payload;              //create struct object

const uint8_t PIN_RED_SWITCH = 8;               // Connect Red Button to input pin 5
const uint8_t PIN_GREEN_SWITCH = 7;              // Connect Green Button to input pin 6

// Initialise the other nodes
int node1 = 0;
int node2 = 0;
int node3 = 0;
int node4 = 0;
int node5 = 0;


// ===================================================================
// =================          Setup            =======================
// ===================================================================

void setup() {
  payload.node = NODE_NUMBER;
  wirelessSPI.begin();  //Start the nRF24 module
  wirelessSPI.setChannel(rFChan); //set communication frequency channel
  wirelessSPI.openReadingPipe(1, READ_ADDRESS);  //This is receiver or master so we need to be ready to read data from transmitters
  wirelessSPI.startListening();    // Start listening for messages
  Serial.begin(115200);  //serial port to display received data
  Serial.print("Node ");
  Serial.print( payload.node);
  Serial.println(" is online...");
  pinMode(PIN_RED_SWITCH, INPUT);     // declare pushbutton as input
  pinMode(PIN_GREEN_SWITCH, INPUT);     // declare pushbutton as input
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("********************");
  lcd.setCursor(0, 1);
  lcd.print("*  RF Domination   *");
  lcd.setCursor(0, 2);
  lcd.print("*                  *");
  lcd.setCursor(3, 2);
  lcd.print("Node ");
  lcd.print(payload.node);
  lcd.setCursor(0, 3);
  lcd.print("********************");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("*NODE  : 1 2 3 4 5 *");
  lcd.setCursor(6, 1);
  lcd.print(payload.node);
  lcd.setCursor(0, 2);
  lcd.print("*                  *");
  status();
}

// ===================================================================
// =================          Loop Code        =======================
// ===================================================================
void loop()
{
  payload.node = NODE_NUMBER;
  if (counter > 2000)
  {
    counter = 0;
    status();
  }
  counter = counter + 1;
  lcd.setCursor(1,2);
  lcd.print(counter);
  setrx();
  winning();
  updatedisplay();

  if (digitalRead(PIN_RED_SWITCH) == false) // check if the button is pressed
  {
    Serial.println("Red High");
    lcd.setCursor(0,2);
    lcd.print("*                  *");
    lcd.setCursor(3,2);
    lcd.print("Red pressed");
    lcd.setCursor(0,2);
    lcd.print("*                  *");
    payload.sensor = 2; //  Set Sensor Value to 2 (Red)

    // Change node variable to reflect nome number (2nd time of 5)

    node1 = payload.sensor;
    settx();
    // payload.sensor = 0; //  Reset  Sensor Value to 0
  }

  if (digitalRead(PIN_GREEN_SWITCH) == false) // check if the button is pressed
  {
    Serial.println("Green High");
    lcd.setCursor(0,2);
    lcd.print("*                  *");
    lcd.setCursor(3,2);
    lcd.print("Green pressed");
    lcd.setCursor(0,2);
    lcd.print("*                  *");
    payload.sensor = 3;   //Set Sensor Value to 3 (Green)
    // Change node variable to reflect nome number (3rd time of 5)

    node1 = payload.sensor;
    settx();
    // payload.sensor = 0; //  Reset  Sensor Value to 0
  }
}


// ===================================================================
// =================       Recevive Mode       =======================
// ===================================================================
void setrx()
{
  wirelessSPI.openReadingPipe(1, READ_ADDRESS);  // Go into listening mode
  wirelessSPI.startListening();             // Start listening for messages
  wirelessSPI.setDataRate(RF24_250KBPS) ;
  wirelessSPI.setPALevel(RF24_PA_HIGH);
  if (wirelessSPI.available()) //Check if recieved data
  {
    wirelessSPI.read(&payload, sizeof(payload)); //read packet of data and store it in struct object
    Serial.print("Received data packet from node: ");
    Serial.println(payload.node);          //print node number or channel
    Serial.print("Node sensor value is: ");
    Serial.println(payload.sensor);        //print node's sensor value
    Serial.println();

    if (payload.node == 1)
      node1 = payload.sensor;
    else if (payload.node == 2)
      node2 = payload.sensor;
    else if (payload.node == 3)
      node3 = payload.sensor;
    else if (payload.node == 4)
      node4 = payload.sensor;
    else if (payload.node == 5)
      node5 = payload.sensor;
  }

// ============================================================================
// =================           Transmit Mode            =======================
// ================= Only called when button is pressed =======================
// ============================================================================
void settx()
{
  for (int i = 0; i < 4; ++i)
  {
    lcd.setCursor(0, 0);
    lcd.print("****TX**");
    wirelessSPI.setRetries(rDelay, rNum); //if a transmit fails to reach receiver (no ack packet) then this sets retry attempts and delay between retries
    wirelessSPI.openWritingPipe(W_ADDRESS); //open writing or transmit pipe
    wirelessSPI.setPALevel(RF24_PA_HIGH);
    wirelessSPI.setDataRate(RF24_250KBPS) ;
    wirelessSPI.stopListening(); //go into transmit mode

    if (!wirelessSPI.write(&payload, sizeof(payload))) //send data and remember it will retry if it fails
    {
      delay(random(5,20)); //as another back up, delay for a random amount of time and try again
      if (!wirelessSPI.write(&payload, sizeof(payload)))
      {
        //set error flag if it fails again
        lcd.setCursor(0,0);
        lcd.print("*TX fail**");
      }
    }
    lcd.setCursor(0,0);
    lcd.print("********");
  }

  updatedisplay();
}


void status()
{
  lcd.setCursor(1,2);
  lcd.print("Sync");
  setrx();

  // Set change the following for the node number (4th and 5th time of 5)
  payload.node = 1; // maybe bug? was ==
  payload.sensor = node1;
  settx();
  delay(100);
  lcd.setCursor(1,2);
  lcd.print("    ");
}

void updatedisplay()
{
  lcd.setCursor(9, 2);
  if (node1 == 2)
    lcd.print("R");
  if (node1 == 3)
    lcd.print("G");
  if (node1 == 0)
    lcd.print("-");

  lcd.setCursor(11, 2);
  if (node2 == 2)
    lcd.print("R");
  if (node2 == 3)
    lcd.print("G");
  if (node2 == 0)
    lcd.print("-");

  lcd.setCursor(13, 2);
  if (node3 == 2)
    lcd.print("R");
  if (node3 == 3)
    lcd.print("G");
  if (node3 == 0)
    lcd.print("-");

  lcd.setCursor(15, 2);
  if (node4 == 2)
    lcd.print("R");
  if (node4 == 3)
    lcd.print("G");
  if (node4 == 0)
    lcd.print("-");

  lcd.setCursor(17, 2);
  if (node5 == 2)
    lcd.print("R");
  if (node5 == 3)
    lcd.print("G");
  if (node5 == 0)
    lcd.print("-");

  Serial.print("Node1 value is: ");
  Serial.println(node1);        //print node's sensor value
  Serial.print("Node2 value is: ");
  Serial.println(node2);        //print node's sensor value
  Serial.print("Node3 value is: ");
  Serial.println(node3);        //print node's sensor value
  Serial.print("Node4 value is: ");
  Serial.println(node4);        //print node's sensor value
  Serial.print("Node5 value is: ");
  Serial.println(node5);        //print node's sensor value
  Serial.println();
}


void winning()
{
  if (node1 == 2 && node2 == 2 && node3 == 2 && node4 == 2 && node5 == 2)
  {
    lcd.setCursor(0,0);
    lcd.print("********************");
    lcd.setCursor(0,1);
    lcd.print("*  RED TEAM WINS   *");
    lcd.setCursor(0,2);
    lcd.print("*    WELL DONE!    *");
    lcd.setCursor(0,3);
    lcd.print("********************");
    delay(1000);
  }

  if (node1 == 3 && node2 == 3 && node3 == 3 && node4 == 3 && node5 == 3)
  {
    lcd.setCursor(0,0);
    lcd.print("********************");
    lcd.setCursor(0,1);
    lcd.print("* GREEN TEAM WINS   *");
    lcd.setCursor(0,2);
    lcd.print("*    WELL DONE!    *");
    lcd.setCursor(0,3);
    lcd.print("********************");
    delay(1000);
  }
}

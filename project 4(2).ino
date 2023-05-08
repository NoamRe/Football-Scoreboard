#include <EEPROM.h>
#define playerOne 1
#define playerTwo 2

//ARDUINO PINS CONSTANTS
const byte playerOnePin = 12;
const byte playerTwoPin = 2;
const byte DIN = 9;
const byte CS = 8;
const byte CLK = 7;
const int photoResistorPin = A0;
const int temperatureSensorPin = A1;

//EXERCISE PARAMETERS
const byte highTemp = 30;
const int noLightDuration = 5000;
const int messageDisplayTime = 4000;

//** VOLTAGE TO TEMPERATURE CONVERT PARAMETERS **
float R1 = 10000;
float logR2, R2, temperature;
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741;

//SCORE MANAGEMENT PARAMETERS + DEBOUNCE
byte playerOneScore = 0;
byte playerTwoScore = 0;
byte lastPlayerOneSwitchState = HIGH;
byte playerOneSwitchState = HIGH;
byte lastPlayerTwoSwitchState = HIGH;
byte playerTwoSwitchState = HIGH;
byte playerOneScoreInMatrix;
byte playerTwoScoreInMatrix;
const byte maxScore = 9;
const byte minScore = 0;
const byte firstPlayerScoreInSSD = 0;
const byte secondPlayerScoreInSSD = 1;
bool afterSpecialModeScorePrinted = false;
unsigned long lastDebounceTimeSwitchOne = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimeSwitchTwo = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

//LONG PRESS DETECTION PARAMETERS
unsigned long currentTime;
unsigned long playerOneSwitchStartClickTime;
unsigned long switchOneLongPressDetectTime;
unsigned long playerTwoSwitchStartClickTime;
unsigned long switchTwoLongPressDetectTime;
const int longPressInterval = 2000;
const int flashFrequency = 200;
const int clickingDelaySafety = 200;
bool isSwitchOneClicked=false;
bool isSwitchTwoClicked=false;
bool isLongPressSwitchOne=false;
bool isLongPressSwitchTwo=false;
bool doneDealingWithLongPressSwitchOne = true;
bool doneDealingWithLongPressSwitchTwo = true;
bool scoreJustReseted = false;

//HOT WEATHER MODE PARAMETERS
bool isHotWeather = false;
bool isHotWeatherModeActive = false;
bool isCountingHotWeatherMsgTimeGap = false;
bool isPrintingHotWeatherMessage = false;
int hotWeatherMessagePrintsCounter =0;
const float hotWeatherSafety = 0.1;
const byte drinkWaterMessageNumOfPrint = 60;
unsigned long drinkWaterMessagePrintInterval;
unsigned long printingHotWeatherMessageTime;
unsigned long hotWeatherMessagePrintingOverTime;

//DARKNESS MODE PARAMETERS
bool isDarkness = false;
bool darknessDetected = false;
bool isDarknessModeActivated = false;
bool isPrintingDarknessMessage = false;
bool isCountingDarknessMsgTimeGap = false;
int DarknessMessagePrintInterval;
int darknessMessagePrintsCounter =0;
const int lowLightness = 300;
const byte remainSeatedMessageNumberOfPrint = 69;
unsigned long darknessDetectTime;
unsigned long printingDarknessMessageTime;
unsigned long DarknessMessagePrintingOverTime;


const unsigned int MessageDisplayGap = 10000;

//MESSAGES ARRAY
byte drinkWaterMessage[68] = {
    0x3C, 0x46, 0xA9, 0x9D, 0xBB, 0xD1, 0x4A, 0x3C, 0x00,
    0x38, 0x44, 0x44, 0xFF, 0x00,
    0x7C, 0x08, 0x04, 0x04, 0x00,
    0x44, 0x7D, 0x40, 0x00,
    0x7C, 0x04, 0x04, 0x78, 0x00,
    0xFF, 0x10, 0x28, 0x44, 0x00,
    0x00, 0x00,
    0x3C, 0x40, 0x3C, 0x40, 0x3C, 0x00,
    0x20, 0x54, 0x54, 0x78, 0x00,
    0x04, 0x7F, 0x44, 0x00,
    0x38, 0x54, 0x54, 0x18, 0x00,
    0x7C, 0x08, 0x04, 0x04, 0x00,
    0x3C, 0x46, 0xA9, 0x9D, 0xBB, 0xD1, 0x4A, 0x3C
};

byte remainSeatedMessage[77] = {
    0x3C, 0x46, 0xA9, 0x9D, 0xBB, 0xD1, 0x4A, 0x3C, 0x00,
    0x7C, 0x08, 0x04, 0x04, 0x00,
    0x38, 0x54, 0x54, 0x18, 0x00,
    0x7C, 0x04, 0x7C, 0x04, 0x78,
    0x20, 0x54, 0x54, 0x78, 0x00,
    0x44, 0x7D, 0x40, 0x00,
    0x7C, 0x04, 0x04, 0x78, 0x00,
    0x00, 0x00,
    0x28, 0x54, 0x54, 0x24, 0x00,
    0x38, 0x54, 0x54, 0x18, 0x00,
    0x20, 0x54, 0x54, 0x78, 0x00,
    0x04, 0x7F, 0x44, 0x00,
    0x38, 0x54, 0x54, 0x18, 0x00,
    0x38, 0x44, 0x44, 0xFF, 0x00,
    0x3C, 0x46, 0xA9, 0x9D, 0xBB, 0xD1, 0x4A, 0x3C
};


//BYTE ARRAYS
byte scoresRepresentation[10][3]= {
  {0x3C, 0x42, 0x3C},
  {0x02, 0x7E, 0x00},
  {0x72, 0x52, 0x5E},
  {0x52, 0x52, 0x7E},
  {0x0E, 0x08, 0x7E},
  {0x5E, 0x52, 0x72},
  {0x7E, 0x52, 0x72},
  {0x02, 0x12, 0x7E},
  {0x7E,0x4A, 0x7E},
  {0x4E, 0x4A, 0x7E}
};

byte scoreToPrintInMatrix[8];
byte messageToPrintInMatrix[8];

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(playerOnePin,INPUT_PULLUP);
  pinMode(playerTwoPin,INPUT_PULLUP);
  pinMode(DIN,OUTPUT);
  pinMode(CS,OUTPUT);
  pinMode(CLK,OUTPUT);
  initConfiguration();
  initProgramSetting();
  //score seperating line
  scoreToPrintInMatrix[3] = 0x20;
  scoreToPrintInMatrix[4] = 0x20;
  //loading score from SSD and printing initial result
  loadInitialScore();
  turnOffMatrix();
  if(checkScoreReset())
  {
    resetScore();
    scoreJustReseted=true;
    afterSpecialModeScorePrinted = true;
    Serial.println((String)"initial score is "+playerOneScore+"-"+playerTwoScore);
    scoreJustReseted = true;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  checkHighTemperature();
  checkStadiumLightning();
  temperatureHandler();
  LightingHandler();

  if(!isHotWeather && !isDarkness)
  {
    printScoreInMatrix();
  }

  managePlayerOneScore();
  managePlayerTwoScore();
  reduceScoreManagement();
}

//a function that read the value from photoResistorPin and update relevant flags
void checkStadiumLightning()
{
  int lightness = analogRead(photoResistorPin);
  if(lightness<=lowLightness)
  {
    //darkness was just detencted
    if(!darknessDetected)
    {
      Serial.println("dark was detected!");
      darknessDetected = true;
      darknessDetectTime = millis();
    }

    if((millis() - darknessDetectTime >= noLightDuration) && !isDarknessModeActivated)
    {
      //stadium is dark for long time
      Serial.println((String)"stadium has been dark for more than "+(noLightDuration/1000)+" seconds ,turning darkness mode on!");
      isDarkness=true;
    }
  }
  else
  {
    isDarkness=false;
    darknessDetected=false;
  }
}

//a function that responds to stadium lightning changes
void LightingHandler()
{
  if(isDarkness)
  {
    if(!isDarknessModeActivated)
    {
      activateDarknessMode();
    }
    else
    {
      managerDarknessMode();
    } 
  }
  else
  {
    if(isDarknessModeActivated)
      deActivateDarknessMode();
  }
}

//a function that activate darkness mode
//by managing relevant flags
void activateDarknessMode()
{
  Serial.println("Darkness mode activated");
  isDarknessModeActivated=true;
  isPrintingDarknessMessage = true;
  isCountingDarknessMsgTimeGap = false;
  printingDarknessMessageTime=millis();
  printMatrixFromRecievedIndex(remainSeatedMessage,darknessMessagePrintsCounter);
  darknessMessagePrintsCounter++;
}

//the function deActivate darkness mode
//by managing relevant flags
void deActivateDarknessMode()
{
  Serial.println("Darkness mode deactivated");
  isDarknessModeActivated=false;
  isPrintingDarknessMessage = false;
  isCountingDarknessMsgTimeGap = false;
  afterSpecialModeScorePrinted=false;
  darknessMessagePrintsCounter = 0;
  printScoreInMatrix();
}

//a function to manage darkness mode
//the function prints the mode message every 10 secs for 4 seconds
void managerDarknessMode()
{
  if(isPrintingDarknessMessage)
  {
    printDarknessMessage();
  }
  else if(isCountingDarknessMsgTimeGap)
  {
    manageDarknessMsgTimeGap();    
  }
}

void manageDarknessMsgTimeGap()
{
  if(millis()-DarknessMessagePrintingOverTime >= MessageDisplayGap)
  {
    printingDarknessMessageTime=millis();
    printMatrixFromRecievedIndex(remainSeatedMessage,darknessMessagePrintsCounter);
    darknessMessagePrintsCounter++;
    isCountingDarknessMsgTimeGap = false;
    isPrintingDarknessMessage = true;
  }

  if(isCountingDarknessMsgTimeGap)
    printScoreInMatrix();
}

void printDarknessMessage()
{
  if(!isPrintingDarknessMessage)
  {
    isPrintingDarknessMessage = true;
    printingDarknessMessageTime=millis();
    printMatrixFromRecievedIndex(remainSeatedMessage,darknessMessagePrintsCounter);
    darknessMessagePrintsCounter++;
  }

  if(millis()-printingDarknessMessageTime>=DarknessMessagePrintInterval)
  {
    printMatrixFromRecievedIndex(remainSeatedMessage,darknessMessagePrintsCounter);
    darknessMessagePrintsCounter++;
    printingDarknessMessageTime=millis();
    if(darknessMessagePrintsCounter == remainSeatedMessageNumberOfPrint)
    {
      isPrintingDarknessMessage=false;
      darknessMessagePrintsCounter = 0;
      isCountingDarknessMsgTimeGap = true;
      DarknessMessagePrintingOverTime=millis();
    }
  }
}

void checkHighTemperature()
{
  int temperatureVoltage = analogRead(temperatureSensorPin);
  //CODE TO CONVERT KY-013
  R2 = R1 * (1023.0 / (float)temperatureVoltage - 1.0); //calculate resistance on thermistor
  logR2 = log(R2);
  temperature = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // temperature in Kelvin
  temperature = temperature - 273.15;
  //Serial.println((String)"Current temperature is: "+temperature);
  if(temperature>=highTemp*(1+hotWeatherSafety) && !isHotWeatherModeActive)
  {
    isHotWeather=true;
  }
  else if(temperature<highTemp*(1-hotWeatherSafety) && isHotWeatherModeActive)
  {
    isHotWeather=false;
  }
}

void temperatureHandler()
{
  if(isHotWeather)
  {
    if(!isHotWeatherModeActive)
      activateHotWeather();
    else
      managerHotWeather();    
  }
  else
  {
    if(isHotWeatherModeActive)
      deActivateHotWeather();
  }
}

void activateHotWeather()
{
  Serial.println("Hot weather mode activated");
  isHotWeatherModeActive=true;
  isPrintingHotWeatherMessage = true;
  isCountingHotWeatherMsgTimeGap = false;
  printingHotWeatherMessageTime=millis();
  printMatrixFromRecievedIndex(drinkWaterMessage,hotWeatherMessagePrintsCounter);
  hotWeatherMessagePrintsCounter++;
}

void managerHotWeather()
{
  if(isPrintingHotWeatherMessage)
  {
    printHotWeatherMessage();
  }
  else if(isCountingHotWeatherMsgTimeGap)
  {
    manageHotWeatherMsgTimeGap();    
  }
}

void deActivateHotWeather()
{
  Serial.println("Hot weather mode deactivated");
  isHotWeatherModeActive=false;
  isPrintingHotWeatherMessage = false;
  isCountingHotWeatherMsgTimeGap = false;
  afterSpecialModeScorePrinted=false;
  hotWeatherMessagePrintsCounter = 0;
  printScoreInMatrix();
}

void manageHotWeatherMsgTimeGap()
{
  if(millis()-hotWeatherMessagePrintingOverTime >= MessageDisplayGap)
  {
    printingHotWeatherMessageTime=millis();
    printMatrixFromRecievedIndex(drinkWaterMessage,hotWeatherMessagePrintsCounter);
    hotWeatherMessagePrintsCounter++;
    isCountingHotWeatherMsgTimeGap = false;
    isPrintingHotWeatherMessage = true;
  }

  if(isCountingHotWeatherMsgTimeGap)
    printScoreInMatrix();
}

void printHotWeatherMessage()
{
  if(millis()-printingHotWeatherMessageTime>=drinkWaterMessagePrintInterval)
  {
    printMatrixFromRecievedIndex(drinkWaterMessage,hotWeatherMessagePrintsCounter);
    hotWeatherMessagePrintsCounter++;
    printingHotWeatherMessageTime=millis();
    if(hotWeatherMessagePrintsCounter == drinkWaterMessageNumOfPrint)
    {
      isPrintingHotWeatherMessage=false;
      hotWeatherMessagePrintsCounter = 0;
      isCountingHotWeatherMsgTimeGap = true;
      hotWeatherMessagePrintingOverTime=millis();
    }
  }
}

//function to check score reset
bool checkScoreReset()
{
  while(digitalRead(playerTwoPin) == LOW && digitalRead(playerOnePin) == LOW && millis()<=longPressInterval)
  {
    Serial.println((String)"long press duration :"+millis());
  }
  
  return (millis()>longPressInterval);
}
//the function loads score from ssd
//if theres no score saved, then the function starts the score with 0-0
void loadInitialScore()
{
  readScoreFromSSD();
  if(playerOneScore == 255 || playerTwoScore == 255)
  {
    playerOneScore = 0;
    playerTwoScore = 0;
  }
}

//function to manage player two score
void managePlayerOneScore(){
  byte currentPlayerOneSwitchState = digitalRead(playerOnePin);

  //debounce
  if(currentPlayerOneSwitchState != lastPlayerOneSwitchState){
    lastDebounceTimeSwitchOne = millis();
  }

  if((millis()-lastDebounceTimeSwitchOne) >=debounceDelay){
    if(currentPlayerOneSwitchState != playerOneSwitchState){
      playerOneSwitchState = currentPlayerOneSwitchState;
      if(currentPlayerOneSwitchState == HIGH){
        playerOneSwitchHighManagement();
      }
      else // current state is LOW
      {
        playerOneSwitchStartClickTime = millis();
      }
    }
    else
    {
      if(currentPlayerOneSwitchState == LOW)
      {
        playerOneLongSwitchLowManagement();
      }
    }
  }
  lastPlayerOneSwitchState = currentPlayerOneSwitchState;  
}

//the function react to player one switch high state
void playerOneSwitchHighManagement()
{
  if(!isLongPressSwitchOne && !scoreJustReseted)
  {
    if(playerOneScore<maxScore)
    {
      increasePlayerScore(playerOne);
      updateSpecialModeFlags();
    }
  }
  else //we had a long press, no need to increase score
  {
    isLongPressSwitchOne = false;
    scoreJustReseted = false;
  }
}

//the function react to player one switch long low state
void playerOneLongSwitchLowManagement()
{
  if(doneDealingWithLongPressSwitchOne && !isLongPressSwitchOne)
  {
    currentTime = millis();
    if(currentTime - playerOneSwitchStartClickTime >= longPressInterval)
    {
      switchOneLongPressDetectTime = millis();
      playerOneSwitchStartClickTime=millis();
      Serial.println("switch ONE long press detected!");
      doneDealingWithLongPressSwitchOne=false;
      isLongPressSwitchOne = true;      
    }
  }
}

//function to manage player two score
void managePlayerTwoScore(){
  byte currentPlayerTwoSwitchState = digitalRead(playerTwoPin);
  //debounce - each click is counted once only.
  if(currentPlayerTwoSwitchState != lastPlayerTwoSwitchState){
    lastDebounceTimeSwitchTwo = millis();
  }

  if((millis()-lastDebounceTimeSwitchTwo) >=debounceDelay){
    if(currentPlayerTwoSwitchState != playerTwoSwitchState){
      playerTwoSwitchState = currentPlayerTwoSwitchState;
      if(currentPlayerTwoSwitchState == HIGH){
        playerTwoSwitchHighManagement();
      }
      else
      {
        playerTwoSwitchStartClickTime = millis();
      }
    }
    else
    {  
      if(currentPlayerTwoSwitchState == LOW)
      {
        playerTwoLongSwitchLowManagement();
      }
    }
  }
  lastPlayerTwoSwitchState = currentPlayerTwoSwitchState;  
}

//the function react to player two switch high state
void playerTwoSwitchHighManagement()
{
  if(!isLongPressSwitchTwo && !scoreJustReseted)
  {
    if(playerTwoScore < maxScore){
      updateSpecialModeFlags(); 
      increasePlayerScore(playerTwo);
    }
  }
  else
  {
    isLongPressSwitchTwo = false;
    scoreJustReseted = false;
  }
}

//the function react to player one switch long low state
void playerTwoLongSwitchLowManagement()
{
  if(doneDealingWithLongPressSwitchTwo && !isLongPressSwitchTwo)
  {
    currentTime = millis();
    if(currentTime - playerTwoSwitchStartClickTime >= longPressInterval)
    {
      playerTwoSwitchStartClickTime=millis();
      switchTwoLongPressDetectTime = millis();
      Serial.println("switch TWO long press detected!");
      doneDealingWithLongPressSwitchTwo=false;
      isLongPressSwitchTwo = true;  
    }
  }
}

void updateSpecialModeFlags()
{
  if(isHotWeather)
  {
    isPrintingHotWeatherMessage = false;
    isCountingHotWeatherMsgTimeGap = true;
    hotWeatherMessagePrintingOverTime = millis();
    hotWeatherMessagePrintsCounter = 0;
  }

  if(isDarkness)
  {
    isPrintingDarknessMessage = false;
    isCountingDarknessMsgTimeGap = true;
    DarknessMessagePrintingOverTime = millis();
    darknessMessagePrintsCounter = 0;
  }
}

//function that recieves player number and increase his score by one.
void increasePlayerScore(int playerNum)
{
  if(playerNum == playerOne)
    playerOneScore++;
  else
    playerTwoScore++;
  
  saveScoreToSSD();
  Serial.print((String)"Player "+playerNum+" scored a goal!, current score is ");
  Serial.print(playerOneScore);
  Serial.print("-");
  Serial.println(playerTwoScore);
}

//the function manage score reducing or reseting 
//the function work depends on spicific flags
void reduceScoreManagement()
{
  if(isLongPressSwitchOne && !doneDealingWithLongPressSwitchOne ){
    reducePlayerOneScore();
    updateSpecialModeFlags();
    doneDealingWithLongPressSwitchOne = true;
  }
  if(isLongPressSwitchTwo && !doneDealingWithLongPressSwitchTwo){
    reducePlayerTwoScore();
    updateSpecialModeFlags();
    doneDealingWithLongPressSwitchTwo = true;
  }
}

//function to reset score
void resetScore()
{
  playerOneScore=0;
  playerTwoScore=0;
  saveScoreToSSD();
  printScoreInMatrix();
  Serial.println("new game has started, current score is 0-0");
}

//function to reduce player one score by one.
void reducePlayerOneScore()
{
  if(playerOneScore > minScore)
  {
    flashPlayerScore(1);
    playerOneScore--;
    saveScoreToSSD();
    printScoreInMatrix();
    Serial.println((String)"VAR DECISION: NO GOAL!. current score is "+playerOneScore+"-"+playerTwoScore);
  }
}

//function to redcude player two score by 1
void reducePlayerTwoScore()
{
  if(playerTwoScore > minScore)
  {
    flashPlayerScore(2);
    playerTwoScore--;
    saveScoreToSSD();
    printScoreInMatrix();
    Serial.println((String)"VAR DECISION: NO GOAL!. current score is "+playerOneScore+"-"+playerTwoScore);
  }
}

//function that reduce recieved player score 3 times.
void flashPlayerScore(byte playerNumber)
{
  byte turnedOffPlayerScoreMatrix[8] = {0x00,0x00,0x00,0x20,0x20,0x00,0x00,0x00};
  byte startIndex = playerNumber ==1 ? 5 : 0;

  for(int i=0;i<3;i++){
    turnedOffPlayerScoreMatrix[startIndex+i] = scoreToPrintInMatrix[startIndex+i];
  }

  for(int i=0;i<3;i++)
  {
    printMatrix(turnedOffPlayerScoreMatrix);
    delay(flashFrequency);
    printMatrix(scoreToPrintInMatrix);
    delay(flashFrequency);
  }
}

//function that update the score in the matrix
void printScoreInMatrix()
{
  //check if score has changed
  if(!afterSpecialModeScorePrinted || playerOneScoreInMatrix != playerOneScore || playerTwoScoreInMatrix != playerTwoScore)
  {
    updatePlayerScoreInMatrixArray(1,playerOneScore);
    playerOneScoreInMatrix = playerOneScore;
    updatePlayerScoreInMatrixArray(2,playerTwoScore);
    playerTwoScoreInMatrix = playerTwoScore;
    printMatrix(scoreToPrintInMatrix);
  }
}

//function the recieve player number and a score
//the function update the scoreArray to update the matrix
void updatePlayerScoreInMatrixArray(byte playerNumber,byte scoreToUpdate)
{
  byte startIndex;
  startIndex = playerNumber ==1 ? 0 : 5;
  for(int i=0; i<3;i++)
    scoreToPrintInMatrix[startIndex+i] = scoresRepresentation[scoreToUpdate][i];
}

//function that recieve 8 byte array (all matrix columns)
//and print the array context in the matrix
void printMatrix(byte* columns)
{
  for(byte i=1;i<=8;i++)
    writeColumn(i,columns[i-1]);
}

void printMatrixFromRecievedIndex(byte* matrix, int index)
{
  for(byte i=1;i<=8;i++)
    writeColumn(i,matrix[index+i-1]);
}



void writeColumn(byte columnIndex, byte ledsToLight)
{
  if(columnIndex < 9)
  {
    for(int i=7; i >= 0 ; i--)
      writeBit((columnIndex>>i)&1);
    for(int i=7; i >= 0 ; i--)
      writeBit((ledsToLight>>i)&1);
  }
  latchBuf();
}

//function to load 16 bit from MAX7219 into matrix
void latchBuf() // Latch the entire buffer
{
  digitalWrite(CS,LOW);
  digitalWrite(CS,HIGH);
}

//function to push a bit into MAX7219
void writeBit(bool bit) // Write 1 bit to the buffer
{
  digitalWrite(DIN,bit);
  digitalWrite(CLK,LOW);
  digitalWrite(CLK,HIGH);
}

//function to turn off all the leds in the matrix
void turnOffMatrix()
{
  for(byte i=1;i<8;i++)
      writeColumn(i,0);
}

void saveScoreToSSD()
{
  EEPROM.update(firstPlayerScoreInSSD, playerOneScore);
  EEPROM.update(secondPlayerScoreInSSD, playerTwoScore);
}

void readScoreFromSSD()
{
  playerOneScore = EEPROM.read(firstPlayerScoreInSSD);
  playerTwoScore = EEPROM.read(secondPlayerScoreInSSD);
}

//a function that initilize the matrix configuration
void initConfiguration()
{
  for (int i=0; i<4; i++) writeBit(LOW);
  writeBit(HIGH);
  for (int i=0; i<2; i++) writeBit(LOW);
  writeBit(HIGH);
  for (int i=0; i<8; i++) writeBit(LOW);
  latchBuf();  
  for (int i=0; i<4; i++) writeBit(LOW);
  writeBit(HIGH);
  writeBit(LOW);
  writeBit(HIGH);
  writeBit(HIGH);
  for (int i=0; i<5; i++) writeBit(LOW);
  for (int i=0; i<3; i++) writeBit(HIGH);
  latchBuf(); 
  for (int i=0; i<4; i++) writeBit(LOW);
  for (int i=0; i<2; i++) writeBit(HIGH);
  for (int i=0; i<2; i++) writeBit(LOW);
  for (int i=0; i<7; i++) writeBit(LOW);
  writeBit(HIGH);
  latchBuf();

  playerOneSwitchState = digitalRead(playerOnePin);
  playerTwoSwitchState = digitalRead(playerTwoPin);
}

void initProgramSetting()
{
  drinkWaterMessagePrintInterval = messageDisplayTime / drinkWaterMessageNumOfPrint;
  DarknessMessagePrintInterval = messageDisplayTime / remainSeatedMessageNumberOfPrint;
  Serial.println((String)"hotWeather message interval is: "+drinkWaterMessagePrintInterval+",  darkness message interval is "+DarknessMessagePrintInterval);
}
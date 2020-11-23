int hum = A0;
//int temp=A1;

int pin_led=11;

#include <Fuzzy.h>
Fuzzy * fuzzy = new Fuzzy();

// error = Humiditysensor - Humiditysetup 
FuzzySet *Negative = new FuzzySet(-100, -100, -100, 0);
FuzzySet *Z        = new FuzzySet( -10, 0, 0, 10);
FuzzySet *Positive = new FuzzySet( 0, 100, 100, 100);
// temperature 
FuzzySet *cold     = new FuzzySet(-5, -5, 8, 15);
FuzzySet *cool     = new FuzzySet(8, 18, 18, 28);
FuzzySet *normal   = new FuzzySet(15, 25, 25, 35);
FuzzySet *hot      = new FuzzySet(30, 35, 35, 40);
FuzzySet *veryhot  = new FuzzySet(35, 43, 50, 50);
// fuzzy output
FuzzySet *high     = new FuzzySet(0, 0, 0, 0.5);
FuzzySet *slow     = new FuzzySet(0.3, 0.5, 0.5, 0.9);
FuzzySet *off      = new FuzzySet(0.8, 1, 1, 1);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  FuzzyInput *eror = new FuzzyInput(1);

  eror->addFuzzySet(Negative);
  eror->addFuzzySet(Z);
  eror->addFuzzySet(Positive);
  fuzzy->addFuzzyInput(eror);

  //fuzzyInput2
  FuzzyInput *temp =new FuzzyInput(2);

  temp->addFuzzySet(cold);
  temp->addFuzzySet(cool);
  temp->addFuzzySet(normal);
  temp->addFuzzySet(hot);
  temp->addFuzzySet(veryhot);
  fuzzy->addFuzzyInput(temp);

  // fuzzy output
  FuzzyOutput *led =new FuzzyOutput(1);

  led->addFuzzySet(high);
  led->addFuzzySet(slow);
  led->addFuzzySet(off);
  fuzzy->addFuzzyOutput(led);
  //---------------------------

  //Fuzzyrule 1
  FuzzyRuleAntecedent *iferorisN = new FuzzyRuleAntecedent();
  iferorisN->joinSingle(Negative);
  FuzzyRuleConsequent *thenWateringH = new FuzzyRuleConsequent();
  thenWateringH->addOutput(high);
  FuzzyRule * fuzzyRule01 = new FuzzyRule (1, iferorisN, thenWateringH);
  fuzzy->addFuzzyRule(fuzzyRule01);
  //rule 2 
  FuzzyRuleAntecedent *iferorisZ_Cold = new FuzzyRuleAntecedent();
  iferorisZ_Cold->joinWithAND(Z, cold);
  FuzzyRuleConsequent *thenWateringS =new FuzzyRuleConsequent();
  thenWateringS ->addOutput(slow);
  FuzzyRule *fuzzyRule02 = new FuzzyRule(2, iferorisZ_Cold,thenWateringS);
  fuzzy->addFuzzyRule(fuzzyRule02);

  // rule 3
  FuzzyRuleAntecedent *iferorisZ_Cool = new FuzzyRuleAntecedent();
  iferorisZ_Cool->joinWithAND(Z, cool);
  FuzzyRuleConsequent *thenWateringSlow =new FuzzyRuleConsequent();
  thenWateringSlow ->addOutput(slow);
  FuzzyRule *fuzzyRule03 = new FuzzyRule(3, iferorisZ_Cool,thenWateringSlow);
  fuzzy->addFuzzyRule(fuzzyRule03);

  // rule 4
  FuzzyRuleAntecedent *iferorisZ_Normal = new FuzzyRuleAntecedent();
  iferorisZ_Normal->joinWithAND(Z, normal);
  FuzzyRuleConsequent *thenWateringoff =new FuzzyRuleConsequent();
  thenWateringoff ->addOutput(off);
  FuzzyRule *fuzzyRule04 = new FuzzyRule(4, iferorisZ_Normal,thenWateringoff);
  fuzzy->addFuzzyRule(fuzzyRule04);
  //rule 5
  FuzzyRuleAntecedent *iferorisZ_Hot = new FuzzyRuleAntecedent();
  iferorisZ_Hot->joinWithAND(Z, hot);
  FuzzyRuleConsequent *thenWateringoff1 =new FuzzyRuleConsequent();
  thenWateringoff1 ->addOutput(off);
  FuzzyRule *fuzzyRule05 = new FuzzyRule(5, iferorisZ_Hot,thenWateringoff1);
  fuzzy->addFuzzyRule(fuzzyRule05);
  //rule 6
  FuzzyRuleAntecedent *iferorisZ_Veryhot = new FuzzyRuleAntecedent();
  iferorisZ_Veryhot->joinWithAND(Z, veryhot);
  FuzzyRuleConsequent *thenWateringoff2 =new FuzzyRuleConsequent();
  thenWateringS ->addOutput(off);
  FuzzyRule *fuzzyRule06 = new FuzzyRule(6, iferorisZ_Veryhot,thenWateringoff2);
  fuzzy->addFuzzyRule(fuzzyRule06);
  //rule 7
  FuzzyRuleAntecedent *iferorisP = new FuzzyRuleAntecedent();
  iferorisP->joinSingle(Positive);
  FuzzyRuleConsequent *thenWateringoff3 =new FuzzyRuleConsequent();
  thenWateringoff3 ->addOutput(off);
  FuzzyRule *fuzzyRule07 = new FuzzyRule(7, iferorisP,thenWateringoff3);
  fuzzy->addFuzzyRule(fuzzyRule02);
}

void loop() {
  // put your main code here, to run repeatedly:
  int eror =-40;
  int temp =10;

  fuzzy->setInput(1,eror);
  fuzzy->setInput(2,temp);
  fuzzy->fuzzify();

  float output = fuzzy->defuzzify(1);

  Serial.print("output is " );
  Serial.print(output);
  Serial.print("\n");
  delay(1000);
  

}

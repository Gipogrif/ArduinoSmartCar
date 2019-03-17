#include <Arduino_FreeRTOS.h>

void TaskServo(void *pvParameters);
void TaskBluetoothRead(void *pvParameters);

// сервопривод
#include <Servo.h>
Servo myservo;
int pos = 0;

// ультразвуковой сенсор
#include "SR04.h"
#define TRIG_PIN 11
#define ECHO_PIN 10
SR04 sr04 = SR04(ECHO_PIN, TRIG_PIN);
long a;

// ИК пульт
#include <IRremote.h>
int RECV_PIN = 13;
IRrecv irrecv(RECV_PIN);
decode_results results;

// Bluetooth 
unsigned char Bluetooth_val;       //переменная команды
#define Lpwm_pin  9     // скорость левых колес
#define Rpwm_pin  4    //скорость правых колес
int pinLB = 8;     //  левое переднее колесо
int pinLF = 7;     // левое заднее колесо
int pinRB = 6;    // правое переднее колесо
int pinRF = 5;    // правое заднее колесо
unsigned char Lpwm_val = 255;
unsigned char Rpwm_val = 255;
int Car_state = 0;

void Create_Task()
{
	// Теперь создадим две задачи, чтобы они работали независимо
	 // друг от друга:
	xTaskCreate(
		TaskServo
		, (const portCHAR *)"Servo"  // Это просто любое имя, удобное
									  // для чтения человеком.
		, 128                        // Размер стека задачи
		, NULL
		, 2                          // Приоритет задачи.
		, NULL);

	xTaskCreate(
		TaskBluetoothRead
		, (const portCHAR *) "BluetoothRead"
		, 128                        // Этот размер стека может быть проверен
									  // и подстроен путем чтения Highwater.
		, NULL
		, 1                          // Приоритет задачи.
		, NULL);

	// Теперь автоматически и неявно для пользователя запустится scheduler,
	// который возьмет на себя управление планированием запуска отдельных задач.
}
void Control_config(void)
{
	pinMode(pinLB, OUTPUT); // pin 8
	pinMode(pinLF, OUTPUT); // pin 7
	pinMode(pinRB, OUTPUT); // pin 6 
	pinMode(pinRF, OUTPUT); // pin 5
	pinMode(Lpwm_pin, OUTPUT); // pin 9 (PWM) 
	pinMode(Rpwm_pin, OUTPUT); // pin 4 (PWM) 
	myservo.attach(12);
}
void Set_Speed(unsigned char Lpwm, unsigned char Rpwm) // установка скорости
{
	analogWrite(Lpwm_pin, Lpwm);
	analogWrite(Rpwm_pin, Rpwm);
}
void forward()     //  движение вперед
{
	digitalWrite(pinRB, LOW);
	digitalWrite(pinRF, HIGH);
	digitalWrite(pinLB, LOW);
	digitalWrite(pinLF, HIGH);
	Car_state = 1;
}
void turnR()        //поворот вправо
{
	digitalWrite(pinRB, LOW);  //making motor move towards right rear
	digitalWrite(pinRF, HIGH);
	digitalWrite(pinLB, HIGH);
	digitalWrite(pinLF, LOW);  //making motor move towards left front
	Car_state = 4;
}
void turnL()        //поворот влево
{
	digitalWrite(pinRB, HIGH);
	digitalWrite(pinRF, LOW);   //making motor move towards right front
	digitalWrite(pinLB, LOW);   //making motor move towards left rear
	digitalWrite(pinLF, HIGH);
	Car_state = 3;
}
void stopp()         //stop
{
	digitalWrite(pinRB, HIGH);
	digitalWrite(pinRF, HIGH);
	digitalWrite(pinLB, HIGH);
	digitalWrite(pinLF, HIGH);
	Car_state = 5;
}
void back()          // движение назад
{
	digitalWrite(pinRB, HIGH);  //making motor move towards right rear
	digitalWrite(pinRF, LOW);
	digitalWrite(pinLB, HIGH);  //making motor move towards left rear
	digitalWrite(pinLF, LOW);
	Car_state = 2;
}

void setup() {
	Create_Task();
	Serial.begin(9600); // Инициализация последовательного обмена данными на скорости 9600 бит в секунду
	irrecv.enableIRIn(); // Старт приема от ИК пульта
	Control_config(); // задаем конфигурацию разъемов платы
	Set_Speed(Lpwm_val, Rpwm_val); // устанавливаем максимальную скорость
	stopp(); // подаем команду остановки двигателей
}

void wall_distance(long a)
{
	if (a < 15)
	{
		back();
		vTaskDelay(1000/portTICK_PERIOD_MS);
		stopp();
	}
}

void loop() {
	/*if (irrecv.decode(&results)) {
	  Serial.println(results.value, HEX); // пишем на монитор код с ИК пульта
	  irrecv.resume();
	}

	  if(Serial.available()) // если есть какие то данные
	  {
	   Bluetooth_val=Serial.read();  // читаем Bluetooth
	  switch(Bluetooth_val)
	   {
		 case 'F':forward(); //UP
		 break;
		 case 'B': back();   //back
		 break;
		 case 'L':turnL();   //Left
		 break;
		 case 'R':turnR();  //Right
		 break;
		 case 'S':stopp();    //stop
		 break;
		 case 'M': Set_Speed(170,170);
		 break;
		 case 'H': Set_Speed(255,255);
		 break;
	   }
	  }

	for (pos = 45; pos <= 135; pos += 45)
	{
	  myservo.write(pos);
	  a=sr04.Distance();
	  Serial.print(a);
	  Serial.println("cm");
	  delay(250);
	 }
	for (pos = 135; pos >= 45; pos -= 45)
	{
	  myservo.write(pos);
	  a=sr04.Distance();
	  Serial.print(a);
	  Serial.println("cm");
	  delay(250);
	}
	if (a<20){
	  back();
	  delay(100);
	  stopp();
	}*/


}
void TaskServo(void *pvParameters)
{
	(void)pvParameters;

	for (;;) // A Task shall never return or exit.
	{
		for (pos = 45; pos <= 135; pos += 45)
		{
			myservo.write(pos);
			a = sr04.Distance();
			wall_distance(a);
			Serial.print(a);
			Serial.println("cm");
			vTaskDelay(250 / portTICK_PERIOD_MS);
		}
		for (pos = 135; pos >= 45; pos -= 45)
		{
			myservo.write(pos);
			a = sr04.Distance();
			wall_distance(a);
			Serial.print(a);
			Serial.println("cm");
			vTaskDelay(250 / portTICK_PERIOD_MS);
		}
		/* digitalWrite(13, HIGH);    // включение светодиода LED
		 vTaskDelay( 1000 / portTICK_PERIOD_MS ); // ожидание в 1 секунду
		 digitalWrite(13, LOW);     // включение светодиода LED
		 vTaskDelay( 1000 / portTICK_PERIOD_MS ); // ожидание в 1 секунду*/
	}
}

void TaskBluetoothRead(void *pvParameters)
{
	(void)pvParameters;

	for (;;)
	{
		if (Serial.available()) // если есть какие то данные
		{
			Bluetooth_val = Serial.read();  // читаем Bluetooth 
			switch (Bluetooth_val)
			{
			case 'F': forward(); //UP
				break;
			case 'B': back();   //back
				break;
			case 'L': turnL();   //Left
				break;
			case 'R': turnR();  //Right
				break;
			case 'S': stopp();    //stop
				break;
			case 'M': Set_Speed(170, 170);
				break;
			case 'H': Set_Speed(255, 255);
				break;
			}
			// Задержка в 1 тик (15 мс) между чтениями, для стабильности:
			vTaskDelay(1);
		}
	}
}